set(CORE_HEADERS
    "${CMAKE_SOURCE_DIR}/include/TrinityLib/core/version_manager.hpp"
    "${CMAKE_SOURCE_DIR}/include/TrinityLib/core/game_launcher.hpp"
    "${CMAKE_SOURCE_DIR}/include/TrinityLib/core/pack_installer.hpp"
    "${CMAKE_SOURCE_DIR}/include/TrinityLib/core/version_config.hpp"
    "${CMAKE_SOURCE_DIR}/include/TrinityLib/core/exporter.hpp")

add_library(
  TrinityCore STATIC version_manager.cpp game_launcher.cpp pack_installer.cpp
                     version_config.cpp exporter.cpp ${CORE_HEADERS})

target_link_libraries(TrinityCore PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
target_include_directories(TrinityCore PUBLIC "${CMAKE_SOURCE_DIR}/include")
