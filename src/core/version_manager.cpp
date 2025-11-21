#include "version_manager.h"
#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <QFileInfo>
#include <QDebug>

VersionManager::VersionManager(QObject *parent) : QObject(parent) {}

QStringList VersionManager::getInstalledVersions() const {
    QString versionsDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                          + "/mcpelauncher/versions";
    QDir dir(versionsDir);
    if (dir.exists()) {
        return dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    }
    return QStringList();
}

QString VersionManager::getVersionPath(const QString &versionName) const {
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
           + "/mcpelauncher/versions/" + versionName;
}

bool VersionManager::isVersionValid(const QString &versionName) const {
    QString libPath = getVersionPath(versionName) + "/lib/x86_64/libminecraftpe.so";
    return QFileInfo::exists(libPath);
}

bool VersionManager::extractApk(const QString &apkPath, const QString &versionName, QString &errorMsg) {
    QString destDir = getVersionPath(versionName);
    if (!QDir().mkpath(destDir)) {
        errorMsg = "No se pudo crear el directorio de destino.";
        return false;
    }

    QString extractorPath = QStandardPaths::findExecutable("mcpelauncher-extract");
    if (extractorPath.isEmpty()) {
        errorMsg = "mcpelauncher-extract no encontrado.";
        return false;
    }

    QProcess process;
    process.start(extractorPath, {apkPath, destDir});
    process.waitForFinished(-1);

    if (process.exitCode() == 0) {
        return true;
    } else {
        errorMsg = process.readAllStandardError();
        if (errorMsg.isEmpty()) errorMsg = "Error desconocido durante la extracción.";
        return false;
    }
}
