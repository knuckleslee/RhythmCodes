/* Arduino-HID-Lighting-Library
 * 
 * This Arduino-HID-Lighting-Library is derived from Arduino-HID-Lighting, whose copyriht owner is mon.
 * More information about Arduino-HID-Lighting you can find under:
 * 
 * mon's Arduino-HID-Lighting
 * https://github.com/mon/Arduino-HID-Lighting
 * 
 * 2018 (C) Arduino-HID-Lighting-Library, Knuckleslee
*/
#include "HID.h"

#define NUMBER_OF_SINGLE 11
#define NUMBER_OF_RGB 0
#define NUMBER_OF_LIGHTS (NUMBER_OF_SINGLE + NUMBER_OF_RGB*3)
extern boolean hidMode;
extern char rgbCommon;

typedef struct {
  byte brightness;
} SingleLED;

typedef struct {
  byte r;
  byte g;
  byte b;
} RGBLed;

typedef struct {
  byte pin;
} SinglePin;

typedef struct {
  byte r;
  byte g;
  byte b;
} RGBPin;

union {
  struct {
    SingleLED singles[NUMBER_OF_SINGLE];
    RGBLed rgb[NUMBER_OF_RGB];
  } leds;
  byte raw[NUMBER_OF_LIGHTS];
} led_data;
