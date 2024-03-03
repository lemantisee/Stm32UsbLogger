#pragma once

#include <stdint.h>

#include <optional>

typedef struct usb_setup_req
{
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

    uint8_t bmRequest = 0;
    Request bRequest = RequestGetStatus;
    uint16_t wValue = 0;
    
    uint16_t wLength = 0;

    RecipientType getRecipient() const;
    Request getRequest() const;
    RequestType getRequestType() const;
    void parse(uint8_t *pdata);

    uint8_t getEndpointAddress() const;
    uint8_t getInterfaceNumber() const;
    std::optional<uint8_t> getDeviceAddress() const;
    uint8_t getDescriptorType() const;
    uint8_t getStringIndex() const;
    uint8_t getFeatureRequest() const;
    uint8_t getConfigIndex() const;

private:
    uint16_t mIndex = 0;

} USBD_SetupReqTypedef;