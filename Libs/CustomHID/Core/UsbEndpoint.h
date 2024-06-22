#pragma once

#include <stdint.h>

struct UsbEndpoint
{
  uint32_t status = 0;
  bool isUsed = false;
  uint32_t lengthToSend = 0;
  uint32_t currentLength = 0;
  uint32_t maxPacketLength = 0;

  void setLength(uint16_t length);
};