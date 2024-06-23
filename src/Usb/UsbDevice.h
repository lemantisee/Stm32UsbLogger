#pragma once

#include "UsbDriverF103.h"
#include "UsbDeviceDescriptor.h"
#include "CustomHid.h"
#include "Logger.h"

class UsbDevice : public Printer
{
public:
    bool init();
    bool sendData(const char *data);
    bool popData(std::span<char> buffer);

    void print(const char *str) override;

private:
    UsbDriverF103 mDriver;
    UsbHandle mHandle;
    UsbDeviceDescriptor mDescriptor;
    CustomHid mCustomHid;
};
