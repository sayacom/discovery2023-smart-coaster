#include <Arduino.h>
#include <FreeRTOS.h>
#include <freertos/queue.h>
#include "task.hpp"

#include "../utils/led_control.hpp"

#define NUMPIXELS 3

void ledControlTask(void *pvParameters)
{
  led_animation_t t, current_t;

  while (1)
  {
    xQueueReceive(ledControlQueue, &t, portMAX_DELAY);

    if (t.type == LED_ANIMATION_TYPE_OFF)
    {
      coasterLeds.clear();
      coasterLeds.show();
    }
    else if (t.type == LED_ANIMATION_TYPE_STATIC)
    {
      t.target_h = (uint16_t)(t.target_h / 1000) * 1000;
      t.target_s = (uint8_t)(t.target_s / 10) * 10;
      t.target_v = (uint8_t)(t.target_v / 10) * 10;

      coasterLeds.clear();
      uint32_t color = coasterLeds.ColorHSV(t.target_h, t.target_s, t.target_v);
      coasterLeds.setPixelColor(t.index, color);
      coasterLeds.show();

      current_t.target_h = t.target_h;
      current_t.target_s = t.target_s;
      current_t.target_v = t.target_v;
    }
    else if (t.type == LED_ANIMATION_TYPE_FADE)
    {
      t.target_h = (uint16_t)(t.target_h / 1000) * 1000;
      t.target_s = (uint8_t)(t.target_s / 10) * 10;
      t.target_v = (uint8_t)(t.target_v / 10) * 10;
      // Serial.print(">>> Hue: "); Serial.println(t.target_h);

      // Serial.println(">>> Output leds...");
      while (1)
      {
        uint16_t h = current_t.target_h;
        if (t.target_h > current_t.target_h)
        {
          h = current_t.target_h + 100;
        }
        else if (t.target_h < current_t.target_h)
        {
          h = current_t.target_h - 100;
        }

        uint8_t s = current_t.target_s;
        if (t.target_s > current_t.target_s)
        {
          s = current_t.target_s + 10;
        }
        else if (t.target_s < current_t.target_s)
        {
          s = current_t.target_s - 10;
        }

        uint8_t v = current_t.target_v;
        if (t.target_v > current_t.target_v)
        {
          v = current_t.target_v + 10;
        }
        else if (t.target_v < current_t.target_v)
        {
          v = current_t.target_v - 10;
        }

        uint32_t color = coasterLeds.ColorHSV(h, s, v);
        for (int i = 0; i < NUMPIXELS; i++)
        {
          coasterLeds.setPixelColor(i, color);
        }

        coasterLeds.show();

        current_t.target_h = h;
        current_t.target_s = s;
        current_t.target_v = v;
        vTaskDelay(t.tick_ms);

        if (current_t.target_h == t.target_h && current_t.target_s == t.target_s && current_t.target_v == t.target_v)
        {
          break;
        }
      }
    }
    else if (t.type == LED_ANIMATION_TYPE_BLINK)
    {
      t.target_h = (uint16_t)(t.target_h / 1000) * 1000;
      t.target_s = (uint8_t)(t.target_s / 10) * 10;
      t.target_v = (uint8_t)(t.target_v / 10) * 10;
      uint32_t color = coasterLeds.ColorHSV(t.target_h, t.target_s, t.target_v);

      coasterLeds.clear();

      for (int i = 0; i < t.blink_count; i++)
      {
        // ON as specified color
        for (int j = 0; j < NUMPIXELS; j++)
        {
          coasterLeds.setPixelColor(j, color);
        }
        coasterLeds.show();
        vTaskDelay(t.keep_ms);

        // OFF
        coasterLeds.clear();
        coasterLeds.show();
        vTaskDelay(t.keep_ms);
      }

      current_t.target_h = t.target_h;
      current_t.target_s = t.target_s;
      current_t.target_v = t.target_v;
    }

    // Serial.println(">>> LED Output done.");
    vTaskDelay(t.keep_ms);
  }
}
