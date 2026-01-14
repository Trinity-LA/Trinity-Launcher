#include "TrinityLib/ui/windows/launcher_window.hpp"
#include "TrinityLib/core//exporter.hpp"
#include "TrinityLib/core/game_launcher.hpp"
#include "TrinityLib/core/version_config.hpp"
#include "TrinityLib/core/version_manager.hpp"
#include "TrinityLib/ui/dialogs/extract_dialog.hpp"

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QProcess>
#include <QProgressBar>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include <QVBoxLayout>

LauncherWindow::LauncherWindow(QWidget *parent)
    : QWidget(parent) {
    setupUi();
    setupConnections();
    loadInstalledVersions();
    exporter = new Exporter(this);

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
    setWindowTitle("Trinity Launcher - Minecraft Bedrock");
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
        "border-radius: 12px; }");

    // Main Vertical Layout (Top Bar, Content, Status Bar)
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
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

    QLabel *titleLabel = new QLabel("Trinity Launcher");
    titleLabel->setObjectName("Title");
    topBarLayout->addWidget(titleLabel);

    topBarLayout->addStretch();

    // --- Switcher languages
    languageCombo = new QComboBox();
    languageCombo->addItem("Español", "es"); // Index 0, data "es"
    languageCombo->addItem("English", "en"); // Index 1, data "en"
    languageCombo->addItem("Català", "ca"); // Index 2, data "ca"
    languageCombo->setFixedWidth(120);

    QSettings settings;
    QString currentLang = settings.value("language", "es").toString();
    int index = languageCombo->findData(currentLang);
    if (index != -1) {
        languageCombo->blockSignals(true);
        languageCombo->setCurrentIndex(index);
        languageCombo->blockSignals(false);
    }

    languageCombo->setStyleSheet(
        "QComboBox { background-color: #1e293b; color: white; border-radius: "
        "5px; padding: 5px; }"
        "QComboBox::drop-down { border: 0px; }");

    topBarLayout->addWidget(languageCombo);

    extractButton = new QPushButton(tr("+ Extraer APK"));
    extractButton->setObjectName("ActionButton"); // Accent color
    topBarLayout->addWidget(extractButton);

    importButton = new QPushButton(tr("Importar")); // Import button
    importButton->setObjectName("ActionButton");
    topBarLayout->addWidget(importButton);

    toolsButton = new QPushButton(tr("Herramientas"));
    toolsButton->setObjectName("ActionButton"); // Apply accent style
    topBarLayout->addWidget(toolsButton);

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
    versionNameLabel = new QLabel(tr("Selecciona una versión"));
    versionNameLabel->setObjectName("VersionName");
    versionNameLabel->setAlignment(Qt::AlignCenter);
    panelLayout->addWidget(versionNameLabel);

    versionTypeLabel = new QLabel("");
    versionTypeLabel->setObjectName("VersionType");
    versionTypeLabel->setAlignment(Qt::AlignCenter);
    panelLayout->addWidget(versionTypeLabel);

    panelLayout->addSpacing(20);

    // Actions
    playButton = new QPushButton(tr("JUGAR"));
    playButton->setObjectName("ActionButton");
    playButton->setMinimumHeight(35);
    playButton->setEnabled(false);
    panelLayout->addWidget(playButton);

    // Botón "Crear Acceso Directo" (debajo de JUGAR)
    shortcutButton = new QPushButton(tr("Crear Acceso Directo"));
    shortcutButton->setObjectName("ActionButton");
    shortcutButton->setMinimumHeight(35);
    panelLayout->addWidget(shortcutButton);

    editButton = new QPushButton(tr("Editar Configuración"));
    editButton->setObjectName("ActionButton");
    panelLayout->addWidget(editButton);

    QHBoxLayout *secondaryActions = new QHBoxLayout();
    exportButton = new QPushButton(tr("Exportar"));
    exportButton->setObjectName("ActionButton");
    deleteButton = new QPushButton(tr("Eliminar"));
    deleteButton->setObjectName("ActionButton");
    secondaryActions->addWidget(exportButton);
    secondaryActions->addWidget(deleteButton);
    panelLayout->addLayout(secondaryActions);

    panelLayout->addStretch();

    contentLayout->addWidget(contextPanel);
    rootLayout->addLayout(contentLayout);

    // --- Status Bar ---
    statusLabel = new QLabel(tr("Listo"));
    statusLabel->setObjectName("Status");
    rootLayout->addWidget(statusLabel);
}

void LauncherWindow::setupConnections() {
    connect(extractButton, &QPushButton::clicked, this,
            &LauncherWindow::showExtractDialog);
    connect(toolsButton, &QPushButton::clicked, this,
            &LauncherWindow::launchTools);
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
    versionTypeLabel->setText("Bedrock Edition"); // Placeholder type
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

void LauncherWindow::onEditConfigClicked() {
    if (versionList->selectedItems().isEmpty()) {
        QMessageBox::warning(this, tr("Advertencia"),
                             tr("No hay ninguna versión seleccionada."));
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
                             tr("No hay ninguna versión seleccionada."));
        return;
    }
    QString selectedVersion = versionList->selectedItems().first()->text();

    exporter->exportVersion(selectedVersion);
}

void LauncherWindow::onDeleteClicked() {
    if (versionList->selectedItems().isEmpty()) {
        QMessageBox::warning(this, tr("Advertencia"),
                             tr("No hay ninguna versión seleccionada."));
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
                             tr("No hay ninguna versión seleccionada."));
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
        this, 
        tr("Se necesita reiniciar"),
        tr("El idioma cambiará a '%1'.\n¿Deseas reiniciar la aplicación ahora para aplicar los cambios?")
            .arg(languageCombo->currentText()),
        QMessageBox::Yes | QMessageBox::No, 
        QMessageBox::Yes
    );

    if (r == QMessageBox::Yes) {
        QString program = QCoreApplication::applicationFilePath();
        QStringList arguments = QCoreApplication::arguments();

        QStringList args = arguments.mid(1);

        QProcess::startDetached(program, args);

        QApplication::quit();
    }       

}
