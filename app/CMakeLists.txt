set(RESOURCES_FILE "../resources/resources.qrc")

# --- TRINCHETE ---
add_executable(trinchete src/main.cpp ${RESOURCES_FILE})
target_link_libraries(trinchete PRIVATE Qt6::Core Qt6::Widgets TrinityCore
                                        TrinityUI)

# --- TRINITO ---
add_executable(trinito src/trinito_main.cpp ${RESOURCES_FILE})
target_link_libraries(trinito PRIVATE Qt6::Core Qt6::Widgets TrinityCore
                                      TrinityUI)
