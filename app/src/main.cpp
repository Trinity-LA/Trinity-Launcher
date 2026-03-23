#include "TrinityLib/ui/app_helpers.hpp"
#include "TrinityLib/ui/windows/launcher_window.hpp"
#include <QApplication>
#include <QFontDatabase>
#include <QDebug>
#include <TrinityLib/core/discord_manager.hpp>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Load Roboto font (clean modern sans-serif)
    int fontId = QFontDatabase::addApplicationFont(":/fonts/Roboto.ttf");
    if (fontId != -1) {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        if (!fontFamilies.isEmpty()) {
            app.setFont(QFont(fontFamilies.first(), 11));
        }
    }

    // Load Pixellari font for title (pixel-style font)
    int pixellariFontId = QFontDatabase::addApplicationFont(":/fonts/Pixellari.ttf");
    if (pixellariFontId != -1) {
        qDebug() << "[Font] Pixellari loaded successfully";
    }

    // Now more simpler!
    Trinity::UI::setupThemeAndLocale(app, "");

    DiscordManager::instance().init(1460749513667907658);
    DiscordManager::instance().updateActivityMain();

    LauncherWindow window;
    window.show();
    return app.exec();
}
