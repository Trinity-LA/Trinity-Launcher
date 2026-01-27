#include "TrinityLib/ui/windows/launcher_window.hpp"
#include "TrinityLib/ui/app_helpers.hpp"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Now more simpler!
    Trinity::UI::setupThemeAndLocale(app, "Trinity Launcher");

    LauncherWindow window;
    window.show();
    return app.exec();
}