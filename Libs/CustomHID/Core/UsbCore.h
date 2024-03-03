#pragma once

#include "usbd_def.h"

class UsbCore
{
public:
    static bool init(USBD_HandleTypeDef *pdev, USBD_DescriptorsTypeDef *pdesc, uint8_t id);
    static bool start(USBD_HandleTypeDef *pdev);

private:
    UsbCore() = default;
    ~UsbCore() = default;
};
