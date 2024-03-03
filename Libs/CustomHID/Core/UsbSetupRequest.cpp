#include "UsbSetupRequest.h"

namespace {

uint16_t swapByte(uint8_t *addr) { return uint16_t(*addr) + (uint16_t(*(addr + 1)) << 8); }
uint8_t loByte(uint16_t value) { return uint8_t(value & 0x00FF); }

} // namespace

usb_setup_req::RecipientType usb_setup_req::getRecipient() const
{
    return RecipientType(mMaskRequest & 0x1FU);
}

usb_setup_req::Request usb_setup_req::getRequest() const { return mRequest; }

usb_setup_req::RequestType usb_setup_req::getRequestType() const
{
    const uint32_t requestTypeMask = 96;
    return RequestType(mMaskRequest & requestTypeMask);
}

void usb_setup_req::parse(uint8_t *pdata)
{
    mMaskRequest = *pdata;
    mRequest = USBD_SetupReqTypedef::Request(*(pdata + 1));
    mValue = swapByte(pdata + 2);
    mIndex = swapByte(pdata + 4);
    mLength = swapByte(pdata + 6);
}

uint16_t usb_setup_req::getLength() const { return mLength; }

uint8_t usb_setup_req::getEndpointAddress() const { return loByte(mIndex); }

uint8_t usb_setup_req::getInterfaceIndex() const { return loByte(mIndex); }

std::optional<uint8_t> usb_setup_req::getDeviceAddress() const
{
    if (mIndex != 0 || mLength != 0 || mValue >= 128) {
        return std::nullopt;
    }

    return (uint8_t)(mValue) & 0x7F;
}

uint8_t usb_setup_req::getDescriptorType() const { return mValue >> 8; }

uint8_t usb_setup_req::getStringIndex() const { return uint8_t(mValue); }

uint8_t usb_setup_req::getFeatureRequest() const { return mValue; }

uint8_t usb_setup_req::getConfigIndex() const { return mValue; }

uint8_t usb_setup_req::getProtocol() const { return uint8_t(mValue); }

uint8_t usb_setup_req::getIdleState() const { return uint8_t(mValue >> 8); }

uint8_t usb_setup_req::getEndpointFromMask() const { return mMaskRequest & 0x80U; }
