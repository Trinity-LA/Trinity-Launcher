set(CORE_HEADERS
    "${TRINITY_INCLUDE_DIR}/TrinityLib/core/version_manager.hpp"
    "${TRINITY_INCLUDE_DIR}/TrinityLib/core/game_launcher.hpp"
    "${TRINITY_INCLUDE_DIR}/TrinityLib/core/pack_installer.hpp"
    "${TRINITY_INCLUDE_DIR}/TrinityLib/core/version_config.hpp"
    "${TRINITY_INCLUDE_DIR}/TrinityLib/core/exporter.hpp")

set(CORE_SOURCES version_manager.cpp game_launcher.cpp pack_installer.cpp
                 version_config.cpp exporter.cpp)

add_library(TrinityCore STATIC ${CORE_SOURCES} ${CORE_HEADERS})

target_include_directories(TrinityCore
                           PUBLIC "$<BUILD_INTERFACE:${TRINITY_INCLUDE_DIR}>")

target_link_libraries(TrinityCore PUBLIC Qt6::Core Qt6::Widgets)
