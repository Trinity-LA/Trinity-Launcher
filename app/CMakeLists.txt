set(RESOURCES_FILE "${CMAKE_SOURCE_DIR}/resources/resources.qrc")

# --- TRINITY (single binary) ---
qt_add_executable(trinity src/main.cpp ${RESOURCES_FILE})

target_link_libraries(trinity PRIVATE Qt6::Core Qt6::Widgets TrinityCore TrinityUI)

qt_finalize_executable(trinity)
