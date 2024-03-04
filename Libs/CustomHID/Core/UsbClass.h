#pragma once

#include "usbd_def.h"

class UsbHandle;

class UsbClass {
public:
  virtual bool Init(UsbHandle *pdev, uint8_t cfgidx) = 0;
  virtual bool DeInit(UsbHandle *pdev, uint8_t cfgidx) = 0;
  /* Control Endpoints*/
  virtual bool Setup(UsbHandle *pdev, USBD_SetupReqTypedef  *req) = 0;
  virtual bool EP0_TxSent(UsbHandle *pdev) = 0;
  virtual bool EP0_RxReady(UsbHandle *pdev) = 0;
  /* Class Specific Endpoints*/
  virtual bool DataIn(UsbHandle *pdev, uint8_t epnum) = 0;
  virtual bool DataOut(UsbHandle *pdev, uint8_t epnum) = 0;
  virtual bool SOF(UsbHandle *pdev) = 0;
  virtual bool IsoINIncomplete(UsbHandle *pdev, uint8_t epnum) = 0;
  virtual bool IsoOUTIncomplete(UsbHandle *pdev, uint8_t epnum) = 0;

  virtual uint8_t *GetHSConfigDescriptor(uint16_t *length) = 0;
  virtual uint8_t *GetFSConfigDescriptor(uint16_t *length) = 0;
  virtual uint8_t *GetOtherSpeedConfigDescriptor(uint16_t *length) = 0;
  virtual uint8_t *GetDeviceQualifierDescriptor(uint16_t *length) = 0;
#if (USBD_SUPPORT_USER_STRING_DESC == 1U)
  virtual uint8_t *GetUsrStrDescriptor(UsbHandle *pdev, uint8_t index,  uint16_t *length) = 0;
#endif
};

struct UsbClassStruct
{
  uint8_t (*Init)(UsbHandle *pdev, uint8_t cfgidx);
  uint8_t (*DeInit)(UsbHandle *pdev, uint8_t cfgidx);
  /* Control Endpoints*/
  uint8_t (*Setup)(UsbHandle *pdev, USBD_SetupReqTypedef  *req);
  uint8_t (*EP0_TxSent)(UsbHandle *pdev);
  uint8_t (*EP0_RxReady)(UsbHandle *pdev);
  /* Class Specific Endpoints*/
  uint8_t (*DataIn)(UsbHandle *pdev, uint8_t epnum);
  uint8_t (*DataOut)(UsbHandle *pdev, uint8_t epnum);
  uint8_t (*SOF)(UsbHandle *pdev);
  uint8_t (*IsoINIncomplete)(UsbHandle *pdev, uint8_t epnum);
  uint8_t (*IsoOUTIncomplete)(UsbHandle *pdev, uint8_t epnum);

  uint8_t  *(*GetHSConfigDescriptor)(uint16_t *length);
  uint8_t  *(*GetFSConfigDescriptor)(uint16_t *length);
  uint8_t  *(*GetOtherSpeedConfigDescriptor)(uint16_t *length);
  uint8_t  *(*GetDeviceQualifierDescriptor)(uint16_t *length);
#if (USBD_SUPPORT_USER_STRING_DESC == 1U)
  uint8_t  *(*GetUsrStrDescriptor)(UsbHandle *pdev, uint8_t index,  uint16_t *length);
#endif

};