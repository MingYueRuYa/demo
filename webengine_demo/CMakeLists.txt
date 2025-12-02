cmake_minimum_required(VERSION 3.21)

project(WebEngineDemo VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT DEFINED Qt6_DIR AND NOT DEFINED Qt5_DIR)
    message(STATUS "请确保已经正确安装 Qt 并设置 Qt6_DIR 或 Qt5_DIR。")
endif()

find_package(Qt6 6.4 COMPONENTS Core Gui Widgets WebEngineWidgets WebChannel REQUIRED)

qt_add_executable(WebEngineDemo
    src/main.cpp
    src/browserwindow.cpp
    src/browserwindow.h
    src/configmanager.cpp
    src/configmanager.h
    src/messageconsole.cpp
    src/messageconsole.h
    src/webenginepane.cpp
    src/webenginepane.h
    src/webbridge.cpp
    src/webbridge.h
    resources.qrc
)

target_compile_features(WebEngineDemo PRIVATE cxx_std_17)

target_compile_definitions(WebEngineDemo PRIVATE
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060400
)

target_link_libraries(WebEngineDemo PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::WebEngineWidgets
    Qt6::WebChannel
)

set_target_properties(WebEngineDemo PROPERTIES
    WIN32_EXECUTABLE ON
)

if(MSVC)
    target_compile_options(WebEngineDemo PRIVATE /utf-8)
endif()

