#include "TrinityLib/core/game_launcher.hpp"
#include "TrinityLib/core/version_config.hpp"
#include "TrinityLib/core/version_manager.hpp"
#include <QCoreApplication>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QDir> 
GameLauncher::GameLauncher(QObject *parent)
    : QObject(parent) {}

bool GameLauncher::launchGame(const QString &versionName, QString &errorMsg) {
    VersionManager vm;
    QString dataDir = vm.getVersionPath(versionName);

    // Search for mcpelauncher-client in the same directory as the Trinity executable first
    QString appDir = QCoreApplication::applicationDirPath();
    QString bundledClientPath = appDir + "/mcpelauncher-client";

    QString clientPath;
    if (QFileInfo::exists(bundledClientPath) && QFileInfo(bundledClientPath).isExecutable()) {
        clientPath = bundledClientPath;
        // qDebug() << "Using bundled mcpelauncher-client:" << clientPath; // Optional debug output
    } else {
        // Fallback to searching in the system PATH if not found bundled
        clientPath = QStandardPaths::findExecutable("mcpelauncher-client");
        // qDebug() << "Using system mcpelauncher-client:" << clientPath; // Optional debug output
    }
    // --- CORRECTION ENDS HERE ---

    // Validate if the client path was found
    if (clientPath.isEmpty() || !QFileInfo::exists(clientPath)) {
        errorMsg = "mcpelauncher-client not found.";
        return false;
    }

    // Read additional launch arguments
    VersionConfig config(versionName);
    QString extraEnv = config.getLaunchArgs();

    QStringList args;
    args << "-dg" << dataDir;

    // If there are extra environment variables, launch with `env`
    if (!extraEnv.isEmpty()) {
        // Execute with QProcess, using shell for `env` to work
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
    // Search for trinito in the same directory as the Trinity executable first
    QString appDir = QCoreApplication::applicationDirPath();
    QString toolsPath = appDir + "/trinito";

    if (!QFileInfo::exists(toolsPath)) {
        // Fallback to searching in the system PATH if not found bundled
        toolsPath = QStandardPaths::findExecutable("trinito");
    }

    if (toolsPath.isEmpty() || !QFileInfo::exists(toolsPath)) {
        errorMsg = "Trinito not found.";
        return false;
    }

    return QProcess::startDetached(toolsPath);
}
