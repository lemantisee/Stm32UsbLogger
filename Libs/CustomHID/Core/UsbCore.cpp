#include "UsbCore.h"

#include "usbd_conf.h"
#include "usbd_def.h"
#include "usbd_ioreq.h"
#include "usbd_ctlreq.h"

UsbCore *UsbCore::ref()
{
    static UsbCore core;
    return &core;
}

bool UsbCore::init(USBD_HandleTypeDef *pdev, USBD_DescriptorsTypeDef *pdesc, uint8_t id)
{
    if (!pdev)
    {
        return false;
    }

    if (pdev->pClass)
    {
        pdev->pClass = nullptr;
    }

         if (pdesc)
    {
        pdev->pDesc = pdesc;
    }

    pdev->dev_state = USBD_STATE_DEFAULT;
    pdev->id = id;

    return USBD_LL_Init(pdev) == USBD_OK;
}

bool UsbCore::start(USBD_HandleTypeDef *pdev)
{
    return USBD_LL_Start(pdev) == USBD_OK;
}

bool UsbCore::deinit(USBD_HandleTypeDef *pdev)
{
  pdev->dev_state = USBD_STATE_DEFAULT;
  pdev->pClass->DeInit(pdev, (uint8_t)pdev->dev_config);

  if (USBD_LL_Stop(pdev) != USBD_OK) {
    return false;
  }

  return USBD_LL_DeInit(pdev) == USBD_OK;
}

bool UsbCore::stop(USBD_HandleTypeDef *pdev)
{
  pdev->pClass->DeInit(pdev, (uint8_t)pdev->dev_config);

  return USBD_LL_Stop(pdev) == USBD_OK;
}

bool UsbCore::registerClass(USBD_HandleTypeDef *pdev, USBD_ClassTypeDef *pclass)
{
    if(!pclass) {
        return false;
    }

    pdev->pClass = pclass;
    return true;
}

bool UsbCore::runTestMode(USBD_HandleTypeDef *pdev)
{
    return true;
}

bool UsbCore::setClassConfig(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    if(!pdev->pClass) {
        return false;
    }

    return pdev->pClass->Init(pdev, cfgidx) == USBD_OK;
}

bool UsbCore::clearClassConfig(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    return pdev->pClass->DeInit(pdev, cfgidx) == USBD_OK;
}

void UsbCore::delay(uint32_t ms)
{
    HAL_Delay(ms);
}

bool UsbCore::setupStage(USBD_HandleTypeDef *pdev, uint8_t *psetup)
{
  USBD_ParseSetupRequest(&pdev->request, psetup);

  pdev->ep0_state = USBD_EP0_SETUP;
  pdev->ep0_data_len = pdev->request.wLength;

  switch (pdev->request.bmRequest & 0x1FU)
  {
    case USB_REQ_RECIPIENT_DEVICE:
      USBD_StdDevReq(pdev, &pdev->request);
      break;

    case USB_REQ_RECIPIENT_INTERFACE:
      USBD_StdItfReq(pdev, &pdev->request);
      break;

    case USB_REQ_RECIPIENT_ENDPOINT:
      USBD_StdEPReq(pdev, &pdev->request);
      break;

    default:
      USBD_LL_StallEP(pdev, (pdev->request.bmRequest & 0x80U));
      break;
  }

  return true;
}

bool UsbCore::dataOutStage(USBD_HandleTypeDef *pdev, uint8_t epnum, uint8_t *pdata)
{
  USBD_EndpointTypeDef *pep;

  if (epnum == 0U)
  {
    pep = &pdev->ep_out[0];

    if (pdev->ep0_state == USBD_EP0_DATA_OUT)
    {
      if (pep->rem_length > pep->maxpacket)
      {
        pep->rem_length -= pep->maxpacket;

        USBD_CtlContinueRx(pdev, pdata,
                           (uint16_t)MIN(pep->rem_length, pep->maxpacket));
      }
      else
      {
        if ((pdev->pClass->EP0_RxReady != NULL) &&
            (pdev->dev_state == USBD_STATE_CONFIGURED))
        {
          pdev->pClass->EP0_RxReady(pdev);
        }
        USBD_CtlSendStatus(pdev);
      }
    }
    else
    {
      if (pdev->ep0_state == USBD_EP0_STATUS_OUT)
      {
        /*
         * STATUS PHASE completed, update ep0_state to idle
         */
        pdev->ep0_state = USBD_EP0_IDLE;
        USBD_LL_StallEP(pdev, 0U);
      }
    }
  }
  else if ((pdev->pClass->DataOut != NULL) &&
           (pdev->dev_state == USBD_STATE_CONFIGURED))
  {
    pdev->pClass->DataOut(pdev, epnum);
  }
  else
  {
    /* should never be in this condition */
    return false;
  }

  return true;
}

bool UsbCore::dataInStage(USBD_HandleTypeDef *pdev, uint8_t epnum, uint8_t *pdata)
{
  USBD_EndpointTypeDef *pep;

  if (epnum == 0U)
  {
    pep = &pdev->ep_in[0];

    if (pdev->ep0_state == USBD_EP0_DATA_IN)
    {
      if (pep->rem_length > pep->maxpacket)
      {
        pep->rem_length -= pep->maxpacket;

        USBD_CtlContinueSendData(pdev, pdata, (uint16_t)pep->rem_length);

        /* Prepare endpoint for premature end of transfer */
        USBD_LL_PrepareReceive(pdev, 0U, NULL, 0U);
      }
      else
      {
        /* last packet is MPS multiple, so send ZLP packet */
        if ((pep->total_length % pep->maxpacket == 0U) &&
            (pep->total_length >= pep->maxpacket) &&
            (pep->total_length < pdev->ep0_data_len))
        {
          USBD_CtlContinueSendData(pdev, NULL, 0U);
          pdev->ep0_data_len = 0U;

          /* Prepare endpoint for premature end of transfer */
          USBD_LL_PrepareReceive(pdev, 0U, NULL, 0U);
        }
        else
        {
          if ((pdev->pClass->EP0_TxSent != NULL) &&
              (pdev->dev_state == USBD_STATE_CONFIGURED))
          {
            pdev->pClass->EP0_TxSent(pdev);
          }
          USBD_LL_StallEP(pdev, 0x80U);
          USBD_CtlReceiveStatus(pdev);
        }
      }
    }
    else
    {
      if ((pdev->ep0_state == USBD_EP0_STATUS_IN) ||
          (pdev->ep0_state == USBD_EP0_IDLE))
      {
        USBD_LL_StallEP(pdev, 0x80U);
      }
    }

    if (pdev->dev_test_mode == 1U)
    {
      UsbCore::runTestMode(pdev);
      pdev->dev_test_mode = 0U;
    }
  }
  else if (pdev->pClass->DataIn && pdev->dev_state == USBD_STATE_CONFIGURED)
  {
    pdev->pClass->DataIn(pdev, epnum);
  }
  else
  {
    /* should never be in this condition */
    return false;
  }

  return true;
}

bool UsbCore::resetDevice(USBD_HandleTypeDef *pdev)
{
  /* Open EP0 OUT */
  USBD_LL_OpenEP(pdev, 0x00U, USBD_EP_TYPE_CTRL, USB_MAX_EP0_SIZE);
  pdev->ep_out[0x00U & 0xFU].is_used = 1U;

  pdev->ep_out[0].maxpacket = USB_MAX_EP0_SIZE;

  /* Open EP0 IN */
  USBD_LL_OpenEP(pdev, 0x80U, USBD_EP_TYPE_CTRL, USB_MAX_EP0_SIZE);
  pdev->ep_in[0x80U & 0xFU].is_used = 1U;

  pdev->ep_in[0].maxpacket = USB_MAX_EP0_SIZE;

  /* Upon Reset call user call back */
  pdev->dev_state = USBD_STATE_DEFAULT;
  pdev->ep0_state = USBD_EP0_IDLE;
  pdev->dev_config = 0U;
  pdev->dev_remote_wakeup = 0U;

  return pdev->pClassData ? 
        pdev->pClass->DeInit(pdev, (uint8_t)pdev->dev_config) == USBD_OK : true;
}

void UsbCore::setSpeed(USBD_HandleTypeDef *pdev, USBD_SpeedTypeDef speed)
{
    pdev->dev_speed = speed;
}

void UsbCore::suspend(USBD_HandleTypeDef *pdev)
{
  pdev->dev_old_state =  pdev->dev_state;
  pdev->dev_state  = USBD_STATE_SUSPENDED;
}

void UsbCore::resume(USBD_HandleTypeDef *pdev)
{
    if (pdev->dev_state == USBD_STATE_SUSPENDED) {
        pdev->dev_state = pdev->dev_old_state;
    }
}

void UsbCore::SOF(USBD_HandleTypeDef *pdev)
{
    if (pdev->dev_state == USBD_STATE_CONFIGURED) {
        if (pdev->pClass->SOF) {
            pdev->pClass->SOF(pdev);
        }
    }
}

bool UsbCore::isoInIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    return true;
}

bool UsbCore::isoOUTIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    return true;
}

bool UsbCore::deviceConnected(USBD_HandleTypeDef *pdev)
{
    return true;
}

bool UsbCore::deviceDisconnected(USBD_HandleTypeDef *pdev)
{
    pdev->dev_state = USBD_STATE_DEFAULT;
    return pdev->pClass->DeInit(pdev, (uint8_t)pdev->dev_config) == USBD_OK;
}
