project(lwjson C CXX)

add_library(${PROJECT_NAME} STATIC
    lwjson/lwjson.h lwjson/lwjson.c
    lwjson/lwjson_opt.h
    lwjson/lwjson_stream.c
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lwjson
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
    LWJSON_IGNORE_USER_OPTS
)

target_compile_options( ${PROJECT_NAME} PUBLIC
    -mcpu=cortex-m3
)

target_link_options( ${PROJECT_NAME} PUBLIC
    -mcpu=cortex-m3
)