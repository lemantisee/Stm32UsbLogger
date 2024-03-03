project(CustomHID)

add_library(${PROJECT_NAME} STATIC
    Core/UsbCore.h Core/UsbCore.cpp
    Core/usbd_ctlreq.cpp
    Core/usbd_ctlreq.h
    Core/usbd_def.h
    Core/usbd_ioreq.cpp
    Core/usbd_ioreq.h

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
