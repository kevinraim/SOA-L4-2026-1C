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
bool sistemaArmado = false;
bool alarmaActiva = false;
float lastX, lastY, lastZ;

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

void detectaBoton(){
  if (digitalRead(PIN_BUTTON) == LOW) {
    sistemaArmado = !sistemaArmado;
    alarmaActiva = false;
    
    if (sistemaArmado) {
      Serial.println(">>> SISTEMA ARMADO");
      guardaPosicion();
      
      digitalWrite(LED_PIN, HIGH);
    } else {
      Serial.println(">>> SISTEMA DESARMADO");
      noTone(SPEAKER_PIN);
      digitalWrite(LED_PIN, LOW);
    }
    delay(500);
  }
}

void calculaMovimiento(){
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  float diffX = abs(a.acceleration.x - lastX);
  float diffY = abs(a.acceleration.y - lastY);
  float diffZ = abs(a.acceleration.z - lastZ);
  if (diffX > UMBRAL_MOVIMIENTO || diffY > UMBRAL_MOVIMIENTO || diffZ > UMBRAL_MOVIMIENTO) {
    alarmaActiva = true;
    Serial.println("¡MOVIMIENTO DETECTADO!");
  }
}

void detectaMovimiento(){
  if (sistemaArmado) {
    calculaMovimiento();
    if (alarmaActiva) {
      digitalWrite(LED_PIN, HIGH);
      tone(SPEAKER_PIN, 880);
    }
  }
}

void loop() {
  detectaBoton();
  detectaMovimiento();
  delay(100);
}