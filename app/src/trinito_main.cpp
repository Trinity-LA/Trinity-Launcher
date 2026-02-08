#include "TrinityLib/ui/windows/trinito_window.hpp"
#include "TrinityLib/ui/app_helpers.hpp"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Ditto!
    Trinity::UI::setupThemeAndLocale(app, "Trinito Tool");

    TrinitoWindow window;
    window.show();
    return app.exec();
}