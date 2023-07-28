#ifndef _LED_CONTROL_HPP_INCLUDED_
#define _LED_CONTROL_HPP_INCLUDED_

#define LED_ANIMATION_TYPE_OFF 0x00
#define LED_ANIMATION_TYPE_STATIC 0x01
#define LED_ANIMATION_TYPE_FADE 0x02
#define LED_ANIMATION_TYPE_BLINK 0x03

struct led_animation_t
{
  uint8_t type;

  uint8_t index;
  uint16_t target_h;
  uint8_t target_s;
  uint8_t target_v;

  uint16_t tick_ms;
  uint16_t keep_ms;

  uint16_t blink_count;
};

led_animation_t off(uint16_t keep_ms);
led_animation_t staticColor(uint8_t index, uint16_t hue, uint8_t sat, uint8_t val, uint16_t keep_ms);
led_animation_t fadeAll(uint16_t hue, uint8_t sat, uint8_t val, uint16_t tick_ms, uint16_t keep_ms);
led_animation_t blinkAll(uint16_t hue, uint8_t sat, uint8_t val, uint16_t keep_ms, uint16_t blink_count);

#endif