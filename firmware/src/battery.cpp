#include <Arduino.h>

#include "battery.h"

// Initialize ADC settings for the battery status
void init_battery()
{
  analogReference(DEFAULT);
  analogReadResolution(12);
  analogReadAveraging(32);
}

// Get the charging status
charging_status get_charging_status()
{
  // Read the charging status pins from the charge controller
  float v1 = (float)(analogRead(VDIV2) * 2) / 1245;
  float v2 = (float)(analogRead(VDIV3) * 2) / 1245;

  // Determine the charge status
  if(v1 < 1 && v2 < 1) return DISCHARGING;
  else if (v1 > 2 && v2 < 2) return CHARGED;
  else if (v1 < 2 && v2 > 2) return CHARGING;
  return UNKNOWN;
}

// Get the battery voltage
float get_battery_voltage()
{
  return (float)(analogRead(VDIV1) * 2) / 1245;
}
