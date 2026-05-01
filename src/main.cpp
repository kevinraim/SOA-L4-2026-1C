#include <Arduino.h>
#include "state_machine.h"
#include "events.h"

void setup() {
  Serial.begin(SERIAL_BAUD);
  initHardware();
  initEventDetectors();
}

void loop() {
  procesarEvento();
  vTaskDelay(pdMS_TO_TICKS(200));
}
