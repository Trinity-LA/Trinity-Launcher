#include "TrinityLib/ui/windows/trinito_window.hpp"
#include "TrinityLib/core/pack_installer.hpp"

#include <QApplication>
#include <QCheckBox>
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

#include <QRandomGenerator>
#include <QProgressDialog>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

TrinitoWindow::TrinitoWindow(QWidget *parent)
    : QWidget(parent) {
    setWindowTitle(tr(" Gestor de Contenido para Bedrock"));
    resize(820, 500);

    // Global Dark Theme (Violet Accent)
    setStyleSheet(
        "QWidget { background-color: #020617; color: #ffffff; font-family: "
        "'Inter', 'Roboto', sans-serif; }"
        "QTabWidget::pane { border: 1px solid #1e293b; background-color: "
        "#090f20; border-radius: 8px; top: -1px; }"
        "QTabBar::tab { background: #1e293b; color: #94a3b8; padding: 10px "
        "20px; "
        "border-top-left-radius: 6px; border-top-right-radius: 6px; "
        "margin-right: 4px; border: none; }"
        "QTabBar::tab:selected { background: #8b5cf6; color: #ffffff; }"
        "QTabBar::tab:hover { background: #334155; }"
        "QPushButton { background-color: #1e293b; border: none; border-radius: "
        "6px; padding: 8px 16px; color: #ffffff; font-weight: bold; }"
        "QPushButton:hover { background-color: #334155; }"
        "QPushButton:pressed { background-color: #0f172a; }"
        "QLabel { color: #e2e8f0; font-size: 14px; margin-bottom: 5px; }"
        "QListWidget { background-color: #090f20; border: 1px solid #1e293b; "
        "border-radius: 8px; padding: 5px; outline: 0; }"
        "QListWidget::item { padding: 10px; border-radius: 5px; "
        "margin-bottom: 5px; border: none; }"
        "QListWidget::item:selected { background-color: #8b5cf6; "
        "color: #ffffff; }"
        "QListWidget::item:hover { background-color: #1e293b; }");

    auto *layout = new QVBoxLayout(this);
    QTabWidget *tabs = new QTabWidget();
    layout->addWidget(tabs);

    // Pestañas de instalación (como antes)
    tabs->addTab(createPackTab("behavior_packs", "Behavior Pack (mods)"),
                 "Mods");
    tabs->addTab(createPackTab("resource_packs", "Resource Pack"),
                 tr("Texturas"));
    tabs->addTab(createDevTab(), tr("Desarrollo"));
    tabs->addTab(createWorldTab(), tr("Mundos"));
    // Añadir la nueva pestaña de Shaders/Mods
    tabs->addTab(createShadersModsTab(), "Shaders/Libs");
}

QWidget *TrinitoWindow::createManageTab(const QString &packType,
                                        const QString &displayName) {
    auto *widget = new QWidget();
    auto *layout = new QVBoxLayout(widget);

    QLabel *label = new QLabel(tr("Lista de %1 instalados:").arg(displayName));
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
    QPushButton *refreshButton = new QPushButton(tr("Recargar Lista"));
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
    QLabel *installLabel = new QLabel(tr("Instalar nuevo ") + labelText + ":");
    layout->addWidget(installLabel);

    auto *installButton = new QPushButton(tr("Seleccionar archivo..."));
    layout->addWidget(installButton);

    connect(installButton, &QPushButton::clicked, this, [=, this]() {
        QString path = QFileDialog::getOpenFileName(
            this, tr("Seleccionar pack"), QDir::homePath(),
            tr("Archivos compatibles (*.zip *.mcpack);;Todos los archivos "
               "(*)"));
        if (!path.isEmpty()) {
            installItem(path, targetSubdir);
        }
    });

    layout->addSpacing(15);

    // Sección de gestión
    QLabel *manageLabel =
        new QLabel(tr("Gestionar ") + labelText + tr(" instalados:"));
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
    QPushButton *refreshButton = new QPushButton(tr("Recargar Lista"));
    connect(refreshButton, &QPushButton::clicked, this,
            [=, this]() { loadPacks(targetSubdir, listWidget); });
    layout->addWidget(refreshButton);

    // Botón para eliminar seleccionado
    QPushButton *deleteButton = new QPushButton(tr("Eliminar Seleccionado"));
    connect(deleteButton, &QPushButton::clicked, this, [=, this]() {
        if (listWidget->selectedItems().isEmpty()) {
            QMessageBox::warning(this, tr("Advertencia"),
                                 tr("No hay ningún elemento seleccionado."));
            return;
        }

        QString selectedEntry = listWidget->selectedItems().first()->text();
        int r = QMessageBox::warning(
            this, tr("Advertencia"),
            QString(tr("¿Estás seguro de eliminar '%1'?\nEsta acción no se "
                       "puede deshacer."))
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
                this, tr("Éxito"),
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
    QLabel *titleLabel = new QLabel("Development Packs");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 16px;");
    layout->addWidget(titleLabel);

    // Contenedor horizontal para los dos botones de instalación
    auto *buttonLayout = new QHBoxLayout();

    // Botón para Development Behavior Pack
    auto *behButton =
        new QPushButton(tr("Añadir Development Behavior Pack (archivo)..."));
    connect(behButton, &QPushButton::clicked, this, [=, this]() {
        QString path = QFileDialog::getOpenFileName(
            this, tr("Añadir Development Behavior Pack"), QDir::homePath(),
            tr("Archivos compatibles (*.zip *.mcpack);;Todos los archivos "
               "(*)"));
        if (!path.isEmpty()) {
            installItem(path, "development_behavior_packs");
        }
    });
    buttonLayout->addWidget(behButton);

    // Botón para Development Resource Pack
    auto *resButton =
        new QPushButton(tr("Añadir Development Resource Pack (archivo)..."));
    connect(resButton, &QPushButton::clicked, this, [=, this]() {
        QString path = QFileDialog::getOpenFileName(
            this, tr("Añadir Development Resource Pack"), QDir::homePath(),
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
    QLabel *manageLabel = new QLabel(tr("Gestionar Development Packs:"));
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
    QPushButton *refreshButton = new QPushButton(tr("Recargar Listas"));
    connect(refreshButton, &QPushButton::clicked, this, [=, this]() {
        loadPacks("development_behavior_packs", behListWidget);
        loadPacks("development_resource_packs", resListWidget);
    });
    layout->addWidget(refreshButton);

    // Contenedor horizontal para los dos botones de eliminar
    auto *deleteLayout = new QHBoxLayout();

    // Botón para eliminar un pack seleccionado en la lista de Behavior Packs
    QPushButton *deleteBehButton =
        new QPushButton(tr("Eliminar Behavior Pack Seleccionado"));
    connect(deleteBehButton, &QPushButton::clicked, this, [=, this]() {
        if (behListWidget->selectedItems().isEmpty()) {
            QMessageBox::warning(
                this, tr("Advertencia"),
                tr("No hay ningún Behavior Pack seleccionado."));
            return;
        }

        QString selectedEntry = behListWidget->selectedItems().first()->text();
        int r = QMessageBox::warning(
            this, tr("Advertencia"),
            QString(tr("¿Estás seguro de eliminar el Behavior Pack '%1'?\nEsta "
                       "acción no se puede deshacer."))
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
                this, tr("Éxito"),
                QString(tr("eliminado correctamente.")).arg(selectedEntry));
            // Recargar la lista
            loadPacks("development_behavior_packs", behListWidget);
        } else {
            QMessageBox::critical(
                this, "Error",
                QString(tr("No se pudo eliminar")).arg(selectedEntry));
        }
    });
    deleteLayout->addWidget(deleteBehButton);

    // Botón para eliminar un pack seleccionado en la lista de Resource Packs
    QPushButton *deleteResButton =
        new QPushButton(tr("Eliminar Resource Pack Seleccionado"));
    connect(deleteResButton, &QPushButton::clicked, this, [=, this]() {
        if (resListWidget->selectedItems().isEmpty()) {
            QMessageBox::warning(
                this, tr("Advertencia"),
                tr("No hay ningún Resource Pack seleccionado."));
            return;
        }

        QString selectedEntry = resListWidget->selectedItems().first()->text();
        int r = QMessageBox::warning(
            this, tr("Advertencia"),
            QString(tr("¿Estás seguro de eliminar el Resource Pack '%1'?\nEsta "
                       "acción no se puede deshacer."))
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
                this, tr("Éxito"),
                QString(tr("eliminado correctamente.")).arg(selectedEntry));
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
    QLabel *titleLabel = new QLabel(tr("Mundos Guardados"));
    titleLabel->setStyleSheet("font-weight: bold; font-size: 16px;");
    layout->addWidget(titleLabel);

    // Botón para seleccionar carpeta del mundo
    auto *button = new QPushButton(tr("Añadir carpeta del mundo..."));
    layout->addWidget(button);

    connect(button, &QPushButton::clicked, this, [=, this]() {
        QString path = QFileDialog::getExistingDirectory(
            this, tr("Seleccionar carpeta del mundo"), QDir::homePath());
        if (!path.isEmpty()) {
            installItem(path, "minecraftWorlds");
        }
    });

    layout->addSpacing(15);

    // Sección de gestión
    QLabel *manageLabel = new QLabel(tr("Gestionar Mundos:"));
    manageLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    layout->addWidget(manageLabel);

    QListWidget *listWidget = new QListWidget();
    layout->addWidget(listWidget);

    // Asignar a variable miembro
    mapsList = listWidget;

    // Cargar mundos al mostrar la pestaña
    loadPacks("minecraftWorlds", listWidget);

    // Botón para recargar la lista
    QPushButton *refreshButton = new QPushButton(tr("Recargar Lista"));
    connect(refreshButton, &QPushButton::clicked, this,
            [=, this]() { loadPacks("minecraftWorlds", listWidget); });
    layout->addWidget(refreshButton);

    // Botón para borrar un mundo seleccionado
    QPushButton *deleteButton =
        new QPushButton(tr("Borrar Mundo Seleccionado"));
    connect(deleteButton, &QPushButton::clicked, this, [=, this]() {
        if (listWidget->selectedItems().isEmpty()) {
            QMessageBox::warning(this, tr("Advertencia"),
                                 tr("No hay ningún mundo seleccionado."));
            return;
        }

        QString selectedWorld = listWidget->selectedItems().first()->text();
        int r = QMessageBox::warning(
            this, tr("Advertencia"),
            QString(tr("¿Estás seguro de eliminar el mundo '%1'?\nEsta acción "
                       "no 0se puede deshacer."))
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
            QMessageBox::information(this, tr("Éxito"),
                                     tr("Mundo eliminado correctamente."));
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
            this, tr("Advertencia"),
            QString(tr("Ya existe un elemento llamado:\n%1\n\n¿Reemplazarlo?"))
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
            this, tr("Éxito"),
            QString(tr("¡%1 instalado correctamente en:\n%2"))
                .arg(installer.getTargetName(sourcePath))
                .arg(targetSubdir));
    } else {
        QMessageBox::critical(this, "Error", tr("Falló la instalación:\n") + errorMsg);
    }
}

// --- NUEVAS FUNCIONES PARA SHADERS/MODS ---

// Función auxiliar para obtener directorio de shaders
QString TrinitoWindow::getShadersDir() {
    QString flatpakDir = QDir::homePath() + "/.var/app/com.trench.trinity.launcher/data/mcpelauncher";
    QString shadersDir = flatpakDir + "/shaders";

    // Si la carpeta base de Trinity Flatpak existe, usamos shaders dentro de ella
    if (QDir(flatpakDir).exists()) {
        return shadersDir;
    } else {
        // Si no, usamos la carpeta local
        return QDir::homePath() + "/.local/share/mcpelauncher/shaders";
    }
}

QWidget *TrinitoWindow::createShadersModsTab() {
    auto *widget = new QWidget();
    auto *layout = new QVBoxLayout(widget);

    // Horizontal layout to split into left (Shaders) and right (Libs)
    auto *mainSplitLayout = new QHBoxLayout();

    // --- Left: Shaders Section ---
    auto *shadersSection = new QWidget();
    auto *shadersLayout = new QVBoxLayout(shadersSection);
    shadersLayout->setSpacing(10);

    QLabel *shadersTitle = new QLabel(tr("Gestionar Shaders:"));
    shadersTitle->setStyleSheet("font-weight: bold; font-size: 14px;");
    shadersLayout->addWidget(shadersTitle);

    shadersList = new QListWidget();
    shadersLayout->addWidget(shadersList);

    auto *shadersButtonsLayout = new QHBoxLayout();
    installShaderpackButton = new QPushButton(tr("Instalar Shaderpack..."));
    removeShaderpackButton = new QPushButton(tr("Eliminar Shaderpack Seleccionado"));
    refreshShaderListButton = new QPushButton(tr("Actualizar Lista"));
    shadersButtonsLayout->addWidget(installShaderpackButton);
    shadersButtonsLayout->addWidget(removeShaderpackButton);
    shadersButtonsLayout->addWidget(refreshShaderListButton);
    shadersLayout->addLayout(shadersButtonsLayout);

    mainSplitLayout->addWidget(shadersSection);

    // --- Right: Libs Section ---
    auto *libsSection = new QWidget();
    auto *libsLayout = new QVBoxLayout(libsSection);
    libsLayout->setSpacing(10);

    QLabel *libsTitle = new QLabel(tr("Gestionar Libs:")); // Cambiado Mods por Libs
    libsTitle->setStyleSheet("font-weight: bold; font-size: 14px;");
    libsLayout->addWidget(libsTitle);

    // Available Libs
    QLabel *availableLibsLabel = new QLabel(tr("Libs Disponibles:")); // Cambiado
    libsLayout->addWidget(availableLibsLabel);
    availableModsList = new QListWidget(); // El nombre de la variable puede mantenerse por simplicidad interna
    libsLayout->addWidget(availableModsList);

    downloadModButton = new QPushButton(tr("Descargar Lib Seleccionada")); // Cambiado
    libsLayout->addWidget(downloadModButton);

    // Installed Libs
    QLabel *installedLibsLabel = new QLabel(tr("Libs Instaladas:")); // Cambiado
    libsLayout->addWidget(installedLibsLabel);
    installedModsList = new QListWidget(); // El nombre de la variable puede mantenerse por simplicidad interna
    libsLayout->addWidget(installedModsList);

    removeInstalledModButton = new QPushButton(tr("Eliminar Lib Seleccionada")); // Cambiado
    libsLayout->addWidget(removeInstalledModButton);

    mainSplitLayout->addWidget(libsSection);

    layout->addLayout(mainSplitLayout);

    // Connect signals (las funciones miembro siguen siendo las mismas, solo cambia la UI)
    connect(installShaderpackButton, &QPushButton::clicked, this, &TrinitoWindow::onInstallShaderpackClicked);
    connect(removeShaderpackButton, &QPushButton::clicked, this, &TrinitoWindow::onRemoveShaderpackClicked);
    connect(refreshShaderListButton, &QPushButton::clicked, this, &TrinitoWindow::onRefreshShaderListClicked);
    connect(downloadModButton, &QPushButton::clicked, this, &TrinitoWindow::onDownloadModClicked);
    connect(removeInstalledModButton, &QPushButton::clicked, this, &TrinitoWindow::onRemoveInstalledModClicked);

    // Initialize data
    populateInstalledShaders();
    populateAvailableMods(); // Esta función seguirá cargando la lista de "mods" disponibles, pero ahora se mostrará como "libs"
    populateInstalledMods(); // Esta función seguirá cargando la lista de "mods" instalados, pero ahora se mostrará como "libs"

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

    QFileInfoList files = dir.entryInfoList(QStringList() << "*.material.bin", QDir::Files);
    for (const QFileInfo &file : files) {
        shadersList->addItem(file.fileName());
    }
}

void TrinitoWindow::populateAvailableMods() {
    QStringList availableMods = {
        "libmcpelaunchershadersmod.so",
        "libmcpelauncherdcblock.so",
        "libmcpelauncherlegacyx86_64.so",
        "libmcpelauncherlegacyarm64-v8a.so",
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
    QString modsDir = QDir::homePath() + "/.var/app/com.trench.trinity.launcher/data/mcpelauncher/mods";
    QDir dir(modsDir);

    installedModsList->clear();

    if (!dir.exists()) {
        installedModsList->addItem("(0 mods )");
        return;
    }

    QFileInfoList files = dir.entryInfoList(QStringList() << "*.so" << "*.so.disabled", QDir::Files);
    for (const QFileInfo &file : files) {
        installedModsList->addItem(file.fileName());
    }
}

void TrinitoWindow::onInstallShaderpackClicked() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("Minecraft Shaderpacks (*.mcpack)");
    dialog.setDirectory(QDir::homePath());

    if (dialog.exec()) {
        QString filePath = dialog.selectedFiles().first();

        if (filePath.isEmpty()) return;

        QString shadersDir = getShadersDir(); // Detectar carpeta correcta
        QDir().mkpath(shadersDir);

        QString tempDirPath = QDir::tempPath() + "/shaderpack_extract_" + QString::number(QRandomGenerator::global()->bounded(INT_MAX));
        QDir().mkpath(tempDirPath);

        // Extraer .mcpack con unzip
        QProcess process;
        process.start("unzip", QStringList() << filePath << "-d" << tempDirPath);
        process.waitForFinished();

        if (process.exitCode() != 0) {
            QMessageBox::critical(this, "Error", tr("No se pudo extraer el archivo .mcpack."));
            QDir(tempDirPath).removeRecursively(); // Limpiar
            return;
        }

        // Buscar recursivamente todos los .material.bin en la estructura
        QDirIterator it(tempDirPath, QStringList() << "*.material.bin", QDir::Files, QDirIterator::Subdirectories);
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
                    QMessageBox::warning(this, tr("Advertencia"), tr("No se pudo copiar ") + fileName + " (output: " + QString::number(process.exitCode()) + ")");
                }
                // Si existe, ignoramos el error
            }
        }

        // Limpiar directorio temporal
        QDir(tempDirPath).removeRecursively();

        QMessageBox::information(this, tr("Éxito"), tr("Shaderpack instalado correctamente."));
        populateInstalledShaders(); // Actualizar lista
    }
}

void TrinitoWindow::onRemoveShaderpackClicked() {
    if (shadersList->selectedItems().isEmpty()) {
        QMessageBox::warning(this, tr("Advertencia"), tr("No hay ningún shader seleccionado."));
        return;
    }

    QString selectedShader = shadersList->selectedItems().first()->text();
    QString shadersDir = getShadersDir();
    QString shaderPath = shadersDir + "/" + selectedShader;

    if (QFile::remove(shaderPath)) {
        QMessageBox::information(this, tr("Éxito"), tr("Shader eliminado correctamente."));
        populateInstalledShaders(); // Actualizar lista
    } else {
        QMessageBox::critical(this, "Error", tr("No se pudo eliminar el shader."));
    }
}

void TrinitoWindow::onRefreshShaderListClicked() {
    populateInstalledShaders();
}

void TrinitoWindow::onDownloadModClicked() {
    if (availableModsList->selectedItems().isEmpty()) {
        QMessageBox::warning(this, tr("Advertencia"), tr("Por favor, selecciona un mod para descargar."));
        return;
    }

    QString selected = availableModsList->selectedItems().first()->text();
    // Asegúrate de que la URL sea correcta
    QString url = "https://huggingface.co/datasets/JaviercPLUS/mods-mcpe/resolve/main/" + selected;

    QString modsDir = QDir::homePath() + "/.var/app/com.trench.trinity.launcher/data/mcpelauncher/mods";
    QDir().mkpath(modsDir);

    QString destination = modsDir + "/" + selected;

    // Crear diálogo de progreso indeterminado
    QProgressDialog progress(tr("Descargando ") + selected, tr("Cancelar"), 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    // Descargar en hilo secundario
    QFuture<void> future = QtConcurrent::run([=]() {
        QProcess process;
        process.start("curl", QStringList() << "-L" << url << "-o" << destination);
        process.waitForFinished(-1);
    });

    // Esperar a que termine sin congelar la interfaz
    while (!future.isFinished()) {
        QCoreApplication::processEvents(); // Actualizar la interfaz
        QThread::msleep(100); // Pausa breve
    }

    // Verificar si el archivo se descargó
    if (!QFile::exists(destination)) {
        QMessageBox::critical(this, "Error", tr("No se pudo descargar el mod."));
        return;
    }

    progress.close();
    QMessageBox::information(this, tr("Éxito"), tr("Mod instalado correctamente."));
    populateInstalledMods();
}

void TrinitoWindow::onRemoveInstalledModClicked() {
    if (installedModsList->selectedItems().isEmpty()) {
        QMessageBox::warning(this, tr("Advertencia"), tr("Por favor, selecciona un mod para eliminar."));
        return;
    }

    QString selected = installedModsList->selectedItems().first()->text();
    QString modsDir = QDir::homePath() + "/.var/app/com.trench.trinity.launcher/data/mcpelauncher/mods";
    QString filePath = modsDir + "/" + selected;

    QFile file(filePath);
    if (file.exists() && file.remove()) {
        QMessageBox::information(this, tr("Eliminado"), selected + tr(" ha sido eliminado."));
        populateInstalledMods();
    } else {
        QMessageBox::critical(this, "Error", tr("No se pudo eliminar ") + selected);
    }
}
