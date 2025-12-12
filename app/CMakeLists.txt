# --- TRINCHETE ---
add_executable(trinchete src/main.cpp
                         "${CMAKE_SOURCE_DIR}/resources/resources.qrc")
target_link_libraries(trinchete PRIVATE Qt6::Core Qt6::Widgets TrinityCore
                                        TrinityUI)

# --- TRINITO ---
add_executable(trinito src/trinito_main.cpp)
target_link_libraries(trinito PRIVATE Qt6::Widgets TrinityCore TrinityUI)
