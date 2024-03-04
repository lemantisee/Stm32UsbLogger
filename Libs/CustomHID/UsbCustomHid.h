#pragma once

#include "UsbClass.h"

class UsbCustomHid : public UsbClass
{
public:
    bool Init(UsbHandle *pdev, uint8_t cfgidx) override;
    bool DeInit(UsbHandle *pdev, uint8_t cfgidx) override;
    bool Setup(UsbHandle *pdev, USBD_SetupReqTypedef *req) override;
    bool EP0_TxSent(UsbHandle *pdev) override;
    bool EP0_RxReady(UsbHandle *pdev) override;
    bool DataIn(UsbHandle *pdev, uint8_t epnum) override;
    bool DataOut(UsbHandle *pdev, uint8_t epnum) override;
    bool SOF(UsbHandle *pdev) override;
    bool IsoINIncomplete(UsbHandle *pdev, uint8_t epnum) override;
    bool IsoOUTIncomplete(UsbHandle *pdev, uint8_t epnum) override;

    uint8_t *GetHSConfigDescriptor(uint16_t *length) override;
    uint8_t *GetFSConfigDescriptor(uint16_t *length) override;
    uint8_t *GetOtherSpeedConfigDescriptor(uint16_t *length) override;
    uint8_t *GetDeviceQualifierDescriptor(uint16_t *length) override;
};
