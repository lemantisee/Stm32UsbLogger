/**
  ******************************************************************************
  * @file    usbd_customhid.c
  * @author  MCD Application Team
  * @brief   This file provides the CUSTOM_HID core functions.
  *
  * @verbatim
  *
  *          ===================================================================
  *                                CUSTOM_HID Class  Description
  *          ===================================================================
  *           This module manages the CUSTOM_HID class V1.11 following the "Device Class Definition
  *           for Human Interface Devices (CUSTOM_HID) Version 1.11 Jun 27, 2001".
  *           This driver implements the following aspects of the specification:
  *             - The Boot Interface Subclass
  *             - Usage Page : Generic Desktop
  *             - Usage : Vendor
  *             - Collection : Application
  *
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *
  *
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbd_customhid.h"

#include <algorithm>

#include "UsbCore.h"

#define CUSTOM_HID_EPIN_ADDR                 0x81U
#define CUSTOM_HID_EPIN_SIZE                 0x40U

#define CUSTOM_HID_EPOUT_ADDR                0x01U
#define CUSTOM_HID_EPOUT_SIZE                0x40U

#define USB_CUSTOM_HID_CONFIG_DESC_SIZ       41U
#define USB_CUSTOM_HID_DESC_SIZ              9U

/**
  * @brief  USBD_CUSTOM_HID_SendReport
  *         Send CUSTOM_HID Report
  * @param  pdev: device instance
  * @param  buff: pointer to report
  * @retval status
  */
uint8_t USBD_CUSTOM_HID_SendReport(UsbHandle *pdev, uint8_t *report, uint16_t len)
{
    USBD_CUSTOM_HID_HandleTypeDef *hhid = (USBD_CUSTOM_HID_HandleTypeDef *)pdev->mClassData;

    if (pdev->isConfigured()) {
        if (hhid->state == CUSTOM_HID_IDLE) {
            hhid->state = CUSTOM_HID_BUSY;
            UsbCore::ref()->transmit(pdev, CUSTOM_HID_EPIN_ADDR, report, len);
        } else {
            return USBD_BUSY;
        }
    }
    return USBD_OK;
}