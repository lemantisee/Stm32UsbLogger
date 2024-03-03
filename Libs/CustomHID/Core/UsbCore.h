#pragma once

#include "usbd_def.h"

#ifndef USBD_DEBUG_LEVEL
#define USBD_DEBUG_LEVEL           0U
#endif /* USBD_DEBUG_LEVEL */

#define USBD_SOF          USBD_LL_SOF

class UsbCore
{
public:
    static UsbCore *ref();
    
    bool init(USBD_HandleTypeDef *pdev, USBD_DescriptorsTypeDef *pdesc, uint8_t id);
    bool start(USBD_HandleTypeDef *pdev);
    bool deinit(USBD_HandleTypeDef *pdev);
    bool stop(USBD_HandleTypeDef *pdev);

    bool registerClass(USBD_HandleTypeDef *pdev, USBD_ClassTypeDef *pclass);
    bool runTestMode(USBD_HandleTypeDef  *pdev);
    bool setClassConfig(USBD_HandleTypeDef  *pdev, uint8_t cfgidx);
    bool clearClassConfig(USBD_HandleTypeDef  *pdev, uint8_t cfgidx);
    void delay(uint32_t ms);

    // Low level

    /**
    * @brief  setupStage
    *         Handle the setup stage
    * @param  pdev: device instance
    * @retval status
    */
    bool setupStage(USBD_HandleTypeDef *pdev, uint8_t *psetup);

    /**
    * @brief  dataOutStage
    *         Handle data OUT stage
    * @param  pdev: device instance
    * @param  epnum: endpoint index
    * @retval status
    */
    bool dataOutStage(USBD_HandleTypeDef *pdev, uint8_t epnum, uint8_t *pdata);

    /**
    * @brief  USBD_DataInStage
    *         Handle data in stage
    * @param  pdev: device instance
    * @param  epnum: endpoint index
    * @retval status
    */
    bool dataInStage(USBD_HandleTypeDef *pdev, uint8_t epnum, uint8_t *pdata);

    bool resetDevice(USBD_HandleTypeDef  *pdev);
    void setSpeed(USBD_HandleTypeDef  *pdev, USBD_SpeedTypeDef speed);
    void suspend(USBD_HandleTypeDef  *pdev);
    void resume(USBD_HandleTypeDef  *pdev);
    void SOF(USBD_HandleTypeDef  *pdev);
    bool isoInIncomplete(USBD_HandleTypeDef  *pdev, uint8_t epnum);
    bool isoOUTIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum);

    bool deviceConnected(USBD_HandleTypeDef  *pdev);
    bool deviceDisconnected(USBD_HandleTypeDef  *pdev);





private:
    UsbCore() = default;
    ~UsbCore() = default;
};

USBD_StatusTypeDef  USBD_LL_Init(USBD_HandleTypeDef *pdev);
USBD_StatusTypeDef  USBD_LL_DeInit(USBD_HandleTypeDef *pdev);
USBD_StatusTypeDef  USBD_LL_Start(USBD_HandleTypeDef *pdev);
USBD_StatusTypeDef  USBD_LL_Stop(USBD_HandleTypeDef *pdev);
USBD_StatusTypeDef  USBD_LL_OpenEP(USBD_HandleTypeDef *pdev,
                                   uint8_t  ep_addr,
                                   uint8_t  ep_type,
                                   uint16_t ep_mps);

USBD_StatusTypeDef  USBD_LL_CloseEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr);
USBD_StatusTypeDef  USBD_LL_FlushEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr);
USBD_StatusTypeDef  USBD_LL_StallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr);
USBD_StatusTypeDef  USBD_LL_ClearStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr);
uint8_t             USBD_LL_IsStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr);
USBD_StatusTypeDef  USBD_LL_SetUSBAddress(USBD_HandleTypeDef *pdev, uint8_t dev_addr);
USBD_StatusTypeDef  USBD_LL_Transmit(USBD_HandleTypeDef *pdev,
                                     uint8_t  ep_addr,
                                     uint8_t  *pbuf,
                                     uint16_t  size);

USBD_StatusTypeDef  USBD_LL_PrepareReceive(USBD_HandleTypeDef *pdev,
                                           uint8_t  ep_addr,
                                           uint8_t  *pbuf,
                                           uint16_t  size);

uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef *pdev, uint8_t  ep_addr);
