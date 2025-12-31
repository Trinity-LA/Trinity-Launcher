#include "TrinityLib/core/version_config.hpp"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QStandardPaths>

VersionConfig::VersionConfig(const QString &versionName, QObject *parent)
    : QObject(parent), versionName(versionName) {
    load();
}

VersionConfig::~VersionConfig() {}

QString VersionConfig::getLaunchArgs() const {
    return launchArgs;
}

void VersionConfig::setLaunchArgs(const QString &args) {
    launchArgs = args;
}

bool VersionConfig::save() {
    QDir().mkpath(QFileInfo(configPath()).dir().path());
    QFile file(configPath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream out(&file);
    out << launchArgs;
    file.close();
    return true;
}

bool VersionConfig::load() {
    QFile file(configPath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream in(&file);
    launchArgs = in.readAll().trimmed();
    file.close();
    return true;
}

QString VersionConfig::configPath() const {
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
           + "/mcpelauncher/versions/" + versionName + "/trinity-config.txt";
}
