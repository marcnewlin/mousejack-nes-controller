#pragma once

#include <Arduino.h>
#define printf Serial.printf

/****************
 * Radio Config *
 ****************/

// Logitech channel count
const uint8_t channel_count = 24;

// Logitech channel numbers
const uint8_t channels[channel_count] = { 5, 8, 11, 14, 17, 20, 23, 26, 29, 32, 35, 38, 41, 44, 47, 50, 53, 56, 59, 62, 65, 68, 71, 74 };

// Logitech ESB address length
const uint8_t address_length = 5;

// Radio count and CE/CS pins
const uint8_t radio_count = 5;
const uint8_t radio_pins[radio_count][2] = { {17, 16}, {19, 18}, {4, 3}, {5, 6}, {8, 7} };

/**********************
 * NES Buttons Config *
 **********************/

// Button pins
const uint8_t nes_button_count = 8;
const uint8_t nes_button_pins[nes_button_count] = { 25, 26, 27, 28, 29, 30, 31, 32 };

// NES button indexes
#include "nes_buttons.h"
const nes_button nes_button_lookup[nes_button_count] =
  { NES_UP, NES_SELECT, NES_RIGHT, NES_LEFT, NES_A, NES_DOWN, NES_START, NES_B };


/**************************
 * Voltage Divider Config *
 **************************/

#define VDIV1 A7
#define VDIV2 A8
#define VDIV3 A8


/******************
 * microSD Config *
 ******************/

#define SD_CS 20


/***************
 * OLED Config *
 ***************/

#define OLED_CS 2
#define OLED_A0 1
#define OLED_RST 0