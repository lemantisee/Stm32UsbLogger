#pragma once

#include "UsbDriverF103.h"
#include "UsbDeviceDescriptor.h"
#include "CustomHid.h"
#include "Logger.h"

class UsbDevice
{
public:
    bool init();
    bool sendData(const SString<64> &data);
    SString<64> popData();

private:
    UsbDriverF103 mDriver;
    UsbHandle mHandle;
    UsbDeviceDescriptor mDescriptor;
    CustomHid mCustomHid;
};
