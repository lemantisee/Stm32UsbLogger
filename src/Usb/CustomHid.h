#pragma once

#include "UsbCustomHid.h"

class UsbDriver;

class CustomHid : public UsbCustomHid
{
protected:
    void onReceive(uint8_t *state, uint32_t size) override;
    uint8_t *getReportDescriptor() const override;
};
