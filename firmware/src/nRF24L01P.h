#pragma once

// Configuration
enum CONFIG : uint8_t
{
  PRIM_RX     = 0x01,
  PWR_UP      = 0x02,
  CRCO        = 0x04,
  EN_CRC      = 0x08,
  MASK_MAX_RT = 0x10,
  MASK_TX_DS  = 0x20,
  MASK_RX_DR  = 0x40,
};

// Auto Acknowledgement
enum EN_AA : uint8_t
{
  ENAA_NONE = 0x00,
  ENAA_P0   = 0x01,
  ENAA_P1   = 0x02,
  ENAA_P2   = 0x04,
  ENAA_P3   = 0x08,
  ENAA_P4   = 0x10,
  ENAA_P5   = 0x20,
};

// Enabled RX Addresses
enum EN_RXADDR : uint8_t
{
  ENRX_P0 = 0x01,
  ENRX_P1 = 0x02,
  ENRX_P2 = 0x04,
  ENRX_P3 = 0x08,
  ENRX_P4 = 0x10,
  ENRX_P5 = 0x20,
};

// Address Widths
enum SETUP_AW : uint8_t
{
  AW_2 = 0x00,
  AW_3 = 0x01,
  AW_4 = 0x02,
  AW_5 = 0x03,
};

// RF Setup
enum RF_SETUP : uint8_t
{
  CONT_WAVE = 0x80,
  PLL_LOCK  = 0x10,
  RATE_2M   = 0x08,
  RATE_1M   = 0x00,
  RATE_250K = 0x20,
  RF_PWR_4  = 0x06,
  RF_PWR_3  = 0x04,
  RF_PWR_2  = 0x02,
  RF_PWR_1  = 0x00,
};

// Dynamic payloads
enum DYNPD : uint8_t
{
  DPL_P5 = 0x20,
  DPL_P4 = 0x10,
  DPL_P3 = 0x08,
  DPL_P2 = 0x04,
  DPL_P1 = 0x02,
  DPL_P0 = 0x01,
};

// Features
enum FEATURE : uint8_t
{
  EN_DPL     = 0x04,
  EN_ACK_PAY = 0x02,
  EN_DYN_ACK = 0x01  
};

// Status flags
enum STATUS : uint8_t
{
  RX_DR   = 0x40,
  TX_DS   = 0x20,
  MAX_RT  = 0x10,
  TX_FULL = 0x01,
};

struct nrf24_status
{
  bool rx_data_ready;
  bool tx_data_sent;
  bool max_tx_retransmits;
  uint8_t rx_data_pipe;
  bool tx_full; 

  nrf24_status(uint8_t byte)
  {
    tx_full = byte & 0x01;
    rx_data_pipe = (byte >> 1) & 0x07;
    max_tx_retransmits = byte & 0x10;
    tx_data_sent = byte & 0x20;
    rx_data_ready = byte & 0x40;
  }
};

enum nrf24_command
{
  R_REGISTER   = 0x00,
  W_REGISTER   = 0x20,
  R_RX_PL_WID  = 0x60,
  R_RX_PAYLOAD = 0x61,
  W_TX_PAYLOAD = 0xA0,
  FLUSH_TX     = 0xE1,
  FLUSH_RX     = 0xE2,
  _NOP         = 0xFF,
};

enum nrf24_register
{
  CONFIG      = 0x00,
  EN_AA       = 0x01,
  EN_RXADDR   = 0x02,
  SETUP_AW    = 0x03,
  SETUP_RETR  = 0x04,
  RF_CH       = 0x05,
  RF_SETUP    = 0x06,
  STATUS      = 0x07,
  OBSERVE_TX  = 0x08,
  RPD         = 0x09,
  RX_ADDR_P0  = 0x0A,
  RX_ADDR_P1  = 0x0B,
  RX_ADDR_P2  = 0x0C,
  RX_ADDR_P3  = 0x0D,
  RX_ADDR_P4  = 0x0E,
  RX_ADDR_P5  = 0x0F,
  TX_ADDR     = 0x10,
  RX_PW_P0    = 0x11,
  RX_PW_P1    = 0x12,
  RX_PW_P2    = 0x13,
  RX_PW_P3    = 0x14,
  RX_PW_P4    = 0x15,
  RX_PW_P5    = 0x16,
  FIFO_STATUS = 0x17,
  DYNPD       = 0x1C,
  FEATURE     = 0x1D,
};
