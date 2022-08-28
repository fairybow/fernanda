cmake_minimum_required(VERSION 3.16)

project(fernanda VERSION 0.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
set(CMAKE_AUTORCC ON)

qt_standard_project_setup()

add_executable(fernanda
    fernanda.cpp
    fernanda.ui
    main.cpp
    pane.cpp
    pane_delegate.cpp
    texteditor.cpp
    fernanda.rc
    fonts.qrc
    sample.qrc
    themes.qrc
)

target_link_libraries(fernanda PRIVATE Qt6::Widgets)