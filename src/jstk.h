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

#ifndef __JSTK_H_INCLUDED__
#define __JSTK_H_INCLUDED__




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

/**
 * DEBUG Makros
 **/

#if DEBUG
extern int      debug_level;
#define DBG(lvl, f) {if ((lvl) <= debug_level) f;}
#else
#define DBG(lvl, f)
#endif



#define MAXBUTTONS 32
#define MAXAXES 32
#define MAXKEYSPERBUTTON 4

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

enum JOYSTICKEVENT {
  EVENT_NONE=0,
  EVENT_BUTTON,
  EVENT_AXIS
};



typedef struct
{
  int          fd;             /* Actual file descriptor */
  char         *device;        /* Name of the device */

  OsTimerPtr   timer;
  Bool         timerrunning;
  float        x,y,zx,zy;      /* Pending subpixel movements */

  Bool         mouse_enabled, keys_enabled;
  float        amplify;        /* Global amplifier of axis movement */

  struct AXIS
  {
    int   value;
    int   deadzone;
    float temp,amplify;
    enum JOYSTICKTYPE type;
    enum JOYSTICKMAPPING mapping;
  }axis[MAXAXES];                   /* Configuration per axis */

  struct BUTTON
  {
    char pressed;
    int value;
    unsigned int keys[MAXKEYSPERBUTTON];
    float temp;
    enum JOYSTICKMAPPING mapping;
  }button[MAXBUTTONS];                 /* Configuration per button */
  unsigned char axes, buttons;         /* Number of axes and buttons */
} JoystickDevRec, *JoystickDevPtr;


#endif
