#include "TrinityLib/core/discord_manager.hpp"
#include "discord_presence/ffi.h"
#include "discord_presence/types.h"
#include <QDebug>

DiscordManager &DiscordManager::instance() {
    static DiscordManager inst;
    return inst;
}

void DiscordManager::init(std::int64_t clientId) {
    discord::Result result{discord::Core::Create(
        clientId, DiscordCreateFlags_NoRequireDiscord, &m_core)};

    qDebug() << "[Discord] Core::Create result:" << static_cast<int>(result);

    if (result == discord::Result::Ok && m_core) {
        qDebug() << "[Discord] SDK initialized successfully";
        m_updateTimer = new QTimer(this);
        m_updateTimer->setTimerType(Qt::PreciseTimer);
        connect(m_updateTimer, &QTimer::timeout, this,
                &DiscordManager::runCallbacks);
        m_updateTimer->start(16);
    } else {
        qDebug() << "[Discord] SDK initialization FAILED";
        m_core = nullptr;
    }
}

void DiscordManager::runCallbacks() {
    if (m_core)
        m_core->RunCallbacks();
}

void DiscordManager::updateActivity(const QString &details,
                                    const QString &state,
                                    const QString &smallImageKey,
                                    const QString &smallImageText,
                                    bool useTimer) {
    if (!m_core) {
        qDebug() << "[Discord] updateActivity skipped - m_core is null";
        return;
    }

    discord::Activity activity{};

    activity.SetDetails(details.toUtf8().constData());
    activity.SetState(state.toUtf8().constData());

    activity.GetAssets().SetLargeImage("trini");
    activity.GetAssets().SetLargeText("Trinity Launcher");

    if (!smallImageKey.isEmpty()) {
        activity.GetAssets().SetSmallImage(smallImageKey.toUtf8().constData());
        activity.GetAssets().SetSmallText(smallImageText.toUtf8().constData());
    }

    if (useTimer) {
        activity.GetTimestamps().SetStart(std::time(nullptr));
    }

    qDebug() << "[Discord] Sending activity:" << details << "|" << state;
    m_core->ActivityManager().UpdateActivity(activity,
                                             [](discord::Result res) {
        qDebug() << "[Discord] UpdateActivity result:" << static_cast<int>(res);
    });
}

void DiscordManager::updateActivityMain() {
    DiscordManager::instance().updateActivity(
        DiscordManager::tr("Esperando para iniciar"), // Details
        DiscordManager::tr("En el menú principal"),   // State
        DiscordManager::tr("axe_icon"),               // Small Image Key
        DiscordManager::tr("Configurando"),           // Tooltip Small Image
        false                                         // Sin cronómetro
    );
}
