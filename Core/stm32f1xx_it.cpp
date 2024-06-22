// #include "main.h"
#include "stm32f1xx_it.h"
#include "stm32f1xx_hal.h"

#include "UsbDriverF103.h"

void SysTick_Handler(void)
{
  HAL_IncTick();
}
