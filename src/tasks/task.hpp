#ifndef _TASK_HPP_INCLUDED_
#define _TASK_HPP_INCLUDED_

#include <FreeRTOS.h>
#include <freertos/queue.h>
#include <WireGuard-ESP32.h>
#include <Adafruit_NeoPixel.h>

#define FORCE_SENSOR A9
#define COASTER_LED D10

#define NUMPIXELS 3

extern WireGuard wg;
extern QueueHandle_t sensorValueQueue;
extern QueueHandle_t ledControlQueue;
extern Adafruit_NeoPixel coasterLeds;
extern int SENSOR_CUTOFF_MIN_VALUE;
extern int SENSOR_CUTOFF_MAX_VALUE;
extern int DEFAULT_LED_HUE;

void ledControlTask(void *pvParameters);
void forceSensorTask(void *pvParameters);
void sendDataTask(void *pvParameters);
void getMetadataTask(void *pvParameters);

#endif