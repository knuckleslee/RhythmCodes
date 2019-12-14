/* Arduino Groove Coaster Controller Code for Leonardo
 * 2 Joystick + 3 Buttons + 3 reactive LED
 * release page
 * http://knuckleslee.blogspot.com/2018/06/RhythmCodes.html
 */
#include <Keyboard.h>
//     {Button L,     R, 3rd, Lstick up, down, left, right, Rstick up, down, left, right};
//        {Space, Rctrl, Esc,         w,    s,    a,     d,  Arrow up, down, left, right};
int BT[]  = { 23,    19,  21,         0,    2,    3,     1,         4,    6,    7,     5};
int KB[]  = { 32,   132, 177,       119,  115,   97,   100,       218,  217,  216,   215};
int LED[] = { 22,    18,  20};
/* pin assignments
 * Joystick L up, down, left, right to pin 0, 2, 3, 1
 * Joystick R up, down, left, right to pin 4, 6, 7, 5
 * current pin layout
 *  LED[] = { 22, 18, 20} = LED 1 to 3
 *    connect pin to resistor and then + termnial of LED
 *    connect ground to - terminal of LED
 *  BT[]  = { 23, 19, 21} = Button input 1 to 3
 *    connect button pin to ground to trigger button press
 */
const byte ButtonCount = sizeof(BT) / sizeof(BT[0]);
const byte LedCount = sizeof(LED) / sizeof(LED[0]);
boolean KeyStatus[2*ButtonCount] = {0};

int ReportDelay = 700;
unsigned long ReportRate ;

void setup() {
  Serial.begin(9600);
  Keyboard.begin();
  for (int i = 0; i < ButtonCount; i++) 
  pinMode(BT[i], INPUT_PULLUP);
  
  for (int i = 0; i < LedCount; i++) 
  pinMode(LED[i],OUTPUT);
}

void loop() {
  ReportRate = micros() ;

  for (int i = 0; i < ButtonCount; i++) {
    KeyStatus[i] = digitalRead(BT[i]);  //read status
    if(KeyStatus[i] != KeyStatus[i+ButtonCount]) {
      if(KeyStatus[i] == LOW )
        { Keyboard.press((char)KB[i]);  }
      else
        { Keyboard.release((char)KB[i]);  }
    }
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
