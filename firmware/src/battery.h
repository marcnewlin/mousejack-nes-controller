#pragma once

#include "config.h"

// Charging status
enum charging_status
{
  CHARGING,
  CHARGED,
  DISCHARGING,
  UNKNOWN,
};

// Get the charging status
charging_status get_charging_status();

// Initialize ADC settings for the battery status
void init_battery();

// Get the battery voltage
float get_battery_voltage();
