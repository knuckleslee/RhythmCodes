/* Arduino Pop'n Controller Code for Leonardo
 * 12 Buttons + 12 HID controlable LED
 * release page
 * http://knuckleslee.blogspot.com/2018/06/RhythmCodes.html
 * 
 * Arduino Joystick Library
 * https://github.com/MHeironimus/ArduinoJoystickLibrary/
 * mon's Arduino-HID-Lighting
 * https://github.com/mon/Arduino-HID-Lighting
 */
#include <Joystick.h>
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_GAMEPAD, 12, 0,
 false, false, false, false, false, false, false, false, false, false, false);

boolean hidMode;
byte SinglePins[] = {0,2,4,6,8,10,12,18,20,22,14,16};
byte ButtonPins[] = {1,3,5,7,9,11,13,19,21,23,15,17};

/* pin assignments
 * current pin layout
 *  SinglePins {0,2,4,6,8,10,12,18,20,22,14,16} = LED 1 to 12
 *    connect pin to resistor and then + termnial of LED
 *    connect ground to - terminal of LED
 *  ButtonPins {1,3,5,7,9,11,13,19,21,23,15,17} = Button input 1 to 12
 *    connect button pin to ground to trigger button press
 *  Light mode detection by read first button while connecting usb 
 *   hold    = false = reactive lighting 
 *   release = true  = HID lighting
 */

byte ButtonCount = sizeof(ButtonPins) / sizeof(ButtonPins[0]);
byte SingleCount = sizeof(SinglePins) / sizeof(SinglePins[0]);

int ReportDelay = 700;
unsigned long ReportRate;

void setup() {
  Serial.begin(9600) ;
  Joystick.begin(false);
  
  // setup I/O for pins
  for(int i=0;i<ButtonCount;i++) {
    pinMode(ButtonPins[i],INPUT_PULLUP);
  }
  for(int i=0;i<SingleCount;i++) {
    pinMode(SinglePins[i],OUTPUT);
  }

  // light mode detection
  hidMode = digitalRead(ButtonPins[0]);
  while(digitalRead(ButtonPins[0])==LOW) {
    if ( (millis() % 1000) < 500) {
      for(int i=0;i<SingleCount;i++) {
        digitalWrite(SinglePins[i],HIGH);
      }
    }
    else if ( (millis() % 1000) > 500) {
      for(int i=0;i<SingleCount;i++) {
        digitalWrite(SinglePins[i],LOW);
      }
    }
  }
  for(int i=0;i<SingleCount;i++) {
    digitalWrite(SinglePins[i],LOW);
  }

  //boot light
  for(int i=0; i<ButtonCount; i++) {
    digitalWrite(SinglePins[i],HIGH);
    delay(80);
    digitalWrite(SinglePins[i],LOW);
  }
  for(int i=ButtonCount-2; i>=0; i--) {
    digitalWrite(SinglePins[i],HIGH);
    delay(80);
    digitalWrite(SinglePins[i],LOW);
  }
  for(int i=0;i<ButtonCount ;i++) {
    digitalWrite(SinglePins[i],HIGH);
  }
    delay(500);
  for(int i=0;i<ButtonCount ;i++) {
    digitalWrite(SinglePins[i],LOW);
  }
} //end setup

void loop() {
  ReportRate = micros() ;
  // read buttons
  for(int i=0;i<ButtonCount;i++) {
    if(digitalRead(ButtonPins[i])==LOW) {
      Joystick.setButton (i,1);
    } else {
      Joystick.setButton (i,0);
    }
  }

  if(hidMode==false){
    for(int i=0;i<ButtonCount;i++) {
      if(digitalRead(ButtonPins[i])==LOW) {
      digitalWrite(SinglePins[i],HIGH);
      } else {
      digitalWrite(SinglePins[i],LOW);
      }
    }
  }

  //report
  Joystick.sendState();
  delayMicroseconds(ReportDelay);
  //ReportRate Display
  Serial.print(micros() - ReportRate) ;
  Serial.println(" micro sec per loop") ;
}//end loop
