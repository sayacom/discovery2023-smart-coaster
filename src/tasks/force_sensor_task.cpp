#include <Arduino.h>
#include <FreeRTOS.h>
#include <freertos/queue.h>
#include <AceSorting.h>
#include "task.hpp"
#include "../utils/led_control.hpp"

using ace_sorting::shellSortKnuth;

#define SAMPLING_TICK_MS 50
#define SENSING_ARRAY_SIZE 20
#define SENSING_HISTORY_ARRAY_SIZE 15
void forceSensorTask(void *pvParameters)
{
  int sensorValues[SENSING_ARRAY_SIZE] = {0};
  int sensorValuesHistory[SENSING_HISTORY_ARRAY_SIZE] = {0};

  uint8_t sensorValueIndex = 0, sensorValuesHistoryIndex = 0;

  int forceSensorRawValue = 0;
  while (1)
  {
    led_animation_t led;
    forceSensorRawValue = 4096 - analogRead(FORCE_SENSOR);

    // Turn off LEDs when sensor no load (also exclude to send data)
    if (forceSensorRawValue <= SENSOR_CUTOFF_MIN_VALUE)
    {
      led = staticColor(1, DEFAULT_LED_HUE, 255, 192, 0);
      xQueueSend(ledControlQueue, &led, 0);

      vTaskDelay(SAMPLING_TICK_MS);
      continue;
    }

    sensorValues[sensorValueIndex] = forceSensorRawValue;

    uint16_t mappedHue = map(forceSensorRawValue, SENSOR_CUTOFF_MIN_VALUE, SENSOR_CUTOFF_MAX_VALUE, 0, 65535);
    led = fadeAll(mappedHue, 255, 192, 0, 0);

    xQueueSend(ledControlQueue, &led, 0);
    Serial.println(forceSensorRawValue);

    sensorValueIndex++;
    if (sensorValueIndex >= SENSING_ARRAY_SIZE)
    {
      shellSortKnuth(sensorValues, SENSING_ARRAY_SIZE);
      int median_short = sensorValues[SENSING_ARRAY_SIZE / 2];

      // Set long-term sampling values
      sensorValuesHistory[sensorValuesHistoryIndex] = median_short;
      sensorValuesHistoryIndex++;

      // Sort long-term values (window period), and send
      if (sensorValuesHistoryIndex >= SENSING_HISTORY_ARRAY_SIZE)
      {
        shellSortKnuth(sensorValuesHistory, SENSING_HISTORY_ARRAY_SIZE);
        int median_long = sensorValuesHistory[SENSING_HISTORY_ARRAY_SIZE / 2];

        xQueueSend(sensorValueQueue, &median_long, 0);
        sensorValuesHistoryIndex = 0;
      }

      sensorValueIndex = 0;
    }

    vTaskDelay(SAMPLING_TICK_MS);
  }
}
