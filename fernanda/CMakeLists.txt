cmake_minimum_required(VERSION 3.16)

project(fernanda VERSION 0.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_LIST_DIR}/src)

qt_add_executable(fernanda
    src/fernanda.ui
    
    include/fernanda.h
    include/io.h
    include/pane_delegate.h
    include/pane.h
    include/rc.h
    include/texteditor.h 
    include/version.h
    
    src/fernanda.cpp
    src/io.cpp
    src/main.cpp
    src/pane.cpp
    src/pane_delegate.cpp
    src/rc.cpp
    src/texteditor.cpp
    
    rc/fernanda.rc
    rc/fonts.qrc
    rc/sample.qrc
    rc/themes.qrc
)

target_include_directories(fernanda PRIVATE include)
target_include_directories(fernanda PRIVATE src)
target_link_libraries(fernanda PRIVATE Qt6::Widgets)
