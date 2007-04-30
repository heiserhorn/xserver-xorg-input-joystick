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


typedef enum _JOYSTICKTYPE{
    TYPE_NONE,        /* Axis value is not relevant */
    TYPE_BYVALUE,     /* Speed of cursor is relative to amplitude */
    TYPE_ACCELERATED, /* Speed is accelerated */
    TYPE_ABSOLUTE     /* The amplitude defines the cursor position */
} JOYSTICKTYPE;

typedef enum _JOYSTICKMAPPING{
    MAPPING_NONE,           /* Nothing */
    MAPPING_X,              /* X-Axis */
    MAPPING_Y,              /* Y-Axis */
    MAPPING_ZX,             /* Horizontal scrolling */
    MAPPING_ZY,             /* Vertical scrolling */
    MAPPING_BUTTON,         /* Mouse button */
    MAPPING_KEY,            /* Keyboard event */
    MAPPING_SPEED_MULTIPLY, /* Will amplify all axis movement */
    MAPPING_DISABLE,        /* Disable mouse and key events */
    MAPPING_DISABLE_MOUSE,  /* Disable only mouse events */
    MAPPING_DISABLE_KEYS    /* Disable only key events */
} JOYSTICKMAPPING;


typedef struct _AXIS {
    JOYSTICKTYPE    type;
    JOYSTICKMAPPING mapping;
    int             value;
    int             deadzone;
    float           currentspeed;
    float           previousposition;
    float           amplify;
} AXIS;

#define MAXKEYSPERBUTTON 4

typedef struct _BUTTON {
   JOYSTICKMAPPING mapping;
   char pressed;
   int buttonnumber;    /* MAPPING_BUTTON */
   float amplify;       /* MAPPING_X/Y/ZX/ZY, 
                           MAPPING_SPEED_MULTIPLY */
   float currentspeed;  /* MAPPING_X/Y/ZX/ZY */
   unsigned int keys[MAXKEYSPERBUTTON]; /* MAPPING_KEY */
} BUTTON;

#define MAXBUTTONS 32
#define MAXAXES 32

typedef struct _JoystickDevRec {
    int          fd;          /* Actual file descriptor */
    void         *devicedata; /* Extra platform device dependend data */
    char         *device;     /* Name of the device */

    OsTimerPtr   timer;       /* Timer for axis movement */
    Bool         timerrunning;
    float        x,y,zx,zy;   /* Pending subpixel movements */

    Bool         mouse_enabled, keys_enabled;
    float        amplify;     /* Global amplifier of axis movement */

    AXIS axis[MAXAXES];           /* Configuration per axis */
    BUTTON button[MAXBUTTONS];    /* Configuration per button */
} JoystickDevRec, *JoystickDevPtr;

#endif
