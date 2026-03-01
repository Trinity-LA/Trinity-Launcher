#include "TrinityLib/ui/dialogs/extract_dialog.hpp"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>

ExtractDialog::ExtractDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("New version from APK"));
    resize(400, 150);

    auto *layout = new QFormLayout(this);

    auto *apkButton = new QPushButton(tr("Select APK..."));
    apkLabel = new QLabel(tr("No file selected"));
    apkLabel->setWordWrap(true);

    connect(apkButton, &QPushButton::clicked, this, &ExtractDialog::onSelectApk);

    nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("Ej: 1.21.0");
    connect(nameEdit, &QLineEdit::textChanged, [this](const QString &text) { versionName = text.trimmed(); });

    layout->addRow("APK:", apkButton);
    layout->addRow(apkLabel);
    layout->addRow(tr("Version name:"), nameEdit);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if (apkPath.isEmpty()) {
            QMessageBox::warning(this, tr("Error"), tr("Please select an APK file."));
            return;
        }
        if (versionName.isEmpty()) {
            QMessageBox::warning(this, tr("Error"), tr("Please enter a version name."));
            return;
        }
        accept();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void ExtractDialog::onSelectApk() {
    QString file = QFileDialog::getOpenFileName(this, tr("Select Minecraft APK"), QDir::homePath(), tr("APK files (*.apk)"));
    if (!file.isEmpty()) {
        apkPath = file;
        apkLabel->setText(QFileInfo(file).fileName());
    }
}

QString ExtractDialog::getApkPath() const {
    return apkPath;
}

QString ExtractDialog::getVersionName() const {
    return versionName;
}
