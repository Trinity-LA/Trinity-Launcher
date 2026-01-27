#include "TrinityLib/ui/windows/launcher_window.hpp"
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

    // We load Spanish on default
    QString lang = settings.value("language", "es").toString();

    // Finding files what we pack in CMakeLists for less else if ladders
    if (translator.load(":/i18n/trinity_" + lang)) {
        app.installTranslator(&translator);
    }

    QIcon appIcon(":/icons/appicon");
    app.setWindowIcon(appIcon);

    LauncherWindow window;
    window.show();
    return app.exec();
}
