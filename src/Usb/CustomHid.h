#pragma once

#include "UsbCustomHid.h"

#include "SString.h"

class CustomHid : public UsbCustomHid
{
public:
    bool popReport(std::span<char> buffer);

protected:
    void onReceive(uint8_t *state, uint32_t size) override;
    uint8_t *getReportDescriptor() const override;

private:
    SString<128> mBuffer;
};
