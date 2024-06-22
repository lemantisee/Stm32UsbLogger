#include "UsbDriverF103.h"

#include "UsbCustomHid.h"

namespace {
PCD_HandleTypeDef mPcd;
}

extern "C"{
void USB_LP_CAN1_RX0_IRQHandler() { HAL_PCD_IRQHandler(&mPcd); }

void HAL_PCD_MspInit(PCD_HandleTypeDef *pcdHandle)
{
    if (pcdHandle->Instance == USB) {
        __HAL_RCC_USB_CLK_ENABLE();
        HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
    }
}

void HAL_PCD_MspDeInit(PCD_HandleTypeDef *pcdHandle)
{
    if (pcdHandle->Instance == USB) {
        __HAL_RCC_USB_CLK_DISABLE();
        HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
    }
}

} 

bool UsbDriverF103::openEndpoint(uint8_t ep_addr, UsbHandle::EndpointType enType, uint16_t ep_mps)
{
    return HAL_PCD_EP_Open(&mPcd, ep_addr, ep_mps, uint8_t(enType)) == HAL_OK;
}

bool UsbDriverF103::closeEndpoint(uint8_t ep_addr)
{
    return HAL_PCD_EP_Close(&mPcd, ep_addr) == HAL_OK;
}

bool UsbDriverF103::flushEndpoint(uint8_t ep_addr)
{
    return HAL_PCD_EP_Flush(&mPcd, ep_addr) == HAL_OK;
}

bool UsbDriverF103::stallEndpoint(uint8_t ep_addr)
{
    return HAL_PCD_EP_SetStall(&mPcd, ep_addr) == HAL_OK;
}

bool UsbDriverF103::clearStallEndpoint(uint8_t ep_addr)
{
    return HAL_PCD_EP_ClrStall(&mPcd, ep_addr) == HAL_OK;
}

bool UsbDriverF103::isEndpointStall(uint8_t ep_addr) const
{
    if ((ep_addr & usb::defualtEndpointInAddress) == usb::defualtEndpointInAddress) {
        return mPcd.IN_ep[ep_addr & 0x7F].is_stall;
    }

    return mPcd.OUT_ep[ep_addr & 0x7F].is_stall;
}

bool UsbDriverF103::setUsbAddress(uint8_t dev_addr)
{
    return HAL_PCD_SetAddress(&mPcd, dev_addr) == HAL_OK;
}

bool UsbDriverF103::transmit(uint8_t ep_addr, std::span<uint8_t> data) const
{
    return HAL_PCD_EP_Transmit(&mPcd, ep_addr, data.data(), data.size()) == HAL_OK;
}

bool UsbDriverF103::prepareReceive(uint8_t ep_addr, std::span<uint8_t> data)
{
    return HAL_PCD_EP_Receive(&mPcd, ep_addr, data.data(), data.size()) == HAL_OK;
}

uint32_t UsbDriverF103::getRxDataSize(uint8_t ep_addr)
{
    return HAL_PCD_EP_GetRxCount(&mPcd, ep_addr);
}

bool UsbDriverF103::initInterface(UsbHandle *pdev)
{
    #if USE_HAL_PCD_REGISTER_CALLBACKS != 1
    #error "Error. PCD callback not enabled"
    #endif

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


    HAL_PCD_RegisterCallback(&mPcd, HAL_PCD_SOF_CB_ID, UsbDriverF103::PCD_SOFCallback);
    HAL_PCD_RegisterCallback(&mPcd, HAL_PCD_SETUPSTAGE_CB_ID,
                             UsbDriverF103::PCD_SetupStageCallback);
    HAL_PCD_RegisterCallback(&mPcd, HAL_PCD_RESET_CB_ID, UsbDriverF103::PCD_ResetCallback);
    HAL_PCD_RegisterCallback(&mPcd, HAL_PCD_SUSPEND_CB_ID, UsbDriverF103::PCD_SuspendCallback);
    HAL_PCD_RegisterCallback(&mPcd, HAL_PCD_RESUME_CB_ID, UsbDriverF103::PCD_ResumeCallback);
    HAL_PCD_RegisterCallback(&mPcd, HAL_PCD_CONNECT_CB_ID, UsbDriverF103::PCD_ConnectCallback);
    HAL_PCD_RegisterCallback(&mPcd, HAL_PCD_DISCONNECT_CB_ID,
                             UsbDriverF103::PCD_DisconnectCallback);

    HAL_PCD_RegisterDataOutStageCallback(&mPcd, UsbDriverF103::PCD_DataOutStageCallback);
    HAL_PCD_RegisterDataInStageCallback(&mPcd, UsbDriverF103::PCD_DataInStageCallback);
    HAL_PCD_RegisterIsoOutIncpltCallback(&mPcd, UsbDriverF103::PCD_ISOOUTIncompleteCallback);
    HAL_PCD_RegisterIsoInIncpltCallback(&mPcd, UsbDriverF103::PCD_ISOINIncompleteCallback);

    //endpoint configuration
    HAL_PCDEx_PMAConfig(&mPcd, usb::defualtEndpointOutAddress, PCD_SNG_BUF, 0x18);
    HAL_PCDEx_PMAConfig(&mPcd, usb::defualtEndpointInAddress, PCD_SNG_BUF, 0x58);

    HAL_PCDEx_PMAConfig(&mPcd, UsbCustomHid::endpointInAddress, PCD_SNG_BUF, 0x98);
    HAL_PCDEx_PMAConfig(&mPcd, UsbCustomHid::endpointOutAddress, PCD_SNG_BUF, 0xD8);
    return true;
}

bool UsbDriverF103::deinitInterface() { return HAL_PCD_DeInit(&mPcd) == HAL_OK; }

bool UsbDriverF103::startInterface() { return HAL_PCD_Start(&mPcd) == HAL_OK; }

bool UsbDriverF103::stopInterface() { return HAL_PCD_Stop(&mPcd) == HAL_OK; }

void UsbDriverF103::PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
{
    UsbHandle *handle = (UsbHandle *)hpcd->pData;
    handle->setup((uint8_t *)hpcd->Setup);
}

void UsbDriverF103::PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
    UsbHandle *handle = (UsbHandle *)hpcd->pData;
    handle->dataOutStage(epnum, hpcd->OUT_ep[epnum].xfer_buff);
}

void UsbDriverF103::PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
    UsbHandle *handle = (UsbHandle *)hpcd->pData;
    handle->dataInStage(epnum, hpcd->IN_ep[epnum].xfer_buff);
}

void UsbDriverF103::PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
{
    UsbHandle *handle = (UsbHandle *)hpcd->pData;
    handle->sof();
}

void UsbDriverF103::PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
{
    if (hpcd->Init.speed != PCD_SPEED_FULL) {
        return;
    }

    UsbHandle *handle = (UsbHandle *)hpcd->pData;
    handle->resetUsb();
}

void UsbDriverF103::PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
{
    UsbHandle *handle = (UsbHandle *)hpcd->pData;
    handle->suspend();
    /* Enter in STOP mode. */
    if (hpcd->Init.low_power_enable) {
        /* Set SLEEPDEEP bit and SleepOnExit of Cortex System Control Register. */
        SCB->SCR |= (uint32_t)((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
    }
}

void UsbDriverF103::PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
{
    UsbHandle *handle = (UsbHandle *)hpcd->pData;
    handle->resume();
}

void UsbDriverF103::PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
    UsbHandle *handle = (UsbHandle *)hpcd->pData;
    handle->isoOUTIncomplete(epnum);
}

void UsbDriverF103::PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
    UsbHandle *handle = (UsbHandle *)hpcd->pData;
    handle->isoInIncomplete(epnum);
}

void UsbDriverF103::PCD_ConnectCallback(PCD_HandleTypeDef *hpcd) {}

void UsbDriverF103::PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
{
    UsbHandle *handle = (UsbHandle *)hpcd->pData;
    handle->disconnect();
}

void UsbDriverF103::PCDEx_SetConnectionState(PCD_HandleTypeDef *hpcd, uint8_t state)
{
    if (state == 1) {
        /* Configure Low connection state. */
        return;
    }

    /* Configure High connection state. */
}