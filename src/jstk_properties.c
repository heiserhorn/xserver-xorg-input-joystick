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
#include "joystick-properties.h" /* definitions of properties */


static Atom prop_debuglevel = 0;
static Atom prop_numbuttons = 0;
static Atom prop_numaxes = 0;
static Atom prop_mouse_enabled = 0;
static Atom prop_keys_enabled  = 0;
static Atom prop_axis_deadzone = 0;
static Atom prop_axis_type = 0;
static Atom prop_axis_mapping = 0;
static Atom prop_axis_amplify = 0;
static Atom prop_axis_keys_low = 0;
static Atom prop_axis_keys_high = 0;
static Atom prop_button_mapping = 0;
static Atom prop_button_buttonnumber = 0;
static Atom prop_button_amplify = 0;
static Atom prop_button_keys = 0;



static int
jstkSetProperty(DeviceIntPtr pJstk, Atom atom, XIPropertyValuePtr val,
                BOOL checkonly)
{
    InputInfoPtr  pInfo = pJstk->public.devicePrivate;
    JoystickDevPtr priv = pInfo->private;
    int i;

    if (atom == prop_debuglevel)
    {
#if DEBUG
        if (val->size != 1 || val->format != 8 || val->type != XA_INTEGER)
            return BadMatch;
        if (!checkonly)
        {
            debug_level = *((INT8*)val->data);
            ErrorF("JOYSTICK: DebugLevel set to %d\n", debug_level);
        }
#endif
    }else if (atom == prop_numbuttons)
    {
        if (val->size != 1 || val->format != 8 || val->type != XA_INTEGER)
            return BadMatch;
        if ((*((INT8*)val->data)) != priv->num_buttons)
            return BadMatch;
        return Success;
    }else if (atom == prop_numaxes)
    {
        if (val->size != 1 || val->format != 8 || val->type != XA_INTEGER)
            return BadMatch;
        if ((*((INT8*)val->data)) != priv->num_axes)
            return BadMatch;
        return Success;
    }else if (atom == prop_mouse_enabled)
    {
        if (val->size != 1 || val->format != 8 || val->type != XA_INTEGER)
            return BadMatch;
        if (!checkonly)
        {
            priv->mouse_enabled = (*((INT8*)val->data)) != 0;
            DBG(1, ErrorF("mouse_enabled set to %d\n", priv->mouse_enabled));
        }
    }else if (atom == prop_keys_enabled)
    {
        if (val->size != 1 || val->format != 8 || val->type != XA_INTEGER)
            return BadMatch;
        if (!checkonly)
        {
            priv->keys_enabled = (*((INT8*)val->data)) != 0;
            DBG(1, ErrorF("keys_enabled set to %d\n", priv->keys_enabled));
        }
    }else if (atom == prop_axis_deadzone)
    {
        INT32 *values;
        if (val->size != priv->num_axes || val->format != 32 || val->type != XA_INTEGER)
            return BadMatch;
        values = (INT32*)val->data;
        for (i =0; i<val->size; i++) /* Fail, if one value is out of range */ 
            if (values[i] > 30000 || values[i] < -30000)
                return BadValue;
        if (!checkonly)
        {
            for (i =0; i<val->size; i++) {
                priv->axis[i].deadzone = (values[i]<0)?(-values[i]):(values[i]);
                DBG(1, ErrorF("Deadzone of axis %d set to %d\n",i, priv->axis[i].deadzone));
            }
        }
    }else if (atom == prop_axis_type)
    {
        INT8 *values;
        if (val->size != priv->num_axes || val->format != 8 || val->type != XA_INTEGER)
            return BadMatch;
        if (!checkonly)
        {
            values = (INT8*)val->data;
            for (i =0; i<val->size; i++) {
                priv->axis[i].type = values[i];
                DBG(1, ErrorF("Type of axis %d set to %d\n",i, priv->axis[i].type));
            }
        }
    }else if (atom == prop_axis_mapping)
    {
        INT8 *values;
        if (val->size != priv->num_axes || val->format != 8 || val->type != XA_INTEGER)
            return BadMatch;
        if (!checkonly)
        {
            values = (INT8*)val->data;
            for (i =0; i<val->size; i++) {
                priv->axis[i].mapping = values[i];
                DBG(1, ErrorF("Mapping of axis %d set to %d\n",i, priv->axis[i].mapping));
            }
        }
    }else if (atom == prop_axis_amplify)
    {
        /* FIXME */
        return BadValue;
    }else if (atom == prop_axis_keys_low)
    {
        /* FIXME */
        return BadValue;
    }else if (atom == prop_axis_keys_high)
    {
        /* FIXME */
        return BadValue;
    }else if (atom == prop_button_mapping)
    {
        INT8 *values;
        if (val->size != priv->num_buttons || val->format != 8 || val->type != XA_INTEGER)
            return BadMatch;
        if (!checkonly)
        {
            values = (INT8*)val->data;
            for (i =0; i<val->size; i++) {
                priv->button[i].mapping = values[i];
                DBG(1, ErrorF("Mapping of button %d set to %d\n",i, priv->button[i].mapping));
            }
        }
    }else if (atom == prop_button_buttonnumber)
    {
        CARD8 *values;
        if (val->size != priv->num_buttons || val->format != 8 || val->type != XA_INTEGER)
            return BadMatch;
        values = (INT8*)val->data;
        for (i = 0; i<val->size; i++) {
            if (values[i] > BUTTONMAP_SIZE) {
               DBG(1, ErrorF("Button number out of range (0..%d): %d\n",
                   BUTTONMAP_SIZE, values[i]));
               return BadValue;
            }
        }
        if (!checkonly)
        {
            for (i = 0; i<val->size; i++) {
                priv->button[i].buttonnumber =
                    values[i];
                DBG(1, ErrorF("Button number of button %d set to %d\n",
                            i,
                            priv->button[i].buttonnumber));
            }
        }
        return Success;
    }else if (atom == prop_button_amplify)
    {
        /* FIXME */
        return BadValue;
    }else if (atom == prop_button_keys)
    {
        /* FIXME */
        return BadValue;
    }

    /* property not handled, report success */
    return Success;
}

Bool
jstkInitProperties(DeviceIntPtr pJstk, JoystickDevPtr priv)
{
    INT32 axes_values32[MAXAXES];
    INT8  axes_values8[MAXAXES];
    INT8  button_values8[MAXBUTTONS];
    int i;

    XIRegisterPropertyHandler(pJstk, jstkSetProperty, NULL, NULL);

#ifdef DEBUG
    /* Debug Level */
    prop_debuglevel = MakeAtom(JSTK_PROP_DEBUGLEVEL, strlen(JSTK_PROP_DEBUGLEVEL), TRUE);
    XIChangeDeviceProperty(pJstk, prop_debuglevel, XA_INTEGER, 8,
                                PropModeReplace, 1,
                                &debug_level,
                                FALSE);
    XISetDevicePropertyDeletable(pJstk, prop_debuglevel, FALSE);
#endif

    /* priv->num_buttons */
    prop_numbuttons = MakeAtom(JSTK_PROP_NUMBUTTONS, strlen(JSTK_PROP_NUMBUTTONS), TRUE);
    XIChangeDeviceProperty(pJstk, prop_numbuttons, XA_INTEGER, 8,
                                PropModeReplace, 1,
                                &priv->num_buttons,
                                FALSE);
    XISetDevicePropertyDeletable(pJstk, prop_numbuttons, FALSE);


    /* priv->num_axes */
    prop_numaxes = MakeAtom(JSTK_PROP_NUMAXES, strlen(JSTK_PROP_NUMAXES), TRUE);
    XIChangeDeviceProperty(pJstk, prop_numaxes, XA_INTEGER, 8,
                                PropModeReplace, 1,
                                &priv->num_axes,
                                FALSE);
    XISetDevicePropertyDeletable(pJstk, prop_numaxes, FALSE);


    /* priv->mouse_enabled */
    prop_mouse_enabled = MakeAtom(JSTK_PROP_MOUSE_ENABLED, strlen(JSTK_PROP_MOUSE_ENABLED), TRUE);
    XIChangeDeviceProperty(pJstk, prop_mouse_enabled, XA_INTEGER, 8,
                                PropModeReplace, 1,
                                &priv->mouse_enabled,
                                FALSE);
    XISetDevicePropertyDeletable(pJstk, prop_mouse_enabled, FALSE);

    /* priv->keys_enabled */
    prop_keys_enabled = MakeAtom(JSTK_PROP_KEYS_ENABLED, strlen(JSTK_PROP_KEYS_ENABLED), TRUE);
    XIChangeDeviceProperty(pJstk, prop_keys_enabled, XA_INTEGER, 8,
                                PropModeReplace, 1,
                                &priv->keys_enabled,
                                FALSE);
    XISetDevicePropertyDeletable(pJstk, prop_keys_enabled, FALSE);

    /* priv->axis[].deadzone */
    for (i=0;i<priv->num_axes;i++)
        axes_values32[i] = priv->axis[i].deadzone;
    prop_axis_deadzone = MakeAtom(JSTK_PROP_AXIS_DEADZONE, strlen(JSTK_PROP_AXIS_DEADZONE), TRUE);
    XIChangeDeviceProperty(pJstk, prop_axis_deadzone, XA_INTEGER, 32,
                                PropModeReplace, priv->num_axes,
                                axes_values32,
                                FALSE);
    XISetDevicePropertyDeletable(pJstk, prop_axis_deadzone, FALSE);

    /* priv->axis[].type */
    for (i=0;i<priv->num_axes;i++)
        axes_values8[i] = priv->axis[i].type;
    prop_axis_type = MakeAtom(JSTK_PROP_AXIS_TYPE, strlen(JSTK_PROP_AXIS_TYPE), TRUE);
    XIChangeDeviceProperty(pJstk, prop_axis_type, XA_INTEGER, 8,
                                PropModeReplace, priv->num_axes,
                                axes_values8,
                                FALSE);
    XISetDevicePropertyDeletable(pJstk, prop_axis_type, FALSE);

    /* priv->axis[].mapping */
    for (i=0;i<priv->num_axes;i++)
        axes_values8[i] = (INT8)priv->axis[i].mapping;
    prop_axis_mapping = MakeAtom(JSTK_PROP_AXIS_MAPPING, strlen(JSTK_PROP_AXIS_MAPPING), TRUE);
    XIChangeDeviceProperty(pJstk, prop_axis_mapping, XA_INTEGER, 8,
                                PropModeReplace, priv->num_axes,
                                axes_values8,
                                FALSE);
    XISetDevicePropertyDeletable(pJstk, prop_axis_mapping, FALSE);

    /* priv->axis[].amplify */
    /* FIXME: prop_axis_amplify as float[] */

    /* priv->axis[].keys_low */
    /* FIXME: prop_axis_keys_low */

    /* priv->axis[].keys_high */
    /* FIXME: prop_axis_keys_high */




    /* priv->button[].mapping */
    for (i=0;i<priv->num_buttons;i++)
        button_values8[i] = (INT8)priv->button[i].mapping;
    prop_button_mapping = MakeAtom(JSTK_PROP_BUTTON_MAPPING, strlen(JSTK_PROP_BUTTON_MAPPING), TRUE);
    XIChangeDeviceProperty(pJstk, prop_button_mapping, XA_INTEGER, 8,
                                PropModeReplace, priv->num_buttons,
                                button_values8,
                                FALSE);
    XISetDevicePropertyDeletable(pJstk, prop_button_mapping, FALSE);

    /* priv->button[].buttonnumber */
    for (i=0;i<priv->num_buttons;i++) {
        int index = priv->button[i].buttonnumber;
        if (index>=0 && index<=MAXBUTTONS)
            button_values8[i] = (CARD8)index;
        else button_values8[i] = 0;
    }
    prop_button_buttonnumber = MakeAtom(JSTK_PROP_BUTTON_BUTTONNUMBER, strlen(JSTK_PROP_BUTTON_BUTTONNUMBER), TRUE);
    XIChangeDeviceProperty(pJstk, prop_button_buttonnumber, XA_INTEGER, 8,
                                PropModeReplace, priv->num_buttons,
                                button_values8,
                                FALSE);
    XISetDevicePropertyDeletable(pJstk, prop_button_buttonnumber, FALSE);

    /* priv->button[].amplify */
    /* FIXME: prop_button_amplify as float[] */

    /* priv->button[].keys */
    /* FIXME: prop_button_keys */

    return TRUE;
}

#endif