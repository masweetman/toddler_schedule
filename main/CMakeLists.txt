cmake_minimum_required(VERSION 3.16)

file(GLOB IMAGE_SOURCES "images/*.c")

idf_component_register(
    SRCS
        "main.c"
        ${IMAGE_SOURCES}
    INCLUDE_DIRS
        "."
    REQUIRES
        esp-box-3
        lvgl
)
