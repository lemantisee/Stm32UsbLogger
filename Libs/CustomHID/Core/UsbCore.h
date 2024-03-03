#pragma once

#include "usbd_def.h"

#include <stdint.h>

#ifndef USBD_DEBUG_LEVEL
#define USBD_DEBUG_LEVEL           0U
#endif /* USBD_DEBUG_LEVEL */

#define USBD_SOF          USBD_LL_SOF

class UsbCore
{
public:
    UsbCore() = default;
    virtual ~UsbCore() = default;

    static void setImpl(UsbCore *impl);
    static UsbCore *ref();
    
    bool init(USBD_HandleTypeDef *pdev, USBD_DescriptorsTypeDef *pdesc, uint8_t id);
    bool start(USBD_HandleTypeDef *pdev);
    bool deinit(USBD_HandleTypeDef *pdev);
    bool stop(USBD_HandleTypeDef *pdev);

    bool registerClass(USBD_HandleTypeDef *pdev, USBD_ClassTypeDef *pclass);
    bool runTestMode(USBD_HandleTypeDef  *pdev);
    bool setClassConfig(USBD_HandleTypeDef  *pdev, uint8_t cfgidx);
    bool clearClassConfig(USBD_HandleTypeDef  *pdev, uint8_t cfgidx);

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

    virtual bool openEndpoint(USBD_HandleTypeDef *pdev,
                                   uint8_t  ep_addr,
                                   uint8_t  ep_type,
                                   uint16_t ep_mps) = 0;

    virtual bool closeEndpoint(USBD_HandleTypeDef *pdev, uint8_t  ep_addr) = 0;
    virtual bool flushEndpoint(USBD_HandleTypeDef *pdev, uint8_t ep_addr) = 0;
    virtual bool stallEndpoint(USBD_HandleTypeDef *pdev, uint8_t ep_addr) = 0;
    virtual bool clearStallEndpoint(USBD_HandleTypeDef *pdev, uint8_t ep_addr) = 0;
    virtual bool isEndpointStall(USBD_HandleTypeDef *pdev, uint8_t ep_addr) const = 0;
    virtual bool setUsbAddress(USBD_HandleTypeDef *pdev, uint8_t dev_addr) = 0;
    virtual bool transmit(USBD_HandleTypeDef *pdev,
                                     uint8_t  ep_addr,
                                     uint8_t  *pbuf,
                                     uint16_t  size) = 0;

    virtual bool prepareReceive(USBD_HandleTypeDef *pdev,
                                    uint8_t  ep_addr,
                                    uint8_t  *pbuf,
                                    uint16_t  size) = 0;

    virtual uint32_t getRxDataSize(USBD_HandleTypeDef *pdev, uint8_t  ep_addr) = 0;

    virtual uint32_t *malloc(uint32_t size) = 0;
    virtual void free(void *mem) = 0;

protected:
    virtual bool initInterface(USBD_HandleTypeDef *pdev) = 0;
    virtual bool deinitInterface(USBD_HandleTypeDef *pdev) = 0;
    virtual bool startInterface(USBD_HandleTypeDef *pdev) = 0;
    virtual bool stopInterface(USBD_HandleTypeDef *pdev) = 0;

private:
};

// USBD_StatusTypeDef  USBD_LL_Init(USBD_HandleTypeDef *pdev);
