#include "CustomHid.h"

#include "DataAlign.h"

namespace {
    __ALIGN_BEGIN static uint8_t reportDescriptor[customHidReportDescriptoSize] __ALIGN_END =
{
		0x06, 0x00, 0xff, // Usage Page(Undefined )
		0x09, 0x01, // USAGE (Undefined)
		0xa1, 0x01, // COLLECTION (Application)
		0x15, 0x00, // LOGICAL_MINIMUM (0)
		0x26, 0xff, 0x00, // LOGICAL_MAXIMUM (255)
		0x75, 0x08, // REPORT_SIZE (8)
		0x95, 0x40, // REPORT_COUNT (64)
		0x09, 0x01, // USAGE (Undefined)
		0x81, 0x02, // INPUT (Data,Var,Abs)
		0x95, 0x40, // REPORT_COUNT (64)
		0x09, 0x01, // USAGE (Undefined)
		0x91, 0x02, // OUTPUT (Data,Var,Abs)
		0x95, 0x40, // REPORT_COUNT (64)
		0x09, 0x01, // USAGE (Undefined)
		0xb1, 0x02, // FEATURE (Data,Var,Abs) 
		0xC0 /* END_COLLECTION */
};
}

bool CustomHid::popReport(std::span<char> buffer)
{
    if (mBuffer.size() == 0) {
        return false;
    }

    const uint32_t dataSize = std::min<uint32_t>(mBuffer.size(), buffer.size());

    std::memcpy(buffer.data(), mBuffer.data(), dataSize);
    mBuffer.clear();
    return true;
}

void CustomHid::onReceive(uint8_t *state, uint32_t size)
{
	if (mBuffer.capacity() >= size) {
		mBuffer.clear();
		mBuffer.append(state, size);
	}
}

uint8_t *CustomHid::getReportDescriptor() const { return reportDescriptor; }
