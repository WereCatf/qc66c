# Qc66c - a Qt 6 GUI for the RDTech TC66/TC66C USB power meter
# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2026 Nita Vesa

cmake_minimum_required(VERSION 3.16)

project(qc66c VERSION 0.5.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(tiny-aes-c STATIC
    third_party/tiny-aes-c/aes.c
)
target_include_directories(tiny-aes-c PUBLIC third_party/tiny-aes-c)

qt_add_executable(qc66c
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/Tc66Device.cpp
    src/Tc66Device.h
    src/Tc66Protocol.cpp
    src/Tc66Protocol.h
    src/PreferencesDialog.cpp
    src/PreferencesDialog.h
    src/SerialPort.cpp
    src/SerialPort.h
)

set_target_properties(qc66c PROPERTIES
    WIN32_EXECUTABLE TRUE
    OUTPUT_NAME "Qc66c"
)

target_compile_definitions(qc66c PRIVATE APP_VERSION="${PROJECT_VERSION}")

target_include_directories(qc66c PRIVATE src)

target_link_libraries(qc66c PRIVATE
    Qt6::Widgets
    tiny-aes-c
    setupapi
    advapi32
)
