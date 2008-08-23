/*
 * Copyright 2007-2008 by Sascha Hlusiak. <saschahlusiak@freedesktop.org>     
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
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <xf86Module.h>

#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 3


#include <misc.h>
#include <xf86.h>
#include <X11/Xatom.h>
#include <xf86Xinput.h>
#include <exevents.h>

#include "jstk.h"
#include "jstk_properties.h"



#define JSTK_PROP_DEBUGLEVEL "Debug Level"
static Atom prop_debuglevel     = 0;	/* 8 bit, 0..20 */

#define JSTK_PROP_MOUSE_ENABLED "Generate Mouse Events"
static Atom prop_mouse_enabled  = 0;	/* 8 bit, 0 or 1 */

#define JSTK_PROP_KEYS_ENABLED "Generate Key Events"
static Atom prop_keys_enabled   = 0;	/* 8 bit, 0 or 1 */

#define JSTK_PROP_DEADZONES   "Axes Deadzones"
static Atom prop_deadzones      = 0;	/* 32 bit, 0..30000 for each axis*/




static Bool
jstkSetProperty(DeviceIntPtr pJstk, Atom atom, XIPropertyValuePtr val)
{
    InputInfoPtr  pInfo = pJstk->public.devicePrivate;
    JoystickDevPtr priv = pInfo->private;
    int i;

    if (atom == prop_debuglevel)
    {
#if DEBUG
        if (val->size != 1 || val->format != 8 || val->type != XA_INTEGER)
            return FALSE;
        debug_level = *((INT8*)val->data);
	ErrorF("JOYSTICK: DebugLevel set to %d\n", debug_level);
#endif
    }else if (atom == prop_mouse_enabled)
    {
        if (val->size != 1 || val->format != 8 || val->type != XA_INTEGER)
            return FALSE;
        priv->mouse_enabled = (*((INT8*)val->data)) != 0;
        DBG(1, ErrorF("mouse_enabled set to %d\n", priv->mouse_enabled));
    }else if (atom == prop_keys_enabled)
    {
        if (val->size != 1 || val->format != 8 || val->type != XA_INTEGER)
            return FALSE;
        priv->keys_enabled = (*((INT8*)val->data)) != 0;
        DBG(1, ErrorF("keys_enabled set to %d\n", priv->keys_enabled));
    }else if (atom == prop_deadzones)
    {
        if (val->size > MAXAXES || val->format != 32 || val->type != XA_INTEGER)
            return FALSE;
        if (val->size == 1) { /* Single value to be applied to all axes */
            INT32 value;
            value = *((INT32*)val->data);
            if (value < 0) value = (-value);
            if (value > 30000) return FALSE;
            for (i =0; i<MAXAXES; i++)
                priv->axis[i].deadzone = value;
            DBG(1, ErrorF("Deadzone of all axes set to %d\n",value));
        } else { /* Apply supplied values to axes beginning with the first */
            INT32 *values;
            values = (INT32*)val->data;
            for (i =0; i<val->size; i++) /* Fail, if one value is out of range */ 
                if (values[i] > 30000 || values[i] < -30000)
                    return FALSE;
            for (i =0; i<val->size; i++) {
                priv->axis[i].deadzone = (values[i]<0)?(-values[i]):(values[i]);
                DBG(1, ErrorF("Deadzone of axis %d set to %d\n",i, priv->axis[i].deadzone));
            }
        }
    }


    /* property not handled, report success */
    return TRUE;
}

static Bool
jstkGetProperty(DeviceIntPtr pJstk, Atom property)
{
    return TRUE;
}


Bool
jstkInitProperties(DeviceIntPtr pJstk, JoystickDevPtr priv)
{
    int axes_values[MAXAXES];
    INT32 values[32]; /* We won't tell about properties with
                         more than 32 possible values */
    int i;

    XIRegisterPropertyHandler(pJstk, jstkSetProperty, jstkGetProperty);

#ifdef DEBUG
    /* Debug Level */
    prop_debuglevel = MakeAtom(JSTK_PROP_DEBUGLEVEL, strlen(JSTK_PROP_DEBUGLEVEL), TRUE);
    XIChangeDeviceProperty(pJstk, prop_debuglevel, XA_INTEGER, 8,
                                PropModeReplace, 1,
                                &debug_level,
                                FALSE, FALSE, FALSE);
#endif


    /* priv->mouse_enabled */
    prop_mouse_enabled = MakeAtom(JSTK_PROP_MOUSE_ENABLED, strlen(JSTK_PROP_MOUSE_ENABLED), TRUE);
    XIChangeDeviceProperty(pJstk, prop_mouse_enabled, XA_INTEGER, 8,
                                PropModeReplace, 1,
                                &priv->mouse_enabled,
                                FALSE, FALSE, FALSE);
    values[0] = 0;
    values[1] = 1;
    XIConfigureDeviceProperty(pJstk, prop_mouse_enabled, FALSE, FALSE, FALSE, 2, values);


    /* priv->keys_enabled */
    prop_keys_enabled = MakeAtom(JSTK_PROP_KEYS_ENABLED, strlen(JSTK_PROP_KEYS_ENABLED), TRUE);
    XIChangeDeviceProperty(pJstk, prop_keys_enabled, XA_INTEGER, 8,
                                PropModeReplace, 1,
                                &priv->keys_enabled,
                                FALSE, FALSE, FALSE);
    values[0] = 0;
    values[1] = 1;
    XIConfigureDeviceProperty(pJstk, prop_mouse_enabled, FALSE, FALSE, FALSE, 2, values);


    /* priv->axis[].deadzone */
    for (i=0;i<MAXAXES;i++)
        axes_values[i] = priv->axis[i].deadzone;
    prop_deadzones = MakeAtom(JSTK_PROP_DEADZONES, strlen(JSTK_PROP_DEADZONES), TRUE);
    XIChangeDeviceProperty(pJstk, prop_deadzones, XA_INTEGER, 32,
                                PropModeReplace, MAXAXES,
                                axes_values,
                                FALSE, FALSE, FALSE);



    return TRUE;
}

#endif
