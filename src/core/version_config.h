#ifndef VERSION_CONFIG_H
#define VERSION_CONFIG_H

#include <QObject>
#include <QString>
#include <QMap>

class VersionConfig : public QObject {
    Q_OBJECT

public:
    explicit VersionConfig(const QString &versionName, QObject *parent = nullptr);
    ~VersionConfig();

    // Get/Set additional launch arguments
    QString getLaunchArgs() const;
    void setLaunchArgs(const QString &args);

    // Save and load from file
    bool save();
    bool load();

private:
    QString versionName;
    QString launchArgs;
    QString configPath() const;
};

#endif // VERSION_CONFIG_H