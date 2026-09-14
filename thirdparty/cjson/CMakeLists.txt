project(cJSON LANGUAGES C)

add_library(cJSON STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/cJSON.c
        ${CMAKE_CURRENT_SOURCE_DIR}/cJSON_Utils.c
)

target_include_directories(cJSON PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})