#pragma once

// Initialize the SD card
void init_sd();

// Create a new mouse log file
void create_log(uint8_t address[5]);

// Append a 9 byte mouse frame to the log file
void append_log(uint8_t address[5], uint8_t payload[9]);

// Write a log file's contents over seial
void dump_log(uint8_t address[5]);
