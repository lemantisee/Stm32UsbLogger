#include "UsbCore.h"

#include "usbd_core.h"

bool UsbCore::init(USBD_HandleTypeDef *pdev, USBD_DescriptorsTypeDef *pdesc, uint8_t id)
{
    if (!pdev)
    {
        return false;
    }

    if (pdev->pClass)
    {
        pdev->pClass = nullptr;
    }

         if (pdesc)
    {
        pdev->pDesc = pdesc;
    }

    pdev->dev_state = USBD_STATE_DEFAULT;
    pdev->id = id;

    return USBD_LL_Init(pdev) == USBD_OK;
}

bool UsbCore::start(USBD_HandleTypeDef *pdev)
{
    return USBD_LL_Start(pdev) == USBD_OK;
}
