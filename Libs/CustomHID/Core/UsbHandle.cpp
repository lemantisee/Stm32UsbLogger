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
    mClassType->DeInit(this, (uint8_t)mConfig);
    if (!UsbCore::ref()->stopInterface(this)) {
        return false;
    }

    return UsbCore::ref()->deinitInterface(this);
}

void _USBD_HandleTypeDef::setup(uint8_t *psetup)
{
    parseSetupRequest(&mRequest, psetup);

    mEndpoint0State = EndpointSetup;
    mEndpoint0Size = mRequest.wLength;

    switch (mRequest.bmRequest & 0x1FU) {
    case USB_REQ_RECIPIENT_DEVICE: onDeviceRequest(&mRequest); break;
    case USB_REQ_RECIPIENT_INTERFACE: onInterfaceRequest(&mRequest); break;
    case USB_REQ_RECIPIENT_ENDPOINT: onEndpointRequest(&mRequest); break;
    default: UsbCore::ref()->stallEndpoint(this, (mRequest.bmRequest & 0x80U)); break;
    }
}

bool _USBD_HandleTypeDef::start() { return UsbCore::ref()->startInterface(this); }

bool _USBD_HandleTypeDef::stop()
{
    mClassType->DeInit(this, (uint8_t)mConfig);
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
    mConfig = 0U;
    mRemoteWakeup = 0U;

    return mClassData ? mClassType->DeInit(this, (uint8_t)mConfig) == USBD_OK : true;
}

bool _USBD_HandleTypeDef::disconnect()
{
    mState = DeviceDefault;
    return mClassType->DeInit(this, (uint8_t)mConfig) == USBD_OK;
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
    USBD_EndpointTypeDef *pep;

    if (epnum == 0U) {
        pep = &ep_out[0];

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
        USBD_EndpointTypeDef *endpoint = &ep_in[0];

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
    ep_in[0].total_length = len;
    ep_in[0].rem_length = len;

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
    ep_out[0].total_length = len;
    ep_out[0].rem_length = len;

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

    switch (req->bmRequest & USB_REQ_TYPE_MASK) {
    case USB_REQ_TYPE_CLASS:
    case USB_REQ_TYPE_VENDOR: mClassType->Setup(this, req); break;

    case USB_REQ_TYPE_STANDARD:
        switch (req->bRequest) {
        case USB_REQ_GET_DESCRIPTOR: getDescriptor(req); break;

        case USB_REQ_SET_ADDRESS: setAddress(req); break;

        case USB_REQ_SET_CONFIGURATION: setConfig(req); break;

        case USB_REQ_GET_CONFIGURATION: getConfig(req); break;

        case USB_REQ_GET_STATUS: getStatus(req); break;

        case USB_REQ_SET_FEATURE: setFeature(req); break;

        case USB_REQ_CLEAR_FEATURE: clearFeature(req); break;

        default: stallEndpoints(); break;
        }
        break;

    default: stallEndpoints(); break;
    }

    return ret;
}

bool _USBD_HandleTypeDef::onInterfaceRequest(USBD_SetupReqTypedef *req)
{
    switch (req->bmRequest & USB_REQ_TYPE_MASK) {
    case USB_REQ_TYPE_CLASS:
    case USB_REQ_TYPE_VENDOR:
    case USB_REQ_TYPE_STANDARD:
        switch (mState) {
        case DeviceDefault:
        case DeviceAddressed:
        case DeviceConfigured:

            if (LOBYTE(req->wIndex) <= USBD_MAX_NUM_INTERFACES) {
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
    USBD_EndpointTypeDef *pep;
    uint8_t ep_addr;
    USBD_StatusTypeDef ret = USBD_OK;
    ep_addr = LOBYTE(req->wIndex);

    switch (req->bmRequest & USB_REQ_TYPE_MASK) {
    case USB_REQ_TYPE_CLASS:
    case USB_REQ_TYPE_VENDOR: mClassType->Setup(this, req); break;

    case USB_REQ_TYPE_STANDARD:
        /* Check if it is a class request */
        if ((req->bmRequest & 0x60U) == 0x20U) {
            ret = (USBD_StatusTypeDef)mClassType->Setup(this, req);

            return ret;
        }

        switch (req->bRequest) {
        case USB_REQ_SET_FEATURE:
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
                if (req->wValue == USB_FEATURE_EP_HALT) {
                    if ((ep_addr != 0x00U) && (ep_addr != 0x80U) && (req->wLength == 0x00U)) {
                        UsbCore::ref()->stallEndpoint(this, ep_addr);
                    }
                }
                sendStatus();

                break;

            default: stallEndpoints(); break;
            }
            break;

        case USB_REQ_CLEAR_FEATURE:

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
                if (req->wValue == USB_FEATURE_EP_HALT) {
                    if ((ep_addr & 0x7FU) != 0x00U) {
                        UsbCore::ref()->clearStallEndpoint(this, ep_addr);
                    }
                    sendStatus();
                }
                break;

            default: stallEndpoints(); break;
            }
            break;

        case USB_REQ_GET_STATUS:
            switch (mState) {
            case DeviceAddressed:
                if ((ep_addr != 0x00U) && (ep_addr != 0x80U)) {
                    stallEndpoints();
                    break;
                }
                pep = ((ep_addr & 0x80U) == 0x80U) ? &ep_in[ep_addr & 0x7FU]
                                                   : &ep_out[ep_addr & 0x7FU];

                pep->status = 0x0000U;
                sendData((uint8_t *)(void *)&pep->status, 2U);
                break;

            case DeviceConfigured:
                if ((ep_addr & 0x80U) == 0x80U) {
                    if (ep_in[ep_addr & 0xFU].is_used == 0U) {
                        stallEndpoints();
                        break;
                    }
                } else {
                    if (ep_out[ep_addr & 0xFU].is_used == 0U) {
                        stallEndpoints();
                        break;
                    }
                }

                pep = ((ep_addr & 0x80U) == 0x80U) ? &ep_in[ep_addr & 0x7FU]
                                                   : &ep_out[ep_addr & 0x7FU];

                if ((ep_addr == 0x00U) || (ep_addr == 0x80U)) {
                    pep->status = 0x0000U;
                } else if (UsbCore::ref()->isEndpointStall(this, ep_addr)) {
                    pep->status = 0x0001U;
                } else {
                    pep->status = 0x0000U;
                }

                sendData((uint8_t *)(void *)&pep->status, 2U);
                break;

            default: stallEndpoints(); break;
            }
            break;

        default: stallEndpoints(); break;
        }
        break;

    default: stallEndpoints(); break;
    }

    return ret;
}

void _USBD_HandleTypeDef::setConfig(USBD_SetupReqTypedef *req)
{
    static uint8_t cfgidx;

    cfgidx = (uint8_t)(req->wValue);

    if (cfgidx > USBD_MAX_NUM_CONFIGURATION) {
        stallEndpoints();
    } else {
        switch (mState) {
        case DeviceAddressed:
            if (cfgidx) {
                mConfig = cfgidx;
                mState = DeviceConfigured;
                if (!setClassConfig(cfgidx)) {
                    stallEndpoints();
                    return;
                }
                sendStatus();
            } else {
                sendStatus();
            }
            break;

        case DeviceConfigured:
            if (cfgidx == 0U) {
                mState = DeviceAddressed;
                mConfig = cfgidx;
                clearClassConfig(cfgidx);
                sendStatus();
            } else if (cfgidx != mConfig) {
                /* Clear old configuration */
                clearClassConfig((uint8_t)mConfig);

                /* set new configuration */
                mConfig = cfgidx;
                if (!setClassConfig(cfgidx)) {
                    stallEndpoints();
                    return;
                }
                sendStatus();
            } else {
                sendStatus();
            }
            break;

        default:
            stallEndpoints();
            clearClassConfig(cfgidx);
            break;
        }
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
    case DeviceConfigured: sendData((uint8_t *)(void *)&mConfig, 1U); break;
    default: stallEndpoints(); break;
    }
}

void _USBD_HandleTypeDef::getDescriptor(USBD_SetupReqTypedef *req)
{
    uint16_t len = 0U;
    uint8_t *pbuf = NULL;
    uint8_t err = 0U;

    switch (req->wValue >> 8) {
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
        switch ((uint8_t)(req->wValue)) {
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
    uint8_t dev_addr;

    if ((req->wIndex == 0U) && (req->wLength == 0U) && (req->wValue < 128U)) {
        dev_addr = (uint8_t)(req->wValue) & 0x7FU;

        if (mState == DeviceConfigured) {
            stallEndpoints();
        } else {
            mAddress = dev_addr;
            UsbCore::ref()->setUsbAddress(this, dev_addr);
            sendStatus();

            if (dev_addr != 0U) {
                mState = DeviceAddressed;
            } else {
                mState = DeviceDefault;
            }
        }
    } else {
        stallEndpoints();
    }
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
    if (req->wValue == USB_FEATURE_REMOTE_WAKEUP) {
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
        if (req->wValue == USB_FEATURE_REMOTE_WAKEUP) {
            mRemoteWakeup = 0U;
            sendStatus();
        }
        break;

    default: stallEndpoints(); break;
    }
}

void _USBD_HandleTypeDef::parseSetupRequest(USBD_SetupReqTypedef *req, uint8_t *pdata) const
{
    req->bmRequest = *(uint8_t *)(pdata);
    req->bRequest = *(uint8_t *)(pdata + 1U);
    req->wValue = SWAPBYTE(pdata + 2U);
    req->wIndex = SWAPBYTE(pdata + 4U);
    req->wLength = SWAPBYTE(pdata + 6U);
}

void _USBD_HandleTypeDef::stallEndpoints()
{
    UsbCore::ref()->stallEndpoint(this, 0x80U);
    UsbCore::ref()->stallEndpoint(this, 0U);
}

void _USBD_HandleTypeDef::openOutEndpoint0()
{
    UsbCore::ref()->openEndpoint(this, 0x00U, EndpointControl, USB_MAX_EP0_SIZE);
    ep_out[0x00U & 0xFU].is_used = 1U;
    ep_out[0].maxpacket = USB_MAX_EP0_SIZE;
}

void _USBD_HandleTypeDef::openInEndpoint0()
{
    UsbCore::ref()->openEndpoint(this, 0x80U, EndpointControl, USB_MAX_EP0_SIZE);
    ep_in[0x80U & 0xFU].is_used = 1U;
    ep_in[0].maxpacket = USB_MAX_EP0_SIZE;
}
