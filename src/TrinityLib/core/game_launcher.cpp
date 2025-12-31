#include "TrinityLib/core/game_launcher.hpp"
#include "TrinityLib/core/version_config.hpp"
#include "TrinityLib/core/version_manager.hpp"
#include <QCoreApplication>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>

GameLauncher::GameLauncher(QObject *parent)
    : QObject(parent) {}

bool GameLauncher::launchGame(const QString &versionName, QString &errorMsg) {
    VersionManager vm;
    // if (!vm.isVersionValid(versionName)) {
    //       errorMsg = QString("Los datos de '%1' están incompletos o no
    //       existen.").arg(versionName);
    //    return false;
    //   }

    QString dataDir = vm.getVersionPath(versionName);
    QString clientPath = QStandardPaths::findExecutable("mcpelauncher-client");

    if (clientPath.isEmpty()) {
        errorMsg = "mcpelauncher-client no encontrado.";
        return false;
    }

    // Leer argumentos adicionales
    VersionConfig config(versionName);
    QString extraEnv = config.getLaunchArgs();

    QStringList args;
    args << "-dg" << dataDir;

    // Si hay variables de entorno, usar QProcessEnvironment o lanzar con `env`
    if (!extraEnv.isEmpty()) {
        // Ejecutar con QProcess, usando shell para que `env` funcione
        QProcess process;
        QStringList fullCommand = {"env"};
        fullCommand += extraEnv.split(' ', Qt::SkipEmptyParts);
        fullCommand += clientPath;
        fullCommand += args;

        return process.startDetached("sh", {"-c", fullCommand.join(' ')});
    }

    return QProcess::startDetached(clientPath, args);
}

bool GameLauncher::launchTrinito(QString &errorMsg) {
    QString appDir = QCoreApplication::applicationDirPath();
    QString toolsPath = appDir + "/trinito";

    // Check if we are in flatpak, maybe the path is different or we should use
    // flatpak-spawn? The README says: flatpak run --command=trinito
    // com.trench.trinity.launcher But from inside the app, we might just call
    // the binary if it's in the same dir. If running locally: ./trinito

    if (!QFileInfo::exists(toolsPath)) {
        // Fallback to searching in PATH if not in app dir (e.g. installed in
        // /usr/bin)
        toolsPath = QStandardPaths::findExecutable("trinito");
    }

    if (toolsPath.isEmpty() || !QFileInfo::exists(toolsPath)) {
        errorMsg = "Trinito no encontrado.";
        return false;
    }

    return QProcess::startDetached(toolsPath);
}
