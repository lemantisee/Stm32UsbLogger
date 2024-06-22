#include "UsbSetupRequest.h"

namespace {

uint16_t swapByte(const uint8_t *addr) { return uint16_t(*addr) + (uint16_t(*(addr + 1)) << 8); }
uint8_t loByte(uint16_t value) { return uint8_t(value & 0x00FF); }

} // namespace

UsbSetupRequest::RecipientType UsbSetupRequest::getRecipient() const
{
    return RecipientType(mMaskRequest & 0x1FU);
}

UsbSetupRequest::Request UsbSetupRequest::getRequest() const { return mRequest; }

UsbSetupRequest::RequestType UsbSetupRequest::getRequestType() const
{
    const uint32_t requestTypeMask = 96;
    return RequestType(mMaskRequest & requestTypeMask);
}

void UsbSetupRequest::parse(const uint8_t *pdata)
{
    mMaskRequest = *pdata;
    mRequest = UsbSetupRequest::Request(*(pdata + 1));
    mValue = swapByte(pdata + 2);
    mIndex = swapByte(pdata + 4);
    mLength = swapByte(pdata + 6);
}

uint16_t UsbSetupRequest::getLength() const { return mLength; }

uint8_t UsbSetupRequest::getEndpointAddress() const { return loByte(mIndex); }

uint8_t UsbSetupRequest::getInterfaceIndex() const { return loByte(mIndex); }

std::optional<uint8_t> UsbSetupRequest::getDeviceAddress() const
{
    if (mIndex != 0 || mLength != 0 || mValue >= 128) {
        return std::nullopt;
    }

    return (uint8_t)(mValue) & 0x7F;
}

usb::DescriptorType UsbSetupRequest::getDescriptorType() const
{
    return usb::DescriptorType(mValue >> 8);
}

UsbDescriptor::StringIndex UsbSetupRequest::getStringIndex() const
{
    return UsbDescriptor::StringIndex(uint8_t(mValue));
}

UsbSetupRequest::FeatureType UsbSetupRequest::getFeatureRequest() const
{
    return FeatureType(mValue);
}

uint8_t UsbSetupRequest::getConfigIndex() const { return mValue; }

uint8_t UsbSetupRequest::getProtocol() const { return uint8_t(mValue); }

uint8_t UsbSetupRequest::getIdleState() const { return uint8_t(mValue >> 8); }

uint8_t UsbSetupRequest::getEndpointFromMask() const { return mMaskRequest & 0x80U; }
