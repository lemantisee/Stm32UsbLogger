#pragma once

#include "UsbCustomHid.h"

#include "StringBuffer.h"

class CustomHid : public UsbCustomHid
{
public:
    bool popReport(std::span<char> buffer);

protected:
    void onReceive(uint8_t *state, uint32_t size) override;
    uint8_t *getReportDescriptor() const override;

private:
    StringBuffer<128> mBuffer;
};
