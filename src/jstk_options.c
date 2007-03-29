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


/**
 * Parses strings like:
 * x, +y, -zx, 3x, 3.5zy, -8x 
 * And returns the mapping and stores the optional factor
 * In the float referenced by 'value'
 **/

static enum JOYSTICKMAPPING
jstkGetAxisMapping(float *value, const char* param, const char* name) {
  if (sscanf(param, "%f", value)==0) {
    *value = 1.0;
    if (param[0]=='-')
      *value = -1.0;
  }
  if (strstr(param, "zx") != NULL)
    return MAPPING_ZX;
  else if (strstr(param, "zy") != NULL)
    return MAPPING_ZY;
  else if (strstr(param, "x") != NULL)
    return MAPPING_X;
  else if (strstr(param, "y") != NULL)
    return MAPPING_Y;

  return MAPPING_NONE;
}

/**
 * Interpretes one ButtonMappingX option, given in 'org'
 * stores the result in *button
 * name is the name of the InputDevice
 **/

void
jstkParseButtonOption(const char* org, 
                      JoystickDevPtr priv,
                      int number,
                      const char* name) {
  char *param;
  char *tmp;
  int value;
  float fvalue;
  char p[64];
  struct BUTTON* button;

  button = &priv->button[number];
  param = xstrdup(org);

  for (tmp = param; *tmp; tmp++) *tmp = tolower(*tmp);

  if (strcmp(param, "none") == 0) {
    button->mapping = MAPPING_NONE;
  } else if (sscanf(param, "button=%d", &value) == 1) {
    button->mapping = MAPPING_BUTTON;
    button->value   = value;
  } else if (sscanf(param, "axis=%15s", p) == 1) {
    button->mapping = jstkGetAxisMapping(&fvalue, p, name);
    button->value = (int)(fvalue*1000.0);
    if (button->mapping == MAPPING_NONE)
      xf86Msg(X_WARNING, "%s: error parsing axis: %s.\n", 
              name, p);
  } else if (sscanf(param, "amplify=%f", &fvalue) == 1) {
    button->mapping = MAPPING_SPEED_MULTIPLY;
    button->value = (int)(fvalue*1000.0);
  } else if (sscanf(param, "key=%30s", p) == 1) {
    char *current,*next;
    current = p;
    button->mapping = MAPPING_KEY;
    for (value = 0; value < MAXKEYSPERBUTTON; value++) if (current != NULL) {
      next = strchr(current, ',');
      if (next) *(next++) = '\0';
      button->keys[value] = atoi(current);
      if (button->keys[value] == 0)
        xf86Msg(X_WARNING, "%s: error parsing key value: %s.\n", 
                name, current);
      current = next;
    } else button->keys[value] = 0;
  } else if (strcmp(param, "disable-all") == 0) {
    button->mapping = MAPPING_DISABLE;
  } else if (strcmp(param, "disable-mouse") == 0) {
    button->mapping = MAPPING_DISABLE_MOUSE;
  } else if (strcmp(param, "disable-keys") == 0) {
    button->mapping = MAPPING_DISABLE_KEYS;
  } else {
    xf86Msg(X_WARNING, "%s: error parsing button parameter.\n", 
            name);
  }
  xfree(param);
}


/**
 * Interpretes one AxisMappingX option, given in 'org'
 * stores the result in *axis
 * name is the name of the InputDevice
 **/

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
      else if (strcmp(p, "none") == 0)
        axis->type = TYPE_NONE;
      else {
        axis->type = TYPE_NONE;
        xf86Msg(X_WARNING, "%s: \"%s\": error parsing mode.\n", 
                name, param);
      }
    }else xf86Msg(X_WARNING, "%s: \"%s\": error parsing mode.\n", 
                  name, param);
  }

  if ((tmp=strstr(param, "axis=")) != NULL) {
    if (sscanf(tmp, "axis=%15s", p) == 1) {
      p[15]='\0';
      axis->mapping = jstkGetAxisMapping(&fvalue, p, name);
      axis->amplify = fvalue;
      if (axis->mapping == MAPPING_NONE)
        xf86Msg(X_WARNING, "%s: error parsing axis: %s.\n", 
                name, p);
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

  xfree(param);
}