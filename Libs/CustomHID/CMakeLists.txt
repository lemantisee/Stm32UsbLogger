project(CustomHID)

add_library(${PROJECT_NAME} STATIC
    Core/UsbClass.h
    Core/UsbEndpoint.h Core/UsbEndpoint.cpp
    Core/UsbDriver.h
    Core/UsbDescriptor.h
    Core/UsbHandle.h Core/UsbHandle.cpp
    Core/UsbSetupRequest.h Core/UsbSetupRequest.cpp
    Core/usbd_def.h

    UsbCustomHid.h UsbCustomHid.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Core
) 

target_link_libraries(${PROJECT_NAME} PRIVATE
    CMSIS
    HAL
)
