# discord_presence.cpp uses the old Game SDK headers which have been removed.
# The SDK was replaced with a native IPC implementation in discord_manager.cpp.
# This test target is kept as a stub to avoid CMake errors but does nothing.
# add_executable(discord_test discord_presence.cpp)
