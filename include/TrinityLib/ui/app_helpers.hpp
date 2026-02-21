#ifndef APP_HELPERS_HPP
#define APP_HELPERS_HPP

#include <QApplication>
#include <QIcon>
#include <QLocale>
#include <QSettings>
#include <QTranslator>
#include <QFile>
#include <QCoreApplication>

namespace Trinity::UI {
    /**
    * Inicializa los parámetros comunes para todas las aplicaciones Trinity.
    */
    inline void setupThemeAndLocale(QApplication &app, const QString &appName) {
        QCoreApplication::setOrganizationName("Trench");
        QCoreApplication::setApplicationName("Trinity Launcher"); 

        // Default Spanish language
        QString systemLang = QLocale::system().name().split('_').first();
        if (!QFile::exists(":/i18n/trinity_" + systemLang + ".qm") && systemLang != "es") {
            systemLang = "es";
        }

        // Creamos un objeto QSettings y cargamos el idioma.
        QSettings settings; 
        QString lang = settings.value("language", systemLang).toString();

        // 3. Descargar traducción
        static QTranslator translator; 
        if (translator.load(":/i18n/trinity_" + lang)) {
            app.installTranslator(&translator);
        }

        app.setWindowIcon(QIcon(":/icons/appicon"));
    }
}

#endif