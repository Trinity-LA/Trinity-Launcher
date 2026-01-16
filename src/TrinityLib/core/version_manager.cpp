#include "TrinityLib/core/version_manager.hpp"
#include "TrinityLib/core/version_config.hpp"
#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>
#include <QThread> // Para QThread::msleep (si es necesario)
#include <QDirIterator>

VersionManager::VersionManager(QObject *parent) : QObject(parent) {}

QString VersionManager::getVersionPath(const QString &versionName) {
    // Assuming standard location under GenericDataLocation
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/mcpelauncher/versions/" + versionName;
}

bool VersionManager::isVersionValid(const QString &versionName) {
    QString path = getVersionPath(versionName);
    // Check for the presence of the lib folder containing the game library
    return QDir(path + "/version_content/lib").exists();
}

QStringList VersionManager::getInstalledVersions() {
    QString versionsDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/mcpelauncher/versions/";
    QDir dir(versionsDir);
    QStringList versions = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    // Filter valid versions (those containing lib/x86_64/libminecraftpe.so)
    QStringList validVersions;
    for (const QString &version : versions) {
        if (isVersionValid(version)) {
            validVersions << version;
        }
    }
    return validVersions;
}

bool VersionManager::extractApk(const QString &apkPath, const QString &versionName, QString &errorMsg) {
    QString destDir = getVersionPath(versionName);
    if (!QDir().mkpath(destDir)) {
        errorMsg = "Could not create destination directory.";
        return false;
    }

    // Search for mcpelauncher-extract in the same directory as the Trinity executable first
    QString appDir = QCoreApplication::applicationDirPath();
    QString bundledExtractorPath = appDir + "/mcpelauncher-extract";

    QString extractorPath;
    if (QFileInfo::exists(bundledExtractorPath) && QFileInfo(bundledExtractorPath).isExecutable()) {
        extractorPath = bundledExtractorPath;
        // qDebug() << "Using bundled mcpelauncher-extract:" << extractorPath; // Optional debug output
    } else {
        // Fallback to searching in the system PATH if not found bundled
        extractorPath = QStandardPaths::findExecutable("mcpelauncher-extract");
        // qDebug() << "Using system mcpelauncher-extract:" << extractorPath; // Optional debug output
    }

    // Validate if the extractor path was found
    if (extractorPath.isEmpty()) {
        errorMsg = "mcpelauncher-extract not found.";
        return false;
    }


    // Create extraction process
    QProcess process;

    // Start the process
    process.start(extractorPath, {apkPath, destDir});

    // Wait for the process to start (without blocking the UI)
    process.waitForStarted(-1);

    // Report initial status
    emit extractionProgress("Starting extraction...");

    // Wait for the process to finish (without blocking the UI)
    process.waitForFinished(-1);

    if (process.exitCode() != 0) {
        QString err = process.readAllStandardError();
        if (err.isEmpty()) err = "Unknown error during extraction.";
        errorMsg = err;
        emit extractionProgress("Error during extraction.");
        return false;
    }

    emit extractionProgress("Extraction finished.");
    return true;
}

bool VersionManager::deleteVersion(const QString &versionName, QString &errorMsg) {
    QString versionPath = getVersionPath(versionName);
    if (!QDir(versionPath).removeRecursively()) {
        errorMsg = "Could not delete the version.";
        return false;
    }
    return true;
}

bool VersionManager::editVersion(const QString &versionName, const QString &newArgs, QString &errorMsg) {
    // Assuming you use VersionConfig to save the configuration
    VersionConfig config(versionName);
    config.setLaunchArgs(newArgs);
    if (!config.save()) {
        errorMsg = "Could not save the configuration.";
        return false;
    }
    return true;
}
