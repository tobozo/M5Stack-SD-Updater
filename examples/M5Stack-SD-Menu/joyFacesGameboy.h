#ifndef __JOYFACESGAMEBOY_H
#define __JOYFACESGAMEBOY_H

/*
 * M5Stack Faces Gameboy Controls for M5Stack SD Menu
 * Currently handles up/down/select/load actions
 * 
 * Most code found here was taken from Macsbug's blog:
 * https://macsbug.wordpress.com/2018/03/07/pacman-with-m5stack/amp/
 * 
 */


bool keypadDetected = false;


void initKeypad() {
  if(!Wire.requestFrom(0X88, 1)) {
    // TODO: artwork this    
    Serial.println(DEBUG_KEYPAD_NOTFOUND);
  } else {
    Serial.println(DEBUG_KEYPAD_FOUND);
    keypadDetected = true;
    pinMode(5, INPUT_PULLUP);
    M5.Lcd.setCursor(100,100);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.print(DEBUG_KEYPAD_FOUND);
    M5.Lcd.drawJpg(joyicon_jpeg, 1070, 132, 32);
    delay(500);

  }
}


HIDSignal getKeypad(){
  if(keypadDetected==false) return UI_INERT;
  char r;
  if(digitalRead(5) == LOW) {
    Wire.requestFrom(0X88, 1);         // request 1 byte from keyboard
    while (Wire.available()) { 
      uint8_t key_val = Wire.read();   // receive a byte as character
      if ( key_val == 191) { r = 'z';} // select
      if ( key_val == 127) { r = 'x';} // start
      if ( key_val == 247) { r = '8';} // up
      if ( key_val == 251) { r = '2';} // down
      if ( key_val == 254) { r = '4';} // left
      if ( key_val == 253) { r = '6';} // right
    }
  }
  if (r == 'z') { delay(300); return UI_INFO; }
  if (r == 'x') { delay(300); return UI_LOAD; }
  if (r == '8') { return UI_UP; }
  if (r == '2') { return UI_DOWN; }
  if (r == '4') { return UI_INFO; }
  if (r == '6') { return UI_INFO; }
  
  return UI_INERT;
}

#endif
