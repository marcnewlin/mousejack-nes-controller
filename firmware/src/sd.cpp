#include <SD.h>

#include "config.h"
#include "sd.h"
#include "led.h"

// Initialize the SD card
void init_sd()
{
  SD.begin(SD_CS);

  // Create the mouse movement log directory
  if(!SD.exists("/logs"))
  {
    printf("creating /logs\n");
    SD.mkdir("/logs");
  }

  // List mouse log files
  File log_root = SD.open("/logs");
  File entry = log_root.openNextFile();
  while(entry)
  {
    printf("Found Log File %s\n", entry.name());
    entry = log_root.openNextFile();
  }
}

// Create a new mouse log file
void create_log(uint8_t address[5])
{
  char filename[23];
  sprintf(filename, "/logs/%02X%02X%02X%02X.%02X", address[4], address[3], address[2], address[1], address[0]);
  if(!SD.exists(filename))
  {
    File file = SD.open(filename, FILE_WRITE);
    file.close();
  }
}

// Append a 9 byte mouse frame to the log file
void append_log(uint8_t address[5], uint8_t payload[9])
{
  char filename[23];
  sprintf(filename, "/logs/%02X%02X%02X%02X.%02X", address[4], address[3], address[2], address[1], address[0]);

  File file = SD.open(filename, FILE_WRITE);
  if(file.write(payload, 9) == 9)
  {
    // flash purple to indicate write success
    flash_led(10, 0, 10, 10);
  }
  else
  {
    // flash red to indicate write failure
    flash_led(10, 0, 0, 10);
  }
  file.close();
}

// Dump a log file over serial
void dump_log(uint8_t address[5])
{
  char filename[23];
  sprintf(filename, "/logs/%02X%02X%02X%02X.%02X", address[4], address[3], address[2], address[1], address[0]);

  File file = SD.open(filename);
  uint64_t index = 0;
  while(file.seek(index++))
  {
    Serial.write(file.read());
  }
  file.close();
}
