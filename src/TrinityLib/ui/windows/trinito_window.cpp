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

    // First tab: Instances
    tabs->addTab(createInstancesTab(), tr("Instances"));

    // Installation tabs (as before)
    tabs->addTab(createPackTab("behavior_packs", tr("Behavior Pack (mods)")),
                 tr("Mods"));
    tabs->addTab(createPackTab("resource_packs", tr("Resource Pack")),
                 tr("Textures"));
    tabs->addTab(createDevTab(), tr("Development"));
    tabs->addTab(createWorldTab(), tr("Worlds"));
    // Add the new Shaders/Libs tab
    tabs->addTab(createShadersModsTab(), tr("Shaders/Libs"));
    // Data directory tab
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
    listLabel->setStyleSheet("font-size: 16px; background: transparent;");
    headerRow->addWidget(listLabel, 3);

    QLabel *actionsLabel = new QLabel(tr("Actions"));
    actionsLabel->setStyleSheet("font-size: 16px; background: transparent;");
    headerRow->addWidget(actionsLabel, 2);

    outerLayout->addLayout(headerRow);

    // ── Thin separator below headers ──────────────────────────────────────
    auto *sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setObjectName("Divider");
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
    versionsList->setIconSize(QSize(20, 20));
    // ListWidget style handled by applyTheme global stylesheet

    // Refresh function for instances list
    auto refreshInstancesList = [this, versionsList]() {
        versionsList->clear();
        if (m_launcher) {
            VersionManager vm;
            for (const QString &v : vm.getInstalledVersions()) {
                auto *item = new QListWidgetItem(QIcon(":/icons/cube"), v);
                item->setSizeHint(QSize(0, 44));
                versionsList->addItem(item);

                QWidget* rowWidget = new QWidget(versionsList);
                rowWidget->setStyleSheet("background: transparent;");
                auto* rowLayout = new QHBoxLayout(rowWidget);
                rowLayout->setContentsMargins(0, 0, 8, 0);
                rowLayout->addStretch();

                auto* delBtn = new QPushButton();
                delBtn->setIcon(QIcon(":/icons/trash"));
                delBtn->setIconSize(QSize(18, 18));
                delBtn->setFixedSize(23, 23);
                delBtn->setCursor(Qt::PointingHandCursor);
                delBtn->setStyleSheet("QPushButton { border: none; background: transparent; }");

                rowLayout->addWidget(delBtn);
                versionsList->setItemWidget(item, rowWidget);

                connect(delBtn, &QPushButton::clicked, this, [this, v, versionsList, item]() {
                    VersionManager vm;
                    QString errorMsg;
                    if (!vm.deleteVersion(v, errorMsg)) {
                        QMessageBox::critical(this, tr("Error"), tr("Could not delete version:\n") + errorMsg);
                    } else {
                        delete versionsList->takeItem(versionsList->row(item));
                        if (m_launcher) {
                            m_launcher->loadInstalledVersions();
                        }
                    }
                });
            }
        }
    };

    // Populate from VersionManager
    if (m_launcher) {
        refreshInstancesList();
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
        connect(m_launcher, &LauncherWindow::versionsChanged, this, [versionsList, refreshInstancesList]() {
    
refreshInstancesList();
refreshInstancesList();
refreshInstancesList();
            if (versionsList->count() > 0)
                versionsList->setCurrentRow(0);
        });

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
refreshInstancesList();
refreshInstancesList();
refreshInstancesList();
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

    // Assign the list to the corresponding member variable to use it in loadPacks
    if (packType == "behavior_packs") {
        modsList = listWidget;
    } else if (packType == "resource_packs") {
        resourcesList = listWidget;
    } else if (packType == "minecraftWorlds") {
        mapsList = listWidget;
    }

    // Load packs when showing the tab (optional, or upon construction)
    loadPacks(packType, listWidget);

    // Button to reload the list
    QPushButton *refreshButton = new QPushButton(tr("Refresh List"));
    connect(refreshButton, &QPushButton::clicked, this,
            [=]() { loadPacks(packType, listWidget); });
    layout->addWidget(refreshButton);

    return widget;
}

void TrinitoWindow::loadPacks(const QString &packType,
                              QListWidget *listWidget) {
    listWidget->clear(); // Clear current list

    QString baseDataDir =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
        "/mcpelauncher/games/com.mojang";
    QString packDirPath = baseDataDir + "/" + packType;

    QDir packDir(packDirPath);
    if (!packDir.exists()) {
        // QMessageBox::information(this, "Info", "No " +
        // packType + " found.");
        return; // Exit if the folder doesn't exist
    }

    QStringList entries =
        packDir.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);

    for (const QString &entry : entries) {
        QFileInfo info(packDirPath + "/" + entry);
        bool isEnabled = true;

        // Check if it is disabled (renamed)
        if (entry.endsWith(".disabled")) {
            isEnabled = false;
        }

        // Create list item
        QListWidgetItem *item = new QListWidgetItem();
        item->setCheckState(isEnabled ? Qt::Checked : Qt::Unchecked);
        item->setText(entry); // Show original name or with .disabled

        listWidget->addItem(item);
    }

    // Connect the itemChanged signal of the QListWidget to detect changes in the checkboxes
    connect(listWidget, &QListWidget::itemChanged, this,
            [=](QListWidgetItem *changedItem) {
                // Get the current state of the checkbox
                Qt::CheckState state = changedItem->checkState();
                bool newState = (state == Qt::Checked);

                // Get the pack name
                QString packName = changedItem->text();

                // Call togglePack to rename
                togglePack(packType, packName, newState);

                // Optional: Update the displayed name if state changes
                QString newName = packName;
                if (newState) {
                    // If enabled and the name ends in .disabled, remove it
                    if (packName.endsWith(".disabled")) {
                        newName = packName.chopped(9); // .disabled.length() = 9
                    }
                } else {
                    // If disabled and the name doesn't end in .disabled, add it
                    if (!packName.endsWith(".disabled")) {
                        newName = packName + ".disabled";
                    }
                }
                // Update the item text (this could trigger another itemChanged if not handled carefully)
                // Compare newName with the current text before changing it to avoid loops
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
        // Enable: decompress the .disabled file
        if (packName.endsWith(".disabled")) {
            // The newPath is the original name (without .disabled)
            newPath = oldPath.chopped(9); // Removes ".disabled" (length 9)

            // Command: tar -xzf <compressed_file> -C <destination_directory>
            QProcess process;
            process.start("tar", {"-xzf", oldPath, "-C", packDirPath});
            process.waitForFinished(-1);

            if (process.exitCode() == 0) {
                // Delete the .disabled file after decompressing
                QFile::remove(oldPath);
                // QMessageBox::information(this, "Success", QString("Pack '%1'
                // enabled.").arg(packName));
            } else {
                QString err = process.readAllStandardError();
                QMessageBox::critical(
                    this, "Error",
                    QString("Could not enable the pack '%1'.\nError while "
                            "decompressing:\n%2")
                        .arg(packName)
                        .arg(err.isEmpty() ? "Unknown Error." : err));
                return;
            }
        }
        // If it doesn't end in .disabled, do nothing (it's already enabled)
    } else {
        // Disable: compress to .disabled
        if (!packName.endsWith(".disabled")) {
            // The newPath is the original name + .disabled
            newPath = oldPath + ".disabled";

            // Command: tar -czf <output_file> -C <parent_directory>
            // <folder_or_file_name> Example: tar -czf
            // pack1.mcpack.disabled.tar.gz -C /container/path pack1.mcpack
            QFileInfo fileInfo(oldPath);
            QString parentDir =
                fileInfo.absolutePath(); // Parent directory of the pack
            QString baseName =
                fileInfo.fileName(); // Name of the pack (file or folder)

            QProcess process;
            process.start("tar", {"-czf", newPath, "-C", parentDir, baseName});
            process.waitForFinished(-1);

            if (process.exitCode() == 0) {
                // Delete the original file/folder after compressing
                if (QDir(oldPath).exists()) {
                    QDir(oldPath).removeRecursively();
                } else {
                    QFile::remove(oldPath);
                }
                // QMessageBox::information(this, "Success", QString("Pack '%1'
                // disabled.").arg(packName));
            } else {
                QString err = process.readAllStandardError();
                QMessageBox::critical(
                    this, "Error",
                    QString("Could not disable the pack '%1'.\nError while "
                            "compressing:\n%2")
                        .arg(packName)
                        .arg(err.isEmpty() ? "Unknown Error." : err));
                return;
            }
        }
        // If it already ends in .disabled, do nothing (already disabled)
    }

    // Optional: Update the packs list in the UI if you are on the
    // corresponding tab. For example, if you are on the "Manage Mods" tab:
    // if (currentTab == "Manage Mods") {
    //     loadPacks("behavior_packs", modsList);
    // }
    // Or just show a general success message here if you prefer not to constantly reload.
}

// ... rest of the code (createPackTab, createDevTab, createWorldTab,
// installItem) ...

QWidget *TrinitoWindow::createPackTab(const QString &targetSubdir,
                                      const QString &labelText) {
    auto *widget = new QWidget();
    auto *layout = new QVBoxLayout(widget);

    // Title
    QLabel *titleLabel = new QLabel(labelText);
    titleLabel->setObjectName("Title");
    layout->addWidget(titleLabel);

    // Installation section
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

    // Management section
    QLabel *manageLabel =
        new QLabel(tr("Manage ") + labelText + tr(" installed:"));
    layout->addWidget(manageLabel);

    QListWidget *listWidget = new QListWidget();
    layout->addWidget(listWidget);

    // Assign the list to the corresponding member variable to use it in loadPacks
    if (targetSubdir == "behavior_packs") {
        modsList = listWidget;
    } else if (targetSubdir == "resource_packs") {
        resourcesList = listWidget;
    } else if (targetSubdir == "minecraftWorlds") {
        mapsList = listWidget;
    }

    // Load packs when showing the tab
    loadPacks(targetSubdir, listWidget);

    // Button to reload the list
    QPushButton *refreshButton = new QPushButton(tr("Refresh List"));
    connect(refreshButton, &QPushButton::clicked, this,
            [=]() { loadPacks(targetSubdir, listWidget); });
    layout->addWidget(refreshButton);

    // Button to delete selected
    QPushButton *deleteButton = new QPushButton(tr("Delete Selected"));
    connect(deleteButton, &QPushButton::clicked, this, [=]() {
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

        // Delete the file/folder
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
                QString(tr("'%1' deleted successfully."))
                    .arg(selectedEntry));
            // Reload the list
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

    // Title
    QLabel *titleLabel = new QLabel(tr("Development Packs"));
    titleLabel->setObjectName("Title");
    layout->addWidget(titleLabel);

    // Horizontal container for the two installation buttons
    auto *buttonLayout = new QHBoxLayout();

    // Button for Development Behavior Pack
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

    // Button for Development Resource Pack
    auto *resButton =
        new QPushButton(tr("Add Development Resource Pack (file)..."));
    connect(resButton, &QPushButton::clicked, this, [=]() {
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

    // Management section
    QLabel *manageLabel = new QLabel(tr("Manage Development Packs:"));
    // Uses default label color
    layout->addWidget(manageLabel);

    // Create a container for the two lists
    auto *devLayout = new QHBoxLayout();

    // List of Development Behavior Packs
    auto *behListWidget = new QListWidget();
    devLayout->addWidget(behListWidget);
    // Assign to member variable if you need to reload it
    // Not necessary if not reloading here, but for consistency:
    // developmentBehaviorList = behListWidget;

    // List of Development Resource Packs
    auto *resListWidget = new QListWidget();
    devLayout->addWidget(resListWidget);
    // developmentResourceList = resListWidget;

    layout->addLayout(devLayout);

    // Load packs when showing the tab
    loadPacks("development_behavior_packs", behListWidget);
    loadPacks("development_resource_packs", resListWidget);

    // Button to reload the list
    QPushButton *refreshButton = new QPushButton(tr("Refresh Lists"));
    connect(refreshButton, &QPushButton::clicked, this, [=, this]() {
        loadPacks("development_behavior_packs", behListWidget);
        loadPacks("development_resource_packs", resListWidget);
    });
    layout->addWidget(refreshButton);

    // Horizontal container for the two delete buttons
    auto *deleteLayout = new QHBoxLayout();

    // Button to delete a selected pack in the Behavior Packs list
    QPushButton *deleteBehButton =
        new QPushButton(tr("Delete Selected Behavior Pack"));
    connect(deleteBehButton, &QPushButton::clicked, this, [=]() {
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
            // Reload the list
            loadPacks("development_behavior_packs", behListWidget);
        } else {
            QMessageBox::critical(
                this, "Error",
                QString(tr("Could not delete")).arg(selectedEntry));
        }
    });
    deleteLayout->addWidget(deleteBehButton);

    // Button to delete a selected pack in the Resource Packs list
    QPushButton *deleteResButton =
        new QPushButton(tr("Delete Selected Resource Pack"));
    connect(deleteResButton, &QPushButton::clicked, this, [=]() {
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
            // Reload the list
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

    // Title
    QLabel *titleLabel = new QLabel(tr("Saved Worlds"));
    titleLabel->setObjectName("Title");
    layout->addWidget(titleLabel);

    // Button to select world folder
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

    // Management section
    QLabel *manageLabel = new QLabel(tr("Manage Worlds:"));
    // Uses default label color
    layout->addWidget(manageLabel);

    QListWidget *listWidget = new QListWidget();
    layout->addWidget(listWidget);

    // Assign to member variable
    mapsList = listWidget;

    // Load worlds when showing the tab
    loadPacks("minecraftWorlds", listWidget);

    // Button to reload the list
    QPushButton *refreshButton = new QPushButton(tr("Refresh List"));
    connect(refreshButton, &QPushButton::clicked, this,
            [=]() { loadPacks("minecraftWorlds", listWidget); });
    layout->addWidget(refreshButton);

    // Button to delete a selected world
    QPushButton *deleteButton =
        new QPushButton(tr("Delete Selected World"));
    connect(deleteButton, &QPushButton::clicked, this, [=]() {
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

        // Delete the world
        QString baseDataDir = QStandardPaths::writableLocation(
                                  QStandardPaths::GenericDataLocation) +
                              "/mcpelauncher/games/com.mojang";
        QString worldPath = baseDataDir + "/minecraftWorlds/" + selectedWorld;

        if (QDir(worldPath).removeRecursively()) {
            QMessageBox::information(this, tr("Success"),
                                     tr("World deleted successfully."));
            // Update the list
            loadPacks("minecraftWorlds", listWidget);
        } else {
            QMessageBox::critical(this, "Error",
                                  "Could not delete the world.");
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

// --- NEW FUNCTIONS FOR SHADERS/MODS ---

// Helper function to get shaders directory
QString TrinitoWindow::getShadersDir() {
    QString flatpakDir =
        QDir::homePath() +
        "/.var/app/com.trench.trinity.launcher/data/mcpelauncher";
    QString shadersDir = flatpakDir + "/shaders";

    // If the base Trinity Flatpak folder exists, we use shaders inside it
    if (QDir(flatpakDir).exists()) {
        return shadersDir;
    } else {
        // If not, we use the local folder
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
    shadersTitle->setObjectName("Title");
    shadersLayout->addWidget(shadersTitle);

    auto *shadersSep = new QFrame();
    shadersSep->setFrameShape(QFrame::HLine);
    shadersSep->setObjectName("Divider");
    shadersLayout->addWidget(shadersSep);

    shadersList = new QListWidget();
    shadersLayout->addWidget(shadersList, 1); // stretch=1 → take available space

    // Shader buttons in column
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
    libsTitle->setObjectName("Title");
    libsLayout->addWidget(libsTitle);

    auto *libsSep = new QFrame();
    libsSep->setFrameShape(QFrame::HLine);
    libsSep->setObjectName("Divider");
    libsLayout->addWidget(libsSep);

    // Available Libs
    auto *availableLibsLabel = new QLabel(tr("Available libs:"));
    availableLibsLabel->setStyleSheet("font-size: 16px; background: transparent;");
    libsLayout->addWidget(availableLibsLabel);

    availableModsList = new QListWidget();
    libsLayout->addWidget(availableModsList, 1);

    downloadModButton = new QPushButton(tr("Download Selected Lib"));
    downloadModButton->setObjectName("ActionButton");
    downloadModButton->setCursor(Qt::PointingHandCursor);
    downloadModButton->setMinimumHeight(36);
    libsLayout->addWidget(downloadModButton);

    libsLayout->addSpacing(8);

    // Installed Libs
    auto *installedLibsLabel = new QLabel(tr("Installed libs (✓ = active):"));
    installedLibsLabel->setStyleSheet("font-size: 16px; background: transparent;");
    libsLayout->addWidget(installedLibsLabel);

    installedModsList = new QListWidget();
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
    QString shadersDir = getShadersDir(); // Detect correct folder
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
    // Libs without any that contains "arm" in the name
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

    // Disconnect to avoid triggering while we fill the list
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

    // Connect the checkbox toggle (rename on disk)
    // Disconnect first if it was already connected to avoid duplicate connections
    disconnect(installedModsList, &QListWidget::itemChanged, nullptr, nullptr);
    connect(installedModsList, &QListWidget::itemChanged, this,
            [this, modsDir](QListWidgetItem *changedItem) {
        bool enable = (changedItem->checkState() == Qt::Checked);
        QString name = changedItem->text();
        QString oldPath = modsDir + "/" + name;
        QString newPath;

        if (enable && name.endsWith(".disabled")) {
            newPath = modsDir + "/" + name.chopped(9); // removes ".disabled"
        } else if (!enable && !name.endsWith(".disabled")) {
            newPath = modsDir + "/" + name + ".disabled";
        } else {
            return; // no change needed
        }

        if (QFile::rename(oldPath, newPath)) {
            // Update the item text without re-triggering signals
            installedModsList->blockSignals(true);
            changedItem->setText(QFileInfo(newPath).fileName());
            installedModsList->blockSignals(false);
        } else {
            // Revert the checkbox if rename failed
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

        QString shadersDir = getShadersDir(); // Detect correct folder
        QDir().mkpath(shadersDir);

        QString tempDirPath =
            QDir::tempPath() + "/shaderpack_extract_" +
            QString::number(QRandomGenerator::global()->bounded(INT_MAX));
        QDir().mkpath(tempDirPath);

        // Extract .mcpack with unzip
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
    titleLabel->setObjectName("Title");
    layout->addWidget(titleLabel);

    auto *descLabel = new QLabel(
        tr("This is where Minecraft Bedrock stores your worlds, packs, and other user data."));
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("font-size: 16px; background: transparent;");
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
    card->setObjectName("ContextPanel");
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20, 16, 20, 16);
    cardLayout->setSpacing(8);

    auto *typeLbl = new QLabel(typeLabel);
    typeLbl->setStyleSheet("font-size: 16px; font-weight: bold; "
                           "letter-spacing: 1px; text-transform: uppercase; background: transparent;");
    cardLayout->addWidget(typeLbl);

    auto *pathLbl = new QLabel(dataPath);
    pathLbl->setWordWrap(true);
    pathLbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
    pathLbl->setCursor(Qt::IBeamCursor);
    pathLbl->setStyleSheet(
        "font-size: 16px; font-family: monospace; background: transparent;");
    cardLayout->addWidget(pathLbl);

    layout->addWidget(card);

    // Botones
    auto *btnLayout = new QHBoxLayout();

    auto *openBtn = new QPushButton(tr("Open Location"));
    openBtn->setFixedHeight(38);
    openBtn->setCursor(Qt::PointingHandCursor);
    openBtn->setObjectName("ActionButton");
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

