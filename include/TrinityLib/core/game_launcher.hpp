#ifndef GAME_LAUNCHER_H
#define GAME_LAUNCHER_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QTimer>

class GameLauncher : public QObject {
        Q_OBJECT

    public:
        explicit GameLauncher(QObject *parent = nullptr);
        ~GameLauncher();

        bool launchGame(const QString &versionName, QString &errorMsg);
        bool launchTrinito(QString &errorMsg);

    signals:
        void gameFinished(int exitCode, QProcess::ExitStatus exitStatus);
        void gameError(QProcess::ProcessError error);

    private slots:
        void onGameOutput();
        void forceKillGame();

    private:
        QProcess *m_process; // El proceso persistente
        QTimer *m_killTimer; // Temporizador de seguridad
        bool m_isClosing;    // Para saber si ya empezamos a cerrar
};

#endif // GAME_LAUNCHER_H
