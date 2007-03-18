/*
 * Copyright 2007      by Sascha Hlusiak. <saschahlusiak@freedesktop.org>     
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

#ifndef _XF86JSTK_H_INCLUDED_
#define _XF86JSTK_H_INCLUDED_


#include <xf86.h>


#define MAXBUTTONS 32
#define MAXAXES MAXBUTTONS

enum JOYSTICKTYPE {
  TYPE_NONE,
  TYPE_BYVALUE,     /* Speed of cursor is relative to amplitude */
  TYPE_ACCELERATED, /* Speed is accelerated */
  TYPE_ABSOLUTE     /* The amplitude defines the cursor position */
};

enum JOYSTICKMAPPING {
  MAPPING_NONE=0,
  MAPPING_X,
  MAPPING_Y,
  MAPPING_ZX,
  MAPPING_ZY,
  MAPPING_BUTTON,
  MAPPING_KEY,
  MAPPING_SPEED_MULTIPLY,
  MAPPING_DISABLE,
  MAPPING_DISABLE_MOUSE,
  MAPPING_DISABLE_KEYS
};



typedef struct
{
  int          fd;             /* Actual file descriptor */
  OsTimerPtr   timer;
  int          timeout;
  char         *device;        /* Name of the device */


  struct AXIS
  {
    int value;
    int deadzone;
    enum JOYSTICKTYPE type;
    enum JOYSTICKMAPPING mapping;
  }axis[32];                   /* Configuration per axis */

  struct BUTTON
  {
    char pressed;
    int value;
    enum JOYSTICKMAPPING mapping;
  }button[32];                 /* Configuration per button */
  int axes, buttons;           /* Number of axes and buttons */
} JoystickDevRec, *JoystickDevPtr;


#endif
