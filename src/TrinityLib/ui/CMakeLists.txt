set(INC_DIR "${CMAKE_SOURCE_DIR}/include")
set(TS_DIR "${CMAKE_SOURCE_DIR}/resources/i18n")

file(MAKE_DIRECTORY ${TS_DIR})

set(UI_HEADERS
    "${INC_DIR}/TrinityLib/ui/windows/launcher_window.hpp"
    "${INC_DIR}/TrinityLib/ui/windows/trinito_window.hpp"
    "${INC_DIR}/TrinityLib/ui/dialogs/extract_dialog.hpp"
    "${INC_DIR}/TrinityLib/ui/widgets/version_selector.hpp")

set(UI_SOURCES windows/launcher_window.cpp windows/trinito_window.cpp
               dialogs/extract_dialog.cpp widgets/version_selector.cpp)

add_library(TrinityUI STATIC ${UI_SOURCES} ${UI_HEADERS})

target_include_directories(
  TrinityUI PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${INC_DIR}>")

target_link_libraries(
  TrinityUI
  PUBLIC Qt6::Core Qt6::Widgets
  PRIVATE TrinityCore)

qt_add_translations(
  TrinityUI
  TS_FILE_BASE
  "trinity"
  TS_FILE_DIR
  "${TS_DIR}"
  RESOURCE_PREFIX
  "/i18n"
  SOURCES
  ${PROJECT_TRANSLATABLE_FILES})
