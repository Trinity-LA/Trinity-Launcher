#include "discord_presence/core.h"
#include <QObject>
#include <QSettings>
#include <QTimer>
#include <cstdint>

class DiscordManager : public QObject {
        Q_OBJECT
    public:
        static DiscordManager &instance();
        void init(std::int64_t clientId);
        void updateActivity(const QString &details, const QString &state,
                            const QString &smallImageKey = "",
                            const QString &smallImageText = "",
                            bool useTimer = false);
        void updateActivityMain();
        void runCallbacks();

        void setEnabled(bool enabled);
        bool isEnabled() const;

    private:
        DiscordManager() = default;
        discord::Core *m_core{nullptr};
        QTimer *m_updateTimer{nullptr};
        bool m_enabled{true};
};
