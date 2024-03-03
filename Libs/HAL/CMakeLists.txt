project(HAL)

add_library(${PROJECT_NAME} STATIC
    Inc/Legacy/stm32_hal_legacy.h
    Inc/stm32f1xx_hal.h
    Inc/stm32f1xx_hal_cortex.h
    Inc/stm32f1xx_hal_def.h
    Inc/stm32f1xx_hal_dma.h
    Inc/stm32f1xx_hal_dma_ex.h
    Inc/stm32f1xx_hal_exti.h
    Inc/stm32f1xx_hal_flash.h
    Inc/stm32f1xx_hal_flash_ex.h
    Inc/stm32f1xx_hal_gpio.h
    Inc/stm32f1xx_hal_gpio_ex.h
    Inc/stm32f1xx_hal_pcd.h
    Inc/stm32f1xx_hal_pcd_ex.h
    Inc/stm32f1xx_hal_pwr.h
    Inc/stm32f1xx_hal_rcc.h
    Inc/stm32f1xx_hal_rcc_ex.h
    Inc/stm32f1xx_hal_tim.h
    Inc/stm32f1xx_hal_tim_ex.h
    Inc/stm32f1xx_ll_bus.h
    Inc/stm32f1xx_ll_cortex.h
    Inc/stm32f1xx_ll_dma.h
    Inc/stm32f1xx_ll_exti.h
    Inc/stm32f1xx_ll_gpio.h
    Inc/stm32f1xx_ll_pwr.h
    Inc/stm32f1xx_ll_rcc.h
    Inc/stm32f1xx_ll_system.h
    Inc/stm32f1xx_ll_usb.h
    Inc/stm32f1xx_ll_utils.h

    Src/stm32f1xx_hal.c
    Src/stm32f1xx_hal_cortex.c
    Src/stm32f1xx_hal_dma.c
    Src/stm32f1xx_hal_exti.c
    Src/stm32f1xx_hal_flash.c
    Src/stm32f1xx_hal_flash_ex.c
    Src/stm32f1xx_hal_gpio.c
    Src/stm32f1xx_hal_gpio_ex.c
    Src/stm32f1xx_hal_pcd.c
    Src/stm32f1xx_hal_pcd_ex.c
    Src/stm32f1xx_hal_pwr.c
    Src/stm32f1xx_hal_rcc.c
    Src/stm32f1xx_hal_rcc_ex.c
    Src/stm32f1xx_hal_tim.c
    Src/stm32f1xx_hal_tim_ex.c
    Src/stm32f1xx_ll_usb.c

    stm32f1xx_hal_conf.h
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Src
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc/Legacy
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
    USE_HAL_DRIVER
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    CMSIS
)
