#include <QApplication>
#include "ui/windows/trinito_window.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QIcon appIcon(":/icons/appicon"); //ASIGNO VARIABLE DE ICONO
    app.setWindowIcon(appIcon); //ESTABLEZCO ICONO
    TrinitoWindow window;
    window.show();
    return app.exec();
}
