#include "TrinityLib/ui/windows/trinito_window.hpp"
#include <QApplication>
#include <QIcon>
#include <QTranslator>
#include <QLocale>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QTranslator translator;
    if (translator.load(QLocale::system(), "trinity", "_", ":/i18n")) {
        app.installTranslator(&translator);
    }

    QIcon appIcon(":/icons/appicon");
    app.setWindowIcon(appIcon);
    
    TrinitoWindow window;
    window.show();
    return app.exec();
}
