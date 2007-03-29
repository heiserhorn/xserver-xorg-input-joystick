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

// #include <xf86.h>


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <xf86Xinput.h>
#include <xf86_OSproc.h>
#include <math.h>
#include "jstk.h"
#include "jstk_axis.h"



static CARD32
jstkAxisTimer(OsTimerPtr        timer,
              CARD32            atime,
              pointer           arg)
{
#define NEXTTIMER 15
  DeviceIntPtr          device = (DeviceIntPtr)arg;
  JoystickDevPtr        priv = (JoystickDevPtr) XI_PRIVATE(device);

  int sigstate, i;
  int nexttimer;
  nexttimer = 0;

  sigstate = xf86BlockSIGIO ();

  for (i=0; i<MAXAXES; i++) if ((priv->axis[i].value != 0)&&
                                (priv->axis[i].type != TYPE_NONE)) {
    float p1 = 0.0;
    float p2 = 0.0;
    float scale;
    struct AXIS *axis;
    axis = &priv->axis[i];

    nexttimer = NEXTTIMER;

    if (priv->axis[i].type == TYPE_BYVALUE) {
      /* Calculate scale value, so we still get a range from 0 to 32768 */
      scale = (32768.0/(float)(32768 - axis->deadzone));

      p1 = ((pow((abs((float)axis->value)-(float)axis->deadzone)*
             scale/1700.0, 3.5))+100.0)*
            ((float)NEXTTIMER/40000.0);
      p2 = ((pow((abs((float)axis->value)-(float)axis->deadzone)*
             scale/1000.0, 2.5))+200.0)*
            ((float)NEXTTIMER/200000.0);


    } else if (axis->type == TYPE_ACCELERATED) {
      if (axis->temp < 120.0) axis->temp *= 1.15;

      p1 = (axis->temp - 0.1) * (float)NEXTTIMER / 180.0;
      p2 = p1 / 8.0;
    }
    if (axis->value < 0) {
      p1 *= -1.0;
      p2 *= -1.0;
    }
    p1 *= axis->amplify * priv->amplify;
    p2 *= axis->amplify * priv->amplify;

    switch (axis->mapping) {
      case MAPPING_X:
        priv->x += p1;
        break;
      case MAPPING_Y:
        priv->y += p1;
        break;
      case MAPPING_ZX:
        priv->zx += p2;
        break;
      case MAPPING_ZY:
        priv->zy += p2;
        break;
      default:
        break;
    }
  }

  for (i=0; i<MAXBUTTONS; i++) if (priv->button[i].pressed == 1) {
    float p1;
    float p2;

    if (priv->button[i].temp < 120.0) priv->button[i].temp *= 1.15;
    p1 = (priv->button[i].temp - 0.1) * (float)NEXTTIMER / 180.0 * ((float)priv->button[i].value)/1000.0;
    p1 *= priv->amplify;
    p2 = p1 / 8.0;

    switch (priv->button[i].mapping) {
      case MAPPING_X:
        priv->x += p1;
        nexttimer = NEXTTIMER;
        break;
      case MAPPING_Y:
        priv->y += p1;
        nexttimer = NEXTTIMER;
        break;
      case MAPPING_ZX:
        priv->zx += p2;
        nexttimer = NEXTTIMER;
        break;
      case MAPPING_ZY:
        priv->zy += p2;
        nexttimer = NEXTTIMER;
        break;
      default:
        break;
    }
  }


  if (((int)priv->x != 0)||((int)priv->y != 0))
    xf86PostMotionEvent(device, 0, 0, 2, (int)priv->x, (int)priv->y);
  priv->x = priv->x - (int)priv->x;
  priv->y = priv->y - (int)priv->y;

  while (priv->zy >= 1.0) {
    xf86PostButtonEvent(device, 0, 5, 1, 0, 0);
    xf86PostButtonEvent(device, 0, 5, 0, 0, 0);
    priv->zy-=1.0;
  }
  while (priv->zy <= -1.0) {
    xf86PostButtonEvent(device, 0, 4, 1, 0, 0);
    xf86PostButtonEvent(device, 0, 4, 0, 0, 0);
    priv->zy+=1.0;
  }

  while (priv->zx >= 1.0) {
    xf86PostButtonEvent(device, 0, 7, 1, 0, 0);
    xf86PostButtonEvent(device, 0, 7, 0, 0, 0);
    priv->zx-=1.0;
  }
  while (priv->zx <= -1.0) {
    xf86PostButtonEvent(device, 0, 6, 1, 0, 0);
    xf86PostButtonEvent(device, 0, 6, 0, 0, 0);
    priv->zx+=1.0;
  }

  if (priv->mouse_enabled == FALSE) nexttimer = 0;
  if (nexttimer == 0) {
    priv->timerrunning = FALSE;
    priv->x  = 0.0;
    priv->y  = 0.0;
    priv->zx = 0.0;
    priv->zy = 0.0;
    DBG(2, ErrorF("Stopping Axis Timer\n"));
  }
  xf86UnblockSIGIO (sigstate);
  return nexttimer;
}

void
jstkStartAxisTimer(LocalDevicePtr device, int number) {
  JoystickDevPtr priv = device->private;

  if (priv->timerrunning) return;
  priv->timerrunning = TRUE;

  int pixel = 1;
  if (priv->axis[number].value < 0) pixel = -1;
  switch (priv->axis[number].mapping) {
    case MAPPING_X:
      priv->x += pixel;
      break;
    case MAPPING_Y:
      priv->y += pixel;
      break;
    case MAPPING_ZX:
      priv->zx += pixel;
      break;
    case MAPPING_ZY:
      priv->zy += pixel;
      break;
    default:
      break;
  }

  DBG(2, ErrorF("Starting Axis Timer (triggered by axis %d)\n", number));
  priv->timer = TimerSet(
    priv->timer, 
    0,         /* Relative */
    5,
    jstkAxisTimer,
    device->dev);
}

void
jstkStartButtonAxisTimer(LocalDevicePtr device, int number) {
  JoystickDevPtr priv = device->private;

  if (priv->timerrunning) return;
  priv->timerrunning = TRUE;

  int pixel = 1;
  if (priv->button[number].value < 0) pixel = -1;
  switch (priv->button[number].mapping) {
    case MAPPING_X:
      priv->x += pixel;
      break;
    case MAPPING_Y:
      priv->y += pixel;
      break;
    case MAPPING_ZX:
      priv->zx += pixel;
      break;
    case MAPPING_ZY:
      priv->zy += pixel;
      break;
    default:
      break;
  }

  DBG(2, ErrorF("Starting Axis Timer (triggered by button %d)\n", number));
  priv->timer = TimerSet(
    priv->timer, 
    0,         /* Relative */
    5,
    jstkAxisTimer,
    device->dev);
}


void
jstkHandleAbsoluteAxis(LocalDevicePtr device, int number) {
  JoystickDevPtr priv = device->private;
  int x,y,i;
  x = screenInfo.screens[0]->width / 2;
  y = screenInfo.screens[0]->height / 2;

  for (i=0; i<MAXAXES; i++) 
    if ((priv->axis[i].type == TYPE_ABSOLUTE)&&(priv->axis[i].value != 0))
  {
    float rel;
    rel = (priv->axis[i].value>0)?
           (priv->axis[i].value - priv->axis[i].deadzone):
           (priv->axis[i].value + priv->axis[i].deadzone);
    rel = (rel)/(2.0*(float)(32768 - priv->axis[i].deadzone)) + 0.5;

    if (priv->axis[i].mapping == MAPPING_X)
      x = rel * screenInfo.screens[0]->width;
    if (priv->axis[i].mapping == MAPPING_Y)
      y = rel * screenInfo.screens[0]->height;
  }
  DBG(3, ErrorF("Setting mouse to %dx%d\n",x,y));
  xf86PostMotionEvent(device->dev, 1, 0, 2, x, y);
}

