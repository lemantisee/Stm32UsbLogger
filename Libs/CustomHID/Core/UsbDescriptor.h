#pragma once

#include "usbd_def.h"
#include <stdint.h>
#include <span>

class UsbDescriptor
{
public:
    enum StringIndex {
        LanguageIdStringIndex = 0,
        ManufactureStringIndex = 1,
        ProductStringIndex = 2,
        SerialStringIndex = 3,
        ConfigStringIndex = 4,
        InterfaceStringIndex = 5,
    };

    virtual std::span<uint8_t> GetDeviceDescriptor(usb::UsbSpeed speed) const = 0;

    virtual std::span<uint8_t> getStringDescriptor(usb::UsbSpeed speed, StringIndex strIndex) const
        = 0;

    virtual std::span<uint8_t> getBOSDescriptor(usb::UsbSpeed speed) const = 0;
};