#include "UsbDevice.h"

#include "UsbDriver.h"

#include <cstring>

namespace {
const uint8_t deviceId = 0;
}

bool UsbDevice::init()
{
    if (!mHandle.init(&mDescriptor, deviceId, &mDriver)) {
        return false;
    }

    mHandle.registerClass(&mCustomHid);

    return mHandle.start();
}

bool UsbDevice::sendData(const char *data)
{
    return mCustomHid.sendReport(&mHandle, {(uint8_t *)data, strlen(data)});
}

void UsbDevice::print(const char *str)
{
    mCustomHid.sendReport(&mHandle, {(uint8_t *)str, strlen(str)});
}
