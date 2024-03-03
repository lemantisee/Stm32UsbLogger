#include "UsbDevice.h"

#include "usbd_desc.h"
#include "usbd_customhid.h"
#include "usbd_custom_hid_if.h"

#include "UsbCore.h"

#include <cstring>

namespace {
  const uint8_t deviceId = 0;
}

bool UsbDevice::init()
{
  if (!UsbCore::ref()->init(&mHandle, &FS_Desc, deviceId)) {
      return false;
  }

  mHandle.pClass = &USBD_CUSTOM_HID;
  mHandle.pUserData = &USBD_CustomHID_fops_FS;

  return UsbCore::ref()->start(&mHandle);
}

bool UsbDevice::sendData(const char *data)
{
    return USBD_CUSTOM_HID_SendReport(&mHandle, (uint8_t *)data, strlen(data)) == USBD_OK;
}
