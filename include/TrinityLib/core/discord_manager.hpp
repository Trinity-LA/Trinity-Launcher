#pragma once

#include <QObject>
#include <QSettings>
#include <QTimer>
#include <cstdint>

/**
 * DiscordManager
 *
 * Implements Discord Rich Presence via the native Unix IPC socket protocol.
 * No external SDK dependency — communicates directly with the Discord desktop
 * client through /tmp/discord-ipc-N sockets.
 */
class DiscordManager : public QObject {
    Q_OBJECT
public:
    static DiscordManager &instance();

    void init(int64_t clientId);

    void updateActivity(const QString &details, const QString &state,
                        const QString &smallImageKey = "",
                        const QString &smallImageText = "",
                        bool useTimer = false);

    void updateActivityMain();

    /** Called periodically to drain the receive buffer (keeps socket healthy). */
    void runCallbacks();

    void setEnabled(bool enabled);
    bool isEnabled() const;

private:
    DiscordManager() = default;
    ~DiscordManager();

    /** Try to open /tmp/discord-ipc-0 .. /tmp/discord-ipc-9 */
    bool connectToDiscord();

    /** Send the IPC handshake (opcode 0). */
    bool sendHandshake();

    /**
     * Send a framed IPC message.
     * Frame layout: [opcode u32le][length u32le][json bytes]
     */
    bool sendFrame(uint32_t opcode, const QByteArray &json);

    /** Read & discard any pending data so the socket buffer stays clear. */
    void drainSocket();

    /** Clear the Rich Presence activity (hides it in Discord). */
    void clearActivity();

    void disconnect();
    void scheduleReconnect();

    int64_t  m_clientId{0};
    int      m_socketFd{-1};
    bool     m_connected{false};
    bool     m_enabled{true};
    int64_t  m_startTimestamp{0};

    QTimer  *m_callbackTimer{nullptr};   ///< Drains socket @ 16 ms
    QTimer  *m_reconnectTimer{nullptr};  ///< Retries connection every 5 s
};
