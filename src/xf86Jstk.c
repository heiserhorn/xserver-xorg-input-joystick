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

#include <xf86Version.h>

#include <misc.h>
#include <xf86.h>
#include <xf86Xinput.h>
#include <xisb.h>
#include <exevents.h>		/* Needed for InitValuator/Proximity stuff */
#include <X11/keysym.h>

#ifdef XFree86LOADER
#include <xf86Module.h>
#endif


#include "xf86Jstk.h"
#include "linux_jstk.h"



/******************************************************************************
 * debugging macro
 *****************************************************************************/
#ifdef DBG
#undef DBG
#endif
#ifdef DEBUG
#undef DEBUG
#endif

#define DEBUG 1

#if DEBUG
static int      debug_level = 0;
#define DBG(lvl, f) {if ((lvl) <= debug_level) f;}
#else
#define DBG(lvl, f)
#endif



/****************************************************************************
 * Forward declarations
 ****************************************************************************/

static Bool xf86JstkProc(DeviceIntPtr pJstk, int what);



/*
 ***************************************************************************
 *
 * xf86JstkConvert --
 *	Convert valuators to X and Y.
 *
 ***************************************************************************
 */
/* FIXME: What is this for? */

static Bool
xf86JstkConvert(LocalDevicePtr	local,
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
 * xf86JstkTimer --
 *      A timer fired
 */
/* FIXME */

static CARD32
xf86JstkTimer(OsTimerPtr        timer,
               CARD32            atime,
               pointer           arg)
{
  DeviceIntPtr          device = (DeviceIntPtr)arg;
  JoystickDevPtr        priv = (JoystickDevPtr) XI_PRIVATE(device);
  int                   x, y, buttons;

//   int sigstate;
// 
// 
   DBG(5, ErrorF("xf86JstkEvents BEGIN device=0x%x priv=0x%x"
                 " timeout=%d\n",
                 device, priv, priv->timeout));


//     sigstate = xf86BlockSIGIO ();
// 	xf86PostMotionEvent(device, 0, 0, 2, v0, v1);
// 	    xf86PostButtonEvent(device, 0, loop+1, ((buttons & (1<<loop)) == (1<<loop)),
// 				0, 2, v0, v1);
//     xf86UnblockSIGIO (sigstate);


  return priv->timeout;     /* New timeout value */
}




/*
 * xf86JstkRead --
 *      This is called, when there is data to read at the device
 */

static void
xf86JstkRead(LocalDevicePtr local)
{
  enum JOYSTICKEVENT event;
  int number;

  JoystickDevPtr priv = local->private;
  DBG(2, ErrorF("xf86JstkRead\n"));

  if (xf86ReadJoystickData(priv, &event, &number)==0) {
    int sigstate;
    xf86Msg(X_WARNING, "JOYSTICK: Read failed. Deactivating device.\n");

    if (local->fd >= 0)
      RemoveEnabledDevice(local->fd);
    return;
  }

  /* A button's status changed */
  if (event == EVENT_BUTTON) {
    switch (priv->button[number].mapping) {
      case MAPPING_BUTTON:
        xf86PostButtonEvent(local->dev, 0, priv->button[number].value,
          priv->button[number].pressed, 0, 0);
        break;

      case MAPPING_X: /* FIXME */
        break;
      case MAPPING_Y: /* FIXME */
        break;
      case MAPPING_ZX: /* FIXME */
        break;
      case MAPPING_ZY: /* FIXME */
        break;
      case MAPPING_KEY: /* FIXME */
        break;
      case MAPPING_SPEED_MULTIPLY: /* FIXME */
        break;
      case MAPPING_DISABLE: /* FIXME */
        break;
      case MAPPING_DISABLE_MOUSE: /* FIXME */
        break;
      case MAPPING_DISABLE_KEYS: /* FIXME */
        break;
    }
  }

  /* An axis was moved */
  if (event == EVENT_AXIS) {
    switch (priv->button[number].mapping) {
      case MAPPING_X: /* FIXME */
        break;
      case MAPPING_Y: /* FIXME */
        break;
      case MAPPING_ZX: /* FIXME */
        break;
      case MAPPING_ZY: /* FIXME */
        break;
    }
  }
}



/*
 * xf86JstkProc --
 *      Handle the initialization, etc. of a joystick
 */
static Bool
xf86JstkProc(DeviceIntPtr       pJstk,
	     int                what)
{
  int i;
  CARD8                 map[MAXBUTTONS];
  LocalDevicePtr        local = (LocalDevicePtr)pJstk->public.devicePrivate;
  JoystickDevPtr        priv = (JoystickDevPtr)XI_PRIVATE(pJstk);

  switch (what)
    {
    case DEVICE_INIT: 
      DBG(1, ErrorF("xf86JstkProc pJstk=0x%x what=INIT\n", pJstk));
      for (i=1; i<MAXBUTTONS; i++) map[i] = i;

      if (InitButtonClassDeviceStruct(pJstk,
                                      priv->buttons,
                                      map) == FALSE) 
        {
          ErrorF("unable to allocate Button class device\n");
          return !Success;
        }

      if (InitFocusClassDeviceStruct(pJstk) == FALSE)
        {
          ErrorF("unable to init Focus class device\n");
          return !Success;
        }

//       if (InitPtrFeedbackClassDeviceStruct(pJstk,
//                                            xf86JstkControlProc) == FALSE)
//         {
//           ErrorF("unable to init ptr feedback\n");
//           return !Success;
//         }

      if (InitValuatorClassDeviceStruct(pJstk, 
                                    2,    /* FIXME */
                                    xf86GetMotionEvents, 
                                    local->history_size,
                                    Relative) /* relative or absolute */
          == FALSE) 
        {
          ErrorF("unable to allocate Valuator class device\n"); 
          return !Success;
        }
      else 
        {
	    InitValuatorAxisStruct(pJstk,
				   0,
				   0, /* min val */
				   screenInfo.screens[0]->width, /* max val */
				   1, /* resolution */
				   0, /* min_res */
				   1); /* max_res */
	    InitValuatorAxisStruct(pJstk,
				   1,
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
      i = xf86JoystickOn(priv, FALSE);

      DBG(1, ErrorF("xf86JstkProc  pJstk=0x%x what=ON name=%s\n", pJstk,
                    priv->device));

      if (i != 0)
      {
        priv->timer = TimerSet(NULL, 0, /*TimerAbsolute,*/
                                   priv->timeout,
                                   xf86JstkTimer,
                                   (pointer)pJstk);
        pJstk->public.on = TRUE;
        local->fd = priv->fd;
        AddEnabledDevice(local->fd);
      }
      else
        return !Success;
    break;

    case DEVICE_OFF:
    case DEVICE_CLOSE:
      DBG(1, ErrorF("xf86JstkProc  pJstk=0x%x what=%s\n", pJstk,
                    (what == DEVICE_CLOSE) ? "CLOSE" : "OFF"));

      if (priv->timer)
        TimerFree(priv->timer);
      priv->timer = NULL;
      if (local->fd >= 0)
        RemoveEnabledDevice(local->fd);
      local->fd = -1;
      xf86JoystickOff(priv);
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
 * xf86JstckUnplug --
 *
 * called when the module subsection is found in XF86Config
 */
static void
xf86JstkUnplug(pointer	p)
{
/*    LocalDevicePtr local = (LocalDevicePtr) p;
    JoystickDevPtr priv = (JoystickDevPtr) local->private;
    
    
    xf86JstkProc(local->dev, DEVICE_OFF);
    
    xfree (priv);
    xfree (local);*/
    ErrorF("xf86JstckUnplug\n");
}

/*
 * xf86JstckPlug --
 *
 * called when the module subsection is found in XF86Config
 */

static InputInfoPtr
xf86JstkCorePreInit(InputDriverPtr drv, IDevPtr dev, int flags)
{
    LocalDevicePtr	local = NULL;
    JoystickDevPtr	priv = NULL;
    char                *s;
    int                 i;

    local = xf86AllocateInput(drv, 0);
    if (!local) {
        goto SetupProc_fail;
    }

    local->private = (JoystickDevPtr)xalloc(sizeof(JoystickDevRec));
    priv = (JoystickDevPtr) local->private;
  
    local->name = dev->identifier;
    local->flags = XI86_POINTER_CAPABLE | XI86_SEND_DRAG_EVENTS;
    local->device_control = xf86JstkProc;
    local->read_input = xf86JstkRead;
    local->close_proc = NULL;
    local->control_proc = NULL;
    local->switch_mode = NULL;
    local->conversion_proc = xf86JstkConvert;
    local->fd = -1;
    local->dev = NULL;
    local->private = priv;
    local->type_name = "JOYSTICK";
    local->history_size  = 0;
    local->always_core_feedback = 0;
    local->conf_idev = dev;

    priv->fd = -1;
    priv->timer = NULL;
    priv->timeout = 10;
    priv->device = NULL;

    /* Initialize default mappings */
    for (i=0; i<MAXAXES; i++) {
      priv->axis[i].value     = 0;
      priv->axis[i].deadzone  = 1000;
      priv->axis[i].type      = TYPE_BYVALUE;
      priv->axis[i].mapping   = MAPPING_NONE;
    }
    for (i=0; i<MAXBUTTONS; i++) {
      priv->button[i].pressed = 0;
      priv->button[i].value   = i+1;  /* The button number to simulate */
      priv->button[i].mapping = MAPPING_BUTTON;
    }

    xf86CollectInputOptions(local, NULL, NULL);
    xf86OptionListReport(local->options);

    /* Joystick device is mandatory */
    priv->device = xf86CheckStrOption(dev->commonOptions, "Device", NULL);

    if (!priv->device) {
	xf86Msg (X_ERROR, "JOYSTICK: No Device specified.\n");
/*	*errmaj = LDR_BADUSAGE;*/
	goto SetupProc_fail;
    }

    xf86Msg(X_CONFIG, "JOYSTICK device is %s\n", priv->device);
    if (xf86JoystickOn(priv, TRUE) == -1) {
	goto SetupProc_fail;
    }
    xf86JoystickOff(priv);


    xf86ProcessCommonOptions(local, local->options);

#if DEBUG
    debug_level = xf86SetIntOption(dev->commonOptions, "DebugLevel", 0);
    if (debug_level > 0) {
	xf86Msg(X_CONFIG, "JOYSTICK: debug level set to %d\n", debug_level);
    }
#else
    if (xf86SetIntOption(dev->commonOptions, "DebugLevel", 0) != 0) {
        xf86Msg(X_WARNING, "JOYSTICK: Compiled without Debug support!\n");
    }
#endif

    priv->timeout = xf86SetIntOption(dev->commonOptions, "Timeout", 10);

    /* Process button mapping options */
    for (i=0; i<priv->buttons; i++) if (i<MAXBUTTONS) {
      char p[20];

      sprintf(p,"MapButton%d",i+1);
      s = xf86CheckStrOption(dev->commonOptions, p, NULL);
      if (s != NULL) {
        char *param;
        char *tmp;
        int value;
        param = xstrdup(s);

        for (tmp = param; *tmp; tmp++) *tmp = toupper(*tmp);

        if (sscanf(param, "BUTTON %d", &value) == 1) {
          priv->button[i].mapping = MAPPING_BUTTON;
          priv->button[i].value   = value;
          xf86Msg(X_CONFIG, "JOYSTICK: button %d mapped to mouse button %d\n", 
            i+1, value);
        }else xf86Msg(X_WARNING, "JOYSTICK: '%s' has invalid format ('%s'), ignored.\n", p, param);

        xfree(param);
      }
    }




    /* return the LocalDevice */
    local->flags |= XI86_CONFIGURED ;

    return (local);
  SetupProc_fail:
    if (priv)
        xfree(priv);
    if (local)
        xfree(local);
    return NULL;
}


/*
 * xf86JstkCoreUnInit --
 *
 * called when the driver is unloaded.
 */
static void
xf86JstkCoreUnInit(InputDriverPtr    drv,
                   LocalDevicePtr    local,
                   int               flags)
{
    JoystickDevPtr device = (JoystickDevPtr) local->private;

    DBG(1, ErrorF("xf86JstkUninit\n"));

    xf86JstkProc(local->dev, DEVICE_OFF);

    xfree (device);
    xf86DeleteInput(local, 0);
}

_X_EXPORT InputDriverRec JOYSTICK = {
    1,
    "joystick",
    NULL,
    xf86JstkCorePreInit,
    xf86JstkCoreUnInit,
    NULL,
    0
};

static pointer
xf86JstkPlug(pointer	module,
             pointer	options,
             int	*errmaj,
             int	*errmin)
{
    xf86AddInputDriver(&JOYSTICK, module, 0);
    return module;
}

static XF86ModuleVersionInfo xf86JstkVersionRec =
{
	"joystick",
	MODULEVENDORSTRING,
	MODINFOSTRING1,
	MODINFOSTRING2,
	XORG_VERSION_CURRENT,
	1, 1, 1,
	ABI_CLASS_XINPUT,
	ABI_XINPUT_VERSION,
	MOD_CLASS_XINPUT,
	{0, 0, 0, 0}		/* signature, to be patched into the file by */
				/* a tool */
};

_X_EXPORT XF86ModuleData joystickModuleData = {
    &xf86JstkVersionRec,
    xf86JstkPlug,
    xf86JstkUnplug
};
#endif /* XFree86LOADER */

	
/* end of xf86Jstk.c */
