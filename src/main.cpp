#include "stm32f1xx_hal.h"

#include "UsbDevice.h"
#include "Logger.h"
#include "StringBuffer.h"
#include "lwjson.h"

namespace {
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

enum PanelCommandId {
    UnknownCommand = 0,
    EchoCommand = 1,
};

PanelCommandId getCommandId(lwjson_t &root)
{
    const lwjson_token_t *idToken = lwjson_find(&root, "id");
    if (idToken && idToken->type == LWJSON_TYPE_NUM_INT) {
        return PanelCommandId(idToken->u.num_int);
    }

    return UnknownCommand;
}

StringBuffer<64> getData(lwjson_t &root)
{
    const lwjson_token_t *dataToken = lwjson_find(&root, "data");
    if (dataToken && dataToken->type == LWJSON_TYPE_STRING) {
        return StringBuffer<64>(dataToken->u.str.token_value, dataToken->u.str.token_value_len);
    }

    return {};
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

} // namespace

int main(void)
{
    HAL_Init();
    if (!SystemClock_Config()) {
        return 1;
    }

    MX_GPIO_Init();

    UsbDevice usb;
    if (!usb.init()) {
        return 1;
    }

    Logger::setPrinter(&usb);
    initLedPin();

    StringBuffer<64> inBuffer;

    lwjson_token_t tokens[4];
    lwjson_t lwjson;

    lwjson_init(&lwjson, tokens, 4);

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

    while (1) {
        inBuffer.clear();
        if (usb.popData({inBuffer.data(), inBuffer.size()})) {
            if (lwjson_parse(&lwjson, inBuffer.data()) != lwjsonOK) {
                continue;
            }

            PanelCommandId id = getCommandId(lwjson);

            switch (id) {
            case EchoCommand: {
                StringBuffer<64> data = getData(lwjson);
                if (!data.empty()) {
                    usb.sendData(data.data());
                }
            } break;
            default: break;
            }
        }
    }
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
