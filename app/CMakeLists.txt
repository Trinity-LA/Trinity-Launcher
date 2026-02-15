set(RESOURCES_FILE "${CMAKE_SOURCE_DIR}/resources/resources.qrc")

# --- TRINITY (single binary) ---
qt_add_executable(trinity src/main.cpp ${RESOURCES_FILE})
add_custom_command(
  TARGET trinity
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/external/discord_presence/discord_game_sdk.so"
    $<TARGET_FILE_DIR:trinity>)

target_link_libraries(trinity PRIVATE Qt6::Core Qt6::Widgets DiscordPresence
                                      TrinityCore TrinityUI)

set_target_properties(trinity PROPERTIES
    BUILD_RPATH "$ORIGIN"
    INSTALL_RPATH "$ORIGIN"
)

qt_finalize_executable(trinity)
