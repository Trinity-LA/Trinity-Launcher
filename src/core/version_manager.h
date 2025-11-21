#ifndef VERSION_MANAGER_H
#define VERSION_MANAGER_H

#include <QString>
#include <QStringList>
#include <QObject>

class VersionManager : public QObject {
    Q_OBJECT

public:
    explicit VersionManager(QObject *parent = nullptr);

    QStringList getInstalledVersions() const;
    bool isVersionValid(const QString &versionName) const;
    QString getVersionPath(const QString &versionName) const;

    // Synchronous for now, or we could make it async with signals
    bool extractApk(const QString &apkPath, const QString &versionName, QString &errorMsg);

signals:
    void extractionFinished(bool success, QString message);
};

#endif // VERSION_MANAGER_H
