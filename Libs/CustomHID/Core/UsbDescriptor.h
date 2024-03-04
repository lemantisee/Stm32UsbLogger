#pragma once

#include "usbd_def.h"

class UsbDescriptor {
public:
    virtual uint8_t *GetDeviceDescriptor(UsbSpeed speed, uint16_t *length) = 0;
    virtual uint8_t *GetLangIDStrDescriptor(UsbSpeed speed, uint16_t *length) = 0;
    virtual uint8_t *GetManufacturerStrDescriptor(UsbSpeed speed, uint16_t *length) = 0;
    virtual uint8_t *GetProductStrDescriptor(UsbSpeed speed, uint16_t *length) = 0;
    virtual uint8_t *GetSerialStrDescriptor(UsbSpeed speed, uint16_t *length) = 0;
    virtual uint8_t *GetConfigurationStrDescriptor(UsbSpeed speed, uint16_t *length) = 0;
    virtual uint8_t *GetInterfaceStrDescriptor(UsbSpeed speed, uint16_t *length) = 0;

    #if (USBD_LPM_ENABLED == 1U)
    virtual uint8_t *GetBOSDescriptor(UsbSpeed speed, uint16_t *length) = 0;
    #endif
};