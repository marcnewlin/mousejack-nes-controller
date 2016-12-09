#include "led.h"

// LED state info
_led_state led_state;

// LED interrupt timer
IntervalTimer led_timer;

// Initialize the LED
void init_led()
{
  FastLED.addLeds<NEOPIXEL, LED_DATA_PIN>(led_state.leds, 1);
  set_led_color(0, 0, 0);
  led_timer.begin(update_led, 1000 * 1000 / LED_REFRESH_RATE);
}

// Set the persistent LED color
void set_led_color(uint8_t r, uint8_t g, uint8_t b)
{
  led_state.persistent_color = CRGB(r, g, b);
}

// Flash the LED with a given color for a given duration
void flash_led(uint8_t r, uint8_t g, uint8_t b, int duration_ms)
{
  led_state.temporary_color = CRGB(r, g, b);
  led_state.temporary_color_start = millis();
  led_state.temporary_color_duration = duration_ms;
}

// Update the current LED state
void update_led()
{
  int temporary_color_elapsed = millis() - led_state.temporary_color_start;

  if(temporary_color_elapsed > led_state.temporary_color_duration)
  {
    led_state.leds[0] = led_state.persistent_color;    
  }

  if(temporary_color_elapsed < led_state.temporary_color_duration)
  {
    led_state.leds[0] = led_state.temporary_color;     
  }

  FastLED.show();
}
