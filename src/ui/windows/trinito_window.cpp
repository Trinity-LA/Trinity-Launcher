#include "trinito_window.h"
#include "../../core/pack_installer.h"

#include <QApplication>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>

TrinitoWindow::TrinitoWindow(QWidget *parent) : QWidget(parent) {
    setWindowTitle("Trinito — Gestor de Contenido para Bedrock");
    resize(500, 220);
    setFixedSize(size());

    auto *layout = new QVBoxLayout(this);
    QTabWidget *tabs = new QTabWidget();
    layout->addWidget(tabs);

    tabs->addTab(createPackTab("behavior_packs", "Behavior Pack (mods)"), "Mods");
    tabs->addTab(createPackTab("resource_packs", "Resource Pack (texturas)"), "Texturas");
    tabs->addTab(createDevTab(), "Desarrollo");
    tabs->addTab(createWorldTab(), "Mundos");
}

QWidget* TrinitoWindow::createPackTab(const QString &targetSubdir, const QString &labelText) {
    auto *widget = new QWidget();
    auto *layout = new QVBoxLayout(widget);

    auto *label = new QLabel(labelText);
    layout->addWidget(label);

    auto *button = new QPushButton("Seleccionar archivo...");
    layout->addWidget(button);

    connect(button, &QPushButton::clicked, this, [=]() {
        QString path = QFileDialog::getOpenFileName(
            this, "Seleccionar pack", QDir::homePath(),
            "Archivos compatibles (*.zip *.mcpack);;Todos los archivos (*)"
        );
        if (!path.isEmpty()) {
            installItem(path, targetSubdir);
        }
    });

    layout->addStretch();
    return widget;
}

QWidget* TrinitoWindow::createDevTab() {
    auto *widget = new QWidget();
    auto *layout = new QVBoxLayout(widget);

    auto *behButton = new QPushButton("Seleccionar Development Behavior Pack (archivo)...");
    connect(behButton, &QPushButton::clicked, this, [=]() {
        QString path = QFileDialog::getOpenFileName(
            this, "Seleccionar Development Behavior Pack", QDir::homePath(),
            "Archivos compatibles (*.zip *.mcpack);;Todos los archivos (*)"
        );
        if (!path.isEmpty()) {
            installItem(path, "development_behavior_packs");
        }
    });
    layout->addWidget(new QLabel("Development Behavior Pack:"));
    layout->addWidget(behButton);

    layout->addSpacing(15);

    auto *resButton = new QPushButton("Seleccionar Development Resource Pack (archivo)...");
    connect(resButton, &QPushButton::clicked, this, [=]() {
        QString path = QFileDialog::getOpenFileName(
            this, "Seleccionar Development Resource Pack", QDir::homePath(),
            "Archivos compatibles (*.zip *.mcpack);;Todos los archivos (*)"
        );
        if (!path.isEmpty()) {
            installItem(path, "development_resource_packs");
        }
    });
    layout->addWidget(new QLabel("Development Resource Pack:"));
    layout->addWidget(resButton);

    layout->addStretch();
    return widget;
}

QWidget* TrinitoWindow::createWorldTab() {
    auto *widget = new QWidget();
    auto *layout = new QVBoxLayout(widget);

    auto *label = new QLabel("Instalar mundo guardado (carpeta de mundo):");
    layout->addWidget(label);

    auto *button = new QPushButton("Seleccionar carpeta del mundo...");
    layout->addWidget(button);

    connect(button, &QPushButton::clicked, this, [=]() {
        QString path = QFileDialog::getExistingDirectory(
            this, "Seleccionar carpeta del mundo", QDir::homePath()
        );
        if (!path.isEmpty()) {
            installItem(path, "minecraftWorlds");
        }
    });

    layout->addStretch();
    return widget;
}

void TrinitoWindow::installItem(const QString &sourcePath, const QString &targetSubdir) {
    PackInstaller installer;
    
    if (installer.itemExists(sourcePath, targetSubdir)) {
        int r = QMessageBox::warning(this, "Advertencia",
            QString("Ya existe un elemento llamado:\n%1\n\n¿Reemplazarlo?").arg(installer.getTargetName(sourcePath)),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No
        );
        if (r == QMessageBox::No) return;
    }

    QString errorMsg;
    // We pass true for forceOverwrite because we already asked the user if it existed.
    // If it didn't exist, forceOverwrite=true is fine (it just copies).
    // Wait, my PackInstaller logic: if exists and !force, return error. If exists and force, delete and copy.
    // So passing true is correct here after user confirmation.
    
    if (installer.installItem(sourcePath, targetSubdir, true, errorMsg)) {
        QMessageBox::information(this, "Éxito",
            QString("¡%1 instalado correctamente en:\n%2")
            .arg(installer.getTargetName(sourcePath)).arg(targetSubdir));
    } else {
        QMessageBox::critical(this, "Error", "Falló la instalación:\n" + errorMsg);
    }
}
