set(RESOURCES_FILE "${CMAKE_SOURCE_DIR}/resources/resources.qrc")

# --- TRINITY (single binary) ---
add_executable(trinity src/main.cpp ${RESOURCES_FILE} ${TRANSLATION_RESOURCES})

target_link_libraries(trinity PRIVATE Qt5::Core Qt5::Widgets TrinityCore TrinityUI)
