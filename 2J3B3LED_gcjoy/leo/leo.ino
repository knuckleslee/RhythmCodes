/* Arduino Groove Coaster Controller Code for Leonardo
 * 2 Joystick + 3 Buttons + 3 reactive LED
 * release page
 * http://knuckleslee.blogspot.com/2018/06/RhythmCodes.html
 * 
 * SwitchControlLibrary
 * https://github.com/celclow/SwitchControlLibrary
 */
#include <SwitchControlLibrary.h>
//          {  L,  R, Plus, Lstick up, down, left, right, Rstick up(X), down(B), left(Y), right(A)};
int BT[]  = { 23, 19,   21,         0,    2,    3,     1,            4,       6,       7,        5};
int LED[] = { 22, 18,   20};
/* pin assignments
 * Joystick L up, down, left, right to pin 0, 2, 3, 1
 * Joystick R up, down, left, right to pin 4, 6, 7, 5
 * current pin layout
 *  LED[] = { 22, 18, 20} = LED 1 to 3
 *    connect pin to resistor and then + termnial of LED
 *    connect ground to - terminal of LED
 *  BT[]  = { 23, 19, 21} = Button input 1 to 3
 *    connect button pin to ground to trigger button press
 * Virtual 3rd button:
 *  hold L+R button for 5 second to press 3rd(plus) button
 */
const byte DigitalStick = 100;  //Input value for emulated analog stick
const byte ButtonCount = sizeof(BT) / sizeof(BT[0]);
const byte LedCount = sizeof(LED) / sizeof(LED[0]);
byte LeftStickX = 128;  //centered analog
byte LeftStickY = 128;  //centered analog
boolean KeyStatus[2*ButtonCount] = {0};

const int T_VButtonPlus = 5000;  //Threshold of Virtual ButtonPlus (millis)
unsigned long B_VButtonPlus[2] = {0};  //Timing buffers of Virtual ButtonPlus
unsigned long ReportRate;
int ReportDelay = 700;

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < ButtonCount; i++) 
  pinMode(BT[i], INPUT_PULLUP);
  
  for (int i = 0; i < LedCount; i++) 
  pinMode(LED[i],OUTPUT);
}

void loop() {
  ReportRate = micros() ;

  for (int i = 0; i < ButtonCount; i++) {
    KeyStatus[i] = digitalRead(BT[i]);  //read status
  }
  if(KeyStatus[0] != KeyStatus[0+ButtonCount]) {
    if(KeyStatus[0] == LOW )
      { SwitchControlLibrary().PressButtonL();  }
    else
      { SwitchControlLibrary().ReleaseButtonL();  }
  }
  if(KeyStatus[1] != KeyStatus[1+ButtonCount]) {
    if(KeyStatus[1] == LOW )
      { SwitchControlLibrary().PressButtonR();  }
    else
      { SwitchControlLibrary().ReleaseButtonR();  }
  }

//initialize Virtual ButtonPlus
  if(KeyStatus[0] != KeyStatus[0+ButtonCount] || KeyStatus[1] != KeyStatus[1+ButtonCount]) {
    if(KeyStatus[0] == LOW && KeyStatus[1] == LOW ) {
      B_VButtonPlus[0] = millis();
    }
  }
  if(millis()-B_VButtonPlus[0] > T_VButtonPlus && KeyStatus[0] == LOW && KeyStatus[1] == LOW) {
    KeyStatus[2] = LOW;
  }

  if(KeyStatus[2] != KeyStatus[2+ButtonCount]) {
    if(KeyStatus[2] == LOW )
      { SwitchControlLibrary().PressButtonPlus();  }
    else
      { SwitchControlLibrary().ReleaseButtonPlus();  }
  }
  
//initialize LeftStick
  if(KeyStatus[3] != KeyStatus[3+ButtonCount]) {
    if(KeyStatus[3] == LOW)
      {LeftStickY = LeftStickY-DigitalStick;  }
    else
      {LeftStickY = LeftStickY+DigitalStick;  }
  }
  
  if(KeyStatus[4] != KeyStatus[4+ButtonCount]) {
    if(KeyStatus[4] == LOW)
      {LeftStickY = LeftStickY+DigitalStick;  }
    else
      {LeftStickY = LeftStickY-DigitalStick;  }
  }

  if(KeyStatus[5] != KeyStatus[5+ButtonCount]) {
    if(KeyStatus[5] == LOW)
      {LeftStickX = LeftStickX-DigitalStick;  }
    else
      {LeftStickX = LeftStickX+DigitalStick;  }
  }

  if(KeyStatus[6] != KeyStatus[6+ButtonCount]) {
    if(KeyStatus[6] == LOW)
      {LeftStickX = LeftStickX+DigitalStick;  }
    else
      {LeftStickX = LeftStickX-DigitalStick;  }
  }

  SwitchControlLibrary().MoveLeftStick(LeftStickX, LeftStickY);

//
  if(KeyStatus[7] != KeyStatus[7+ButtonCount]) {
    if(KeyStatus[7] == LOW )
      { SwitchControlLibrary().PressButtonX();  }
    else
      { SwitchControlLibrary().ReleaseButtonX();  }
  }
  if(KeyStatus[8] != KeyStatus[8+ButtonCount]) {
    if(KeyStatus[8] == LOW )
      { SwitchControlLibrary().PressButtonB();  }
    else
      { SwitchControlLibrary().ReleaseButtonB();  }
  }
  if(KeyStatus[9] != KeyStatus[9+ButtonCount]) {
    if(KeyStatus[9] == LOW )
      { SwitchControlLibrary().PressButtonY();  }
    else
      { SwitchControlLibrary().ReleaseButtonY();  }
  }
  if(KeyStatus[10] != KeyStatus[10+ButtonCount]) {
    if(KeyStatus[10] == LOW )
      { SwitchControlLibrary().PressButtonA();  }
    else
      { SwitchControlLibrary().ReleaseButtonA();  }
  }

  for (int i = 0; i < ButtonCount; i++) {
    KeyStatus[i+ButtonCount] = KeyStatus[i];  //save status
  }

  for (int i = 0; i < LedCount; i++) {
    if(digitalRead(BT[i])==LOW) {
    digitalWrite(LED[i],HIGH);
    } else {
    digitalWrite(LED[i],LOW);
    }
  }

  delayMicroseconds(ReportDelay);
  //ReportRate Display
  Serial.print(micros() - ReportRate) ;
  Serial.println(" micro sec per loop") ;
}  //end loop
