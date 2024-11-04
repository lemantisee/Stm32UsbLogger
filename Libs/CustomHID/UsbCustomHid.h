#pragma once

#include "UsbClass.h"

const uint8_t customHidReportDescriptoSize = 33;
const uint8_t customHidOutReportBufferSize = 64;

class UsbCustomHid : public UsbClass
{
public:
    bool init(UsbHandle *pdev, uint8_t cfgidx) override;
    bool deInit(UsbHandle *pdev, uint8_t cfgidx) override;
    bool setup(UsbHandle *pdev, const UsbSetupRequest &req) override;
    bool ep0_TxSent(UsbHandle *pdev) override;
    bool ep0_RxReady(UsbHandle *pdev) override;
    bool dataIn(UsbHandle *pdev, uint8_t epnum) override;
    bool dataOut(UsbHandle *pdev, uint8_t epnum) override;
    bool SOF(UsbHandle *pdev) override;
    bool isoINIncomplete(UsbHandle *pdev, uint8_t epnum) override;
    bool isoOUTIncomplete(UsbHandle *pdev, uint8_t epnum) override;

    std::span<uint8_t> getConfigDescriptor(usb::UsbSpeed speed) override;
    std::span<uint8_t> getOtherSpeedConfigDescriptor() override;
    std::span<uint8_t> getDeviceQualifierDescriptor() override;

    bool sendReport(UsbHandle *pdev, const std::span<uint8_t> &data);

    static const uint8_t endpointInAddress = 0x81;
    static const uint8_t endpointOutAddress = 0x01;

    static uint8_t getEndpointInBufferSize();
    static uint8_t getEndpointOutBufferSize();

protected:
    virtual bool onInit() { return true; };
    virtual bool onDeinit() { return true; };
    virtual void onReceive(uint8_t *state, uint32_t size) {};
    virtual uint8_t *getReportDescriptor() const = 0;

private:
    bool onRequestClass(UsbHandle *pdev, const UsbSetupRequest &req);
    bool onRequestStandart(UsbHandle *pdev, const UsbSetupRequest &req);

    enum HidState {
        HidIdle,
        HidBusy,
    };

    uint8_t mReportBuffer[customHidOutReportBufferSize];
    uint8_t mProtocol = 0;
    uint8_t mIdleState = 0;
    uint32_t mAltSetting = 0;
    bool mIsReportAvailable = false;
    HidState mState = HidIdle;
};
