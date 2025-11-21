#include <QApplication>
#include "ui/windows/trinito_window.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    TrinitoWindow window;
    window.show();
    return app.exec();
}
