#include "TrinityLib/core/version_manager.hpp"
#include "TrinityLib/core/version_config.hpp"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QFileInfo>
#include <QCoreApplication>
#include <QSysInfo>
#include <QThread> // Para QThread::msleep (si es necesario)

VersionManager::VersionManager(QObject *parent) : QObject(parent) {}

bool VersionManager::isFlatpak() {
    static bool flatpak = QCoreApplication::applicationDirPath().startsWith("/app")
                          || !qEnvironmentVariableIsEmpty("FLATPAK_ID");
    return flatpak;
}

QString VersionManager::getDataRoot() {
    if (isFlatpak()) {
        return QDir::homePath()
            + "/.var/app/com.trench.trinity.launcher/data/mcpelauncher";
    }
#ifdef Q_OS_MAC
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
        + "/mcpelauncher";
#else
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
        + "/mcpelauncher";
#endif
}

QStringList VersionManager::getInstalledVersions() const {
    QString versionsDir = getDataRoot() + "/versions";
    QDir dir(versionsDir);
    if (dir.exists()) {
        return dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    }
    return QStringList();
}

QString VersionManager::getVersionPath(const QString &versionName) const {
    return getDataRoot() + "/versions/" + versionName;
}

// Check for the main runtime library (libminecraftpe.so).
// See: https://github.com/minecraft-linux/mcpelauncher-manifest (GPLv3)
bool VersionManager::isVersionValid(const QString &versionName) const {
    QString libPath = getVersionPath(versionName) + "/lib/x86_64/libminecraftpe.so";
    return QFileInfo::exists(libPath);
}

namespace {

quint16 readUInt16(const QByteArray &data, qint64 pos) {
    if (pos + 2 > data.size()) return 0;
    const auto *p = reinterpret_cast<const uchar *>(data.constData()) + pos;
    return quint16(p[0]) | (quint16(p[1]) << 8);
}

quint32 readUInt32(const QByteArray &data, qint64 pos) {
    if (pos + 4 > data.size()) return 0;
    const auto *p = reinterpret_cast<const uchar *>(data.constData()) + pos;
    return quint32(p[0]) | (quint32(p[1]) << 8) |
           (quint32(p[2]) << 16) | (quint32(p[3]) << 24);
}

quint64 readUInt64(const QByteArray &data, qint64 pos) {
    if (pos + 8 > data.size()) return 0;
    const auto *p = reinterpret_cast<const uchar *>(data.constData()) + pos;
    quint64 v = 0;
    for (int i = 7; i >= 0; --i)
        v = (v << 8) | p[i];
    return v;
}

} // namespace

QString VersionManager::getHostArchitecture() {
    const QString arch = QSysInfo::currentCpuArchitecture().toLower();
    if (arch == "i386" || arch == "i486" || arch == "i586" || arch == "i686" ||
        arch == "x86")
        return "x86";
    if (arch == "amd64" || arch == "x86_64")
        return "x86_64";
    if (arch == "aarch64" || arch == "arm64")
        return "arm64";
    if (arch == "arm" || arch == "armhf" || arch == "armv7l" ||
        arch.startsWith("armv7"))
        return "arm";
    if (arch == "riscv64")
        return "riscv64";
    return arch;
}

// Read the ZIP central directory of the APK and collect every native library
// ABI found under "lib/<abi>/...".
QStringList VersionManager::getApkArchitectures(const QString &apkPath) {
    QStringList archs;
    QFile file(apkPath);
    if (!file.open(QIODevice::ReadOnly))
        return archs;

    const qint64 fileSize = file.size();
    if (fileSize < 22)
        return archs;

    // The End Of Central Directory record (22 bytes) lives at the end of the
    // file, possibly followed by a comment (up to 65535 bytes).
    const qint64 tailSize = qMin<qint64>(fileSize, 22 + 65535);
    file.seek(fileSize - tailSize);
    const QByteArray tail = file.read(tailSize);

    // Locate EOCD signature "PK\x05\x06".
    qint64 eocd = -1;
    for (qint64 i = tail.size() - 22; i >= 0; --i) {
        const auto *p = reinterpret_cast<const uchar *>(tail.constData()) + i;
        if (p[0] == 0x50 && p[1] == 0x4b && p[2] == 0x05 && p[3] == 0x06) {
            eocd = i;
            break;
        }
    }
    if (eocd < 0)
        return archs;

    quint32 entryCount = readUInt16(tail, eocd + 10);
    quint32 cdSize     = readUInt32(tail, eocd + 12);
    quint64 cdOffset   = readUInt32(tail, eocd + 16);

    // Support ZIP64 archives (large APKs with many files).
    if (entryCount == 0xFFFF || cdSize == 0xFFFFFFFF || cdOffset == 0xFFFFFFFF) {
        const qint64 locatorPos = (fileSize - tailSize) + eocd - 20;
        if (locatorPos >= 0 && file.seek(locatorPos)) {
            const QByteArray loc = file.read(20);
            if (loc.size() == 20 && (uchar)loc[0] == 0x50 &&
                (uchar)loc[1] == 0x4b && (uchar)loc[2] == 0x07 &&
                (uchar)loc[3] == 0x06) {
                const quint64 zip64EocdOffset = readUInt64(loc, 8);
                if (file.seek(qint64(zip64EocdOffset))) {
                    const QByteArray e = file.read(56);
                    if (e.size() == 56 && (uchar)e[0] == 0x50 &&
                        (uchar)e[1] == 0x4b && (uchar)e[2] == 0x06 &&
                        (uchar)e[3] == 0x06) {
                        entryCount = quint32(readUInt64(e, 32));
                        cdSize     = quint32(readUInt64(e, 40));
                        cdOffset   = readUInt64(e, 48);
                    }
                }
            }
        }
    }

    if (cdOffset + quint64(cdSize) > quint64(fileSize))
        return archs;
    if (!file.seek(qint64(cdOffset)))
        return archs;
    const QByteArray cd = file.read(cdSize);

    qint64 pos = 0;
    const qint64 cdLen = cd.size();
    while (pos + 46 <= cdLen && entryCount > 0) {
        const auto *p = reinterpret_cast<const uchar *>(cd.constData()) + pos;
        if (p[0] != 0x50 || p[1] != 0x4b || p[2] != 0x01 || p[3] != 0x02)
            break; // Not a central directory file header
        const quint32 nameLen    = readUInt16(cd, pos + 28);
        const quint32 extraLen   = readUInt16(cd, pos + 30);
        const quint32 commentLen = readUInt16(cd, pos + 32);
        if (pos + 46 + nameLen > cdLen)
            break;

        const QByteArray name = cd.mid(pos + 46, nameLen);
        if (name.startsWith("lib/") && name.size() > 5) {
            const int slash = name.indexOf('/', 4);
            if (slash > 4) {
                const QString arch = QString::fromUtf8(name.mid(4, slash - 4));
                if (!arch.isEmpty() && !archs.contains(arch))
                    archs.append(arch);
            }
        }

        pos += 46 + nameLen + extraLen + commentLen;
        --entryCount;
    }

    return archs;
}

bool VersionManager::archCompatible(const QString &apkArch,
                                    const QString &hostArch) {
    if (hostArch == "x86_64")
        return apkArch == "x86_64" || apkArch == "x86";
    if (hostArch == "x86")
        return apkArch == "x86";
    if (hostArch == "arm64")
        return apkArch == "arm64-v8a" || apkArch == "armeabi-v7a" ||
               apkArch == "armeabi";
    if (hostArch == "arm")
        return apkArch == "armeabi-v7a" || apkArch == "armeabi";
    if (hostArch == "riscv64")
        return apkArch == "riscv64";
    return apkArch == hostArch;
}

bool VersionManager::isApkCompatible(const QString &apkPath,
                                     QString &errorMsg) const {
    // Los paquetes .tmc ya vienen extraídos, no aplica la comprobación.
    if (apkPath.endsWith(".tmc", Qt::CaseInsensitive))
        return true;

    const QStringList apkArchs = getApkArchitectures(apkPath);
    // Sin librerías nativas (APK 100% Java) o archivo ilegible: sin restricción.
    if (apkArchs.isEmpty())
        return true;

    const QString hostArch = getHostArchitecture();
    for (const QString &arch : apkArchs) {
        if (archCompatible(arch, hostArch))
            return true;
    }

    errorMsg =
        tr("This APK cannot be run on your device. Try another one compatible "
           "with your processor (architecture).");
    return false;
}

// Use mcpelauncher-extract to extract an APK into the version directory.
// See: https://github.com/minecraft-linux/mcpelauncher-extract (MIT) 
bool VersionManager::extractApk(const QString &apkPath, const QString &versionName, QString &errorMsg) {
    // Rechazar APKs con librerías nativas incompatibles con el procesador
    // antes de extraer (ej. APK ARM en un equipo x86_64 y viceversa).
    if (!isApkCompatible(apkPath, errorMsg))
        return false;

    QString destDir = getVersionPath(versionName);
    if (!QDir().mkpath(destDir)) {
        errorMsg = "No se pudo crear el directorio de destino.";
        return false;
    }

    // Crear proceso de extracción
    QProcess process;

    // Si es un .tmc, usar tar para extraer
    if (apkPath.endsWith(".tmc", Qt::CaseInsensitive)) {
        process.start("tar", {"-xvf", apkPath, "-C", destDir, "--strip-components=1"});
    } else {
        QString appDir = QCoreApplication::applicationDirPath();
        QString extractorPath = appDir + "/mcpelauncher-extract";
        if (!QFileInfo::exists(extractorPath)) {
            extractorPath = QStandardPaths::findExecutable("mcpelauncher-extract");
        }
        if (extractorPath.isEmpty()) {
            errorMsg = "mcpelauncher-extract no encontrado.";
            return false;
        }

        // Iniciar proceso
        process.start(extractorPath, {apkPath, destDir});
    }

    // Esperar a que termine (sin bloquear la UI)
    process.waitForStarted(-1);

    // Reportar estado inicial
    emit extractionProgress(tr("Starting extraction..."));

    // Esperar a que termine (sin bloquear la UI)
    while (process.state() == QProcess::Running) {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep(100); // Pausa breve para no saturar la CPU
    }

    process.waitForFinished(-1);

    if (process.exitCode() == 0) {
        emit extractionProgress(tr("Extraction completed."));
        return true;
    } else {
        QString err = process.readAllStandardError();
        if (err.isEmpty()) err = tr("Unknown error during extraction.");
        errorMsg = err;
        emit extractionProgress(tr("Error during extraction."));
        return false;
    }
}

bool VersionManager::deleteVersion(const QString &versionName, QString &errorMsg) {
    QString versionPath = getVersionPath(versionName);

    if (!QDir(versionPath).removeRecursively()) {
        errorMsg = tr("Could not delete version.");
        return false;
    }

    return true;
}

bool VersionManager::editVersion(const QString &versionName, const QString &newArgs, QString &errorMsg) {
    // Suponiendo que usas VersionConfig para guardar la configuración
    VersionConfig config(versionName);
    config.setLaunchArgs(newArgs);

    if (!config.save()) {
        errorMsg = tr("Could not save configuration.");
        return false;
    }

    return true;
}
