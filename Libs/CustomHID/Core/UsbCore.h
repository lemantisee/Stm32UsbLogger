#pragma once

#include "UsbHandle.h"

#include <stdint.h>

#ifndef USBD_DEBUG_LEVEL
#define USBD_DEBUG_LEVEL           0U
#endif /* USBD_DEBUG_LEVEL */

#define USBD_SOF          USBD_LL_SOF

class UsbCore
{
public:
    UsbCore() = default;
    virtual ~UsbCore() = default;

    static void setImpl(UsbCore *impl);
    static UsbCore *ref();

    // Low level

    virtual bool openEndpoint(UsbHandle *pdev,
                                   uint8_t  ep_addr,
                                   UsbHandle::EndpointType enType,
                                   uint16_t ep_mps) = 0;

    virtual bool closeEndpoint(UsbHandle *pdev, uint8_t  ep_addr) = 0;
    virtual bool flushEndpoint(UsbHandle *pdev, uint8_t ep_addr) = 0;
    virtual bool stallEndpoint(UsbHandle *pdev, uint8_t ep_addr) = 0;
    virtual bool clearStallEndpoint(UsbHandle *pdev, uint8_t ep_addr) = 0;
    virtual bool isEndpointStall(UsbHandle *pdev, uint8_t ep_addr) const = 0;
    virtual bool setUsbAddress(UsbHandle *pdev, uint8_t dev_addr) = 0;
    virtual bool transmit(UsbHandle *pdev,
                                     uint8_t  ep_addr,
                                     uint8_t  *pbuf,
                                     uint16_t  size) = 0;

    virtual bool prepareReceive(UsbHandle *pdev,
                                    uint8_t  ep_addr,
                                    uint8_t  *pbuf,
                                    uint16_t  size) = 0;

    virtual uint32_t getRxDataSize(UsbHandle *pdev, uint8_t  ep_addr) = 0;

    virtual uint32_t *malloc(uint32_t size) = 0;
    virtual void free(void *mem) = 0;

    virtual bool initInterface(UsbHandle *pdev) = 0;
    virtual bool deinitInterface(UsbHandle *pdev) = 0;
    virtual bool startInterface(UsbHandle *pdev) = 0;
    virtual bool stopInterface(UsbHandle *pdev) = 0;

private:
};
