add_library(TrinityCore
    version_manager.cpp
    version_manager.h
    game_launcher.cpp
    game_launcher.h
    pack_installer.cpp
    pack_installer.h
)
target_link_libraries(TrinityCore PRIVATE Qt5::Core)
target_include_directories(TrinityCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
