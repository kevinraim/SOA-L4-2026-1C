#include "events.h"

// --- Variables globales compartidas ---
Estado           estadoActual       = APAGADO;
Evento           newEvent;
short            lastIndexTypeSensor = 0;
Adafruit_MPU6050 mpu;
float            lastX, lastY, lastZ;

// Botón: semáforo con tarea FreeRTOS
static SemaphoreHandle_t xBotonSemaphore      = nullptr;
static volatile bool     botonEventoPendiente = false;

static void isrBoton() {
  xSemaphoreGiveFromISR(xBotonSemaphore, nullptr);
}

static void tareaBoton(void* pvParameters) {
  while (1) {
    xSemaphoreTake(xBotonSemaphore, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_MS));
    if (digitalRead(PIN_BUTTON) == LOW) {
      botonEventoPendiente = true;
    }
  }
}

static bool detectaBotonEvent() {
  if (botonEventoPendiente) {
    botonEventoPendiente = false;
    newEvent = (estadoActual == APAGADO) ? PRENDER : APAGAR;
    return true;
  }
  return false;
}

// Movimiento: MPU6050 (acelerometro)
static bool detectaMovimientoBrusco() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  float diffX = abs(a.acceleration.x - lastX);
  float diffY = abs(a.acceleration.y - lastY);
  float diffZ = abs(a.acceleration.z - lastZ);
  return (diffX > UMBRAL_MOVIMIENTO || diffY > UMBRAL_MOVIMIENTO || diffZ > UMBRAL_MOVIMIENTO);
}

static bool detectaMovimientoEvent() {
  if ((estadoActual == ACTIVO || estadoActual == ADVERTENCIA_CONTACTO) && detectaMovimientoBrusco()) {
    newEvent = MOV_DETECTADO;
    return true;
  }
  return false;
}

// Touch: potenciómetro (imita sensor Hall)
static bool detectaTouchEvent() {
  int potValue = analogRead(PIN_POTENCIOMETRO);
  if ((estadoActual == ACTIVO || estadoActual == ADVERTENCIA_MOVIMIENTO) && potValue > UMBRAL_TOUCH) {
    newEvent = TOUCH_DETECTADO;
    return true;
  }
  return false;
}

// Timeout de advertencia: timeout con FreeRTOS
static TimerHandle_t     xTimerAdvertencia          = nullptr;
static volatile bool     timeoutAdvertenciaPendiente = false;

void iniciarTimeoutAdvertencia() {
  xTimerReset(xTimerAdvertencia, 0);
}

void cancelarTimeoutAdvertencia() {
  xTimerStop(xTimerAdvertencia, 0);
  timeoutAdvertenciaPendiente = false;
}

static bool detectaTimeoutAdvertencia() {
  if (timeoutAdvertenciaPendiente) {
    timeoutAdvertenciaPendiente = false;
    newEvent = TIMEOUT_ADVERTENCIA;
    return true;
  }
  return false;
}

// Tabla de detectores
EventDetector eventType[MAX_TYPE_EVENTS] = {
  detectaBotonEvent,
  detectaMovimientoEvent,
  detectaTouchEvent,
  detectaTimeoutAdvertencia
};

// Inicializar tareas de freertos
void initEventDetectors() {
  xBotonSemaphore   = xSemaphoreCreateBinary();
  xTimerAdvertencia = xTimerCreate("tAdvertencia", pdMS_TO_TICKS(TIMEOUT_ADVERTENCIA_MS),
                                   pdFALSE, nullptr,
                                   [](TimerHandle_t) { timeoutAdvertenciaPendiente = true; });
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON), isrBoton, FALLING);
  xTaskCreate(tareaBoton, "tareaBoton", 2048, nullptr, 1, nullptr);
}

bool getNuevoEvento() {
  short index = lastIndexTypeSensor % MAX_TYPE_EVENTS;
  lastIndexTypeSensor++;
  return eventType[index]();
}
