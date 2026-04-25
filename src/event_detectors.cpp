#include "event_detectors.h"

bool detectaBotonEvent() {
  if (digitalRead(PIN_BUTTON) == LOW) {
    delay(500);
    newEvent = (estadoActual == APAGADO) ? PRENDER : APAGAR;
    return true;
  }
  return false;
}

bool detectaMovimientoBrusco() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  float diffX = abs(a.acceleration.x - lastX);
  float diffY = abs(a.acceleration.y - lastY);
  float diffZ = abs(a.acceleration.z - lastZ);
  return (diffX > UMBRAL_MOVIMIENTO || diffY > UMBRAL_MOVIMIENTO || diffZ > UMBRAL_MOVIMIENTO);
}

bool detectaMovimientoEvent() {
  if ((estadoActual == ACTIVO || estadoActual == ADVERTENCIA_CONTACTO) && detectaMovimientoBrusco()) {
    newEvent = MOV_DETECTADO;
    return true;
  }
  return false;
}

bool detectaTouchEvent() {
  int potValue = analogRead(PIN_POTENCIOMETRO);
  if ((estadoActual == ACTIVO || estadoActual == ADVERTENCIA_MOVIMIENTO) && potValue > UMBRAL_TOUCH) {
    newEvent = TOUCH_DETECTADO;
    return true;
  }
  return false;
}

EventDetector eventType[MAX_TYPE_EVENTS] = {
  detectaBotonEvent,
  detectaMovimientoEvent,
  detectaTouchEvent
};

bool getNuevoEvento() {
  short index = lastIndexTypeSensor % MAX_TYPE_EVENTS;
  lastIndexTypeSensor++;
  return eventType[index]();
}
