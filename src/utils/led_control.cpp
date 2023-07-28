#include <Arduino.h>
#include "led_control.hpp"

led_animation_t off(uint16_t keep_ms)
{
  led_animation_t led;
  led.type = LED_ANIMATION_TYPE_OFF;
  led.keep_ms = keep_ms;

  return led;
}

led_animation_t staticColor(uint8_t index, uint16_t hue, uint8_t sat, uint8_t val, uint16_t keep_ms)
{
  led_animation_t led;
  led.type = LED_ANIMATION_TYPE_STATIC;
  led.index = index;
  led.target_h = hue;
  led.target_s = sat;
  led.target_v = val;
  led.keep_ms = keep_ms;

  return led;
}

led_animation_t fadeAll(uint16_t hue, uint8_t sat, uint8_t val, uint16_t tick_ms, uint16_t keep_ms)
{
  led_animation_t led;
  led.type = LED_ANIMATION_TYPE_FADE;
  led.target_h = hue;
  led.target_s = sat;
  led.target_v = val;
  led.tick_ms = tick_ms;
  led.keep_ms = keep_ms;

  return led;
}

led_animation_t blinkAll(uint16_t hue, uint8_t sat, uint8_t val, uint16_t keep_ms, uint16_t blink_count)
{
  led_animation_t led;
  led.type = LED_ANIMATION_TYPE_BLINK;
  led.target_h = hue;
  led.target_s = sat;
  led.target_v = val;
  led.keep_ms = keep_ms;
  led.blink_count = blink_count;

  return led;
}