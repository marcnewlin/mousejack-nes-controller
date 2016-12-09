#pragma once

#include "config.h"

// Logitech address 
struct logitech_address
{
  uint8_t bytes[address_length];

  // Constructors
  logitech_address(uint8_t _bytes[address_length]) { memcpy(bytes, _bytes, address_length); }
  logitech_address(){}

  // Equality operators
  inline bool operator==(const logitech_address& addr) { return memcmp(bytes, addr.bytes, address_length) == 0; }
  inline bool operator==(const uint8_t addr[address_length]) { return memcmp(bytes, addr, address_length) == 0; }  
};

// Mouse button mask
enum mouse_button_mask
{
  button_left    = 0x01,
  button_right   = 0x02,
  button_wheel   = 0x04,
  button_back    = 0x08,
  button_forward = 0x10,
};

// Mouse state
enum mouse_state
{
  state_unknown,
  state_active,
  state_idle,
  state_sleep,
};

// Mouse state
struct logitech_mouse
{
  // Address
  logitech_address address;

  // State
  mouse_state state;

  // Dongle address
  logitech_address dongle_address;

  // RF channel (last known)
  uint8_t channel;
  uint64_t last_tune_time;

  // Time of last received packet 
  uint64_t last_packet_time_rx;

  // Time of last transmitted (and ACK'd) packet
  uint64_t last_packet_time_tx;

  // Time of last transmitted (and ACK'd) movement packet
  uint64_t last_movement_packet_time_tx;

  // Time of last transmitted (and ACK'd) End TX packet
  uint64_t last_end_tx_packet_time_tx;

  // Start time of the current pending movement
  uint64_t pending_movement_time; 

  // Flag indicating if we have transmitted movement frames (without movement complete frames)
  bool pending_movement; 

  // Flag inidcating if this mouse is actively being followed
  bool followed;

  // Time of the last channel sweep and individual ping looking for this mouse
  uint64_t last_sweep_time;
  uint64_t last_ping_time; 

  // Flag indicating that this mouse is selected in the UI
  bool selected;

  // Flag indicating that this mouse is currently hijacked
  bool hijacked;

  // Get the most recent packet time (tx or rx)
  uint64_t last_packet_time()
  {
    if(last_packet_time_rx > last_packet_time_tx) return last_packet_time_rx;
    else return last_packet_time_tx;
  }

  // Constructors
  logitech_mouse(){}
  logitech_mouse(uint8_t * address, uint8_t channel) : 
    address(address),
    channel(channel),
    followed(false),
    last_packet_time_rx(0),
    last_packet_time_tx(0),
    last_sweep_time(0),
    selected(false),
    hijacked(false),
    pending_movement_time(0),
    pending_movement(false),
    last_ping_time(0),
    state(state_unknown),
    last_movement_packet_time_tx(0),
    last_end_tx_packet_time_tx(0)
  {
    dongle_address = address;
    dongle_address.bytes[0] = 0;
  }
};
