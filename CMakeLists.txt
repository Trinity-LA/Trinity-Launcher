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
    "i18n/trinity_es.ts"
    "i18n/trinity_en.ts"
)

qt_add_translations(Trinity
    TS_FILES ${TS_FILES}
    RESOURCE_PREFIX "/i18n"
)


add_subdirectory(src/TrinityLib/core)
add_subdirectory(src/TrinityLib/ui)

add_subdirectory(app)
