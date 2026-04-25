#ifndef EVENT_DETECTORS_H
#define EVENT_DETECTORS_H

#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#define PIN_BUTTON          2
#define PIN_POTENCIOMETRO   3
#define UMBRAL_MOVIMIENTO   2.5
#define UMBRAL_TOUCH        2000
#define MAX_TYPE_EVENTS     3
#define TOTAL_EVENTOS       4

enum Estado {
  APAGADO,
  ACTIVO,
  ADVERTENCIA_MOVIMIENTO,
  ADVERTENCIA_CONTACTO,
  ALERTA
};

enum Evento {
  APAGAR,
  PRENDER,
  MOV_DETECTADO,
  TOUCH_DETECTADO
};

typedef bool (*EventDetector)();

extern Estado           estadoActual;
extern Evento           newEvent;
extern short            lastIndexTypeSensor;
extern Adafruit_MPU6050 mpu;
extern float            lastX, lastY, lastZ;
extern EventDetector    eventType[MAX_TYPE_EVENTS];

bool detectaBotonEvent      ();
bool detectaMovimientoBrusco();
bool detectaMovimientoEvent ();
bool detectaTouchEvent      ();
bool getNuevoEvento         ();

#endif
