/*
 * Copyright 2007-2008 by Sascha Hlusiak. <saschahlusiak@freedesktop.org>     
 * Copyright 1995-1999 by Frederic Lepied, France. <Lepied@XFree86.org>       
 *                                                                            
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is  hereby granted without fee, provided that
 * the  above copyright   notice appear  in   all  copies and  that both  that
 * copyright  notice   and   this  permission   notice  appear  in  supporting
 * documentation, and that  the  names  of copyright holders not  be  used  in
 * advertising or publicity pertaining to distribution of the software without
 * specific,  written      prior  permission.  The copyright holders  make  no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.                   
 *                                                                            
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED   WARRANTIES OF MERCHANTABILITY  AND   FITNESS, IN NO
 * EVENT  SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT   OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA  OR PROFITS, WHETHER  IN  AN ACTION OF  CONTRACT,  NEGLIGENCE OR OTHER
 * TORTIOUS  ACTION, ARISING    OUT OF OR   IN  CONNECTION  WITH THE USE    OR
 * PERFORMANCE OF THIS SOFTWARE.
 */



#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <xorg-server.h>
#include <xf86.h>
#include <xf86Xinput.h>
#include <X11/extensions/XKBsrv.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <xf86Optrec.h>

#include "jstk.h"
#include "jstk_key.h"

#include <xkbsrv.h>

#define MIN_KEYCODE 8
#define GLYPHS_PER_KEY 2

#define AltMask		Mod1Mask
#define NumLockMask	Mod2Mask
#define AltLangMask	Mod3Mask
#define KanaMask	Mod4Mask
#define ScrollLockMask	Mod5Mask

static void
jstkKbdCtrl(DeviceIntPtr device, KeybdCtrl *ctrl)
{
}


/*
 ***************************************************************************
 *
 * jstkInitKeys --
 *
 * Sets up the keymap, modmap and the keyboard device structs
 *
 ***************************************************************************
 */
static int
jstkInitKeys(DeviceIntPtr pJstk, JoystickDevPtr priv)
{
    XkbSrvInfoPtr xkbi;
    XkbControlsPtr ctrls;

    if (!InitKeyboardDeviceStruct(pJstk, &priv->rmlvo, NULL, jstkKbdCtrl))
    {
        ErrorF("unable to init keyboard device\n");
        return !Success;
    }

    /* Set Autorepeat and Delay */
    if ((priv->repeat_delay || priv->repeat_interval) && 
        pJstk->key && 
        pJstk->key->xkbInfo)
    {
        xkbi = pJstk->key->xkbInfo;
        if (xkbi && xkbi->desc)
        {
            ctrls = xkbi->desc->ctrls;
            ctrls->repeat_delay = priv->repeat_delay;
            ctrls->repeat_interval = priv->repeat_interval;
        }
    }

    return Success;
}



/*
 ***************************************************************************
 *
 * jstkGenerateKeys
 *
 * Generates a series of keydown or keyup events of the specified 
 * KEYSCANCODES
 *
 ***************************************************************************
 */
void
jstkGenerateKeys(InputInfoPtr device, KEYSCANCODES keys, char pressed)
{
    int i;
    unsigned int k;

    if (device == NULL)
        return;
    for (i=0;i<MAXKEYSPERBUTTON;i++) {
        if (pressed != 0) 
            k = keys[i];
        else k = keys[MAXKEYSPERBUTTON - i - 1];

        if (k != 0) {
            DBG(2, ErrorF("Generating key %s event with keycode %d\n", 
                (pressed)?"press":"release", k));
            xf86PostKeyboardEvent(device->dev, k, pressed);
        }
    }
}


/*
 ***************************************************************************
 *
 * jstkKeyboardDeviceControlProc --
 *
 * Handles the initialization, etc. of the keyboard device
 *
 ***************************************************************************
 */
static Bool
jstkKeyboardDeviceControlProc(DeviceIntPtr       dev,
                              int                what)
{
    JoystickDevPtr priv  = (JoystickDevPtr)XI_PRIVATE(dev);
    if (!priv) {
        DBG(2, ErrorF("jstkKeyboardDeviceControlProc: priv == NULL\n"));
        return !Success;
    }
    switch (what) {
    case DEVICE_INIT:
        DBG(2, ErrorF("jstkKeyboardDeviceControlProc what=DEVICE_INIT\n"));
        if (InitFocusClassDeviceStruct(dev) == FALSE) {
            ErrorF("unable to init Focus class device\n");
            return !Success;
        }
        if (jstkInitKeys(dev, priv) != Success)
            return !Success;
        break;
    case DEVICE_ON:
        DBG(2, ErrorF("jstkKeyboardDeviceControlProc what=DEVICE_ON\n"));
        dev->public.on = TRUE;
        break;
    case DEVICE_OFF:
        DBG(2, ErrorF("jstkKeyboardDeviceControlProc what=DEVICE_OFF\n"));
        dev->public.on = FALSE;
        break;
    case DEVICE_CLOSE:
        DBG(2, ErrorF("jstkKeyboardDeviceControlProc what=DEVICE_CLOSE\n"));
        dev->public.on = FALSE;
        break;
    }

    return Success;
}


/*
 ***************************************************************************
 *
 * jstkKeyboardPreInit --
 *
 * Called manually to create a keyboard device for the joystick
 *
 ***************************************************************************
 */
InputInfoPtr
jstkKeyboardPreInit(InputDriverPtr drv, IDevPtr _dev, int flags)
{
    InputInfoPtr pInfo = NULL;
    IDevPtr dev = NULL;
    char name[512] = {0};

    pInfo = xf86AllocateInput(drv, 0);
    if (!pInfo) {
        goto SetupProc_fail;
    }

    dev = calloc(sizeof(IDevRec), 1);
    strcpy(name, _dev->identifier);
    strcat(name, " (keys)");
    dev->identifier = xstrdup(name);
    dev->driver = xstrdup(_dev->driver);
    dev->commonOptions = (pointer)xf86optionListDup(_dev->commonOptions);
    dev->extraOptions = (pointer)xf86optionListDup(_dev->extraOptions);

    pInfo->name   = dev->identifier;
    pInfo->flags  = XI86_KEYBOARD_CAPABLE;
    pInfo->device_control = jstkKeyboardDeviceControlProc;
    pInfo->read_input = NULL;
    pInfo->control_proc = NULL;
    pInfo->switch_mode = NULL;
    pInfo->fd = -1;
    pInfo->dev = NULL;
    pInfo->private = NULL;
    pInfo->type_name = XI_JOYSTICK;
    pInfo->history_size = 0;
    pInfo->always_core_feedback = 0;
    pInfo->conf_idev = dev;

    xf86CollectInputOptions(pInfo, NULL, NULL);
    xf86OptionListReport(pInfo->options);
    xf86ProcessCommonOptions(pInfo, pInfo->options);


    /* return the LocalDevice */
    pInfo->flags |= XI86_CONFIGURED;

    return (pInfo);

SetupProc_fail:
    if (pInfo)
        pInfo->private = NULL;
    if (dev) {
        if (dev->identifier) free(dev->identifier);
        if (dev->driver) free(dev->driver);
        free(dev);
    }
    return NULL;
}


/*
 ***************************************************************************
 *
 * jstkKeyboardUnInit --
 *
 * Called when the keyboard slave device gets removed
 *
 ***************************************************************************
 */
void
jstkKeyboardUnInit(InputDriverPtr    drv,
                   InputInfoPtr      pInfo,
                   int               flags)
{
    JoystickDevPtr device = (JoystickDevPtr) pInfo->private;
    DBG(2, ErrorF("jstkKeyboardUnInit.\n"));

    device->keyboard_device = NULL;
    pInfo->private = NULL;

    xf86DeleteInput(pInfo, 0);
}

