#pragma once

#include "UsbCore.h"

#include <stm32f1xx_hal.h>

class UsbCoreF103 : public UsbCore
{
public:
    static PCD_HandleTypeDef &getPcdHandle();

    bool openEndpoint(UsbHandle *pdev, uint8_t ep_addr, UsbHandle::EndpointType enType,
                      uint16_t ep_mps) override;

    bool closeEndpoint(UsbHandle *pdev, uint8_t ep_addr) override;
    bool flushEndpoint(UsbHandle *pdev, uint8_t ep_addr) override;
    bool stallEndpoint(UsbHandle *pdev, uint8_t ep_addr) override;
    bool clearStallEndpoint(UsbHandle *pdev, uint8_t ep_addr) override;
    bool isEndpointStall(UsbHandle *pdev, uint8_t ep_addr) const override;
    bool setUsbAddress(UsbHandle *pdev, uint8_t dev_addr) override;
    bool transmit(UsbHandle *pdev, uint8_t ep_addr, uint8_t *pbuf, uint16_t size) override;

    bool prepareReceive(UsbHandle *pdev, uint8_t ep_addr, uint8_t *pbuf, uint16_t size) override;

    uint32_t getRxDataSize(UsbHandle *pdev, uint8_t ep_addr) override;

    virtual uint32_t *malloc(uint32_t size) override;
    virtual void free(void *mem) override;

    bool initInterface(UsbHandle *pdev) override;
    bool deinitInterface(UsbHandle *pdev) override;
    bool startInterface(UsbHandle *pdev) override;
    bool stopInterface(UsbHandle *pdev) override;

private:
    static PCD_HandleTypeDef mPcd;
};
