#include "stm32f1xx_hal.h"

#include "UsbDevice.h"
#include "Logger.h"
#include "JsonObject.h"
#include "MonitorCommand.h"
#include "LogDump.h"

namespace {

UsbDevice usbHost;
bool enableLogs = false;
LogDump logDumper;

bool SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        return false;
    }

    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1
                                  | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        return false;
    }

    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
    PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        return false;
    }

    return true;
}

void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
}

void initLedPin()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
}

void processUsbCmd(const SString<64> &buffer)
{
    JsonObject inMessage(buffer);

    MonitorCommandId id = MonitorCommandId(inMessage.getInt("id", UnknownCommand));

    switch (id) {
    case GetLog: logDumper.dump(usbHost); break;
    case EnableLog: enableLogs = inMessage.getBool("d", enableLogs); break;
    default: break;
    }
}

} // namespace

int main(void)
{
    HAL_Init();
    if (!SystemClock_Config()) {
        return 1;
    }

    MX_GPIO_Init();

    if (!usbHost.init()) {
        return 1;
    }

    initLedPin();

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

    LOG("Yourself off its pleasant ecstatic now law.Ye their mirth seems of songs");
    LOG("Prospect out bed contempt separate");
    LOG("Her inquietude our shy yet sentiments collecting");
    LOG("Cottage fat beloved himself arrived old. Grave widow hours among him no you led");

    int count = 0;
    int logNum = 0;

    while (1) {
        if (count == 10000) {
            ++logNum;
            count = 0;
            LOG("Log line %i", logNum);
        }

        ++count;

        const SString<64> inBuffer = usbHost.popData();
        if (!inBuffer.empty()) {
            processUsbCmd(inBuffer);
        }
    }
}
