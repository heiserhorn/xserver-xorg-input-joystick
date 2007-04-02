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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <linux/joystick.h>

#include <xf86.h>
#include <xf86_OSproc.h>

#include "jstk.h"
#include "linux_jstk.h"


/***********************************************************************
 *
 * jstkOpenDevice --
 *
 * Open and initialize a joystick device
 * Returns the filedescriptor, or -1 in case of error
 *
 ***********************************************************************
 */

int
jstkOpenDevice(JoystickDevPtr joystick,int init)
{
  char joy_name[128];
  int driver_version;

  if ((joystick->fd = open(joystick->device, O_RDWR | O_NDELAY, 0)) < 0) {
    xf86Msg(X_ERROR, "Cannot open joystick '%s' (%s)\n", joystick->device,
            strerror(errno));
    return -1;
  }

  if (ioctl(joystick->fd, JSIOCGVERSION, &driver_version) == -1) {
    xf86Msg(X_ERROR, "Joystick: ioctl on '%s' failed: %s\n", joystick->device,
            strerror(errno));
    return -1;
  }
  if ((driver_version >> 16) < 1) {
    xf86Msg(X_WARNING, "Joystick: Driver version is only %d.%d.%d\n",
            driver_version >> 16,
            (driver_version >> 8) & 0xff,
            driver_version & 0xff);
  }

  if (ioctl(joystick->fd, JSIOCGAXES, &joystick->axes) == -1) {
    xf86Msg(X_ERROR, "Joystick: ioctl on '%s' failed: %s\n", joystick->device,
            strerror(errno));
    return -1;
  }
  if (ioctl(joystick->fd, JSIOCGBUTTONS, &joystick->buttons) == -1) {
    xf86Msg(X_ERROR, "Joystick: ioctl on '%s' failed: %s\n", joystick->device,
            strerror(errno));
    return -1;
  }

  /* Only show these information once, not every time the device is opened */
  if (init != 0) {
    if (ioctl(joystick->fd, JSIOCGNAME(128), joy_name) == -1) {
      xf86Msg(X_ERROR, "Joystick: ioctl on '%s' failed: %s\n", 
              joystick->device, strerror(errno));
      return -1;
    }

    xf86Msg(X_INFO, "Joystick: %s. %d buttons, %d axes\n", 
      joy_name, joystick->axes, joystick->buttons);
  }

  return joystick->fd;
}


/***********************************************************************
 *
 * jstkCloseDevice --
 *
 * close the handle.
 *
 ***********************************************************************
 */

void
jstkCloseDevice(JoystickDevPtr joystick)
{
  if ((joystick->fd >= 0)) {
    xf86CloseSerial(joystick->fd);
    joystick->fd = -1;
  }
}


/***********************************************************************
 *
 * jstkReadData --
 *
 * Reads data from fd and stores it in the JoystickDevRec struct
 * fills in the type of event and the number of the button/axis
 * return 1 if success, 0 otherwise. Success does not neccessarily
 * mean that there is a new event waiting.
 *
 ***********************************************************************
 */

int
jstkReadData(JoystickDevPtr joystick,
             enum JOYSTICKEVENT *event,
             int *number)
{
  struct js_event js;
  if (event != NULL) *event = EVENT_NONE;
  if (xf86ReadSerial(joystick->fd,
                     &js,
                     sizeof(struct js_event)
      ) != sizeof(struct js_event))
    return 0;

  switch(js.type & ~JS_EVENT_INIT) {
    case JS_EVENT_BUTTON:
      if (js.number < MAXBUTTONS)
      {
        joystick->button[js.number].pressed = js.value;
        if (event != NULL) *event = EVENT_BUTTON;
        if (number != NULL) *number = js.number;
      }
      break;
    case JS_EVENT_AXIS:
      if (js.number < MAXAXES) {
        if (abs(js.value) < joystick->axis[js.number].deadzone) {
          /* We only want one event when in deadzone */
          if (joystick->axis[js.number].value != 0) {
            joystick->axis[js.number].value = 0;
            if (event != NULL) *event = EVENT_AXIS;
            if (number != NULL) *number = js.number;
          }
        }else{
          joystick->axis[js.number].value = js.value;
          if (event != NULL) *event = EVENT_AXIS;
          if (number != NULL) *number = js.number;
        }
      }
      break;
  }

  /* If it is an JS_EVENT_INIT just save the state, but don't report
     as an event */
  if ((js.type & JS_EVENT_INIT) == JS_EVENT_INIT) {
    if (event != NULL) *event = EVENT_NONE;
  }
  return 1;
}
