#include "TrinityLib/ui/app_helpers.hpp"
#include "TrinityLib/ui/windows/launcher_window.hpp"
#include <QApplication>
#include <QFontDatabase>
#include <QDebug>
#include <TrinityLib/core/discord_manager.hpp>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Load custom font
    int fontId = QFontDatabase::addApplicationFont(":/fonts/TerminessNerdFontMono-Regular.ttf");
    if (fontId != -1) {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        if (!fontFamilies.isEmpty()) {
            app.setFont(QFont(fontFamilies.first(), 12));
        }
    }

    // Now more simpler!
    Trinity::UI::setupThemeAndLocale(app, "");

    DiscordManager::instance().init(1460749513667907658);
    DiscordManager::instance().updateActivityMain();

    LauncherWindow window;
    window.show();
    return app.exec();
}
