#ifndef VERSION_MANAGER_H
#define VERSION_MANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>

class VersionManager : public QObject {
    Q_OBJECT

public:
    explicit VersionManager(QObject *parent = nullptr);

    QStringList getInstalledVersions() const;
    QString getVersionPath(const QString &versionName) const;
    bool isVersionValid(const QString &versionName) const;

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
    // Función auxiliar para copiar directorios
    bool copyDirectory(const QString &srcPath, const QString &dstPath);
};

#endif // VERSION_MANAGER_H
