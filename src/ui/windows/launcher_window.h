#ifndef LAUNCHER_WINDOW_H
#define LAUNCHER_WINDOW_H

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include "../../core/version_config.h"

class LauncherWindow : public QWidget {
    Q_OBJECT

public:
    explicit LauncherWindow(QWidget *parent = nullptr);

    /**
     * Carga las versiones instaladas desde el directorio mcpelauncher
     */
    void loadInstalledVersions();

private slots:
    void onVersionSelected(QListWidgetItem *item);
    void showExtractDialog();
    void launchGame();
    void launchTools();

    // Nuevos slots para los botones
    void onEditConfigClicked();
    void onExportClicked();
    void onDeleteClicked();
    void onImportClicked();
private:
    // Layouts
    QHBoxLayout *mainLayout;
    QVBoxLayout *rightPanelLayout;

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
    QPushButton *toolsButton;

    // Status Bar
    QLabel *statusLabel;

    void setupUi();
    void setupConnections();
    void updateContextPanel(const QString &versionName);
    bool copyDirectory(const QString &srcPath, const QString &dstPath);
    bool extractZip(const QString &zipPath, const QString &destDir);

};

#endif // LAUNCHER_WINDOW_H