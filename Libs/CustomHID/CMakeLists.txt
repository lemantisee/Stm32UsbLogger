project(CustomHID)

add_library(${PROJECT_NAME} STATIC
    Core/UsbEndpoint.h Core/UsbEndpoint.cpp
    Core/UsbCore.h Core/UsbCore.cpp
    Core/UsbDescriptor.h
    Core/UsbHandle.h Core/UsbHandle.cpp
    Core/UsbSetupRequest.h Core/UsbSetupRequest.cpp
    Core/usbd_def.h

    usbd_customhid.h usbd_customhid.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Core
) 

target_link_libraries(${PROJECT_NAME} PRIVATE
    CMSIS
    HAL
)
