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

/**
 * This provides the backend for Linux evdev devices.
 * Devices are usually /dev/input/event?
 **/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <linux/joystick.h>

#include <xf86.h>
#include <xf86_OSproc.h>

#include "jstk.h"
#include "evdev_jstk.h"


/***********************************************************************
 *
 * jstkOpenDevice --
 *
 * Open and initialize a joystick device. The device name is
 * taken from JoystickDevPtr 
 * Returns the filedescriptor, or -1 in case of error
 *
 ***********************************************************************
 */

int
jstkOpenDevice_evdev(JoystickDevPtr joystick)
{
    if (1) {
        xf86Msg(X_ERROR, "Joystick: '%s': evdev failed: IMPLEMENTATION MISSING\n", 
                  joystick->device);
        return -1;
    }

/*    xf86Msg(X_INFO, "Joystick: %s. %d axes, %d buttons\n", 
            joy_name, axes, buttons); */

    joystick->read_proc = jstkReadData_evdev;
    joystick->close_proc = jstkCloseDevice_evdev;
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
jstkCloseDevice_evdev(JoystickDevPtr joystick)
{
/*    if ((joystick->fd >= 0)) {
        xf86CloseSerial(joystick->fd);
        joystick->fd = -1;
    }*/
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
jstkReadData_evdev(JoystickDevPtr joystick,
                   JOYSTICKEVENT *event,
                   int *number)
{
    if (event != NULL) *event = EVENT_NONE;
    return 0;

    return 1;
}
