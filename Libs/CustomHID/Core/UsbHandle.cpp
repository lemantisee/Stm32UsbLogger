#include "UsbHandle.h"

#include <algorithm>

#include "UsbDescriptor.h"
#include "UsbClass.h"
#include "UsbDriver.h"
#include "UsbSetupRequest.h"

namespace {
const uint8_t USB_CONFIG_REMOTE_WAKEUP = 0x02;
const uint8_t USB_CONFIG_SELF_POWERED = 0x01;
} // namespace

bool UsbHandle::init(const UsbDescriptor *descriptor, uint8_t id, UsbDriver *driver)
{
    mDriver = driver;
    mDescriptor = descriptor;
    mState = DeviceDefault;
    mId = id;

    return mDriver->initInterface(this);
}

bool UsbHandle::deinit()
{
    mState = DeviceDefault;
    mClassType->deInit(this, (uint8_t)mConfigIndex);
    if (!mDriver->stopInterface()) {
        return false;
    }

    return mDriver->deinitInterface();
}

void UsbHandle::setup(const uint8_t *psetup)
{
    UsbSetupRequest request;
    request.parse(psetup);

    mEndpoint0State = EndpointSetup;
    mEndpoint0Size = request.getLength();

    switch (request.getRecipient()) {
    case UsbSetupRequest::RecipientDevice: onDeviceRequest(request); break;
    case UsbSetupRequest::RecipientInterface: onInterfaceRequest(request); break;
    case UsbSetupRequest::RecipientEndpoint: onEndpointRequest(request); break;
    default: mDriver->stallEndpoint(request.getEndpointFromMask()); break;
    }
}

bool UsbHandle::start() { return mDriver->startInterface(); }

bool UsbHandle::stop()
{
    mClassType->deInit(this, (uint8_t)mConfigIndex);
    return mDriver->stopInterface();
}

bool UsbHandle::resetUsb()
{
    mSpeed = usb::FullSpeed;

    if (!openEndpoint(usb::defualtEndpointOutAddress, usb::endpont0_Size, EndpointControl)) {
        return false;
    }

    if (!openEndpoint(usb::defualtEndpointInAddress, usb::endpont0_Size, EndpointControl)) {
        return false;
    }

    mState = DeviceDefault;
    mEndpoint0State = EndpointIdle;
    mConfigIndex = 0;
    mRemoteWakeup = false;

    return mClassType ? mClassType->deInit(this, mConfigIndex) : true;
}

bool UsbHandle::disconnect()
{
    mState = DeviceDefault;
    return mClassType->deInit(this, mConfigIndex);
}

void UsbHandle::suspend()
{
    mDeviceOldState = mState;
    mState = DeviceSuspended;
}

void UsbHandle::resume()
{
    if (mState == DeviceSuspended) {
        mState = mDeviceOldState;
    }
}

void UsbHandle::sof()
{
    if (mState == DeviceConfigured) {
        mClassType->SOF(this);
    }
}

bool UsbHandle::isConfigured() const { return mState == DeviceConfigured; }

bool UsbHandle::setClassConfig(uint8_t cfgidx)
{
    return mClassType ? mClassType->init(this, cfgidx) : false;
}

bool UsbHandle::clearClassConfig(uint8_t cfgidx)
{
    return mClassType ? mClassType->deInit(this, cfgidx) : false;
}

bool UsbHandle::registerClass(UsbClass *pclass)
{
    if (!pclass) {
        return false;
    }

    mClassType = pclass;
    return true;
}

bool UsbHandle::dataOutStage(uint8_t epnum, uint8_t *pdata)
{
    if (epnum > 0) {
        if (mState == DeviceConfigured) {
            mClassType->dataOut(this, epnum);
            return true;
        }

        return false;
    }

    if (mEndpoint0State == EndpointStatusOut) {
        //STATUS PHASE completed, update ep0_state to idle
        mEndpoint0State = EndpointIdle;
        mDriver->stallEndpoint(usb::defualtEndpointOutAddress);
        return true;
    }

    if (mEndpoint0State == EndpointDataOut) {
        UsbEndpoint *endpoint = getEndpoint(usb::defualtEndpointOutAddress);
        if (endpoint->currentLength > endpoint->maxPacketLength) {
            endpoint->currentLength -= endpoint->maxPacketLength;

            mDriver->prepareReceive(usb::defualtEndpointOutAddress, {pdata,
                                    std::min(endpoint->currentLength, endpoint->maxPacketLength)});
            return true;
        }

        if (mState == DeviceConfigured) {
            mClassType->ep0_RxReady(this);
        }
        sendStatus();
    }

    return true;
}

bool UsbHandle::dataInStage(uint8_t epnum, uint8_t *pdata)
{
    if (epnum > 0) {
        if (mState == DeviceConfigured) {
            mClassType->dataIn(this, epnum);
            return true;
        }

        return false;
    }

    if (mEndpoint0State != EndpointDataIn) {
        if (mEndpoint0State == EndpointStatusIn || mEndpoint0State == EndpointIdle) {
            mDriver->stallEndpoint(usb::defualtEndpointInAddress);
        }

        return true;
    }

    UsbEndpoint *endpoint = getEndpoint(usb::defualtEndpointInAddress);

    if (endpoint->currentLength > endpoint->maxPacketLength) {
        endpoint->currentLength -= endpoint->maxPacketLength;

        continueSendData({pdata, (uint16_t)endpoint->currentLength});

        /* Prepare endpoint for premature end of transfer */
        mDriver->prepareReceive(usb::defualtEndpointOutAddress, {});
        return true;
    }

    /* last packet is MPS multiple, so send ZLP packet */
    if ((endpoint->lengthToSend % endpoint->maxPacketLength == 0)
        && (endpoint->lengthToSend >= endpoint->maxPacketLength)
        && (endpoint->lengthToSend < mEndpoint0Size)) {
        continueSendData({});
        mEndpoint0Size = 0;

        /* Prepare endpoint for premature end of transfer */
        mDriver->prepareReceive(usb::defualtEndpointOutAddress, {});
        return true;
    }

    if (mState == DeviceConfigured) {
        mClassType->ep0_TxSent(this);
    }
    mDriver->stallEndpoint(usb::defualtEndpointInAddress);
    receiveStatus();

    return true;
}

bool UsbHandle::isoInIncomplete(uint8_t epnum) { return true; }

bool UsbHandle::isoOUTIncomplete(uint8_t epnum) { return true; }

uint32_t UsbHandle::getRxCount(uint8_t ep_addr) { return mDriver->getRxDataSize(ep_addr); }

bool UsbHandle::sendData(std::span<uint8_t> data)
{
    mEndpoint0State = EndpointDataIn;
    UsbEndpoint *ep = getEndpoint(usb::defualtEndpointInAddress);
    ep->setLength(data.size());

    return mDriver->transmit(usb::defualtEndpointInAddress, data);
}

bool UsbHandle::sendData(uint8_t data) { return sendData({&data, sizeof(data)}); }

bool UsbHandle::continueSendData(std::span<uint8_t> data)
{
    return mDriver->transmit(usb::defualtEndpointInAddress, data);
}

bool UsbHandle::receiveData(std::span<uint8_t> buffer)
{
    mEndpoint0State = EndpointDataOut;
    UsbEndpoint *ep = getEndpoint(usb::defualtEndpointOutAddress);
    ep->setLength(buffer.size());

    return mDriver->prepareReceive(usb::defualtEndpointOutAddress, buffer);
}

bool UsbHandle::continueReceiveData(std::span<uint8_t> buffer)
{
    return mDriver->prepareReceive(usb::defualtEndpointOutAddress, buffer);
}

bool UsbHandle::sendStatus()
{
    mEndpoint0State = EndpointStatusIn;
    return mDriver->transmit(usb::defualtEndpointInAddress, {});
}

void UsbHandle::setSelfPowered(bool state) { mSelfPowered = state; }

bool UsbHandle::receiveStatus()
{
    mEndpoint0State = EndpointStatusOut;
    return mDriver->prepareReceive(usb::defualtEndpointOutAddress, {});
}

bool UsbHandle::onDeviceRequest(const UsbSetupRequest &req)
{
    bool ok = false;

    switch (req.getRequestType()) {
    case UsbSetupRequest::RequestClass:
    case UsbSetupRequest::RequestVendor: ok = mClassType->setup(this, req); break;
    case UsbSetupRequest::RequestStandart:
        switch (req.getRequest()) {
        case UsbSetupRequest::RequestGetDescriptor: ok = sendDescriptor(req); break;
        case UsbSetupRequest::RequestSetAddress: ok = setAddress(req); break;
        case UsbSetupRequest::RequestSetConfiguration: ok = setConfig(req); break;
        case UsbSetupRequest::RequestGetConfiguration: ok = sendConfig(req); break;
        case UsbSetupRequest::RequestGetStatus: ok = sendStatus(req); break;
        case UsbSetupRequest::RequestSetFeature: ok = setFeature(req); break;
        case UsbSetupRequest::RequestClearFeature: ok = clearFeature(req); break;
        default: break;
        }
        break;
    default: break;
    }

    if (ok) {
        return true;
    }

    stallEndpoints();
    return false;
}

bool UsbHandle::onInterfaceRequest(const UsbSetupRequest &req)
{
    switch (req.getRequestType()) {
    case UsbSetupRequest::RequestClass:
    case UsbSetupRequest::RequestVendor:
    case UsbSetupRequest::RequestStandart:
        switch (mState) {
        case DeviceDefault:
        case DeviceAddressed:
        case DeviceConfigured:
            if (req.getInterfaceIndex() > usb::maxInterfaceNumber) {
                break;
            }

            if (!mClassType->setup(this, req)) {
                break;
            }

            if (req.getLength() == 0) {
                sendStatus();
            }
            return true;
        default: break;
        }
        break;

    default: break;
    }

    stallEndpoints();
    return false;
}

bool UsbHandle::onEndpointRequest(const UsbSetupRequest &req)
{
    switch (req.getRequestType()) {
    case UsbSetupRequest::RequestClass:
    case UsbSetupRequest::RequestVendor:
        if (mClassType->setup(this, req)) {
            return true;
        }
        break;
    case UsbSetupRequest::RequestStandart:
        if (onStandartRequest(req)) {
            return true;
        }
        break;
    default: break;
    }

    stallEndpoints();
    return false;
}

bool UsbHandle::onStandartRequest(const UsbSetupRequest &req)
{
    switch (req.getRequest()) {
    case UsbSetupRequest::RequestSetFeature: return onSetFeature(req);
    case UsbSetupRequest::RequestClearFeature: return onClearFeature(req);
    case UsbSetupRequest::RequestGetStatus: return onGetEndpointStatus(req);
    default: break;
    }

    return false;
}

bool UsbHandle::onSetFeature(const UsbSetupRequest &req)
{
    const uint8_t endpointAddress = req.getEndpointAddress();
    const bool isUserEndpoint = endpointAddress != usb::defualtEndpointOutAddress
                                && endpointAddress != usb::defualtEndpointInAddress;

    switch (mState) {
    case DeviceAddressed:
        if (!isUserEndpoint) {
            break;
        }
        mDriver->stallEndpoint(endpointAddress);
        mDriver->stallEndpoint(usb::defualtEndpointInAddress);
        return true;

    case DeviceConfigured:
        if (req.getFeatureRequest() == UsbSetupRequest::FeatureHaltEndpoint) {
            if (isUserEndpoint && req.getLength() == 0) {
                mDriver->stallEndpoint(endpointAddress);
            }
        }
        sendStatus();
        return true;
    default: break;
    }

    return false;
}

bool UsbHandle::onClearFeature(const UsbSetupRequest &req)
{
    const uint8_t endpointAddress = req.getEndpointAddress();
    const bool isUserEndpoint = endpointAddress != usb::defualtEndpointOutAddress
                                && endpointAddress != usb::defualtEndpointInAddress;

    switch (mState) {
    case DeviceAddressed:
        if (!isUserEndpoint) {
            break;
        }
        mDriver->stallEndpoint(endpointAddress);
        mDriver->stallEndpoint(usb::defualtEndpointInAddress);
        return true;
    case DeviceConfigured:
        if (req.getFeatureRequest() == UsbSetupRequest::FeatureHaltEndpoint) {
            if (isUserEndpoint) {
                mDriver->clearStallEndpoint(endpointAddress);
            }
            sendStatus();
        }
        return true;
    default: break;
    }

    return false;
}

bool UsbHandle::onGetEndpointStatus(const UsbSetupRequest &req)
{
    const uint8_t endpointAddress = req.getEndpointAddress();
    const bool isUserEndpoint = endpointAddress != usb::defualtEndpointOutAddress
                                && endpointAddress != usb::defualtEndpointInAddress;

    UsbEndpoint *ep = getEndpoint(endpointAddress);

    switch (mState) {
    case DeviceAddressed:
        if (isUserEndpoint) {
            break;
        }
        ep->status = 0;
        sendData({(uint8_t *)(void *)&ep->status, 2});
        return true;
    case DeviceConfigured:
        if (ep->isUsed) {
            break;
        }

        if (!isUserEndpoint) {
            ep->status = 0;
        } else {
            ep->status = mDriver->isEndpointStall(endpointAddress) ? 1 : 0;
        }

        sendData({(uint8_t *)(void *)&ep->status, 2});
        return true;

    default: break;
    }

    return false;
}

bool UsbHandle::setConfig(const UsbSetupRequest &req)
{
    const uint8_t configIndex = req.getConfigIndex();
    if (configIndex > usb::maxConfigurationNumber) {
        return false;
    }

    switch (mState) {
    case DeviceAddressed:
        if (configIndex == 0) {
            sendStatus();
            return true;
        }

        mConfigIndex = configIndex;
        mState = DeviceConfigured;
        if (!setClassConfig(configIndex)) {
            break;
        }

        sendStatus();
        return true;
    case DeviceConfigured:
        if (configIndex == 0) {
            mState = DeviceAddressed;
            mConfigIndex = configIndex;
            clearClassConfig(configIndex);
            sendStatus();
            return true;
        }

        if (configIndex != mConfigIndex) {
            clearClassConfig((uint8_t)mConfigIndex);

            mConfigIndex = configIndex;
            if (!setClassConfig(configIndex)) {
                break;
            }
        }

        sendStatus();
        return true;
    default: clearClassConfig(configIndex); break;
    }

    return false;
}

bool UsbHandle::sendConfig(const UsbSetupRequest &req)
{
    if (req.getLength() != 1) {
        return false;
    }

    switch (mState) {
    case DeviceDefault:
    case DeviceAddressed:
        mConfigDefault = 0;
        return sendData(uint8_t(mConfigDefault));
    case DeviceConfigured: return sendData(uint8_t(mConfigIndex));
    default: break;
    }

    return false;
}

bool UsbHandle::sendDescriptor(const UsbSetupRequest &req)
{
    std::span<uint8_t> buffer;

    switch (req.getDescriptorType()) {
    case usb::DeviceDescriptor: buffer = mDescriptor->GetDeviceDescriptor(mSpeed); break;
    case usb::ConfigurationDescriptor: buffer = mClassType->getConfigDescriptor(mSpeed); break;
    case usb::StringDescriptor:
        buffer = mDescriptor->getStringDescriptor(mSpeed, req.getStringIndex());
        break;
    case usb::QualfierDescriptor:
        if (mSpeed != usb::HighSpeed) {
            break;
        }
        buffer = mClassType->getDeviceQualifierDescriptor();
        break;
    case usb::ConfigurationOtherSpeedDescriptor:
        if (mSpeed != usb::HighSpeed) {
            break;
        }
        buffer = mClassType->getOtherSpeedConfigDescriptor();
        break;
    default: break;
    }

    if (!buffer.data()) {
        return false;
    }

    if (req.getLength() == 0) {
        sendStatus();
        return true;
    }

    uint16_t len = std::min<uint16_t>(buffer.size(), req.getLength());
    return sendData({buffer.data(), len});
}

bool UsbHandle::setAddress(const UsbSetupRequest &req)
{
    if (mState == DeviceConfigured) {
        return false;
    }

    auto deviceAddrOpt = req.getDeviceAddress();
    if (!deviceAddrOpt) {
        return false;
    }

    mAddress = *deviceAddrOpt;
    mDriver->setUsbAddress(mAddress);
    if (!sendStatus()) {
        return false;
    }

    mState = mAddress != 0 ? DeviceAddressed : DeviceDefault;
    return true;
}

bool UsbHandle::sendStatus(const UsbSetupRequest &req)
{
    switch (mState) {
    case DeviceDefault:
    case DeviceAddressed:
    case DeviceConfigured:
        if (req.getLength() != 2) {
            break;
        }

        if (mSelfPowered) {
            mConfigStatus = USB_CONFIG_SELF_POWERED;
        }

        if (mRemoteWakeup) {
            mConfigStatus |= USB_CONFIG_REMOTE_WAKEUP;
        }

        return sendData({(uint8_t *)(void *)&mConfigStatus, 2});
    default: break;
    }

    return false;
}

bool UsbHandle::setFeature(const UsbSetupRequest &req)
{
    if (req.getFeatureRequest() == UsbSetupRequest::FeatureRemoteWakeUp) {
        mRemoteWakeup = true;
        sendStatus();
    }

    return true;
}

bool UsbHandle::clearFeature(const UsbSetupRequest &req)
{
    switch (mState) {
    case DeviceDefault:
    case DeviceAddressed:
    case DeviceConfigured:
        if (req.getFeatureRequest() == UsbSetupRequest::FeatureRemoteWakeUp) {
            mRemoteWakeup = false;
            sendStatus();
        }
        return true;
    default: break;
    }

    return false;
}

UsbEndpoint *UsbHandle::getEndpoint(uint8_t epAddress)
{
    const bool isInEndpoint = epAddress
                              & usb::defualtEndpointInAddress == usb::defualtEndpointInAddress;
    return isInEndpoint ? &mInEndpoints[epAddress & 0x0F] : &mOutEndpoints[epAddress & 0x0F];
}

void UsbHandle::stallEndpoints()
{
    mDriver->stallEndpoint(usb::defualtEndpointInAddress);
    mDriver->stallEndpoint(usb::defualtEndpointOutAddress);
}

UsbDriver *UsbHandle::getDriver() { return mDriver; }

bool UsbHandle::openEndpoint(uint8_t address, uint16_t size, EndpointType type)
{
    if (!mDriver->openEndpoint(address, type, size)) {
        return false;
    }

    UsbEndpoint *ep = getEndpoint(address);
    ep->isUsed = true;
    ep->maxPacketLength = size;

    return true;
}

bool UsbHandle::closeEndpoint(uint8_t address)
{
    if (!mDriver->closeEndpoint(address)) {
        return false;
    }

    UsbEndpoint *ep = getEndpoint(address);
    ep->isUsed = false;

    return true;
}