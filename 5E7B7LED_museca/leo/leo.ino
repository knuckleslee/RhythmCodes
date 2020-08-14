/* Arduino Museca Controller Code for Leonardo
 * 5 Encoders + 7 Buttons + 7 HID controlable LED
 * release page
 * http://knuckleslee.blogspot.com/2018/06/RhythmCodes.html
 * 
 * Arduino Joystick Library
 * https://github.com/MHeironimus/ArduinoJoystickLibrary/
 * mon's Arduino-HID-Lighting
 * https://github.com/mon/Arduino-HID-Lighting
 */
#include <Joystick.h>
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_GAMEPAD, 8, 0,
 true, true, true, true, true, true, false, false, false, false, false);

boolean hidMode;
const int LEDdelay = 500;  //light duration for SinglePins[0]~[4] (millisecond)
const int PULSE = 600;  //number of pulses per revolution of encoders (600) or gear teeth (20)
byte EncPins[]    = {0, 4, 1, 5, 2, 6, 3, 8, 7, 9};
byte SinglePins[] = {10,12,18,20,22,14,16};
byte ButtonPins[] = {11,13,19,21,23,15,17};
unsigned long ReactiveTimeoutMax = 1000;  //number of cycles before HID falls back to reactive

/* pin assignments
 * Encoder 0 Green to pin 0 and White to pin 4
 * Encoder 1 Green to pin 1 and White to pin 5
 * Encoder 2 Green to pin 2 and White to pin 6
 * Encoder 3 Green to pin 3 and White to pin 8
 * Encoder 4 Green to pin 7 and White to pin 9
 * current pin layout
 *  SinglePins {10,12,18,20,22,14,16} = LED 1 to 7
 *    connect pin to resistor and then + termnial of LED
 *    connect ground to - terminal of LED
 *  ButtonPins {11,13,19,21,23,15,17} = Button input 1 to 7
 *    connect button pin to ground to trigger button press
 *  Light mode detection by read first button while connecting usb 
 *   hold    = false = reactive lighting
 *   release = true  = HID lighting with reactive fallback
 */
const byte ButtonCount = sizeof(ButtonPins) / sizeof(ButtonPins[0]);
const byte SingleCount = sizeof(SinglePins) / sizeof(SinglePins[0]);
const byte EncPinCount = sizeof(EncPins) / sizeof(EncPins[0]);
const byte EncCount = EncPinCount / 2;
int enc[EncCount]={0};
boolean state[EncCount]={false}, set[EncPinCount]={false};
unsigned long LEDmillis[EncCount]={0};
unsigned long ReactiveTimeoutCount = ReactiveTimeoutMax;

int ReportDelay = 700;
unsigned long ReportRate;

void setup() {
  Serial.begin(9600) ;
  Joystick.begin(false);
  Joystick.setXAxisRange(-PULSE/2, PULSE/2-1);
  Joystick.setYAxisRange(-PULSE/2, PULSE/2-1);
  Joystick.setZAxisRange(-PULSE/2, PULSE/2-1);
  Joystick.setRxAxisRange(-PULSE/2, PULSE/2-1);
  Joystick.setRyAxisRange(-PULSE/2, PULSE/2-1);
  Joystick.setRzAxisRange(-PULSE/2, PULSE/2-1);
  
  // setup I/O for pins
  for(int i=0;i<ButtonCount;i++) {
    pinMode(ButtonPins[i],INPUT_PULLUP);
  }
  for(int i=0;i<SingleCount;i++) {
    pinMode(SinglePins[i],OUTPUT);
  }
  for(int i=0;i<EncPinCount;i++) {
    pinMode(EncPins[i],INPUT_PULLUP);
  }

  //setup interrupts
  attachInterrupt(digitalPinToInterrupt(EncPins[0]), doEncoder0, CHANGE);
  attachInterrupt(digitalPinToInterrupt(EncPins[2]), doEncoder1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(EncPins[4]), doEncoder2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(EncPins[6]), doEncoder3, CHANGE);
  attachInterrupt(digitalPinToInterrupt(EncPins[8]), doEncoder4, CHANGE);
  
  // light mode detection
  hidMode = !digitalRead(ButtonPins[0]);  //logical reverse
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
    Joystick.setButton (i,!(digitalRead(ButtonPins[i])));
  }

  //reactive lighting
  if(hidMode==false || (hidMode==true && ReactiveTimeoutCount>=ReactiveTimeoutMax)){
    for(int i=0;i<EncCount;i++) {  //trigger for spinner light
      if(digitalRead(ButtonPins[i])==LOW) {
        LEDmillis[i] = millis();
      }
    }
    for(int i=EncCount;i<ButtonCount;i++) {  //trigger for rest of buttons
      digitalWrite(SinglePins[i],!(digitalRead(ButtonPins[i])));
    }
    for(int i=0;i<EncCount;i++) {  //control of lights
      if((millis() - LEDmillis[i]) < LEDdelay ) {
      digitalWrite(SinglePins[i],HIGH);
      } else {
      digitalWrite(SinglePins[i],LOW);
      }
    }
  }
  else if(hidMode==true) {
    ReactiveTimeoutCount++;
  }

  //read encoders, detect overflow and rollover
  for(int i=0; i<EncCount; i++) {
    if(enc[i] < -PULSE/2 || enc[i] > PULSE/2-1)
      enc[i] = constrain (enc[i]*-1, -PULSE/2, PULSE/2-1);
  }
  Joystick.setXAxis(enc[0]);
  Joystick.setZAxis(enc[1]);
  Joystick.setRxAxis(enc[2]);
  Joystick.setRyAxis(enc[3]);
  Joystick.setRzAxis(enc[4]);

  //report
  Joystick.sendState();
  delayMicroseconds(ReportDelay);
  //ReportRate Display
  Serial.print(micros() - ReportRate) ;
  Serial.println(" micro sec per loop") ;
}//end loop

//Interrupts
void doEncoder0() {
  if(state[0] == false && digitalRead(EncPins[0]) == LOW) {
    set[0] = digitalRead(EncPins[1]);
    state[0] = true;
  }
  if(state[0] == true && digitalRead(EncPins[0]) == HIGH) {
    set[1] = !digitalRead(EncPins[1]);
    if(set[0] == true && set[1] == true) {
      enc[0]++;
    }
    if(set[0] == false && set[1] == false) {
      enc[0]--;
    }
    state[0] = false;
  }
}

void doEncoder1() {
  if(state[1] == false && digitalRead(EncPins[2]) == LOW) {
    set[2] = digitalRead(EncPins[3]);
    state[1] = true;
  }
  if(state[1] == true && digitalRead(EncPins[2]) == HIGH) {
    set[3] = !digitalRead(EncPins[3]);
    if(set[2] == true && set[3] == true) {
      enc[1]++;
    }
    if(set[2] == false && set[3] == false) {
      enc[1]--;
    }
    state[1] = false;
  }
}

void doEncoder2() {
  if(state[2] == false && digitalRead(EncPins[4]) == LOW) {
    set[4] = digitalRead(EncPins[5]);
    state[2] = true;
  }
  if(state[2] == true && digitalRead(EncPins[4]) == HIGH) {
    set[5] = !digitalRead(EncPins[5]);
    if(set[4] == true && set[5] == true) {
      enc[2]++;
    }
    if(set[4] == false && set[5] == false) {
      enc[2]--;
    }
    state[2] = false;
  }
}

void doEncoder3() {
  if(state[3] == false && digitalRead(EncPins[6]) == LOW) {
    set[6] = digitalRead(EncPins[7]);
    state[3] = true;
  }
  if(state[3] == true && digitalRead(EncPins[6]) == HIGH) {
    set[7] = !digitalRead(EncPins[7]);
    if(set[6] == true && set[7] == true) {
      enc[3]++;
    }
    if(set[6] == false && set[7] == false) {
      enc[3]--;
    }
    state[3] = false;
  }
}

void doEncoder4() {
  if(state[4] == false && digitalRead(EncPins[8]) == LOW) {
    set[8] = digitalRead(EncPins[9]);
    state[4] = true;
  }
  if(state[4] == true && digitalRead(EncPins[8]) == HIGH) {
    set[9] = !digitalRead(EncPins[9]);
    if(set[8] == true && set[9] == true) {
      enc[4]++;
    }
    if(set[8] == false && set[9] == false) {
      enc[4]--;
    }
    state[4] = false;
  }
}
