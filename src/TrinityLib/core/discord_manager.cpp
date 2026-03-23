#include "TrinityLib/core/discord_manager.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QUuid>

// Discord IPC opcodes
static constexpr uint32_t OPCODE_HANDSHAKE = 0;
static constexpr uint32_t OPCODE_FRAME = 1;

// ──────────────────────────────────────────────
// Singleton
// ──────────────────────────────────────────────

DiscordManager &DiscordManager::instance() {
  static DiscordManager inst;
  return inst;
}

DiscordManager::~DiscordManager() { disconnect(); }

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

  // Reconnect timer – retries every 5 s when disconnected
  m_reconnectTimer = new QTimer(this);
  m_reconnectTimer->setInterval(5000);
  connect(m_reconnectTimer, &QTimer::timeout, this, [this]() {
    if (!m_connected && m_enabled) {
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
    scheduleReconnect();
    return;
  }

  // ── Build the SET_ACTIVITY payload ──
  QJsonObject assets;
  assets["large_image"] = "trini";
  assets["large_text"] = "Trinity Launcher";

  if (!smallImageKey.isEmpty()) {
    assets["small_image"] = smallImageKey;
    assets["small_text"] = smallImageText;
  }

  QJsonObject activity;
  activity["details"] = details;
  activity["state"] = state;
  activity["assets"] = assets;

  if (useTimer) {
    m_startTimestamp = static_cast<int64_t>(std::time(nullptr));
    QJsonObject timestamps;
    timestamps["start"] = static_cast<qint64>(m_startTimestamp);
    activity["timestamps"] = timestamps;
  }

  // Nonce – unique per request (Discord expects this)
  QString nonce = QUuid::createUuid().toString(QUuid::WithoutBraces);

  QJsonObject args;
  args["pid"] = static_cast<int>(QCoreApplication::applicationPid());
  args["activity"] = activity;

  QJsonObject payload;
  payload["cmd"] = "SET_ACTIVITY";
  payload["args"] = args;
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
  if (!m_enabled)
    return;
  DiscordManager::instance().updateActivity(
      DiscordManager::tr("Waiting to start"), // Details
      DiscordManager::tr("In the main menu"), // State
      DiscordManager::tr("axe_icon"),         // Small Image Key
      DiscordManager::tr("Configuring"),      // Tooltip Small Image
      false                                   // No timer
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

bool DiscordManager::isEnabled() const { return m_enabled; }

// ──────────────────────────────────────────────
// Private helpers
// ──────────────────────────────────────────────

bool DiscordManager::connectToDiscord() {
  // Clean up any existing socket first
  if (m_socket) {
    m_socket->abort();
    m_socket->deleteLater();
    m_socket = nullptr;
  }
  
  m_socket = new QLocalSocket(this);

  // Only connect signals if not already connected
  static bool signalsConnected = false;
  if (!signalsConnected) {
    connect(m_socket, &QLocalSocket::readyRead, this,
            &DiscordManager::drainSocket, Qt::UniqueConnection);
    connect(m_socket, &QLocalSocket::disconnected, this,
            &DiscordManager::scheduleReconnect, Qt::UniqueConnection);
    signalsConnected = true;
  }

  QString runtimeDir = qEnvironmentVariable("XDG_RUNTIME_DIR", "/tmp");

  for (int i = 0; i < 10; ++i) {
    QString path = QString("%1/discord-ipc-%2").arg(runtimeDir).arg(i);
    
    // Check if socket file exists before trying to connect (Flatpak fix)
    QFileInfo socketInfo(path);
    if (!socketInfo.exists()) {
      continue;
    }
    
    m_socket->connectToServer(path);
    if (m_socket->waitForConnected(100)) {
      m_connected = true;
      return sendHandshake();
    }
  }
  
  // Clean up socket if connection failed
  if (m_socket) {
    m_socket->abort();
    m_socket->deleteLater();
    m_socket = nullptr;
  }
  return false;
}

bool DiscordManager::sendHandshake() {
  QJsonObject obj;
  obj["v"] = 1;
  obj["client_id"] = QString::number(m_clientId);

  QByteArray json = QJsonDocument(obj).toJson(QJsonDocument::Compact);
  return sendFrame(OPCODE_HANDSHAKE, json);
}

bool DiscordManager::sendFrame(uint32_t opcode, const QByteArray &json) {
  if (!m_socket || !m_socket->isOpen())
    return false;

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

  m_socket->write(frame);
  m_socket->flush();

  return true;
}

void DiscordManager::drainSocket() {
  if (m_socket && m_socket->isOpen()) {
    m_socket->readAll();
  }
}

void DiscordManager::clearActivity() {
  if (!m_connected)
    return;

  QString nonce = QUuid::createUuid().toString(QUuid::WithoutBraces);

  QJsonObject args;
  args["pid"] = static_cast<int>(QCoreApplication::applicationPid());
  // 'activity' key intentionally omitted to clear it

  QJsonObject payload;
  payload["cmd"] = "SET_ACTIVITY";
  payload["args"] = args;
  payload["nonce"] = nonce;

  QByteArray json = QJsonDocument(payload).toJson(QJsonDocument::Compact);
  sendFrame(OPCODE_FRAME, json);
  qDebug() << "[Discord] Activity cleared.";
}

void DiscordManager::disconnect() {
  if (m_socket) {
    m_socket->abort();
    m_socket->deleteLater();
    m_socket = nullptr;
  }
  m_connected = false;
}

void DiscordManager::scheduleReconnect() {
  if (!m_enabled)
    return;
    
  if (m_reconnectTimer && !m_reconnectTimer->isActive()) {
    qDebug() << "[Discord] Scheduling reconnect in 5 s.";
    m_reconnectTimer->start();
  }
}
