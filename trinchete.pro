QT += widgets
TARGET = trinchete
TEMPLATE = app

DESTDIR = bin
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/rcc
UI_DIR = build/ui

INCLUDEPATH += src
RESOURCES += resources/resources.qrc
SOURCES += src/main.cpp \
           src/core/version_manager.cpp \
           src/core/game_launcher.cpp \
           src/core/version_config.cpp  \
           src/ui/windows/launcher_window.cpp \
           src/ui/dialogs/extract_dialog.cpp

HEADERS += src/core/version_manager.h \
           src/core/game_launcher.h \
           src/core/version_config.h  \
           src/ui/windows/launcher_window.h \
           src/ui/dialogs/extract_dialog.h
