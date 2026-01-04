#include "TrinityLib/ui/windows/trinito_window.hpp"
#include <QApplication>
#include <QIcon>
#include <QLocale>
#include <QSettings>
#include <QTranslator>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Trench");
    QCoreApplication::setApplicationName("Trinity Launcher");

    QTranslator translator;
    QSettings settings;
    QString lang = settings.value("language", "es").toString();

    if (lang == "en") {
        if (translator.load(":/i18n/trinity_en")) {
            app.installTranslator(&translator);
        }
    }

    QIcon appIcon(":/icons/appicon");
    app.setWindowIcon(appIcon);

    TrinitoWindow window;
    window.show();
    return app.exec();
}
