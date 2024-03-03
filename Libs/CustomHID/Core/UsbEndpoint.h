#pragma once

#include <stdint.h>

struct UsbEndpoint
{
  uint32_t status = 0;
  bool is_used = false;
  uint32_t total_length = 0;
  uint32_t rem_length = 0;
  uint32_t maxpacket = 0;

  void setLength(uint16_t length);
};