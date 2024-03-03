#include "UsbDescriptor.h"

class UsbDeviceDescriptor : public UsbDescriptor
{
public:
    uint8_t *GetDeviceDescriptor(UsbSpeed speed, uint16_t *length) override;
    uint8_t *GetLangIDStrDescriptor(UsbSpeed speed, uint16_t *length) override;
    uint8_t *GetManufacturerStrDescriptor(UsbSpeed speed, uint16_t *length) override;
    uint8_t *GetProductStrDescriptor(UsbSpeed speed, uint16_t *length) override;
    uint8_t *GetSerialStrDescriptor(UsbSpeed speed, uint16_t *length) override;
    uint8_t *GetConfigurationStrDescriptor(UsbSpeed speed, uint16_t *length) override;
    uint8_t *GetInterfaceStrDescriptor(UsbSpeed speed, uint16_t *length) override;

private:
    void asciiToUnicode(char *ascii, uint8_t *unicode, uint16_t *len) const;
    void intToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len) const;
    void fillSerialNumber(uint8_t *value1, uint8_t value1Len, uint8_t *value2, uint8_t value2Len);
};
