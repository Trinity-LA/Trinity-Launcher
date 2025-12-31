#ifndef EXTRACT_DIALOG_H
#define EXTRACT_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>

class ExtractDialog : public QDialog {
    Q_OBJECT

public:
    explicit ExtractDialog(QWidget *parent = nullptr);

    QString getApkPath() const;
    QString getVersionName() const;

private:
    QString apkPath;
    QString versionName;
    QLabel *apkLabel;
    QLineEdit *nameEdit;

    void onSelectApk();
};

#endif // EXTRACT_DIALOG_H
