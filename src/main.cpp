#include <QApplication>
#include <QIcon>    //AÑADI QICON
#include "ui/windows/launcher_window.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
	QIcon appIcon(":/icons/appicon"); //ASIGNO VARIABLE DE ICONO
	app.setWindowIcon(appIcon); //ESTABLEZCO ICONO
    LauncherWindow window;
    window.show();
    return app.exec();
}
