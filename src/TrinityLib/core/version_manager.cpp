#include "TrinityLib/core/version_manager.hpp"
#include "TrinityLib/core/version_config.hpp"
#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>
#include <QThread> // Para QThread::msleep (si es necesario)

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

// Check for the main runtime library (libminecraftpe.so).
// See: https://github.com/minecraft-linux/mcpelauncher-manifest (GPLv3)
bool VersionManager::isVersionValid(const QString &versionName) const {
    QString libPath = getVersionPath(versionName) + "/lib/x86_64/libminecraftpe.so";
    return QFileInfo::exists(libPath);
}

// Use mcpelauncher-extract to extract an APK into the version directory.
// See: https://github.com/minecraft-linux/mcpelauncher-extract (MIT) 
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

    // Crear proceso de extracción
    QProcess process;

    // Iniciar proceso
    process.start(extractorPath, {apkPath, destDir});

    // Esperar a que termine (sin bloquear la UI)
    process.waitForStarted(-1);

    // Reportar estado inicial
    emit extractionProgress(tr("Iniciando extracción..."));

    // Esperar a que termine (sin bloquear la UI)
    while (process.state() == QProcess::Running) {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep(100); // Pausa breve para no saturar la CPU
    }

    process.waitForFinished(-1);

    if (process.exitCode() == 0) {
        emit extractionProgress(tr("Extracción completada."));
        return true;
    } else {
        QString err = process.readAllStandardError();
        if (err.isEmpty()) err = tr("Error desconocido durante la extracción.");
        errorMsg = err;
        emit extractionProgress(tr("Error durante la extracción."));
        return false;
    }
}

bool VersionManager::deleteVersion(const QString &versionName, QString &errorMsg) {
    QString versionPath = getVersionPath(versionName);

    if (!QDir(versionPath).removeRecursively()) {
        errorMsg = tr("No se pudo eliminar la versión.");
        return false;
    }

    return true;
}

bool VersionManager::editVersion(const QString &versionName, const QString &newArgs, QString &errorMsg) {
    // Suponiendo que usas VersionConfig para guardar la configuración
    VersionConfig config(versionName);
    config.setLaunchArgs(newArgs);

    if (!config.save()) {
        errorMsg = tr("No se pudo guardar la configuración.");
        return false;
    }

    return true;
}
