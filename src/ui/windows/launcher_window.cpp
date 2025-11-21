#include "launcher_window.h"
#include "../../core/version_manager.h"
#include "../../core/game_launcher.h"
#include "../widgets/version_selector.h"
#include "../dialogs/extract_dialog.h"

#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QFileInfo>

LauncherWindow::LauncherWindow(QWidget *parent) : QWidget(parent) {
    setWindowTitle("Trinity Launcher - Multiversions");
    resize(480, 220);
    setFixedSize(size());

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(12);

    QLabel *titleLabel = new QLabel("Selecciona una versión instalada:");
    titleLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; }");
    mainLayout->addWidget(titleLabel);

    versionSelector = new VersionSelector();
    versionSelector->setMinimumHeight(30);
    mainLayout->addWidget(versionSelector);

    extractButton = new QPushButton("Extraer nueva versión desde APK");
    extractButton->setMinimumHeight(35);
    connect(extractButton, &QPushButton::clicked, this, &LauncherWindow::showExtractDialog);
    mainLayout->addWidget(extractButton);

    playButton = new QPushButton("Jugar");
    playButton->setMinimumHeight(35);
    connect(playButton, &QPushButton::clicked, this, &LauncherWindow::launchGame);
    mainLayout->addWidget(playButton);

    toolsButton = new QPushButton("Tools");
    toolsButton->setMinimumHeight(35);
    connect(toolsButton, &QPushButton::clicked, this, &LauncherWindow::launchTools);
    mainLayout->addWidget(toolsButton);

    connect(versionSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LauncherWindow::onVersionSelected);

    // Initial state check
    onVersionSelected(versionSelector->currentIndex());
}

void LauncherWindow::loadInstalledVersions() {
    versionSelector->refreshVersions();
    onVersionSelected(versionSelector->currentIndex());
}

void LauncherWindow::onVersionSelected(int index) {
    QString version = versionSelector->getSelectedVersion();
    playButton->setEnabled(!version.isEmpty());
}

void LauncherWindow::showExtractDialog() {
    ExtractDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) return;

    QString apkPath = dialog.getApkPath();
    QString versionName = dialog.getVersionName();

    VersionManager vm;
    QString errorMsg;
    if (vm.extractApk(apkPath, versionName, errorMsg)) {
        QMessageBox::information(this, "Éxito", "¡Versión extraída correctamente!");
        loadInstalledVersions();
    } else {
        QMessageBox::critical(this, "Error", "Falló la extracción:\n" + errorMsg);
    }
}

void LauncherWindow::launchGame() {
    QString selectedVersion = versionSelector->getSelectedVersion();
    if (selectedVersion.isEmpty()) return;

    GameLauncher launcher;
    QString errorMsg;
    if (!launcher.launchGame(selectedVersion, errorMsg)) {
        QMessageBox::critical(this, "Error", errorMsg);
        return;
    }
    QApplication::quit();
}

void LauncherWindow::launchTools() {
    GameLauncher launcher;
    QString errorMsg;
    if (!launcher.launchTrinito(errorMsg)) {
        QMessageBox::critical(this, "Error", errorMsg);
    }
}

