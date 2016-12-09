#include "nes_buttons.h"
#include "config.h"

// Button states 
bool button_states[nes_button_count]; 

// Button clicks
volatile uint8_t button_clicks[nes_button_count];

// Button depressed ticks
volatile uint16_t button_ticks[nes_button_count];

// Last button depress times
uint64_t button_depress_times[nes_button_count];
uint64_t button_tick_times[nes_button_count];

// Button tick interval
const uint16_t tick_interval = 25;

// Buttom update timer
IntervalTimer button_timer;

// Initialize the NES buttons
void init_buttons()
{
  for(int x = 0; x < nes_button_count; x++)
  {
    pinMode(nes_button_pins[x], INPUT_PULLUP);
    button_ticks[x] = 0;
    button_clicks[x] = 0;
    button_states[x] = false;
  }
  button_timer.begin(update_buttons, 1000 * 1);
}

// Handle a button click
void click(uint8_t button)
{
  button_clicks[button]++;
}

// Update the button states
void update_buttons()
{
  bool current_states[nes_button_count] = {false};

  // Get the button states
  for(int x = 0; x < nes_button_count; x++)
  {
    if(digitalRead(nes_button_pins[x]) == LOW)
    {
      current_states[x] = true;
    }
  }

  // Flag clicks
  for(int x = 0; x < nes_button_count; x++)
  {
    if(button_states[x] && !current_states[x])    
    {
      click(nes_button_lookup[x]);
    }
  }

  // Flag ticks
  for(int x = 0; x < nes_button_count; x++)
  {
    // Already depressed
    if(button_states[x] && current_states[x])    
    {
      uint64_t elapsed = millis() - button_depress_times[x];
      uint64_t last_tick = millis() - button_tick_times[x];

      if(elapsed > tick_interval && last_tick > tick_interval)
      {
        last_tick = millis();
        button_ticks[x]++;
      }
    }

    // Newly depressed
    else if(button_states[x] == false && current_states[x] == true)    
    {
      button_depress_times[x] = millis();
    }
  }  

  for(int x = 0; x < nes_button_count; x++) button_states[x] = current_states[x];
}
