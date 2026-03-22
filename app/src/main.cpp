#include "TrinityLib/ui/app_helpers.hpp"
#include "TrinityLib/ui/windows/launcher_window.hpp"
#include <QApplication>
#include <QFontDatabase>
#include <QDebug>
#include <TrinityLib/core/discord_manager.hpp>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    int fontId = QFontDatabase::addApplicationFont(":/fonts/TerminusTTF.ttf");
    if (fontId != -1) {
        qDebug() << "Loaded TerminusTTF font families:" << QFontDatabase::applicationFontFamilies(fontId);
    } else {
        qWarning() << "Failed to load TerminusTTF font!";
    }

    // Now more simpler!
    Trinity::UI::setupThemeAndLocale(app, "");

    DiscordManager::instance().init(1460749513667907658);
    DiscordManager::instance().updateActivityMain();

    LauncherWindow window;
    window.show();
    return app.exec();
}
