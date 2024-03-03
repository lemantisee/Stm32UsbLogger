#pragma once

#include "UsbEndpoint.h"
#include "UsbDescriptor.h"

typedef struct _USBD_HandleTypeDef
{
    enum EndpointType {
        EndpointControl = 0,
        EndpointIsochronous,
        EndpointBulk,
        EndpointInterrupt,
    };

    // enum Speed {
    //     SpeedHigh = 0,
    //     SpeedFull,
    //     SpeedLow,
    // };

    UsbEndpoint ep_in[16];
    UsbEndpoint ep_out[16];

    void *mClassData = nullptr;
    void *pUserData = nullptr;
    void *pData = nullptr;

    bool init(UsbDescriptor *descriptor, uint8_t id_);
    bool deinit();

    void setup(uint8_t *psetup);

    bool start();
    bool stop();
    bool resetUsb();
    bool disconnect();
    void suspend();
    void resume();
    void sof();

    bool isConfigured() const;

    void setSpeed(UsbSpeed speed);
    bool setClassConfig(uint8_t cfgidx);
    bool clearClassConfig(uint8_t cfgidx);
    bool registerClass(USBD_ClassTypeDef *pclass);

    bool dataOutStage(uint8_t epnum, uint8_t *pdata);
    bool dataInStage(uint8_t epnum, uint8_t *pdata);
    bool isoInIncomplete(uint8_t epnum);
    bool isoOUTIncomplete(uint8_t epnum);

    uint32_t getRxCount(uint8_t ep_addr);
    bool sendData(uint8_t *pbuf, uint16_t len);
    bool continueSendData(uint8_t *pbuf, uint16_t len);
    bool prepareRx(uint8_t *pbuf, uint16_t len);
    bool sendStatus();

    void stallEndpoints();

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

    bool onDeviceRequest(USBD_SetupReqTypedef *req);
    bool onInterfaceRequest(USBD_SetupReqTypedef *req);
    bool onEndpointRequest(USBD_SetupReqTypedef *req);

    void openOutEndpoint0();
    void openInEndpoint0();

    bool receiveStatus();
    bool continueRx(uint8_t *pbuf, uint16_t len);

    void setConfig(USBD_SetupReqTypedef *req);
    void getConfig(USBD_SetupReqTypedef *req);
    void getDescriptor(USBD_SetupReqTypedef *req);
    void setAddress(USBD_SetupReqTypedef *req);
    void getStatus(USBD_SetupReqTypedef *req);
    void setFeature(USBD_SetupReqTypedef *req);
    void clearFeature(USBD_SetupReqTypedef *req);

    bool isEndpointIn(uint8_t epAddress) const;

    uint8_t mId = 0;
    DeviceState mState = DeviceDefault;
    uint32_t mConfigIndex = 0;
    uint32_t mConfigDefault = 0;
    uint32_t mConfigStatus = 0;
    DeviceState mDeviceOldState = DeviceDefault;
    uint8_t mAddress = 0;
    uint8_t mConnectionStatus = 0;
    uint8_t mTestMode = 0;
    uint32_t mRemoteWakeup = 0;
    EndpointState mEndpoint0State = EndpointIdle;
    uint32_t mEndpoint0Size = 0;
    UsbSpeed mSpeed = USBD_SPEED_HIGH;
    USBD_SetupReqTypedef mRequest;
    UsbDescriptor *mDescriptor = nullptr;
    USBD_ClassTypeDef *mClassType = nullptr;

} UsbHandle;