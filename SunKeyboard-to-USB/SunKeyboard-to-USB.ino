/*
 * USB Adapter for Sun Type 5 Keyboard
 * Ben Rockwood <benr@cuddletech.com> 1/1/17
 *
 * Developed on Arduino Micro
 * MiniDIN 8a Adapter P/N: MD-80PL100
 * Wiring:
 *    Keyboard Pin 2 (White): GND
 *    Keyboard Pin 8 (Red): +5V
 *    Keyboard Pin 6 (Yellow): Arduino Pin D10 (Serial RX)     // Keyboard's Input
 *    Keyboard Pin 5 (Green): Aruidno Pin D11 (Serial TX)      // Keyboard's Output
 *
 *
 * Modified for Arduino Uno
 * 2019/07/10
 */

/* #include <Keyboard.h> */
#include <SoftwareSerial.h>
#include "sun_keyboard_map.h"

//Software serial for Sun KBD
//Use Serial Pin 10,11
SoftwareSerial sunSerial(10, 11, true);

boolean NUM_LOCK_ON = false;  // led bitfield xxx1 (1)
boolean CAPS_LOCK_ON = false; // led bitfield 1xxx (8)

byte led_cmd[2] = { 0x0E, 0x01 };  // Used for sending the 2 byte commands to the keyboard

boolean flg_test = false ; 
int KEY_UP = -1 ;
int KEY_DOWN = 1 ;

// int seq_no = 0;
int modifier = 0;

/* Sun Key */
#define KEY_RESET       0x00
uint8_t keyData[8] = { 0, 0, 0, 0, 0, 0, 0 };


void setup() {
  /* Serial.begin(1200); */       // Normal serial for Serial Monitor Debugging
  Serial.begin(9600);		// Serial Connection to PC's USB --PIN is 0,1--  :at original, Serial is only for Debugging 
  
  sunSerial.begin(1200);	// Serial Connection to Sun Keyboard
 
  /* Keyboard.begin(); */            // Initialize USB Keyboard Interface

  sunSerial.write(led_cmd, 2);  // Enable Number Lock by Default


}


void loop() {
  char c = sunSerial.read();
    
  if (c != 0xFFFFFFFF) {
   switch(c) {
    case 45:  sunSerial.write(CMD_DISABLE_CLICK); break;     // Mute Key to Disable Click Sound
    case 2:   sunSerial.write(CMD_ENABLE_CLICK); break;      // Decr Vol Key to Enable Click Sound
    case 4:   sunSerial.write(CMD_ENABLE_BELL); break;       // Incr Vol to Enable Bell
    case 48:  sunSerial.write(CMD_DISABLE_BELL); break;      // Power Key to Disable Bell 
    case 98:  break;                                         // Ignore NumLock
    case 119: if (!CAPS_LOCK_ON) {                           // Caps Lock Toggle
                CAPS_LOCK_ON = true;
                led_cmd[1] += 8;
                sunSerial.write(led_cmd, 2);
              } else {
                CAPS_LOCK_ON = false;
                led_cmd[1] -= 8;
                sunSerial.write(led_cmd, 2);                 
              }
    default: outputKey(c); break;
   }
  }
}


void outputKey(int key){

 int keycode = sun_to_ascii[key];
 
 /* HID protocol
Byte	Contents
0 : Bit0	Left CTRL
0 : Bit1	Left SHIFT
0 : Bit2	Left ALT
0 : Bit3	Left GUI
0 : Bit4	Right CTRL
0 : Bit5	Right SHIFT
0 : Bit6	Right ALT
0 : Bit7	Right GUI
1	no use
2~7	HID active key codes
 */

 
 if (key < 0) {	
  key += 128;
  // Keyboard.release(sun_to_ascii[key]);
  if( flg_test ){
    modifierKey(key , KEY_DOWN);
    Serial.print("Key Up: "); Serial.print(key); Serial.print("->") ; Serial.print("Mod:"); Serial.print(modifier,BIN); Serial.print(" keycode:"); Serial.println(sun_to_ascii[key]);
  }else{
    modifierKey(key , KEY_UP);
    keyData[0] = modifier;
    keyData[2] = KEY_RESET;
    Serial.write(keyData , 8);  // send key release
  }
  
 } else if (key == 127) {
  // Keyboard.releaseAll();
  if( flg_test ){
    modifier = 0;
    Serial.println("All Keys Released");
  }else{
    keyData[0] = KEY_RESET;
    keyData[2] = KEY_RESET;
    modifier = 0;
    Serial.write(keyData , 8);  // send key release
  }
  
 } else {
  // Keyboard.press(sun_to_ascii[key]);
  if( flg_test ){
    modifierKey(key , KEY_DOWN);
    Serial.print("Key Down: ");  Serial.print(key); Serial.print("->") ;Serial.print("Mod:"); Serial.print(modifier,BIN); Serial.print(" keycode:"); Serial.println(sun_to_ascii[key]);
  }else{
    modifierKey(key , KEY_DOWN);
    keyData[0] = modifier;
    keyData[2] = keycode;
    Serial.write(keyData , 8);  // send key press
  }  
 }
}

void modifierKey(int key, int keyflg){
 // keyflg = 1   keydown
 // keyflg = -1  keyup  

  switch(key){
  case 76: // Ctrl_L
    modifier += 1*keyflg ;
    break;
  case 99: // Shift_L
    modifier += 2*keyflg ;
    break;
  case 19: // Alt_L
    modifier += 4*keyflg ;
    break;
  case 120: // Meta_L
    modifier += 8*keyflg ;
    break;
  case 110: // Shift_R
    modifier += 32*keyflg ;
    break;
  case 122: // Meta_R
    modifier += 128*keyflg ;
    break;
  default:
    break;
  }
}

