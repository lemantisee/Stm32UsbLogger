#include "UsbEndpoint.h"

#include "usbd_def.h"

void UsbEndpoint::setLength(uint16_t length)
{
    lengthToSend = length;
    currentLength = length;
}
