#pragma once

#include "usbd_desc.h"

class UsbDevice
{
public:
    UsbDevice() = default;
    ~UsbDevice() = default;

    bool init();
    bool sendData(const char *data);
private:
    USBD_HandleTypeDef mHandle;
};
