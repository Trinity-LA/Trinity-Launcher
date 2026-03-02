cmake_minimum_required(VERSION 3.17)
project(Trinity LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(TRINITY_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

include_directories(include)

find_package(
  Qt6
  COMPONENTS Core Widgets Concurrent LinguistTools
  REQUIRED)

# We define which language use
qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES en ca uk es pt pt_BR)

set(TRINITY_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(TRINITY_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

file(GLOB_RECURSE PROJECT_TRANSLATABLE_FILES "${TRINITY_SRC_DIR}/*.cpp"
     "${TRINITY_INC_DIR}/*.hpp")

add_subdirectory(external)

add_subdirectory(src/TrinityLib/core)
add_subdirectory(src/TrinityLib/ui)

add_subdirectory(app)

# --- Translations ---
# Must be after add_subdirectory(app) so trinity target exists
# Using qt_add_lrelease + qt_add_resources for Qt6 < 6.7 compatibility
set(TS_DIR "${CMAKE_SOURCE_DIR}/resources/i18n")

set(TS_FILES
  "${TS_DIR}/trinity_en.ts"
  "${TS_DIR}/trinity_ca.ts"
  "${TS_DIR}/trinity_uk.ts"
  "${TS_DIR}/trinity_es.ts"
  "${TS_DIR}/trinity_pt.ts"
  "${TS_DIR}/trinity_pt_BR.ts")

set(QM_FILES
  "${TS_DIR}/trinity_en.qm"
  "${TS_DIR}/trinity_ca.qm"
  "${TS_DIR}/trinity_uk.qm"
  "${TS_DIR}/trinity_es.qm"
  "${TS_DIR}/trinity_pt.qm"
  "${TS_DIR}/trinity_pt_BR.qm")

# Embed .qm resources into trinity
qt_add_resources(trinity "translations"
  PREFIX "/i18n"
  BASE "${TS_DIR}"
  FILES ${QM_FILES})
