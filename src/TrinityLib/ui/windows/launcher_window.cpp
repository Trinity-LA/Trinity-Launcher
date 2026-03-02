#include "TrinityLib/ui/windows/launcher_window.hpp"
#include "TrinityLib/ui/windows/trinito_window.hpp"
#include "TrinityLib/core/discord_manager.hpp"
#include "TrinityLib/core/version_config.hpp"
#include "TrinityLib/core/version_manager.hpp"
#include "TrinityLib/ui/dialogs/extract_dialog.hpp"

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
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QLocale>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QProcess>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QStandardPaths>
#include <QStackedWidget>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QStyle>
#include <QScreen>

LauncherWindow::LauncherWindow(QWidget *parent)
    : QWidget(parent) {
    setupUi();
    setupConnections();
    loadInstalledVersions();
    exporter = new Exporter(this);

    m_gameLauncher = new GameLauncher(this);

    connect(m_gameLauncher, &GameLauncher::gameFinished, this,
            [this](int code, QProcess::ExitStatus status) {
                // 1. Volver a mostrar el launcher
                this->show();
                this->raise(); // Traer al frente
                this->activateWindow();
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

    // Restaurar iconos personalizados al arranque
    {
        QSettings icfg;
        struct { QString key; QPushButton *btn; } iconMap[] = {
            { "icon/trinity", sidebarTrinityBtn },
            { "icon/content", sidebarContentBtn },
            { "icon/discord", sidebarDiscordBtn },
            { "icon/about",   sidebarAboutBtn   },
            { "icon/settings",sidebarSettingsBtn },
        };
        for (auto &e : iconMap) {
            QString path = icfg.value(e.key, "").toString();
            if (!path.isEmpty() && QFile::exists(path))
                e.btn->setIcon(QIcon(path));
        }
    }
}



void LauncherWindow::setupUi() {
    setWindowTitle(tr("Trinity Launcher - Minecraft Bedrock"));
    resize(960, 560);
    setMinimumSize(960, 560); // Tamaño mínimo

    // Apply theme from saved settings (or defaults if not set)
    {
        QSettings cfg;
        applyTheme(
            cfg.value("theme/accent",    "#8b5cf6").toString(),
            cfg.value("theme/bg",        "#020617").toString(),
            cfg.value("theme/panel",     "#090f20").toString(),
            cfg.value("theme/hover",     "#1e293b").toString(),
            cfg.value("theme/btnHover",  "#334155").toString(),
            cfg.value("theme/textMuted", "#94a3b8").toString()
        );
    }

    // Root: horizontal layout (sidebar | divider | content)
    QHBoxLayout *windowLayout = new QHBoxLayout(this);
    windowLayout->setContentsMargins(0, 0, 0, 0);
    windowLayout->setSpacing(0);

    // --- Sidebar ---
    QWidget *sidebar = new QWidget();
    sidebar->setObjectName("Sidebar");
    sidebar->setFixedWidth(52);
    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(0, 8, 0, 8);
    sidebarLayout->setSpacing(4);

    sidebarTrinityBtn = new QPushButton(QIcon(":/icons/cube-w"), "");
    sidebarTrinityBtn->setObjectName("SidebarBtnActive");
    sidebarTrinityBtn->setIconSize(QSize(26, 26));
    sidebarTrinityBtn->setFixedSize(52, 48);
    sidebarTrinityBtn->setCursor(Qt::PointingHandCursor);
    sidebarTrinityBtn->setToolTip(tr("Trinity"));

    sidebarContentBtn = new QPushButton(QIcon(":/icons/config"), "");
    sidebarContentBtn->setObjectName("SidebarBtn");
    sidebarContentBtn->setIconSize(QSize(26, 26));
    sidebarContentBtn->setFixedSize(52, 48);
    sidebarContentBtn->setCursor(Qt::PointingHandCursor);
    sidebarContentBtn->setToolTip(tr("Content Manager"));

    sidebarDiscordBtn = new QPushButton(QIcon(":/icons/discord"), "");
    sidebarDiscordBtn->setObjectName("SidebarBtn");
    sidebarDiscordBtn->setIconSize(QSize(26, 26));
    sidebarDiscordBtn->setFixedSize(52, 48);
    sidebarDiscordBtn->setCursor(Qt::PointingHandCursor);
    sidebarDiscordBtn->setToolTip(tr("Discord"));

    sidebarAboutBtn = new QPushButton(QIcon(":/icons/heart"), "");
    sidebarAboutBtn->setObjectName("SidebarBtn");
    sidebarAboutBtn->setIconSize(QSize(26, 26));
    sidebarAboutBtn->setFixedSize(52, 48);
    sidebarAboutBtn->setCursor(Qt::PointingHandCursor);
    sidebarAboutBtn->setToolTip(tr("About Trinity Launcher"));

    sidebarSettingsBtn = new QPushButton(QIcon(":/icons/settings"), "");
    sidebarSettingsBtn->setObjectName("SidebarBtn");
    sidebarSettingsBtn->setIconSize(QSize(26, 26));
    sidebarSettingsBtn->setFixedSize(52, 48);
    sidebarSettingsBtn->setCursor(Qt::PointingHandCursor);
    sidebarSettingsBtn->setToolTip(tr("Settings"));

    sidebarLayout->addWidget(sidebarTrinityBtn);
    sidebarLayout->addWidget(sidebarContentBtn);
    sidebarLayout->addWidget(sidebarDiscordBtn);
    sidebarLayout->addWidget(sidebarAboutBtn);
    sidebarLayout->addStretch();
    sidebarLayout->addWidget(sidebarSettingsBtn); // Settings al fondo
    windowLayout->addWidget(sidebar);

    // --- Vertical divider ---
    QFrame *divider = new QFrame();
    divider->setFrameShape(QFrame::VLine);
    divider->setStyleSheet("color: #1e293b; background-color: #1e293b; max-width: 1px;");
    windowLayout->addWidget(divider);

    // --- Content stack ---
    contentStack = new QStackedWidget();
    windowLayout->addWidget(contentStack);

    // === Page 0: Trinity (Launcher) — Background image + floating dock ===

    // Background container — uses a QLabel with scaled pixmap overlay via paintEvent.
    // We use a plain QWidget with a stylesheet border-image for the background.
    QWidget *launcherTab = new QWidget();
    launcherTab->setObjectName("LauncherTab");
    // Load saved custom wallpaper; fall back to the built-in resource.
    {
        QSettings bgCfg;
        QString savedBg = bgCfg.value("background/path", "").toString();
        if (!savedBg.isEmpty() && QFile::exists(savedBg)) {
            launcherTab->setStyleSheet(
                QString("QWidget#LauncherTab {"
                        "  border-image: url(\"%1\") 0 0 0 0 stretch stretch;"
                        "}").arg(savedBg));
        } else {
            launcherTab->setStyleSheet(
                "QWidget#LauncherTab {"
                "  border-image: url(:/branding/background) 0 0 0 0 stretch stretch;"
                "}");
        }
    }

    // Root layout for launcherTab — stacks content vertically:
    // [stretch] then [dock row] then [status label]
    QVBoxLayout *rootLayout = new QVBoxLayout(launcherTab);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // Hidden version list — keeps all installed-version logic intact
    versionList = new QListWidget();
    versionList->setVisible(false);
    versionList->setIconSize(QSize(48, 48));

    // Logo overlay — top-right of the background image
    {
        QHBoxLayout *topLogoRow = new QHBoxLayout();
        topLogoRow->setContentsMargins(14, 10, 14, 0);

        QLabel *logoLabel = new QLabel();
        logoLabel->setFixedSize(38, 38);
        logoLabel->setStyleSheet(
            "border-image: url(:/branding/logo);"
            "border-radius: 8px;"
            "background: transparent;");
        topLogoRow->addStretch();
        topLogoRow->addWidget(logoLabel);
        rootLayout->addLayout(topLogoRow);
    }

    rootLayout->addStretch();

    // ── Floating dock ──────────────────────────────────────────────────────
    // A semi-transparent rounded bar at the bottom of the background area.
    QWidget *dock = new QWidget();
    dock->setObjectName("FloatingDock");
    dock->setStyleSheet(
        "QWidget#FloatingDock {"
        "  background-color: rgba(9, 15, 32, 0.82);"
        "  border-radius: 12px;"
        "  border: 1px solid rgba(139, 92, 246, 0.25);"
        "}"
    );
    dock->setFixedHeight(72);

    QHBoxLayout *dockLayout = new QHBoxLayout(dock);
    dockLayout->setContentsMargins(20, 10, 20, 10);
    dockLayout->setSpacing(16);

    // Left: Extract APK button
    extractButton = new QPushButton(tr("Extract APK"));
    extractButton->setObjectName("ActionButton");
    extractButton->setFixedWidth(140);
    extractButton->setMinimumHeight(44);
    extractButton->setCursor(Qt::PointingHandCursor);
    dockLayout->addWidget(extractButton);

    dockLayout->addStretch();

    // Center: PLAY button
    playButton = new QPushButton(tr("▶  PLAY"));
    playButton->setObjectName("ActionButton");
    playButton->setFixedWidth(180);
    playButton->setMinimumHeight(44);
    playButton->setEnabled(false);
    playButton->setCursor(Qt::PointingHandCursor);
    playButton->setStyleSheet(
        "QPushButton#ActionButton {"
        "  font-size: 15px;"
        "  font-weight: bold;"
        "  letter-spacing: 1px;"
        "}"
    );
    dockLayout->addWidget(playButton);

    dockLayout->addStretch();

    // Right: version combo (roller)
    versionCombo = new QComboBox();
    versionCombo->setFixedWidth(200);
    versionCombo->setMinimumHeight(44);
    versionCombo->setCursor(Qt::PointingHandCursor);
    versionCombo->setStyleSheet(
        "QComboBox {"
        "  background-color: rgba(30, 41, 59, 0.85);"
        "  color: white;"
        "  border-radius: 8px;"
        "  border: 1px solid rgba(139, 92, 246, 0.4);"
        "  padding: 6px 12px;"
        "  font-size: 13px;"
        "}"
        "QComboBox::drop-down { border: 0px; }"
        "QComboBox QAbstractItemView {"
        "  background-color: #090f20;"
        "  selection-background-color: #8b5cf6;"
        "  color: white;"
        "  border-radius: 6px;"
        "}"
    );
    dockLayout->addWidget(versionCombo);

    // Wrap dock in a horizontal layout with margins so it floats above the bottom edge
    QHBoxLayout *dockRow = new QHBoxLayout();
    dockRow->setContentsMargins(24, 0, 24, 0);
    dockRow->addWidget(dock);
    rootLayout->addLayout(dockRow);

    // Status bar — small translucent label below the dock
    statusLabel = new QLabel(tr("Ready"));
    statusLabel->setObjectName("Status");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet(
        "QLabel#Status {"
        "  font-size: 11px;"
        "  color: rgba(148, 163, 184, 0.8);"
        "  background: transparent;"
        "  padding: 4px 0px 6px 0px;"
        "}"
    );
    rootLayout->addWidget(statusLabel);

    // Placeholder members that were used by old context panel — kept to avoid linker errors
    versionIconLabel  = new QLabel(); versionIconLabel->setVisible(false);  versionIconLabel->setParent(launcherTab);
    versionNameLabel  = new QLabel(); versionNameLabel->setVisible(false);  versionNameLabel->setParent(launcherTab);
    versionTypeLabel  = new QLabel(); versionTypeLabel->setVisible(false);  versionTypeLabel->setParent(launcherTab);
    contextPanel      = new QWidget(); contextPanel->setVisible(false);     contextPanel->setParent(launcherTab);
    shortcutButton    = new QPushButton(); shortcutButton->setVisible(false); shortcutButton->setParent(launcherTab);
    editButton        = new QPushButton(); editButton->setVisible(false);     editButton->setParent(launcherTab);
    exportButton      = new QPushButton(); exportButton->setVisible(false);   exportButton->setParent(launcherTab);
    deleteButton      = new QPushButton(); deleteButton->setVisible(false);   deleteButton->setParent(launcherTab);
    importButton      = new QPushButton(); importButton->setVisible(false);   importButton->setParent(launcherTab);

    // Add launcher page to stack
    contentStack->addWidget(launcherTab);


    // === Page 1: Gestor de Contenido (Trinito) ===
    TrinitoWindow *contentManager = new TrinitoWindow(this, this);
    contentStack->addWidget(contentManager);

    // === Page 2: Discord ===
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
    discordIcon->setStyleSheet("background: transparent;");
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
    QPushButton *discordUrlBox = new QPushButton("https://discord.gg/yBaDq2Bnuw");
    discordUrlBox->setFlat(true);
    discordUrlBox->setStyleSheet("background-color: #1e293b; color: #a78bfa; border: 1px dashed #475569; border-radius: 6px; padding: 8px; font-size: 14px; font-weight: bold; text-align: center;");
    discordUrlBox->setMinimumHeight(40);
    discordUrlBox->setMaximumWidth(300);
    discordUrlBox->setCursor(Qt::PointingHandCursor);
    discordUrlBox->setToolTip(tr("Click to copy the link"));
    discordLayout->addWidget(discordUrlBox, 0, Qt::AlignCenter);
    
    connect(discordUrlBox, &QPushButton::clicked, this, [discordUrlBox]() {
        QApplication::clipboard()->setText("https://discord.gg/yBaDq2Bnuw");
        
        discordUrlBox->setText(tr("✓ Copied!"));
        discordUrlBox->setStyleSheet("background-color: #1e293b; color: #4ade80; border: 1px dashed #4ade80; border-radius: 6px; padding: 8px; font-size: 14px; font-weight: bold; text-align: center;");
        
        QTimer::singleShot(1500, discordUrlBox, [discordUrlBox]() {
            discordUrlBox->setText("https://discord.gg/yBaDq2Bnuw");
            discordUrlBox->setStyleSheet("background-color: #1e293b; color: #a78bfa; border: 1px dashed #475569; border-radius: 6px; padding: 8px; font-size: 14px; font-weight: bold; text-align: center;");
        });
    });

    discordLayout->addSpacing(20);

    // Rich Presence toggle
    QHBoxLayout *toggleRow = new QHBoxLayout();
    toggleRow->setSpacing(12);
    QLabel *rpcLabel = new QLabel(tr("Discord Rich Presence"));
    rpcLabel->setStyleSheet("font-size: 14px; background: transparent;");
    QCheckBox *rpcToggle = new QCheckBox();
    rpcToggle->setChecked(DiscordManager::instance().isEnabled());
    rpcToggle->setStyleSheet(
        "QCheckBox::indicator { width: 22px; height: 22px; border-radius: 11px; "
        "background-color: #1e293b; border: 2px solid #334155; }"
        "QCheckBox::indicator:checked { background-color: #8b5cf6; border-color: #8b5cf6; }");
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
    contentStack->addWidget(discordPage);

    // === Page 3: About ===
    QWidget *aboutPage = new QWidget();
    QVBoxLayout *aboutLayout = new QVBoxLayout(aboutPage);
    aboutLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background: transparent;");

    QWidget *scrollContent = new QWidget();
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(40, 40, 40, 40);
    scrollLayout->setSpacing(20);

    QLabel *aboutTitle = new QLabel(tr("About Trinity Launcher"));
    aboutTitle->setObjectName("VersionName"); // Reusing style
    aboutTitle->setAlignment(Qt::AlignCenter);
    scrollLayout->addWidget(aboutTitle);

    QLabel *aboutDesc = new QLabel(tr("Trinity Launcher is an open-source, community-driven launcher for Minecraft Bedrock. "
                                      "Focused on user freedom and free redistribution, it provides a powerful interface to "
                                      "manage multiple instances, worlds, textures, and mods seamlessly."));
    aboutDesc->setWordWrap(true);
    aboutDesc->setStyleSheet("font-size: 14px; color: #cbd5e1;");
    aboutDesc->setAlignment(Qt::AlignJustify);
    scrollLayout->addWidget(aboutDesc);

    QLabel *teamTitle = new QLabel(tr("Our Team"));
    teamTitle->setObjectName("VersionName");
    teamTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #8b5cf6; margin-top: 20px;");
    scrollLayout->addWidget(teamTitle);

    QLabel *teamDesc = new QLabel(tr("Trinity is built by a dedicated group of developers, designers, and contributors:"));
    teamDesc->setWordWrap(true);
    teamDesc->setStyleSheet("font-size: 14px; color: #cbd5e1;");
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
        tr("<b>Future Contributor</b>: This spot is reserved for you. Join us!")
    };

    for (const QString &member : teamMembers) {
        QLabel *memberLabel = new QLabel(member);
        memberLabel->setTextFormat(Qt::RichText);
        memberLabel->setWordWrap(true);
        memberLabel->setStyleSheet("font-size: 14px; color: #cbd5e1; margin-left: 10px;");
        scrollLayout->addWidget(memberLabel);
    }

    QLabel *thanksTitle = new QLabel(tr("Special Thanks"));
    thanksTitle->setObjectName("VersionName");
    thanksTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #8b5cf6; margin-top: 20px;");
    scrollLayout->addWidget(thanksTitle);

    QLabel *thanksDesc = new QLabel(tr("We would like to express our sincere gratitude to the team behind the "
                                       "<b>Unofficial NIX Launcher for Minecraft</b>. Their work provides the essential runtime "
                                       "to run Minecraft, which has been fundamental to the development of this project."));
    thanksDesc->setTextFormat(Qt::RichText);
    thanksDesc->setWordWrap(true);
    thanksDesc->setStyleSheet("font-size: 14px; color: #cbd5e1;");
    thanksDesc->setAlignment(Qt::AlignJustify);
    scrollLayout->addWidget(thanksDesc);

    scrollLayout->addStretch();
    scrollArea->setWidget(scrollContent);
    aboutLayout->addWidget(scrollArea);

    contentStack->addWidget(aboutPage);

    // === Page 4: Settings ===
    contentStack->addWidget(createSettingsPage());

    contentStack->setCurrentIndex(0);

    // Helper lambda to update all sidebar button styles
    auto updateSidebar = [this](int activeIndex) {
        contentStack->setCurrentIndex(activeIndex);
        QPushButton *btns[] = {sidebarTrinityBtn, sidebarContentBtn,
                               sidebarDiscordBtn, sidebarAboutBtn,
                               sidebarSettingsBtn};
        for (int i = 0; i < 5; ++i) {
            btns[i]->setObjectName(i == activeIndex ? "SidebarBtnActive" : "SidebarBtn");
            btns[i]->style()->unpolish(btns[i]);
            btns[i]->style()->polish(btns[i]);
        }
    };

    // Sidebar button connections
    connect(sidebarTrinityBtn, &QPushButton::clicked, this, [updateSidebar]() {
        updateSidebar(0);
    });
    connect(sidebarContentBtn, &QPushButton::clicked, this, [updateSidebar]() {
        updateSidebar(1);
    });
    connect(sidebarDiscordBtn, &QPushButton::clicked, this, [updateSidebar]() {
        updateSidebar(2);
    });
    connect(sidebarAboutBtn, &QPushButton::clicked, this, [updateSidebar]() {
        updateSidebar(3);
    });
    connect(sidebarSettingsBtn, &QPushButton::clicked, this, [updateSidebar]() {
        updateSidebar(4);
    });

    // Center the window
    setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            size(),
            QGuiApplication::primaryScreen()->availableGeometry()
        )
    );
}

void LauncherWindow::setupConnections() {
    connect(extractButton, &QPushButton::clicked, this,
            &LauncherWindow::showExtractDialog);

    connect(playButton, &QPushButton::clicked, this,
            &LauncherWindow::launchGame);
    connect(versionList, &QListWidget::itemClicked, this,
            &LauncherWindow::onVersionSelected);
    connect(versionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LauncherWindow::onVersionComboChanged);
    connect(shortcutButton, &QPushButton::clicked, this,
            &LauncherWindow::createDesktopShortcut);
    // Conecta los nuevos botones
    connect(editButton, &QPushButton::clicked, this,
            &LauncherWindow::onEditConfigClicked);
    connect(exportButton, &QPushButton::clicked, this,
            &LauncherWindow::onExportClicked);
    connect(deleteButton, &QPushButton::clicked, this,
            &LauncherWindow::onDeleteClicked);
    connect(importButton, &QPushButton::clicked, this,
            &LauncherWindow::onImportClicked);
}

void LauncherWindow::loadInstalledVersions() {
    versionList->clear();
    versionCombo->clear();
    VersionManager vm;
    QStringList versions = vm.getInstalledVersions();

    for (const QString &v : versions) {
        QListWidgetItem *item = new QListWidgetItem(v);
        item->setIcon(QIcon(":/icons/cube"));
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
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
    if (versionName.isEmpty()) {
        versionNameLabel->setText(tr("No versions"));
        versionTypeLabel->setText("");
        playButton->setEnabled(false);
        statusLabel->setText(tr("No versions installed."));
        return;
    }

    versionNameLabel->setText(versionName);
    versionTypeLabel->setText(tr("Bedrock Edition")); // Placeholder type
    playButton->setEnabled(true);

    // Update status bar with size info (mockup)
    VersionManager vm;
    QString path = vm.getVersionPath(versionName);
    statusLabel->setText(QString(tr("Selected: %1 | Path: %2"))
                             .arg(versionName)
                             .arg(path));
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
    progressDlg.setWindowTitle(tr("Extracting APK..."));
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

    // Diálogo simple de edición
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Edit configuration of ") + selectedVersion);
    dialog.resize(500, 300);

    auto *layout = new QVBoxLayout(&dialog);
    QLabel *label = new QLabel(
        tr("Launch parameters (before mcpelauncher-client):"));
    layout->addWidget(label);

    // Obtener argumentos actuales
    VersionConfig config(selectedVersion);
    QString currentArgs = config.getLaunchArgs();

    QLineEdit *argsEdit = new QLineEdit(currentArgs);
    argsEdit->setPlaceholderText(
        "Ej: DRI_PRIME=1 vblank_mode=0 MESA_LOADER_DRIVER_OVERRIDE=zink");
    layout->addWidget(argsEdit);

    auto *buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, [&]() {
        QString newArgs = argsEdit->text().trimmed();
        config.setLaunchArgs(newArgs);

        // Editar versión
        VersionManager vm;
        QString errorMsg;
        if (!vm.editVersion(selectedVersion, newArgs, errorMsg)) {
            QMessageBox::critical(&dialog, "Error",
                                  tr("Could not save configuration:\n") +
                                      errorMsg);
        } else {
            QMessageBox::information(&dialog, tr("Success"),
                                     tr("Configuration saved."));
            dialog.accept();
        }
    });
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

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

bool LauncherWindow::copyDirectory(const QString &srcPath,
                                   const QString &dstPath) {
    QDir srcDir(srcPath);
    if (!srcDir.exists())
        return false;
    if (!QDir().mkpath(dstPath))
        return false;

    for (const QFileInfo &info : srcDir.entryInfoList(QDir::Dirs | QDir::Files |
                                                      QDir::NoDotAndDotDot)) {
        QString srcItem = srcPath + "/" + info.fileName();
        QString dstItem = dstPath + "/" + info.fileName();

        if (info.isDir()) {
            if (!copyDirectory(srcItem, dstItem))
                return false;
        } else {
            if (!QFile::copy(srcItem, dstItem))
                return false;
        }
    }
    return true;
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

    // Construir el comando para el .desktop
    QString execCmd = "flatpak run --command=mcpelauncher-client "
                      "com.trench.trinity.launcher -dg \"" +
                      versionPath + "\"";

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
// Settings Page
// ──────────────────────────────────────────────

void LauncherWindow::applyTheme(const QString &accent,
                                const QString &bg,
                                const QString &panel,
                                const QString &hover,
                                const QString &btnHover,
                                const QString &textMuted) {
    QString ss =
        QString(
            "QWidget { background-color: %2; color: #ffffff; "
            "font-family: 'Inter', 'Roboto', sans-serif; }"
            "QListWidget { background-color: %3; border: 1px solid %4; "
            "border-radius: 8px; padding: 5px; outline: 0; }"
            "QListWidget::item { padding: 10px; border-radius: 5px; "
            "margin-bottom: 5px; border: none; }"
            "QListWidget::item:selected { background-color: %1; color: #ffffff; }"
            "QListWidget::item:hover { background-color: %4; }"
            "QPushButton { background-color: %4; border: none; "
            "border-radius: 6px; padding: 8px 16px; color: #ffffff; "
            "font-weight: bold; }"
            "QPushButton:hover { background-color: %5; }"
            "QPushButton:pressed { background-color: %2; }"
            "QPushButton#ActionButton { background-color: %1; color: #ffffff; }"
            "QPushButton#ActionButton:hover { background-color: %1; opacity: 0.85; }"
            "QLabel#Title { font-size: 18px; font-weight: bold; color: %1; background: transparent; }"
            "QLabel#VersionName { font-size: 24px; font-weight: bold; background: transparent; }"
            "QLabel#VersionType { font-size: 14px; color: %6; background: transparent; }"
            "QLabel#Status { font-size: 12px; color: %6; padding: 5px; background: transparent; }"
            "QWidget#ContextPanel { background-color: %3; border-radius: 12px; }"
            "QWidget#Sidebar { background-color: %2; }"
            "QPushButton#SidebarBtn { background: transparent; border: none; "
            "border-left: 3px solid transparent; border-radius: 0px; padding: 14px; }"
            "QPushButton#SidebarBtn:hover { background: %5; }"
            "QPushButton#SidebarBtnActive { background: transparent; border: none; "
            "border-left: 3px solid %1; border-radius: 0px; padding: 14px; }"
            "QTabWidget::pane { border: 1px solid %4; background-color: %3; border-radius: 8px; top: -1px; }"
            "QTabBar::tab { background: %4; color: %6; padding: 10px 20px; "
            "border-top-left-radius: 6px; border-top-right-radius: 6px; margin-right: 4px; border: none; }"
            "QTabBar::tab:selected { background: %1; color: #ffffff; }"
            "QTabBar::tab:hover { background: %5; }"
        )
        .arg(accent)    // %1
        .arg(bg)        // %2
        .arg(panel)     // %3
        .arg(hover)     // %4
        .arg(btnHover)  // %5
        .arg(textMuted);// %6

    qApp->setStyleSheet(ss);

    // Persistir
    QSettings settings;
    settings.setValue("theme/accent",    accent);
    settings.setValue("theme/bg",        bg);
    settings.setValue("theme/panel",     panel);
    settings.setValue("theme/hover",     hover);
    settings.setValue("theme/btnHover",  btnHover);
    settings.setValue("theme/textMuted", textMuted);
}

QWidget *LauncherWindow::createSettingsPage() {
    // Defaults
    const QString DEF_ACCENT    = "#8b5cf6";
    const QString DEF_BG        = "#020617";
    const QString DEF_PANEL     = "#090f20";
    const QString DEF_HOVER     = "#1e293b";
    const QString DEF_BTNHOVER  = "#334155";
    const QString DEF_TEXTMUTED = "#94a3b8";

    QSettings cfg;
    QString accent    = cfg.value("theme/accent",    DEF_ACCENT).toString();
    QString bg        = cfg.value("theme/bg",        DEF_BG).toString();
    QString panel     = cfg.value("theme/panel",     DEF_PANEL).toString();
    QString hover     = cfg.value("theme/hover",     DEF_HOVER).toString();
    QString btnHover  = cfg.value("theme/btnHover",  DEF_BTNHOVER).toString();
    QString textMuted = cfg.value("theme/textMuted", DEF_TEXTMUTED).toString();

    auto *page = new QWidget();
    auto *outerLayout = new QVBoxLayout(page);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    auto *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background: transparent;");

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
    langSection->setStyleSheet("font-size: 16px; font-weight: bold; color: #94a3b8;");
    layout->addWidget(langSection);

    auto *langSeparator = new QFrame();
    langSeparator->setFrameShape(QFrame::HLine);
    langSeparator->setStyleSheet("color: #1e293b;");
    layout->addWidget(langSeparator);

    {
        auto *langRow = new QHBoxLayout();
        auto *langLabel = new QLabel(tr("Interface language:"));
        langLabel->setStyleSheet("font-size: 14px;");
        langLabel->setMinimumWidth(180);

        settingsLanguageCombo = new QComboBox();
        settingsLanguageCombo->setFixedWidth(180);
        settingsLanguageCombo->setStyleSheet(
            "QComboBox { background-color: #1e293b; color: white; border-radius: "
            "6px; padding: 6px 10px; font-size: 13px; }"
            "QComboBox::drop-down { border: 0px; }"
            "QComboBox QAbstractItemView { background-color: #090f20; "
            "selection-background-color: #8b5cf6; color: white; }");

        settingsLanguageCombo->addItem("Espa\u00f1ol", "es");
        settingsLanguageCombo->addItem("Português (Brasil)", "pt_BR");

        QDir translationsDir(":/i18n");
        QStringList langFiles =
            translationsDir.entryList(QStringList() << "trinity_*.qm", QDir::Files);

        for (const QString &file : langFiles) {
            if (file.length() <= 11) continue;
            QString code = file.mid(8, file.length() - 11);
            if (code == "es") continue;
            if (code == "pt_BR") continue;
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
    colorSection->setStyleSheet("font-size: 16px; font-weight: bold; color: #94a3b8;");
    layout->addWidget(colorSection);

    auto *colorsSeparator = new QFrame();
    colorsSeparator->setFrameShape(QFrame::HLine);
    colorsSeparator->setStyleSheet("color: #1e293b;");
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

    auto makeColorRow = [&](const QString &labelText, QString *colorRef,
                            const QString &settingKey) {
        auto *row = new QHBoxLayout();
        auto *lbl = new QLabel(labelText);
        lbl->setStyleSheet("font-size: 14px;");
        lbl->setMinimumWidth(180);

        auto *preview = new QPushButton();
        preview->setFixedSize(36, 36);
        preview->setStyleSheet(
            QString("background-color: %1; border-radius: 6px; border: 2px solid #334155;")
                .arg(*colorRef));
        preview->setCursor(Qt::PointingHandCursor);
        preview->setToolTip(tr("Click to change color"));

        auto *hexLabel = new QLabel(*colorRef);
        hexLabel->setStyleSheet("font-size: 12px; color: #64748b; font-family: monospace;");
        hexLabel->setMinimumWidth(80);

        connect(preview, &QPushButton::clicked, this,
                [this, preview, hexLabel, colorRef,
                 accentVal, bgVal, panelVal, hoverVal, btnHoverVal, textMutedVal]() {
                    QColor initial(*colorRef);
                    QColor chosen = QColorDialog::getColor(initial, this, tr("Select Color"));
                    if (!chosen.isValid()) return;
                    *colorRef = chosen.name();
                    preview->setStyleSheet(
                        QString("background-color: %1; border-radius: 6px; border: 2px solid #334155;")
                            .arg(*colorRef));
                    hexLabel->setText(*colorRef);
                    applyTheme(*accentVal, *bgVal, *panelVal, *hoverVal, *btnHoverVal, *textMutedVal);
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


    // Botón Reset de colores
    auto *resetColorsBtn = new QPushButton(tr("Reset Colors to Default"));
    resetColorsBtn->setObjectName("ActionButton");
    connect(resetColorsBtn, &QPushButton::clicked, this,
            [this, accentVal, bgVal, panelVal, hoverVal, btnHoverVal, textMutedVal,
             DEF_ACCENT, DEF_BG, DEF_PANEL, DEF_HOVER, DEF_BTNHOVER, DEF_TEXTMUTED]() {
                *accentVal    = DEF_ACCENT;
                *bgVal        = DEF_BG;
                *panelVal     = DEF_PANEL;
                *hoverVal     = DEF_HOVER;
                *btnHoverVal  = DEF_BTNHOVER;
                *textMutedVal = DEF_TEXTMUTED;
                applyTheme(DEF_ACCENT, DEF_BG, DEF_PANEL, DEF_HOVER, DEF_BTNHOVER, DEF_TEXTMUTED);
                QMessageBox::information(this, tr("Settings"),
                    tr("Colors reset to default. Reopen Settings to see the updated previews."));
            });
    auto *resetColRow = new QHBoxLayout();
    resetColRow->addWidget(resetColorsBtn);
    resetColRow->addStretch();
    layout->addLayout(resetColRow);

    layout->addSpacing(12);

    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    // SECCIÓN: Wallpaper / Background
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    auto *wallpaperSection = new QLabel(tr("Wallpaper"));
    wallpaperSection->setStyleSheet("font-size: 16px; font-weight: bold; color: #94a3b8;");
    layout->addWidget(wallpaperSection);

    auto *wallpaperSep = new QFrame();
    wallpaperSep->setFrameShape(QFrame::HLine);
    wallpaperSep->setStyleSheet("color: #1e293b;");
    layout->addWidget(wallpaperSep);

    {
        auto *wpRow = new QHBoxLayout();

        // Current background preview
        auto *wpPreview = new QLabel();
        wpPreview->setFixedSize(120, 68);
        wpPreview->setAlignment(Qt::AlignCenter);
        wpPreview->setStyleSheet(
            "border-radius: 8px; border: 2px solid #334155; background: #0f172a;");
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
        wpPathLabel->setStyleSheet("font-size: 12px; color: #94a3b8;");
        wpPathLabel->setWordWrap(true);
        wpInfoLayout->addWidget(wpPathLabel);

        auto *wpBtnRow = new QHBoxLayout();

        auto *wpChangeBtn = new QPushButton(tr("Change..."));
        wpChangeBtn->setObjectName("ActionButton");
        wpChangeBtn->setCursor(Qt::PointingHandCursor);

        auto *wpResetBtn = new QPushButton(tr("Reset"));
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

        // Change button: pick image file, save path, update preview & home tab
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

                // Apply immediately: find the LauncherTab widget and update its style
                QWidget *launcherTab = contentStack->widget(0);
                if (launcherTab) {
                    launcherTab->setStyleSheet(
                        QString("QWidget#LauncherTab {"
                                "  border-image: url(\"%1\") 0 0 0 0 stretch stretch;"
                                "}").arg(path));
                }
            });

        // Reset button: clear saved path, revert to built-in background
        connect(wpResetBtn, &QPushButton::clicked, this,
            [this, wpPreview, wpPathLabel]() {
                QSettings bgCfg;
                bgCfg.remove("background/path");
                bgCfg.sync();

                wpPreview->setPixmap(QPixmap(":/branding/background").scaled(120, 68, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
                wpPathLabel->setText(tr("Default background"));

                QWidget *launcherTab = contentStack->widget(0);
                if (launcherTab) {
                    launcherTab->setStyleSheet(
                        "QWidget#LauncherTab {"
                        "  border-image: url(:/branding/background) 0 0 0 0 stretch stretch;"
                        "}");
                }
            });
    }

    layout->addSpacing(12);

    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    // SECCIÓN: Iconos del sidebar
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    auto *iconSection = new QLabel(tr("Sidebar Icons"));
    iconSection->setStyleSheet("font-size: 16px; font-weight: bold; color: #94a3b8;");
    layout->addWidget(iconSection);

    auto *iconSeparator = new QFrame();
    iconSeparator->setFrameShape(QFrame::HLine);
    iconSeparator->setStyleSheet("color: #1e293b;");
    layout->addWidget(iconSeparator);

    auto *iconNote = new QLabel(tr("You can customize the sidebar icons. The app logo is fixed and cannot be changed."));
    iconNote->setWordWrap(true);
    iconNote->setStyleSheet("font-size: 12px; color: #64748b;");
    layout->addWidget(iconNote);

    // Datos de iconos cambiables
    struct IconEntry {
        QString name;        // Nombre legible
        QString settingKey;  // Clave en QSettings
        QString defaultRes;  // Recurso por defecto (:/icons/...)
        QPushButton *btn;    // Botón del sidebar a actualizar
    };

    QList<IconEntry> icons = {
        { tr("Trinity (Home)"),   "icon/trinity",  ":/icons/cube-w",  sidebarTrinityBtn },
        { tr("Content Manager"),  "icon/content",  ":/icons/config",  sidebarContentBtn },
        { tr("Discord"),          "icon/discord",  ":/icons/discord", sidebarDiscordBtn },
        { tr("About"),            "icon/about",    ":/icons/heart",   sidebarAboutBtn   },
        { tr("Settings"),         "icon/settings", ":/icons/settings",sidebarSettingsBtn},
    };

    for (const auto &entry : icons) {
        auto *row = new QHBoxLayout();

        // Preview del icono actual
        QSettings icfg;
        QString customPath = icfg.value(entry.settingKey, "").toString();
        QIcon currentIcon = customPath.isEmpty()
            ? QIcon(entry.defaultRes)
            : QIcon(customPath);

        auto *iconPreview = new QLabel();
        iconPreview->setFixedSize(36, 36);
        iconPreview->setPixmap(currentIcon.pixmap(32, 32));
        iconPreview->setStyleSheet("background: #1e293b; border-radius: 6px; padding: 2px;");
        iconPreview->setAlignment(Qt::AlignCenter);

        auto *nameLbl = new QLabel(entry.name);
        nameLbl->setStyleSheet("font-size: 14px;");
        nameLbl->setMinimumWidth(180);

        auto *changeBtn = new QPushButton(tr("Change..."));
        changeBtn->setCursor(Qt::PointingHandCursor);

        // Captura por valor para la lambda
        QPushButton *sideBtn = entry.btn;
        QString settingKey   = entry.settingKey;
        QString defaultRes   = entry.defaultRes;

        connect(changeBtn, &QPushButton::clicked, this,
                [this, iconPreview, sideBtn, settingKey]() {
                    QString path = QFileDialog::getOpenFileName(
                        this, tr("Select Icon"), QDir::homePath(),
                        tr("Images (*.png *.svg *.ico *.jpg);;All files (*)"));
                    if (path.isEmpty()) return;

                    // Copiar al directorio de config del usuario
                    QString configDir = QStandardPaths::writableLocation(
                                            QStandardPaths::AppConfigLocation)
                                        + "/icons";
                    QDir().mkpath(configDir);
                    QFileInfo fi(path);
                    QString dest = configDir + "/" + fi.fileName();
                    if (QFile::exists(dest)) QFile::remove(dest);
                    QFile::copy(path, dest);

                    QSettings icfg;
                    icfg.setValue(settingKey, dest);

                    QIcon newIcon(dest);
                    iconPreview->setPixmap(newIcon.pixmap(32, 32));
                    sideBtn->setIcon(newIcon);
                });

        row->addWidget(iconPreview);
        row->addWidget(nameLbl);
        row->addStretch();
        row->addWidget(changeBtn);
        layout->addLayout(row);
    }

    layout->addSpacing(8);

    // Botón Reset de iconos
    auto *resetIconsBtn = new QPushButton(tr("Reset Icons to Default"));
    resetIconsBtn->setObjectName("ActionButton");
    connect(resetIconsBtn, &QPushButton::clicked, this,
            [this, icons]() {
                QSettings icfg;
                for (const auto &entry : icons) {
                    icfg.remove(entry.settingKey);
                    entry.btn->setIcon(QIcon(entry.defaultRes));
                }
                QMessageBox::information(this, tr("Settings"),
                    tr("Icons reset to default. Reopen Settings to see the updated previews."));
            });
    auto *resetIconRow = new QHBoxLayout();
    resetIconRow->addWidget(resetIconsBtn);
    resetIconRow->addStretch();
    layout->addLayout(resetIconRow);

    layout->addStretch();
    scrollArea->setWidget(content);
    outerLayout->addWidget(scrollArea);

    return page;
}

