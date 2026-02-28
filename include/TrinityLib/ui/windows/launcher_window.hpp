#ifndef LAUNCHER_WINDOW_H
#define LAUNCHER_WINDOW_H

#include "TrinityLib/core/exporter.hpp"
#include "TrinityLib/core/game_launcher.hpp"
#include <QColorDialog>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>
class LauncherWindow : public QWidget {
        Q_OBJECT

    public:
        explicit LauncherWindow(QWidget *parent = nullptr);

        /**
         * Carga las versiones instaladas desde el directorio mcpelauncher
         */
        void loadInstalledVersions();

        // Accessible by TrinitoWindow to sync version selection
        void selectVersion(const QString &version);
        QComboBox *versionCombo = nullptr; // dock version selector (public for cross-widget sync)

    public slots:
        // Instance-action slots (also called from TrinitoWindow::Instancias tab)
        void onEditConfigClicked();
        void onExportClicked();
        void onDeleteClicked();
        void onImportClicked();
        void createDesktopShortcut();
        void onLanguageChanged(int index);

    private slots:
        void onVersionSelected(QListWidgetItem *item);
        void onVersionComboChanged(int index);
        void showExtractDialog();
        void launchGame();

    private:
        // Layouts
        QHBoxLayout *mainLayout;
        QVBoxLayout *rightPanelLayout;
        void deletePack(const QString &packType, const QString &packName);
        // Left Side - Version List
        QListWidget *versionList;

        // Right Side - Context Panel
        QWidget *contextPanel;
        QLabel *versionIconLabel;
        QLabel *versionNameLabel;
        QLabel *versionTypeLabel;
        QPushButton *playButton;
        QPushButton *editButton;   // Placeholder for now
        QPushButton *exportButton; // Placeholder
        QPushButton *deleteButton; // Placeholder
        QPushButton *importButton; //
        // Top Bar
        QPushButton *extractButton;
        QStackedWidget *contentStack;
        QPushButton *sidebarTrinityBtn;
        QPushButton *sidebarContentBtn;
        QPushButton *sidebarDiscordBtn;
        QPushButton *sidebarAboutBtn;
        QPushButton *sidebarSettingsBtn;
        Exporter *exporter;
        // Status Bar
        QLabel *statusLabel;
        QPushButton *shortcutButton;
        QComboBox *settingsLanguageCombo; // Language selector shown in Settings

        GameLauncher *m_gameLauncher;

        void setupUi();
        void setupConnections();
        void updateContextPanel(const QString &versionName);
        bool copyDirectory(const QString &srcPath, const QString &dstPath);
        bool extractZip(const QString &zipPath, const QString &destDir);
        QWidget *createSettingsPage();
        void applyTheme(const QString &accent, const QString &bg,
                        const QString &panel,  const QString &hover,
                        const QString &btnHover  = "#334155",
                        const QString &textMuted = "#94a3b8");
};

#endif // LAUNCHER_WINDOW_H
