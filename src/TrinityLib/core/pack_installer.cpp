#include "TrinityLib/core/pack_installer.hpp"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

PackInstaller::PackInstaller(QObject *parent) : QObject(parent) {
    baseGameDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                  + "/mcpelauncher/games/com.mojang";
}

QString PackInstaller::getTargetName(const QString &sourcePath) {
    return QFileInfo(sourcePath).fileName();
}

bool PackInstaller::itemExists(const QString &sourcePath, const QString &targetSubdir) {
    QString targetDir = baseGameDir + "/" + targetSubdir;
    QString finalDest = targetDir + "/" + getTargetName(sourcePath);
    return QFileInfo::exists(finalDest);
}

bool PackInstaller::installItem(const QString &sourcePath, const QString &targetSubdir, bool forceOverwrite, QString &errorMsg) {
    QFileInfo sourceInfo(sourcePath);
    if (!sourceInfo.exists()) {
        errorMsg = "La ruta seleccionada no existe.";
        return false;
    }

    QString targetDir = baseGameDir + "/" + targetSubdir;
    if (!QDir().mkpath(targetDir)) {
        errorMsg = "No se pudo crear el directorio de destino.";
        return false;
    }

    QString finalDest = targetDir + "/" + sourceInfo.fileName();

    if (QFileInfo::exists(finalDest)) {
        if (!forceOverwrite) {
            errorMsg = "El elemento ya existe.";
            return false;
        }
        // Remove existing
        if (QFileInfo(finalDest).isDir()) {
            if (!QDir(finalDest).removeRecursively()) {
                errorMsg = "No se pudo eliminar el directorio existente.";
                return false;
            }
        } else {
            if (!QFile::remove(finalDest)) {
                errorMsg = "No se pudo eliminar el archivo existente.";
                return false;
            }
        }
    }

    bool success = false;
    if (sourceInfo.isDir()) {
        success = copyDirectory(sourcePath, finalDest);
    } else {
        success = QFile::copy(sourcePath, finalDest);
    }

    if (!success) {
        errorMsg = "Falló la copia del archivo o carpeta.";
        return false;
    }

    return true;
}

bool PackInstaller::copyDirectory(const QString &srcPath, const QString &dstPath) {
    QDir srcDir(srcPath);
    if (!srcDir.exists()) return false;
    QDir().mkpath(dstPath);
    for (const QFileInfo &info : srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        QString srcItem = srcPath + "/" + info.fileName();
        QString dstItem = dstPath + "/" + info.fileName();
        if (info.isDir()) {
            if (!copyDirectory(srcItem, dstItem)) return false;
        } else {
            if (!QFile::copy(srcItem, dstItem)) return false;
        }
    }
    return true;
}
