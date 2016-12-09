#include <Arduino.h>
#include <SPI.h>

#include "config.h"
#include "led.h"
#include "oled.h"
#include "radio_manager.h"
#include "battery.h"
#include "sd.h"
#include "nes_buttons.h"

// Initialization method
void setup()
{
  Serial.begin(115200);
  SPI.begin();

  init_oled();
  init_battery();
  init_led();
  init_sd();
  init_buttons();
}

// Main loop
void loop()
{
  radio_manager::instance().tick();
  update_oled();
}
