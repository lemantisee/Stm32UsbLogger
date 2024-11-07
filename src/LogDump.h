#pragma once

#include "SString.h"

class UsbDevice;

class LogDump
{
public:
    void dump(UsbDevice &usbDevice);

private:
    SString<64> createLogUnit(bool end) const;
};
