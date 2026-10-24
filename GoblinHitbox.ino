#include <Arduino.h>
#include <Joystick.h>      // USB joystick (built into arduino-pico)
#include <JoystickBLE.h>   // BLE joystick (built into arduino-pico)
#include "pico/cyw43_arch.h"

// Pin layout
const int PIN_LEFT = 15, PIN_DOWN = 13, PIN_RIGHT = 14, PIN_UP = 12;
const int PIN_START = 2, PIN_SELECT = 3;
const int PIN_A = 5, PIN_B = 20, PIN_X = 19, PIN_Y = 18;
const int PIN_SHOULDER_L = 16, PIN_SHOULDER_R = 17;
const int PIN_SHOULDER_2_L = 4, PIN_SHOULDER_2_R = 27;
const int PIN_ANALOG_BTN_L = 26, PIN_ANALOG_BTN_R = 22;

// Buttons in order -> joystick buttons 1..12
const int buttonPins[] = {
  PIN_A, PIN_B, PIN_X, PIN_Y,
  PIN_SHOULDER_L, PIN_SHOULDER_R, PIN_SHOULDER_2_L, PIN_SHOULDER_2_R,
  PIN_ANALOG_BTN_L, PIN_ANALOG_BTN_R, PIN_START, PIN_SELECT
};
const int NUM_BUTTONS = sizeof(buttonPins) / sizeof(buttonPins[0]);

bool isWiredMode = false;
uint32_t lastState = 0xFFFFFFFF;  // forces one report at startup

// Returns hat angle in degrees, or -1 when centered
int readHat() {
  bool up    = digitalRead(PIN_UP)    == LOW;
  bool down  = digitalRead(PIN_DOWN)  == LOW;
  bool left  = digitalRead(PIN_LEFT)  == LOW;
  bool right = digitalRead(PIN_RIGHT) == LOW;

  if (up && right)   return 45;
  if (down && right) return 135;
  if (down && left)  return 225;
  if (up && left)    return 315;
  if (up)    return 0;
  if (right) return 90;
  if (down)  return 180;
  if (left)  return 270;
  return -1;
}

void setup() {
  const int allPins[] = {
    PIN_LEFT, PIN_DOWN, PIN_RIGHT, PIN_UP, PIN_START, PIN_SELECT,
    PIN_A, PIN_B, PIN_X, PIN_Y, PIN_SHOULDER_L, PIN_SHOULDER_R,
    PIN_SHOULDER_2_L, PIN_SHOULDER_2_R, PIN_ANALOG_BTN_L, PIN_ANALOG_BTN_R
  };
  for (int p : allPins) pinMode(p, INPUT_PULLUP);

  // Pico W: VBUS sense is on the wireless chip's GPIO2
  delay(100);  // let the radio chip finish starting
  isWiredMode = cyw43_arch_gpio_get(CYW43_WL_GPIO_VBUS_PIN);

  if (isWiredMode) {
    Joystick.useManualSend(true);  // we call send_now() ourselves
    Joystick.begin();
  } else {
    JoystickBLE.useManualSend(true);
    JoystickBLE.begin();
  }
}

void loop() {
  int hat = readHat();

  // Pack buttons and hat into one number so changes are easy to detect
  uint32_t state = 0;
  for (int i = 0; i < NUM_BUTTONS; i++) {
    if (digitalRead(buttonPins[i]) == LOW) state |= (1UL << i);
  }
  state |= ((uint32_t)(hat + 1)) << 16;  // hat: -1..315 stored in upper bits

  // Only send a report when something actually changed
  if (state != lastState) {
    lastState = state;
    if (isWiredMode) {
      Joystick.hat(hat);
      for (int i = 0; i < NUM_BUTTONS; i++) {
        Joystick.button(i + 1, (state >> i) & 1);
      }
      Joystick.send_now();
    } else {
      JoystickBLE.hat(hat);
      for (int i = 0; i < NUM_BUTTONS; i++) {
        JoystickBLE.button(i + 1, (state >> i) & 1);
      }
      JoystickBLE.send_now();
    }
  }

  delay(5);  // scan inputs ~200 times per second
}