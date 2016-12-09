#include <SPI.h>

#include "radio.h"
#include "nRF24L01P.h"
#include "radio_manager.h"

// Radios
radio radios[radio_count];

// Constructor
radio::radio(unsigned char ce_pin, unsigned char cs_pin) : 
  m_ce_pin(ce_pin),
  m_cs_pin(cs_pin),
  m_spi_settings(10000000, MSBFIRST, SPI_MODE0),
  m_channel(0),
  m_following(false)
{
  // Setup the CS/CS pins
  pinMode(m_ce_pin, OUTPUT);
  pinMode(m_cs_pin, OUTPUT);
  digitalWrite(m_ce_pin, LOW);
  digitalWrite(m_cs_pin, HIGH);
}

// Read some bytes over SPI
void radio::spi_write(uint8_t command, uint8_t * buffer, uint8_t length)
{
  SPI.beginTransaction(m_spi_settings);
  digitalWrite(m_cs_pin, LOW);
  SPI.transfer(command);
  for(int x = 0; x < length; x++) SPI.transfer(buffer[x]);
  digitalWrite(m_cs_pin, HIGH);
  SPI.endTransaction();
}

// Write some bytes over SPI
void radio::spi_read(uint8_t command, uint8_t * buffer, uint8_t length)
{
  SPI.beginTransaction(m_spi_settings);
  digitalWrite(m_cs_pin, LOW);
  SPI.transfer(command);
  for(int x = 0; x < length; x++) buffer[x] = SPI.transfer(_NOP);
  digitalWrite(m_cs_pin, HIGH);
  SPI.endTransaction();
}

// Read a single byte from a register
void radio::read_register(uint8_t reg, uint8_t & byte) 
{ 
  read_register(reg, &byte, 1); 
}

// Write a single byte to a register
void radio::write_register(uint8_t reg, uint8_t byte) 
{ 
  write_register(reg, &byte, 1); 
}

// Read multiple bytes from a register
void radio::read_register(uint8_t reg, uint8_t * buffer, uint8_t length)
{
  spi_read(R_REGISTER | reg, buffer, length);
}

// Write multiple bytes to a register
void radio::write_register(uint8_t reg, uint8_t * buffer, uint8_t length)
{
  spi_write(W_REGISTER | reg, buffer, length);
}

// Flush the RX FIFO buffer
void radio::flush_rx()
{
  spi_write(FLUSH_RX, NULL, 0);
}

// Flush the RX FIFO buffer
void radio::flush_tx()
{
  spi_write(FLUSH_TX, NULL, 0);
}


// Read and parse the status register
nrf24_status radio::get_status()
{
  SPI.beginTransaction(m_spi_settings);
  digitalWrite(m_cs_pin, LOW);
  nrf24_status status = nrf24_status(SPI.transfer(_NOP));
  digitalWrite(m_cs_pin, HIGH);
  SPI.endTransaction();
  return status; 
}

// Read a promiscuous mode payload (fixed at 32 bytes)
void radio::read_payload_fixed(uint8_t * buff)
{
  SPI.beginTransaction(m_spi_settings);
  digitalWrite(m_cs_pin, LOW);
  SPI.transfer(R_RX_PAYLOAD);
  for(int x = 0; x < 32; x++)
  {
    buff[x] = SPI.transfer(_NOP);
  }
  digitalWrite(m_cs_pin, HIGH);
  SPI.endTransaction();
}

// Read an ESB mode payload (variable 1-32 bytes)
void radio::read_payload_dynamic(uint8_t * buff, uint8_t & length)
{
  // Clear the RX fifo interrupt
  write_register(STATUS, RX_DR);

  // Get the payload length
  unsigned char bytes[1];
  spi_read(R_RX_PL_WID, bytes, 1);
  length = bytes[0];
  
  if(length > 32)
  {
    printf("invalid length: %i\n", length);
    length = 0;
    flush_rx();
    return;
  }

  // Read the payload
  SPI.beginTransaction(m_spi_settings);
  digitalWrite(m_cs_pin, LOW);
  SPI.transfer(R_RX_PAYLOAD);
  for(int x = 0; x < length; x++)
  {
    buff[x] = SPI.transfer(_NOP);
  }
  digitalWrite(m_cs_pin, HIGH);
  SPI.endTransaction();   
}

// Transmit a dynamic length payload
bool radio::transmit_payload(uint8_t * payload, uint8_t length, uint8_t timeout, uint8_t retransmits, char * message)
{
  digitalWrite(m_ce_pin, LOW);

  // Setup auto-retransmit
  // - timeout is in multiples of 250us
  uint8_t retr = 1 << 4 | 5;
  write_register(SETUP_RETR, retr);

  // Flush the TX/RX buffers
  flush_tx();
  flush_rx();

  // Clear the max retries and data sent flags
  write_register(STATUS, MAX_RT | TX_DS | RX_DR);

  // Enable 16 bit CRC, enable TX, power up
  write_register(CONFIG, PWR_UP | EN_CRC | CRCO); 

  // Write the payload
  spi_write(W_TX_PAYLOAD, payload, length);

  // Enable dynamic payload length and automatic ACK handling
  write_register(FEATURE, EN_DPL | EN_DYN_ACK);
  write_register(EN_AA, ENAA_P0);

  // Bring CE high for 10us to initiate the transfer
  digitalWrite(m_ce_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(m_ce_pin, LOW);

  // Wait for success, failure, or timeout
  uint64_t start = micros();
  bool success = true;
  while(true)
  {
    nrf24_status status = get_status();
    if(status.max_tx_retransmits)  
    {
      printf("[%lu] [%s] TX Failure, Max Retransmits Reached\n", millis(), message);
      success = false;
      break;
    }
    if(status.tx_data_sent)
    {
      printf("[%lu] [%s] TX Successful\n", millis(), message);
      success = true;
      break;
    }
  }

  // Disable auto ack
  write_register(FEATURE, EN_DPL);
  write_register(EN_AA, ENAA_NONE);  

  // Enable 16 bit CRC, enable RX, power up
  write_register(CONFIG, PWR_UP | EN_CRC | CRCO | PRIM_RX);   

  digitalWrite(m_ce_pin, HIGH);

  return success;
}

// Put the radio in mouse sniffer mode
void radio::enter_sniffer_mode(uint8_t * address)
{
  digitalWrite(m_ce_pin, LOW);

  // Enable RX pipe 0
  write_register(EN_RXADDR, ENRX_P0);

  // Enable 5 byte addresses
  write_register(SETUP_AW, AW_5);

  // Set the addresses
  write_register(TX_ADDR, address, 5);
  write_register(RX_ADDR_P0, address, 5);

  // Enable dynamic payload length
  write_register(FEATURE, EN_DPL);
  write_register(DYNPD, DPL_P0);

  // Disable auto-acking 
  write_register(EN_AA, ENAA_NONE);

  // Enable 16 bit CRC, enable RX, power up
  write_register(CONFIG, PWR_UP | EN_CRC | CRCO | PRIM_RX); 

  // Set the data rate to 2Mbps, max transmit power
  write_register(RF_SETUP, RATE_2M | RF_PWR_4);

  // Tune the radio
  set_channel(m_channel);

  digitalWrite(m_ce_pin, HIGH);  
}

// Put the radio in promiscuous mode
void radio::enter_promiscuous_mode()
{
  digitalWrite(m_ce_pin, LOW);

  // Enable RX pipes 0 and 1
  write_register(EN_RXADDR, ENRX_P0 | ENRX_P1);

  // Enable 2 byte addresses
  write_register(SETUP_AW, AW_2);

  // Set the RX addresses
  uint8_t addresses[2][2] = { {0xAA, 0x00}, {0x55, 0x00} };
  write_register(RX_ADDR_P0, addresses[0], 2);
  write_register(RX_ADDR_P1, addresses[1], 2);

  // Set the RX payload widths to 32
  write_register(RX_PW_P0, 32);
  write_register(RX_PW_P1, 32);

  // Disable dynamic payload length and automatic ACK handling
  write_register(FEATURE, 0);
  write_register(DYNPD, 0);
  write_register(EN_AA, ENAA_NONE);

  // Disable CRC, enable RX, power up
  write_register(CONFIG, PRIM_RX | PWR_UP); 

  // Set the data rate to 2Mbps, max transmit power
  write_register(RF_SETUP, RATE_2M | RF_PWR_4);

  // Tune the radio
  set_channel(m_channel);

  digitalWrite(m_ce_pin, HIGH);  
}

// Put the radio in ESB-TX mode
void radio::enter_esb_tx_mode(uint8_t * address, uint8_t timeout, uint8_t retries)
{
  digitalWrite(m_ce_pin, LOW);

  // Enable RX pipe 0
  write_register(EN_RXADDR, ENRX_P0);

  // Enable 5 byte addresses
  write_register(SETUP_AW, AW_5);

  // Set the addresses
  write_register(TX_ADDR, address, 5);
  write_register(RX_ADDR_P0, address, 5);

  // Enable dynamic payload length and automatic ACK handling
  write_register(FEATURE, EN_DPL | EN_DYN_ACK);
  write_register(DYNPD, DPL_P0);
  write_register(EN_AA, ENAA_P0);

  // Setup auto-retransmit
  // - timeout is in multiples of 250us
  uint8_t retr = timeout << 4 | retries;
  write_register(SETUP_RETR, retr);

  // Set the data rate to 2Mbps, max transmit power
  write_register(RF_SETUP, RATE_2M | RF_PWR_4);

  // Enable 16 bit CRC, enable TX, power up
  write_register(CONFIG, PWR_UP | EN_CRC | CRCO); 

  // Tune the radio
  set_channel(m_channel);
}

// Tune the radio
void radio::set_channel(unsigned char ch)
{
  m_channel = ch;
  write_register(RF_CH, m_channel);  
  flush_rx();
  flush_tx();
}

// Set the mouse to follow
void radio::follow_mouse(logitech_mouse * mouse)
{
  m_mouse = mouse; 

  m_following = true;

  enter_sniffer_mode(m_mouse->address.bytes);

  printf("Following Mouse %02X:%02X:%02X:%02X:%02X\n",
    m_mouse->address.bytes[0],
    m_mouse->address.bytes[1],
    m_mouse->address.bytes[2],
    m_mouse->address.bytes[3],
    m_mouse->address.bytes[4]);
}

// Update the channel if needed
void radio::update_following_channel()
{
  if(m_mouse->channel != m_channel) set_channel(m_mouse->channel);
}
