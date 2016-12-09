#pragma once

#include "nRF24L01P.h"
#include "logitech.h"
#include "config.h"

// Radios
extern class radio radios[radio_count];

// Radio
class radio
{
public:

  // Constructor
  radio(unsigned char ce_pin, unsigned char cs_pin);
  radio(){};

  // Put the radio in promiscuous mode
  void enter_promiscuous_mode();

  // Put the radio in ESB mode
  // - default timeout is 1ms (4 * 250us)
  // - default number of retries is 5
  void enter_esb_tx_mode(uint8_t * address, uint8_t timeout=4, uint8_t retries=5);

  // Put the radio in follow mode
  void enter_sniffer_mode(uint8_t * address);

  // Read and parse the status register
  nrf24_status get_status();

  // Read a promiscuous mode payload (fixed at 32 bytes)
  void read_payload_fixed(uint8_t * buff);

  // Read an ESB mode payload (variable 1-32 bytes)
  void read_payload_dynamic(uint8_t * buff, uint8_t & length);

  // Transmit a dynamic length payload
  // - default timeout is 6000us
  bool transmit_payload(uint8_t * payload, uint8_t length, uint8_t timeout=1, uint8_t retransmits=5, char * message = "");

  // Transmit a dynamic length payload in soft-ESB mode with a hijacked mouse
  bool transmit_payload_hijacked(uint8_t * payload, uint8_t length);

  // Channel getter/setter
  inline unsigned char channel() { return m_channel; }
  inline void channel(unsigned char channel) { set_channel(channel); }

  // Following getter/setter
  inline void following(bool following) { m_following = following; }
  inline bool following() { return m_following; }

  // Current mouse getter
  inline logitech_mouse * mouse() { return m_mouse; }

  // Set the mouse to follow
  void follow_mouse(logitech_mouse * mouse);

  // Update the channel if needed
  void update_following_channel();

private:

  // Current channel
  unsigned char m_channel;

  // Following/promiscuous flag
  bool m_following;

  // arget mouse (when following)
  logitech_mouse * m_mouse;

  // SPI settings
  SPISettings m_spi_settings; 

  // CE/CS pins
  unsigned char m_ce_pin;
  unsigned char m_cs_pin;

  // Read some bytes over SPI
  void spi_read(uint8_t command, uint8_t * buffer, uint8_t length);

  // Write some bytes over SPI
  void spi_write(uint8_t command, uint8_t * buffer, uint8_t length);

  // Read multiple bytes from a register
  void read_register(uint8_t reg, uint8_t * buffer, uint8_t length);

  // Write multiple bytes to a register
  void write_register(uint8_t reg, uint8_t * buffer, uint8_t length);

  // Read a single byte form a register
  void read_register(uint8_t reg, uint8_t & byte);

  // Write a single byte to a register
  void write_register(uint8_t reg, uint8_t byte);

  // Flush the RX FIFO buffer
  void flush_rx();

  // Flush the TX FIFO buffer
  void flush_tx();

  // Tune the radio
  void set_channel(unsigned char ch);  
};

