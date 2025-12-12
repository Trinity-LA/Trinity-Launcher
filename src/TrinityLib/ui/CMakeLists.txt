set(UI_HEADERS
    "${CMAKE_SOURCE_DIR}/include/TrinityLib/ui/windows/launcher_window.hpp"
    "${CMAKE_SOURCE_DIR}/include/TrinityLib/ui/windows/trinito_window.hpp"
    "${CMAKE_SOURCE_DIR}/include/TrinityLib/ui/dialogs/extract_dialog.hpp"
    "${CMAKE_SOURCE_DIR}/include/TrinityLib/ui/widgets/version_selector.hpp")

add_library(
  TrinityUI STATIC
  windows/launcher_window.cpp windows/trinito_window.cpp
  dialogs/extract_dialog.cpp widgets/version_selector.cpp ${UI_HEADERS})

target_link_libraries(TrinityUI PRIVATE Qt6::Core Qt6::Widgets TrinityCore)
target_include_directories(TrinityUI PUBLIC "${CMAKE_SOURCE_DIR}/include")
