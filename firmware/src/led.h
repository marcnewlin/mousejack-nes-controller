#pragma once

#include <FastLED.h>

// LED data pin
#define LED_DATA_PIN 15

// LED refresh rate (Hz)
#define LED_REFRESH_RATE 200

// Initialize the LED
void init_led();

// Set the persistent LED color
void set_led_color(uint8_t r, uint8_t g, uint8_t b);

// Flash the LED with a given color for a given duration
void flash_led(uint8_t r, uint8_t g, uint8_t b, int duration_ms);

// Update the current LED state
void update_led();

// LED state
struct _led_state
{
  // LED array referenced by FastLED
  CRGB leds[1];

  // Persistent color
  CRGB persistent_color; 

  // Temporary color
  CRGB temporary_color; 

  // Temporary color duration 
  int temporary_color_duration;

  // Temporary color start time
  int temporary_color_start;
};
extern _led_state led_state;
