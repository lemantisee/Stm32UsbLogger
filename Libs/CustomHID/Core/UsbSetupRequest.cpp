#include "UsbSetupRequest.h"

namespace {

uint16_t swapByte(uint8_t *addr) { return uint16_t(*addr) + (uint16_t(*(addr + 1)) << 8); }
uint8_t loByte(uint16_t value) { return uint8_t(value & 0x00FF); }

} // namespace

usb_setup_req::RecipientType usb_setup_req::getRecipient() const
{
    return RecipientType(bmRequest & 0x1FU);
}

usb_setup_req::Request usb_setup_req::getRequest() const { return bRequest; }

usb_setup_req::RequestType usb_setup_req::getRequestType() const
{
    const uint32_t requestTypeMask = 96;
    return RequestType(bmRequest & requestTypeMask);
}

void usb_setup_req::parse(uint8_t *pdata)
{
    bmRequest = *(uint8_t *)(pdata);
    bRequest = USBD_SetupReqTypedef::Request(*(uint8_t *)(pdata + 1U));
    wValue = swapByte(pdata + 2U);
    mIndex = swapByte(pdata + 4U);
    wLength = swapByte(pdata + 6U);
}

uint8_t usb_setup_req::getEndpointAddress() const { return loByte(mIndex); }

uint8_t usb_setup_req::getInterfaceNumber() const { return loByte(mIndex); }

std::optional<uint8_t> usb_setup_req::getDeviceAddress() const
{
    if (mIndex != 0 || wLength != 0 || wValue >= 128) {
        return std::nullopt;
    }

    return (uint8_t)(wValue) & 0x7FU;
}

uint8_t usb_setup_req::getDescriptorType() const { return wValue >> 8; }

uint8_t usb_setup_req::getStringIndex() const { return uint8_t(wValue); }

uint8_t usb_setup_req::getFeatureRequest() const { return wValue; }

uint8_t usb_setup_req::getConfigIndex() const { return wValue; }
