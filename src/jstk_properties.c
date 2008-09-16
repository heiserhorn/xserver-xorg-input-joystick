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


/* 8 bit, 0..20 */
#define JSTK_PROP_DEBUGLEVEL "Debug Level"
static Atom prop_debuglevel  = 0;

/* 8 bit, 0 or 1 */
#define JSTK_PROP_MOUSE_ENABLED "Generate Mouse Events"
static Atom prop_mouse_enabled  = 0;

/* 8 bit, 0 or 1 */
#define JSTK_PROP_KEYS_ENABLED "Generate Key Events"
static Atom prop_keys_enabled  = 0;

/* 32 bit, 0..30000 for each axis*/
#define JSTK_PROP_AXIS_DEADZONE   "Axis Deadzone"
static Atom prop_axis_deadzone    = 0;

/* 8 bit, one of enum _JOYSTICKTYPE @ jstk.h per axis*/
#define JSTK_PROP_AXIS_TYPE   "Axis Type"
static Atom prop_axis_type    = 0;

/* 8 bit, one of enum _JOYSTICKMAPPING @ jstk.h per axis */
#define JSTK_PROP_AXIS_MAPPING   "Axis Mapping"
static Atom prop_axis_mapping = 0;

/* float, movement amplify value per axis */
#define JSTK_PROP_AXIS_AMPLIFY "Axis amplify"
static Atom prop_axis_amplify = 0;

/* 16 bit, set keysyms for low axis. Format: (axis keysym1 keysym2 keysym3 keysym4) */
#define JSTK_PROP_AXIS_KEYS_LOW "Axis keys (low) (set only)"
static Atom prop_axis_keys_low = 0;

/* 16 bit, set keysyms for high axis. Format: (axis keysym1 keysym2 keysym3 keysym4) */
#define JSTK_PROP_AXIS_KEYS_HIGH "Axis keys (high) (set only)"
static Atom prop_axis_keys_high = 0;

/* 8 bit, one of enum _JOYSTICKMAPPING @ jstk.h per button */
#define JSTK_PROP_BUTTON_MAPPING   "Button Mapping"
static Atom prop_button_mapping = 0;

/* 8 bit, logical button number per button */
#define JSTK_PROP_BUTTON_BUTTONNUMBER "Button number"
static Atom prop_button_buttonnumber = 0;

/* float, button amplify value per button */
#define JSTK_PROP_BUTTON_AMPLIFY "Button amplify"
static Atom prop_button_amplify = 0;

/* 16 bit, set keysyms for button. Format: (button keysym1 keysym2 keysym3 keysym4) */
#define JSTK_PROP_BUTTON_KEYS "Button keys (set only)"
static Atom prop_button_keys = 0;



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
    }else if (atom == prop_axis_deadzone)
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
    }else if (atom == prop_axis_type)
    {
        INT8 *values;
        if (val->size > MAXAXES || val->format != 8 || val->type != XA_INTEGER)
            return FALSE;
        values = (INT8*)val->data;
        for (i =0; i<val->size; i++) {
            priv->axis[i].type = values[i];
            DBG(1, ErrorF("Type of axis %d set to %d\n",i, priv->axis[i].type));
        }
    }else if (atom == prop_axis_mapping)
    {
        INT8 *values;
        if (val->size > MAXAXES || val->format != 8 || val->type != XA_INTEGER)
            return FALSE;
        values = (INT8*)val->data;
        for (i =0; i<val->size; i++) {
            priv->axis[i].mapping = values[i];
            DBG(1, ErrorF("Mapping of axis %d set to %d\n",i, priv->axis[i].mapping));
        }
    }else if (atom == prop_axis_amplify)
    {
        /* FIXME */
        return FALSE;
    }else if (atom == prop_axis_keys_low)
    {
        /* FIXME */
        return FALSE;
    }else if (atom == prop_axis_keys_high)
    {
        /* FIXME */
        return FALSE;
    }else if (atom == prop_button_mapping)
    {
        INT8 *values;
        if (val->size > MAXBUTTONS || val->format != 8 || val->type != XA_INTEGER)
            return FALSE;
        values = (INT8*)val->data;
        for (i =0; i<val->size; i++) {
            priv->button[i].mapping = values[i];
            DBG(1, ErrorF("Mapping of button %d set to %d\n",i, priv->button[i].mapping));
        }
    }else if (atom == prop_button_buttonnumber)
    {
        /* FIXME */
        return FALSE;
    }else if (atom == prop_button_amplify)
    {
        /* FIXME */
        return FALSE;
    }else if (atom == prop_button_keys)
    {
        /* FIXME */
        return FALSE;
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
    INT32 axes_values32[MAXAXES];
    INT8  axes_values8[MAXAXES];
    INT8  button_values8[MAXBUTTONS];
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
    XIConfigureDeviceProperty(pJstk, prop_keys_enabled, FALSE, FALSE, FALSE, 2, values);


    /* priv->axis[].deadzone */
    for (i=0;i<MAXAXES;i++)
        axes_values32[i] = priv->axis[i].deadzone;
    prop_axis_deadzone = MakeAtom(JSTK_PROP_AXIS_DEADZONE, strlen(JSTK_PROP_AXIS_DEADZONE), TRUE);
    XIChangeDeviceProperty(pJstk, prop_axis_deadzone, XA_INTEGER, 32,
                                PropModeReplace, MAXAXES,
                                axes_values32,
                                FALSE, FALSE, FALSE);

    /* priv->axis[].type */
    for (i=0;i<MAXAXES;i++)
        axes_values8[i] = priv->axis[i].type;
    prop_axis_type = MakeAtom(JSTK_PROP_AXIS_TYPE, strlen(JSTK_PROP_AXIS_TYPE), TRUE);
    XIChangeDeviceProperty(pJstk, prop_axis_type, XA_INTEGER, 8,
                                PropModeReplace, MAXAXES,
                                axes_values8,
                                FALSE, FALSE, FALSE);
    values[0] = TYPE_NONE;
    values[1] = TYPE_BYVALUE;
    values[2] = TYPE_ACCELERATED;
    values[3] = TYPE_ABSOLUTE;
    XIConfigureDeviceProperty(pJstk, prop_axis_type, FALSE, FALSE, FALSE, 4, values);

    /* priv->axis[].mapping */
    for (i=0;i<MAXAXES;i++)
        axes_values8[i] = (INT8)priv->axis[i].mapping;
    prop_axis_mapping = MakeAtom(JSTK_PROP_AXIS_MAPPING, strlen(JSTK_PROP_AXIS_MAPPING), TRUE);
    XIChangeDeviceProperty(pJstk, prop_axis_mapping, XA_INTEGER, 8,
                                PropModeReplace, MAXBUTTONS,
                                axes_values8,
                                FALSE, FALSE, FALSE);
    values[0] = MAPPING_NONE;
    values[1] = MAPPING_X;
    values[2] = MAPPING_Y;
    values[3] = MAPPING_ZX;
    values[4] = MAPPING_ZY;
    values[5] = MAPPING_KEY;
    XIConfigureDeviceProperty(pJstk, prop_axis_mapping, FALSE, FALSE, FALSE, 6, values);


    /* priv->axis[].amplify */
    /* FIXME: prop_axis_amplify as float[] */

    /* priv->axis[].keys_low */
    /* FIXME: prop_axis_keys_low */

    /* priv->axis[].keys_high */
    /* FIXME: prop_axis_keys_high */




    /* priv->button[].mapping */
    for (i=0;i<MAXBUTTONS;i++)
        button_values8[i] = (INT8)priv->button[i].mapping;
    prop_button_mapping = MakeAtom(JSTK_PROP_BUTTON_MAPPING, strlen(JSTK_PROP_BUTTON_MAPPING), TRUE);
    XIChangeDeviceProperty(pJstk, prop_button_mapping, XA_INTEGER, 8,
                                PropModeReplace, MAXBUTTONS,
                                button_values8,
                                FALSE, FALSE, FALSE);
    values[0] = MAPPING_NONE;
    values[1] = MAPPING_X;
    values[2] = MAPPING_Y;
    values[3] = MAPPING_ZX;
    values[4] = MAPPING_ZY;
    values[5] = MAPPING_BUTTON;
    values[6] = MAPPING_KEY;
    values[7] = MAPPING_SPEED_MULTIPLY;
    values[8] = MAPPING_DISABLE;
    values[9] = MAPPING_DISABLE_MOUSE;
    values[10] = MAPPING_DISABLE_KEYS;
    XIConfigureDeviceProperty(pJstk, prop_button_mapping, FALSE, FALSE, FALSE, 11, values);


    /* priv->button[].buttonnumber */
    for (i=0;i<MAXAXES;i++)
        button_values8[i] = (INT8)priv->button[i].buttonnumber;
    prop_button_buttonnumber = MakeAtom(JSTK_PROP_BUTTON_BUTTONNUMBER, strlen(JSTK_PROP_BUTTON_BUTTONNUMBER), TRUE);
    XIChangeDeviceProperty(pJstk, prop_button_buttonnumber, XA_INTEGER, 8,
                                PropModeReplace, MAXBUTTONS,
                                button_values8,
                                FALSE, FALSE, FALSE);

    /* priv->button[].amplify */
    /* FIXME: prop_button_amplify as float[] */

    /* priv->button[].keys */
    /* FIXME: prop_button_keys */

    return TRUE;
}

#endif
