set(RESOURCES_FILE "${CMAKE_SOURCE_DIR}/resources/resources.qrc")

# --- TRINCHETE ---
qt_add_executable(trinchete src/main.cpp ${RESOURCES_FILE})
add_custom_command(
  TARGET trinchete
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/external/discord_presence/discord_game_sdk.so"
    $<TARGET_FILE_DIR:trinchete>)

target_link_libraries(trinchete PRIVATE Qt6::Core Qt6::Widgets DiscordPresence
                                        TrinityCore TrinityUI)

# --- TRINITO ---
qt_add_executable(trinito src/trinito_main.cpp ${RESOURCES_FILE})

target_link_libraries(trinito PRIVATE Qt6::Core Qt6::Widgets TrinityCore
                                      TrinityUI)

qt_finalize_executable(trinchete)
qt_finalize_executable(trinito)
