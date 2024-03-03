#include "UsbCore.h"

namespace
{
    UsbCore *coreImpl = nullptr;
} // namespace


void UsbCore::setImpl(UsbCore *impl)
{
    coreImpl = impl;
}

UsbCore *UsbCore::ref()
{
    return coreImpl;
}
