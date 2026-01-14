set(INC_DC "${CMAKE_CURRENT_SOURCE_DIR}/discord_presence")

set(DC_CORE
    "${INC_DC}/achievement_manager.cpp"
    "${INC_DC}/activity_manager.cpp"
    "${INC_DC}/application_manager.cpp"
    "${INC_DC}/core.cpp"
    "${INC_DC}/image_manager.cpp"
    "${INC_DC}/lobby_manager.cpp"
    "${INC_DC}/network_manager.cpp"
    "${INC_DC}/overlay_manager.cpp"
    "${INC_DC}/relationship_manager.cpp"
    "${INC_DC}/storage_manager.cpp"
    "${INC_DC}/store_manager.cpp"
    "${INC_DC}/types.cpp"
    "${INC_DC}/user_manager.cpp"
    "${INC_DC}/voice_manager.cpp")

set(DC_HEADERS
    "${INC_DC}/achievement_manager.h"
    "${INC_DC}/activity_manager.h"
    "${INC_DC}/application_manager.h"
    "${INC_DC}/core.h"
    "${INC_DC}/image_manager.h"
    "${INC_DC}/lobby_manager.h"
    "${INC_DC}/network_manager.h"
    "${INC_DC}/overlay_manager.h"
    "${INC_DC}/relationship_manager.h"
    "${INC_DC}/storage_manager.h"
    "${INC_DC}/store_manager.h"
    "${INC_DC}/types.h"
    "${INC_DC}/user_manager.h"
    "${INC_DC}/voice_manager.h")

add_library(DiscordPresence STATIC ${DC_CORE} ${DC_HEADERS})

add_library(DiscordSDK_Bin SHARED IMPORTED)
set_target_properties(DiscordSDK_Bin PROPERTIES IMPORTED_LOCATION
                                                "${INC_DC}/discord_game_sdk.so")

target_link_libraries(DiscordPresence PUBLIC DiscordSDK_Bin)
target_include_directories(
  DiscordPresence PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")
