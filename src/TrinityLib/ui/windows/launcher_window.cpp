#include "TrinityLib/ui/windows/launcher_window.hpp"
#include "TrinityLib/ui/windows/trinito_window.hpp"
#include "TrinityLib/core/color_extractor.hpp"
#include "TrinityLib/core/discord_manager.hpp"
#include "TrinityLib/core/version_config.hpp"
#include "TrinityLib/core/version_manager.hpp"
#include "TrinityLib/ui/dialogs/extract_dialog.hpp"
#include "TrinityLib/ui/widgets/instance_delegate.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QColorDialog>
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QLocale>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QProcess>
#include <QProgressBar>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QSettings>
#include <QStandardPaths>
#include <QStatusBar>
#include <QStyle>
#include <QTextEdit>
#include <QTimer>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>

LauncherWindow::LauncherWindow(QWidget *parent)
    : QMainWindow(parent) {
    setupUi();
    setupConnections();
    loadInstalledVersions();
    exporter = new Exporter(this);

    m_gameLauncher = new GameLauncher(this);

    // Show one-time donation notice
    {
        QSettings settings;
        if (!settings.value("donation_notice_shown", false).toBool()) {
            QMessageBox::information(this, tr("Trinity Launcher"),
                tr("This project is currently being maintained by a single developer.\n\n"
                   "If you want to help or support, you can donate!"));
            settings.setValue("donation_notice_shown", true);
        }
    }

    connect(m_gameLauncher, &GameLauncher::gameFinished, this,
            [this](int code, QProcess::ExitStatus status) {
                // 1. Volver a mostrar el launcher
                this->show();
                this->raise(); // Traer al frente
                this->activateWindow();
            });

    // Connect game output to log widget
    connect(m_gameLauncher, &GameLauncher::gameOutput, this,
            [this](const QString &text) {
                if (logTextEdit) {
                    logTextEdit->moveCursor(QTextCursor::End);
                    logTextEdit->insertPlainText(text);
                    logTextEdit->moveCursor(QTextCursor::End);
                }
            });
    // Conectar señales de exporter

    connect(exporter, &Exporter::exportFinished, this,
            [this](bool success, const QString &msg) {
                if (success) {
                    QMessageBox::information(this, "Éxito", msg);
                    statusLabel->setText(msg);
                } else {
                    QMessageBox::critical(this, "Error", msg);
                }
            });

    connect(exporter, &Exporter::importFinished, this,
            [this](bool success, const QString &msg) {
                if (success) {
                    QMessageBox::information(this, "Éxito", msg);
                    loadInstalledVersions(); // Recargar lista
                    statusLabel->setText(msg);
                } else {
                    QMessageBox::critical(this, "Error", msg);
                }
            });
}

void LauncherWindow::setupUi() {
    setWindowTitle(tr("Trinity Launcher"));

    resize(1024, 600);
    setMinimumSize(860, 520);

    // Apply theme from saved settings (or Shinonome defaults if not set)
    {
        QSettings cfg;
        applyTheme(
            cfg.value("theme/accent",    "#D5ACA9").toString(),
            cfg.value("theme/bg",        "#1A1D20").toString(),
            cfg.value("theme/panel",     "#2D3339").toString(),
            cfg.value("theme/hover",     "#424B54").toString(),
            cfg.value("theme/btnHover",  "#525C66").toString(),
            cfg.value("theme/textMuted", "#B38D97").toString(),
            cfg.value("theme/text",      "#EBCFB2").toString()
        );
    }

    // ── Actions ──────────────────────────────────────────────────────
    actionExtract = new QAction(QIcon(":/icons/cube"), tr("Extract APK"), this);
    actionExtract->setToolTip(tr("Extract a new version from an APK/TMC file"));

    actionImport = new QAction(style()->standardIcon(QStyle::SP_DialogOpenButton), tr("Import"), this);
    actionImport->setToolTip(tr("Restore a saved version from a .tar.gz archive"));

    actionTrinito = new QAction(QIcon(":/icons/cube-w"), tr("Content Manager"), this);
    actionTrinito->setToolTip(tr("Open the Trinito content manager"));

    actionSettings = new QAction(QIcon(":/icons/settings"), tr("Settings"), this);
    actionLog = new QAction(QIcon(":/icons/warns"), tr("View Log"), this);
    actionDiscord = new QAction(QIcon(":/icons/discord"), tr("Discord"), this);
    actionAbout = new QAction(QIcon(":/icons/heart"), tr("About"), this);

    actionLaunch = new QAction(style()->standardIcon(QStyle::SP_MediaPlay), tr("Launch"), this);
    actionLaunch->setToolTip(tr("Launch the selected version"));

    actionConfig = new QAction(QIcon(":/icons/config"), tr("Config"), this);
    actionConfig->setToolTip(tr("Edit environment variables for the selected version"));

    actionExport = new QAction(style()->standardIcon(QStyle::SP_DialogSaveButton), tr("Export"), this);
    actionExport->setToolTip(tr("Back up the selected version to a .tar.gz file"));

    actionShortcut = new QAction(style()->standardIcon(QStyle::SP_DesktopIcon), tr("Shortcut"), this);
    actionShortcut->setToolTip(tr("Create a .desktop shortcut for the selected version"));

    actionDelete = new QAction(QIcon(":/icons/trash"), tr("Delete"), this);
    actionDelete->setToolTip(tr("Permanently remove the selected version"));

    // ── Menu bar ─────────────────────────────────────────────────────
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(actionSettings);
    fileMenu->addSeparator();
    QAction *actionQuit = fileMenu->addAction(tr("&Quit"));
    connect(actionQuit, &QAction::triggered, this, &QWidget::close);

    QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(actionExtract);
    toolsMenu->addAction(actionImport);
    toolsMenu->addSeparator();
    toolsMenu->addAction(actionTrinito);
    toolsMenu->addSeparator();
    toolsMenu->addAction(actionLog);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(actionDiscord);
    helpMenu->addAction(actionAbout);

    // ── Main toolbar (global actions) ────────────────────────────────
    QToolBar *mainToolBar = new QToolBar(tr("Main Toolbar"), this);
    mainToolBar->setObjectName("MainToolBar");
    mainToolBar->setMovable(false);
    mainToolBar->setFloatable(false);
    mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mainToolBar->setIconSize(QSize(20, 20));
    mainToolBar->addAction(actionExtract);
    mainToolBar->addAction(actionImport);
    mainToolBar->addSeparator();
    mainToolBar->addAction(actionTrinito);
    mainToolBar->addSeparator();
    mainToolBar->addAction(actionSettings);
    addToolBar(Qt::TopToolBarArea, mainToolBar);

    // ── Instance toolbar (contextual actions, right side) ────────────
    QToolBar *instanceToolBar = new QToolBar(tr("Instance Actions"), this);
    instanceToolBar->setObjectName("InstanceToolBar");
    instanceToolBar->setMovable(false);
    instanceToolBar->setFloatable(false);
    instanceToolBar->setOrientation(Qt::Vertical);
    instanceToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    instanceToolBar->setIconSize(QSize(28, 28));
    instanceToolBar->addAction(actionLaunch);
    instanceToolBar->addSeparator();
    instanceToolBar->addAction(actionConfig);
    instanceToolBar->addAction(actionExport);
    instanceToolBar->addAction(actionShortcut);
    instanceToolBar->addSeparator();
    instanceToolBar->addAction(actionDelete);
    addToolBar(Qt::RightToolBarArea, instanceToolBar);

    // ── Central widget: instance grid ────────────────────────────────
    versionList = new QListWidget();
    versionList->setObjectName("InstanceGrid");
    versionList->setViewMode(QListView::IconMode);
    versionList->setGridSize(QSize(116, 116));
    versionList->setIconSize(QSize(64, 64));
    versionList->setSpacing(8);
    versionList->setMovement(QListView::Static);
    versionList->setResizeMode(QListView::Adjust);
    versionList->setWordWrap(true);
    versionList->setTextElideMode(Qt::ElideRight);
    versionList->setUniformItemSizes(true);
    versionList->setItemDelegate(new InstanceDelegate(versionList));
    setCentralWidget(versionList);

    // Context menu on instances (same actions as the right toolbar)
    QAction *gridSeparator = new QAction(this);
    gridSeparator->setSeparator(true);
    versionList->setContextMenuPolicy(Qt::ActionsContextMenu);
    versionList->addActions({actionLaunch, actionConfig, actionExport,
                             actionShortcut, gridSeparator, actionDelete});

    // Restore saved wallpaper as the grid background
    {
        QSettings bgCfg;
        QString savedBg = bgCfg.value("background/path", "").toString();
        if (!savedBg.isEmpty() && QFile::exists(savedBg)) {
            versionList->setStyleSheet(
                QString("QListWidget#InstanceGrid {"
                        "  border-image: url(\"%1\") 0 0 0 0 stretch stretch;"
                        "}").arg(savedBg));
        }
    }

    // ── Status bar ───────────────────────────────────────────────────
    statusLabel = new QLabel(tr("Ready"));
    statusLabel->setObjectName("Status");
    pathLabel = new QLabel();
    pathLabel->setObjectName("PathLabel");
    statusBar()->addWidget(statusLabel, 1);
    statusBar()->addPermanentWidget(pathLabel);

    // Hidden versionCombo — kept for cross-window sync with TrinitoWindow
    versionCombo = new QComboBox(this);
    versionCombo->setVisible(false);

    // ── Secondary windows ────────────────────────────────────────────
    setupDialogs();
}

void LauncherWindow::setupDialogs() {
    m_trinito = new TrinitoWindow(this, this);

    auto wrapInDialog = [this](const QString &title, QWidget *page,
                               const QSize &size) -> QDialog * {
        QDialog *dialog = new QDialog(this);
        dialog->setWindowTitle(title);
        auto *layout = new QVBoxLayout(dialog);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(page);
        dialog->resize(size);
        return dialog;
    };

    m_settingsDialog = wrapInDialog(tr("Settings"), createSettingsPage(),
                                    QSize(680, 620));
    m_aboutDialog = wrapInDialog(tr("About Trinity Launcher"), createAboutPage(),
                                 QSize(560, 520));
    m_discordDialog = wrapInDialog(tr("Discord"), createDiscordPage(),
                                   QSize(420, 380));
    m_logDialog = wrapInDialog(tr("Log Output"), createLogPage(),
                               QSize(760, 480));
}

void LauncherWindow::setupConnections() {
    connect(actionExtract, &QAction::triggered, this,
            &LauncherWindow::showExtractDialog);
    connect(actionImport, &QAction::triggered, this,
            &LauncherWindow::onImportClicked);
    connect(actionTrinito, &QAction::triggered, this, [this]() {
        m_trinito->show();
        m_trinito->raise();
        m_trinito->activateWindow();
    });
    connect(actionSettings, &QAction::triggered, this, [this]() {
        m_settingsDialog->show();
        m_settingsDialog->raise();
        m_settingsDialog->activateWindow();
    });
    connect(actionLog, &QAction::triggered, this, [this]() {
        m_logDialog->show();
        m_logDialog->raise();
        m_logDialog->activateWindow();
    });
    connect(actionDiscord, &QAction::triggered, this, [this]() {
        m_discordDialog->show();
        m_discordDialog->raise();
        m_discordDialog->activateWindow();
    });
    connect(actionAbout, &QAction::triggered, this, [this]() {
        m_aboutDialog->show();
        m_aboutDialog->raise();
        m_aboutDialog->activateWindow();
    });

    connect(actionLaunch, &QAction::triggered, this,
            &LauncherWindow::launchGame);
    connect(actionConfig, &QAction::triggered, this,
            &LauncherWindow::onEditConfigClicked);
    connect(actionExport, &QAction::triggered, this,
            &LauncherWindow::onExportClicked);
    connect(actionShortcut, &QAction::triggered, this,
            &LauncherWindow::createDesktopShortcut);
    connect(actionDelete, &QAction::triggered, this,
            &LauncherWindow::onDeleteClicked);

    connect(versionList, &QListWidget::itemClicked, this,
            &LauncherWindow::onVersionSelected);
    connect(versionList, &QListWidget::itemDoubleClicked, this,
            [this](QListWidgetItem *) { launchGame(); });
    connect(versionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LauncherWindow::onVersionComboChanged);
}

void LauncherWindow::loadInstalledVersions() {
    versionList->clear();
    versionCombo->clear();
    VersionManager vm;
    QStringList versions = vm.getInstalledVersions();

    for (const QString &v : versions) {
        QListWidgetItem *item = new QListWidgetItem(QIcon(":/icons/cube"), v);
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
        const bool valid = vm.isVersionValid(v);
        item->setData(Qt::UserRole, valid);
        item->setToolTip(vm.getVersionPath(v));
        versionList->addItem(item);
        versionCombo->addItem(v);
    }

    if (versionList->count() > 0) {
        versionList->setCurrentRow(0);
        onVersionSelected(versionList->item(0));
        versionCombo->setCurrentIndex(0);
    } else {
        updateContextPanel("");
    }

    emit versionsChanged();
}

void LauncherWindow::onVersionSelected(QListWidgetItem *item) {
    if (!item)
        return;
    updateContextPanel(item->text());
    // Sync versionCombo to match
    int idx = versionCombo->findText(item->text());
    if (idx != -1)
        versionCombo->setCurrentIndex(idx);
}

void LauncherWindow::onVersionComboChanged(int index) {
    if (index < 0) return;
    QString version = versionCombo->itemText(index);
    // Sync hidden versionList
    for (int i = 0; i < versionList->count(); ++i) {
        if (versionList->item(i)->text() == version) {
            versionList->setCurrentRow(i);
            break;
        }
    }
    updateContextPanel(version);
}

void LauncherWindow::updateContextPanel(const QString &versionName) {
    const bool hasSelection = !versionName.isEmpty();

    actionLaunch->setEnabled(hasSelection);
    actionConfig->setEnabled(hasSelection);
    actionExport->setEnabled(hasSelection);
    actionShortcut->setEnabled(hasSelection);
    actionDelete->setEnabled(hasSelection);
    actionImport->setEnabled(true);

    if (!hasSelection) {
        pathLabel->clear();
        statusLabel->setText(tr("No versions installed."));
        return;
    }

    VersionManager vm;
    pathLabel->setText(vm.getVersionPath(versionName));
    statusLabel->setText(tr("Ready"));
}

void LauncherWindow::showExtractDialog() {
    ExtractDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    QString apkPath = dialog.getApkPath();
    QString versionName = dialog.getVersionName();

    // Verificar si ya existe la versión
    VersionManager vm;
    if (vm.getInstalledVersions().contains(versionName)) {
        int r = QMessageBox::warning(
            this, tr("Warning"),
            QString(tr("A version named '%1' already exists.\nReplace it?"))
                .arg(versionName),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (r == QMessageBox::No)
            return;
    }

    // Crear diálogo de progreso
    QDialog progressDlg(this);
    progressDlg.setWindowTitle(tr("Extracting Version..."));
    progressDlg.setFixedSize(300, 100);

    auto *layout = new QVBoxLayout(&progressDlg);
    QLabel *label = new QLabel(tr("Extracting version..."));
    QProgressBar *progressBar = new QProgressBar();
    progressBar->setRange(0, 0); // Indefinido
    layout->addWidget(label);
    layout->addWidget(progressBar);

    progressDlg.show();
    QApplication::processEvents(); // Actualizar UI

    // Conectar signal de progreso (opcional)
    QObject::connect(&vm, &VersionManager::extractionProgress, &progressDlg,
                     [&label](const QString &msg) {
                         label->setText(msg);
                         QApplication::processEvents(); // Actualizar UI
                     });

    // Extraer versión
    QString errorMsg;
    bool success = vm.extractApk(apkPath, versionName, errorMsg);

    // Cerrar diálogo de progreso
    progressDlg.accept();

    if (!success) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Extraction failed:\n") + errorMsg);
        return;
    }

    QMessageBox::information(this, tr("Success"),
                             tr("Version extracted successfully!"));
    loadInstalledVersions(); // Recargar lista.
}

void LauncherWindow::launchGame() {
    QString selectedVersion = versionCombo->currentText();
    if (selectedVersion.isEmpty())
        return;

    QString errorMsg;

    if (!m_gameLauncher->launchGame(selectedVersion, errorMsg)) {
        QMessageBox::critical(this, "Error", errorMsg);
        return;
    }
    this->hide();
}

void LauncherWindow::onEditConfigClicked() {
    QString selectedVersion = versionCombo->currentText();
    if (selectedVersion.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("No version selected."));
        return;
    }

    // Read active theme colors (same defaults used at startup)
    QSettings cfg;
    const QString accent    = cfg.value("theme/accent",    "#D5ACA9").toString();
    const QString bg        = cfg.value("theme/bg",        "#1A1D20").toString();
    const QString panel     = cfg.value("theme/panel",     "#2D3339").toString();
    const QString hover     = cfg.value("theme/hover",     "#424B54").toString();
    const QString btnHover  = cfg.value("theme/btnHover",  "#525C66").toString();
    const QString textMuted = cfg.value("theme/textMuted", "#B38D97").toString();
    const QString text      = cfg.value("theme/text",      "#EBCFB2").toString();

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Environment Parameters — ") + selectedVersion);
    dialog.setFixedWidth(500);

    // Dialog background matches app bg
    dialog.setStyleSheet(QString(
        "QDialog { background-color: %1; }"
        "QLabel  { background: transparent; color: %2; font-size: 13px; }"
        "QPushButton { background-color: %3; border: none; border-radius: 6px; "
        "              padding: 8px 20px; color: %2; font-weight: bold; font-size: 13px; }"
        "QPushButton:hover  { background-color: %4; }"
        "QPushButton:pressed{ background-color: %1; }"
        "QPushButton#OkBtn  { background-color: %5; color: %1; }"
        "QPushButton#OkBtn:hover { background-color: %5; opacity: 0.85; }"
    ).arg(bg, text, hover, btnHover, accent));

    auto *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(10);

    // Title label
    QLabel *titleLabel = new QLabel(tr("Environment Variables"));
    titleLabel->setStyleSheet(QString(
        "font-size: 15px; font-weight: bold; color: %1; background: transparent;")
        .arg(text));
    layout->addWidget(titleLabel);

    // Hint label
    QLabel *hintLabel = new QLabel(
        tr("One variable per line, or space-separated. Example:"));
    hintLabel->setStyleSheet(QString(
        "font-size: 12px; color: %1; background: transparent;").arg(textMuted));
    layout->addWidget(hintLabel);

    // Example label (monospace, accent colored)
    QLabel *exampleLabel = new QLabel("DRI_PRIME=1   vblank_mode=0   MESA_LOADER_DRIVER_OVERRIDE=zink");
    exampleLabel->setStyleSheet(QString(
        "font-family: 'Monospace', monospace; font-size: 11px; "
        "color: %1; background: transparent; padding: 2px 0;").arg(accent));
    exampleLabel->setWordWrap(true);
    layout->addWidget(exampleLabel);

    layout->addSpacing(4);

    // Obtain current args
    VersionConfig config(selectedVersion);
    QString currentArgs = config.getLaunchArgs();

    // Styled QTextEdit — visually distinct from the dialog background
    QTextEdit *argsEdit = new QTextEdit();
    argsEdit->setPlainText(currentArgs);
    argsEdit->setPlaceholderText("DRI_PRIME=1\nvblank_mode=0\nMESA_LOADER_DRIVER_OVERRIDE=zink");
    argsEdit->setAcceptRichText(false);
    argsEdit->setFixedHeight(130);
    argsEdit->setStyleSheet(QString(
        "QTextEdit {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 8px;"
        "  padding: 10px;"
        "  font-family: 'Monospace', monospace;"
        "  font-size: 13px;"
        "  selection-background-color: %4;"
        "  selection-color: %5;"
        "}"
        "QTextEdit:focus {"
        "  border: 1.5px solid %4;"
        "}"
    ).arg(panel, text, hover, accent, bg));
    layout->addWidget(argsEdit);

    layout->addSpacing(6);

    // Button row
    auto *btnRow = new QHBoxLayout();
    btnRow->setSpacing(10);
    auto *cancelBtn = new QPushButton(tr("Cancel"));
    auto *okBtn     = new QPushButton(tr("Save"));
    okBtn->setObjectName("OkBtn");
    okBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    btnRow->addStretch();
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(okBtn);
    layout->addLayout(btnRow);

    dialog.adjustSize();

    connect(okBtn, &QPushButton::clicked, &dialog, [&]() {
        QString newArgs = argsEdit->toPlainText().trimmed();
        config.setLaunchArgs(newArgs);

        VersionManager vm;
        QString errorMsg;
        if (!vm.editVersion(selectedVersion, newArgs, errorMsg)) {
            QMessageBox::critical(&dialog, "Error",
                                  tr("Could not save configuration:\n") + errorMsg);
        } else {
            dialog.accept();
        }
    });
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        statusLabel->setText(QString(tr("Configuration of %1 updated."))
                                 .arg(selectedVersion));
    }
}

void LauncherWindow::onExportClicked() {
    QString selectedVersion = versionCombo->currentText();
    if (selectedVersion.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("No version selected."));
        return;
    }

    exporter->exportVersion(selectedVersion);
}

void LauncherWindow::onDeleteClicked() {
    QString selectedVersion = versionCombo->currentText();
    if (selectedVersion.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("No version selected."));
        return;
    }

    int r = QMessageBox::warning(
        this, tr("Warning"),
        QString(tr("Are you sure you want to delete version '%1'?\nThis action "
                   "cannot be undone.")
                    .arg(selectedVersion)),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (r == QMessageBox::No)
        return;

    // Eliminar versión
    VersionManager vm;
    QString errorMsg;
    if (!vm.deleteVersion(selectedVersion, errorMsg)) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Could not delete version:\n") +
                                  errorMsg);
        return;
    }

    QMessageBox::information(this, tr("Success"),
                             tr("Version deleted successfully."));
    loadInstalledVersions(); // Recargar lista
    statusLabel->setText(
        QString(tr("Version %1 deleted.")).arg(selectedVersion));
}

void LauncherWindow::selectVersion(const QString &version) {
    if (versionCombo)
        versionCombo->setCurrentText(version);
}

void LauncherWindow::onImportClicked() { exporter->importVersion(); }

void LauncherWindow::createDesktopShortcut() {
    QString selectedVersion = versionCombo->currentText();
    if (selectedVersion.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("No version selected."));
        return;
    }

    // Obtener la ruta de la versión
    VersionManager vm;
    QString versionPath = vm.getVersionPath(selectedVersion);

    if (!vm.isVersionValid(selectedVersion)) {
        QMessageBox::critical(
            this, "Error",
            QString(tr("Version '%1' is not valid or complete."))
                .arg(selectedVersion));
        return;
    }

    // Ruta de la carpeta Downloads (segura para Flatpak)
    QString downloadsDir =
        QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QString shortcutPath =
        downloadsDir + "/Minecraft " + selectedVersion + ".desktop";

    // Verificar si ya existe
    if (QFile::exists(shortcutPath)) {
        int r = QMessageBox::question(
            this, tr("Confirm"),
            QString(
                tr("A shortcut for '%1' already exists.\nReplace it?"))
                .arg(selectedVersion),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (r == QMessageBox::No)
            return;
    }

    // Determine the mcpelauncher data directory
    QString mcpelauncherDataDir = VersionManager::getDataRoot();

    // Construir el comando para el .desktop
    QString execCmd;
#ifdef Q_OS_LINUX
    if (VersionManager::isFlatpak()) {
        execCmd = "flatpak run --command=mcpelauncher-client "
                  "com.trench.trinity.launcher -dg \"" +
                  versionPath + "\" -dd \"" + mcpelauncherDataDir + "\"";
    } else {
        execCmd = "mcpelauncher-client -dg \"" + versionPath
                  + "\" -dd \"" + mcpelauncherDataDir + "\"";
    }
#else
    execCmd = "mcpelauncher-client -dg \"" + versionPath
              + "\" -dd \"" + mcpelauncherDataDir + "\"";
#endif

    // Usar un icono genérico de juego
    QString iconIdentifier =
        "applications-games"; // O puedes probar con "minecraft"

    // Crear contenido del archivo .desktop
    QString desktopContent =
        QString("[Desktop Entry]\n"
                "Type=Application\n"
                "Name=Minecraft %1\n" // %1 es el nombre de la versión
                "Exec=%2\n"           // %2 es el comando exec
                "Icon=%3\n" // %3 es el identificador del icono genérico
                "Terminal=false\n"
                "Categories=Game;\n"
                "Comment=Jugar a Minecraft %1 desde Trinity Launcher\n"
                "StartupNotify=true\n")
            .arg(selectedVersion, execCmd, iconIdentifier);

    QFile desktopFile(shortcutPath);
    if (!desktopFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(
            this, "Error",
            tr("No se pudo crear el archivo de acceso directo en:\n") +
                shortcutPath);
        return;
    }

    QTextStream out(&desktopFile);
    out << desktopContent;
    desktopFile.close();

    // Mensaje de éxito
    QMessageBox::information(
        this, tr("Success"),
        QString(tr("Shortcut created in Downloads folder")
                    .arg(shortcutPath)));
}

void LauncherWindow::onLanguageChanged(int index) {
    // Guardamos el índice anterior (el último válido en QSettings)
    QSettings settings;
    QString currentSavedLang = settings.value("language",
        QLocale::system().name().split('_').first()).toString();
    int prevIndex = settingsLanguageCombo->findData(currentSavedLang);
    if (prevIndex == -1) prevIndex = 0;

    // Si el usuario seleccionó el mismo idioma que ya estaba guardado, ignorar
    QString newLangCode = settingsLanguageCombo->itemData(index).toString();
    if (newLangCode == currentSavedLang)
        return;

    int r = QMessageBox::question(
        this, tr("Restart required"),
        tr("The language will change to '%1'.\nDo you want to restart the application now "
           "to apply the changes?")
            .arg(settingsLanguageCombo->currentText()),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (r == QMessageBox::Yes) {
        settings.setValue("language", newLangCode);
        settings.sync();
        QString program = QCoreApplication::applicationFilePath();
        QStringList arguments = QCoreApplication::arguments();
        QStringList args = arguments.mid(1);
        QProcess::startDetached(program, args);
        QApplication::quit();
    } else {
        // Revertir el combo al idioma guardado sin disparar de nuevo esta señal
        settingsLanguageCombo->blockSignals(true);
        settingsLanguageCombo->setCurrentIndex(prevIndex);
        settingsLanguageCombo->blockSignals(false);
    }
}

// ──────────────────────────────────────────────
// About / Discord / Log pages
// ──────────────────────────────────────────────

QWidget *LauncherWindow::createAboutPage() {
    QWidget *aboutPage = new QWidget();
    QVBoxLayout *aboutLayout = new QVBoxLayout(aboutPage);
    aboutLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setObjectName("AboutScroll");

    QWidget *scrollContent = new QWidget();
    scrollContent->setObjectName("AboutContent");
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(40, 40, 40, 40);
    scrollLayout->setSpacing(20);

    QLabel *aboutTitle = new QLabel(tr("About Trinity Launcher"));
    aboutTitle->setObjectName("VersionName");
    aboutTitle->setAlignment(Qt::AlignCenter);
    scrollLayout->addWidget(aboutTitle);

    QLabel *aboutDesc = new QLabel(tr("Trinity Launcher is an open-source, community-driven launcher for Minecraft Bedrock. "
                                      "Focused on user freedom and free redistribution, it provides a powerful interface to "
                                      "manage multiple instances, worlds, textures, and mods seamlessly."));
    aboutDesc->setWordWrap(true);
    aboutDesc->setObjectName("AboutText");
    aboutDesc->setAlignment(Qt::AlignJustify);
    scrollLayout->addWidget(aboutDesc);

    // Maintenance & donation section
    QFrame *donateSep = new QFrame();
    donateSep->setFrameShape(QFrame::HLine);
    scrollLayout->addWidget(donateSep);

    QLabel *maintenanceMsg = new QLabel(tr("This project is currently being maintained by a single developer. "
                                           "If you want to help or support, you can donate!"));
    maintenanceMsg->setWordWrap(true);
    maintenanceMsg->setObjectName("AboutText");
    maintenanceMsg->setAlignment(Qt::AlignCenter);
    scrollLayout->addWidget(maintenanceMsg);

    QPushButton *donateBtn = new QPushButton(tr("DONAR"));
    donateBtn->setObjectName("ActionButton");
    donateBtn->setMinimumHeight(40);
    donateBtn->setCursor(Qt::PointingHandCursor);
    scrollLayout->addWidget(donateBtn, 0, Qt::AlignCenter);

    connect(donateBtn, &QPushButton::clicked, this, []() {
        QDesktopServices::openUrl(QUrl("https://linktr.ee/javiercplusx"));
    });

    QFrame *donateSep2 = new QFrame();
    donateSep2->setFrameShape(QFrame::HLine);
    scrollLayout->addWidget(donateSep2);

    QLabel *teamTitle = new QLabel(tr("Our Team"));
    teamTitle->setObjectName("Title");
    scrollLayout->addWidget(teamTitle);

    QLabel *teamDesc = new QLabel(tr("Trinity is built by a dedicated group of developers, designers, and contributors:"));
    teamDesc->setWordWrap(true);
    teamDesc->setObjectName("AboutText");
    scrollLayout->addWidget(teamDesc);

    // Team list
    QStringList teamMembers = {
        tr("<b>Crow</b>: Project Creator & Visionary."),
        tr("<b>JavierC</b>: Co-Creator & Development Supervisor."),
        tr("<b>Orta</b>: Project Supervisor & Software Architect."),
        tr("<b>MrTanuk</b>: Core Developer."),
        tr("<b>Ezequiel</b>: Web Design & Frontend Developer."),
        tr("<b>KevinRunforrestt</b>: Documentation, Translation & Support."),
        tr("<b>IoselDev</b>: AUR Package Maintainer."),
        tr("<b>HylianSoul</b>: Catalan Translation & Community Support."),
        tr("<b>BrokenByteOfCode</b>: Ukrainian Translation"),
        tr("<b>Future Contributor</b>: This spot is reserved for you. Join us!")
    };

    for (const QString &member : teamMembers) {
        QLabel *memberLabel = new QLabel(member);
        memberLabel->setTextFormat(Qt::RichText);
        memberLabel->setWordWrap(true);
        memberLabel->setStyleSheet("font-size: 15px; background: transparent; margin-left: 10px;");
        scrollLayout->addWidget(memberLabel);
    }

    QLabel *thanksTitle = new QLabel(tr("Special Thanks"));
    thanksTitle->setObjectName("Title");
    scrollLayout->addWidget(thanksTitle);

    QLabel *thanksDesc = new QLabel(tr("We would like to express our sincere gratitude to the team behind the "
                                       "<b>Unofficial NIX Launcher for Minecraft</b>. Their work provides the essential runtime "
                                       "to run Minecraft, which has been fundamental to the development of this project."));
    thanksDesc->setTextFormat(Qt::RichText);
    thanksDesc->setWordWrap(true);
    thanksDesc->setStyleSheet("font-size: 15px; background: transparent;");
    thanksDesc->setAlignment(Qt::AlignJustify);
    scrollLayout->addWidget(thanksDesc);

    scrollLayout->addStretch();
    scrollArea->setWidget(scrollContent);
    aboutLayout->addWidget(scrollArea);

    return aboutPage;
}

QWidget *LauncherWindow::createDiscordPage() {
    QWidget *discordPage = new QWidget();
    QVBoxLayout *discordLayout = new QVBoxLayout(discordPage);
    discordLayout->setContentsMargins(40, 40, 40, 40);
    discordLayout->setSpacing(20);
    discordLayout->addStretch();

    // Discord icon
    QLabel *discordIcon = new QLabel();
    discordIcon->setFixedSize(64, 64);
    discordIcon->setPixmap(QPixmap(":/icons/discord").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    discordIcon->setAlignment(Qt::AlignCenter);
    discordIcon->setStyleSheet("background: transparent;"); // keep transparent for icon overlay
    discordLayout->addWidget(discordIcon, 0, Qt::AlignCenter);

    QLabel *discordTitle = new QLabel(tr("Discord"));
    discordTitle->setObjectName("VersionName");
    discordTitle->setAlignment(Qt::AlignCenter);
    discordLayout->addWidget(discordTitle);

    QLabel *discordDesc = new QLabel(tr("Join our community on Discord"));
    discordDesc->setObjectName("VersionType");
    discordDesc->setAlignment(Qt::AlignCenter);
    discordLayout->addWidget(discordDesc);

    discordLayout->addSpacing(10);

    // Discord URL Box (Clickable via QPushButton)
    QPushButton *discordUrlBox = new QPushButton("https://discord.gg/8HvMHypRrP");
    discordUrlBox->setFlat(true);
    discordUrlBox->setObjectName("DiscordUrlBox");
    discordUrlBox->setMinimumHeight(40);
    discordUrlBox->setMaximumWidth(300);
    discordUrlBox->setCursor(Qt::PointingHandCursor);
    discordUrlBox->setToolTip(tr("Click to copy the link"));
    discordLayout->addWidget(discordUrlBox, 0, Qt::AlignCenter);

    connect(discordUrlBox, &QPushButton::clicked, this, [discordUrlBox]() {
        QApplication::clipboard()->setText("https://discord.gg/8HvMHypRrP");

        discordUrlBox->setText(tr("✓ Copied!"));
        discordUrlBox->setStyleSheet("color: #4ade80; border-color: #4ade80;");

        QTimer::singleShot(1500, discordUrlBox, [discordUrlBox]() {
            discordUrlBox->setText("https://discord.gg/8HvMHypRrP");
            discordUrlBox->setStyleSheet(""); // revert to theme default
        });
    });

    discordLayout->addSpacing(20);

    // Rich Presence toggle
    QHBoxLayout *toggleRow = new QHBoxLayout();
    toggleRow->setSpacing(12);
    QLabel *rpcLabel = new QLabel(tr("Discord Rich Presence"));
    rpcLabel->setStyleSheet("font-size: 15px; background: transparent;"); // keep font-size override
    QCheckBox *rpcToggle = new QCheckBox();
    rpcToggle->setChecked(DiscordManager::instance().isEnabled());
    rpcToggle->setObjectName("ThemeCheckBox");
    rpcToggle->setCursor(Qt::PointingHandCursor);
    toggleRow->addStretch();
    toggleRow->addWidget(rpcLabel);
    toggleRow->addWidget(rpcToggle);
    toggleRow->addStretch();
    discordLayout->addLayout(toggleRow);
    connect(rpcToggle, &QCheckBox::toggled, this, [this](bool checked) {
        DiscordManager::instance().setEnabled(checked);
        if (!checked) {
            QMessageBox::information(this, tr("Discord Rich Presence"),
                tr("Close and reopen the launcher to apply the configuration."));
        }
    });

    discordLayout->addStretch();
    return discordPage;
}

QWidget *LauncherWindow::createLogPage() {
    QWidget *page = new QWidget();
    QVBoxLayout *outerLayout = new QVBoxLayout(page);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    logTextEdit = new QTextEdit();
    logTextEdit->setReadOnly(true);
    logTextEdit->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    logTextEdit->setLineWrapMode(QTextEdit::NoWrap);
    logTextEdit->setFont(QFont("Monospace", 11));
    logTextEdit->setObjectName("LogTextEdit");
    outerLayout->addWidget(logTextEdit);

    return page;
}

// ──────────────────────────────────────────────
// Settings Page
// ──────────────────────────────────────────────

void LauncherWindow::applyTheme(const QString &accent,
                                const QString &bg,
                                const QString &panel,
                                const QString &hover,
                                const QString &btnHover,
                                const QString &textMuted,
                                const QString &text) {
    QString ss =
        QString(
            // Base
            "QWidget { background-color: %2; color: %7; "
            "font-family: 'Roboto', sans-serif; }"
            "QMainWindow, QDialog { background-color: %2; }"

            // Menu bar & menus
            "QMenuBar { background-color: %2; color: %7; padding: 2px; }"
            "QMenuBar::item { background: transparent; padding: 6px 10px; "
            "border-radius: 4px; }"
            "QMenuBar::item:selected { background-color: %4; }"
            "QMenu { background-color: %3; color: %7; border: 1px solid %4; "
            "padding: 4px; }"
            "QMenu::item { padding: 6px 24px; border-radius: 4px; }"
            "QMenu::item:selected { background-color: %4; }"
            "QMenu::separator { height: 1px; background: %4; margin: 4px 8px; }"

            // Toolbars
            "QToolBar { background-color: %2; border: none; spacing: 4px; "
            "padding: 4px; }"
            "QToolBar::separator { background: %4; margin: 4px 6px; }"
            "QToolBar::separator:horizontal { width: 1px; }"
            "QToolBar::separator:vertical { height: 1px; }"
            "QToolButton { background: transparent; border: none; "
            "border-radius: 6px; padding: 6px 10px; color: %7; font-size: 12px; }"
            "QToolButton:hover { background-color: %5; }"
            "QToolButton:pressed { background-color: %4; }"
            "QToolButton:disabled { color: %6; }"
            "QToolBar#InstanceToolBar { border-left: 1px solid %4; }"
            "QToolBar#InstanceToolBar QToolButton { min-width: 84px; "
            "min-height: 54px; padding: 8px 4px; font-size: 12px; "
            "font-weight: bold; }"
            "QToolBar#MainToolBar { border-bottom: 1px solid %4; }"

            // Status bar
            "QStatusBar { background-color: %2; color: %6; "
            "border-top: 1px solid %4; padding: 2px 8px; }"
            "QStatusBar::item { border: none; }"

            // Lists (generic, e.g. content lists in Trinito)
            "QListWidget { background-color: %3; border: 1px solid %4; "
            "border-radius: 8px; padding: 5px; outline: 0; }"
            "QListWidget::item { padding: 8px; border-radius: 6px; "
            "margin-bottom: 4px; border: none; }"
            "QListWidget::item:selected { background-color: %1; color: %2; }"
            "QListWidget::item:hover { background-color: %4; }"
            "QListWidget::indicator { width: 16px; height: 16px; }"

            // Instance grid (Prism-style icon view)
            "QListWidget#InstanceGrid { background-color: %2; border: none; "
            "border-radius: 0px; padding: 12px; }"
            "QListWidget#InstanceGrid::item { margin: 0px; padding: 8px; "
            "border: 2px solid transparent; border-radius: 8px; color: %7; }"
            "QListWidget#InstanceGrid::item:selected { background-color: %4; "
            "border: 2px solid %1; color: %7; }"
            "QListWidget#InstanceGrid::item:hover { background-color: %5; }"
            "QListWidget#InstanceGrid::item:selected:hover { "
            "background-color: %4; }"

            // Page sidebar (Trinito dialog navigation)
            "QListWidget#PageSidebar { background-color: %2; border: none; "
            "border-right: 1px solid %4; border-radius: 0px; padding: 8px; }"
            "QListWidget#PageSidebar::item { margin-bottom: 2px; "
            "padding: 8px 12px; border-radius: 6px; color: %6; }"
            "QListWidget#PageSidebar::item:selected { background-color: %4; "
            "color: %7; }"
            "QListWidget#PageSidebar::item:hover { background-color: %5; "
            "color: %7; }"

            // Push buttons
            "QPushButton { background-color: %4; border: none; "
            "border-radius: 6px; padding: 8px 16px; color: %7; "
            "font-weight: bold; font-size: 13px; }"
            "QPushButton:hover { background-color: %5; }"
            "QPushButton:pressed { background-color: %2; }"
            "QPushButton:disabled { background-color: %3; color: %6; }"
            "QPushButton#ActionButton { background-color: %1; color: %2; }"
            "QPushButton#ActionButton:hover { background-color: %1; "
            "opacity: 0.85; }"

            // Inputs
            "QLineEdit { background-color: %3; color: %7; "
            "border: 1px solid %4; border-radius: 6px; padding: 6px 8px; "
            "selection-background-color: %1; selection-color: %2; }"
            "QLineEdit:focus { border: 1px solid %1; }"
            "QTextEdit { background-color: %3; color: %7; "
            "border: 1px solid %4; border-radius: 8px; padding: 6px; "
            "selection-background-color: %1; selection-color: %2; }"
            "QComboBox { background-color: %4; color: %7; border-radius: 6px; "
            "padding: 6px 10px; font-size: 13px; }"
            "QComboBox::drop-down { border: 0px; }"
            "QComboBox QAbstractItemView { background-color: %3; "
            "selection-background-color: %1; selection-color: %2; color: %7; }"

            // Progress bar
            "QProgressBar { background-color: %3; border: 1px solid %4; "
            "border-radius: 6px; text-align: center; color: %7; }"
            "QProgressBar::chunk { background-color: %1; border-radius: 5px; }"

            // Labels
            "QLabel#Title { font-size: 16px; font-weight: bold; color: %1; "
            "background: transparent; }"
            "QLabel#VersionName { font-size: 18px; font-weight: bold; "
            "background: transparent; }"
            "QLabel#VersionType { font-size: 14px; color: %6; "
            "background: transparent; }"
            "QLabel#Status { font-size: 12px; color: %6; "
            "background: transparent; }"
            "QLabel#PathLabel { font-size: 12px; color: %6; "
            "background: transparent; font-family: 'Monospace', monospace; }"
            "QLabel#AboutText { font-size: 15px; background: transparent; }"
            "QLabel#PanelTitle { font-size: 13px; font-weight: bold; "
            "color: %6; background: transparent; }"

            // Cards / panels
            "QWidget#ContextPanel { background-color: %3; "
            "border-radius: 12px; }"
            "QWidget#Panel { background-color: %3; border-radius: 12px; }"

            // Tabs (kept for any tabbed secondary content)
            "QTabWidget::pane { border: 1px solid %4; background-color: %3; "
            "border-radius: 8px; top: -1px; }"
            "QTabBar::tab { background: %4; color: %6; padding: 10px 20px; "
            "border-top-left-radius: 6px; border-top-right-radius: 6px; "
            "margin-right: 4px; border: none; }"
            "QTabBar::tab:selected { background: %1; color: %2; }"
            "QTabBar::tab:hover { background: %5; }"

            // Discord URL box
            "QPushButton#DiscordUrlBox { background-color: %4; color: %1; "
            "border: 1px dashed %5; border-radius: 6px; padding: 8px; "
            "font-size: 13px; font-weight: bold; text-align: center; }"

            // Themed checkbox
            "QCheckBox#ThemeCheckBox::indicator { width: 22px; height: 22px; "
            "border-radius: 11px; background-color: %4; "
            "border: 2px solid %5; }"
            "QCheckBox#ThemeCheckBox::indicator:checked { "
            "background-color: %1; border-color: %1; }"

            // Scroll areas & bars
            "QScrollArea { background: transparent; border: none; }"
            "QScrollBar:vertical { background: %2; width: 10px; "
            "border-radius: 5px; }"
            "QScrollBar::handle:vertical { background: %4; "
            "border-radius: 5px; min-height: 24px; }"
            "QScrollBar::handle:vertical:hover { background: %5; }"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { "
            "height: 0; }"
            "QScrollBar:horizontal { background: %2; height: 10px; "
            "border-radius: 5px; }"
            "QScrollBar::handle:horizontal { background: %4; "
            "border-radius: 5px; min-width: 24px; }"
            "QScrollBar::handle:horizontal:hover { background: %5; }"
            "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { "
            "width: 0; }"

            // Tooltips
            "QToolTip { background-color: %3; color: %7; "
            "border: 1px solid %4; padding: 4px; }"

            // Separator frames
            "QFrame[frameShape=\"4\"] { color: %4; background-color: %4; "
            "max-height: 1px; }"
            "QFrame[frameShape=\"5\"] { color: %4; background-color: %4; "
            "max-width: 1px; }"
            "QFrame#Divider { color: %4; background-color: %4; }"
        )
        .arg(accent)    // %1
        .arg(bg)        // %2
        .arg(panel)     // %3
        .arg(hover)     // %4
        .arg(btnHover)  // %5
        .arg(textMuted) // %6
        .arg(text);     // %7

    qApp->setStyleSheet(ss);

    // Persistir
    QSettings settings;
    settings.setValue("theme/accent",    accent);
    settings.setValue("theme/bg",        bg);
    settings.setValue("theme/panel",     panel);
    settings.setValue("theme/hover",     hover);
    settings.setValue("theme/btnHover",  btnHover);
    settings.setValue("theme/textMuted", textMuted);
    settings.setValue("theme/text",      text);
}

void LauncherWindow::generateThemeFromWallpaper(const QString &wallpaperPath) {
    auto colors = ColorExtractor::extractColors(wallpaperPath, 6);
    if (colors.isEmpty()) {
        qWarning() << "[Theme] Could not extract colors from wallpaper";
        return;
    }

    // Calculate average brightness of the wallpaper to determine text color
    double totalLightness = 0;
    for (const auto &c : colors) {
        totalLightness += c.color.lightnessF();
    }
    double avgLightness = totalLightness / colors.size();

    // Find the most vibrant color (highest saturation) for accent
    int accentIdx = 0;
    double bestSat = 0;
    for (int i = 0; i < colors.size(); ++i) {
        double sat = colors[i].color.saturationF();
        double light = colors[i].color.lightnessF();
        // Prefer colors that are saturated and not too dark/light
        double score = sat * (1.0 - std::abs(light - 0.5) * 1.5);
        if (score > bestSat) {
            bestSat = score;
            accentIdx = i;
        }
    }

    QColor accentColor = colors[accentIdx].color;
    double hue = accentColor.hslHueF();
    double sat = accentColor.hslSaturationF();

    // Generate all 6 tokens from the accent hue
    QColor accent   = QColor::fromHslF(hue, qMin(sat * 1.1, 1.0), 0.55);
    QColor bg       = QColor::fromHslF(hue, sat * 0.6, 0.02);
    QColor panel    = QColor::fromHslF(hue, sat * 0.5, 0.06);
    QColor hover    = QColor::fromHslF(hue, sat * 0.3, 0.15);
    QColor btnHover = QColor::fromHslF(hue, sat * 0.2, 0.25);
    QColor muted    = QColor::fromHslF(hue, sat * 0.15, 0.65);

    // Text color based on wallpaper average brightness
    QColor text;
    if (avgLightness > 0.6) {
        text = QColor("#0f172a");  // Dark text for bright wallpapers
    } else if (avgLightness < 0.4) {
        text = QColor("#ffffff");  // Light text for dark wallpapers
    } else {
        text = (bg.lightnessF() > 0.5) ? QColor("#0f172a") : QColor("#ffffff");
    }

    qDebug() << "[Theme] Generated from wallpaper:";
    qDebug() << "  accent:"   << accent.name();
    qDebug() << "  bg:"       << bg.name();
    qDebug() << "  panel:"    << panel.name();
    qDebug() << "  hover:"    << hover.name();
    qDebug() << "  btnHover:" << btnHover.name();
    qDebug() << "  muted:"    << muted.name();
    qDebug() << "  text:"     << text.name();

    applyTheme(accent.name(), bg.name(), panel.name(),
               hover.name(), btnHover.name(), muted.name(), text.name());
}

QWidget *LauncherWindow::createSettingsPage() {
    // Defaults - Shinonome (dark) palette
    const QString DEF_ACCENT    = "#D5ACA9";
    const QString DEF_BG        = "#1A1D20";
    const QString DEF_PANEL     = "#2D3339";
    const QString DEF_HOVER     = "#424B54";
    const QString DEF_BTNHOVER  = "#525C66";
    const QString DEF_TEXTMUTED = "#B38D97";
    const QString DEF_TEXT      = "#EBCFB2";

    QSettings cfg;
    QString accent    = cfg.value("theme/accent",    DEF_ACCENT).toString();
    QString bg        = cfg.value("theme/bg",        DEF_BG).toString();
    QString panel     = cfg.value("theme/panel",     DEF_PANEL).toString();
    QString hover     = cfg.value("theme/hover",     DEF_HOVER).toString();
    QString btnHover  = cfg.value("theme/btnHover",  DEF_BTNHOVER).toString();
    QString textMuted = cfg.value("theme/textMuted", DEF_TEXTMUTED).toString();
    QString textColor = cfg.value("theme/text",      DEF_TEXT).toString();

    auto *page = new QWidget();
    auto *outerLayout = new QVBoxLayout(page);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    auto *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *content = new QWidget();
    auto *layout  = new QVBoxLayout(content);
    layout->setContentsMargins(40, 30, 40, 30);
    layout->setSpacing(24);

    // ── Título ──
    auto *titleLabel = new QLabel(tr("Settings"));
    titleLabel->setObjectName("VersionName");
    titleLabel->setAlignment(Qt::AlignLeft);
    layout->addWidget(titleLabel);

    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    // SECCIÓN: Idioma / Language
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    auto *langSection = new QLabel(tr("Language"));
    langSection->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1;").arg(textMuted));
    layout->addWidget(langSection);

    auto *langSeparator = new QFrame();
    langSeparator->setFrameShape(QFrame::HLine);
    langSeparator->setStyleSheet(QString("color: %1;").arg(hover));
    layout->addWidget(langSeparator);

    {
        auto *langRow = new QHBoxLayout();
        auto *langLabel = new QLabel(tr("Interface language:"));
        langLabel->setStyleSheet("font-size: 15px;");
        langLabel->setMinimumWidth(180);

        settingsLanguageCombo = new QComboBox();
        settingsLanguageCombo->setFixedWidth(180);
        // Explicit styles for AppImage compatibility
        settingsLanguageCombo->setStyleSheet(
            QString(
                "QComboBox {"
                "    background-color: %1;"
                "    color: %2;"
                "    border: 1px solid %3;"
                "    border-radius: 4px;"
                "    padding: 4px 8px;"
                "    font-size: 14px;"
                "}"
                "QComboBox:hover {"
                "    background-color: %4;"
                "    border-color: %3;"
                "}"
                "QComboBox::drop-down {"
                "    border: none;"
                "    width: 20px;"
                "}"
                "QComboBox::down-arrow {"
                "    image: none;"
                "    border-left: 5px solid transparent;"
                "    border-right: 5px solid transparent;"
                "    border-top: 6px solid %5;"
                "    margin-right: 8px;"
                "}"
                "QComboBox QAbstractItemView {"
                "    background-color: %1;"
                "    color: %2;"
                "    border: 1px solid %3;"
                "    selection-background-color: %4;"
                "    selection-color: %2;"
                "    outline: none;"
                "    padding: 4px;"
                "}"
                "QComboBox QAbstractItemView::item {"
                "    min-height: 30px;"
                "    padding: 4px 8px;"
                "}"
                "QComboBox QAbstractItemView::item:hover {"
                "    background-color: %4;"
                "}"
                "QComboBox QAbstractItemView::item:selected {"
                "    background-color: %4;"
                "    color: %2;"
                "}"
            ).arg(panel, textColor, hover, btnHover, textMuted)
        );

        // Available languages (explicit list for AppImage compatibility)
        // Format: { language code, native name }
        const QStringList availableLanguages = {
            "es",     // Español (always available)
            "pt_BR",  // Português (Brasil) (always available)
            "en",     // English
            "ca",     // Català
            "uk"      // Українська
        };

        // Add default languages
        settingsLanguageCombo->addItem("Español", "es");
        settingsLanguageCombo->addItem("Português (Brasil)", "pt_BR");

        // Add other available languages
        for (const QString &code : availableLanguages) {
            if (code == "es" || code == "pt_BR")
                continue;
            // Check if translation file exists
            if (!QFile::exists(":/i18n/trinity_" + code + ".qm"))
                continue;
            QLocale loc(code);
            QString nativeName = loc.nativeLanguageName();
            if (!nativeName.isEmpty())
                nativeName[0] = nativeName[0].toUpper();
            else
                nativeName = code;
            settingsLanguageCombo->addItem(nativeName, code);
        }

        QSettings settings;
        QString systemLang = QLocale::system().name().split('_').first();
        if (!QFile::exists(":/i18n/trinity_" + systemLang + ".qm") && systemLang != "es")
            systemLang = "es";
        QString currentLang = settings.value("language", systemLang).toString();
        int langIdx = settingsLanguageCombo->findData(currentLang);
        if (langIdx != -1)
            settingsLanguageCombo->setCurrentIndex(langIdx);

        connect(settingsLanguageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &LauncherWindow::onLanguageChanged);

        langRow->addWidget(langLabel);
        langRow->addWidget(settingsLanguageCombo);
        langRow->addStretch();
        layout->addLayout(langRow);
    }

    layout->addSpacing(12);

    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    // SECCIÓN: Colores del tema
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    auto *colorSection = new QLabel(tr("UI Colors"));
    colorSection->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1;").arg(textMuted));
    layout->addWidget(colorSection);

    auto *colorsSeparator = new QFrame();
    colorsSeparator->setFrameShape(QFrame::HLine);
    colorsSeparator->setStyleSheet(QString("color: %1;").arg(hover));
    layout->addWidget(colorsSeparator);

    // Helper to create a color-picker row
    struct ColorRow {
        QString label;
        QString *value;
        QString settingKey;
    };

    auto *accentVal    = new QString(accent);
    auto *bgVal        = new QString(bg);
    auto *panelVal     = new QString(panel);
    auto *hoverVal     = new QString(hover);
    auto *btnHoverVal  = new QString(btnHover);
    auto *textMutedVal = new QString(textMuted);
    auto *textVal      = new QString(textColor);

    auto makeColorRow = [&](const QString &labelText, QString *colorRef,
                            const QString &settingKey) {
        auto *row = new QHBoxLayout();
        auto *lbl = new QLabel(labelText);
        lbl->setStyleSheet("font-size: 15px;");
        lbl->setMinimumWidth(180);

        auto *preview = new QPushButton();
        preview->setFixedSize(36, 36);
        preview->setStyleSheet(
            QString("background-color: %1; border-radius: 6px; border: 2px solid %2;")
                .arg(*colorRef, hover));
        preview->setCursor(Qt::PointingHandCursor);
        preview->setToolTip(tr("Click to change color"));

        auto *hexLabel = new QLabel(*colorRef);
        hexLabel->setStyleSheet(QString("font-size: 15px; color: %1; font-family: monospace;").arg(textMuted));
        hexLabel->setMinimumWidth(80);

        connect(preview, &QPushButton::clicked, this,
                [this, preview, hexLabel, colorRef,
                 accentVal, bgVal, panelVal, hoverVal, btnHoverVal, textMutedVal, textVal]() {
                    QColor initial(*colorRef);
                    QColor chosen = QColorDialog::getColor(initial, this, tr("Select Color"));
                    if (!chosen.isValid()) return;
                    *colorRef = chosen.name();
                    preview->setStyleSheet(
                        QString("background-color: %1; border: 1px solid %2;")
                            .arg(*colorRef, *hoverVal));
                    hexLabel->setText(*colorRef);
                    applyTheme(*accentVal, *bgVal, *panelVal, *hoverVal, *btnHoverVal, *textMutedVal, *textVal);
                });

        row->addWidget(lbl);
        row->addWidget(preview);
        row->addWidget(hexLabel);
        row->addStretch();
        layout->addLayout(row);
    };

    makeColorRow(tr("Accent color"),        accentVal,    "theme/accent");
    makeColorRow(tr("Background color"),    bgVal,        "theme/bg");
    makeColorRow(tr("Panel color"),         panelVal,     "theme/panel");
    makeColorRow(tr("Hover / border color"),hoverVal,     "theme/hover");
    makeColorRow(tr("Button hover color"),  btnHoverVal,  "theme/btnHover");
    makeColorRow(tr("Muted text color"),    textMutedVal, "theme/textMuted");
    makeColorRow(tr("Text color"),          textVal,      "theme/text");

    // Botón Reset de colores
    auto *resetColorsBtn = new QPushButton(tr("Reset Colors to Default"));
    resetColorsBtn->setObjectName("ActionButton");
    connect(resetColorsBtn, &QPushButton::clicked, this,
            [this, accentVal, bgVal, panelVal, hoverVal, btnHoverVal, textMutedVal, textVal,
             DEF_ACCENT, DEF_BG, DEF_PANEL, DEF_HOVER, DEF_BTNHOVER, DEF_TEXTMUTED, DEF_TEXT]() {
                *accentVal    = DEF_ACCENT;
                *bgVal        = DEF_BG;
                *panelVal     = DEF_PANEL;
                *hoverVal     = DEF_HOVER;
                *btnHoverVal  = DEF_BTNHOVER;
                *textMutedVal = DEF_TEXTMUTED;
                applyTheme(DEF_ACCENT, DEF_BG, DEF_PANEL, DEF_HOVER, DEF_BTNHOVER, DEF_TEXTMUTED, DEF_TEXT);
                QMessageBox::information(this, tr("Settings"),
                    tr("Colors reset to default. Reopen Settings to see the updated previews."));
            });
    auto *resetColRow = new QHBoxLayout();
    resetColRow->addWidget(resetColorsBtn);
    resetColRow->addStretch();
    layout->addLayout(resetColRow);

    layout->addSpacing(12);

    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    // Wallpaper / Background
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    auto *wallpaperSection = new QLabel(tr("Wallpaper"));
    wallpaperSection->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1;").arg(textMuted));
    layout->addWidget(wallpaperSection);

    auto *wallpaperSep = new QFrame();
    wallpaperSep->setFrameShape(QFrame::HLine);
    wallpaperSep->setStyleSheet(QString("color: %1;").arg(hover));
    layout->addWidget(wallpaperSep);

    {
        auto *wpRow = new QHBoxLayout();

        // Current background preview
        auto *wpPreview = new QLabel();
        wpPreview->setFixedSize(120, 68);
        wpPreview->setAlignment(Qt::AlignCenter);
        wpPreview->setStyleSheet(
            QString("border-radius: 8px; border: 2px solid %1; background: %2;").arg(hover, panel));
        wpPreview->setScaledContents(true);

        // Load currently set background
        QSettings bgCfg;
        QString savedBg = bgCfg.value("background/path", "").toString();
        if (!savedBg.isEmpty() && QFile::exists(savedBg))
            wpPreview->setPixmap(QPixmap(savedBg).scaled(120, 68, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        else
            wpPreview->setPixmap(QPixmap(":/branding/background").scaled(120, 68, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

        auto *wpInfoLayout = new QVBoxLayout();
        auto *wpPathLabel = new QLabel(savedBg.isEmpty() ? tr("Default background") : QFileInfo(savedBg).fileName());
        wpPathLabel->setStyleSheet(QString("font-size: 15px; color: %1;").arg(textMuted));
        wpPathLabel->setWordWrap(true);
        wpInfoLayout->addWidget(wpPathLabel);

        auto *wpBtnRow = new QHBoxLayout();

        auto *wpChangeBtn = new QPushButton(tr("Change..."));
        wpChangeBtn->setObjectName("ActionButton");
        wpChangeBtn->setCursor(Qt::PointingHandCursor);

        auto *wpResetBtn = new QPushButton(tr("Reset"));
        wpResetBtn->setObjectName("ActionButton");
        wpResetBtn->setCursor(Qt::PointingHandCursor);

        wpBtnRow->addWidget(wpChangeBtn);
        wpBtnRow->addWidget(wpResetBtn);
        wpBtnRow->addStretch();
        wpInfoLayout->addLayout(wpBtnRow);
        wpInfoLayout->addStretch();

        wpRow->addWidget(wpPreview);
        wpRow->addSpacing(16);
        wpRow->addLayout(wpInfoLayout);
        wpRow->addStretch();
        layout->addLayout(wpRow);

        // Change button: pick image file, save path, update preview & instance grid
        connect(wpChangeBtn, &QPushButton::clicked, this,
            [this, wpPreview, wpPathLabel]() {
                QString path = QFileDialog::getOpenFileName(
                    this, tr("Select background image"), QDir::homePath(),
                    tr("Images (*.png *.jpg *.jpeg *.bmp *.webp);;All files (*)"));
                if (path.isEmpty()) return;

                QSettings bgCfg;
                bgCfg.setValue("background/path", path);
                bgCfg.sync();

                wpPreview->setPixmap(QPixmap(path).scaled(120, 68, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
                wpPathLabel->setText(QFileInfo(path).fileName());

                // Apply immediately to the instance grid background
                versionList->setStyleSheet(
                    QString("QListWidget#InstanceGrid {"
                            "  border-image: url(\"%1\") 0 0 0 0 stretch stretch;"
                            "}").arg(path));

                // Auto-generate theme colors from the new wallpaper
                generateThemeFromWallpaper(path);
            });

        // Reset button: clear saved path, revert to plain background
        connect(wpResetBtn, &QPushButton::clicked, this,
            [this, wpPreview, wpPathLabel]() {
                QSettings bgCfg;
                bgCfg.remove("background/path");
                bgCfg.sync();

                wpPreview->setPixmap(QPixmap(":/branding/background").scaled(120, 68, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
                wpPathLabel->setText(tr("Default background"));

                // Revert to the theme default background
                versionList->setStyleSheet("");
            });
    }

    layout->addStretch();
    scrollArea->setWidget(content);
    outerLayout->addWidget(scrollArea);

    return page;
}
