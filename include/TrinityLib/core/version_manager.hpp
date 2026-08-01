#ifndef VERSION_MANAGER_H
#define VERSION_MANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>

class VersionManager : public QObject {
    Q_OBJECT

public:
    explicit VersionManager(QObject *parent = nullptr);

    static QString getDataRoot();
    static bool isFlatpak();

    QStringList getInstalledVersions() const;
    QString getVersionPath(const QString &versionName) const;
    bool isVersionValid(const QString &versionName) const;

    // Comprobar si un APK es compatible con la arquitectura del procesador.
    // Devuelve false y llena errorMsg con un mensaje legible si no lo es.
    bool isApkCompatible(const QString &apkPath, QString &errorMsg) const;

    // Arquitectura actual del procesador (normalizada: "x86_64", "x86",
    // "arm64", "arm", "riscv64", ...).
    static QString getHostArchitecture();

    // Extraer APK (mover desde launcher_window)
    bool extractApk(const QString &apkPath, const QString &versionName, QString &errorMsg);

    // Eliminar versión (mover desde launcher_window)
    bool deleteVersion(const QString &versionName, QString &errorMsg);

    // Editar configuración de versión (mover desde launcher_window)
    bool editVersion(const QString &versionName, const QString &newArgs, QString &errorMsg);

signals:
    // Nuevo signal para reportar progreso (si es posible detectarlo)
    void extractionProgress(const QString &statusMessage);

private:
    // Lista las arquitecturas (ABIs) de las librerías nativas "lib/<arch>/..."
    // contenidas en el APK leyendo el directorio central del ZIP.
    static QStringList getApkArchitectures(const QString &apkPath);

    // Regla de compatibilidad entre una ABI del APK y la arquitectura del host.
    static bool archCompatible(const QString &apkArch, const QString &hostArch);

    // Función auxiliar para copiar directorios
    bool copyDirectory(const QString &srcPath, const QString &dstPath);
};

#endif // VERSION_MANAGER_H
