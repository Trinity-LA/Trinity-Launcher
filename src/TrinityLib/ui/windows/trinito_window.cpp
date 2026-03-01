#include "TrinityLib/ui/windows/trinito_window.hpp"
#include "TrinityLib/ui/windows/launcher_window.hpp"
#include "TrinityLib/core/pack_installer.hpp"
#include "TrinityLib/core/version_manager.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QTabWidget>
#include <QVBoxLayout>

#include <QProgressDialog>
#include <QHBoxLayout>
#include <QRandomGenerator>
#include <QTimer>
#include <QUrl>
#include <QtConcurrent/QtConcurrent>

TrinitoWindow::TrinitoWindow(QWidget *parent, LauncherWindow *launcher)
    : QWidget(parent), m_launcher(launcher) {
    setWindowTitle(tr("Content Manager for Bedrock"));
    resize(820, 500);

    auto *layout = new QVBoxLayout(this);
    QTabWidget *tabs = new QTabWidget();
    layout->addWidget(tabs);

    // Primera pestaña: Instancias
    tabs->addTab(createInstancesTab(), tr("Instances"));

    // Pestañas de instalación (como antes)
    tabs->addTab(createPackTab("behavior_packs", tr("Behavior Pack (mods)")),
                 tr("Mods"));
    tabs->addTab(createPackTab("resource_packs", tr("Resource Pack")),
                 tr("Textures"));
    tabs->addTab(createDevTab(), tr("Development"));
    tabs->addTab(createWorldTab(), tr("Worlds"));
    // Añadir la nueva pestaña de Shaders/Mods
    tabs->addTab(createShadersModsTab(), tr("Shaders/Libs"));
    // Pestaña de directorio de datos
    tabs->addTab(createDirectoryTab(), tr("Directory"));
}


QWidget *TrinitoWindow::createInstancesTab() {
    auto *widget = new QWidget();
    auto *outerLayout = new QVBoxLayout(widget);
    outerLayout->setContentsMargins(16, 12, 16, 12);
    outerLayout->setSpacing(8);

    // ── Shared column header row (same height, same visual level) ─────────
    auto *headerRow = new QHBoxLayout();
    headerRow->setContentsMargins(0, 0, 0, 0);
    headerRow->setSpacing(16);

    QLabel *listLabel = new QLabel(tr("Installed versions:"));
    listLabel->setStyleSheet("font-size: 13px; color: #94a3b8;");
    headerRow->addWidget(listLabel, 3);

    QLabel *actionsLabel = new QLabel(tr("Actions"));
    actionsLabel->setStyleSheet("font-size: 13px; color: #94a3b8;");
    headerRow->addWidget(actionsLabel, 2);

    outerLayout->addLayout(headerRow);

    // ── Thin separator below headers ──────────────────────────────────────
    auto *sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: #1e293b;");
    outerLayout->addWidget(sep);

    // ── Split: [version list] | [action panel] ────────────────────────────
    auto *splitLayout = new QHBoxLayout();
    splitLayout->setSpacing(16);

    // LEFT: installed versions
    auto *leftWidget = new QWidget();
    auto *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(6);

    auto *versionsList = new QListWidget();
    versionsList->setIconSize(QSize(24, 24));
    versionsList->setStyleSheet(
        "QListWidget { border-radius: 8px; padding: 4px; outline: 0; }"
        "QListWidget::item { padding: 8px; border-radius: 5px; margin-bottom: 3px; }"
    );

    // Populate from VersionManager
    if (m_launcher) {
        VersionManager vm;
        for (const QString &v : vm.getInstalledVersions()) {
            auto *item = new QListWidgetItem(QIcon(":/icons/cube"), v);
            versionsList->addItem(item);
        }
        if (versionsList->count() > 0)
            versionsList->setCurrentRow(0);
    }
    leftLayout->addWidget(versionsList);

    splitLayout->addWidget(leftWidget, 3); // takes 3/5 of space

    // RIGHT: action buttons panel
    auto *rightWidget = new QWidget();
    rightWidget->setObjectName("ContextPanel");
    auto *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(16, 8, 16, 16);
    rightLayout->setSpacing(10);

    // Helper: makes a full-width button with tooltip
    auto makeBtn = [&](const QString &label, const QString &tip) -> QPushButton* {
        auto *btn = new QPushButton(label);
        btn->setObjectName("ActionButton");
        btn->setMinimumHeight(40);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setToolTip(tip);
        btn->setEnabled(false); // enabled when a version is selected below
        rightLayout->addWidget(btn);
        return btn;
    };

    auto *shortcutBtn = makeBtn(tr("Create shortcut"),
        tr("Creates a .desktop shortcut in Downloads for the selected version."));
    auto *envBtn      = makeBtn(tr("Environment Parameters"),
        tr("Edit launch arguments and environment variables."));
    auto *importBtn   = makeBtn(tr("Import"),
        tr("Import a previously exported version archive."));
    auto *exportBtn   = makeBtn(tr("Export"),
        tr("Export the selected version as an archive."));

    rightLayout->addStretch();
    splitLayout->addWidget(rightWidget, 2); // takes 2/5 of space

    outerLayout->addLayout(splitLayout);

    // ── Wire buttons only when launcher available ──────────────────────────
    if (m_launcher) {
        connect(shortcutBtn, &QPushButton::clicked, m_launcher, &LauncherWindow::createDesktopShortcut);
        connect(envBtn,      &QPushButton::clicked, m_launcher, &LauncherWindow::onEditConfigClicked);
        connect(importBtn,   &QPushButton::clicked, m_launcher, &LauncherWindow::onImportClicked);
        connect(exportBtn,   &QPushButton::clicked, m_launcher, &LauncherWindow::onExportClicked);

        // Enable/disable action buttons based on list selection,
        // and sync selection back to the launcher's versionCombo.
        connect(versionsList, &QListWidget::currentTextChanged, m_launcher,
            [this, shortcutBtn, envBtn, exportBtn](const QString &version) {
                bool valid = !version.isEmpty();
                shortcutBtn->setEnabled(valid);
                envBtn->setEnabled(valid);
                exportBtn->setEnabled(valid);
                // Sync the launcher's version combo to match
                if (m_launcher)
                    m_launcher->versionCombo->setCurrentText(version);
            });

        // Import doesn't depend on a selected version
        importBtn->setEnabled(true);

        // Enable buttons for initial selection if any
        if (versionsList->count() > 0) {
            shortcutBtn->setEnabled(true);
            envBtn->setEnabled(true);
            exportBtn->setEnabled(true);
        }
    }

    return widget;
}



QWidget *TrinitoWindow::createManageTab(const QString &packType,
                                        const QString &displayName) {
    auto *widget = new QWidget();
    auto *layout = new QVBoxLayout(widget);

    QLabel *label = new QLabel(tr("List of installed %1:").arg(displayName));
    layout->addWidget(label);

    QListWidget *listWidget = new QListWidget();
    layout->addWidget(listWidget);

    // Asignar la lista a la variable miembro correspondiente para usarla en
    // loadPacks
    if (packType == "behavior_packs") {
        modsList = listWidget;
    } else if (packType == "resource_packs") {
        resourcesList = listWidget;
    } else if (packType == "minecraftWorlds") {
        mapsList = listWidget;
    }

    // Cargar packs al mostrar la pestaña (opcional, o al construir)
    loadPacks(packType, listWidget);

    // Botón para recargar la lista
    QPushButton *refreshButton = new QPushButton(tr("Refresh List"));
    connect(refreshButton, &QPushButton::clicked, this,
            [=, this]() { loadPacks(packType, listWidget); });
    layout->addWidget(refreshButton);

    return widget;
}

void TrinitoWindow::loadPacks(const QString &packType,
                              QListWidget *listWidget) {
    listWidget->clear(); // Limpiar lista actual

    QString baseDataDir =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
        "/mcpelauncher/games/com.mojang";
    QString packDirPath = baseDataDir + "/" + packType;

    QDir packDir(packDirPath);
    if (!packDir.exists()) {
        // QMessageBox::information(this, "Info", "No se encontraron " +
        // packType + ".");
        return; // Si no existe la carpeta, salir
    }

    QStringList entries =
        packDir.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);

    for (const QString &entry : entries) {
        QFileInfo info(packDirPath + "/" + entry);
        bool isEnabled = true;

        // Verificar si está deshabilitado (renombrado)
        if (entry.endsWith(".disabled")) {
            isEnabled = false;
        }

        // Crear item de lista
        QListWidgetItem *item = new QListWidgetItem();
        item->setCheckState(isEnabled ? Qt::Checked : Qt::Unchecked);
        item->setText(entry); // Mostrar nombre original o con .disabled

        listWidget->addItem(item);
    }

    // Conectar la señal itemChanged del QListWidget para detectar cambios en
    // los checkboxes
    connect(listWidget, &QListWidget::itemChanged, this,
            [=, this](QListWidgetItem *changedItem) {
                // Obtener el estado actual del checkbox
                Qt::CheckState state = changedItem->checkState();
                bool newState = (state == Qt::Checked);

                // Obtener el nombre del pack
                QString packName = changedItem->text();

                // Llamar a togglePack para renombrar
                togglePack(packType, packName, newState);

                // Opcional: Actualizar el nombre mostrado si cambia el estado
                QString newName = packName;
                if (newState) {
                    // Si está habilitado y el nombre termina en .disabled,
                    // removerlo
                    if (packName.endsWith(".disabled")) {
                        newName = packName.chopped(9); // .disabled.length() = 9
                    }
                } else {
                    // Si está deshabilitado y el nombre no termina en
                    // .disabled, añadirlo
                    if (!packName.endsWith(".disabled")) {
                        newName = packName + ".disabled";
                    }
                }
                // Actualizar el texto del item (esto puede causar un nuevo
                // itemChanged si no se maneja cuidadosamente) Para evitar
                // loops, podrías comparar newName con el texto actual antes de
                // cambiarlo
                if (changedItem->text() != newName) {
                    changedItem->setText(newName);
                }
            });
}

void TrinitoWindow::togglePack(const QString &packType, const QString &packName,
                               bool enable) {
    QString baseDataDir =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
        "/mcpelauncher/games/com.mojang";
    QString packDirPath = baseDataDir + "/" + packType;

    QString oldPath = packDirPath + "/" + packName;
    QString newPath = oldPath;

    if (enable) {
        // Habilitar: descomprimir el .disabled
        if (packName.endsWith(".disabled")) {
            // El newPath es el nombre original (sin .disabled)
            newPath = oldPath.chopped(9); // Quita ".disabled" (longitud 9)

            // Comando: tar -xzf <archivo_comprimido> -C <directorio_destino>
            QProcess process;
            process.start("tar", {"-xzf", oldPath, "-C", packDirPath});
            process.waitForFinished(-1);

            if (process.exitCode() == 0) {
                // Eliminar el archivo .disabled después de descomprimir
                QFile::remove(oldPath);
                // QMessageBox::information(this, "Éxito", QString("Pack '%1'
                // habilitado.").arg(packName));
            } else {
                QString err = process.readAllStandardError();
                QMessageBox::critical(
                    this, "Error",
                    QString("No se pudo habilitar el pack '%1'.\nError al "
                            "descomprimir:\n%2")
                        .arg(packName)
                        .arg(err.isEmpty() ? "Error desconocido." : err));
                return;
            }
        }
        // Si no termina en .disabled, no hacer nada (ya está habilitado)
    } else {
        // Deshabilitar: comprimir a .disabled
        if (!packName.endsWith(".disabled")) {
            // El newPath es el nombre original + .disabled
            newPath = oldPath + ".disabled";

            // Comando: tar -czf <archivo_salida> -C <directorio_padre>
            // <nombre_carpeta_o_archivo> Ejemplo: tar -czf
            // pack1.mcpack.disabled.tar.gz -C /ruta/contenedora pack1.mcpack
            QFileInfo fileInfo(oldPath);
            QString parentDir =
                fileInfo.absolutePath(); // Directorio padre del pack
            QString baseName =
                fileInfo.fileName(); // Nombre del pack (archivo o carpeta)

            QProcess process;
            process.start("tar", {"-czf", newPath, "-C", parentDir, baseName});
            process.waitForFinished(-1);

            if (process.exitCode() == 0) {
                // Eliminar el archivo/carpeta original después de comprimir
                if (QDir(oldPath).exists()) {
                    QDir(oldPath).removeRecursively();
                } else {
                    QFile::remove(oldPath);
                }
                // QMessageBox::information(this, "Éxito", QString("Pack '%1'
                // deshabilitado.").arg(packName));
            } else {
                QString err = process.readAllStandardError();
                QMessageBox::critical(
                    this, "Error",
                    QString("No se pudo deshabilitar el pack '%1'.\nError al "
                            "comprimir:\n%2")
                        .arg(packName)
                        .arg(err.isEmpty() ? "Error desconocido." : err));
                return;
            }
        }
        // Si ya termina en .disabled, no hacer nada (ya está deshabilitado)
    }

    // Opcional: Actualizar la lista de packs en la UI si estás en la pestaña
    // correspondiente Por ejemplo, si estás en la pestaña "Gestionar Mods": if
    // (currentTab == "Gestionar Mods") {
    //     loadPacks("behavior_packs", modsList);
    // }
    // O simplemente mostrar un mensaje de éxito general aquí si prefieres no
    // recargar constantemente.
}

// ... resto del código (createPackTab, createDevTab, createWorldTab,
// installItem) ...

QWidget *TrinitoWindow::createPackTab(const QString &targetSubdir,
                                      const QString &labelText) {
    auto *widget = new QWidget();
    auto *layout = new QVBoxLayout(widget);

    // Título
    QLabel *titleLabel = new QLabel(labelText);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 16px;");
    layout->addWidget(titleLabel);

    // Sección de instalación
    QLabel *installLabel = new QLabel(tr("Install new ") + labelText + ":");
    layout->addWidget(installLabel);

    auto *installButton = new QPushButton(tr("Select file..."));
    layout->addWidget(installButton);

    connect(installButton, &QPushButton::clicked, this, [=, this]() {
        QString path = QFileDialog::getOpenFileName(
            this, tr("Select pack"), QDir::homePath(),
            tr("Archivos compatibles (*.zip *.mcpack);;Todos los archivos "
               "(*)"));
        if (!path.isEmpty()) {
            installItem(path, targetSubdir);
        }
    });

    layout->addSpacing(15);

    // Sección de gestión
    QLabel *manageLabel =
        new QLabel(tr("Manage ") + labelText + tr(" installed:"));
    layout->addWidget(manageLabel);

    QListWidget *listWidget = new QListWidget();
    layout->addWidget(listWidget);

    // Asignar la lista a la variable miembro correspondiente para usarla en
    // loadPacks
    if (targetSubdir == "behavior_packs") {
        modsList = listWidget;
    } else if (targetSubdir == "resource_packs") {
        resourcesList = listWidget;
    } else if (targetSubdir == "minecraftWorlds") {
        mapsList = listWidget;
    }

    // Cargar packs al mostrar la pestaña
    loadPacks(targetSubdir, listWidget);

    // Botón para recargar la lista
    QPushButton *refreshButton = new QPushButton(tr("Refresh List"));
    connect(refreshButton, &QPushButton::clicked, this,
            [=, this]() { loadPacks(targetSubdir, listWidget); });
    layout->addWidget(refreshButton);

    // Botón para eliminar seleccionado
    QPushButton *deleteButton = new QPushButton(tr("Delete Selected"));
    connect(deleteButton, &QPushButton::clicked, this, [=, this]() {
        if (listWidget->selectedItems().isEmpty()) {
            QMessageBox::warning(this, tr("Warning"),
                                 tr("No element selected."));
            return;
        }

        QString selectedEntry = listWidget->selectedItems().first()->text();
        int r = QMessageBox::warning(
            this, tr("Warning"),
            QString(tr("Are you sure you want to delete '%1'?\n"
"This action " "cannot be undone."))
                .arg(selectedEntry),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (r == QMessageBox::No)
            return;

        // Eliminar el archivo/carpeta
        QString baseDataDir = QStandardPaths::writableLocation(
                                  QStandardPaths::GenericDataLocation) +
                              "/mcpelauncher/games/com.mojang";
        QString fullPath =
            baseDataDir + "/" + targetSubdir + "/" + selectedEntry;

        QFileInfo info(fullPath);
        bool success = false;
        if (info.isDir()) {
            success = QDir(fullPath).removeRecursively();
        } else {
            success = QFile::remove(fullPath);
        }

        if (success) {
            QMessageBox::information(
                this, tr("Success"),
                QString(tr("'%1' eliminado correctamente."))
                    .arg(selectedEntry));
            // Recargar la lista
            loadPacks(targetSubdir, listWidget);
        } else {
            QMessageBox::critical(
                this, "Error",
                QString("No se pudo eliminar '%1'.").arg(selectedEntry));
        }
    });
    layout->addWidget(deleteButton);

    return widget;
}

QWidget *TrinitoWindow::createDevTab() {
    auto *widget = new QWidget();
    auto *layout = new QVBoxLayout(widget);

    // Título
    QLabel *titleLabel = new QLabel(tr("Development Packs"));
    titleLabel->setStyleSheet("font-weight: bold; font-size: 16px;");
    layout->addWidget(titleLabel);

    // Contenedor horizontal para los dos botones de instalación
    auto *buttonLayout = new QHBoxLayout();

    // Botón para Development Behavior Pack
    auto *behButton =
        new QPushButton(tr("Add Development Behavior Pack (file)..."));
    connect(behButton, &QPushButton::clicked, this, [=, this]() {
        QString path = QFileDialog::getOpenFileName(
            this, tr("Add Development Behavior Pack"), QDir::homePath(),
            tr("Archivos compatibles (*.zip *.mcpack);;Todos los archivos "
               "(*)"));
        if (!path.isEmpty()) {
            installItem(path, "development_behavior_packs");
        }
    });
    buttonLayout->addWidget(behButton);

    // Botón para Development Resource Pack
    auto *resButton =
        new QPushButton(tr("Add Development Resource Pack (file)..."));
    connect(resButton, &QPushButton::clicked, this, [=, this]() {
        QString path = QFileDialog::getOpenFileName(
            this, tr("Add Development Resource Pack"), QDir::homePath(),
            tr("Archivos compatibles (*.zip *.mcpack);;Todos los archivos "
               "(*)"));
        if (!path.isEmpty()) {
            installItem(path, "development_resource_packs");
        }
    });
    buttonLayout->addWidget(resButton);

    layout->addLayout(buttonLayout);

    layout->addSpacing(15);

    // Sección de gestión
    QLabel *manageLabel = new QLabel(tr("Manage Development Packs:"));
    manageLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    layout->addWidget(manageLabel);

    // Crear un contenedor para las dos listas
    auto *devLayout = new QHBoxLayout();

    // Lista de Development Behavior Packs
    auto *behListWidget = new QListWidget();
    devLayout->addWidget(behListWidget);
    // Asignar a variable miembro si necesitas recargarla
    // No es necesario si no vas a recargarla, pero por consistencia:
    // developmentBehaviorList = behListWidget;

    // Lista de Development Resource Packs
    auto *resListWidget = new QListWidget();
    devLayout->addWidget(resListWidget);
    // developmentResourceList = resListWidget;

    layout->addLayout(devLayout);

    // Cargar packs al mostrar la pestaña
    loadPacks("development_behavior_packs", behListWidget);
    loadPacks("development_resource_packs", resListWidget);

    // Botón para recargar la lista
    QPushButton *refreshButton = new QPushButton(tr("Refresh Lists"));
    connect(refreshButton, &QPushButton::clicked, this, [=, this]() {
        loadPacks("development_behavior_packs", behListWidget);
        loadPacks("development_resource_packs", resListWidget);
    });
    layout->addWidget(refreshButton);

    // Contenedor horizontal para los dos botones de eliminar
    auto *deleteLayout = new QHBoxLayout();

    // Botón para eliminar un pack seleccionado en la lista de Behavior Packs
    QPushButton *deleteBehButton =
        new QPushButton(tr("Delete Selected Behavior Pack"));
    connect(deleteBehButton, &QPushButton::clicked, this, [=, this]() {
        if (behListWidget->selectedItems().isEmpty()) {
            QMessageBox::warning(
                this, tr("Warning"),
                tr("No Behavior Pack selected."));
            return;
        }

        QString selectedEntry = behListWidget->selectedItems().first()->text();
        int r = QMessageBox::warning(
            this, tr("Warning"),
            QString(tr("Are you sure you want to delete Behavior Pack '%1'?\n"
"This " "action cannot be undone."))
                .arg(selectedEntry),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (r == QMessageBox::No)
            return;

        // Eliminar el archivo/carpeta
        QString baseDataDir = QStandardPaths::writableLocation(
                                  QStandardPaths::GenericDataLocation) +
                              "/mcpelauncher/games/com.mojang";
        QString fullPath =
            baseDataDir + "/development_behavior_packs/" + selectedEntry;

        QFileInfo info(fullPath);
        bool success = false;
        if (info.isDir()) {
            success = QDir(fullPath).removeRecursively();
        } else {
            success = QFile::remove(fullPath);
        }

        if (success) {
            QMessageBox::information(
                this, tr("Success"),
                QString(tr("deleted successfully.")).arg(selectedEntry));
            // Recargar la lista
            loadPacks("development_behavior_packs", behListWidget);
        } else {
            QMessageBox::critical(
                this, "Error",
                QString(tr("Could not delete")).arg(selectedEntry));
        }
    });
    deleteLayout->addWidget(deleteBehButton);

    // Botón para eliminar un pack seleccionado en la lista de Resource Packs
    QPushButton *deleteResButton =
        new QPushButton(tr("Delete Selected Resource Pack"));
    connect(deleteResButton, &QPushButton::clicked, this, [=, this]() {
        if (resListWidget->selectedItems().isEmpty()) {
            QMessageBox::warning(
                this, tr("Warning"),
                tr("No Resource Pack selected."));
            return;
        }

        QString selectedEntry = resListWidget->selectedItems().first()->text();
        int r = QMessageBox::warning(
            this, tr("Warning"),
            QString(tr("Are you sure you want to delete Resource Pack '%1'?\n"
"This " "action cannot be undone."))
                .arg(selectedEntry),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (r == QMessageBox::No)
            return;

        // Eliminar el archivo/carpeta
        QString baseDataDir = QStandardPaths::writableLocation(
                                  QStandardPaths::GenericDataLocation) +
                              "/mcpelauncher/games/com.mojang";
        QString fullPath =
            baseDataDir + "/development_resource_packs/" + selectedEntry;

        QFileInfo info(fullPath);
        bool success = false;
        if (info.isDir()) {
            success = QDir(fullPath).removeRecursively();
        } else {
            success = QFile::remove(fullPath);
        }

        if (success) {
            QMessageBox::information(
                this, tr("Success"),
                QString(tr("deleted successfully.")).arg(selectedEntry));
            // Recargar la lista
            loadPacks("development_resource_packs", resListWidget);
        } else {
            QMessageBox::critical(
                this, "Error",
                QString("No se pudo eliminar el Resource Pack '%1'.")
                    .arg(selectedEntry));
        }
    });
    deleteLayout->addWidget(deleteResButton);

    layout->addLayout(deleteLayout);

    return widget;
}

QWidget *TrinitoWindow::createWorldTab() {
    auto *widget = new QWidget();
    auto *layout = new QVBoxLayout(widget);

    // Título
    QLabel *titleLabel = new QLabel(tr("Saved Worlds"));
    titleLabel->setStyleSheet("font-weight: bold; font-size: 16px;");
    layout->addWidget(titleLabel);

    // Botón para seleccionar carpeta del mundo
    auto *button = new QPushButton(tr("Add world folder..."));
    layout->addWidget(button);

    connect(button, &QPushButton::clicked, this, [=, this]() {
        QString path = QFileDialog::getExistingDirectory(
            this, tr("Select world folder"), QDir::homePath());
        if (!path.isEmpty()) {
            installItem(path, "minecraftWorlds");
        }
    });

    layout->addSpacing(15);

    // Sección de gestión
    QLabel *manageLabel = new QLabel(tr("Manage Worlds:"));
    manageLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    layout->addWidget(manageLabel);

    QListWidget *listWidget = new QListWidget();
    layout->addWidget(listWidget);

    // Asignar a variable miembro
    mapsList = listWidget;

    // Cargar mundos al mostrar la pestaña
    loadPacks("minecraftWorlds", listWidget);

    // Botón para recargar la lista
    QPushButton *refreshButton = new QPushButton(tr("Refresh List"));
    connect(refreshButton, &QPushButton::clicked, this,
            [=, this]() { loadPacks("minecraftWorlds", listWidget); });
    layout->addWidget(refreshButton);

    // Botón para borrar un mundo seleccionado
    QPushButton *deleteButton =
        new QPushButton(tr("Delete Selected World"));
    connect(deleteButton, &QPushButton::clicked, this, [=, this]() {
        if (listWidget->selectedItems().isEmpty()) {
            QMessageBox::warning(this, tr("Warning"),
                                 tr("No world selected."));
            return;
        }

        QString selectedWorld = listWidget->selectedItems().first()->text();
        int r = QMessageBox::warning(
            this, tr("Warning"),
            QString(tr("Are you sure you want to delete world '%1'?\n"
"This action " "cannot be undone."))
                .arg(selectedWorld),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (r == QMessageBox::No)
            return;

        // Borrar el mundo
        QString baseDataDir = QStandardPaths::writableLocation(
                                  QStandardPaths::GenericDataLocation) +
                              "/mcpelauncher/games/com.mojang";
        QString worldPath = baseDataDir + "/minecraftWorlds/" + selectedWorld;

        if (QDir(worldPath).removeRecursively()) {
            QMessageBox::information(this, tr("Success"),
                                     tr("World deleted successfully."));
            // Actualizar la lista
            loadPacks("minecraftWorlds", listWidget);
        } else {
            QMessageBox::critical(this, "Error",
                                  "No se pudo eliminar el mundo.");
        }
    });
    layout->addWidget(deleteButton);

    return widget;
}

void TrinitoWindow::installItem(const QString &sourcePath,
                                const QString &targetSubdir) {
    PackInstaller installer;

    if (installer.itemExists(sourcePath, targetSubdir)) {
        int r = QMessageBox::warning(
            this, tr("Warning"),
            QString(tr("An item named:\n%1\nalready exists.\n\nReplace it?"))
                .arg(installer.getTargetName(sourcePath)),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (r == QMessageBox::No)
            return;
    }

    QString errorMsg;
    // We pass true for forceOverwrite because we already asked the user if it
    // existed. If it didn't exist, forceOverwrite=true is fine (it just
    // copies). Wait, my PackInstaller logic: if exists and !force, return
    // error. If exists and force, delete and copy. So passing true is correct
    // here after user confirmation.

    if (installer.installItem(sourcePath, targetSubdir, true, errorMsg)) {
        QMessageBox::information(
            this, tr("Success"),
            QString(tr("%1 installed successfully in:\n%2"))
                .arg(installer.getTargetName(sourcePath))
                .arg(targetSubdir));
    } else {
        QMessageBox::critical(this, "Error",
                              tr("Installation failed:\n") + errorMsg);
    }
}

// --- NUEVAS FUNCIONES PARA SHADERS/MODS ---

// Función auxiliar para obtener directorio de shaders
QString TrinitoWindow::getShadersDir() {
    QString flatpakDir =
        QDir::homePath() +
        "/.var/app/com.trench.trinity.launcher/data/mcpelauncher";
    QString shadersDir = flatpakDir + "/shaders";

    // Si la carpeta base de Trinity Flatpak existe, usamos shaders dentro de
    // ella
    if (QDir(flatpakDir).exists()) {
        return shadersDir;
    } else {
        // Si no, usamos la carpeta local
        return QDir::homePath() + "/.local/share/mcpelauncher/shaders";
    }
}

QWidget *TrinitoWindow::createShadersModsTab() {
    auto *widget = new QWidget();
    auto *outerLayout = new QVBoxLayout(widget);
    outerLayout->setContentsMargins(12, 12, 12, 12);
    outerLayout->setSpacing(0);

    // Horizontal split: left = Shaders (50%), right = Libs (50%)
    auto *mainSplitLayout = new QHBoxLayout();
    mainSplitLayout->setSpacing(16);

    // ── LEFT: Shaders Section ─────────────────────────────────────────────
    auto *shadersSection = new QWidget();
    shadersSection->setObjectName("ContextPanel");
    auto *shadersLayout = new QVBoxLayout(shadersSection);
    shadersLayout->setContentsMargins(12, 12, 12, 12);
    shadersLayout->setSpacing(8);

    auto *shadersTitle = new QLabel(tr("Installed Shaders"));
    shadersTitle->setStyleSheet("font-weight: bold; font-size: 14px; color: #8b5cf6;");
    shadersLayout->addWidget(shadersTitle);

    auto *shadersSep = new QFrame();
    shadersSep->setFrameShape(QFrame::HLine);
    shadersSep->setStyleSheet("color: #1e293b;");
    shadersLayout->addWidget(shadersSep);

    shadersList = new QListWidget();
    shadersList->setStyleSheet(
        "QListWidget { border-radius: 6px; padding: 4px; outline: 0; }"
        "QListWidget::item { padding: 6px; border-radius: 4px; margin-bottom: 2px; }"
    );
    shadersLayout->addWidget(shadersList, 1); // stretch=1 → toma el espacio disponible

    // Botones de shaders en columna
    installShaderpackButton  = new QPushButton(tr("Install Shaderpack..."));
    installShaderpackButton->setObjectName("ActionButton");
    installShaderpackButton->setCursor(Qt::PointingHandCursor);
    installShaderpackButton->setMinimumHeight(36);

    removeShaderpackButton   = new QPushButton(tr("Delete all Shaders"));
    removeShaderpackButton->setCursor(Qt::PointingHandCursor);
    removeShaderpackButton->setMinimumHeight(36);

    refreshShaderListButton  = new QPushButton(tr("Refresh List"));
    refreshShaderListButton->setCursor(Qt::PointingHandCursor);
    refreshShaderListButton->setMinimumHeight(32);

    shadersLayout->addWidget(installShaderpackButton);
    shadersLayout->addWidget(removeShaderpackButton);
    shadersLayout->addWidget(refreshShaderListButton);

    mainSplitLayout->addWidget(shadersSection, 1); // 50%

    // ── RIGHT: Libs Section ───────────────────────────────────────────────
    auto *libsSection = new QWidget();
    libsSection->setObjectName("ContextPanel");
    auto *libsLayout = new QVBoxLayout(libsSection);
    libsLayout->setContentsMargins(12, 12, 12, 12);
    libsLayout->setSpacing(8);

    auto *libsTitle = new QLabel(tr("Manage Libs"));
    libsTitle->setStyleSheet("font-weight: bold; font-size: 14px; color: #8b5cf6;");
    libsLayout->addWidget(libsTitle);

    auto *libsSep = new QFrame();
    libsSep->setFrameShape(QFrame::HLine);
    libsSep->setStyleSheet("color: #1e293b;");
    libsLayout->addWidget(libsSep);

    // Available Libs
    auto *availableLibsLabel = new QLabel(tr("Available libs:"));
    availableLibsLabel->setStyleSheet("font-size: 12px; color: #94a3b8;");
    libsLayout->addWidget(availableLibsLabel);

    availableModsList = new QListWidget();
    availableModsList->setStyleSheet(
        "QListWidget { border-radius: 6px; padding: 4px; outline: 0; }"
        "QListWidget::item { padding: 6px; border-radius: 4px; margin-bottom: 2px; }"
    );
    libsLayout->addWidget(availableModsList, 1);

    downloadModButton = new QPushButton(tr("Download Selected Lib"));
    downloadModButton->setObjectName("ActionButton");
    downloadModButton->setCursor(Qt::PointingHandCursor);
    downloadModButton->setMinimumHeight(36);
    libsLayout->addWidget(downloadModButton);

    libsLayout->addSpacing(8);

    // Installed Libs
    auto *installedLibsLabel = new QLabel(tr("Installed libs (✓ = active):"));
    installedLibsLabel->setStyleSheet("font-size: 12px; color: #94a3b8;");
    libsLayout->addWidget(installedLibsLabel);

    installedModsList = new QListWidget();
    installedModsList->setStyleSheet(
        "QListWidget { border-radius: 6px; padding: 4px; outline: 0; }"
        "QListWidget::item { padding: 6px; border-radius: 4px; margin-bottom: 2px; }"
    );
    libsLayout->addWidget(installedModsList, 1);

    removeInstalledModButton = new QPushButton(tr("Delete Selected Lib"));
    removeInstalledModButton->setCursor(Qt::PointingHandCursor);
    removeInstalledModButton->setMinimumHeight(36);
    libsLayout->addWidget(removeInstalledModButton);

    mainSplitLayout->addWidget(libsSection, 1); // 50%

    outerLayout->addLayout(mainSplitLayout);

    // Connect signals
    connect(installShaderpackButton,  &QPushButton::clicked, this,
            &TrinitoWindow::onInstallShaderpackClicked);
    connect(removeShaderpackButton,   &QPushButton::clicked, this,
            &TrinitoWindow::onRemoveShaderpackClicked);
    connect(refreshShaderListButton,  &QPushButton::clicked, this,
            &TrinitoWindow::onRefreshShaderListClicked);
    connect(downloadModButton,        &QPushButton::clicked, this,
            &TrinitoWindow::onDownloadModClicked);
    connect(removeInstalledModButton, &QPushButton::clicked, this,
            &TrinitoWindow::onRemoveInstalledModClicked);

    // Initialize data
    populateInstalledShaders();
    populateAvailableMods();
    populateInstalledMods();

    return widget;
}

void TrinitoWindow::populateInstalledShaders() {
    QString shadersDir = getShadersDir(); // Detectar carpeta correcta
    QDir dir(shadersDir);

    shadersList->clear();

    if (!dir.exists()) {
        shadersList->addItem("(0 shaders)");
        return;
    }

    QFileInfoList files =
        dir.entryInfoList(QStringList() << "*.material.bin", QDir::Files);
    for (const QFileInfo &file : files) {
        shadersList->addItem(file.fileName());
    }
}
// this part it use https://github.com/minecraft-linux/mcpelauncher-moddb
// content under license MIT credits to creators
void TrinitoWindow::populateAvailableMods() {
    // Libs sin ninguna que contenga "arm" en el nombre
    QStringList availableMods = {
        "libmcpelaunchershadersmod.so",
        "libmcpelauncherdcblock.so",
        "libmcpelauncherlegacyx86_64.so",
        "libmcpelaunchernhc.so",
        "libmcpelauncherstrafesprintfix.so",
        "libmcpelauncherzoom.so",
        "libfullbright.so"
    };

    availableModsList->clear();

    for (const QString &mod : availableMods) {
        availableModsList->addItem(mod);
    }
}

void TrinitoWindow::populateInstalledMods() {
    QString modsDir =
        QDir::homePath() +
        "/.var/app/com.trench.trinity.launcher/data/mcpelauncher/mods";
    QDir dir(modsDir);

    // Desconectar para evitar disparos mientras llenamos la lista
    installedModsList->blockSignals(true);
    installedModsList->clear();

    if (!dir.exists()) {
        auto *placeholder = new QListWidgetItem(tr("(0 libs installed)"));
        placeholder->setFlags(placeholder->flags() & ~Qt::ItemIsEnabled);
        installedModsList->addItem(placeholder);
        installedModsList->blockSignals(false);
        return;
    }

    QFileInfoList files = dir.entryInfoList(
        QStringList() << "*.so" << "*.so.disabled", QDir::Files);

    for (const QFileInfo &file : files) {
        bool enabled = !file.fileName().endsWith(".disabled");
        auto *item = new QListWidgetItem(file.fileName());
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
        installedModsList->addItem(item);
    }

    installedModsList->blockSignals(false);

    // Conectar el toggle de checkbox (rename en disco)
    // Desconectamos primero si ya estaba conectado para no duplicar conexiones
    disconnect(installedModsList, &QListWidget::itemChanged, nullptr, nullptr);
    connect(installedModsList, &QListWidget::itemChanged, this,
            [this, modsDir](QListWidgetItem *changedItem) {
        bool enable = (changedItem->checkState() == Qt::Checked);
        QString name = changedItem->text();
        QString oldPath = modsDir + "/" + name;
        QString newPath;

        if (enable && name.endsWith(".disabled")) {
            newPath = modsDir + "/" + name.chopped(9); // quita ".disabled"
        } else if (!enable && !name.endsWith(".disabled")) {
            newPath = modsDir + "/" + name + ".disabled";
        } else {
            return; // sin cambio necesario
        }

        if (QFile::rename(oldPath, newPath)) {
            // Actualizar el texto del item sin re-disparar señales
            installedModsList->blockSignals(true);
            changedItem->setText(QFileInfo(newPath).fileName());
            installedModsList->blockSignals(false);
        } else {
            // Revertir el checkbox si falló el rename
            installedModsList->blockSignals(true);
            changedItem->setCheckState(enable ? Qt::Unchecked : Qt::Checked);
            installedModsList->blockSignals(false);
            QMessageBox::critical(this, tr("Error"),
                tr("Could not change lib state: ") + name);
        }
    });
}

void TrinitoWindow::onInstallShaderpackClicked() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("Minecraft Shaderpacks (*.mcpack)");
    dialog.setDirectory(QDir::homePath());

    if (dialog.exec()) {
        QString filePath = dialog.selectedFiles().first();

        if (filePath.isEmpty())
            return;

        QString shadersDir = getShadersDir(); // Detectar carpeta correcta
        QDir().mkpath(shadersDir);

        QString tempDirPath =
            QDir::tempPath() + "/shaderpack_extract_" +
            QString::number(QRandomGenerator::global()->bounded(INT_MAX));
        QDir().mkpath(tempDirPath);

        // Extraer .mcpack con unzip
        QProcess process;
        process.start("unzip", QStringList()
                                   << filePath << "-d" << tempDirPath);
        process.waitForFinished();

        if (process.exitCode() != 0) {
            QMessageBox::critical(this, "Error",
                                  tr("Could not extract .mcpack file."));
            QDir(tempDirPath).removeRecursively(); // Limpiar
            return;
        }

        // Buscar recursivamente todos los .material.bin en la estructura
        QDirIterator it(tempDirPath, QStringList() << "*.material.bin",
                        QDir::Files, QDirIterator::Subdirectories);
        QStringList materialBins;

        while (it.hasNext()) {
            materialBins << it.next();
        }

        // Copiar cada .material.bin a la carpeta de shaders
        for (const QString &srcPath : materialBins) {
            QFileInfo fileInfo(srcPath);
            QString fileName = fileInfo.fileName();
            QString dstPath = shadersDir + "/" + fileName;

            // Si el archivo ya existe, no lo copiamos (o lo sobrescribimos)
            if (QFile::exists(dstPath)) {
                QFile::remove(dstPath);
            }

            QProcess process;
            process.start("cp", QStringList() << srcPath << dstPath);
            process.waitForFinished();

            if (process.exitCode() != 0) {
                // Verificar si el archivo se creó de todas formas
                if (!QFile::exists(dstPath)) {
                    QMessageBox::warning(
                        this, tr("Warning"),
                        tr("Could not copy ") + fileName + " (output: " +
                            QString::number(process.exitCode()) + ")");
                }
                // Si existe, ignoramos el error
            }
        }

        // Limpiar directorio temporal
        QDir(tempDirPath).removeRecursively();

        QMessageBox::information(this, tr("Success"),
                                 tr("Shaderpack installed successfully."));
        populateInstalledShaders(); // Actualizar lista
    }
}

void TrinitoWindow::onRemoveShaderpackClicked() {
    QString shadersDir = getShadersDir();
    QDir dir(shadersDir);

    if (!dir.exists()) {
        QMessageBox::information(this, tr("Info"),
                                 tr("No shaders installed."));
        return;
    }

    QFileInfoList files = dir.entryInfoList(
        QStringList() << "*.material.bin", QDir::Files);

    if (files.isEmpty()) {
        QMessageBox::information(this, tr("Info"),
                                 tr("No shaders installed."));
        return;
    }

    int r = QMessageBox::warning(
        this, tr("Confirm"),
        tr("Are you sure you want to delete ALL installed shaders (%1 files)?\n"
           "This action cannot be undone.").arg(files.size()),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (r == QMessageBox::No)
        return;

    int removed = 0, failed = 0;
    for (const QFileInfo &fi : files) {
        if (QFile::remove(fi.absoluteFilePath()))
            ++removed;
        else
            ++failed;
    }

    if (failed == 0) {
        QMessageBox::information(this, tr("Success"),
            tr("%1 shader(s) deleted successfully.").arg(removed));
    } else {
        QMessageBox::warning(this, tr("Warning"),
            tr("%1 shader(s) deleted. %2 could not be deleted.").arg(removed).arg(failed));
    }
    populateInstalledShaders();
}

void TrinitoWindow::onRefreshShaderListClicked() { populateInstalledShaders(); }

void TrinitoWindow::onDownloadModClicked() {
    if (availableModsList->selectedItems().isEmpty()) {
        QMessageBox::warning(
            this, tr("Warning"),
            tr("Please select a lib to download."));
        return;
    }

    QString selected = availableModsList->selectedItems().first()->text();
    // Asegúrate de que la URL sea correcta
    QString url =
        "https://huggingface.co/datasets/JaviercPLUS/mods-mcpe/resolve/main/" +
        selected;

    QString modsDir =
        QDir::homePath() +
        "/.var/app/com.trench.trinity.launcher/data/mcpelauncher/mods";
    QDir().mkpath(modsDir);

    QString destination = modsDir + "/" + selected;

    // Crear diálogo de progreso indeterminado
    QProgressDialog progress(tr("Downloading ") + selected, tr("Cancel"), 0,
                             0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    // Descargar en hilo secundario
    QFuture<void> future = QtConcurrent::run([=]() {
        QProcess process;
        process.start("curl", QStringList()
                                  << "-L" << url << "-o" << destination);
        process.waitForFinished(-1);
    });

    // Esperar a que termine sin congelar la interfaz
    while (!future.isFinished()) {
        QCoreApplication::processEvents(); // Actualizar la interfaz
        QThread::msleep(100);              // Pausa breve
    }

    // Verificar si el archivo se descargó
    if (!QFile::exists(destination)) {
        QMessageBox::critical(this, "Error",
                              tr("Could not download the lib."));
        return;
    }

    progress.close();
    QMessageBox::information(this, tr("Success"),
                             tr("Lib installed successfully."));
    populateInstalledMods();
}

void TrinitoWindow::onRemoveInstalledModClicked() {
    if (installedModsList->selectedItems().isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Please select a lib to delete."));
        return;
    }

    QString selected = installedModsList->selectedItems().first()->text();
    QString modsDir =
        QDir::homePath() +
        "/.var/app/com.trench.trinity.launcher/data/mcpelauncher/mods";
    QString filePath = modsDir + "/" + selected;

    QFile file(filePath);
    if (file.exists() && file.remove()) {
        QMessageBox::information(this, tr("Deleted"),
                                 selected + tr(" has been deleted."));
        populateInstalledMods();
    } else {
        QMessageBox::critical(this, "Error",
                              tr("Could not delete ") + selected);
    }
}

// ──────────────────────────────────────────────
// Tab: Directory
// ──────────────────────────────────────────────

QWidget *TrinitoWindow::createDirectoryTab() {
    auto *widget = new QWidget();
    auto *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(20);

    // Título
    auto *titleLabel = new QLabel(tr("Data Directory"));
    titleLabel->setStyleSheet("font-weight: bold; font-size: 20px; color: #8b5cf6;");
    layout->addWidget(titleLabel);

    auto *descLabel = new QLabel(
        tr("This is where Minecraft Bedrock stores your worlds, packs, and other user data."));
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("font-size: 13px; color: #94a3b8;");
    layout->addWidget(descLabel);

    // Detectar la ruta de datos (mismo patrón que getShadersDir())
    QString flatpakBase = QDir::homePath()
        + "/.var/app/com.trench.trinity.launcher/data/mcpelauncher";
    QString nativeBase  = QDir::homePath() + "/.local/share/mcpelauncher";

    // La ruta que mostraremos
    QString dataPath;
    QString typeLabel;

    if (QDir(flatpakBase).exists()) {
        dataPath  = flatpakBase;
        typeLabel = tr("Flatpak installation");
    } else if (QDir(nativeBase).exists()) {
        dataPath  = nativeBase;
        typeLabel = tr("Native installation");
    } else {
        dataPath  = nativeBase; // Ruta esperada aunque no exista aún
        typeLabel = tr("Not found — the launcher may not have run yet");
    }

    // Card de ruta
    auto *card = new QWidget();
    card->setStyleSheet(
        "QWidget { background-color: #090f20; border: 1px solid #1e293b; "
        "border-radius: 10px; }"
    );
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20, 16, 20, 16);
    cardLayout->setSpacing(8);

    auto *typeLbl = new QLabel(typeLabel);
    typeLbl->setStyleSheet("font-size: 12px; color: #64748b; font-weight: bold; "
                           "letter-spacing: 1px; text-transform: uppercase; background: transparent;");
    cardLayout->addWidget(typeLbl);

    auto *pathLbl = new QLabel(dataPath);
    pathLbl->setWordWrap(true);
    pathLbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
    pathLbl->setCursor(Qt::IBeamCursor);
    pathLbl->setStyleSheet(
        "font-size: 14px; color: #e2e8f0; font-family: monospace; background: transparent;");
    cardLayout->addWidget(pathLbl);

    layout->addWidget(card);

    // Botones
    auto *btnLayout = new QHBoxLayout();

    auto *openBtn = new QPushButton(tr("Open Location"));
    openBtn->setFixedHeight(38);
    openBtn->setCursor(Qt::PointingHandCursor);
    openBtn->setStyleSheet(
        "QPushButton { background-color: #8b5cf6; color: #ffffff; "
        "border-radius: 6px; font-weight: bold; padding: 0 20px; }"
        "QPushButton:hover { background-color: #a78bfa; }");
    connect(openBtn, &QPushButton::clicked, this, [dataPath]() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(dataPath));
    });

    auto *copyBtn = new QPushButton(tr("Copy Path"));
    copyBtn->setFixedHeight(38);
    copyBtn->setCursor(Qt::PointingHandCursor);
    connect(copyBtn, &QPushButton::clicked, this, [dataPath, copyBtn]() {
        QApplication::clipboard()->setText(dataPath);
        copyBtn->setText(tr("✓ Copied!"));
        QTimer::singleShot(1500, copyBtn, [copyBtn]() {
            copyBtn->setText(tr("Copy Path"));
        });
    });

    btnLayout->addWidget(openBtn);
    btnLayout->addWidget(copyBtn);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    layout->addStretch();
    return widget;
}

