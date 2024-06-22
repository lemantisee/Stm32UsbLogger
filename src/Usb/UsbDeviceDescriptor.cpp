#include "UsbDeviceDescriptor.h"

#include "stm32f1xx.h"
#include <cstring>

namespace {

const uint32_t stringDescriptorMaxSize = 512;

const uint16_t usbdVid = 1155;
const uint16_t usbPid = 22352;
const uint16_t languageIdString = 1033;
const uint8_t stringSerialSize = 0x1A;
const uint8_t deviceDescriptorSize = 0x12;
const uint8_t languageIdStringSize = 0x04;

const char *manufacturerString = "STMicroelectronics";
const char *productString = "STM32 Custom Human interface";
const char *configurationString = "Custom HID Config";
const char *interfaceString = "Custom HID Interface";

uint8_t loByte(uint16_t value) { return uint8_t(value & 0x00FF); }
uint8_t hiByte(uint16_t value) { return uint8_t((value & 0xFF00) >> 8); }

__ALIGN_BEGIN uint8_t deviceDescriptor[deviceDescriptorSize] __ALIGN_END = {
    deviceDescriptorSize,  /*bLength */
    usb::DeviceDescriptor, /*bDescriptorType*/
    0x00,                  /*bcdUSB */
    0x02,
    0x00,               /*bDeviceClass*/
    0x00,               /*bDeviceSubClass*/
    0x00,               /*bDeviceProtocol*/
    usb::endpont0_Size, /*bMaxPacketSize*/
    loByte(usbdVid),    /*idVendor*/
    hiByte(usbdVid),    /*idVendor*/
    loByte(usbPid),     /*idProduct*/
    hiByte(usbPid),     /*idProduct*/
    0x00,               /*bcdDevice rel. 2.00*/
    0x02,
    UsbDescriptor::ManufactureStringIndex, /*Index of manufacturer  string*/
    UsbDescriptor::ProductStringIndex,     /*Index of product string*/
    UsbDescriptor::SerialStringIndex,      /*Index of serial number string*/
    usb::maxConfigurationNumber            /*bNumConfigurations*/
};

__ALIGN_BEGIN uint8_t languageIdDescriptor[languageIdStringSize] __ALIGN_END
    = {languageIdStringSize, usb::StringDescriptor, loByte(languageIdString),
       hiByte(languageIdString)};

/* Internal string descriptor. */
__ALIGN_BEGIN uint8_t stringDescriptor[stringDescriptorMaxSize] __ALIGN_END;

__ALIGN_BEGIN uint8_t stringSerail[stringSerialSize] __ALIGN_END = {
    stringSerialSize,
    usb::StringDescriptor,
};

} // namespace

std::span<uint8_t> UsbDeviceDescriptor::getStringDescriptor(usb::UsbSpeed speed,
                                                            StringIndex strIndex) const
{
    switch (strIndex) {
    case LanguageIdStringIndex: return GetLangIDStrDescriptor(speed);
    case ManufactureStringIndex: return GetManufacturerStrDescriptor(speed);
    case ProductStringIndex: return GetProductStrDescriptor(speed);
    case SerialStringIndex: return GetSerialStrDescriptor(speed);
    case ConfigStringIndex: return GetConfigurationStrDescriptor(speed);
    case InterfaceStringIndex: return GetInterfaceStrDescriptor(speed);
    default: break;
    }

    return {};
}

std::span<uint8_t> UsbDeviceDescriptor::GetDeviceDescriptor(usb::UsbSpeed speedgth) const
{
    return std::span(deviceDescriptor);
}

std::span<uint8_t> UsbDeviceDescriptor::getBOSDescriptor(usb::UsbSpeed speed) const { return {}; }

std::span<uint8_t> UsbDeviceDescriptor::GetLangIDStrDescriptor(usb::UsbSpeed speedgth) const
{
    return std::span(languageIdDescriptor);
}

std::span<uint8_t> UsbDeviceDescriptor::GetManufacturerStrDescriptor(usb::UsbSpeed) const
{
    return getDescriptor((char *)manufacturerString);
}

std::span<uint8_t> UsbDeviceDescriptor::GetProductStrDescriptor(usb::UsbSpeed) const
{
    return getDescriptor((char *)productString);
}

std::span<uint8_t> UsbDeviceDescriptor::GetSerialStrDescriptor(usb::UsbSpeed) const
{
    fillSerialNumber(&stringSerail[2], 8, &stringSerail[18], 4);
    return std::span(stringSerail);
}

std::span<uint8_t> UsbDeviceDescriptor::GetConfigurationStrDescriptor(usb::UsbSpeed) const
{
    return getDescriptor((char *)configurationString);
}

std::span<uint8_t> UsbDeviceDescriptor::GetInterfaceStrDescriptor(usb::UsbSpeed) const
{
    return getDescriptor((char *)interfaceString);
}

void UsbDeviceDescriptor::asciiToUnicode(char *ascii, uint8_t *unicode, uint16_t *len) const
{
    if (!ascii) {
        return;
    }

    uint8_t idx = 0;
    *len = strlen(ascii) * 2 + 2;
    unicode[idx++] = *(uint8_t *)(void *)len;
    unicode[idx++] = usb::StringDescriptor;

    while (*ascii != '\0') {
        unicode[idx++] = *ascii++;
        unicode[idx++] = 0;
    }
}

std::span<uint8_t> UsbDeviceDescriptor::getDescriptor(char *stringData) const
{
    uint16_t length = 0;
    asciiToUnicode(stringData, stringDescriptor, &length);
    return std::span(stringDescriptor, length);
}

void UsbDeviceDescriptor::intToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len) const
{
    uint8_t idx = 0;

    for (idx = 0; idx < len; idx++) {
        if (((value >> 28)) < 0xA) {
            pbuf[2 * idx] = (value >> 28) + '0';
        } else {
            pbuf[2 * idx] = (value >> 28) + 'A' - 10;
        }

        value = value << 4;

        pbuf[2 * idx + 1] = 0;
    }
}

void UsbDeviceDescriptor::fillSerialNumber(uint8_t *value1, uint8_t value1Len, uint8_t *value2,
                                           uint8_t value2Len) const
{
    const uint32_t DEVICE_ID1 = UID_BASE;
    const uint32_t DEVICE_ID2 = UID_BASE + 0x4;
    const uint32_t DEVICE_ID3 = UID_BASE + 0x8;

    uint32_t deviceserial0 = *(uint32_t *)DEVICE_ID1;
    uint32_t deviceserial1 = *(uint32_t *)DEVICE_ID2;
    uint32_t deviceserial2 = *(uint32_t *)DEVICE_ID3;

    deviceserial0 += deviceserial2;

    if (deviceserial0 != 0) {
        intToUnicode(deviceserial0, value1, value1Len);
        intToUnicode(deviceserial1, value2, value2Len);
    }
}
