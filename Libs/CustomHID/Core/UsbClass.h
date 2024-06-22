#pragma once

#include "usbd_def.h"
#include <stdint.h>
#include <span>

class UsbHandle;
class UsbSetupRequest;

class UsbClass {
public:
  virtual bool init(UsbHandle *pdev, uint8_t cfgidx) = 0;
  virtual bool deInit(UsbHandle *pdev, uint8_t cfgidx) = 0;
  /* Control Endpoints*/
  virtual bool setup(UsbHandle *pdev, const UsbSetupRequest &req) = 0;
  virtual bool ep0_TxSent(UsbHandle *pdev) = 0;
  virtual bool ep0_RxReady(UsbHandle *pdev) = 0;
  /* Class Specific Endpoints*/
  virtual bool dataIn(UsbHandle *pdev, uint8_t epnum) = 0;
  virtual bool dataOut(UsbHandle *pdev, uint8_t epnum) = 0;
  virtual bool SOF(UsbHandle *pdev) = 0;
  virtual bool isoINIncomplete(UsbHandle *pdev, uint8_t epnum) = 0;
  virtual bool isoOUTIncomplete(UsbHandle *pdev, uint8_t epnum) = 0;

  virtual std::span<uint8_t> getConfigDescriptor(usb::UsbSpeed speed) = 0;
  virtual std::span<uint8_t> getOtherSpeedConfigDescriptor() = 0;
  virtual std::span<uint8_t> getDeviceQualifierDescriptor() = 0;
#if (USBD_SUPPORT_USER_STRING_DESC == 1U)
  virtual std::span<uint8_t> GetUsrStrDescriptor(UsbHandle *pdev, uint8_t index) = 0;
#endif
};