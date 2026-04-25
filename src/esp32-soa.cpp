#include <Arduino.h>
#include <Wire.h>
#include "event_detectors.h"

#define SERIAL_BAUD 115200
#define SPEAKER_PIN 0
#define LED_PIN     1
#define TOTAL_ESTADOS 5

Adafruit_MPU6050 mpu;
float lastX, lastY, lastZ;

Evento newEvent;
short  lastIndexTypeSensor = 0;

typedef void (*Accion)();

Estado estadoActual = APAGADO;

void iniciaMPU(){
  Wire.begin(); 
  if (!mpu.begin()) {
    Serial.println("No se encontro el sensor en los pines 8 y 9");
    while (1) delay(10);
  }
  Serial.println("¡MPU6050 conectado en los pines!");
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  iniciaMPU();
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
}

void guardaPosicion(){
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  lastX = a.acceleration.x;
  lastY = a.acceleration.y;
  lastZ = a.acceleration.z;
}

void prender() {
  Serial.println(">>> SISTEMA ARMADO");
  guardaPosicion();
  digitalWrite(LED_PIN, HIGH);
  noTone(SPEAKER_PIN);
  estadoActual = ACTIVO;
}

void advertirMovimiento() {
  Serial.println(">>> ADVERTENCIA: Movimiento detectado");
  digitalWrite(LED_PIN, HIGH);
  noTone(SPEAKER_PIN);
  estadoActual = ADVERTENCIA_MOVIMIENTO;
}

void advertirContacto() {
  Serial.println(">>> ADVERTENCIA: Contacto detectado");
  digitalWrite(LED_PIN, HIGH);
  noTone(SPEAKER_PIN);
  estadoActual = ADVERTENCIA_CONTACTO;
}

void alertar() {
  Serial.println("¡ALERTA! Movimiento Y contacto detectados");
  digitalWrite(LED_PIN, HIGH);
  tone(SPEAKER_PIN, 880);
  estadoActual = ALERTA;
}

void apagar() {
  Serial.println(">>> SISTEMA DESARMADO");
  if (estadoActual == ALERTA) {
    noTone(SPEAKER_PIN);
  }
  digitalWrite(LED_PIN, LOW);
  estadoActual = APAGADO;
}

void errorTransicion() {
  Serial.println("Transicion invalida");
}

const Accion MATRIZ_TRANSICION[TOTAL_ESTADOS][TOTAL_EVENTOS] = {
  // APAGAR,          PRENDER,          MOV_DETECTADO,     TOUCH_DETECTADO
  { errorTransicion,  prender,          errorTransicion,   errorTransicion  }, // APAGADO
  { apagar,           errorTransicion,  advertirMovimiento, advertirContacto }, // ACTIVO
  { apagar,           errorTransicion,  errorTransicion,   alertar          }, // ADVERTENCIA_MOVIMIENTO
  { apagar,           errorTransicion,  alertar,           errorTransicion  }, // ADVERTENCIA_CONTACTO
  { apagar,           errorTransicion,  errorTransicion,   errorTransicion  }  // ALERTA
};

void loop() {
  if (getNuevoEvento()) {
    if ((newEvent >= 0) && (newEvent < TOTAL_EVENTOS) &&
        (estadoActual >= 0) && (estadoActual < TOTAL_ESTADOS)) {
      MATRIZ_TRANSICION[estadoActual][newEvent]();
    }
  }

  delay(100);
}