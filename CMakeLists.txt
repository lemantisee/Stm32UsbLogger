cmake_minimum_required(VERSION 3.20)

project("USB_logger" C CXX ASM)
 
add_executable(${PROJECT_NAME})

target_sources(
    ${PROJECT_NAME} PRIVATE
    src/main.cpp
    src/Usb/UsbCoreF103.h src/Usb/UsbCoreF103.cpp
    src/Usb/UsbDeviceDescriptor.h src/Usb/UsbDeviceDescriptor.cpp
    src/Usb/UsbDevice.h src/Usb/UsbDevice.cpp
    src/Usb/UsbPcdCallbacks.cpp
    src/Usb/usbd_custom_hid_if.h src/Usb/usbd_custom_hid_if.cpp

    Core/stm32f1xx_hal_msp.c
    Core/stm32f1xx_it.cpp
    Core/syscalls.c
    Core/sysmem.c
    Core/system_stm32f1xx.c
    Core/startup_stm32f103c8tx.s
)

add_subdirectory(Libs)

target_include_directories( ${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/Usb
    ${PROJECT_SOURCE_DIR}/Core
)

target_link_libraries( ${PROJECT_NAME} PRIVATE
    CMSIS
    HAL
    CustomHID
)

target_compile_definitions( ${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:DEBUG>"
)

target_link_options( ${PROJECT_NAME} PRIVATE
    -Wl,--print-memory-usage
    -T${PROJECT_SOURCE_DIR}/STM32F103C8TX_FLASH.ld
)

target_compile_options( ${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:-g3>"
    "$<$<NOT:$<CONFIG:Debug>>:-g0>"
    "$<$<NOT:$<CONFIG:Debug>>:-Os>"
)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}>
)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex
    $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.hex
)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary
    $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.bin
)