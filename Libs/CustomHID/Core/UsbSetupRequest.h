#pragma once

#include <stdint.h>

#include <optional>

#include "usbd_def.h"
#include "UsbDescriptor.h"

class UsbSetupRequest
{
public:
    enum RecipientType {
        RecipientDevice = 0,
        RecipientInterface = 1,
        RecipientEndpoint = 2,
    };

    enum Request {
        RequestGetStatus = 0,
        RequestClearFeature = 1,
        RequestSetFeature = 3,
        RequestSetAddress = 5,
        RequestGetDescriptor = 6,
        RequestSetDescriptor = 7,
        RequestGetConfiguration = 8,
        RequestSetConfiguration = 9,
        RequestGetInterface = 10,
        RequestSetInterface = 11,
        RequestSyncFrame = 12,
    };

    enum RequestType {
        RequestStandart = 0,
        RequestClass = 32,
        RequestVendor = 64,
    };

    enum FeatureType {
        FeatureHaltEndpoint = 0,
        FeatureRemoteWakeUp = 1,
        FeatureTestMode = 2,
    };

    RecipientType getRecipient() const;
    Request getRequest() const;
    RequestType getRequestType() const;
    void parse(const uint8_t *pdata);

    uint16_t getLength() const;
    uint8_t getEndpointAddress() const;
    uint8_t getInterfaceIndex() const;
    std::optional<uint8_t> getDeviceAddress() const;
    usb::DescriptorType getDescriptorType() const;
    UsbDescriptor::StringIndex getStringIndex() const;
    FeatureType getFeatureRequest() const;
    uint8_t getConfigIndex() const;
    uint8_t getProtocol() const;
    uint8_t getIdleState() const;
    uint8_t getEndpointFromMask() const;

private:
    Request mRequest = RequestGetStatus;
    uint8_t mMaskRequest = 0;

    uint16_t mLength = 0;
    uint16_t mIndex = 0;
    uint16_t mValue = 0;
};