/*
 * Copyright 2007      by Sascha Hlusiak. <saschahlusiak@freedesktop.org>
 * Copyright 1995-1999 by Frederic Lepied, France. <Lepied@XFree86.org>       
 *                                                                            
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is  hereby granted without fee, provided that
 * the  above copyright   notice appear  in   all  copies and  that both  that
 * copyright  notice   and   this  permission   notice  appear  in  supporting
 * documentation, and that   the  name of  Sascha   Hlusiak  not  be  used  in
 * advertising or publicity pertaining to distribution of the software without
 * specific,  written      prior  permission.     Sascha   Hlusiak   makes  no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.                   
 *                                                                            
 * SASCHA  HLUSIAK  DISCLAIMS ALL   WARRANTIES WITH REGARD  TO  THIS SOFTWARE,
 * INCLUDING ALL IMPLIED   WARRANTIES OF MERCHANTABILITY  AND   FITNESS, IN NO
 * EVENT  SHALL SASCHA  HLUSIAK  BE   LIABLE   FOR ANY  SPECIAL, INDIRECT   OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA  OR PROFITS, WHETHER  IN  AN ACTION OF  CONTRACT,  NEGLIGENCE OR OTHER
 * TORTIOUS  ACTION, ARISING    OUT OF OR   IN  CONNECTION  WITH THE USE    OR
 * PERFORMANCE OF THIS SOFTWARE.
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <xf86Version.h>

#include <misc.h>
#include <xf86.h>
#include <xf86Xinput.h>
#include <xisb.h>
#include <exevents.h>		/* Needed for InitValuator/Proximity stuff */

#include <math.h>

#ifdef XFree86LOADER
#include <xf86Module.h>
#endif


#include "jstk.h"
#include "jstk_hw.h"
#include "jstk_axis.h"
#include "jstk_options.h"



int debug_level = 0;


/*
 ***************************************************************************
 *
 * jstkConvertProc --
 *
 * Convert valuators to X and Y.
 *
 ***************************************************************************
 */

static Bool
jstkConvertProc(LocalDevicePtr	local,
		int		first,
		int		num,
		int		v0,
		int		v1,
		int		v2,
		int		v3,
		int		v4,
		int		v5,
		int*		x,
		int*		y)
{
    if (first != 0 || num != 2)
      return FALSE;

    *x = v0;
    *y = v1;

    return TRUE;
}



/*
 ***************************************************************************
 *
 * jstkReadProc --
 *
 * Called when data is available to read from the device
 * Reads the data and process the events
 *
 ***************************************************************************
 */

static void
jstkReadProc(LocalDevicePtr local)
{
  enum JOYSTICKEVENT event;
  int number;
  int i;
  int r;

  JoystickDevPtr priv = local->private;

  do {
  if ((r=jstkReadData(priv, &event, &number))==0) {
    xf86Msg(X_WARNING, "JOYSTICK: Read failed. Deactivating device.\n");

    if (local->fd >= 0)
       RemoveEnabledDevice(local->fd);
    return;
  }

  /* A button's status changed */
  if (event == EVENT_BUTTON) {
    DBG(4, ErrorF("Button %d %s. Mapping: %d\n", number, 
                 (priv->button[number].pressed == 0) ? "released" : "pressed", 
                 priv->button[number].mapping));
    switch (priv->button[number].mapping) {
      case MAPPING_BUTTON:
        if (priv->mouse_enabled == TRUE) {
          xf86PostButtonEvent(local->dev, 0, priv->button[number].value,
            priv->button[number].pressed, 0, 0);
        }
        break;

      case MAPPING_X:
      case MAPPING_Y:
      case MAPPING_ZX:
      case MAPPING_ZY:
        if (priv->button[number].pressed == 0) /* If button was released */
          priv->button[number].temp = 1.0;     /* Reset speed counter */
        else if (priv->mouse_enabled == TRUE)
          jstkStartButtonAxisTimer(local, number);
        break;

      case MAPPING_KEY:
        if (priv->keys_enabled == TRUE)
        for (i=0;i<MAXKEYSPERBUTTON;i++) {
          unsigned int k;
          if (priv->button[number].pressed == 1) 
            k = priv->button[number].keys[i];
            else k = priv->button[number].keys[MAXKEYSPERBUTTON - i - 1];

          if (k != 0) {
            DBG(2, ErrorF("Generating key %s event with keycode %d\n", 
              (priv->button[number].pressed)?"press":"release", k));
            xf86PostKeyboardEvent(local->dev, k, priv->button[number].pressed);
          }
        }
        break;

      case MAPPING_SPEED_MULTIPLY:
        priv->amplify = 1.0;
        /* Calculate new global amplify value by multiplying them all */
        for (i=0; i<MAXAXES; i++) {
          if ((priv->button[i].pressed) && 
              (priv->button[i].mapping == MAPPING_SPEED_MULTIPLY))
            priv->amplify *= ((float)priv->button[i].value) / 1000.0f;
        }
        DBG(2, ErrorF("Global amplify is now %.3f\n", priv->amplify));

        break;

      case MAPPING_DISABLE:
        if (priv->button[number].pressed == 1) {
          if ((priv->mouse_enabled == TRUE) || (priv->keys_enabled == TRUE)) {
            priv->mouse_enabled = FALSE;
            priv->keys_enabled = FALSE;
            DBG(2, ErrorF("All events disabled\n"));
          } else {
            priv->mouse_enabled = TRUE;
            priv->keys_enabled = TRUE;
            DBG(2, ErrorF("All events enabled\n"));
          }
        }
        break;
      case MAPPING_DISABLE_MOUSE:
        if (priv->button[number].pressed == 1) {
          if (priv->mouse_enabled == TRUE) priv->mouse_enabled = FALSE;
            else priv->mouse_enabled = TRUE;
          DBG(2, ErrorF("Mouse events %s\n", 
              priv->mouse_enabled ? "enabled" : "disabled"));
        }
        break;
      case MAPPING_DISABLE_KEYS:
        if (priv->button[number].pressed == 1) {
          if (priv->keys_enabled == TRUE) priv->keys_enabled = FALSE;
            else priv->keys_enabled = TRUE;
          DBG(2, ErrorF("Keyboard events %s\n", 
              priv->mouse_enabled ? "enabled" : "disabled"));
        }
        break;

      default:
        break;
    }
  }

  /* An axis was moved */
  if ((event == EVENT_AXIS) && 
      (priv->axis[number].mapping != MAPPING_NONE) &&
      (priv->axis[number].type != TYPE_NONE)) {
    DBG(5, ErrorF("Axis %d moved to %d. Type: %d, Mapping: %d\n", number,
                  priv->axis[number].value,
                  priv->axis[number].type,
                  priv->axis[number].mapping));
    switch (priv->axis[number].type) {
      case TYPE_BYVALUE:
      case TYPE_ACCELERATED:
        if (priv->axis[number].value == 0) /* When axis was released */
          priv->axis[number].temp = 1.0;   /* Release speed counter */

        if (priv->mouse_enabled == TRUE)
          jstkStartAxisTimer(local, number);
        break;

      case TYPE_ABSOLUTE:
        if (priv->mouse_enabled == TRUE)
          jstkHandleAbsoluteAxis(local, number);
        break;

      default:
        break;
    }
  }
  while (r == 2);
}



/*
 ***************************************************************************
 *
 * jstkDeviceControlProc --
 *
 * Handles the initialization, etc. of a joystick
 *
 ***************************************************************************
 */

static Bool
jstkDeviceControlProc(DeviceIntPtr       pJstk,
                      int                what)
{
  int i;
  CARD8            map[MAXBUTTONS];
  LocalDevicePtr   local = (LocalDevicePtr)pJstk->public.devicePrivate;
  JoystickDevPtr   priv  = (JoystickDevPtr)XI_PRIVATE(pJstk);

  switch (what)
    {
    case DEVICE_INIT:
      DBG(1, ErrorF("jstkDeviceControlProc what=INIT\n"));
      for (i=1; i<MAXBUTTONS; i++) map[i] = i;

      if (InitButtonClassDeviceStruct(pJstk, MAXBUTTONS, map) == FALSE) {
        ErrorF("unable to allocate Button class device\n");
        return !Success;
      }

      if (InitFocusClassDeviceStruct(pJstk) == FALSE) {
        ErrorF("unable to init Focus class device\n");
        return !Success;
      }

      if (InitValuatorClassDeviceStruct(pJstk, 
                                        2,
                                        xf86GetMotionEvents, 
                                        local->history_size,
                                        Relative) == FALSE) {
        ErrorF("unable to allocate Valuator class device\n"); 
        return !Success;
      } else {
        InitValuatorAxisStruct(pJstk,
                               0, /* valuator num */
                               0, /* min val */
                               screenInfo.screens[0]->width, /* max val */
                               1, /* resolution */
                               0, /* min_res */
                               1); /* max_res */
        InitValuatorAxisStruct(pJstk,
                               1, /* valuator num */
                               0, /* min val */
                               screenInfo.screens[0]->height, /* max val */
                               1, /* resolution */
                               0, /* min_res */
                               1); /* max_res */

        /* allocate the motion history buffer if needed */
        xf86MotionHistoryAllocate(local);
      }
      break; 

    case DEVICE_ON:
      DBG(1, ErrorF("jstkDeviceControlProc  what=ON name=%s\n", priv->device));

      if (jstkOpenDevice(priv) != -1)
      {
        pJstk->public.on = TRUE;
        local->fd = priv->fd;
        AddEnabledDevice(local->fd);
      } else return !Success;
      break;

    case DEVICE_OFF:
    case DEVICE_CLOSE:
      DBG(1, ErrorF("jstkDeviceControlProc  what=%s\n", 
        (what == DEVICE_CLOSE) ? "CLOSE" : "OFF"));

      if (priv->timerrunning == TRUE) {
        priv->timerrunning = FALSE;
        TimerCancel(priv->timer);
      }

      if (local->fd >= 0)
        RemoveEnabledDevice(local->fd);
      local->fd = -1;
      jstkCloseDevice(priv);
      pJstk->public.on = FALSE;
      break;

    default:
      ErrorF("unsupported mode=%d\n", what);
      return !Success;
      break;
    }
  return Success;
}



/*
 ***************************************************************************
 *
 * Dynamic loading functions
 *
 ***************************************************************************
 */
#ifdef XFree86LOADER


/*
 ***************************************************************************
 *
 * jstkCorePreInit --
 *
 * Called when a device will be instantiated
 *
 ***************************************************************************
 */

static InputInfoPtr
jstkCorePreInit(InputDriverPtr drv, IDevPtr dev, int flags)
{
    LocalDevicePtr      local = NULL;
    JoystickDevPtr      priv = NULL;
    char                *s;
    int                 i;

    DBG(0, ErrorF("jstkCorePreInit\n"));
    local = xf86AllocateInput(drv, 0);
    if (!local) {
      goto SetupProc_fail;
    }

    local->private = (JoystickDevPtr)xalloc(sizeof(JoystickDevRec));
    priv = (JoystickDevPtr)local->private;

    local->name   = dev->identifier;
    local->flags  = XI86_POINTER_CAPABLE;
    local->flags |= XI86_KEYBOARD_CAPABLE;
    local->flags |= XI86_SEND_DRAG_EVENTS;
    local->device_control = jstkDeviceControlProc;
    local->read_input = jstkReadProc;
    local->close_proc = NULL;
    local->control_proc = NULL;
    local->switch_mode = NULL;
    local->conversion_proc = jstkConvertProc;
    local->fd = -1;
    local->dev = NULL;
    local->private = priv;
    local->type_name = "JOYSTICK";
    local->history_size = 0;
    local->always_core_feedback = 0;
    local->conf_idev = dev;

    priv->fd = -1;
    priv->device = NULL;
    priv->devicedata = NULL;
    priv->x  = 0.0f;
    priv->y  = 0.0f;
    priv->zx = 0.0f;
    priv->zy = 0.0f;
    priv->timer = NULL;
    priv->timerrunning = FALSE;
    priv->mouse_enabled = TRUE;
    priv->keys_enabled = TRUE;
    priv->amplify = 1.0f;
    priv->axes = 0;
    priv->buttons = 0;

    /* Initialize default mappings */
    for (i=0; i<MAXAXES; i++) {
      priv->axis[i].value     = 0;
      priv->axis[i].deadzone  = 1000;
      priv->axis[i].type      = TYPE_BYVALUE;
      priv->axis[i].mapping   = MAPPING_NONE;
      priv->axis[i].temp      = 0.0f;
      priv->axis[i].amplify   = 1.0f;
    }
    for (i=0; i<MAXBUTTONS; i++) {
      priv->button[i].pressed = 0;
      priv->button[i].value   = 0;
      priv->button[i].mapping = MAPPING_NONE;
      priv->button[i].temp    = 1.0f;
    }

    /* First three joystick buttons generate mouse clicks */
    priv->button[0].mapping = MAPPING_BUTTON;
    priv->button[0].value   = 1;
    priv->button[1].mapping = MAPPING_BUTTON;
    priv->button[1].value   = 2;
    priv->button[2].mapping = MAPPING_BUTTON;
    priv->button[2].value   = 3;

    /* Two axes by default */
    priv->axis[0].type      = TYPE_BYVALUE;
    priv->axis[0].mapping   = MAPPING_X;
    priv->axis[1].type      = TYPE_BYVALUE;
    priv->axis[1].mapping   = MAPPING_Y;


    xf86CollectInputOptions(local, NULL, NULL);
    xf86OptionListReport(local->options);

    /* Joystick device is mandatory */
    priv->device = xf86CheckStrOption(dev->commonOptions, "Device", NULL);

    if (!priv->device) {
      xf86Msg (X_ERROR, "%s: No Device specified.\n", local->name);
      goto SetupProc_fail;
    }

    xf86Msg(X_CONFIG, "%s: device is %s\n", local->name, priv->device);

    xf86ProcessCommonOptions(local, local->options);

#if DEBUG
    debug_level = xf86SetIntOption(dev->commonOptions, "DebugLevel", 0);
    if (debug_level > 0) {
      xf86Msg(X_CONFIG, "%s: debug level set to %d\n", 
              local->name, debug_level);
    }
#else
    if (xf86SetIntOption(dev->commonOptions, "DebugLevel", 0) != 0) {
      xf86Msg(X_WARNING, "%s: DebugLevel: Compiled without Debug support!\n", 
              local->name);
    }
#endif


    /* Process button mapping options */
    for (i=0; i<MAXBUTTONS; i++) {
      char p[64];
      sprintf(p,"MapButton%d",i+1);
      s = xf86CheckStrOption(dev->commonOptions, p, NULL);
      if (s != NULL) {
        xf86Msg(X_CONFIG, "%s: Option \"mapbutton%d\" \"%s\"\n",
                local->name, i+1, s);
        jstkParseButtonOption(s, priv, i, local->name);
      }
      DBG(1, ErrorF("Button %d mapped to %d (value=%d)\n", i+1, 
                    priv->button[i].mapping, priv->button[i].value));
    }

    /* Process button mapping options */
    for (i=0; i<MAXAXES; i++) {
      char p[64];
      sprintf(p,"MapAxis%d",i+1);
      s = xf86CheckStrOption(dev->commonOptions, p, NULL);
      if (s != NULL) {
        xf86Msg(X_CONFIG, "%s: Option \"mapaxis%d\" \"%s\"\n", 
                local->name, i+1, s);
        jstkParseAxisOption(s, &priv->axis[i], local->name);
      }
      DBG(1, ErrorF("Axis %d type is %d, mapped to %d, amplify=%.3f\n", i+1, 
                    priv->axis[i].type,
                    priv->axis[i].mapping,
                    priv->axis[i].amplify));
    }

    /* return the LocalDevice */
    local->flags |= XI86_CONFIGURED ;

    return (local);
  SetupProc_fail:
    if (priv)
        xfree(priv);
    if (local)
        local->private = NULL;
    return local;
}



/*
 ***************************************************************************
 *
 * jstkCoreUnInit --
 *
 * Called when a device is unplugged and needs to be removed
 *
 ***************************************************************************
 */

static void
jstkCoreUnInit(InputDriverPtr    drv,
               LocalDevicePtr    local,
               int               flags)
{
    JoystickDevPtr device = (JoystickDevPtr) local->private;

    DBG(0, ErrorF("jstkCoreUnInit\n"));

    jstkDeviceControlProc(local->dev, DEVICE_OFF);

    xfree (device);
    xf86DeleteInput(local, 0);
}




_X_EXPORT InputDriverRec JOYSTICK = {
    1,
    "joystick",
    NULL,
    jstkCorePreInit,
    jstkCoreUnInit,
    NULL,
    0
};


/*
 ***************************************************************************
 *
 * jstkPlug --
 *
 * Called when the driver is loaded
 *
 ***************************************************************************
 */

static pointer
jstkDriverPlug(pointer  module,
               pointer  options,
               int      *errmaj,
               int      *errmin)
{
    DBG(0, ErrorF("jstkDriverPlug\n"));
    xf86AddInputDriver(&JOYSTICK, module, 0);
    return module;
}


/*
 ***************************************************************************
 *
 * jstkDriverUnplug --
 *
 * Called when the driver is unloaded
 * NOTICE: When does this actually happen? What needs to be cleaned up?
 *
 ***************************************************************************
 */

static void
jstkDriverUnplug(pointer p)
{
    DBG(0, ErrorF("jstkDriverUnplug\n"));
}



/*
 ***************************************************************************
 *
 * Module information for X.Org
 *
 ***************************************************************************
 */
static XF86ModuleVersionInfo jstkVersionRec =
{
	"joystick",
	MODULEVENDORSTRING,
	MODINFOSTRING1,
	MODINFOSTRING2,
	XORG_VERSION_CURRENT,
	PACKAGE_VERSION_MAJOR,
	PACKAGE_VERSION_MINOR,
	PACKAGE_VERSION_PATCHLEVEL,
	ABI_CLASS_XINPUT,
	ABI_XINPUT_VERSION,
	MOD_CLASS_XINPUT,
	{0, 0, 0, 0}		/* signature, to be patched into the file by */
				/* a tool */
};


/*
 ***************************************************************************
 *
 * Exported module Data for X.Org
 *
 ***************************************************************************
 */
_X_EXPORT XF86ModuleData joystickModuleData = {
    &jstkVersionRec,
    jstkDriverPlug,
    jstkDriverUnplug
};

#endif /* XFree86LOADER */
