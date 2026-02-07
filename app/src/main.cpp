#include "TrinityLib/ui/windows/launcher_window.hpp"
#include <QApplication>
#include <QIcon>
#include <QLocale>
#include <QSettings>
#include <QString>
#include <QTranslator>
#include <TrinityLib/core/discord_manager.hpp>
#include <qobject.h>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    DiscordManager::instance().init(1460749513667907658);
    DiscordManager::instance().updateActivityMain();
    QCoreApplication::setOrganizationName("Trench");
    QCoreApplication::setApplicationName("Trinity Launcher");

    QTranslator translator;
    QSettings settings;

    QString lang = settings.value("language", "es").toString();

    if (lang == "en") {
        if (translator.load(":/i18n/trinity_en")) {
            app.installTranslator(&translator);
        }
    } else if (lang == "ca") {
        if (translator.load(":/i18n/trinity_ca")) {
            app.installTranslator(&translator);
        }
    }

    QIcon appIcon(":/icons/appicon");
    app.setWindowIcon(appIcon);

    LauncherWindow window;
    window.show();
    return app.exec();
}
