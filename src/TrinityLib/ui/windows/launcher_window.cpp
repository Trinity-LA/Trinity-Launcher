#include "TrinityLib/ui/windows/launcher_window.hpp"
#include "TrinityLib/ui/windows/trinito_window.hpp"
#include "TrinityLib/core/discord_manager.hpp"
#include "TrinityLib/core/version_config.hpp"
#include "TrinityLib/core/version_manager.hpp"
#include "TrinityLib/ui/dialogs/extract_dialog.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
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
}

void LauncherWindow::setupUi() {
    setWindowTitle(tr("Trinity Launcher - Minecraft Bedrock"));
    resize(960, 560);
    setMinimumSize(960, 560); // Tamaño mínimo

    // Global Dark Theme
    setStyleSheet(
        "QWidget { background-color: #020617; color: #ffffff; "
        "font-family: 'Inter', 'Roboto', sans-serif; }"
        "QListWidget { background-color: #090f20; border: 1px solid "
        "#1e293b; border-radius: 8px; padding: 5px; outline: 0; }"
        "QListWidget::item { padding: 10px; border-radius: 5px; "
        "margin-bottom: 5px; border: none; }"
        "QListWidget::item:selected { background-color: #8b5cf6; "
        "color: #ffffff; }"
        "QListWidget::item:hover { background-color: #1e293b; }"
        "QPushButton { background-color: #1e293b; border: none; "
        "border-radius: 6px; padding: 8px 16px; color: #ffffff; "
        "font-weight: bold; }"
        "QPushButton:hover { background-color: #334155; }"
        "QPushButton:pressed { background-color: #0f172a; }"
        "QPushButton#ActionButton { background-color: #8b5cf6; color: "
        "#ffffff; }" // Violet accent
        "QPushButton#ActionButton:hover { background-color: #a78bfa; }"
        "QLabel#Title { font-size: 18px; font-weight: bold; color: "
        "#8b5cf6; background: transparent; }"
        "QLabel#VersionName { font-size: 24px; font-weight: bold; "
        "background: transparent; }"
        "QLabel#VersionType { font-size: 14px; color: #94a3b8; "
        "background: transparent; }"
        "QLabel#Status { font-size: 12px; color: #64748b; padding: "
        "5px; background: transparent; }"
        "QWidget#ContextPanel { background-color: #090f20; "
        "border-radius: 12px; }"
        "QWidget#Sidebar { background-color: #020617; }"
        "QPushButton#SidebarBtn { background: transparent; border: none; "
        "border-left: 3px solid transparent; border-radius: 0px; "
        "padding: 14px; }"
        "QPushButton#SidebarBtn:hover { background: #0a0f1f; }"
        "QPushButton#SidebarBtnActive { background: transparent; border: none; "
        "border-left: 3px solid #8b5cf6; border-radius: 0px; "
        "padding: 14px; }");

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

    sidebarLayout->addWidget(sidebarTrinityBtn);
    sidebarLayout->addWidget(sidebarContentBtn);
    sidebarLayout->addWidget(sidebarDiscordBtn);
    sidebarLayout->addWidget(sidebarAboutBtn);
    sidebarLayout->addStretch();
    windowLayout->addWidget(sidebar);

    // --- Vertical divider ---
    QFrame *divider = new QFrame();
    divider->setFrameShape(QFrame::VLine);
    divider->setStyleSheet("color: #1e293b; background-color: #1e293b; max-width: 1px;");
    windowLayout->addWidget(divider);

    // --- Content stack ---
    contentStack = new QStackedWidget();
    windowLayout->addWidget(contentStack);

    // === Page 0: Trinity (Launcher) ===
    QWidget *launcherTab = new QWidget();
    QVBoxLayout *rootLayout = new QVBoxLayout(launcherTab);
    rootLayout->setContentsMargins(20, 20, 20, 10);
    rootLayout->setSpacing(15);

    // --- Top Bar ---
    QHBoxLayout *topBarLayout = new QHBoxLayout();

    QLabel *logoLabel = new QLabel();
    logoLabel->setFixedSize(40, 40);
    // Use border-image to ensure the image respects the border-radius
    logoLabel->setStyleSheet(
        "border-image: url(:/branding/logo); border-radius: 60px;");
    topBarLayout->addWidget(logoLabel);

    QLabel *titleLabel = new QLabel(tr("Trinity Launcher"));
    titleLabel->setObjectName("Title");
    topBarLayout->addWidget(titleLabel);

    topBarLayout->addStretch();

    languageCombo = new QComboBox();
    languageCombo->setFixedWidth(120);
    languageCombo->setStyleSheet(
        "QComboBox { background-color: #1e293b; color: white; border-radius: "
        "5px; padding: 5px; }"
        "QComboBox::drop-down { border: 0px; }");

    languageCombo->addItem("Español", "es");

    QDir translationsDir(":/i18n");
    QStringList fileNames =
        translationsDir.entryList(QStringList() << "trinity_*.qm", QDir::Files);

    for (const QString &file : fileNames) {
        if (file.length() <= 11)
            continue;

        QString code = file.mid(8, file.length() - 11);

        if (code == "es")
            continue;

        QLocale loc(code);
        QString nativeName = loc.nativeLanguageName();

        if (!nativeName.isEmpty()) {
            nativeName[0] = nativeName[0].toUpper();
        } else {
            nativeName = code;
        }

        languageCombo->addItem(nativeName, code);
    }

    QSettings settings;
    // Default to system language if not set, fallback to 'es'
    QString systemLang = QLocale::system().name().split('_').first();
    if (!QFile::exists(":/i18n/trinity_" + systemLang + ".qm") &&
        systemLang != "es") {
        systemLang = "es";
    }

    QString currentLang = settings.value("language", systemLang).toString();
    int index = languageCombo->findData(currentLang);
    if (index != -1) {
        languageCombo->setCurrentIndex(index);
    }

    topBarLayout->addWidget(languageCombo);

    extractButton = new QPushButton(tr("+ Extract APK"));
    extractButton->setObjectName("ActionButton"); // Accent color
    topBarLayout->addWidget(extractButton);

    importButton = new QPushButton(tr("Import")); // Import button
    importButton->setObjectName("ActionButton");
    topBarLayout->addWidget(importButton);

    rootLayout->addLayout(topBarLayout);

    // --- Content Area (Split View) ---
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(20);

    // Left: Version List
    versionList = new QListWidget();
    versionList->setIconSize(QSize(48, 48));
    versionList->setFixedWidth(400);
    contentLayout->addWidget(versionList);

    // Right: Context Panel
    contextPanel = new QWidget();
    contextPanel->setObjectName("ContextPanel");
    QVBoxLayout *panelLayout = new QVBoxLayout(contextPanel);
    panelLayout->setContentsMargins(30, 30, 30, 30);
    panelLayout->setSpacing(15);

    // Version Icon (Large)
    versionIconLabel = new QLabel();
    versionIconLabel->setFixedSize(80, 80);
    versionIconLabel->setPixmap(QPixmap(":/icons/creeper"));
    versionIconLabel->setScaledContents(true);
    versionIconLabel->setStyleSheet("background: transparent;");
    versionIconLabel->setAlignment(Qt::AlignCenter);
    panelLayout->addWidget(versionIconLabel, 0, Qt::AlignCenter);

    // Version Info
    versionNameLabel = new QLabel(tr("Select a version"));
    versionNameLabel->setObjectName("VersionName");
    versionNameLabel->setAlignment(Qt::AlignCenter);
    panelLayout->addWidget(versionNameLabel);

    versionTypeLabel = new QLabel("");
    versionTypeLabel->setObjectName("VersionType");
    versionTypeLabel->setAlignment(Qt::AlignCenter);
    panelLayout->addWidget(versionTypeLabel);

    panelLayout->addSpacing(20);

    // Actions
    playButton = new QPushButton(tr("PLAY"));
    playButton->setObjectName("ActionButton");
    playButton->setMinimumHeight(35);
    playButton->setEnabled(false);
    panelLayout->addWidget(playButton);

    // Botón "Crear Acceso Directo" (debajo de JUGAR)
    shortcutButton = new QPushButton(tr("Create Shortcut"));
    shortcutButton->setObjectName("ActionButton");
    shortcutButton->setMinimumHeight(35);
    panelLayout->addWidget(shortcutButton);

    editButton = new QPushButton(tr("Edit Config"));
    editButton->setObjectName("ActionButton");
    panelLayout->addWidget(editButton);

    QHBoxLayout *secondaryActions = new QHBoxLayout();
    exportButton = new QPushButton(tr("Export"));
    exportButton->setObjectName("ActionButton");
    deleteButton = new QPushButton(tr("Delete"));
    deleteButton->setObjectName("ActionButton");
    secondaryActions->addWidget(exportButton);
    secondaryActions->addWidget(deleteButton);
    panelLayout->addLayout(secondaryActions);

    panelLayout->addStretch();

    contentLayout->addWidget(contextPanel);
    rootLayout->addLayout(contentLayout);

    // --- Status Bar ---
    statusLabel = new QLabel(tr("Ready"));
    statusLabel->setObjectName("Status");
    rootLayout->addWidget(statusLabel);

    // Add launcher page to stack
    contentStack->addWidget(launcherTab);

    // === Page 1: Gestor de Contenido (Trinito) ===
    TrinitoWindow *contentManager = new TrinitoWindow(this);
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
    discordUrlBox->setToolTip(tr("Haz clic para copiar el enlace"));
    discordLayout->addWidget(discordUrlBox, 0, Qt::AlignCenter);
    
    connect(discordUrlBox, &QPushButton::clicked, this, [discordUrlBox]() {
        QApplication::clipboard()->setText("https://discord.gg/yBaDq2Bnuw");
        
        discordUrlBox->setText(tr("✓ Copiado!"));
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
                tr("Cierra y vuelve a abrir el launcher para que se aplique la configuración."));
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

    contentStack->setCurrentIndex(0);

    // Helper lambda to update all sidebar button styles
    auto updateSidebar = [this](int activeIndex) {
        contentStack->setCurrentIndex(activeIndex);
        QPushButton *btns[] = {sidebarTrinityBtn, sidebarContentBtn, sidebarDiscordBtn, sidebarAboutBtn};
        for (int i = 0; i < 4; ++i) {
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
    connect(importButton, &QPushButton::clicked, this,
            &LauncherWindow::onImportClicked);
    connect(shortcutButton, &QPushButton::clicked, this,
            &LauncherWindow::createDesktopShortcut);
    // Conecta los nuevos botones
    connect(editButton, &QPushButton::clicked, this,
            &LauncherWindow::onEditConfigClicked);
    connect(exportButton, &QPushButton::clicked, this,
            &LauncherWindow::onExportClicked);
    connect(deleteButton, &QPushButton::clicked, this,
            &LauncherWindow::onDeleteClicked);
    connect(languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LauncherWindow::onLanguageChanged);
}

void LauncherWindow::loadInstalledVersions() {
    versionList->clear();
    VersionManager vm;
    QStringList versions = vm.getInstalledVersions();

    for (const QString &v : versions) {
        QListWidgetItem *item = new QListWidgetItem(v);
        // Try to set an icon if available, otherwise use a default style
        item->setIcon(QIcon(":/icons/cube"));
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        versionList->addItem(item);
    }

    if (versionList->count() > 0) {
        versionList->setCurrentRow(0);
        onVersionSelected(versionList->item(0));
    } else {
        updateContextPanel("");
    }
}

void LauncherWindow::onVersionSelected(QListWidgetItem *item) {
    if (!item)
        return;
    updateContextPanel(item->text());
}

void LauncherWindow::updateContextPanel(const QString &versionName) {
    if (versionName.isEmpty()) {
        versionNameLabel->setText(tr("Sin versiones"));
        versionTypeLabel->setText("");
        playButton->setEnabled(false);
        statusLabel->setText(tr("No hay versiones instaladas."));
        return;
    }

    versionNameLabel->setText(versionName);
    versionTypeLabel->setText(tr("Bedrock Edition")); // Placeholder type
    playButton->setEnabled(true);

    // Update status bar with size info (mockup)
    VersionManager vm;
    QString path = vm.getVersionPath(versionName);
    statusLabel->setText(QString(tr("Versión seleccionada: %1 | Ruta: %2"))
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
            this, tr("Advertencia"),
            QString(tr("Ya existe una versión llamada '%1'.\n¿Reemplazarla?"))
                .arg(versionName),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (r == QMessageBox::No)
            return;
    }

    // Crear diálogo de progreso
    QDialog progressDlg(this);
    progressDlg.setWindowTitle(tr("Extrayendo APK..."));
    progressDlg.setFixedSize(300, 100);

    auto *layout = new QVBoxLayout(&progressDlg);
    QLabel *label = new QLabel(tr("Extrayendo versión..."));
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
                              tr("Falló la extracción:\n") + errorMsg);
        return;
    }

    QMessageBox::information(this, tr("Éxito"),
                             tr("¡Versión extraída correctamente!"));
    loadInstalledVersions(); // Recargar lista.
}

void LauncherWindow::launchGame() {
    if (versionList->selectedItems().isEmpty())
        return;
    QString selectedVersion = versionList->selectedItems().first()->text();

    QString errorMsg;

    if (!m_gameLauncher->launchGame(selectedVersion, errorMsg)) {
        QMessageBox::critical(this, "Error", errorMsg);
        return;
    }
    // QApplication::quit();
    this->hide();
}



void LauncherWindow::onEditConfigClicked() {
    if (versionList->selectedItems().isEmpty()) {
        QMessageBox::warning(this, tr("Advertencia"),
                             tr("No hay ningún versión seleccionada."));
        return;
    }
    QString selectedVersion = versionList->selectedItems().first()->text();

    // Diálogo simple de edición
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Editar configuración de ") + selectedVersion);
    dialog.resize(500, 300);

    auto *layout = new QVBoxLayout(&dialog);
    QLabel *label = new QLabel(
        tr("Parámetros de ejecución (antes de mcpelauncher-client):"));
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
                                  tr("No se pudo guardar la configuración:\n") +
                                      errorMsg);
        } else {
            QMessageBox::information(&dialog, tr("Éxito"),
                                     tr("Configuración guardada."));
            dialog.accept();
        }
    });
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        statusLabel->setText(QString(tr("Configuración de %1 actualizada."))
                                 .arg(selectedVersion));
    }
}

void LauncherWindow::onExportClicked() {
    if (versionList->selectedItems().isEmpty()) {
        QMessageBox::warning(this, tr("Advertencia"),
                             tr("No hay ningún versión seleccionada."));
        return;
    }
    QString selectedVersion = versionList->selectedItems().first()->text();

    exporter->exportVersion(selectedVersion);
}

void LauncherWindow::onDeleteClicked() {
    if (versionList->selectedItems().isEmpty()) {
        QMessageBox::warning(this, tr("Advertencia"),
                             tr("No hay ningún versión seleccionada."));
        return;
    }
    QString selectedVersion = versionList->selectedItems().first()->text();

    int r = QMessageBox::warning(
        this, tr("Advertencia"),
        QString(tr("¿Estás seguro de eliminar la versión '%1'?\nEsta acción no "
                   "se puede deshacer.")
                    .arg(selectedVersion)),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (r == QMessageBox::No)
        return;

    // Eliminar versión
    VersionManager vm;
    QString errorMsg;
    if (!vm.deleteVersion(selectedVersion, errorMsg)) {
        QMessageBox::critical(this, tr("Error"),
                              tr("No se pudo eliminar la versión:\n") +
                                  errorMsg);
        return;
    }

    QMessageBox::information(this, tr("Éxito"),
                             tr("Versión eliminada correctamente."));
    loadInstalledVersions(); // Recargar lista
    statusLabel->setText(
        QString(tr("Versión %1 eliminada.")).arg(selectedVersion));
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

void LauncherWindow::onImportClicked() { exporter->importVersion(); }

void LauncherWindow::createDesktopShortcut() {
    if (versionList->selectedItems().isEmpty()) {
        QMessageBox::warning(this, tr("Advertencia"),
                             tr("No hay ningún versión seleccionada."));
        return;
    }

    QString selectedVersion = versionList->selectedItems().first()->text();

    // Obtener la ruta de la versión
    VersionManager vm;
    QString versionPath = vm.getVersionPath(selectedVersion);

    if (!vm.isVersionValid(selectedVersion)) {
        QMessageBox::critical(
            this, "Error",
            QString(tr("La versión '%1' no es válida o no está completa."))
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
            this, tr("Confirmar"),
            QString(
                tr("Ya existe un acceso directo para '%1'.\n¿Reemplazarlo?"))
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
        this, tr("Éxito"),
        QString(tr("Acceso directo creado en la carpeta Descargas")
                    .arg(shortcutPath)));
}

void LauncherWindow::onLanguageChanged(int index) {
    QString langCode = languageCombo->itemData(index).toString();

    QSettings settings;
    settings.setValue("language", langCode);
    settings.sync();

    int r = QMessageBox::question(
        this, tr("Se necesita reiniciar"),
        tr("El idioma cambiará a '%1'.\n¿Deseas reiniciar la aplicación ahora "
           "para aplicar los cambios?")
            .arg(languageCombo->currentText()),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (r == QMessageBox::Yes) {
        QString program = QCoreApplication::applicationFilePath();
        QStringList arguments = QCoreApplication::arguments();
        QStringList args = arguments.mid(1);
        QProcess::startDetached(program, args);
        QApplication::quit();
    }
}
