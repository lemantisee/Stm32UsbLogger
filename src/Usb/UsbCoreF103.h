#pragma once

#include "UsbCore.h"

#include <stm32f1xx_hal.h>

class UsbCoreF103 : public UsbCore
{
public:

    static PCD_HandleTypeDef &getPcdHandle();

    bool openEndpoint(USBD_HandleTypeDef *pdev,
                                   uint8_t  ep_addr,
                                   uint8_t  ep_type,
                                   uint16_t ep_mps) override;
    
    bool closeEndpoint(USBD_HandleTypeDef *pdev, uint8_t  ep_addr) override;
    bool flushEndpoint(USBD_HandleTypeDef *pdev, uint8_t ep_addr) override;
    bool stallEndpoint(USBD_HandleTypeDef *pdev, uint8_t ep_addr) override;
    bool clearStallEndpoint(USBD_HandleTypeDef *pdev, uint8_t ep_addr) override;
    bool isEndpointStall(USBD_HandleTypeDef *pdev, uint8_t ep_addr) const override;
    bool setUsbAddress(USBD_HandleTypeDef *pdev, uint8_t dev_addr) override;
    bool transmit(USBD_HandleTypeDef *pdev,
                                     uint8_t  ep_addr,
                                     uint8_t  *pbuf,
                                     uint16_t  size) override;

    bool prepareReceive(USBD_HandleTypeDef *pdev,
                                    uint8_t  ep_addr,
                                    uint8_t  *pbuf,
                                    uint16_t  size) override;
    
    uint32_t getRxDataSize(USBD_HandleTypeDef *pdev, uint8_t  ep_addr) override;

    virtual uint32_t *malloc(uint32_t size) override;
    virtual void free(void *mem) override;

protected:
    bool initInterface(USBD_HandleTypeDef *pdev) override;
    bool deinitInterface(USBD_HandleTypeDef *pdev) override;
    bool startInterface(USBD_HandleTypeDef *pdev) override;
    bool stopInterface(USBD_HandleTypeDef *pdev) override;

private:
    static PCD_HandleTypeDef mPcd;
};
