#pragma once

#include <SPI.h>

#include "config.h"
#include "logitech.h"
#include "radio.h"

const uint32_t sniffer_timeout_ms = 2000;
const uint32_t sniffer_sweep_timeout_ms = 1100;

class radio_manager
{
public:

  // Singleton
  static radio_manager & instance()
  {
    static radio_manager instance;
    return instance;
  }
  radio_manager(radio_manager const&);
  void operator=(radio_manager const&);

  // Perform requisite actions on each radio
  void tick();

  // Mouse getters
  inline logitech_mouse * mouse(int index) { return index < m_mouse_count ? m_mice[index] : NULL; }
  inline int mouse_count() { return m_mouse_count; }

  // Hijack state getters
  inline bool hijack_mode() { return m_hijack_mode; }
  inline uint8_t hijack_radio() { return m_hijack_radio; }
  inline logitech_mouse * hijack_mouse() { return m_hijack_mouse; }

private:

  // Update promiscuous/following modes
  void update_radio_modes();

  // Process button presses
  void process_button_presses();

  // Tune radios
  void tune_radios();

  // Process received payloads
  void process_payloads();

  // Process a promiscuous mode payload
  void process_payload_promiscuous(uint8_t * payload, uint8_t channel);

  // Process a sniffer mode payload
  void process_payload_sniffer(uint8_t address[address_length], uint8_t * payload, uint8_t length, uint8_t channel);

  // Enter hijack mode
  void enter_hijack_mode(logitech_mouse * mouse);

  // Hijack mode loop
  void hijack_loop();

  // Enter normal mode
  void enter_normal_mode();

  // Next channel index
  unsigned char m_next_channel_index;

  // CRC computed over the address for the most recent
  // mouse followed by each radio
  uint16_t m_mouse_address_crc[radio_count];

  // Observed Logitech mice
  static const int m_max_mice = 50;
  logitech_mouse * m_mice[m_max_mice];
  int m_mouse_count;

  // Hijacked radio state
  bool m_hijack_mode;
  uint8_t m_hijack_radio;
  logitech_mouse * m_hijack_mouse;

  // Mouse right/left button states
  bool m_left_down;
  bool m_right_down;

  // Last transmitted payload
  uint8_t m_last_tx_payload[32];
  uint8_t m_last_tx_payload_length;

  // Add a mouse in the observed mice list
  void add_mouse(uint8_t address[address_length], uint8_t channel);

  // Check if an address is a known mouse
  bool known_mouse(uint8_t address[address_length]);

  // Update the current channel of a mouse
  void update_mouse(uint8_t address[address_length], uint8_t channel, bool tx=false);

  // Constructor
  radio_manager();

  // Time of last search retune
  uint64_t m_last_retune;

  // Dwell time
  int m_dwell_time;

  // Process a crc byte (or partial byte)
  uint16_t crc_update (uint16_t crc, uint8_t data, uint8_t bits=8);
};
