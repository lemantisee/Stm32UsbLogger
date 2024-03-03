project(CMSIS)

add_library(${PROJECT_NAME} INTERFACE
    Device/ST/STM32F1xx/Include/stm32f103xb.h
    Device/ST/STM32F1xx/Include/stm32f1xx.h
    Device/ST/STM32F1xx/Include/system_stm32f1xx.h

    Include/cmsis_armcc.h
    Include/cmsis_armclang.h
    Include/cmsis_armcc.h
    Include/cmsis_armclang.h
    Include/cmsis_compiler.h
    Include/cmsis_gcc.h
    Include/cmsis_iccarm.h
    Include/cmsis_version.h
    Include/core_armv8mbl.h
    Include/core_armv8mml.h
    Include/core_cm0.h
    Include/core_cm0plus.h
    Include/core_cm1.h
    Include/core_cm23.h
    Include/core_cm3.h
    Include/core_cm33.h
    Include/core_cm4.h
    Include/core_cm7.h
    Include/core_sc000.h
    Include/core_sc300.h
    Include/mpu_armv7.h
    Include/mpu_armv8.h
    Include/tz_context.h
)

target_include_directories(${PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Device/ST/STM32F1xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

target_compile_definitions( ${PROJECT_NAME} INTERFACE
    STM32F103xB
)

target_compile_options( ${PROJECT_NAME} INTERFACE
    -mcpu=cortex-m3
)

target_link_options( ${PROJECT_NAME} INTERFACE
    -mcpu=cortex-m3
)
