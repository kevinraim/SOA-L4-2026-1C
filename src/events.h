#ifndef EVENTS_H
#define EVENTS_H

#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/timers.h>
#include "config.h"

#define MAX_TYPE_EVENTS  4
#define TOTAL_EVENTOS    5

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
  TOUCH_DETECTADO,
  TIMEOUT_ADVERTENCIA
};

typedef bool (*EventDetector)();

extern Estado           estadoActual;
extern Evento           newEvent;
extern short            lastIndexTypeSensor;
extern Adafruit_MPU6050 mpu;
extern float            lastX, lastY, lastZ;
extern EventDetector    eventType[MAX_TYPE_EVENTS];

void initEventDetectors         ();
void iniciarTimeoutAdvertencia  ();
void cancelarTimeoutAdvertencia ();
bool getNuevoEvento             ();

#endif
