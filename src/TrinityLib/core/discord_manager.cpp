#include "TrinityLib/core/discord_manager.hpp"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QUuid>

// POSIX socket headers
#include <cerrno>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

// Discord IPC opcodes
static constexpr uint32_t OPCODE_HANDSHAKE = 0;
static constexpr uint32_t OPCODE_FRAME     = 1;

// ──────────────────────────────────────────────
// Singleton
// ──────────────────────────────────────────────

DiscordManager &DiscordManager::instance() {
    static DiscordManager inst;
    return inst;
}

DiscordManager::~DiscordManager() {
    disconnect();
}

// ──────────────────────────────────────────────
// Public API
// ──────────────────────────────────────────────

void DiscordManager::init(int64_t clientId) {
    m_clientId = clientId;

    QSettings settings;
    m_enabled = settings.value("discord_rpc_enabled", true).toBool();

    if (!m_enabled) {
        qDebug() << "[Discord] RPC disabled by user preference, skipping init.";
        return;
    }

    if (connectToDiscord()) {
        qDebug() << "[Discord] Connected to Discord IPC socket.";
    } else {
        qDebug() << "[Discord] Discord not running, will retry automatically.";
    }

    // Drain timer – keeps the receive buffer clear
    m_callbackTimer = new QTimer(this);
    m_callbackTimer->setTimerType(Qt::PreciseTimer);
    connect(m_callbackTimer, &QTimer::timeout,
            this, &DiscordManager::runCallbacks);
    m_callbackTimer->start(16);

    // Reconnect timer – retries every 5 s when disconnected
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(5000);
    connect(m_reconnectTimer, &QTimer::timeout, this, [this]() {
        if (!m_connected && m_enabled) {
            qDebug() << "[Discord] Attempting reconnect...";
            if (connectToDiscord()) {
                m_reconnectTimer->stop();
                qDebug() << "[Discord] Reconnected successfully.";
                updateActivityMain();
            }
        }
    });
}

void DiscordManager::runCallbacks() {
    if (m_connected)
        drainSocket();
}

void DiscordManager::updateActivity(const QString &details,
                                    const QString &state,
                                    const QString &smallImageKey,
                                    const QString &smallImageText,
                                    bool useTimer) {
    if (!m_enabled) {
        qDebug() << "[Discord] updateActivity skipped - disabled.";
        return;
    }

    if (!m_connected) {
        qDebug() << "[Discord] updateActivity skipped - not connected.";
        scheduleReconnect();
        return;
    }

    // ── Build the SET_ACTIVITY payload ──
    QJsonObject assets;
    assets["large_image"] = "trini";
    assets["large_text"]  = "Trinity Launcher";

    if (!smallImageKey.isEmpty()) {
        assets["small_image"] = smallImageKey;
        assets["small_text"]  = smallImageText;
    }

    QJsonObject activity;
    activity["details"] = details;
    activity["state"]   = state;
    activity["assets"]  = assets;

    if (useTimer) {
        m_startTimestamp = static_cast<int64_t>(std::time(nullptr));
        QJsonObject timestamps;
        timestamps["start"] = static_cast<qint64>(m_startTimestamp);
        activity["timestamps"] = timestamps;
    }

    // Nonce – unique per request (Discord expects this)
    QString nonce = QUuid::createUuid().toString(QUuid::WithoutBraces);

    QJsonObject args;
    args["pid"]      = static_cast<int>(::getpid());
    args["activity"] = activity;

    QJsonObject payload;
    payload["cmd"]   = "SET_ACTIVITY";
    payload["args"]  = args;
    payload["nonce"] = nonce;

    QByteArray json = QJsonDocument(payload).toJson(QJsonDocument::Compact);

    qDebug() << "[Discord] Sending activity:" << details << "|" << state;

    if (!sendFrame(OPCODE_FRAME, json)) {
        qDebug() << "[Discord] sendFrame failed, marking disconnected.";
        disconnect();
        scheduleReconnect();
    }
}

void DiscordManager::updateActivityMain() {
    if (!m_enabled) return;
    DiscordManager::instance().updateActivity(
        DiscordManager::tr("Waiting to start"),   // Details
        DiscordManager::tr("In the main menu"),   // State
        DiscordManager::tr("axe_icon"),           // Small Image Key
        DiscordManager::tr("Configuring"),        // Tooltip Small Image
        false                                     // No timer
    );
}

void DiscordManager::setEnabled(bool enabled) {
    m_enabled = enabled;
    QSettings settings;
    settings.setValue("discord_rpc_enabled", enabled);

    if (!enabled) {
        clearActivity();
    } else {
        // If not connected yet, try now
        if (!m_connected) {
            if (connectToDiscord()) {
                updateActivityMain();
            } else {
                scheduleReconnect();
            }
        } else {
            updateActivityMain();
        }
    }
}

bool DiscordManager::isEnabled() const {
    return m_enabled;
}

// ──────────────────────────────────────────────
// Private helpers
// ──────────────────────────────────────────────

bool DiscordManager::connectToDiscord() {
    disconnect(); // Close any existing socket first

    // Determine the socket directory. Discord may use XDG_RUNTIME_DIR or /tmp.
    QByteArray runtimeDir = qgetenv("XDG_RUNTIME_DIR");
    if (runtimeDir.isEmpty())
        runtimeDir = "/tmp";

    // Try discord-ipc-0 through discord-ipc-9
    for (int i = 0; i < 10; ++i) {
        QString path = QString("%1/discord-ipc-%2")
                           .arg(QString::fromLocal8Bit(runtimeDir))
                           .arg(i);

        int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd < 0) {
            qDebug() << "[Discord] socket() failed:" << strerror(errno);
            return false;
        }

        // Set non-blocking so we never hang
        int flags = ::fcntl(fd, F_GETFL, 0);
        ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);

        struct sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        ::strncpy(addr.sun_path, path.toLocal8Bit().constData(),
                  sizeof(addr.sun_path) - 1);

        // Temporarily set blocking for connect
        ::fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
        int ret = ::connect(fd, reinterpret_cast<struct sockaddr *>(&addr),
                            sizeof(addr));
        ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);

        if (ret == 0) {
            m_socketFd  = fd;
            m_connected = true;
            return sendHandshake();
        }

        ::close(fd);
    }

    return false;
}

bool DiscordManager::sendHandshake() {
    QJsonObject obj;
    obj["v"]         = 1;
    obj["client_id"] = QString::number(m_clientId);

    QByteArray json = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    return sendFrame(OPCODE_HANDSHAKE, json);
}

bool DiscordManager::sendFrame(uint32_t opcode, const QByteArray &json) {
    if (m_socketFd < 0) return false;

    uint32_t length = static_cast<uint32_t>(json.size());

    // Build header (little-endian on any typical x86/ARM Linux)
    uint8_t header[8];
    header[0] = opcode & 0xFF;
    header[1] = (opcode >> 8) & 0xFF;
    header[2] = (opcode >> 16) & 0xFF;
    header[3] = (opcode >> 24) & 0xFF;
    header[4] = length & 0xFF;
    header[5] = (length >> 8) & 0xFF;
    header[6] = (length >> 16) & 0xFF;
    header[7] = (length >> 24) & 0xFF;

    QByteArray frame(reinterpret_cast<const char *>(header), 8);
    frame.append(json);

    ssize_t total = 0;
    ssize_t toSend = static_cast<ssize_t>(frame.size());

    while (total < toSend) {
        ssize_t sent = ::write(m_socketFd, frame.constData() + total,
                               static_cast<size_t>(toSend - total));
        if (sent < 0) {
            if (errno == EINTR) continue;
            qDebug() << "[Discord] write() error:" << strerror(errno);
            return false;
        }
        total += sent;
    }

    return true;
}

void DiscordManager::drainSocket() {
    if (m_socketFd < 0) return;

    char buf[4096];
    ssize_t n;
    while ((n = ::read(m_socketFd, buf, sizeof(buf))) > 0) {
        // We don't need to process responses for basic Rich Presence.
        // Just drain so the kernel buffer never fills.
    }

    if (n == 0) {
        // Socket closed by peer (Discord quit)
        qDebug() << "[Discord] Socket closed by Discord.";
        disconnect();
        scheduleReconnect();
    }
    // n == -1 with EAGAIN/EWOULDBLOCK is normal (non-blocking, no data)
}

void DiscordManager::clearActivity() {
    if (!m_connected) return;

    QString nonce = QUuid::createUuid().toString(QUuid::WithoutBraces);

    QJsonObject args;
    args["pid"] = static_cast<int>(::getpid());
    // 'activity' key intentionally omitted to clear it

    QJsonObject payload;
    payload["cmd"]   = "SET_ACTIVITY";
    payload["args"]  = args;
    payload["nonce"] = nonce;

    QByteArray json = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    sendFrame(OPCODE_FRAME, json);
    qDebug() << "[Discord] Activity cleared.";
}

void DiscordManager::disconnect() {
    if (m_socketFd >= 0) {
        ::close(m_socketFd);
        m_socketFd = -1;
    }
    m_connected = false;
}

void DiscordManager::scheduleReconnect() {
    if (m_reconnectTimer && !m_reconnectTimer->isActive()) {
        qDebug() << "[Discord] Scheduling reconnect in 5 s.";
        m_reconnectTimer->start();
    }
}
