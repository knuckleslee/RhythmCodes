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
#include "HIDLED.h"

SinglePin extern SinglePins[];
RGBPin extern RGBPins[];
unsigned long extern ReactiveTimeoutCount;

void led_stuff(SinglePin pins, byte brightness)
{
  analogWrite(pins.pin, brightness);
}

void rgb_stuff(RGBPin pins, byte r, byte g, byte b)
{
  analogWrite(pins.r, r);
  analogWrite(pins.g, g);
  analogWrite(pins.b, b);
}

void light_update(SingleLED* single_leds, RGBLed* rgb_leds) {
  for(int i = 0; i < NUMBER_OF_SINGLE; i++) {
    if(hidMode == true){
      led_stuff(SinglePins[i], single_leds[i].brightness);
    }
  }
  for(int i = 0; i < NUMBER_OF_RGB; i++) {
    if(rgbCommon == '+'){  //if direct drive common anode LEDs
      rgb_stuff(RGBPins[i], map(rgb_leds[i].r, 0, 255, 255, 0), map(rgb_leds[i].g, 0, 255, 255, 0), map(rgb_leds[i].b, 0, 255, 255, 0));
    }
    if(rgbCommon == '-'){  //if direct drive common cathode LEDs
      rgb_stuff(RGBPins[i], rgb_leds[i].r, rgb_leds[i].g, rgb_leds[i].b);
    }
  }
  ReactiveTimeoutCount = 0;
}

static const byte PROGMEM _hidReportLEDs[] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x00,                    // USAGE (Undefined)
    0xa1, 0x01,                    // COLLECTION (Application)
    // Globals
    0x95, NUMBER_OF_LIGHTS,        //   REPORT_COUNT
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x05, 0x0a,                    //   USAGE_PAGE (Ordinals)
    // Locals
    0x19, 0x01,                    //   USAGE_MINIMUM (Instance 1)
    0x29, NUMBER_OF_LIGHTS,        //   USAGE_MAXIMUM (Instance n)
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
    // BTools needs at least 1 input to work properly
    0x19, 0x00,                    //   USAGE_MINIMUM (Instance 1)
    0x29, 0x00,                    //   USAGE_MAXIMUM (Instance 1)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
    0xc0                           // END_COLLECTION
};

// This is almost entirely copied from NicoHood's wonderful RawHID example
// Trimmed to the bare minimum
// https://github.com/NicoHood/HID/blob/master/src/SingleReport/RawHID.cpp
class HIDLED_ : public PluggableUSBModule {

  byte epType[1];

  public:
    HIDLED_(void) : PluggableUSBModule(1, 1, epType) {
      epType[0] = EP_TYPE_INTERRUPT_IN;
      PluggableUSB().plug(this);
    }

    int getInterface(byte* interfaceCount) {
      *interfaceCount += 1; // uses 1
      HIDDescriptor hidInterface = {
        D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
        D_HIDREPORT(sizeof(_hidReportLEDs)),
        D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 16)
      };
      return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
    }

    int getDescriptor(USBSetup& setup)
    {
      // Check if this is a HID Class Descriptor request
      if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE) { return 0; }
      if (setup.wValueH != HID_REPORT_DESCRIPTOR_TYPE) { return 0; }

      // In a HID Class Descriptor wIndex contains the interface number
      if (setup.wIndex != pluggedInterface) { return 0; }

      return USB_SendControl(TRANSFER_PGM, _hidReportLEDs, sizeof(_hidReportLEDs));
    }

    bool setup(USBSetup& setup)
    {
      if (pluggedInterface != setup.wIndex) {
        return false;
      }

      byte request = setup.bRequest;
      byte requestType = setup.bmRequestType;

      if (requestType == REQUEST_DEVICETOHOST_CLASS_INTERFACE)
      {
        return true;
      }

      if (requestType == REQUEST_HOSTTODEVICE_CLASS_INTERFACE) {
        if (request == HID_SET_REPORT) {
          if(setup.wValueH == HID_REPORT_TYPE_OUTPUT && setup.wLength == NUMBER_OF_LIGHTS){
            USB_RecvControl(led_data.raw, NUMBER_OF_LIGHTS);
            light_update(led_data.leds.singles, led_data.leds.rgb);
            return true;
          }
        }
      }

      return false;
    }
};

HIDLED_ HIDLeds;
