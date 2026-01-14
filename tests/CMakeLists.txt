add_executable(discord_test discord_presence.cpp)
target_link_libraries(discord_test PRIVATE DiscordPresence)

add_custom_command(
  TARGET discord_test
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/external/discord_presence/discord_game_sdk.so"
    $<TARGET_FILE_DIR:discord_test>)
