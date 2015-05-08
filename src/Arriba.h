#ifndef _ARRIBA_H_
#define _ARRIBA_H_
  
#include <pebble.h>

  
  enum {
  SETTINGS,
  PS_APLITE_THEME,
  PS_BASALT_COLORS
};

// Possible messages received from the config page
enum {
  CONFIG_KEY_SECONDS = 0x0,
  CONFIG_KEY_COLON = 0x1,
  CONFIG_KEY_BASALT_COLORS = 0x2,
  CONFIG_KEY_THEME = 0x3
};

//some useful constants
#define CENTERX 72
#define CENTERY 70
#define DIALRADIUS 64
#define SECONDRADIUS 64
#define DOTRADIUS 4
#define HOURLENGTH 40
#define MINUTELENGTH 69
#define COLORSSECOND 300
  
  #endif /* _MAIN_H_ */