/*
 * Copyright 2007      by Sascha Hlusiak. <saschahlusiak@freedesktop.org>
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


#include <xf86.h>
#include <X11/extensions/XKB.h>
#include <X11/extensions/XKBstr.h>
#include <X11/extensions/XKBsrv.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include "jstk.h"
#include "jstk_key.h"

#define MIN_KEYCODE 8


/**
 * Xorg should allow us to have a separate keymap
 * If a keyboard is defined, there already is a keymap and we have to use it
 * If not, we have to provide our own keymap
 *
 * Since we don't know we do NOTHING. :-( So we assume, you have a keyboard attached
 **/


int
jstkInitKeys(DeviceIntPtr pJstk, JoystickDevPtr priv)
{
/*    KeySymsRec keySyms;
    CARD8 modMap[MAP_LENGTH];
    KeySym sym;
    int i, j;
    XkbComponentNamesRec xkbnames;

    static struct { KeySym keysym; CARD8 mask; } modifiers[] = {
        { XK_Shift_L,           ShiftMask },
        { XK_Shift_R,           ShiftMask },
        { XK_Control_L,         ControlMask },
        { XK_Control_R,         ControlMask },
        { XK_Caps_Lock,         LockMask }
        { XK_Alt_L,             AltMask },
        { XK_Alt_R,             AltMask },
        { XK_Num_Lock,          NumLockMask },
        { XK_Scroll_Lock,       ScrollLockMask },
        { XK_Mode_switch,       AltLangMask }
    };

    priv->keymap.size = 20;
    for (i = 0; i < 20; i++) 
        priv->keymap.map[i] = XK_at;

    memset(modMap, 0, sizeof modMap);

    for (i = 0; i < priv->keymap.size; i++) {
        sym = priv->keymap.map[i];
        for (j = 0; j < sizeof(modifiers)/sizeof(modifiers[0]); j++) {
            if (modifiers[j].keysym == sym)
                modMap[i + MIN_KEYCODE] = modifiers[j].mask;
        }
    }

    keySyms.map        = priv->keymap.map;
    keySyms.mapWidth   = 1;
    keySyms.minKeyCode = MIN_KEYCODE;
    keySyms.maxKeyCode = MIN_KEYCODE + priv->keymap.size - 1;

    XkbSetRulesDflts(__XKBDEFRULES__, "pc105", "de", "nodeadkeys", NULL);

    XkbInitKeyboardDeviceStruct (pJstk, &xkbnames, &keySyms, modMap,
            NULL, NULL);
*/

    return Success;
}









