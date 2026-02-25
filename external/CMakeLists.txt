# DiscordPresence is now a no-op interface target.
# The Game SDK (.so) has been replaced by a pure POSIX IPC implementation
# in discord_manager.cpp — no external library needed.
add_library(DiscordPresence INTERFACE)
