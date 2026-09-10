# Qc66c - a Qt 6 GUI for the RDTech TC66/TC66C USB power meter
# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2026 Nita Vesa

cmake_minimum_required(VERSION 3.16)

project(qc66c VERSION 0.7.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Keep in step with the version the release workflow installs.
set(QC66C_QT_VERSION "6.10.3")
find_package(Qt6 ${QC66C_QT_VERSION} REQUIRED COMPONENTS Widgets)
if(NOT Qt6_VERSION VERSION_EQUAL QC66C_QT_VERSION)
    message(WARNING
        "Found Qt ${Qt6_VERSION}; the release workflow builds with "
        "${QC66C_QT_VERSION}. Behaviour may differ.")
endif()

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
    resources/qc66c.qrc
)

set_target_properties(qc66c PROPERTIES
    OUTPUT_NAME "Qc66c"
)

if(WIN32)
    set_target_properties(qc66c PROPERTIES WIN32_EXECUTABLE TRUE)
    enable_language(RC)
    target_sources(qc66c PRIVATE
        src/SerialPortWin.cpp
        resources/qc66c.rc
    )
    target_include_directories(qc66c PRIVATE resources)
else()
    target_sources(qc66c PRIVATE src/SerialPortPosix.cpp)
    set_target_properties(qc66c PROPERTIES
        INSTALL_RPATH "$ORIGIN/../lib/qc66c"
    )
endif()

if(NOT APP_VERSION)
    set(APP_VERSION "${PROJECT_VERSION}")
endif()

target_compile_definitions(qc66c PRIVATE APP_VERSION="${APP_VERSION}")

target_include_directories(qc66c PRIVATE src)

target_link_libraries(qc66c PRIVATE
    Qt6::Widgets
    tiny-aes-c
)

if(WIN32)
    target_link_libraries(qc66c PRIVATE setupapi advapi32)
endif()

# Installation and deployment. On Linux this bundles the Qt runtime into the
# install tree via Qt's CMake deployment support.
install(TARGETS qc66c
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

if(UNIX AND NOT APPLE)
    install(FILES resources/qc66c.desktop DESTINATION share/applications)
    foreach(size 16 24 32 48 64 128 256)
        install(FILES resources/icons/qc66c_${size}.png
            DESTINATION share/icons/hicolor/${size}x${size}/apps
            RENAME qc66c.png)
    endforeach()
endif()

# On Linux, Qt's generic deploy tool uses CMake's file(RPATH_SET), which cannot
# grow a shorter existing RUNPATH (e.g. some plugins). Prefer patchelf, which
# can set an arbitrary RPATH. patchelf must be installed on the build host.
set(QT_DEPLOY_USE_PATCHELF ON)

qt_generate_deploy_app_script(
    TARGET qc66c
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

# Keep the bundled Qt runtime in a private directory so it cannot clash with
# (or overwrite) a distribution's own Qt libraries.
install(CODE "set(QT_DEPLOY_LIB_DIR \"lib/qc66c\")")
install(CODE "set(QT_DEPLOY_PLUGINS_DIR \"lib/qc66c/plugins\")")
install(SCRIPT ${deploy_script})

# Linux packaging (.deb). The portable .tar.gz is assembled in CI from the
# install tree, which keeps it relocatable.
if(UNIX AND NOT APPLE)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_PACKAGE_NAME "qc66c")
    set(CPACK_PACKAGE_VENDOR "Nita Vesa")
    set(CPACK_PACKAGE_CONTACT "Nita Vesa <nita.vesa@outlook.com>")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
        "Qt GUI for the RDTech TC66/TC66C USB power meter")
    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
    set(CPACK_DEBIAN_PACKAGE_NAME "qc66c")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Nita Vesa <nita.vesa@outlook.com>")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    include(CPack)
endif()
