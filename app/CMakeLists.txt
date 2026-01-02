set(RESOURCES_FILE "../resources/resources.qrc")

# --- TRINCHETE ---
add_executable(trinchete src/main.cpp ${RESOURCES_FILE})
target_link_libraries(trinchete PRIVATE Qt6::Core Qt6::Widgets TrinityCore
                                        TrinityUI)

# --- TRINITO ---
add_executable(trinito src/trinito_main.cpp ${RESOURCES_FILE})
target_link_libraries(trinito PRIVATE Qt6::Core Qt6::Widgets TrinityCore
                                      TrinityUI)
qt_add_translations(trinchete
    TS_FILES ${TS_FILES}
    RESOURCE_PREFIX "/i18n"
)

# ... después de add_executable(trinito ...)
qt_add_translations(trinito
    TS_FILES ${TS_FILES}
    RESOURCE_PREFIX "/i18n"
)
