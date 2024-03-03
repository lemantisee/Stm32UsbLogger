#pragma once

#include "UsbCoreF103.h"
#include "UsbDeviceDescriptor.h"

class UsbDevice
{
public:
    UsbDevice();
    ~UsbDevice() = default;

    bool init();
    bool sendData(const char *data);

private:
    UsbCoreF103 mCore;
    UsbHandle mHandle;
    UsbDeviceDescriptor mDescriptor;
};
