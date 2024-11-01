#pragma once

#include "UsbDriverF103.h"
#include "UsbDeviceDescriptor.h"
#include "CustomHid.h"
#include "Logger.h"

class UsbDevice
{
public:
    bool init();
    bool sendData(const char *data);
    bool popData(std::span<char> buffer);

private:
    UsbDriverF103 mDriver;
    UsbHandle mHandle;
    UsbDeviceDescriptor mDescriptor;
    CustomHid mCustomHid;
};
