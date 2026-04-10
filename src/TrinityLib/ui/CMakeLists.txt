
set(UI_HEADERS
    "${TRINITY_INCLUDE_DIR}/TrinityLib/ui/windows/launcher_window.hpp"
    "${TRINITY_INCLUDE_DIR}/TrinityLib/ui/windows/trinito_window.hpp"
    "${TRINITY_INCLUDE_DIR}/TrinityLib/ui/dialogs/extract_dialog.hpp"
    "${TRINITY_INCLUDE_DIR}/TrinityLib/ui/widgets/version_selector.hpp"
    "${TRINITY_INCLUDE_DIR}/TrinityLib/ui/app_helpers.hpp")

set(UI_SOURCES windows/launcher_window.cpp windows/trinito_window.cpp
               dialogs/extract_dialog.cpp widgets/version_selector.cpp)

add_library(TrinityUI STATIC ${UI_SOURCES} ${UI_HEADERS})

# x86_64 architecture warning flag
# Disable with: cmake -DDISABLE_X86_64_WARNING=ON ..
# Enabled by default on x86_64 Linux builds
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
    if(NOT DISABLE_X86_64_WARNING)
        target_compile_definitions(TrinityUI PUBLIC SHOW_X86_64_WARNING)
    endif()
endif()

target_include_directories(TrinityUI
                           PUBLIC "$<BUILD_INTERFACE:${TRINITY_INCLUDE_DIR}>")

target_link_libraries(
  TrinityUI
  PUBLIC Qt6::Core Qt6::Widgets Qt6::Concurrent
  PRIVATE TrinityCore)
