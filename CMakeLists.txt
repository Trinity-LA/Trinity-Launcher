cmake_minimum_required(VERSION 3.14)
project(Trinity LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_subdirectory(src/core)
add_subdirectory(src/ui)

# Executables
add_executable(trinchete 
	src/main.cpp
    resources/resources.qrc)
target_link_libraries(trinchete PRIVATE Qt5::Widgets TrinityCore TrinityUI)

add_executable(trinito src/trinito_main.cpp)
target_link_libraries(trinito PRIVATE Qt5::Widgets TrinityCore TrinityUI)
