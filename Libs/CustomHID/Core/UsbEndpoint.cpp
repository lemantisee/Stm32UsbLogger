#include "UsbEndpoint.h"

#include "usbd_def.h"

void UsbEndpoint::setLength(uint16_t length)
{
    total_length = length;
    rem_length = length;
}
