#pragma once

#include <span>

#include "UsbEndpoint.h"
#include "usbd_def.h"

class UsbClass;
class UsbDriver;
class UsbDescriptor;
class UsbSetupRequest;

class UsbHandle
{
public:
    enum EndpointType {
        EndpointControl = 0,
        EndpointIsochronous,
        EndpointBulk,
        EndpointInterrupt,
    };

    bool init(const UsbDescriptor *descriptor, uint8_t id, UsbDriver *driver);
    bool deinit();

    void setup(const uint8_t *psetup);

    bool start();
    bool stop();
    bool resetUsb();
    bool disconnect();
    void suspend();
    void resume();
    void sof();

    bool isConfigured() const;

    bool registerClass(UsbClass *pclass);

    bool dataOutStage(uint8_t epnum, uint8_t *pdata);
    bool dataInStage(uint8_t epnum, uint8_t *pdata);

    bool sendData(std::span<uint8_t> data);
    bool sendData(uint8_t data);
    bool receiveData(std::span<uint8_t> buffer);
    bool isoInIncomplete(uint8_t epnum);
    bool isoOUTIncomplete(uint8_t epnum);

    void setSelfPowered(bool state);

    UsbDriver *getDriver();

    bool openEndpoint(uint8_t address, uint16_t size, EndpointType type);
    bool closeEndpoint(uint8_t address);

private:
    enum DeviceState {
        DeviceDefault = 1,
        DeviceAddressed = 2,
        DeviceConfigured = 3,
        DeviceSuspended = 4,
    };

    enum EndpointState {
        EndpointIdle = 0,
        EndpointSetup,
        EndpointDataIn,
        EndpointDataOut,
        EndpointStatusIn,
        EndpointStatusOut,
        EndpointStall,
    };

    bool onDeviceRequest(const UsbSetupRequest &req);
    bool onInterfaceRequest(const UsbSetupRequest &req);
    bool onEndpointRequest(const UsbSetupRequest &req);

    bool onStandartRequest(const UsbSetupRequest &req);
    bool onSetFeature(const UsbSetupRequest &req);
    bool onClearFeature(const UsbSetupRequest &req);
    bool onGetEndpointStatus(const UsbSetupRequest &req);

    bool receiveStatus();
    bool continueReceiveData(std::span<uint8_t> buffer);

    bool setClassConfig(uint8_t configIndex);
    bool clearClassConfig(uint8_t configIndex);
    bool setConfig(const UsbSetupRequest &req);
    bool sendConfig(const UsbSetupRequest &req);
    bool sendDescriptor(const UsbSetupRequest &req);
    bool setAddress(const UsbSetupRequest &req);
    bool sendStatus(const UsbSetupRequest &req);
    bool setFeature(const UsbSetupRequest &req);
    bool clearFeature(const UsbSetupRequest &req);

    uint32_t getRxCount(uint8_t ep_addr);
    bool continueSendData(std::span<uint8_t> data);
    bool sendStatus();

    UsbEndpoint *getEndpoint(uint8_t epAddress);
    void stallEndpoints();

    uint8_t mId = 0;
    DeviceState mState = DeviceDefault;
    uint8_t mConfigIndex = 0;
    uint32_t mConfigDefault = 0;
    uint32_t mConfigStatus = 0;
    DeviceState mDeviceOldState = DeviceDefault;
    uint8_t mAddress = 0;
    bool mRemoteWakeup = false;
    EndpointState mEndpoint0State = EndpointIdle;
    uint32_t mEndpoint0Size = 0;
    usb::UsbSpeed mSpeed = usb::HighSpeed;
    const UsbDescriptor *mDescriptor = nullptr;
    UsbClass *mClassType = nullptr;
    UsbDriver *mDriver = nullptr;
    bool mSelfPowered = true;

    UsbEndpoint mInEndpoints[16];
    UsbEndpoint mOutEndpoints[16];
};