#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define SERIAL_BAUD 115200
#define PIN_BUTTON 2
#define SPEAKER_PIN 0  
#define LED_PIN 1      
#define UMBRAL_MOVIMIENTO 2.5 // Sensibilidad: cuanto menor el número, más sensible

Adafruit_MPU6050 mpu;
float lastX, lastY, lastZ;

enum Estado {
  APAGADO = 0,
  ACTIVO,
  ALERTADO,
  TOTAL_ESTADOS
};

enum Evento {
  APAGAR = 0,
  PRENDER,
  MOV_DETECTADO,
  TOTAL_EVENTOS
};

typedef void (*Accion)();

Estado estadoActual = APAGADO;

const Evento EVENTO_INVALIDO = TOTAL_EVENTOS;

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

void nada() {
  Serial.println("Evento sin accion");
}

void prender() {
  Serial.println(">>> SISTEMA ARMADO");
  guardaPosicion();
  digitalWrite(LED_PIN, HIGH);
  noTone(SPEAKER_PIN);
  estadoActual = ACTIVO;
}

void alertar() {
  Serial.println("¡MOVIMIENTO DETECTADO!");
  digitalWrite(LED_PIN, HIGH);
  tone(SPEAKER_PIN, 880);
  estadoActual = ALERTADO;
}

void apagar() {
  Serial.println(">>> SISTEMA DESARMADO");
  if (estadoActual == ALERTADO) {
    noTone(SPEAKER_PIN);
  }
  digitalWrite(LED_PIN, LOW);
  estadoActual = APAGADO;
}

void errorTransicion() {
  Serial.println("Transicion invalida");
}

const Accion MATRIZ_TRANSICION[TOTAL_ESTADOS][TOTAL_EVENTOS] = {
  // APAGAR, PRENDER, MOV_DETECTADO
  { errorTransicion, prender,       errorTransicion }, // APAGADO
  { apagar,          errorTransicion, alertar        }, // ACTIVO
  { apagar,          errorTransicion, errorTransicion } // ALERTADO
};

Evento detectaBoton() {
  if (digitalRead(PIN_BUTTON) == LOW) {
    if (estadoActual == APAGADO) {
      delay(500);
      return PRENDER;
    } else {
      delay(500);
      return APAGAR;
    }
  }

  return EVENTO_INVALIDO;
}

bool detectaMovimientoBrusco(){
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  float diffX = abs(a.acceleration.x - lastX);
  float diffY = abs(a.acceleration.y - lastY);
  float diffZ = abs(a.acceleration.z - lastZ);
  if (diffX > UMBRAL_MOVIMIENTO || diffY > UMBRAL_MOVIMIENTO || diffZ > UMBRAL_MOVIMIENTO) {
    return true;
  }
  return false;
}

Evento detectaMovimiento(){
  if (estadoActual == ACTIVO && detectaMovimientoBrusco()) {
    return MOV_DETECTADO;
  }

  return EVENTO_INVALIDO;
}

Evento getNuevoEvento() {
  Evento evento = detectaBoton();
  if (evento != EVENTO_INVALIDO) {
    return evento;
  }

  return detectaMovimiento();
}

void loop() {
  Evento nuevoEvento = getNuevoEvento();

  if ((nuevoEvento >= 0) && (nuevoEvento < TOTAL_EVENTOS) &&
      (estadoActual >= 0) && (estadoActual < TOTAL_ESTADOS)) {
    MATRIZ_TRANSICION[estadoActual][nuevoEvento]();
  }

  delay(100);
}