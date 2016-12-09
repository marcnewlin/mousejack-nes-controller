#pragma once

#include <Arduino.h>

// Initialize the NES buttons
void init_buttons();

// Update the button states
void update_buttons();

// Buttons
enum nes_button
{
  NES_UP     = 0,
  NES_SELECT = 1,
  NES_RIGHT  = 2,
  NES_LEFT   = 3,
  NES_A      = 4,
  NES_DOWN   = 5,
  NES_START  = 6,
  NES_B      = 7
};

// Button click flags
extern volatile uint8_t button_clicks[8];

// Button tick flags
extern volatile uint16_t button_ticks[8];
