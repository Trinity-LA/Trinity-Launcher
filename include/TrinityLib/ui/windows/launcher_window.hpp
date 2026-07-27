#ifndef LAUNCHER_WINDOW_H
#define LAUNCHER_WINDOW_H

#include "TrinityLib/core/exporter.hpp"
#include "TrinityLib/core/game_launcher.hpp"
#include <QAction>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QTextEdit>

class TrinitoWindow;

class LauncherWindow : public QMainWindow {
        Q_OBJECT

    public:
        explicit LauncherWindow(QWidget *parent = nullptr);

        /**
         * Carga las versiones instaladas desde el directorio mcpelauncher
         */
        void loadInstalledVersions();

        // Accessible by TrinitoWindow to sync version selection
        void selectVersion(const QString &version);
        QComboBox *versionCombo = nullptr; // hidden selector kept for cross-window sync

    signals:
        void versionsChanged();

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
        // Central instance grid (QListView::IconMode)
        QListWidget *versionList;

        // Global actions (main toolbar / menus)
        QAction *actionExtract;
        QAction *actionImport;
        QAction *actionTrinito;
        QAction *actionSettings;
        QAction *actionLog;
        QAction *actionDiscord;
        QAction *actionAbout;

        // Instance actions (right toolbar, contextual)
        QAction *actionLaunch;
        QAction *actionConfig;
        QAction *actionExport;
        QAction *actionShortcut;
        QAction *actionDelete;

        // Status bar
        QLabel *statusLabel;
        QLabel *pathLabel;

        QComboBox *settingsLanguageCombo; // Language selector shown in Settings
        QTextEdit *logTextEdit;           // Log output display

        Exporter *exporter;
        GameLauncher *m_gameLauncher;

        // Secondary windows (created once, shown on demand)
        TrinitoWindow *m_trinito;
        QDialog *m_settingsDialog;
        QDialog *m_aboutDialog;
        QDialog *m_discordDialog;
        QDialog *m_logDialog;

        void setupUi();
        void setupConnections();
        void setupDialogs();
        void updateContextPanel(const QString &versionName);
        QWidget *createSettingsPage();
        QWidget *createLogPage();
        QWidget *createAboutPage();
        QWidget *createDiscordPage();
        void applyTheme(const QString &accent, const QString &bg,
                        const QString &panel,  const QString &hover,
                        const QString &btnHover  = "#525C66",
                        const QString &textMuted = "#B38D97",
                        const QString &text = "#EBCFB2");
        void generateThemeFromWallpaper(const QString &wallpaperPath);
};

#endif // LAUNCHER_WINDOW_H
