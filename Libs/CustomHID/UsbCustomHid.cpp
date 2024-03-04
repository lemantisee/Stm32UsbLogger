#include "UsbCustomHid.h"

#include <algorithm>

#include "usbd_customhid.h"
#include "UsbCore.h"

namespace {

/* USB CUSTOM_HID device FS Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CUSTOM_HID_CfgFSDesc[USB_CUSTOM_HID_CONFIG_DESC_SIZ] __ALIGN_END
    = {
        0x09,                        /* bLength: Configuration Descriptor size */
        USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
        USB_CUSTOM_HID_CONFIG_DESC_SIZ,
        /* wTotalLength: Bytes returned */
        0x00, 0x01, /*bNumInterfaces: 1 interface*/
        0x01,       /*bConfigurationValue: Configuration value*/
        0x00,       /*iConfiguration: Index of string descriptor describing
  the configuration*/
        0xC0,       /*bmAttributes: bus powered */
        0x32,       /*MaxPower 100 mA: this current is used for detecting Vbus*/

        /************** Descriptor of CUSTOM HID interface ****************/
        /* 09 */
        0x09,                    /*bLength: Interface Descriptor size*/
        USB_DESC_TYPE_INTERFACE, /*bDescriptorType: Interface descriptor type*/
        0x00,                    /*bInterfaceNumber: Number of Interface*/
        0x00,                    /*bAlternateSetting: Alternate setting*/
        0x02,                    /*bNumEndpoints*/
        0x03,                    /*bInterfaceClass: CUSTOM_HID*/
        0x00,                    /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
        0x00,                    /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
        0,                       /*iInterface: Index of string descriptor*/
        /******************** Descriptor of CUSTOM_HID *************************/
        /* 18 */
        0x09,                       /*bLength: CUSTOM_HID Descriptor size*/
        CUSTOM_HID_DESCRIPTOR_TYPE, /*bDescriptorType: CUSTOM_HID*/
        0x11,                       /*bCUSTOM_HIDUSTOM_HID: CUSTOM_HID Class Spec release number*/
        0x01, 0x00,                 /*bCountryCode: Hardware target country*/
        0x01, /*bNumDescriptors: Number of CUSTOM_HID class descriptors to follow*/
        0x22, /*bDescriptorType*/
        USBD_CUSTOM_HID_REPORT_DESC_SIZE, /*wItemLength: Total length of Report descriptor*/
        0x00,
        /******************** Descriptor of Custom HID endpoints ********************/
        /* 27 */
        0x07,                   /*bLength: Endpoint Descriptor size*/
        USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

        CUSTOM_HID_EPIN_ADDR,          /*bEndpointAddress: Endpoint Address (IN)*/
        0x03,                          /*bmAttributes: Interrupt endpoint*/
        CUSTOM_HID_EPIN_SIZE,          /*wMaxPacketSize: 2 Byte max */
        0x00, CUSTOM_HID_FS_BINTERVAL, /*bInterval: Polling Interval */
        /* 34 */

        0x07,                          /* bLength: Endpoint Descriptor size */
        USB_DESC_TYPE_ENDPOINT,        /* bDescriptorType: */
        CUSTOM_HID_EPOUT_ADDR,         /*bEndpointAddress: Endpoint Address (OUT)*/
        0x03,                          /* bmAttributes: Interrupt endpoint */
        CUSTOM_HID_EPOUT_SIZE,         /* wMaxPacketSize: 2 Bytes max  */
        0x00, CUSTOM_HID_FS_BINTERVAL, /* bInterval: Polling Interval */
                                       /* 41 */
};

/* USB CUSTOM_HID device HS Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CUSTOM_HID_CfgHSDesc[USB_CUSTOM_HID_CONFIG_DESC_SIZ] __ALIGN_END
    = {
        0x09,                        /* bLength: Configuration Descriptor size */
        USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
        USB_CUSTOM_HID_CONFIG_DESC_SIZ,
        /* wTotalLength: Bytes returned */
        0x00, 0x01, /*bNumInterfaces: 1 interface*/
        0x01,       /*bConfigurationValue: Configuration value*/
        0x00,       /*iConfiguration: Index of string descriptor describing
  the configuration*/
        0xC0,       /*bmAttributes: bus powered */
        0x32,       /*MaxPower 100 mA: this current is used for detecting Vbus*/

        /************** Descriptor of CUSTOM HID interface ****************/
        /* 09 */
        0x09,                    /*bLength: Interface Descriptor size*/
        USB_DESC_TYPE_INTERFACE, /*bDescriptorType: Interface descriptor type*/
        0x00,                    /*bInterfaceNumber: Number of Interface*/
        0x00,                    /*bAlternateSetting: Alternate setting*/
        0x02,                    /*bNumEndpoints*/
        0x03,                    /*bInterfaceClass: CUSTOM_HID*/
        0x00,                    /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
        0x00,                    /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
        0,                       /*iInterface: Index of string descriptor*/
        /******************** Descriptor of CUSTOM_HID *************************/
        /* 18 */
        0x09,                       /*bLength: CUSTOM_HID Descriptor size*/
        CUSTOM_HID_DESCRIPTOR_TYPE, /*bDescriptorType: CUSTOM_HID*/
        0x11,                       /*bCUSTOM_HIDUSTOM_HID: CUSTOM_HID Class Spec release number*/
        0x01, 0x00,                 /*bCountryCode: Hardware target country*/
        0x01, /*bNumDescriptors: Number of CUSTOM_HID class descriptors to follow*/
        0x22, /*bDescriptorType*/
        USBD_CUSTOM_HID_REPORT_DESC_SIZE, /*wItemLength: Total length of Report descriptor*/
        0x00,
        /******************** Descriptor of Custom HID endpoints ********************/
        /* 27 */
        0x07,                   /*bLength: Endpoint Descriptor size*/
        USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

        CUSTOM_HID_EPIN_ADDR,          /*bEndpointAddress: Endpoint Address (IN)*/
        0x03,                          /*bmAttributes: Interrupt endpoint*/
        CUSTOM_HID_EPIN_SIZE,          /*wMaxPacketSize: 2 Byte max */
        0x00, CUSTOM_HID_HS_BINTERVAL, /*bInterval: Polling Interval */
        /* 34 */

        0x07,                          /* bLength: Endpoint Descriptor size */
        USB_DESC_TYPE_ENDPOINT,        /* bDescriptorType: */
        CUSTOM_HID_EPOUT_ADDR,         /*bEndpointAddress: Endpoint Address (OUT)*/
        0x03,                          /* bmAttributes: Interrupt endpoint */
        CUSTOM_HID_EPOUT_SIZE,         /* wMaxPacketSize: 2 Bytes max  */
        0x00, CUSTOM_HID_HS_BINTERVAL, /* bInterval: Polling Interval */
                                       /* 41 */
};

/* USB CUSTOM_HID device Other Speed Configuration Descriptor */
__ALIGN_BEGIN static uint8_t
    USBD_CUSTOM_HID_OtherSpeedCfgDesc[USB_CUSTOM_HID_CONFIG_DESC_SIZ] __ALIGN_END
    = {
        0x09,                        /* bLength: Configuration Descriptor size */
        USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
        USB_CUSTOM_HID_CONFIG_DESC_SIZ,
        /* wTotalLength: Bytes returned */
        0x00, 0x01, /*bNumInterfaces: 1 interface*/
        0x01,       /*bConfigurationValue: Configuration value*/
        0x00,       /*iConfiguration: Index of string descriptor describing
  the configuration*/
        0xC0,       /*bmAttributes: bus powered */
        0x32,       /*MaxPower 100 mA: this current is used for detecting Vbus*/

        /************** Descriptor of CUSTOM HID interface ****************/
        /* 09 */
        0x09,                    /*bLength: Interface Descriptor size*/
        USB_DESC_TYPE_INTERFACE, /*bDescriptorType: Interface descriptor type*/
        0x00,                    /*bInterfaceNumber: Number of Interface*/
        0x00,                    /*bAlternateSetting: Alternate setting*/
        0x02,                    /*bNumEndpoints*/
        0x03,                    /*bInterfaceClass: CUSTOM_HID*/
        0x00,                    /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
        0x00,                    /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
        0,                       /*iInterface: Index of string descriptor*/
        /******************** Descriptor of CUSTOM_HID *************************/
        /* 18 */
        0x09,                       /*bLength: CUSTOM_HID Descriptor size*/
        CUSTOM_HID_DESCRIPTOR_TYPE, /*bDescriptorType: CUSTOM_HID*/
        0x11,                       /*bCUSTOM_HIDUSTOM_HID: CUSTOM_HID Class Spec release number*/
        0x01, 0x00,                 /*bCountryCode: Hardware target country*/
        0x01, /*bNumDescriptors: Number of CUSTOM_HID class descriptors to follow*/
        0x22, /*bDescriptorType*/
        USBD_CUSTOM_HID_REPORT_DESC_SIZE, /*wItemLength: Total length of Report descriptor*/
        0x00,
        /******************** Descriptor of Custom HID endpoints ********************/
        /* 27 */
        0x07,                   /*bLength: Endpoint Descriptor size*/
        USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

        CUSTOM_HID_EPIN_ADDR,          /*bEndpointAddress: Endpoint Address (IN)*/
        0x03,                          /*bmAttributes: Interrupt endpoint*/
        CUSTOM_HID_EPIN_SIZE,          /*wMaxPacketSize: 2 Byte max */
        0x00, CUSTOM_HID_FS_BINTERVAL, /*bInterval: Polling Interval */
        /* 34 */

        0x07,                          /* bLength: Endpoint Descriptor size */
        USB_DESC_TYPE_ENDPOINT,        /* bDescriptorType: */
        CUSTOM_HID_EPOUT_ADDR,         /*bEndpointAddress: Endpoint Address (OUT)*/
        0x03,                          /* bmAttributes: Interrupt endpoint */
        CUSTOM_HID_EPOUT_SIZE,         /* wMaxPacketSize: 2 Bytes max  */
        0x00, CUSTOM_HID_FS_BINTERVAL, /* bInterval: Polling Interval */
                                       /* 41 */
};

/* USB CUSTOM_HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CUSTOM_HID_Desc[USB_CUSTOM_HID_DESC_SIZ] __ALIGN_END = {
    /* 18 */
    0x09,                       /*bLength: CUSTOM_HID Descriptor size*/
    CUSTOM_HID_DESCRIPTOR_TYPE, /*bDescriptorType: CUSTOM_HID*/
    0x11,                       /*bCUSTOM_HIDUSTOM_HID: CUSTOM_HID Class Spec release number*/
    0x01,
    0x00, /*bCountryCode: Hardware target country*/
    0x01, /*bNumDescriptors: Number of CUSTOM_HID class descriptors to follow*/
    0x22, /*bDescriptorType*/
    USBD_CUSTOM_HID_REPORT_DESC_SIZE, /*wItemLength: Total length of Report descriptor*/
    0x00,
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t
    USBD_CUSTOM_HID_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END
    = {
        USB_LEN_DEV_QUALIFIER_DESC,
        USB_DESC_TYPE_DEVICE_QUALIFIER,
        0x00,
        0x02,
        0x00,
        0x00,
        0x00,
        0x40,
        0x01,
        0x00,
};
} // namespace

bool UsbCustomHid::Init(UsbHandle *pdev, uint8_t cfgidx)
{
    uint8_t ret = 0U;

    /* Open EP IN */
    UsbCore::ref()->openEndpoint(pdev, CUSTOM_HID_EPIN_ADDR, UsbHandle::EndpointInterrupt,
                                 CUSTOM_HID_EPIN_SIZE);

    pdev->ep_in[CUSTOM_HID_EPIN_ADDR & 0xFU].is_used = true;

    /* Open EP OUT */
    UsbCore::ref()->openEndpoint(pdev, CUSTOM_HID_EPOUT_ADDR, UsbHandle::EndpointInterrupt,
                                 CUSTOM_HID_EPOUT_SIZE);

    pdev->ep_out[CUSTOM_HID_EPOUT_ADDR & 0xFU].is_used = true;

    pdev->mClassData = UsbCore::ref()->malloc(sizeof(USBD_CUSTOM_HID_HandleTypeDef));

    if (pdev->mClassData == nullptr) {
        return false;
    }

    USBD_CUSTOM_HID_HandleTypeDef *hhid = (USBD_CUSTOM_HID_HandleTypeDef *)pdev->mClassData;

    hhid->state = CUSTOM_HID_IDLE;
    ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData)->Init();

    /* Prepare Out endpoint to receive 1st packet */
    UsbCore::ref()->prepareReceive(pdev, CUSTOM_HID_EPOUT_ADDR, hhid->Report_buf,
                                   USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);

    return true;
}

bool UsbCustomHid::DeInit(UsbHandle *pdev, uint8_t cfgidx)
{ /* Close CUSTOM_HID EP IN */
    UsbCore::ref()->closeEndpoint(pdev, CUSTOM_HID_EPIN_ADDR);
    pdev->ep_in[CUSTOM_HID_EPIN_ADDR & 0xFU].is_used = 0U;

    /* Close CUSTOM_HID EP OUT */
    UsbCore::ref()->closeEndpoint(pdev, CUSTOM_HID_EPOUT_ADDR);
    pdev->ep_out[CUSTOM_HID_EPOUT_ADDR & 0xFU].is_used = 0U;

    /* FRee allocated memory */
    if (pdev->mClassData) {
        ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData)->DeInit();
        UsbCore::ref()->free(pdev->mClassData);
        pdev->mClassData = nullptr;
    }

    return true;
}

bool UsbCustomHid::Setup(UsbHandle *pdev, USBD_SetupReqTypedef *req)
{
    USBD_CUSTOM_HID_HandleTypeDef *hhid = (USBD_CUSTOM_HID_HandleTypeDef *)pdev->mClassData;
    uint16_t len = 0U;
    uint8_t *pbuf = NULL;
    uint16_t status_info = 0U;
    uint8_t ret = USBD_OK;

    switch (req->getRequestType()) {
    case USBD_SetupReqTypedef::RequestClass:
        switch (req->getRequest()) {
        case CUSTOM_HID_REQ_SET_PROTOCOL: hhid->Protocol = req->getProtocol(); break;

        case CUSTOM_HID_REQ_GET_PROTOCOL:
            pdev->sendData((uint8_t *)(void *)&hhid->Protocol, 1U);
            break;

        case CUSTOM_HID_REQ_SET_IDLE: hhid->IdleState = req->getIdleState(); break;

        case CUSTOM_HID_REQ_GET_IDLE:
            pdev->sendData((uint8_t *)(void *)&hhid->IdleState, 1U);
            break;

        case CUSTOM_HID_REQ_SET_REPORT:
            hhid->IsReportAvailable = 1U;
            pdev->prepareRx(hhid->Report_buf, req->getLength());
            break;

        default:
            pdev->stallEndpoints();
            ret = USBD_FAIL;
            break;
        }
        break;

    case USBD_SetupReqTypedef::RequestStandart:
        switch (req->getRequest()) {
        case USBD_SetupReqTypedef::RequestGetStatus:
            if (pdev->isConfigured()) {
                pdev->sendData((uint8_t *)(void *)&status_info, 2U);
            } else {
                pdev->stallEndpoints();
                ret = USBD_FAIL;
            }
            break;

        case USBD_SetupReqTypedef::RequestGetDescriptor:
            if (req->getDescriptorType() == CUSTOM_HID_REPORT_DESC) {
                len = std::min<uint16_t>(USBD_CUSTOM_HID_REPORT_DESC_SIZE, req->getLength());
                pbuf = ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData)->pReport;
            } else {
                if (req->getDescriptorType() == CUSTOM_HID_DESCRIPTOR_TYPE) {
                    pbuf = USBD_CUSTOM_HID_Desc;
                    len = std::min<uint16_t>(USB_CUSTOM_HID_DESC_SIZ, req->getLength());
                }
            }

            pdev->sendData(pbuf, len);
            break;

        case USBD_SetupReqTypedef::RequestGetInterface:
            if (pdev->isConfigured()) {
                pdev->sendData((uint8_t *)(void *)&hhid->AltSetting, 1U);
            } else {
                pdev->stallEndpoints();
                ret = USBD_FAIL;
            }
            break;

        case USBD_SetupReqTypedef::RequestSetInterface:
            if (pdev->isConfigured()) {
                // hhid->AltSetting = (uint8_t)(req->wValue);
                hhid->AltSetting = req->getInterfaceIndex();
            } else {
                pdev->stallEndpoints();
                ret = USBD_FAIL;
            }
            break;

        default:
            pdev->stallEndpoints();
            ret = USBD_FAIL;
            break;
        }
        break;

    default:
        pdev->stallEndpoints();
        ret = USBD_FAIL;
        break;
    }
    return ret == USBD_OK;
}

bool UsbCustomHid::EP0_TxSent(UsbHandle *pdev) { return false; }

bool UsbCustomHid::EP0_RxReady(UsbHandle *pdev)
{
    USBD_CUSTOM_HID_HandleTypeDef *hhid = (USBD_CUSTOM_HID_HandleTypeDef *)pdev->mClassData;

    if (hhid->IsReportAvailable == 1) {
        ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData)->OutEvent(hhid->Report_buf);
        hhid->IsReportAvailable = 0;
    }

    return true;
}

bool UsbCustomHid::DataIn(UsbHandle *pdev, uint8_t epnum)
{ /* Ensure that the FIFO is empty before a new transfer, this condition could
  be caused by  a new transfer before the end of the previous transfer */
    ((USBD_CUSTOM_HID_HandleTypeDef *)pdev->mClassData)->state = CUSTOM_HID_IDLE;

    return true;
}

bool UsbCustomHid::DataOut(UsbHandle *pdev, uint8_t epnum)
{
    USBD_CUSTOM_HID_HandleTypeDef *hhid = (USBD_CUSTOM_HID_HandleTypeDef *)pdev->mClassData;

    ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData)->OutEvent(hhid->Report_buf);

    UsbCore::ref()->prepareReceive(pdev, CUSTOM_HID_EPOUT_ADDR, hhid->Report_buf,
                                   USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);

    return true;
}

bool UsbCustomHid::SOF(UsbHandle *pdev) { return false; }

bool UsbCustomHid::IsoINIncomplete(UsbHandle *pdev, uint8_t epnum) { return false; }

bool UsbCustomHid::IsoOUTIncomplete(UsbHandle *pdev, uint8_t epnum) { return false; }

uint8_t *UsbCustomHid::GetHSConfigDescriptor(uint16_t *length)
{
    *length = sizeof(USBD_CUSTOM_HID_CfgHSDesc);
    return USBD_CUSTOM_HID_CfgHSDesc;
}

uint8_t *UsbCustomHid::GetFSConfigDescriptor(uint16_t *length)
{
    *length = sizeof(USBD_CUSTOM_HID_CfgFSDesc);
    return USBD_CUSTOM_HID_CfgFSDesc;
}

uint8_t *UsbCustomHid::GetOtherSpeedConfigDescriptor(uint16_t *length)
{
    *length = sizeof(USBD_CUSTOM_HID_OtherSpeedCfgDesc);
    return USBD_CUSTOM_HID_OtherSpeedCfgDesc;
}

uint8_t *UsbCustomHid::GetDeviceQualifierDescriptor(uint16_t *length)
{
    *length = sizeof(USBD_CUSTOM_HID_DeviceQualifierDesc);
    return USBD_CUSTOM_HID_DeviceQualifierDesc;
}
