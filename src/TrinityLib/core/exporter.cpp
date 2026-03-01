#include "TrinityLib/core/exporter.hpp"
#include "TrinityLib/core/version_manager.hpp"
#include <QApplication>
#include <QDialog>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QMessageBox>
#include <QMetaObject>
#include <QProcess>
#include <QProgressBar>
#include <QStandardPaths>
#include <QString>
#include <QTemporaryDir>
#include <QThread>
#include <QVBoxLayout>

Exporter::Exporter(QWidget *parent)
    : QObject(parent),
      parentWidget(parent) {}

void Exporter::exportVersion(const QString &versionName) {
    VersionManager vm;
    QString versionPath = vm.getVersionPath(versionName);

    if (!QDir(versionPath).exists()) {
        QMessageBox::critical(parentWidget, tr("Error"),
                              tr("Version does not exist."));
        return;
    }

    // Preguntar si exportar con datos del APK
    int r = QMessageBox::question(
        parentWidget, tr("Export version"),
        QString(tr("Export '%1' with APK data?\n\n"
                   "Yes: Includes the full version (game data).\n"
                   "No: Only exports mods, maps, etc."))
            .arg(versionName),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    bool exportWithApk = (r == QMessageBox::Yes);

    QString zipPath = QFileDialog::getSaveFileName(
        parentWidget, tr("Save as file"), versionName + ".tar.gz",
        tr("TAR files (*.tar.gz);;All files (*)"));
    if (zipPath.isEmpty())
        return;

    // Crear diálogo de progreso
    QDialog progressDlg(parentWidget);
    progressDlg.setWindowTitle(tr("Exporting..."));
    progressDlg.setFixedSize(300, 100);

    auto *layout = new QVBoxLayout(&progressDlg);
    QLabel *label = new QLabel(tr("Exporting version..."));
    QProgressBar *progressBar = new QProgressBar();
    progressBar->setRange(0, 0); // Indefinido
    layout->addWidget(label);
    layout->addWidget(progressBar);

    progressDlg.show();
    QApplication::processEvents(); // Actualizar UI

    // Crear directorio temporal
    QString tempDir = QDir::tempPath() + "/trinity_export_" + versionName;
    if (QDir(tempDir).exists()) {
        QDir(tempDir).removeRecursively();
    }
    QDir().mkpath(tempDir);

    // Crear estructura: tmp/trinity_export_<nombre>/
    QString exportDir = tempDir + "/tmp/trinity_export_" + versionName;
    QDir().mkpath(exportDir);

    // Copiar version_content si se incluye (sin enlaces)
    if (exportWithApk) {
        QString versionContentDest = exportDir + "/version_content";
        if (!copyDirectory(versionPath, versionContentDest)) {
            QMessageBox::critical(parentWidget, tr("Error"),
                                  tr("Could not copy version."));
            QDir(tempDir).removeRecursively();
            return;
        }
    }

    // Copiar games/com.mojang (sin enlaces)
    QString baseDataDir =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
        "/mcpelauncher";
    QString gamesPath = baseDataDir + "/games/com.mojang";
    QString gamesDest = exportDir + "/games";

    if (!copyDirectory(gamesPath, gamesDest)) {
        QMessageBox::critical(parentWidget, tr("Error"),
                              tr("Could not copy resources."));
        QDir(tempDir).removeRecursively();
        return;
    }

    // Comprimir con tar en otro hilo
    QProcess *compressProcess = new QProcess(this);

    connect(
        compressProcess,
        QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
        [this, compressProcess, versionName, zipPath, tempDir, &progressDlg]() {
            // Cerrar diálogo de progreso
            progressDlg.accept();

            if (compressProcess->exitCode() == 0) {
                emit exportFinished(true,
                                    QString(tr("Version %1 exported as %2"))
                                        .arg(versionName, zipPath));
            } else {
                QString err = compressProcess->readAllStandardError();
                emit exportFinished(false, tr("Export failed:\n") + err);
            }

            // Limpiar temporal
            QDir(tempDir).removeRecursively();

            // Eliminar el proceso al terminar
            compressProcess->deleteLater();
        });

    compressProcess->start("tar", {"-czf", zipPath, "-C", exportDir, "."});

    // Mantener el diálogo abierto
    progressDlg.exec();
}

void Exporter::importVersion() {
    QString zipPath = QFileDialog::getOpenFileName(
        parentWidget, tr("Select TAR file"), "",
        tr("TAR files (*.tar.gz *.tar);;All files (*)"));
    if (zipPath.isEmpty())
        return;

    // Extraer nombre de la versión del nombre del archivo (sin extensión)
    QString fileName =
        QFileInfo(zipPath).baseName(); // Ej: "1.21.121" de "1.21.121.tar.gz"

    // Verificar si ya existen versiones o recursos
    QString baseDataDir =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
        "/mcpelauncher";
    QString destVersionDir = baseDataDir + "/versions/" + fileName;
    QString destGamesDir = baseDataDir + "/games/com.mojang";

    bool versionExists = QDir(destVersionDir).exists();
    bool gamesExists = QDir(destGamesDir).exists();

    if (versionExists || gamesExists) {
        int r = QMessageBox::warning(parentWidget, tr("Warning"),
                                     QString(tr("This process may take a while!\\n"
                                     "Do you want to continue?"))
                                         .arg(fileName),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);
        if (r == QMessageBox::No) {
            return;
        }
    }

    // Crear diálogo de progreso
    QDialog *progressDlg = new QDialog(parentWidget);
    progressDlg->setWindowTitle(tr("Importing..."));
    progressDlg->setFixedSize(300, 100);

    auto *layout = new QVBoxLayout(progressDlg);
    QLabel *label = new QLabel(tr("Importing version..."));
    QProgressBar *progressBar = new QProgressBar();
    progressBar->setRange(0, 0); // Indefinido
    layout->addWidget(label);
    layout->addWidget(progressBar);

    progressDlg->show();
    QApplication::processEvents(); // Actualizar UI

    // Directorio temporal para extracción
    QString tempDir = QDir::tempPath() + "/trinity_import_" + fileName;
    if (QDir(tempDir).exists()) {
        QDir(tempDir).removeRecursively();
    }
    QDir().mkpath(tempDir);

    // Descomprimir el TAR en otro hilo
    QProcess *process = new QProcess(this);

    connect(
        process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this,
        [this, process, fileName, tempDir, progressDlg, baseDataDir,
         destVersionDir, destGamesDir, versionExists, gamesExists]() {
            if (process->exitCode() != 0) {
                QString err = process->readAllStandardError();
                emit importFinished(false, tr("Extraction failed:\n") + err);
                QDir(tempDir).removeRecursively();
                // Cerrar diálogo de progreso aquí
                progressDlg->accept();
                progressDlg->deleteLater();
                process->deleteLater();
                return;
            }

            // Buscar recursivamente los directorios "version_content" y "games"
            QString versionContentPath;
            QString gamesPath;

            QDirIterator it(tempDir, QDir::Dirs | QDir::NoDotAndDotDot,
                            QDirIterator::Subdirectories);
            while (it.hasNext()) {
                QString dir = it.next();
                if (QFileInfo(dir).fileName() == "version_content") {
                    versionContentPath = dir;
                } else if (QFileInfo(dir).fileName() == "games") {
                    gamesPath = dir;
                }
            }

            // Ruta base de datos de la app
            QString baseDataDir = QStandardPaths::writableLocation(
                                      QStandardPaths::GenericDataLocation) +
                                  "/mcpelauncher";

            // Rutas de destino
            QString destVersionDir = baseDataDir + "/versions/" + fileName;
            QString destGamesDir = baseDataDir + "/games/com.mojang";

            // Mover contenido de version_content → versions/<nombre> (si
            // existe)
            bool success = true;
            if (!versionContentPath.isEmpty()) {
                if (versionExists) {
                    QDir(destVersionDir)
                        .removeRecursively(); // Eliminar versión anterior
                }
                // Crear directorio de destino
                QDir().mkpath(destVersionDir);

                // Mover *contenido* de version_content a destVersionDir (no la
                // carpeta en sí)
                QDir srcVersionDir(versionContentPath);
                QFileInfoList entries = srcVersionDir.entryInfoList(
                    QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
                for (const QFileInfo &info : entries) {
                    QString srcItem = info.absoluteFilePath();
                    QString dstItem =
                        destVersionDir + "/" +
                        info.fileName(); // Mover directamente a destVersionDir

                    if (!QDir().rename(srcItem, dstItem)) {
                        // Si rename falla (ej. entre dispositivos), usar
                        // copyDirectory y borrar origen
                        if (info.isDir()) {
                            if (!copyDirectory(srcItem, dstItem)) {
                                emit importFinished(
                                    false,
                                    tr("Could not move/copy directory: ") +
                                        info.fileName());
                                success = false;
                                break;
                            }
                        } else {
                            if (!QFile::copy(srcItem, dstItem)) {
                                emit importFinished(
                                    false,
                                    tr("Could not move/copy file: ") +
                                        info.fileName());
                                success = false;
                                break;
                            }
                        }
                        // Borrar origen (solo si se copió exitosamente)
                        if (info.isDir()) {
                            QDir(srcItem).removeRecursively();
                        } else {
                            QFile::remove(srcItem);
                        }
                    }
                }
            }

            // Mover contenido de games → games/com.mojang (si existe)
            if (!gamesPath.isEmpty()) {
                if (gamesExists) {
                    QDir(destGamesDir)
                        .removeRecursively(); // Eliminar recursos anteriores
                }
                // Crear directorio de destino
                QDir().mkpath(destGamesDir);

                // Mover *contenido* de gamesPath a destGamesDir (no la carpeta
                // en sí)
                QDir srcGamesDir(gamesPath);
                QFileInfoList entries = srcGamesDir.entryInfoList(
                    QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
                for (const QFileInfo &info : entries) {
                    QString srcItem = info.absoluteFilePath();
                    QString dstItem =
                        destGamesDir + "/" +
                        info.fileName(); // Mover directamente a destGamesDir

                    if (!QDir().rename(srcItem, dstItem)) {
                        // Si rename falla (ej. entre dispositivos), usar
                        // copyDirectory y borrar origen
                        if (info.isDir()) {
                            if (!copyDirectory(srcItem, dstItem)) {
                                emit importFinished(
                                    false, tr("No se pudo mover/copiar "
                                              "directorio de recursos: ") +
                                               info.fileName());
                                success = false;
                                break;
                            }
                        } else {
                            if (!QFile::copy(srcItem, dstItem)) {
                                emit importFinished(
                                    false, tr("No se pudo mover/copiar "
                                              "archivo de recursos: ") +
                                               info.fileName());
                                success = false;
                                break;
                            }
                        }
                        // Borrar origen (solo si se copió exitosamente)
                        if (info.isDir()) {
                            QDir(srcItem).removeRecursively();
                        } else {
                            QFile::remove(srcItem);
                        }
                    }
                }
            }

            // Limpiar temporal
            QDir(tempDir).removeRecursively();

            if (success) {
                emit importFinished(
                    true, QString(tr("Version %1 imported successfully."))
                              .arg(fileName));
            }

            // Cerrar diálogo de progreso aquí (al final de todo)
            progressDlg->accept();
            progressDlg->deleteLater();

            // Eliminar el proceso al terminar
            process->deleteLater();
        });

    process->start("tar", {"-xzf", zipPath, "-C", tempDir});
}
bool Exporter::copyDirectory(const QString &srcPath, const QString &dstPath) {
    QDir srcDir(srcPath);
    if (!srcDir.exists())
        return false;
    if (!QDir().mkpath(dstPath))
        return false;

    // Copiar archivos y subdirectorios
    QFileInfoList entries =
        srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo &info : entries) {
        QString srcItem = info.absoluteFilePath();
        QString dstItem = dstPath + "/" + info.fileName();

        if (info.isDir()) {
            if (!copyDirectory(srcItem, dstItem))
                return false;
        } else {
            if (!QFile::copy(srcItem, dstItem))
                return false;
        }

        // Permitir que la UI responda
        QApplication::processEvents();
    }
    return true;
}

// ... resto del código ...

void CopyWorker::doCopy() {
    QDir srcDir(srcPath);
    if (!srcDir.exists()) {
        emit copyFinished(false);
        return;
    }
    if (!QDir().mkpath(dstPath)) {
        emit copyFinished(false);
        return;
    }

    // Copiar archivos y subdirectorios
    QFileInfoList entries =
        srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo &info : entries) {
        QString srcItem = info.absoluteFilePath();
        QString dstItem = dstPath + "/" + info.fileName();

        if (info.isDir()) {
            if (!QDir().mkpath(dstItem)) {
                emit copyFinished(false);
                return;
            }
            // Copiar recursivamente
            QProcess process;
            process.start("cp", {"-r", srcItem, dstItem});
            process.waitForFinished(-1);
            if (process.exitCode() != 0) {
                emit copyFinished(false);
                return;
            }
        } else {
            if (!QFile::copy(srcItem, dstItem)) {
                emit copyFinished(false);
                return;
            }
        }
    }
    emit copyFinished(true);
}
