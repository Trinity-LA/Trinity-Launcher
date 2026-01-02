#include "TrinityLib/ui/windows/launcher_window.hpp"
#include <QApplication>
#include <QIcon>
#include <QTranslator>
#include <QLocale>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // --- Lógica de Traducción ---
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "trinity_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    QIcon appIcon(":/icons/appicon");
    app.setWindowIcon(appIcon);
    
    LauncherWindow window;
    window.show();
    return app.exec();
}
