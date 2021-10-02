#include <Arduino.h>
#include "Pushbutton.h"

pushbutton psb(3);
char msg[80];


void setup() {
  Serial.begin(115200);
  delay(3000);
  psb.enableEvents(DOUBLE_TAP | LONG_PRESS);
  psb.enableEvents(0);
  //psb.setDelays(10, 0, 0);
}

void loop() {
  psb.update();
  if (psb.eventDetected()) {
    switch (psb.getEvent()) {
      case SINGLE_TAP:
        Serial.println("Single");
      break;
      case DOUBLE_TAP:
        Serial.println("Double");
      break;
      case LONG_PRESS:
        Serial.println("Long");
      break;
      default:
        Serial.println("Unknown");
      break;
    }
  }
}