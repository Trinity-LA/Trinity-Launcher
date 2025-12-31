#ifndef PACK_INSTALLER_H
#define PACK_INSTALLER_H

#include <QString>
#include <QObject>

class PackInstaller : public QObject {
    Q_OBJECT

public:
    explicit PackInstaller(QObject *parent = nullptr);

    enum InstallResult {
        Success,
        NotFound,
        CreateDirFailed,
        CopyFailed,
        UserCancelled
    };

    // Installs a file or directory to the target subdirectory in the game folder.
    // Returns true if successful, false otherwise.
    // If overwrite is needed, it might need interaction or a flag.
    // For logic separation, we should probably pass a callback or flag for overwrite.
    // But since the UI handled the dialog, maybe we just return a status indicating "Exists"
    // and let the UI ask, then call again with overwrite=true?
    // Or we can pass a std::function for query?
    // For simplicity, let's add a force flag.
    
    bool installItem(const QString &sourcePath, const QString &targetSubdir, bool forceOverwrite, QString &errorMsg);
    
    bool itemExists(const QString &sourcePath, const QString &targetSubdir);
    QString getTargetName(const QString &sourcePath);

private:
    QString baseGameDir;
    bool copyDirectory(const QString &srcPath, const QString &dstPath);
};

#endif // PACK_INSTALLER_H
