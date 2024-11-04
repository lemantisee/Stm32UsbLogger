#include "UsbCustomHid.h"

#include <algorithm>
#include <cstring>

#include "UsbDriver.h"
#include "UsbSetupRequest.h"
#include "DataAlign.h"

namespace {

const uint8_t descriptorType = 0x21;
const uint8_t reportDescriptorType = 0x22;

const uint8_t requestSetProtocol = 11;
const uint8_t requestGetProtocol = 3;
const uint8_t requestSetIdle = 10;
const uint8_t requestGetIdle = 2;
const uint8_t requestSetReport = 9;
const uint8_t requestGetReport = 1;

const uint8_t pollingIntervalHighSpeed = 5;
const uint8_t pollingIntervalFullSpeed = 5;

const uint8_t configDescriptorSize = 41;
const uint8_t descriptorSize = 9;

const uint8_t endpointInSize = 0x40;
const uint8_t endpointOutSize = 0x40;

const uint8_t interfaceDescriptorType = 4;

const uint8_t USB_LEN_DEV_QUALIFIER_DESC = 10;

// USB CUSTOM_HID device FS Configuration Descriptor */
__ALIGN_BEGIN static uint8_t configDescriptorFullSpeed[configDescriptorSize] __ALIGN_END = {
    0x09,                         // bLength: Configuration Descriptor size */
    usb::ConfigurationDescriptor, // bDescriptorType: Configuration */
    configDescriptorSize,         // wTotalLength: Bytes returned */
    0x00, 0x01,                   //bNumInterfaces: 1 interface*/
    0x01,                         //bConfigurationValue: Configuration value*/
    0x00, //iConfiguration: Index of string descriptor describing the configuration
    0xC0, //bmAttributes: bus powered */
    0x32, //MaxPower 100 mA: this current is used for detecting Vbus*/

    //************* Descriptor of CUSTOM HID interface ****************/
    0x09,                    //bLength: Interface Descriptor size*/
    interfaceDescriptorType, //bDescriptorType
    0x00,                    //bInterfaceNumber: Number of Interface*/
    0x00,                    //bAlternateSetting: Alternate setting*/
    0x02,                    //bNumEndpoints*/
    0x03,                    //bInterfaceClass: CUSTOM_HID*/
    0x00,                    //bInterfaceSubClass : 1=BOOT, 0=no boot*/
    0x00,                    //nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
    0,                       //iInterface: Index of string descriptor*/

    //******************* Descriptor of CUSTOM_HID *************************/
    0x09,           //bLength: CUSTOM_HID Descriptor size*/
    descriptorType, //bDescriptorType: CUSTOM_HID*/
    0x11,           //bCUSTOM_HIDUSTOM_HID: CUSTOM_HID Class Spec release number*/
    0x01, 0x00,     //bCountryCode: Hardware target country*/
    0x01,           //bNumDescriptors: Number of CUSTOM_HID class descriptors to follow*/
    0x22,           //bDescriptorType*/
    customHidReportDescriptoSize, //wItemLength: Total length of Report descriptor*/
    0x00,

    //******************* Descriptor of Custom HID endpoints ********************/
    0x07,                    //bLength: Endpoint Descriptor size*/
    usb::EndpointDescriptor, //bDescriptorType:*/
    UsbCustomHid::endpointInAddress,
    0x03,                           //bmAttributes: Interrupt endpoint*/
    endpointInSize,                 //wMaxPacketSize: 2 Byte max */
    0x00, pollingIntervalFullSpeed, //bInterval: Polling Interval */

    0x07,                             // bLength: Endpoint Descriptor size */
    usb::EndpointDescriptor,          // bDescriptorType: */
    UsbCustomHid::endpointOutAddress, //bEndpointAddress: Endpoint Address (OUT)*/
    0x03,                             // bmAttributes: Interrupt endpoint */
    endpointOutSize,                  // wMaxPacketSize: 2 Bytes max  */
    0x00, pollingIntervalFullSpeed,   // bInterval: Polling Interval */
};

// USB CUSTOM_HID device HS Configuration Descriptor */
__ALIGN_BEGIN static uint8_t configDescriptorHighSpeed[configDescriptorSize] __ALIGN_END = {
    0x09,                         // bLength: Configuration Descriptor size
    usb::ConfigurationDescriptor, // bDescriptorType: Configuration
    configDescriptorSize,         // wTotalLength: Bytes returned
    0x00, 0x01,                   //bNumInterfaces: 1 interface
    0x01,                         //bConfigurationValue: Configuration value
    0x00, //iConfiguration: Index of string descriptor describing the configuration
    0xC0, //bmAttributes: bus powered
    0x32, //MaxPower 100 mA: this current is used for detecting Vbus

    //************* Descriptor of CUSTOM HID interface ****************/
    0x09,                    //bLength: Interface Descriptor size*/
    interfaceDescriptorType, //bDescriptorType: Interface descriptor type*/
    0x00,                    //bInterfaceNumber: Number of Interface*/
    0x00,                    //bAlternateSetting: Alternate setting*/
    0x02,                    //bNumEndpoints*/
    0x03,                    //bInterfaceClass: CUSTOM_HID*/
    0x00,                    //bInterfaceSubClass : 1=BOOT, 0=no boot*/
    0x00,                    //nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
    0,                       //iInterface: Index of string descriptor*/

    //******************* Descriptor of CUSTOM_HID *************************/
    0x09,           //bLength: CUSTOM_HID Descriptor size*/
    descriptorType, //bDescriptorType: CUSTOM_HID*/
    0x11,           //bCUSTOM_HIDUSTOM_HID: CUSTOM_HID Class Spec release number*/
    0x01, 0x00,     //bCountryCode: Hardware target country*/
    0x01,           //bNumDescriptors: Number of CUSTOM_HID class descriptors to follow*/
    0x22,           //bDescriptorType*/
    customHidReportDescriptoSize, //wItemLength: Total length of Report descriptor*/
    0x00,

    //******************* Descriptor of Custom HID endpoints ********************/
    0x07,                    //bLength: Endpoint Descriptor size*/
    usb::EndpointDescriptor, //bDescriptorType:*/
    UsbCustomHid::endpointInAddress,
    0x03,                           //bmAttributes: Interrupt endpoint*/
    endpointInSize,                 //wMaxPacketSize: 2 Byte max */
    0x00, pollingIntervalHighSpeed, //bInterval: Polling Interval */

    0x07,                    // bLength: Endpoint Descriptor size */
    usb::EndpointDescriptor, // bDescriptorType: */
    UsbCustomHid::endpointOutAddress,
    0x03,                           // bmAttributes: Interrupt endpoint */
    endpointOutSize,                // wMaxPacketSize: 2 Bytes max  */
    0x00, pollingIntervalHighSpeed, // bInterval: Polling Interval */
};

// USB CUSTOM_HID device Other Speed Configuration Descriptor */
__ALIGN_BEGIN static uint8_t configDescriptorOtherSpeed[configDescriptorSize] __ALIGN_END = {
    0x09,                                   // bLength: Configuration Descriptor size */
    usb::ConfigurationOtherSpeedDescriptor, // bDescriptorType: Configuration */
    configDescriptorSize,                   // wTotalLength: Bytes returned
    0x00, 0x01,                             //bNumInterfaces: 1 interface*/
    0x01,                                   //bConfigurationValue: Configuration value*/
    0x00, //iConfiguration: Index of string descriptor describing the configuration
    0xC0, //bmAttributes: bus powered */
    0x32, //MaxPower 100 mA: this current is used for detecting Vbus*/

    //************* Descriptor of CUSTOM HID interface ****************/
    0x09,                    //bLength: Interface Descriptor size*/
    interfaceDescriptorType, //bDescriptorType: Interface descriptor type*/
    0x00,                    //bInterfaceNumber: Number of Interface*/
    0x00,                    //bAlternateSetting: Alternate setting*/
    0x02,                    //bNumEndpoints*/
    0x03,                    //bInterfaceClass: CUSTOM_HID*/
    0x00,                    //bInterfaceSubClass : 1=BOOT, 0=no boot*/
    0x00,                    //nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
    0,                       //iInterface: Index of string descriptor*/

    //******************* Descriptor of CUSTOM_HID *************************/
    0x09,           //bLength: CUSTOM_HID Descriptor size*/
    descriptorType, //bDescriptorType: CUSTOM_HID*/
    0x11,           //bCUSTOM_HIDUSTOM_HID: CUSTOM_HID Class Spec release number*/
    0x01, 0x00,     //bCountryCode: Hardware target country*/
    0x01,           //bNumDescriptors: Number of CUSTOM_HID class descriptors to follow*/
    0x22,           //bDescriptorType*/
    customHidReportDescriptoSize, //wItemLength: Total length of Report descriptor*/
    0x00,

    //******************* Descriptor of Custom HID endpoints ********************/
    0x07,                            //bLength: Endpoint Descriptor size*/
    usb::EndpointDescriptor,         //bDescriptorType:*/
    UsbCustomHid::endpointInAddress, //bEndpointAddress: Endpoint Address (IN)*/
    0x01,                            //bmAttributes: Interrupt endpoint*/
    endpointInSize,                  //wMaxPacketSize: 2 Byte max */
    0x00, pollingIntervalFullSpeed,  //bInterval: Polling Interval */

    0x07,                             // bLength: Endpoint Descriptor size */
    usb::EndpointDescriptor,          // bDescriptorType: */
    UsbCustomHid::endpointOutAddress, //bEndpointAddress: Endpoint Address (OUT)*/
    0x01,                             // bmAttributes: Interrupt endpoint */
    endpointOutSize,                  // wMaxPacketSize: 2 Bytes max  */
    0x00, pollingIntervalFullSpeed,   // bInterval: Polling Interval */
};

// USB CUSTOM_HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t configDescriptor[descriptorSize] __ALIGN_END = {
    // 18 */
    0x09,           //bLength: CUSTOM_HID Descriptor size*/
    descriptorType, //bDescriptorType: CUSTOM_HID*/
    0x11,           //bCUSTOM_HIDUSTOM_HID: CUSTOM_HID Class Spec release number*/
    0x01,
    0x00, //bCountryCode: Hardware target country*/
    0x01, //bNumDescriptors: Number of CUSTOM_HID class descriptors to follow*/
    0x22, //bDescriptorType*/
    customHidReportDescriptoSize, //wItemLength: Total length of Report descriptor*/
    0x00,
};

// USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t qualifierDescriptor[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END = {
    USB_LEN_DEV_QUALIFIER_DESC,
    usb::QualfierDescriptor,
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

bool UsbCustomHid::init(UsbHandle *pdev, uint8_t cfgidx)
{
    if (!pdev->openEndpoint(UsbCustomHid::endpointInAddress, endpointInSize,
                            UsbHandle::EndpointInterrupt)) {
        return false;
    }

    if (!pdev->openEndpoint(UsbCustomHid::endpointOutAddress, endpointOutSize,
                            UsbHandle::EndpointInterrupt)) {
        return false;
    }

    mState = HidIdle;
    if (!onInit()) {
        return false;
    }

    // Prepare Out endpoint to receive 1st packet */
    pdev->getDriver()->prepareReceive(UsbCustomHid::endpointOutAddress, {mReportBuffer,
                                      customHidOutReportBufferSize});

    return true;
}

bool UsbCustomHid::deInit(UsbHandle *pdev, uint8_t cfgidx)
{
    pdev->closeEndpoint(UsbCustomHid::endpointInAddress);
    pdev->closeEndpoint(UsbCustomHid::endpointOutAddress);
    return true;
}

bool UsbCustomHid::setup(UsbHandle *pdev, const UsbSetupRequest &req)
{
    switch (req.getRequestType()) {
    case UsbSetupRequest::RequestClass: return onRequestClass(pdev, req);
    case UsbSetupRequest::RequestStandart: return onRequestStandart(pdev, req);
    default: break;
    }

    return false;
}

bool UsbCustomHid::ep0_TxSent(UsbHandle *pdev) { return false; }

bool UsbCustomHid::ep0_RxReady(UsbHandle *pdev)
{
    if (mIsReportAvailable) {
        onReceive(mReportBuffer, customHidOutReportBufferSize);
        mIsReportAvailable = false;
    }

    return true;
}

bool UsbCustomHid::dataIn(UsbHandle *pdev, uint8_t epnum)
{
    // Ensure that the FIFO is empty before a new transfer, this condition could
    // be caused by  a new transfer before the end of the previous transfer
    mState = HidIdle;
    return true;
}

bool UsbCustomHid::dataOut(UsbHandle *pdev, uint8_t epnum)
{
    onReceive(mReportBuffer, customHidOutReportBufferSize);
    pdev->getDriver()->prepareReceive(UsbCustomHid::endpointOutAddress, {mReportBuffer,
                                      customHidOutReportBufferSize});

    return true;
}

bool UsbCustomHid::SOF(UsbHandle *pdev) { return false; }

bool UsbCustomHid::isoINIncomplete(UsbHandle *pdev, uint8_t epnum) { return false; }

bool UsbCustomHid::isoOUTIncomplete(UsbHandle *pdev, uint8_t epnum) { return false; }

std::span<uint8_t> UsbCustomHid::getConfigDescriptor(usb::UsbSpeed speed)
{
    switch (speed) {
    case usb::HighSpeed: return std::span(configDescriptorHighSpeed);
    case usb::FullSpeed: return std::span(configDescriptorFullSpeed);
    default: break;
    }

    return {};
}

std::span<uint8_t> UsbCustomHid::getOtherSpeedConfigDescriptor()
{
    return std::span(configDescriptorOtherSpeed);
}

std::span<uint8_t> UsbCustomHid::getDeviceQualifierDescriptor()
{
    return std::span(qualifierDescriptor);
}

bool UsbCustomHid::sendReport(UsbHandle *pdev, const std::span<uint8_t> &data)
{
    if (!pdev->isConfigured()) {
        return false;
    }

    while(mState == HidBusy) {}

    std::array<uint8_t, endpointInSize> dataBuffer;
    dataBuffer.fill(0);
    std::memcpy(dataBuffer.data(), data.data(), data.size());

    mState = HidBusy;
    return pdev->getDriver()->transmit(UsbCustomHid::endpointInAddress, {dataBuffer.data(), dataBuffer.size()});
}

uint8_t UsbCustomHid::getEndpointInBufferSize() { return endpointInSize; }

uint8_t UsbCustomHid::getEndpointOutBufferSize() { return endpointOutSize; }

bool UsbCustomHid::onRequestClass(UsbHandle *pdev, const UsbSetupRequest &req)
{
    switch (req.getRequest()) {
    case requestSetProtocol: mProtocol = req.getProtocol(); return true;
    case requestGetProtocol: return pdev->sendData(mProtocol);
    case requestSetIdle: mIdleState = req.getIdleState(); return true;
    case requestGetIdle: return pdev->sendData(mIdleState);
    case requestSetReport:
        mIsReportAvailable = true;
        return pdev->receiveData({mReportBuffer, req.getLength()});
    default: break;
    }

    return false;
}

bool UsbCustomHid::onRequestStandart(UsbHandle *pdev, const UsbSetupRequest &req)
{
    switch (req.getRequest()) {
    case UsbSetupRequest::RequestGetStatus:
        if (!pdev->isConfigured()) {
            return false;
        }
        {
            uint16_t status_info = 0;
            pdev->sendData({(uint8_t *)(void *)&status_info, 2});
        }
        return true;
    case UsbSetupRequest::RequestGetDescriptor: {
        uint8_t *pbuf = nullptr;
        uint16_t len = 0;
        if (req.getDescriptorType() == reportDescriptorType) {
            len = std::min<uint16_t>(customHidReportDescriptoSize, req.getLength());
            pbuf = getReportDescriptor();
        } else {
            if (req.getDescriptorType() == descriptorType) {
                pbuf = configDescriptor;
                len = std::min<uint16_t>(descriptorSize, req.getLength());
            }
        }

        pdev->sendData({pbuf, len});
    }
        return true;
    case UsbSetupRequest::RequestGetInterface:
        if (!pdev->isConfigured()) {
            return false;
        }

        pdev->sendData(uint8_t(mAltSetting));
        return true;
    case UsbSetupRequest::RequestSetInterface:
        if (!pdev->isConfigured()) {
            return false;
        }

        mAltSetting = req.getInterfaceIndex();
        return true;
    default: break;
    }

    return false;
}
