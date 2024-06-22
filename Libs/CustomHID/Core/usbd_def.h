#pragma once

#include <stdint.h>

namespace usb {

enum DescriptorType {
    DeviceDescriptor = 1,
    ConfigurationDescriptor = 2,
    StringDescriptor = 3,
    EndpointDescriptor = 5,
    QualfierDescriptor = 6,
    ConfigurationOtherSpeedDescriptor = 7,
};

enum UsbSpeed {
    HighSpeed = 0,
    FullSpeed = 1,
    LowSpeed = 2,
};

const uint8_t maxInterfaceNumber = 1;
const uint8_t maxConfigurationNumber = 1;

const uint8_t endpont0_Size = 64;

const uint8_t defualtEndpointInAddress = 0x80;
const uint8_t defualtEndpointOutAddress = 0x00;
}


