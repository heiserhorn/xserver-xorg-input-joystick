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
static Atom prop_debuglevel     = 0;



static Bool
jstkSetProperty(DeviceIntPtr pJstk, Atom atom, XIPropertyValuePtr val)
{
    InputInfoPtr  pInfo = pJstk->public.devicePrivate;
    JoystickDevPtr priv = pInfo->private;

    if (atom == prop_debuglevel)
    {
        debug_level = *((INT8*)val->data);

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
    int          rc     = TRUE;

    XIRegisterPropertyHandler(pJstk, jstkSetProperty, jstkGetProperty);

    /* Debug Level */
    prop_debuglevel = MakeAtom(JSTK_PROP_DEBUGLEVEL, strlen(JSTK_PROP_DEBUGLEVEL), TRUE);
    rc = XIChangeDeviceProperty(pJstk, prop_debuglevel, XA_INTEGER, 8,
                                PropModeReplace, 1,
                                &debug_level,
                                FALSE, FALSE, FALSE);
    if (rc != Success)
        return;


    return TRUE;
}

#endif
