#ifndef LAUNCHER_WINDOW_H
#define LAUNCHER_WINDOW_H

#include "../widgets/version_selector.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>

class LauncherWindow : public QWidget {
    Q_OBJECT

public:
    explicit LauncherWindow(QWidget *parent = nullptr);

private slots:
    void loadInstalledVersions();
    void onVersionSelected(int index);
    void showExtractDialog();
    void launchGame();
    void launchTools();

private:
    QVBoxLayout *mainLayout;
    VersionSelector *versionSelector;
    QPushButton *extractButton;
    QPushButton *playButton;
    QPushButton *toolsButton;
};

#endif // LAUNCHER_WINDOW_H

