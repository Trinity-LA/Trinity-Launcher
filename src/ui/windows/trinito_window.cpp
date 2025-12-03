#include "trinito_window.h"
#include "../../core/pack_installer.h"

#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>

TrinitoWindow::TrinitoWindow(QWidget *parent) : QWidget(parent) {
  setWindowTitle("Trinito — Gestor de Contenido para Bedrock");
  resize(600, 400);

  // Global Dark Theme (Violet Accent)
  setStyleSheet(
      "QWidget { background-color: #020617; color: #ffffff; font-family: "
      "'Inter', 'Roboto', sans-serif; }"
      "QTabWidget::pane { border: 1px solid #1e293b; background-color: "
      "#090f20; border-radius: 8px; top: -1px; }"
      "QTabBar::tab { background: #1e293b; color: #94a3b8; padding: 10px 20px; "
      "border-top-left-radius: 6px; border-top-right-radius: 6px; "
      "margin-right: 4px; border: none; }"
      "QTabBar::tab:selected { background: #8b5cf6; color: #ffffff; }"
      "QTabBar::tab:hover { background: #334155; }"
      "QPushButton { background-color: #1e293b; border: none; border-radius: "
      "6px; padding: 8px 16px; color: #ffffff; font-weight: bold; }"
      "QPushButton:hover { background-color: #334155; }"
      "QPushButton:pressed { background-color: #0f172a; }"
      "QLabel { color: #e2e8f0; font-size: 14px; margin-bottom: 5px; }");

  auto *layout = new QVBoxLayout(this);
  QTabWidget *tabs = new QTabWidget();
  layout->addWidget(tabs);

  tabs->addTab(createPackTab("behavior_packs", "Behavior Pack (mods)"), "Mods");
  tabs->addTab(createPackTab("resource_packs", "Resource Pack (texturas)"),
               "Texturas");
  tabs->addTab(createDevTab(), "Desarrollo");
  tabs->addTab(createWorldTab(), "Mundos");
}

QWidget *TrinitoWindow::createPackTab(const QString &targetSubdir,
                                      const QString &labelText) {
  auto *widget = new QWidget();
  auto *layout = new QVBoxLayout(widget);

  auto *label = new QLabel(labelText);
  layout->addWidget(label);

  auto *button = new QPushButton("Seleccionar archivo...");
  layout->addWidget(button);

  connect(button, &QPushButton::clicked, this, [=]() {
    QString path = QFileDialog::getOpenFileName(
        this, "Seleccionar pack", QDir::homePath(),
        "Archivos compatibles (*.zip *.mcpack);;Todos los archivos (*)");
    if (!path.isEmpty()) {
      installItem(path, targetSubdir);
    }
  });

  layout->addStretch();
  return widget;
}

QWidget *TrinitoWindow::createDevTab() {
  auto *widget = new QWidget();
  auto *layout = new QVBoxLayout(widget);

  auto *behButton =
      new QPushButton("Seleccionar Development Behavior Pack (archivo)...");
  connect(behButton, &QPushButton::clicked, this, [=]() {
    QString path = QFileDialog::getOpenFileName(
        this, "Seleccionar Development Behavior Pack", QDir::homePath(),
        "Archivos compatibles (*.zip *.mcpack);;Todos los archivos (*)");
    if (!path.isEmpty()) {
      installItem(path, "development_behavior_packs");
    }
  });
  layout->addWidget(new QLabel("Development Behavior Pack:"));
  layout->addWidget(behButton);

  layout->addSpacing(15);

  auto *resButton =
      new QPushButton("Seleccionar Development Resource Pack (archivo)...");
  connect(resButton, &QPushButton::clicked, this, [=]() {
    QString path = QFileDialog::getOpenFileName(
        this, "Seleccionar Development Resource Pack", QDir::homePath(),
        "Archivos compatibles (*.zip *.mcpack);;Todos los archivos (*)");
    if (!path.isEmpty()) {
      installItem(path, "development_resource_packs");
    }
  });
  layout->addWidget(new QLabel("Development Resource Pack:"));
  layout->addWidget(resButton);

  layout->addStretch();
  return widget;
}

QWidget *TrinitoWindow::createWorldTab() {
  auto *widget = new QWidget();
  auto *layout = new QVBoxLayout(widget);

  auto *label = new QLabel("Instalar mundo guardado (carpeta de mundo):");
  layout->addWidget(label);

  auto *button = new QPushButton("Seleccionar carpeta del mundo...");
  layout->addWidget(button);

  connect(button, &QPushButton::clicked, this, [=]() {
    QString path = QFileDialog::getExistingDirectory(
        this, "Seleccionar carpeta del mundo", QDir::homePath());
    if (!path.isEmpty()) {
      installItem(path, "minecraftWorlds");
    }
  });

  layout->addStretch();
  return widget;
}

void TrinitoWindow::installItem(const QString &sourcePath,
                                const QString &targetSubdir) {
  PackInstaller installer;

  if (installer.itemExists(sourcePath, targetSubdir)) {
    int r = QMessageBox::warning(
        this, "Advertencia",
        QString("Ya existe un elemento llamado:\n%1\n\n¿Reemplazarlo?")
            .arg(installer.getTargetName(sourcePath)),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (r == QMessageBox::No)
      return;
  }

  QString errorMsg;
  // We pass true for forceOverwrite because we already asked the user if it
  // existed. If it didn't exist, forceOverwrite=true is fine (it just copies).
  // Wait, my PackInstaller logic: if exists and !force, return error. If exists
  // and force, delete and copy. So passing true is correct here after user
  // confirmation.

  if (installer.installItem(sourcePath, targetSubdir, true, errorMsg)) {
    QMessageBox::information(this, "Éxito",
                             QString("¡%1 instalado correctamente en:\n%2")
                                 .arg(installer.getTargetName(sourcePath))
                                 .arg(targetSubdir));
  } else {
    QMessageBox::critical(this, "Error", "Falló la instalación:\n" + errorMsg);
  }
}
