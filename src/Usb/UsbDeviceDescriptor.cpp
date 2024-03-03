#include "UsbDeviceDescriptor.h"

#include "stm32f1xx.h"
#include <cstring>

namespace {

const uint32_t USBD_MAX_STR_DESC_SIZ = 512;

const uint16_t USBD_VID = 1155;
const uint16_t USBD_PID_FS = 22352;
const uint16_t USBD_LANGID_STRING = 1033;
const uint8_t USB_SIZ_STRING_SERIAL = 0x1A;

const char *USBD_MANUFACTURER_STRING = "STMicroelectronics";
const char *USBD_PRODUCT_STRING_FS = "STM32 Custom Human interface";
const char *USBD_CONFIGURATION_STRING_FS = "Custom HID Config";
const char *USBD_INTERFACE_STRING_FS = "Custom HID Interface";

__ALIGN_BEGIN uint8_t USBD_FS_DeviceDesc[USB_LEN_DEV_DESC] __ALIGN_END = {
    0x12,                 /*bLength */
    USB_DESC_TYPE_DEVICE, /*bDescriptorType*/
    0x00,                 /*bcdUSB */
    0x02,
    0x00,                /*bDeviceClass*/
    0x00,                /*bDeviceSubClass*/
    0x00,                /*bDeviceProtocol*/
    USB_MAX_EP0_SIZE,    /*bMaxPacketSize*/
    LOBYTE(USBD_VID),    /*idVendor*/
    HIBYTE(USBD_VID),    /*idVendor*/
    LOBYTE(USBD_PID_FS), /*idProduct*/
    HIBYTE(USBD_PID_FS), /*idProduct*/
    0x00,                /*bcdDevice rel. 2.00*/
    0x02,
    USBD_IDX_MFC_STR,          /*Index of manufacturer  string*/
    USBD_IDX_PRODUCT_STR,      /*Index of product string*/
    USBD_IDX_SERIAL_STR,       /*Index of serial number string*/
    USBD_MAX_NUM_CONFIGURATION /*bNumConfigurations*/
};

__ALIGN_BEGIN uint8_t USBD_LangIDDesc[USB_LEN_LANGID_STR_DESC] __ALIGN_END
    = {USB_LEN_LANGID_STR_DESC, USB_DESC_TYPE_STRING, LOBYTE(USBD_LANGID_STRING),
       HIBYTE(USBD_LANGID_STRING)};

/* Internal string descriptor. */
__ALIGN_BEGIN uint8_t USBD_StrDesc[USBD_MAX_STR_DESC_SIZ] __ALIGN_END;

__ALIGN_BEGIN uint8_t USBD_StringSerial[USB_SIZ_STRING_SERIAL] __ALIGN_END = {
    USB_SIZ_STRING_SERIAL,
    USB_DESC_TYPE_STRING,
};

} // namespace

uint8_t *UsbDeviceDescriptor::GetDeviceDescriptor(UsbSpeed speed, uint16_t *length)
{
    *length = sizeof(USBD_FS_DeviceDesc);
    return USBD_FS_DeviceDesc;
}

uint8_t *UsbDeviceDescriptor::GetLangIDStrDescriptor(UsbSpeed speed, uint16_t *length)
{
    *length = sizeof(USBD_LangIDDesc);
    return USBD_LangIDDesc;
}

uint8_t *UsbDeviceDescriptor::GetManufacturerStrDescriptor(UsbSpeed speed, uint16_t *length)
{
    asciiToUnicode((char *)USBD_MANUFACTURER_STRING, USBD_StrDesc, length);
    return USBD_StrDesc;
}

uint8_t *UsbDeviceDescriptor::GetProductStrDescriptor(UsbSpeed speed, uint16_t *length)
{
    asciiToUnicode((char *)USBD_PRODUCT_STRING_FS, USBD_StrDesc, length);
    return USBD_StrDesc;
}

uint8_t *UsbDeviceDescriptor::GetSerialStrDescriptor(UsbSpeed speed, uint16_t *length)
{
    *length = USB_SIZ_STRING_SERIAL;
    fillSerialNumber(&USBD_StringSerial[2], 8, &USBD_StringSerial[18], 4);
    return USBD_StringSerial;
}

uint8_t *UsbDeviceDescriptor::GetConfigurationStrDescriptor(UsbSpeed speed, uint16_t *length)
{
    asciiToUnicode((char *)USBD_CONFIGURATION_STRING_FS, USBD_StrDesc, length);
    return USBD_StrDesc;
}

uint8_t *UsbDeviceDescriptor::GetInterfaceStrDescriptor(UsbSpeed speed, uint16_t *length)
{
    asciiToUnicode((char *)USBD_INTERFACE_STRING_FS, USBD_StrDesc, length);
    return USBD_StrDesc;
}

void UsbDeviceDescriptor::asciiToUnicode(char *ascii, uint8_t *unicode, uint16_t *len) const
{
    if (!ascii) {
        return;
    }

    uint8_t idx = 0;
    *len = strlen(ascii) * 2 + 2;
    unicode[idx++] = *(uint8_t *)(void *)len;
    unicode[idx++] = USB_DESC_TYPE_STRING;

    while (*ascii != '\0') {
        unicode[idx++] = *ascii++;
        unicode[idx++] = 0;
    }
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

void UsbDeviceDescriptor::fillSerialNumber(uint8_t *value1, uint8_t value1Len, uint8_t *value2, uint8_t value2Len) 
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
