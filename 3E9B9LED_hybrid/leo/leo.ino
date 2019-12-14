/* Arduino IIDX/SDVX Hybrid Controller for Leonardo
 * 3 Encoders + 9 Buttons + 9 HID controlable LED
 * with switchable analog/digital turntable output
 * release page
 * http://knuckleslee.blogspot.com/2018/06/RhythmCodes.html
 * 
 * Arduino Joystick Library
 * https://github.com/MHeironimus/ArduinoJoystickLibrary/
 * mon's Arduino-HID-Lighting
 * https://github.com/mon/Arduino-HID-Lighting
 */
 #include <Joystick.h>
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_GAMEPAD, 14, 0,
 true, true, true, false, false, false, false, false, false, false, false);

boolean hidMode, ttMode, state[3]={false}, set[6]={false};
int encL=0, encR=0, encTT=0, TTold=0;
unsigned long TTmillis;
const int PULSE = 600;  //number of pulses per revolution of encoders 
const int GEAR = 60;    //number of gear teeth or ppr of encoders
const int TTdz = 0;     //digital tt deadzone (pulse)
const int TTdelay = 50;  //digital tt button release delay (millisecond)
byte EncPins[]    = {0, 1, 2, 3, 6, 7};
byte SinglePins[] = {4, 8, 10,12,18,20,22,14,16};
byte ButtonPins[] = {5, 9, 11,13,19,21,23,15,17};

/* pin assignments
 * VOL-L Green to pin 0 and White to pin 1
 * VOL-R Green to pin 2 and White to pin 3
 *   TT Sensor to pin 6 and White to pin 7
 * current pin layout
 *  SinglePins {4,8,10,12,18,20,22,14,16} = LED 1 to 9
 *    connect pin to resistor and then + termnial of LED
 *    connect ground to - terminal of LED
 *  ButtonPins {5,9,11,13,19,21,23,15,17} = Button input 1 to 9
 *    connect button pin to ground to trigger button press
 *  Light mode detection by read first button while connecting usb 
 *   hold    = false = reactive lighting 
 *   release = true  = HID lighting
 *  TT mode detection by read second button while connecting usb 
 *   hold    = false = digital turntable mode
 *   release = true  =  analog turntable mode
 */

byte ButtonCount = sizeof(ButtonPins) / sizeof(ButtonPins[0]);
byte SingleCount = sizeof(SinglePins) / sizeof(SinglePins[0]);
byte EncPinCount = sizeof(EncPins) / sizeof(EncPins[0]);

int ReportDelay = 700;
unsigned long ReportRate ;

void setup() {
  Serial.begin(9600);
  Joystick.begin(false);
  Joystick.setXAxisRange(-PULSE/2, PULSE/2-1);
  Joystick.setYAxisRange(-PULSE/2, PULSE/2-1);
  Joystick.setZAxisRange(-GEAR/2, GEAR/2-1);
  
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
  attachInterrupt(digitalPinToInterrupt(EncPins[5]), doEncoder2, CHANGE);
  
  // light and turntable mode detection
  hidMode = digitalRead(ButtonPins[0]);
   ttMode = digitalRead(ButtonPins[1]);
  while(digitalRead(ButtonPins[0])==LOW
       |digitalRead(ButtonPins[1])==LOW) {
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
    
  //read buttons
  for(int i=0;i<ButtonCount;i++) {
    Joystick.setButton (i,!(digitalRead(ButtonPins[i])));
  }

  if(hidMode==false){
    for(int i=0;i<ButtonCount;i++) {
      digitalWrite (SinglePins[i],!(digitalRead(ButtonPins[i])));
    }
  }

  //read encoders, detect overflow and rollover
  if(encL < -PULSE/2 || encL > PULSE/2-1)
  encL = constrain (encL*-1, -PULSE/2, PULSE/2-1);
  if(encR < -PULSE/2 || encR > PULSE/2-1)
  encR = constrain (encR*-1, -PULSE/2, PULSE/2-1);
  Joystick.setXAxis(encL);
  Joystick.setYAxis(encR);

  //analog mode, detect overflow and rollover
  if(ttMode==true) {
    if(encTT < -GEAR/2 || encTT > GEAR/2-1)
    encTT = constrain (encTT*-1, -GEAR/2, GEAR/2-1);
    Joystick.setZAxis(encTT);
  }
  //digital mode
  else if(ttMode==false) {
    if(encTT != TTold) {
      if(encTT < -TTdz) {
        Joystick.setButton (11,1);
        Joystick.setButton (12,0);
        TTmillis = millis();
        encTT = 0;
      }
      else if(encTT > TTdz) {
        Joystick.setButton (11,0);
        Joystick.setButton (12,1);
        TTmillis = millis();
        encTT = 0;
      }
    }
    if((millis() - TTmillis) > TTdelay) {
      Joystick.setButton (11,0);
      Joystick.setButton (12,0);
      TTmillis = millis();
      encTT = 0;
    }
    TTold = encTT;
  }
  
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
      encL++;
    }
    if(set[0] == false && set[1] == false) {
      encL--;
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
      encR++;
    }
    if(set[2] == false && set[3] == false) {
      encR--;
    }
    state[1] = false;
  }
}

void doEncoder2() {
  if(state[2] == false && digitalRead(EncPins[5]) == LOW) {
    set[4] = digitalRead(EncPins[4]);
    state[2] = true;
  }
  if(state[2] == true && digitalRead(EncPins[5]) == HIGH) {
    set[5] = !digitalRead(EncPins[4]);
    if(set[4] == true && set[5] == true) {
      encTT--;
    }
    if(set[4] == false && set[5] == false) {
      encTT++;
    }
    state[2] = false;
  }
}
