#ifndef EXPORTER_H
#define EXPORTER_H

#include <QObject>
#include <QThread>
#include <QWidget>

class Exporter : public QObject {
        Q_OBJECT

    public:
        explicit Exporter(QWidget *parent = nullptr);

        void exportVersion(const QString &versionName);
        void importVersion();

    signals:
        void exportFinished(bool success, const QString &message);
        void importFinished(bool success, const QString &message);

    private:
        QWidget *parentWidget;
        bool copyDirectory(const QString &srcPath, const QString &dstPath);
};

// Clase auxiliar para copiar directorios en otro hilo
class CopyWorker : public QObject {
        Q_OBJECT

    public:
        QString srcPath;
        QString dstPath;

    signals:
        void copyFinished(bool success);

    public slots:
        void doCopy();
};

#endif // EXPORTER_H
