#pragma once

#include "UsbHandle.h"

#include <stdint.h>
#include <span>

class UsbDriver
{
public:
    virtual ~UsbDriver() = default;

    virtual bool openEndpoint(uint8_t ep_addr, UsbHandle::EndpointType enType, uint16_t ep_mps) = 0;
    virtual bool closeEndpoint(uint8_t ep_addr) = 0;
    virtual bool flushEndpoint(uint8_t ep_addr) = 0;
    virtual bool stallEndpoint(uint8_t ep_addr) = 0;
    virtual bool clearStallEndpoint(uint8_t ep_addr) = 0;
    virtual bool isEndpointStall(uint8_t ep_addr) const = 0;
    virtual bool setUsbAddress(uint8_t dev_addr) = 0;
    virtual bool transmit(uint8_t ep_addr, std::span<uint8_t> data) const = 0;
    virtual bool prepareReceive(uint8_t ep_addr, std::span<uint8_t> data) = 0;
    virtual uint32_t getRxDataSize(uint8_t ep_addr) = 0;

    virtual bool initInterface(UsbHandle *pdev) = 0;
    virtual bool deinitInterface() = 0;
    virtual bool startInterface() = 0;
    virtual bool stopInterface() = 0;
};
