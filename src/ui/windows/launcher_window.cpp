#include "launcher_window.h"
#include "../../core/game_launcher.h"
#include "../../core/version_manager.h"
#include "../dialogs/extract_dialog.h"

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QProcess>
#include <QStandardPaths>
#include <QDirIterator>

LauncherWindow::LauncherWindow(QWidget *parent) : QWidget(parent) {
  setupUi();
  setupConnections();
  loadInstalledVersions();
}

void LauncherWindow::setupUi() {
  setWindowTitle("Trinity Launcher - Multiversions");
  resize(800, 500);
  setFixedSize(800, 500);

  // Global Dark Theme
  setStyleSheet("QWidget { background-color: #020617; color: #ffffff; "
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
      "border-image: url(:/icons/logo); border-radius: 60px;");
  topBarLayout->addWidget(logoLabel);

  QLabel *titleLabel = new QLabel("Trinity Launcher");
  titleLabel->setObjectName("Title");
  topBarLayout->addWidget(titleLabel);

  topBarLayout->addStretch();

  extractButton = new QPushButton("+ Extraer APK");
  extractButton->setObjectName("ActionButton"); // Accent color
  topBarLayout->addWidget(extractButton);

  importButton = new QPushButton("Importar");  // ✅ Nuevo botón
  importButton->setObjectName("ActionButton");
  topBarLayout->addWidget(importButton);

  toolsButton = new QPushButton("Tools");
  toolsButton->setObjectName("ActionButton"); // Apply accent style
  topBarLayout->addWidget(toolsButton);

  rootLayout->addLayout(topBarLayout);

  // --- Content Area (Split View) ---
  QHBoxLayout *contentLayout = new QHBoxLayout();
  contentLayout->setSpacing(20);

  // Left: Version List
  versionList = new QListWidget();
  versionList->setIconSize(QSize(48, 48));
  versionList->setFixedWidth(300);
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
  versionNameLabel = new QLabel("Selecciona una versión");
  versionNameLabel->setObjectName("VersionName");
  versionNameLabel->setAlignment(Qt::AlignCenter);
  panelLayout->addWidget(versionNameLabel);

  versionTypeLabel = new QLabel("");
  versionTypeLabel->setObjectName("VersionType");
  versionTypeLabel->setAlignment(Qt::AlignCenter);
  panelLayout->addWidget(versionTypeLabel);

  panelLayout->addSpacing(20);

  // Actions
  playButton = new QPushButton("JUGAR");
  playButton->setObjectName("ActionButton");
  playButton->setMinimumHeight(45);
  playButton->setEnabled(false);
  panelLayout->addWidget(playButton);

  editButton = new QPushButton("Editar Configuración");
  editButton->setObjectName("ActionButton");
  panelLayout->addWidget(editButton);

  QHBoxLayout *secondaryActions = new QHBoxLayout();
  exportButton = new QPushButton("Exportar");
  exportButton->setObjectName("ActionButton");
  deleteButton = new QPushButton("Eliminar");
  deleteButton->setObjectName("ActionButton");
  secondaryActions->addWidget(exportButton);
  secondaryActions->addWidget(deleteButton);
  panelLayout->addLayout(secondaryActions);

  panelLayout->addStretch();

  contentLayout->addWidget(contextPanel);
  rootLayout->addLayout(contentLayout);

  // --- Status Bar ---
  statusLabel = new QLabel("Listo");
  statusLabel->setObjectName("Status");
  rootLayout->addWidget(statusLabel);
}

void LauncherWindow::setupConnections() {
  connect(extractButton, &QPushButton::clicked, this,
        &LauncherWindow::showExtractDialog);
  connect(toolsButton, &QPushButton::clicked, this,
        &LauncherWindow::launchTools);
  connect(playButton, &QPushButton::clicked, this, &LauncherWindow::launchGame);
  connect(versionList, &QListWidget::itemClicked, this,
        &LauncherWindow::onVersionSelected);
  connect(importButton, &QPushButton::clicked, this, &LauncherWindow::onImportClicked);
// Conecta los nuevos botones
  connect(editButton, &QPushButton::clicked, this, &LauncherWindow::onEditConfigClicked);
  connect(exportButton, &QPushButton::clicked, this, &LauncherWindow::onExportClicked);
  connect(deleteButton, &QPushButton::clicked, this, &LauncherWindow::onDeleteClicked);
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
    versionNameLabel->setText("Sin versiones");
    versionTypeLabel->setText("");
    playButton->setEnabled(false);
    statusLabel->setText("No hay versiones instaladas.");
    return;
  }

  versionNameLabel->setText(versionName);
  versionTypeLabel->setText("Bedrock Edition"); // Placeholder type
  playButton->setEnabled(true);

  // Update status bar with size info (mockup)
  VersionManager vm;
  QString path = vm.getVersionPath(versionName);
  statusLabel->setText(QString("Versión seleccionada: %1 | Ruta: %2")
                           .arg(versionName)
                           .arg(path));
}

void LauncherWindow::showExtractDialog() {
  ExtractDialog dialog(this);
  if (dialog.exec() != QDialog::Accepted)
    return;

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
        QMessageBox::warning(this, "Advertencia", "No hay ninguna versión seleccionada.");
        return;
    }
    QString selectedVersion = versionList->selectedItems().first()->text();

    VersionConfig config(selectedVersion);
    QString currentArgs = config.getLaunchArgs();

    // Diálogo simple de edición
    QDialog dialog(this);
    dialog.setWindowTitle("Editar configuración de " + selectedVersion);
    dialog.resize(500, 300);

    auto *layout = new QVBoxLayout(&dialog);
    QLabel *label = new QLabel("Parámetros de ejecución (antes de mcpelauncher-client):");
    layout->addWidget(label);

    QLineEdit *argsEdit = new QLineEdit(currentArgs);
    argsEdit->setPlaceholderText("Ej: DRI_PRIME=1 vblank_mode=0");
    layout->addWidget(argsEdit);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, [&]() {
        QString newArgs = argsEdit->text().trimmed();
        config.setLaunchArgs(newArgs);
        if (config.save()) {
            QMessageBox::information(&dialog, "Éxito", "Configuración guardada.");
            dialog.accept();
        } else {
            QMessageBox::critical(&dialog, "Error", "No se pudo guardar la configuración.");
        }
    });
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        statusLabel->setText(QString("Configuración de %1 actualizada.").arg(selectedVersion));
    }
}

void LauncherWindow::onExportClicked() {
    if (versionList->selectedItems().isEmpty()) {
        QMessageBox::warning(this, "Advertencia", "No hay ninguna versión seleccionada.");
        return;
    }
    QString selectedVersion = versionList->selectedItems().first()->text();

    VersionManager vm;
    QString versionPath = vm.getVersionPath(selectedVersion);

    if (!QDir(versionPath).exists()) {
        QMessageBox::critical(this, "Error", "La versión no existe.");
        return;
    }

    QString baseDataDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                          + "/mcpelauncher";

    // Carpeta games/com.mojang
    QString gamesPath = baseDataDir + "/games/com.mojang";

    QString zipPath = QFileDialog::getSaveFileName(
        this, "Guardar como ZIP", selectedVersion + ".zip",
        "Archivos ZIP (*.zip);;Todos los archivos (*)"
    );
    if (zipPath.isEmpty()) return;

    // Crear un directorio temporal para organizar lo que se va a comprimir
    QString tempDir = QDir::tempPath() + "/trinity_export_" + selectedVersion;
    if (QDir(tempDir).exists()) {
        QDir(tempDir).removeRecursively();
    }
    QDir().mkpath(tempDir);

    // Copiar contenido de la versión a una subcarpeta temporal
    QString tempVersionDir = tempDir + "/version_content";
    if (!copyDirectory(versionPath, tempVersionDir)) {
        QMessageBox::critical(this, "Error", "No se pudo copiar la versión.");
        QDir(tempDir).removeRecursively();
        return;
    }

    // Copiar games/com.mojang a otra subcarpeta temporal
    QString tempGamesDir = tempDir + "/games";
    if (!copyDirectory(gamesPath, tempGamesDir)) {
        QMessageBox::critical(this, "Error", "No se pudo copiar los recursos.");
        QDir(tempDir).removeRecursively();
        return;
    }

    // Comprimir ambos directorios en el ZIP
    QProcess process;
    process.start("zip", {"-r", zipPath, tempVersionDir, tempGamesDir});
    process.waitForFinished(-1);

    // Limpiar el directorio temporal
    QDir(tempDir).removeRecursively();

    if (process.exitCode() == 0) {
        QMessageBox::information(this, "Éxito", "Versión y recursos exportados correctamente.");
        statusLabel->setText(QString("Versión %1 exportada como %2").arg(selectedVersion, zipPath));
    } else {
        QString err = process.readAllStandardError();
        QMessageBox::critical(this, "Error", "Falló la exportación:\n" + err);
    }
}

void LauncherWindow::onDeleteClicked() {
    if (versionList->selectedItems().isEmpty()) {
        QMessageBox::warning(this, "Advertencia", "No hay ninguna versión seleccionada.");
        return;
    }
    QString selectedVersion = versionList->selectedItems().first()->text();

    int r = QMessageBox::warning(
        this, "Advertencia",
        QString("¿Estás seguro de eliminar la versión '%1'?\nEsta acción no se puede deshacer.")
            .arg(selectedVersion),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No
    );
    if (r == QMessageBox::No) return;

    VersionManager vm;
    QString versionPath = vm.getVersionPath(selectedVersion);

    if (!QDir(versionPath).removeRecursively()) {
        QMessageBox::critical(this, "Error", "No se pudo eliminar la versión.");
        return;
    }

    QMessageBox::information(this, "Éxito", "Versión eliminada correctamente.");
    loadInstalledVersions(); // Recargar lista
    statusLabel->setText(QString("Versión %1 eliminada.").arg(selectedVersion));
}

bool LauncherWindow::copyDirectory(const QString &srcPath, const QString &dstPath) {
    QDir srcDir(srcPath);
    if (!srcDir.exists()) return false;
    if (!QDir().mkpath(dstPath)) return false;

    for (const QFileInfo &info : srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        QString srcItem = srcPath + "/" + info.fileName();
        QString dstItem = dstPath + "/" + info.fileName();

        if (info.isDir()) {
            if (!copyDirectory(srcItem, dstItem)) return false;
        } else {
            if (!QFile::copy(srcItem, dstItem)) return false;
        }
    }
    return true;
}

void LauncherWindow::onImportClicked() {
    QString zipPath = QFileDialog::getOpenFileName(
        this, "Seleccionar archivo ZIP", "",
        "Archivos ZIP (*.zip);;Todos los archivos (*)"
    );
    if (zipPath.isEmpty()) return;

    // Extraer nombre de la versión del nombre del archivo (sin extensión)
    QString fileName = QFileInfo(zipPath).baseName(); // Ej: "1.21.121" de "1.21.121.zip"

    // Directorio temporal para extracción
    QString tempDir = QDir::tempPath() + "/trinity_import_" + fileName;
    if (QDir(tempDir).exists()) {
        QDir(tempDir).removeRecursively();
    }
    QDir().mkpath(tempDir);

    // Descomprimir el ZIP
    QProcess process;
    process.start("unzip", {"-q", zipPath, "-d", tempDir});
    process.waitForFinished(-1);

    if (process.exitCode() != 0) {
        QString err = process.readAllStandardError();
        QMessageBox::critical(this, "Error", "Falló la extracción:\n" + err);
        QDir(tempDir).removeRecursively();
        return;
    }

    // Buscar recursivamente los directorios "version_content" y "games"
    QString versionContentPath;
    QString gamesPath;

    QDirIterator it(tempDir, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString dir = it.next();
        if (QFileInfo(dir).fileName() == "version_content") {
            versionContentPath = dir;
        } else if (QFileInfo(dir).fileName() == "games") {
            gamesPath = dir;
        }
    }

    // Ruta base de datos de la app
    QString baseDataDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                          + "/mcpelauncher";

    // Rutas de destino
    QString destVersionDir = baseDataDir + "/versions/" + fileName;
    QString destGamesDir = baseDataDir + "/games/com.mojang";

    // Verificar si ya existen versiones o recursos
    bool versionExists = QDir(destVersionDir).exists();
    bool gamesExists = QDir(destGamesDir).exists();

    if (versionExists || gamesExists) {
        int r = QMessageBox::warning(
            this, "Advertencia",
            QString("Ya existen datos para '%1'.\n"
                    "¿Reemplazarlos?\n\n"
                    "Esto eliminará los archivos actuales.").arg(fileName),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No
        );
        if (r == QMessageBox::No) {
            QDir(tempDir).removeRecursively();
            return;
        }
    }

    // Copiar version_content → versions/<nombre>
    if (!versionContentPath.isEmpty()) {
        if (versionExists) {
            QDir(destVersionDir).removeRecursively(); // Eliminar versión anterior
        }
        if (!copyDirectory(versionContentPath, destVersionDir)) {
            QMessageBox::critical(this, "Error", "No se pudo copiar la versión.");
            QDir(tempDir).removeRecursively();
            return;
        }
    } else {
        QMessageBox::warning(this, "Advertencia", "El archivo ZIP no contiene 'version_content'.");
    }

    // Copiar games → games/com.mojang
    if (!gamesPath.isEmpty()) {
        if (gamesExists) {
            QDir(destGamesDir).removeRecursively(); // Eliminar recursos anteriores
        }
        if (!copyDirectory(gamesPath, destGamesDir)) {
            QMessageBox::critical(this, "Error", "No se pudo copiar los recursos.");
            QDir(tempDir).removeRecursively();
            return;
        }
    } else {
        QMessageBox::warning(this, "Advertencia", "El archivo ZIP no contiene 'games'.");
    }

    // Limpiar temporal
    QDir(tempDir).removeRecursively();

    QMessageBox::information(this, "Éxito", "Versión e información importadas correctamente.");
    loadInstalledVersions(); // Recargar lista
    statusLabel->setText(QString("Versión %1 importada.").arg(fileName));
}