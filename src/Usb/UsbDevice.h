#pragma once

#include "usbd_desc.h"
#include "UsbCoreF103.h"

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
};
