set(INC_DIR "${CMAKE_SOURCE_DIR}/include")

set(CORE_HEADERS
    "${INC_DIR}/TrinityLib/core/version_manager.hpp"
    "${INC_DIR}/TrinityLib/core/game_launcher.hpp"
    "${INC_DIR}/TrinityLib/core/pack_installer.hpp"
    "${INC_DIR}/TrinityLib/core/version_config.hpp"
    "${INC_DIR}/TrinityLib/core/exporter.hpp")

set(CORE_SOURCES version_manager.cpp game_launcher.cpp pack_installer.cpp
                 version_config.cpp exporter.cpp)

add_library(TrinityCore STATIC ${CORE_SOURCES} ${CORE_HEADERS})

target_include_directories(
  TrinityCore
  PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${INC_DIR}>")

target_link_libraries(TrinityCore PUBLIC Qt6::Core Qt6::Widgets)
