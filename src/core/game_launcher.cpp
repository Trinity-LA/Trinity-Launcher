#include "game_launcher.h"
#include "version_manager.h"
#include <QProcess>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QFileInfo>

GameLauncher::GameLauncher(QObject *parent) : QObject(parent) {}

bool GameLauncher::launchGame(const QString &versionName, QString &errorMsg) {
    VersionManager vm;
    if (!vm.isVersionValid(versionName)) {
        errorMsg = QString("Los datos de '%1' están incompletos o no existen.").arg(versionName);
        return false;
    }

    QString dataDir = vm.getVersionPath(versionName);
    QString clientPath = QStandardPaths::findExecutable("mcpelauncher-client");
    
    if (clientPath.isEmpty()) {
        errorMsg = "mcpelauncher-client no encontrado.";
        return false;
    }

    return QProcess::startDetached(clientPath, QStringList() << "-dg" << dataDir);
}

bool GameLauncher::launchTrinito(QString &errorMsg) {
    QString appDir = QCoreApplication::applicationDirPath();
    QString toolsPath = appDir + "/trinito";
    
    // Check if we are in flatpak, maybe the path is different or we should use flatpak-spawn?
    // The README says: flatpak run --command=trinito com.trench.trinity.launcher
    // But from inside the app, we might just call the binary if it's in the same dir.
    // If running locally: ./trinito
    
    if (!QFileInfo::exists(toolsPath)) {
        // Fallback to searching in PATH if not in app dir (e.g. installed in /usr/bin)
        toolsPath = QStandardPaths::findExecutable("trinito");
    }

    if (toolsPath.isEmpty() || !QFileInfo::exists(toolsPath)) {
        errorMsg = "Trinito no encontrado.";
        return false;
    }

    return QProcess::startDetached(toolsPath);
}
