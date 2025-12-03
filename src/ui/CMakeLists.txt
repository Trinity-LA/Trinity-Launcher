add_library(TrinityUI
    windows/launcher_window.cpp
    windows/launcher_window.h
    windows/trinito_window.cpp
    windows/trinito_window.h
    dialogs/extract_dialog.cpp
    dialogs/extract_dialog.h
)
target_link_libraries(TrinityUI PRIVATE Qt5::Widgets TrinityCore)
target_include_directories(TrinityUI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
