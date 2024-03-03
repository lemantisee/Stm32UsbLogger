#include "UsbCoreF103.h"

#include "usbd_customhid.h"

PCD_HandleTypeDef UsbCoreF103::mPcd;

PCD_HandleTypeDef &UsbCoreF103::getPcdHandle()
{
    return mPcd;
}

bool UsbCoreF103::openEndpoint(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t ep_type, uint16_t ep_mps)
{
  return HAL_PCD_EP_Open((PCD_HandleTypeDef*) pdev->pData, ep_addr, ep_mps, ep_type) == HAL_OK;
}

bool UsbCoreF103::closeEndpoint(USBD_HandleTypeDef *pdev, uint8_t  ep_addr)
{
    return HAL_PCD_EP_Close((PCD_HandleTypeDef*) pdev->pData, ep_addr) == HAL_OK;
}

bool UsbCoreF103::flushEndpoint(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    return HAL_PCD_EP_Flush((PCD_HandleTypeDef*) pdev->pData, ep_addr) == HAL_OK;
}

bool UsbCoreF103::stallEndpoint(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    return HAL_PCD_EP_SetStall((PCD_HandleTypeDef*) pdev->pData, ep_addr) == HAL_OK;
}

bool UsbCoreF103::clearStallEndpoint(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    return HAL_PCD_EP_ClrStall((PCD_HandleTypeDef*) pdev->pData, ep_addr) == HAL_OK;
}

bool UsbCoreF103::isEndpointStall(USBD_HandleTypeDef *pdev, uint8_t ep_addr) const
{
    const PCD_HandleTypeDef *hpcd = (const PCD_HandleTypeDef*) pdev->pData;

  if ((ep_addr & 0x80) == 0x80) {
    return hpcd->IN_ep[ep_addr & 0x7F].is_stall;
  }

  return hpcd->OUT_ep[ep_addr & 0x7F].is_stall;
}

bool UsbCoreF103::setUsbAddress(USBD_HandleTypeDef *pdev, uint8_t dev_addr)
{
    return HAL_PCD_SetAddress((PCD_HandleTypeDef*) pdev->pData, dev_addr) == HAL_OK;
}

bool UsbCoreF103::transmit(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint16_t size)
{
    return HAL_PCD_EP_Transmit((PCD_HandleTypeDef*) pdev->pData, ep_addr, pbuf, size) == HAL_OK;
}

bool UsbCoreF103::prepareReceive(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint16_t size)
{
    return HAL_PCD_EP_Receive((PCD_HandleTypeDef*) pdev->pData, ep_addr, pbuf, size) == HAL_OK;
}

uint32_t UsbCoreF103::getRxDataSize(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    return HAL_PCD_EP_GetRxCount((PCD_HandleTypeDef*) pdev->pData, ep_addr);
}

uint32_t *UsbCoreF103::malloc(uint32_t size)
{
    static uint32_t mem[(sizeof(USBD_CUSTOM_HID_HandleTypeDef)/4+1)];/* On 32-bit boundary */
    return mem;
}

void UsbCoreF103::free(void *)
{
}

bool UsbCoreF103::initInterface(USBD_HandleTypeDef *pdev)
{
  /* Init USB Ip. */
  /* Link the driver to the stack. */
    pdev->pData = &mPcd;
  
  mPcd.pData = pdev;
  mPcd.Instance = USB;
  mPcd.Init.dev_endpoints = 8;
  mPcd.Init.speed = PCD_SPEED_FULL;
  mPcd.Init.low_power_enable = DISABLE;
  mPcd.Init.lpm_enable = DISABLE;
  mPcd.Init.battery_charging_enable = DISABLE;
  if (HAL_PCD_Init(&mPcd) != HAL_OK) {
    return false;
  }

#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
  /* Register USB PCD CallBacks */
  HAL_PCD_RegisterCallback(&hpcd_USB_FS, HAL_PCD_SOF_CB_ID, PCD_SOFCallback);
  HAL_PCD_RegisterCallback(&hpcd_USB_FS, HAL_PCD_SETUPSTAGE_CB_ID, PCD_SetupStageCallback);
  HAL_PCD_RegisterCallback(&hpcd_USB_FS, HAL_PCD_RESET_CB_ID, PCD_ResetCallback);
  HAL_PCD_RegisterCallback(&hpcd_USB_FS, HAL_PCD_SUSPEND_CB_ID, PCD_SuspendCallback);
  HAL_PCD_RegisterCallback(&hpcd_USB_FS, HAL_PCD_RESUME_CB_ID, PCD_ResumeCallback);
  HAL_PCD_RegisterCallback(&hpcd_USB_FS, HAL_PCD_CONNECT_CB_ID, PCD_ConnectCallback);
  HAL_PCD_RegisterCallback(&hpcd_USB_FS, HAL_PCD_DISCONNECT_CB_ID, PCD_DisconnectCallback);

  HAL_PCD_RegisterDataOutStageCallback(&hpcd_USB_FS, PCD_DataOutStageCallback);
  HAL_PCD_RegisterDataInStageCallback(&hpcd_USB_FS, PCD_DataInStageCallback);
  HAL_PCD_RegisterIsoOutIncpltCallback(&hpcd_USB_FS, PCD_ISOOUTIncompleteCallback);
  HAL_PCD_RegisterIsoInIncpltCallback(&hpcd_USB_FS, PCD_ISOINIncompleteCallback);
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
  /* USER CODE BEGIN EndPoint_Configuration */
  HAL_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData , 0x00 , PCD_SNG_BUF, 0x18);
  HAL_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData , 0x80 , PCD_SNG_BUF, 0x58);
  /* USER CODE END EndPoint_Configuration */
  /* USER CODE BEGIN EndPoint_Configuration_CUSTOM_HID */
  HAL_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData , CUSTOM_HID_EPIN_ADDR , PCD_SNG_BUF, 0x98);
  HAL_PCDEx_PMAConfig((PCD_HandleTypeDef*)pdev->pData , CUSTOM_HID_EPOUT_ADDR , PCD_SNG_BUF, 0xD8);
  /* USER CODE END EndPoint_Configuration_CUSTOM_HID */
  return true;
}

bool UsbCoreF103::deinitInterface(USBD_HandleTypeDef *pdev)
{
  return HAL_PCD_DeInit((PCD_HandleTypeDef*) pdev->pData) == HAL_OK;
}

bool UsbCoreF103::startInterface(USBD_HandleTypeDef *pdev)
{
  return HAL_PCD_Start((PCD_HandleTypeDef*) pdev->pData) == HAL_OK;
}

bool UsbCoreF103::stopInterface(USBD_HandleTypeDef *pdev)
{
  return HAL_PCD_Stop((PCD_HandleTypeDef*) pdev->pData) == HAL_OK;
}
