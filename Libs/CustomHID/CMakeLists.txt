project(CustomHID)

add_library(${PROJECT_NAME} STATIC
    Core/UsbCore.h Core/UsbCore.cpp
    Core/usbd_core.c
    Core/usbd_core.h
    Core/usbd_ctlreq.c
    Core/usbd_ctlreq.h
    Core/usbd_def.h
    Core/usbd_ioreq.c
    Core/usbd_ioreq.h

    usbd_conf.h usbd_conf.c
    usbd_customhid.h usbd_customhid.c
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Core
) 

target_link_libraries(${PROJECT_NAME} PRIVATE
    CMSIS
    HAL
)
