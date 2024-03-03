#include "UsbHandle.h"

#include <algorithm>

#include "UsbCore.h"

bool _USBD_HandleTypeDef::init(USBD_DescriptorsTypeDef *pdesc, uint8_t id_)
{
    if (mClassType) {
        mClassType = nullptr;
    }

    if (pdesc) {
        mDescriptor = pdesc;
    }

    mState = DeviceDefault;
    mId = id_;

    return UsbCore::ref()->initInterface(this);
}

bool _USBD_HandleTypeDef::deinit()
{
    mState = DeviceDefault;
    mClassType->DeInit(this, (uint8_t)mConfigIndex);
    if (!UsbCore::ref()->stopInterface(this)) {
        return false;
    }

    return UsbCore::ref()->deinitInterface(this);
}

void _USBD_HandleTypeDef::setup(uint8_t *psetup)
{
    mRequest.parse(psetup);

    mEndpoint0State = EndpointSetup;
    mEndpoint0Size = mRequest.wLength;

    switch (mRequest.getRecipient()) {
    case USBD_SetupReqTypedef::RecipientDevice: onDeviceRequest(&mRequest); break;
    case USBD_SetupReqTypedef::RecipientInterface: onInterfaceRequest(&mRequest); break;
    case USBD_SetupReqTypedef::RecipientEndpoint: onEndpointRequest(&mRequest); break;
    default: UsbCore::ref()->stallEndpoint(this, (mRequest.bmRequest & 0x80U)); break;
    }
}

bool _USBD_HandleTypeDef::start() { return UsbCore::ref()->startInterface(this); }

bool _USBD_HandleTypeDef::stop()
{
    mClassType->DeInit(this, (uint8_t)mConfigIndex);
    return UsbCore::ref()->stopInterface(this);
}

bool _USBD_HandleTypeDef::resetUsb()
{
    /* Open EP0 OUT */
    openOutEndpoint0();

    /* Open EP0 IN */
    openInEndpoint0();

    /* Upon Reset call user call back */
    mState = DeviceDefault;
    mEndpoint0State = EndpointIdle;
    mConfigIndex = 0U;
    mRemoteWakeup = 0U;

    return mClassData ? mClassType->DeInit(this, (uint8_t)mConfigIndex) == USBD_OK : true;
}

bool _USBD_HandleTypeDef::disconnect()
{
    mState = DeviceDefault;
    return mClassType->DeInit(this, (uint8_t)mConfigIndex) == USBD_OK;
}

void _USBD_HandleTypeDef::suspend()
{
    mDeviceOldState = mState;
    mState = DeviceSuspended;
}

void _USBD_HandleTypeDef::resume()
{
    if (mState == DeviceSuspended) {
        mState = mDeviceOldState;
    }
}

void _USBD_HandleTypeDef::sof()
{
    if (mState == DeviceConfigured) {
        if (mClassType->SOF) {
            mClassType->SOF(this);
        }
    }
}

bool _USBD_HandleTypeDef::isConfigured() const { return mState == DeviceConfigured; }

void _USBD_HandleTypeDef::setSpeed(USBD_SpeedTypeDef speed) { mSpeed = speed; }

bool _USBD_HandleTypeDef::setClassConfig(uint8_t cfgidx)
{
    return mClassType ? mClassType->Init(this, cfgidx) == USBD_OK : false;
}

bool _USBD_HandleTypeDef::clearClassConfig(uint8_t cfgidx)
{
    return mClassType ? mClassType->DeInit(this, cfgidx) == USBD_OK : false;
}

bool _USBD_HandleTypeDef::registerClass(USBD_ClassTypeDef *pclass)
{
    if (!pclass) {
        return false;
    }

    mClassType = pclass;
    return true;
}

bool _USBD_HandleTypeDef::dataOutStage(uint8_t epnum, uint8_t *pdata)
{
    if (epnum == 0U) {
        UsbEndpoint *pep = &ep_out[0];

        if (mEndpoint0State == EndpointDataOut) {
            if (pep->rem_length > pep->maxpacket) {
                pep->rem_length -= pep->maxpacket;

                continueRx(pdata, std::min(pep->rem_length, pep->maxpacket));
            } else {
                if ((mClassType->EP0_RxReady != nullptr) && (mState == DeviceConfigured)) {
                    mClassType->EP0_RxReady(this);
                }
                sendStatus();
            }
        } else {
            if (mEndpoint0State == EndpointStatusOut) {
                /*
         * STATUS PHASE completed, update ep0_state to idle
         */
                mEndpoint0State = EndpointIdle;
                UsbCore::ref()->stallEndpoint(this, 0U);
            }
        }
    } else if ((mClassType->DataOut != NULL) && (mState == DeviceConfigured)) {
        mClassType->DataOut(this, epnum);
    } else {
        /* should never be in this condition */
        return false;
    }

    return true;
}

bool _USBD_HandleTypeDef::dataInStage(uint8_t epnum, uint8_t *pdata)
{
    if (epnum == 0) {
        UsbEndpoint *endpoint = &ep_in[0];

        if (mEndpoint0State == EndpointDataIn) {
            if (endpoint->rem_length > endpoint->maxpacket) {
                endpoint->rem_length -= endpoint->maxpacket;

                continueSendData(pdata, (uint16_t)endpoint->rem_length);

                /* Prepare endpoint for premature end of transfer */
                UsbCore::ref()->prepareReceive(this, 0U, nullptr, 0U);
            } else {
                /* last packet is MPS multiple, so send ZLP packet */
                if ((endpoint->total_length % endpoint->maxpacket == 0U)
                    && (endpoint->total_length >= endpoint->maxpacket)
                    && (endpoint->total_length < mEndpoint0Size)) {
                    continueSendData(nullptr, 0U);
                    mEndpoint0Size = 0U;

                    /* Prepare endpoint for premature end of transfer */
                    UsbCore::ref()->prepareReceive(this, 0U, NULL, 0U);
                } else {
                    if ((mClassType->EP0_TxSent != NULL) && (mState == DeviceConfigured)) {
                        mClassType->EP0_TxSent(this);
                    }
                    UsbCore::ref()->stallEndpoint(this, 0x80U);
                    receiveStatus();
                }
            }
        } else {
            if ((mEndpoint0State == EndpointStatusIn) || (mEndpoint0State == EndpointIdle)) {
                UsbCore::ref()->stallEndpoint(this, 0x80U);
            }
        }

        if (mTestMode == 1U) {
            mTestMode = 0U;
        }
        return true;
    }

    if (mClassType->DataIn && mState == DeviceConfigured) {
        mClassType->DataIn(this, epnum);
        return true;
    }

    return false;
}

bool _USBD_HandleTypeDef::isoInIncomplete(uint8_t epnum) { return true; }

bool _USBD_HandleTypeDef::isoOUTIncomplete(uint8_t epnum) { return true; }

uint32_t _USBD_HandleTypeDef::getRxCount(uint8_t ep_addr)
{
    return UsbCore::ref()->getRxDataSize(this, ep_addr);
}

bool _USBD_HandleTypeDef::sendData(uint8_t *pbuf, uint16_t len)
{
    /* Set EP0 State */
    mEndpoint0State = EndpointDataIn;
    ep_in[0].setLength(len);

    return UsbCore::ref()->transmit(this, 0x00U, pbuf, len);
}

bool _USBD_HandleTypeDef::continueSendData(uint8_t *pbuf, uint16_t len)
{
    /* Start the next transfer */
    return UsbCore::ref()->transmit(this, 0x00U, pbuf, len);
}

bool _USBD_HandleTypeDef::prepareRx(uint8_t *pbuf, uint16_t len)
{
    /* Set EP0 State */
    mEndpoint0State = EndpointDataOut;
    ep_out[0].setLength(len);

    return UsbCore::ref()->prepareReceive(this, 0U, pbuf, len);
}

bool _USBD_HandleTypeDef::continueRx(uint8_t *pbuf, uint16_t len)
{
    return UsbCore::ref()->prepareReceive(this, 0U, pbuf, len);
}

bool _USBD_HandleTypeDef::sendStatus()
{
    /* Set EP0 State */
    mEndpoint0State = EndpointStatusIn;
    return UsbCore::ref()->transmit(this, 0x00U, NULL, 0U);
}

bool _USBD_HandleTypeDef::receiveStatus()
{
    /* Set EP0 State */
    mEndpoint0State = EndpointStatusOut;

    /* Start the transfer */
    return UsbCore::ref()->prepareReceive(this, 0U, NULL, 0U);
}

bool _USBD_HandleTypeDef::onDeviceRequest(USBD_SetupReqTypedef *req)
{
    USBD_StatusTypeDef ret = USBD_OK;

    switch (req->getRequestType()) {
    case USBD_SetupReqTypedef::RequestClass:
    case USBD_SetupReqTypedef::RequestVendor: mClassType->Setup(this, req); break;
    case USBD_SetupReqTypedef::RequestStandart:
        switch (req->getRequest()) {
        case USBD_SetupReqTypedef::RequestGetDescriptor: getDescriptor(req); break;
        case USBD_SetupReqTypedef::RequestSetAddress: setAddress(req); break;
        case USBD_SetupReqTypedef::RequestSetConfiguration: setConfig(req); break;
        case USBD_SetupReqTypedef::RequestGetConfiguration: getConfig(req); break;
        case USBD_SetupReqTypedef::RequestGetStatus: getStatus(req); break;
        case USBD_SetupReqTypedef::RequestSetFeature: setFeature(req); break;
        case USBD_SetupReqTypedef::RequestClearFeature: clearFeature(req); break;

        default: stallEndpoints(); break;
        }
        break;

    default: stallEndpoints(); break;
    }

    return ret;
}

bool _USBD_HandleTypeDef::onInterfaceRequest(USBD_SetupReqTypedef *req)
{
    switch (req->getRequestType()) {
    case USBD_SetupReqTypedef::RequestClass:
    case USBD_SetupReqTypedef::RequestVendor:
    case USBD_SetupReqTypedef::RequestStandart:
        switch (mState) {
        case DeviceDefault:
        case DeviceAddressed:
        case DeviceConfigured:

            if (req->getInterfaceNumber() <= USBD_MAX_NUM_INTERFACES) {
                bool ok = (USBD_StatusTypeDef)mClassType->Setup(this, req) == USBD_OK;

                if (req->wLength == 0U && ok) {
                    sendStatus();
                }
            } else {
                stallEndpoints();
            }
            break;

        default: stallEndpoints(); break;
        }
        break;

    default: stallEndpoints(); break;
    }

    return true;
}

bool _USBD_HandleTypeDef::onEndpointRequest(USBD_SetupReqTypedef *req)
{
    switch (req->getRequestType()) {
    case USBD_SetupReqTypedef::RequestClass:
    case USBD_SetupReqTypedef::RequestVendor: mClassType->Setup(this, req); break;
    case USBD_SetupReqTypedef::RequestStandart: {
        uint8_t ep_addr = req->getEndpointAddress();

        switch (req->getRequest()) {
        case USBD_SetupReqTypedef::RequestSetFeature:
            switch (mState) {
            case DeviceAddressed:
                if ((ep_addr != 0x00U) && (ep_addr != 0x80U)) {
                    UsbCore::ref()->stallEndpoint(this, ep_addr);
                    UsbCore::ref()->stallEndpoint(this, 0x80U);
                } else {
                    stallEndpoints();
                }
                break;

            case DeviceConfigured:
                if (req->getFeatureRequest() == USB_FEATURE_EP_HALT) {
                    if ((ep_addr != 0x00U) && (ep_addr != 0x80U) && (req->wLength == 0x00U)) {
                        UsbCore::ref()->stallEndpoint(this, ep_addr);
                    }
                }
                sendStatus();

                break;

            default: stallEndpoints(); break;
            }
            break;

        case USBD_SetupReqTypedef::RequestClearFeature:

            switch (mState) {
            case DeviceAddressed:
                if ((ep_addr != 0x00U) && (ep_addr != 0x80U)) {
                    UsbCore::ref()->stallEndpoint(this, ep_addr);
                    UsbCore::ref()->stallEndpoint(this, 0x80U);
                } else {
                    stallEndpoints();
                }
                break;

            case DeviceConfigured:
                if (req->getFeatureRequest() == USB_FEATURE_EP_HALT) {
                    if ((ep_addr & 0x7FU) != 0x00U) {
                        UsbCore::ref()->clearStallEndpoint(this, ep_addr);
                    }
                    sendStatus();
                }
                break;

            default: stallEndpoints(); break;
            }
            break;

        case USBD_SetupReqTypedef::RequestGetStatus:
            switch (mState) {
            case DeviceAddressed:
                if ((ep_addr != 0x00U) && (ep_addr != 0x80U)) {
                    stallEndpoints();
                    break;
                }
                {
                    UsbEndpoint *ep = isEndpointIn(ep_addr) ? &ep_in[ep_addr & 0x7FU]
                                                            : &ep_out[ep_addr & 0x7FU];
                    ep->status = 0x0000U;
                    sendData((uint8_t *)(void *)&ep->status, 2U);
                }
                break;
            case DeviceConfigured:
                if (isEndpointIn(ep_addr)) {
                    if (!ep_in[ep_addr & 0xFU].is_used) {
                        stallEndpoints();
                        break;
                    }
                } else {
                    if (!ep_out[ep_addr & 0xFU].is_used) {
                        stallEndpoints();
                        break;
                    }
                }

                {
                    UsbEndpoint *ep = isEndpointIn(ep_addr) ? &ep_in[ep_addr & 0x7FU]
                                                            : &ep_out[ep_addr & 0x7FU];

                    if ((ep_addr == 0x00U) || (ep_addr == 0x80U)) {
                        ep->status = 0x0000U;
                    } else if (UsbCore::ref()->isEndpointStall(this, ep_addr)) {
                        ep->status = 0x0001U;
                    } else {
                        ep->status = 0x0000U;
                    }

                    sendData((uint8_t *)(void *)&ep->status, 2U);
                }
                break;

            default: stallEndpoints(); break;
            }
            break;

        default: stallEndpoints(); break;
        }
        break;
    }

    default: stallEndpoints(); break;
    }

    return USBD_OK;
}

void _USBD_HandleTypeDef::setConfig(USBD_SetupReqTypedef *req)
{
    const uint8_t configIndex = req->getConfigIndex();
    if (configIndex > USBD_MAX_NUM_CONFIGURATION) {
        stallEndpoints();
        return;
    }

    switch (mState) {
    case DeviceAddressed:
        if (configIndex == 0) {
            sendStatus();
            break;
        }

        mConfigIndex = configIndex;
        mState = DeviceConfigured;
        if (!setClassConfig(configIndex)) {
            stallEndpoints();
            return;
        }

        sendStatus();
        break;
    case DeviceConfigured:
        if (configIndex == 0) {
            mState = DeviceAddressed;
            mConfigIndex = configIndex;
            clearClassConfig(configIndex);
            sendStatus();
            break;
        }
        
        if (configIndex != mConfigIndex) {
            /* Clear old configuration */
            clearClassConfig((uint8_t)mConfigIndex);

            /* set new configuration */
            mConfigIndex = configIndex;
            if (!setClassConfig(configIndex)) {
                stallEndpoints();
                return;
            }
            sendStatus();
            break;
        }
        sendStatus();
        break;
    default:
        stallEndpoints();
        clearClassConfig(configIndex);
        break;
    }
}

void _USBD_HandleTypeDef::getConfig(USBD_SetupReqTypedef *req)
{
    if (req->wLength != 1U) {
        stallEndpoints();
        return;
    }

    switch (mState) {
    case DeviceDefault:
    case DeviceAddressed:
        mConfigDefault = 0U;
        sendData((uint8_t *)(void *)&mConfigDefault, 1U);
        break;
    case DeviceConfigured: sendData((uint8_t *)(void *)&mConfigIndex, 1U); break;
    default: stallEndpoints(); break;
    }
}

void _USBD_HandleTypeDef::getDescriptor(USBD_SetupReqTypedef *req)
{
    uint16_t len = 0U;
    uint8_t *pbuf = NULL;
    uint8_t err = 0U;

    switch (req->getDescriptorType()) {
#if (USBD_LPM_ENABLED == 1U)
    case USB_DESC_TYPE_BOS:
        if (pDesc->GetBOSDescriptor != NULL) {
            pbuf = pDesc->GetBOSDescriptor(dev_speed, &len);
        } else {
            stallEndpoints();
            err++;
        }
        break;
#endif
    case USB_DESC_TYPE_DEVICE: pbuf = mDescriptor->GetDeviceDescriptor(mSpeed, &len); break;

    case USB_DESC_TYPE_CONFIGURATION:
        if (mSpeed == USBD_SPEED_HIGH) {
            pbuf = mClassType->GetHSConfigDescriptor(&len);
            pbuf[1] = USB_DESC_TYPE_CONFIGURATION;
        } else {
            pbuf = mClassType->GetFSConfigDescriptor(&len);
            pbuf[1] = USB_DESC_TYPE_CONFIGURATION;
        }
        break;

    case USB_DESC_TYPE_STRING:
        switch (req->getStringIndex()) {
        case USBD_IDX_LANGID_STR:
            if (mDescriptor->GetLangIDStrDescriptor != NULL) {
                pbuf = mDescriptor->GetLangIDStrDescriptor(mSpeed, &len);
            } else {
                stallEndpoints();
                err++;
            }
            break;

        case USBD_IDX_MFC_STR:
            if (mDescriptor->GetManufacturerStrDescriptor != NULL) {
                pbuf = mDescriptor->GetManufacturerStrDescriptor(mSpeed, &len);
            } else {
                stallEndpoints();
                err++;
            }
            break;

        case USBD_IDX_PRODUCT_STR:
            if (mDescriptor->GetProductStrDescriptor != NULL) {
                pbuf = mDescriptor->GetProductStrDescriptor(mSpeed, &len);
            } else {
                stallEndpoints();
                err++;
            }
            break;

        case USBD_IDX_SERIAL_STR:
            if (mDescriptor->GetSerialStrDescriptor != NULL) {
                pbuf = mDescriptor->GetSerialStrDescriptor(mSpeed, &len);
            } else {
                stallEndpoints();
                err++;
            }
            break;

        case USBD_IDX_CONFIG_STR:
            if (mDescriptor->GetConfigurationStrDescriptor != NULL) {
                pbuf = mDescriptor->GetConfigurationStrDescriptor(mSpeed, &len);
            } else {
                stallEndpoints();
                err++;
            }
            break;

        case USBD_IDX_INTERFACE_STR:
            if (mDescriptor->GetInterfaceStrDescriptor != NULL) {
                pbuf = mDescriptor->GetInterfaceStrDescriptor(mSpeed, &len);
            } else {
                stallEndpoints();
                err++;
            }
            break;

        default:
#if (USBD_SUPPORT_USER_STRING_DESC == 1U)
            if (pClass->GetUsrStrDescriptor != NULL) {
                pbuf = pClass->GetUsrStrDescriptor(this, (req->wValue), &len);
            } else {
                stallEndpoints();
                err++;
            }
            break;
#else
            stallEndpoints();
            err++;
#endif
        }
        break;

    case USB_DESC_TYPE_DEVICE_QUALIFIER:
        if (mSpeed == USBD_SPEED_HIGH) {
            pbuf = mClassType->GetDeviceQualifierDescriptor(&len);
        } else {
            stallEndpoints();
            err++;
        }
        break;

    case USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION:
        if (mSpeed == USBD_SPEED_HIGH) {
            pbuf = mClassType->GetOtherSpeedConfigDescriptor(&len);
            pbuf[1] = USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION;
        } else {
            stallEndpoints();
            err++;
        }
        break;

    default:
        stallEndpoints();
        err++;
        break;
    }

    if (err != 0U) {
        return;
    } else {
        if ((len != 0U) && (req->wLength != 0U)) {
            len = std::min(len, req->wLength);
            sendData(pbuf, len);
        }

        if (req->wLength == 0U) {
            (void)sendStatus();
        }
    }
}

void _USBD_HandleTypeDef::setAddress(USBD_SetupReqTypedef *req)
{
    if (mState == DeviceConfigured) {
        stallEndpoints();
        return;
    }

    auto deviceAddrOpt = req->getDeviceAddress();
    if (!deviceAddrOpt) {
        stallEndpoints();
        return;
    }

    mAddress = *deviceAddrOpt;
    UsbCore::ref()->setUsbAddress(this, mAddress);
    sendStatus();

    mState = mAddress != 0 ? DeviceAddressed : DeviceDefault;
}

void _USBD_HandleTypeDef::getStatus(USBD_SetupReqTypedef *req)
{
    switch (mState) {
    case DeviceDefault:
    case DeviceAddressed:
    case DeviceConfigured:
        if (req->wLength != 0x2U) {
            stallEndpoints();
            break;
        }

#if (USBD_SELF_POWERED == 1U)
        mConfigStatus = USB_CONFIG_SELF_POWERED;
#else
        dev_config_status = 0U;
#endif

        if (mRemoteWakeup) {
            mConfigStatus |= USB_CONFIG_REMOTE_WAKEUP;
        }

        sendData((uint8_t *)(void *)&mConfigStatus, 2U);
        break;

    default: stallEndpoints(); break;
    }
}

void _USBD_HandleTypeDef::setFeature(USBD_SetupReqTypedef *req)
{
    if (req->getFeatureRequest() == USB_FEATURE_REMOTE_WAKEUP) {
        mRemoteWakeup = 1U;
        sendStatus();
    }
}

void _USBD_HandleTypeDef::clearFeature(USBD_SetupReqTypedef *req)
{
    switch (mState) {
    case DeviceDefault:
    case DeviceAddressed:
    case DeviceConfigured:
        if (req->getFeatureRequest() == USB_FEATURE_REMOTE_WAKEUP) {
            mRemoteWakeup = 0U;
            sendStatus();
        }
        break;

    default: stallEndpoints(); break;
    }
}

bool _USBD_HandleTypeDef::isEndpointIn(uint8_t epAddress) const
{
    return epAddress & 0x80U == 0x80U;
}

void _USBD_HandleTypeDef::stallEndpoints()
{
    UsbCore::ref()->stallEndpoint(this, 0x80U);
    UsbCore::ref()->stallEndpoint(this, 0U);
}

void _USBD_HandleTypeDef::openOutEndpoint0()
{
    UsbCore::ref()->openEndpoint(this, 0x00U, EndpointControl, USB_MAX_EP0_SIZE);
    ep_out[0x00U & 0xFU].is_used = true;
    ep_out[0].maxpacket = USB_MAX_EP0_SIZE;
}

void _USBD_HandleTypeDef::openInEndpoint0()
{
    UsbCore::ref()->openEndpoint(this, 0x80U, EndpointControl, USB_MAX_EP0_SIZE);
    ep_in[0x80U & 0xFU].is_used = true;
    ep_in[0].maxpacket = USB_MAX_EP0_SIZE;
}
