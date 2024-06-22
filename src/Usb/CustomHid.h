#pragma once

#include "UsbCustomHid.h"

class UsbDriver;

class CustomHid : public UsbCustomHid
{
protected:
    void onReceive(uint8_t *state) override;
    uint8_t *getReportDescriptor() const override;
};
