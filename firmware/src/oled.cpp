#include <Arduino.h>

#include "oled.h"
#include "battery.h"
#include "logitech.h"
#include "radio.h"
#include "radio_manager.h"

// OLED instance
U8GLIB_SH1106_128X64 u8g(OLED_CS, OLED_A0, OLED_RST);

// OLED dimensions
const int oled_width = 128;
const int oled_height = 64;

// Charging indicator (flashing battery) state
bool charging_flash_state;
uint64_t charging_flash_last_change;

// OLED refresh state
uint64_t last_oled_refresh;
const uint64_t oled_refresh_interval = 50;

// Splash state
uint64_t oled_init_time = 0;

// Initialize the OLED display
void init_oled()
{
  u8g.setRot180();
  oled_init_time = millis();
}

// Update the OLED display
void update_oled()
{
  // Limit the refresh rate
  uint64_t elapsed = millis() - last_oled_refresh;
  if(elapsed > oled_refresh_interval) last_oled_refresh = millis();
  else return;

  // Draw the display
  u8g.firstPage();
  do
  {
    // 2 second splash screen
    if((millis() - oled_init_time) < 2000)
    {
      u8g.drawXBMP(0, 0, 128, 64, logo);
    }
    else
    {
      if(radio_manager::instance().hijack_mode())
      {
        draw_hijack_mode();
      }
      else
      {
        draw_status_bar();
        draw_mouse_list();
      }
    }
  }
  while ( u8g.nextPage() );
}

// Clear the display
void draw_clear()
{

}

// Draw the list of observed mice
void draw_mouse_list()
{
  // 5x8 monospace font
  u8g.setFont(u8g_font_5x8);
  unsigned int font_width = 5;
  unsigned int font_height = 8;

  // Address buffer
  char buffer[15];

  // Step through the mice
  int mouse_count = radio_manager::instance().mouse_count();
  for(int x = 0; x < mouse_count; x++)
  {
    // Get the address
    logitech_mouse * mouse = radio_manager::instance().mouse(x);
    logitech_address addr = mouse->address;

    // Print the address
    sprintf(buffer, "%02X:%02X:%02X:%02X:%02X",
      addr.bytes[0],
      addr.bytes[1],
      addr.bytes[2],
      addr.bytes[3],
      addr.bytes[4]);
    u8g.drawStr(0, 10 + font_height + x * (font_height + 4), buffer);

    // Underline the address number if this mouse is selected
    if(mouse->selected)
    {
      uint8_t y = 10 + font_height + x * (font_height + 4) + 1;
      u8g.drawLine(0, y, 14 * font_width, y);
    }

    // Get the time elapsed since the last received frame
    uint64_t elapsed = millis() - mouse->last_packet_time();
    if(elapsed < 50 && mouse->followed) sprintf(buffer, " %2u *", mouse->channel);
    else if (mouse->followed) sprintf(buffer, " %2u ", mouse->channel);
    else if (elapsed < 50) sprintf(buffer, "    *");
    else sprintf(buffer, "");

    // Print the mouse status
    u8g.drawStr(18 * font_width + 4, 10 + font_height + x * (font_height + 4), buffer);
  }
}

// Draw the status bar
void draw_status_bar()
{
  // 4x5 monospace font
  u8g.setFont(u8g_font_micro);
  unsigned int font_width = 4;
  unsigned int font_height = 5;

  // Text buffer
  char buffer[6];

  // Draw the current radio channels
  for(int x = 0; x < radio_count; x++)
  {
    // Draw the channel number
    sprintf(buffer, "%2u", radios[x].channel());
    u8g.drawStr(x*(font_width * 2 + 4), font_height, buffer);

    // Underline the channel number if the radio is following a mouse
    if(radios[x].following())
    {
      u8g.drawLine(x*(font_width * 2 + 4), font_height + 1, x*(font_width * 2 + 4) + font_width * 2 - 2, font_height + 1);
    }
  }

  draw_voltage();
}

void draw_voltage()
{
  // Print buffers
  char buffer[6];
  char buffer2[6];

  // 4x5 monospace font
  u8g.setFont(u8g_font_micro);
  unsigned int font_width = 4;
  unsigned int font_height = 5;

  // Draw the voltage
  charging_status status = get_charging_status();
  float voltage = get_battery_voltage();
  dtostrf(voltage, 3, 2, buffer2);
  sprintf(buffer, "%sv", buffer2);
  u8g.drawStr(oled_width - 10 - (5 * font_width + 4), 5, buffer);

  // Charging - flash the plug icon on/off at a 1000ms interval
  if(status == CHARGING)
  {
    // Check if it's time to toggle the battery icon on/off
    if(millis() - charging_flash_last_change > 750)
    {
      charging_flash_last_change = millis();
      charging_flash_state = !charging_flash_state;
    }

    // Display the plug icon
    if(charging_flash_state) u8g.drawXBMP(oled_width - 10, 0, 10, 5, bmp_plug);
  }

  // Charged - display the plug icon
  else if (status == CHARGED) u8g.drawXBMP(oled_width - 10, 0, 10, 5, bmp_plug);

  // Discharging - display the battery icon (solid)
  else if (status == DISCHARGING) u8g.drawXBMP(oled_width - 10, 0, 10, 5, bmp_battery);
}

// Draw the hijack mode state
void draw_hijack_mode()
{
  draw_voltage();

  // Get the hijacked mouse
  logitech_mouse * mouse = radio_manager::instance().hijack_mouse();

  // 4x5 monospace font
  u8g.setFont(u8g_font_micro);
  unsigned int font_width = 4;
  unsigned int font_height = 5;

  // Print the address / channel
  char buffer[27];
  logitech_address addr = mouse->address;
  sprintf(buffer, "%02X:%02X:%02X:%02X:%02X  Ch.%u",
    addr.bytes[0],
    addr.bytes[1],
    addr.bytes[2],
    addr.bytes[3],
    addr.bytes[4],
    mouse->channel,
    radio_manager::instance().hijack_radio());
  u8g.drawStr(0, font_height, buffer);

  // 5x7 monospace font
  u8g.setFont(u8g_font_5x7);
  font_width = 5;
  font_height = 7;

  // Draw the status table
  uint64_t last_rx = millis() - mouse->last_packet_time_rx;
  uint64_t last_tx = millis() - mouse->last_packet_time_tx;
  uint64_t last_tune = millis() - mouse->last_tune_time;
  sprintf(buffer, "Last RX   | %lu\n", last_rx);
  u8g.drawStr(0, 10 + font_height + 0 * (font_height + 4), buffer);
  sprintf(buffer, "Last TX   | %lu\n", last_tx);
  u8g.drawStr(0, 10 + font_height + 1 * (font_height + 4), buffer);
  sprintf(buffer, "Last Tune | %lu\n", last_tune);
  u8g.drawStr(0, 10 + font_height + 2 * (font_height + 4), buffer);
}


// Here's some super hacky code to get U8glib to work on the Teensy and
// still use the platformio package manager:

/**********************************************************************
 * U8glib doesn't support HW SPI on the Teensy, so I've hacked it in. *
 * The required #defines for U8G_COM_HW_SPI and U8G_COM_ST7920_HW_SPI *
 * are passed in to gcc in platformio.ini, and the SPI handler is     *
 * implemented below.                                                 *
 **********************************************************************/

// Wrapper on SPI.transfer
static uint8_t u8g_spi_out(u8g_t *u8g, uint8_t data)
{
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  digitalWrite(u8g->pin_list[U8G_PI_CS], LOW);
  uint8_t ret = SPI.transfer(data);
  digitalWrite(u8g->pin_list[U8G_PI_CS], HIGH);
  SPI.endTransaction();
  return ret;
}

// Teensy 3.1 HW SPI handler
uint8_t u8g_com_arduino_hw_spi_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr)
{
  switch(msg)
  {
    case U8G_COM_MSG_INIT:
      SPI.begin();
      pinMode(u8g->pin_list[U8G_PI_CS], OUTPUT);
      pinMode(u8g->pin_list[U8G_PI_A0], OUTPUT);
      pinMode(u8g->pin_list[U8G_PI_RESET], OUTPUT);
      digitalWrite(u8g->pin_list[U8G_PI_CS], HIGH);
      digitalWrite(u8g->pin_list[U8G_PI_A0], HIGH);
      digitalWrite(u8g->pin_list[U8G_PI_RESET], HIGH);
      break;

    case U8G_COM_MSG_ADDRESS:
      digitalWrite(u8g->pin_list[U8G_PI_A0], arg_val);
      break;

    case U8G_COM_MSG_CHIP_SELECT:
      digitalWrite(u8g->pin_list[U8G_PI_CS], arg_val == 0 ? HIGH : LOW);
      break;

    case U8G_COM_MSG_RESET:
      digitalWrite(u8g->pin_list[U8G_PI_RESET], arg_val);
      break;

    case U8G_COM_MSG_WRITE_BYTE:
      u8g_spi_out(u8g, arg_val);
      break;

    case U8G_COM_MSG_WRITE_SEQ:
      {
        register uint8_t *ptr = (uint8_t *)arg_ptr;
        while( arg_val > 0 )
        {
          u8g_spi_out(u8g, *ptr++);
          arg_val--;
        }
      }
      break;

    case U8G_COM_MSG_WRITE_SEQ_P:
      {
        register uint8_t *ptr = (uint8_t *)arg_ptr;
        while( arg_val > 0 )
        {
          u8g_spi_out(u8g, u8g_pgm_read(ptr));
          ptr++;
          arg_val--;
        }
      }
      break;
  }
  return 1;
}
