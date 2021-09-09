#ifndef Button_h
#define Button_h

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

#include <Arduino.h>

class Button {
  public:
    Button(uint8_t pin, uint8_t invert, uint32_t dbTime);
    uint8_t read();
    uint8_t setState(uint8_t);
    uint8_t isPressed();
    uint8_t isReleased();
    uint8_t wasPressed();
    uint8_t wasReleased();
    uint8_t pressedFor(uint32_t ms);
    uint8_t releasedFor(uint32_t ms);
    uint8_t wasReleasefor(uint32_t ms);
    uint32_t lastChange();

  private:
    uint8_t _pin;           //arduino pin number
    uint8_t _puEnable;      //internal pullup resistor enabled
    uint8_t _invert;        //if 0, interpret high state as pressed, else interpret low state as pressed
    uint8_t _state;         //current button state
    uint8_t _lastState;     //previous button state
    uint8_t _changed;       //state changed since last read
    uint32_t _time;         //time of current state (all times are in ms)
    uint32_t _lastTime;     //time of previous state
    uint32_t _lastChange;   //time of last state change
    uint32_t _dbTime;       //debounce time
    uint32_t _pressTime;    //press time
    uint32_t _hold_time;    //hold time call wasreleasefor

    uint8_t _axis;          //state changed since last read
};


Button::Button(uint8_t pin, uint8_t invert, uint32_t dbTime) {
  _pin = pin;
  _invert = invert;
  _dbTime = dbTime;
  pinMode(_pin, INPUT_PULLUP);
  _state = digitalRead(_pin);
  if (_invert != 0) _state = !_state;
  _time = millis();
  _lastState = _state;
  _changed = 0;
  _hold_time = -1;
  _lastTime = _time;
  _lastChange = _time;
  _pressTime = _time;
}

uint8_t Button::read(void) {
  static uint8_t pinVal;
  #if defined (ARDUINO_M5Stack_Core_ESP32) // m5stack classic/fire
    pinVal = analogRead(_pin);
  #else
    pinVal = digitalRead(_pin);
  #endif
  if (_invert != 0) pinVal = !pinVal;
  return setState(pinVal);
}

uint8_t Button::setState(uint8_t pinVal)
{
  static uint32_t ms;

  ms = millis();
  if (ms - _lastChange < _dbTime) {
    _lastTime = _time;
    _time = ms;
    _changed = 0;
    return _state;
  }
  else {
    _lastTime = _time;
    _time = ms;
    _lastState = _state;
    _state = pinVal;
    if (_state != _lastState) {
      _lastChange = ms;
      _changed = 1;
      if (_state) { _pressTime = _time; }
    }
    else {
      _changed = 0;
    }
    return _state;
  }
}

uint8_t Button::isPressed(void) {
  return _state == 0 ? 0 : 1;
}

uint8_t Button::isReleased(void) {
  return _state == 0 ? 1 : 0;
}

uint8_t Button::wasPressed(void) {
  return _state && _changed;
}

uint8_t Button::wasReleased(void) {
  return !_state && _changed && millis() - _pressTime < _hold_time;
}

uint8_t Button::wasReleasefor(uint32_t ms) {
  _hold_time = ms;
  return !_state && _changed && millis() - _pressTime >= ms;
}


uint8_t Button::pressedFor(uint32_t ms) {
  return (_state == 1 && _time - _lastChange >= ms) ? 1 : 0;
}

uint8_t Button::releasedFor(uint32_t ms) {
  return (_state == 0 && _time - _lastChange >= ms) ? 1 : 0;
}

uint32_t Button::lastChange(void) {
  return _lastChange;
}

#pragma GCC diagnostic pop

#endif
