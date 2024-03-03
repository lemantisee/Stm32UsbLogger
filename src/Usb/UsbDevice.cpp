#include "UsbDevice.h"

#include "usbd_customhid.h"
#include "usbd_custom_hid_if.h"

#include "UsbCore.h"

#include <cstring>

namespace {
const uint8_t deviceId = 0;
}

UsbDevice::UsbDevice() { UsbCore::setImpl(&mCore); }

bool UsbDevice::init()
{
    if (!mHandle.init(&mDescriptor, deviceId)) {
        return false;
    }

    mHandle.registerClass(&USBD_CUSTOM_HID);
    mHandle.pUserData = &USBD_CustomHID_fops_FS;

    return mHandle.start();
}

bool UsbDevice::sendData(const char *data)
{
    return USBD_CUSTOM_HID_SendReport(&mHandle, (uint8_t *)data, strlen(data)) == USBD_OK;
}
