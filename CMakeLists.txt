cmake_minimum_required(VERSION 3.17)
project(Trinity LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include_directories(include)

find_package(Qt6 COMPONENTS Core Widgets LinguistTools REQUIRED) 


set(TS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/i18n/trinity_es.ts"
    "${CMAKE_CURRENT_SOURCE_DIR}/i18n/trinity_en.ts"
)


add_subdirectory(src/TrinityLib/core)
add_subdirectory(src/TrinityLib/ui)

add_subdirectory(app)
