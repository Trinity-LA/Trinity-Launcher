#ifndef GAME_LAUNCHER_H
#define GAME_LAUNCHER_H

#include <QString>
#include <QObject>

class GameLauncher : public QObject {
    Q_OBJECT

public:
    explicit GameLauncher(QObject *parent = nullptr);

    bool launchGame(const QString &versionName, QString &errorMsg);
    bool launchTrinito(QString &errorMsg);
};

#endif // GAME_LAUNCHER_H
