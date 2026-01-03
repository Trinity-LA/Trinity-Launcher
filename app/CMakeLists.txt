set(RESOURCES_FILE "${CMAKE_SOURCE_DIR}/resources/resources.qrc")

# --- TRINCHETE ---
qt_add_executable(trinchete src/main.cpp ${RESOURCES_FILE})
target_link_libraries(trinchete PRIVATE Qt6::Core Qt6::Widgets TrinityCore
                                        TrinityUI)

# --- TRINITO ---
qt_add_executable(trinito src/trinito_main.cpp ${RESOURCES_FILE})

target_link_libraries(trinito PRIVATE Qt6::Core Qt6::Widgets TrinityCore
                                      TrinityUI)

qt_finalize_executable(trinchete)
qt_finalize_executable(trinito)
