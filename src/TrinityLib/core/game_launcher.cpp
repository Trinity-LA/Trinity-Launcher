#include "TrinityLib/core/game_launcher.hpp"
#include "TrinityLib/core/discord_manager.hpp"
#include "TrinityLib/core/version_config.hpp"
#include "TrinityLib/core/version_manager.hpp"
#include <QCoreApplication>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <iostream>

GameLauncher::GameLauncher(QObject *parent)
    : QObject(parent),
      m_isClosing(false) {

    m_process = new QProcess(this);
    m_killTimer = new QTimer(this);

    // Configurar el Terminator (Timer)
    m_killTimer->setSingleShot(true);
    m_killTimer->setInterval(3000); // 3 segundos de tolerancia
    connect(m_killTimer, &QTimer::timeout, this, &GameLauncher::forceKillGame);

    m_process->setProcessChannelMode(QProcess::MergedChannels);

    // Conectamos la lectura de logs
    connect(m_process, &QProcess::readyReadStandardOutput, this,
            &GameLauncher::onGameOutput);

    // Conectamos el fin del juego
    connect(m_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this](int code, QProcess::ExitStatus status) {
                // Si el juego termina bien, detenemos el timer de asesinato
                if (m_killTimer->isActive())
                    m_killTimer->stop();

                m_isClosing = false;

                DiscordManager::instance().updateActivityMain();
                emit gameFinished(code, status);
            });
}

GameLauncher::~GameLauncher() {
    if (m_process->state() == QProcess::Running) {
        m_process->kill(); // Destrucción total al cerrar el launcher
    }
}

void GameLauncher::onGameOutput() {
    // Read all the terminal logs
    QByteArray data = m_process->readAllStandardOutput();
    QString output = QString::fromLocal8Bit(data);

    std::cout << data.toStdString();

    // Messages logs of bedrock close
    if (!m_isClosing && (output.contains("Invoking stop activity callbacks") ||
                         output.contains("The graphics context was lost") ||
                         output.contains("Quit request received") ||
                         output.contains("Android window closed"))) {

        m_isClosing = true;
        DiscordManager::instance().updateActivityMain();

        m_process->terminate();
        m_killTimer->setInterval(2000);
        m_killTimer->start();
    }
}

void GameLauncher::forceKillGame() {
    if (m_process->state() != QProcess::NotRunning)
        m_process->kill(); // SIGKILL
}

bool GameLauncher::launchGame(const QString &versionName, QString &errorMsg) {
    if (m_process->state() != QProcess::NotRunning) {
        errorMsg = tr("El juego ya se está ejecutando.");
        return false;
    }

    m_isClosing = false; // Resetear bandera

    VersionManager vm;
    QString dataDir = vm.getVersionPath(versionName);
    QString clientPath = QStandardPaths::findExecutable("mcpelauncher-client");

    if (clientPath.isEmpty()) {
        errorMsg = tr("mcpelauncher-client no encontrado.");
        return false;
    }

    VersionConfig config(versionName);
    QString extraEnvStr = config.getLaunchArgs();
    QStringList args;
    args << "-dg" << dataDir;

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (!extraEnvStr.isEmpty()) {
        QStringList envVars = extraEnvStr.split(' ', Qt::SkipEmptyParts);
        for (const QString &var : envVars) {
            int equalPos = var.indexOf('=');
            if (equalPos != -1) {
                env.insert(var.left(equalPos), var.mid(equalPos + 1));
            }
        }
    }
    m_process->setProcessEnvironment(env);

    QString displayServer =
        qgetenv("XDG_SESSION_TYPE") == "wayland" ? "Wayland" : "X11";
    QString state = "Ver: " + versionName + " (" + displayServer + ")";

    DiscordManager::instance().updateActivity(
        tr("Jugando Minecraft Bedrock"), // Details
        state,                           // State
        "mc_icon",                       // Small Image
        "Linux",                         // Tooltip
        true                             // Timer: SÍ
    );

    m_process->setProgram(clientPath);
    m_process->setArguments(args);
    m_process->start();

    if (!m_process->waitForStarted(3000)) {
        errorMsg = tr("No se pudo iniciar el proceso del juego.");
        DiscordManager::instance().updateActivity(
            tr("Menú de Trinity Launcher"), tr("Esperando..."));
        return false;
    }

    return true;
}

// Implementaremos un método para forzar el cierre si es necesario
// (Opcional: puedes conectar la señal stateChanged para monitorear)


