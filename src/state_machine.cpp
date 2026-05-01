#include "state_machine.h"
#include <Arduino.h>
#include <Wire.h>

typedef void (*Accion)();

// Acciones
static void guardaPosicion() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  lastX = a.acceleration.x;
  lastY = a.acceleration.y;
  lastZ = a.acceleration.z;
}

static void prender() {
  Serial.println(">>> SISTEMA ARMADO");
  guardaPosicion();
  estadoActual = ACTIVO;
}

static void advertirMovimiento() {
  Serial.println(">>> ADVERTENCIA: Movimiento detectado");
  digitalWrite(LED_PIN, HIGH);
  iniciarTimeoutAdvertencia();
  estadoActual = ADVERTENCIA_MOVIMIENTO;
}

static void advertirContacto() {
  Serial.println(">>> ADVERTENCIA: Contacto detectado");
  digitalWrite(LED_PIN, HIGH);
  iniciarTimeoutAdvertencia();
  estadoActual = ADVERTENCIA_CONTACTO;
}

static void alertar() {
  Serial.println("¡ALERTA! Movimiento Y contacto detectados");
  cancelarTimeoutAdvertencia();
  digitalWrite(LED_PIN, HIGH);
  tone(SPEAKER_PIN, 880);
  estadoActual = ALERTA;
}

static void apagar() {
  Serial.println(">>> SISTEMA DESARMADO");
  cancelarTimeoutAdvertencia();
  if (estadoActual == ALERTA) noTone(SPEAKER_PIN);
  digitalWrite(LED_PIN, LOW);
  estadoActual = APAGADO;
}

static void reiniciarAdvertencia() {
  Serial.println(">>> Timeout: volviendo a ACTIVO");
  digitalWrite(LED_PIN, LOW);
  estadoActual = ACTIVO;
}

static void errorTransicion() {
  Serial.println("Transicion invalida");
}

// Matriz de transición (FSM)
// Eventos: APAGAR, PRENDER, MOV_DETECTADO, TOUCH_DETECTADO, TIMEOUT_ADVERTENCIA
static const Accion MATRIZ_TRANSICION[TOTAL_ESTADOS][TOTAL_EVENTOS] = {
  { errorTransicion, prender,          errorTransicion,    errorTransicion,     errorTransicion      }, // APAGADO
  { apagar,          errorTransicion,  advertirMovimiento, advertirContacto,    errorTransicion      }, // ACTIVO
  { apagar,          errorTransicion,  errorTransicion,    alertar,             reiniciarAdvertencia }, // ADVERTENCIA_MOVIMIENTO
  { apagar,          errorTransicion,  alertar,            errorTransicion,     reiniciarAdvertencia }, // ADVERTENCIA_CONTACTO
  { apagar,          errorTransicion,  errorTransicion,    errorTransicion,     errorTransicion      }  // ALERTA
};

// Inicializar sensores y actuadores
void initHardware() {
  Wire.begin(ACCELEROMETER_SDA, ACCELEROMETER_SCL);
  if (!mpu.begin()) {
    Serial.println("No se encontro el sensor en los pines SDA y SCL");
  }
  Serial.println("¡MPU6050 conectado en los pines!");
  pinMode(PIN_BUTTON,   INPUT_PULLUP);
  pinMode(SPEAKER_PIN,  OUTPUT);
  pinMode(LED_PIN,      OUTPUT);
}


void procesarEvento() {
  if (getNuevoEvento()) {
    if ((newEvent >= 0) && (newEvent < TOTAL_EVENTOS) &&
        (estadoActual >= 0) && (estadoActual < TOTAL_ESTADOS)) {
      MATRIZ_TRANSICION[estadoActual][newEvent]();
    }
  }
}
