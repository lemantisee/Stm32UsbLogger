#include "UsbDescriptor.h"

class UsbDeviceDescriptor : public UsbDescriptor
{
public:
    std::span<uint8_t> getStringDescriptor(usb::UsbSpeed speed, StringIndex strIndex) const override;

    std::span<uint8_t> GetDeviceDescriptor(usb::UsbSpeed speed) const override;
    std::span<uint8_t> getBOSDescriptor(usb::UsbSpeed speed)  const;

private:
    std::span<uint8_t> GetLangIDStrDescriptor(usb::UsbSpeed speed) const;
    std::span<uint8_t> GetManufacturerStrDescriptor(usb::UsbSpeed speed) const;
    std::span<uint8_t> GetProductStrDescriptor(usb::UsbSpeed speed) const;
    std::span<uint8_t> GetSerialStrDescriptor(usb::UsbSpeed speed) const;
    std::span<uint8_t> GetConfigurationStrDescriptor(usb::UsbSpeed speed) const;
    std::span<uint8_t> GetInterfaceStrDescriptor(usb::UsbSpeed speed) const;

    std::span<uint8_t> getDescriptor(char *stringData) const;
    void asciiToUnicode(char *ascii, uint8_t *unicode, uint16_t *len) const;
    void intToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len) const;
    void fillSerialNumber(uint8_t *value1, uint8_t value1Len, uint8_t *value2, uint8_t value2Len)  const;
};
