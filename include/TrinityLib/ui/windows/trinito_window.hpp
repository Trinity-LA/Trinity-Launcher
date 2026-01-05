#ifndef TRINITOWINDOW_H
#define TRINITOWINDOW_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>

class TrinitoWindow : public QWidget {
    Q_OBJECT

public:
    explicit TrinitoWindow(QWidget *parent = nullptr);

private:
    QWidget *createPackTab(const QString &targetSubdir, const QString &labelText);
    QWidget *createDevTab();
    QWidget *createWorldTab();
    QWidget *createShadersModsTab();
    void installItem(const QString &sourcePath, const QString &targetSubdir);

    QWidget *createManageTab(const QString &packType, const QString &displayName);
    void loadPacks(const QString &packType, QListWidget *listWidget);
    void togglePack(const QString &packType, const QString &packName, bool enable);

    QString getShadersDir();

    QListWidget *modsList = nullptr;
    QListWidget *addonsList = nullptr;
    QListWidget *resourcesList = nullptr;
    QListWidget *mapsList = nullptr;

    QListWidget *shadersList = nullptr;
    QPushButton *installShaderpackButton = nullptr;
    QPushButton *removeShaderpackButton = nullptr;
    QPushButton *refreshShaderListButton = nullptr;

    QListWidget *availableModsList = nullptr;
    QPushButton *downloadModButton = nullptr;
    QListWidget *installedModsList = nullptr;
    QPushButton *removeInstalledModButton = nullptr;

private slots:
    void onInstallShaderpackClicked();
    void onRemoveShaderpackClicked();
    void onRefreshShaderListClicked();
    void onDownloadModClicked();
    void onRemoveInstalledModClicked();

    void populateInstalledShaders();
    void populateAvailableMods();
    void populateInstalledMods();
};

#endif // TRINITOWINDOW_H
