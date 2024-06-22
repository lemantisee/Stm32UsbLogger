#pragma once

#include "UsbDriver.h"

#include <stm32f1xx_hal.h>

class UsbDriverF103 : public UsbDriver
{
public:
    bool openEndpoint(uint8_t ep_addr, UsbHandle::EndpointType enType, uint16_t ep_mps) override;

    bool closeEndpoint(uint8_t ep_addr) override;
    bool flushEndpoint(uint8_t ep_addr) override;
    bool stallEndpoint(uint8_t ep_addr) override;
    bool clearStallEndpoint(uint8_t ep_addr) override;
    bool isEndpointStall(uint8_t ep_addr) const override;
    bool setUsbAddress(uint8_t dev_addr) override;
    bool transmit(uint8_t ep_addr, std::span<uint8_t> data) const override;
    bool prepareReceive(uint8_t ep_addr, std::span<uint8_t> data) override;
    uint32_t getRxDataSize(uint8_t ep_addr) override;

    bool initInterface(UsbHandle *pdev) override;
    bool deinitInterface() override;
    bool startInterface() override;
    bool stopInterface() override;

private:
    static void PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd);
    static void PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum);
    static void PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum);
    static void PCD_SOFCallback(PCD_HandleTypeDef *hpcd);
    static void PCD_ResetCallback(PCD_HandleTypeDef *hpcd);
    static void PCD_SuspendCallback(PCD_HandleTypeDef *hpcd);
    static void PCD_ResumeCallback(PCD_HandleTypeDef *hpcd);
    static void PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum);
    static void PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum);
    static void PCD_ConnectCallback(PCD_HandleTypeDef *hpcd);
    static void PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd);
    static void PCDEx_SetConnectionState(PCD_HandleTypeDef *hpcd, uint8_t state);
};
