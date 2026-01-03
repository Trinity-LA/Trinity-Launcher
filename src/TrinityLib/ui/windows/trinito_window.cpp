#include "TrinityLib/ui/windows/trinito_window.hpp"
#include "TrinityLib/core/pack_installer.hpp"

#include <QApplication>
#include <QCheckBox> // Añadir
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QListWidget> // Añadir
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QStandardItemModel> // Añadir
#include <QStandardPaths>
#include <QTabWidget>
#include <QVBoxLayout>

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
            [=]() { loadPacks(packType, listWidget); });
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
            [=](QListWidgetItem *changedItem) {
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

    connect(installButton, &QPushButton::clicked, this, [=]() {
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
            [=]() { loadPacks(targetSubdir, listWidget); });
    layout->addWidget(refreshButton);

    // Botón para eliminar seleccionado
    QPushButton *deleteButton = new QPushButton(tr("Eliminar Seleccionado"));
    connect(deleteButton, &QPushButton::clicked, this, [=]() {
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
    connect(behButton, &QPushButton::clicked, this, [=]() {
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
    connect(resButton, &QPushButton::clicked, this, [=]() {
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
    QPushButton *refreshButton = new QPushButton("Recargar Listas");
    connect(refreshButton, &QPushButton::clicked, this, [=]() {
        loadPacks("development_behavior_packs", behListWidget);
        loadPacks("development_resource_packs", resListWidget);
    });
    layout->addWidget(refreshButton);

    // Contenedor horizontal para los dos botones de eliminar
    auto *deleteLayout = new QHBoxLayout();

    // Botón para eliminar un pack seleccionado en la lista de Behavior Packs
    QPushButton *deleteBehButton =
        new QPushButton("Eliminar Behavior Pack Seleccionado");
    connect(deleteBehButton, &QPushButton::clicked, this, [=]() {
        if (behListWidget->selectedItems().isEmpty()) {
            QMessageBox::warning(this, "Advertencia",
                                 "No hay ningún Behavior Pack seleccionado.");
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
    connect(deleteResButton, &QPushButton::clicked, this, [=]() {
        if (resListWidget->selectedItems().isEmpty()) {
            QMessageBox::warning(this, "Advertencia",
                                 "No hay ningún Resource Pack seleccionado.");
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

    connect(button, &QPushButton::clicked, this, [=]() {
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
            [=]() { loadPacks("minecraftWorlds", listWidget); });
    layout->addWidget(refreshButton);

    // Botón para borrar un mundo seleccionado
    QPushButton *deleteButton =
        new QPushButton(tr("Borrar Mundo Seleccionado"));
    connect(deleteButton, &QPushButton::clicked, this, [=]() {
        if (listWidget->selectedItems().isEmpty()) {
            QMessageBox::warning(this, "Advertencia",
                                 "No hay ningún mundo seleccionado.");
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
        QMessageBox::critical(this, "Error",
                              "Falló la instalación:\n" + errorMsg);
    }
}
