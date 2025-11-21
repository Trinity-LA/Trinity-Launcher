QT += widgets
TARGET = trinito
TEMPLATE = app

DESTDIR = bin
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/rcc
UI_DIR = build/ui

INCLUDEPATH += src

SOURCES += src/trinito_main.cpp \
           src/core/pack_installer.cpp \
           src/ui/windows/trinito_window.cpp

HEADERS += src/core/pack_installer.h \
           src/ui/windows/trinito_window.h
