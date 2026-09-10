cmake_minimum_required(VERSION 3.16)

project(tc66-monitor VERSION 0.3.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(tiny-aes-c STATIC
    third_party/tiny-aes-c/aes.c
)
target_include_directories(tiny-aes-c PUBLIC third_party/tiny-aes-c)

qt_add_executable(tc66-monitor
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

set_target_properties(tc66-monitor PROPERTIES
    WIN32_EXECUTABLE TRUE
)

target_include_directories(tc66-monitor PRIVATE src)

target_link_libraries(tc66-monitor PRIVATE
    Qt6::Widgets
    tiny-aes-c
    setupapi
    advapi32
)
