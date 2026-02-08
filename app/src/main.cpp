#include "TrinityLib/ui/app_helpers.hpp"
#include "TrinityLib/ui/windows/launcher_window.hpp"
#include <QApplication>
#include <TrinityLib/core/discord_manager.hpp>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Now more simpler!
    Trinity::UI::setupThemeAndLocale(app, "Trinity Launcher");

    DiscordManager::instance().init(1460749513667907658);
    DiscordManager::instance().updateActivityMain();

    LauncherWindow window;
    window.show();
    return app.exec();
}
