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

#include <xf86.h>
#include "jstk.h"
#include "jstk_options.h"


void
jstkParseButtonOption(const char* org, 
                      struct BUTTON *button, 
                      const char* name) {
  char *param;
  char *tmp;
  int value;
  char p[64];
  param = xstrdup(org);

  for (tmp = param; *tmp; tmp++) *tmp = tolower(*tmp);

  if (strcmp(param, "none") == 0) {
    button->mapping = MAPPING_NONE;
  } else if (sscanf(param, "button=%d", &value) == 1) {
    button->mapping = MAPPING_BUTTON;
    button->value   = value;
  } else if (sscanf(param, "axis=%15s", p) == 1) {
    if (strcmp(p, "x") == 0)
      button->mapping = MAPPING_X;
    else if (strcmp(p, "y") == 0)
      button->mapping = MAPPING_Y;
    else if (strcmp(p, "zx") == 0)
      button->mapping = MAPPING_ZX;
    else if (strcmp(p, "zy") == 0)
      button->mapping = MAPPING_ZY;
    else {
      xf86Msg(X_WARNING, "%s: error parsing axis.\n", 
              name);
    }
  } else {
    xf86Msg(X_WARNING, "%s: error parsing button parameter.\n", 
            name);
  }
  xfree(param);
}


void
jstkParseAxisOption(const char* org, struct AXIS *axis, const char *name) {
  char *param;
  char *tmp;
  int value;
  float fvalue;
  char p[64];
  param = xstrdup(org);
  for (tmp = param; *tmp; tmp++) *tmp = tolower(*tmp);

  if ((tmp=strstr(param, "mode=")) != NULL) {
    if (sscanf(tmp, "mode=%15s", p) == 1) {
      p[15]='\0';
      if (strcmp(p, "relative") == 0)
        axis->type = TYPE_BYVALUE;
      else if (strcmp(p, "accelerated") == 0)
        axis->type = TYPE_ACCELERATED;
      else if (strcmp(p, "absolute") == 0)
        axis->type = TYPE_ABSOLUTE;
      else {
        xf86Msg(X_WARNING, "%s: \"%s\": error parsing mode.\n", 
                name, param);
      }
    }else xf86Msg(X_WARNING, "%s: \"%s\": error parsing mode.\n", 
                  name, param);
  }

  if ((tmp=strstr(param, "axis=")) != NULL) {
    if (sscanf(tmp, "axis=%15s", p) == 1) {
      p[15]='\0';
      if (strcmp(p, "x") == 0)
        axis->mapping = MAPPING_X;
      else if (strcmp(p, "y") == 0)
        axis->mapping = MAPPING_Y;
      else if (strcmp(p, "zx") == 0)
        axis->mapping = MAPPING_ZX;
      else if (strcmp(p, "zy") == 0)
        axis->mapping = MAPPING_ZY;
      else {
        xf86Msg(X_WARNING, "%s: error parsing axis.\n", 
                name);
      }
    }else xf86Msg(X_WARNING, "%s: error parsing axis.\n", 
                  name);
  }

  if ((tmp=strstr(param, "deadzone=")) != NULL ) {
    if (sscanf(tmp, "deadzone=%d", &value) == 1) {
      value = (value<0)?(-value):value;
      if (value > 30000)
        xf86Msg(X_WARNING, 
          "%s: deadzone of %d seems unreasonable. Ignored.\n", 
          name, value);
      else axis->deadzone = value;
    }else xf86Msg(X_WARNING, "%s: error parsing deadzone.\n", 
                  name);
  }
  if ((tmp=strstr(param, "amplify=")) != NULL ) {
    if (sscanf(tmp, "amplify=%f", &fvalue) == 1) {
      if ((fvalue > 10000)||(fvalue < -10000.0))
        xf86Msg(X_WARNING, 
          "%s: amplifier of %.3f seems unreasonable. Ignored.\n", 
          name, fvalue);
      else axis->amplify = fvalue;
    }else xf86Msg(X_WARNING, "%s: error parsing amplifier.\n", 
                  name);
  }

  xfree(param);
}
