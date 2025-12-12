#ifndef TRINITOWINDOW_H
#define TRINITOWINDOW_H

#include <QWidget>
#include <QListWidget> // Para mostrar packs y mundos

class TrinitoWindow : public QWidget {  // ✅ Aquí debe estar la clase
    Q_OBJECT

public:
    explicit TrinitoWindow(QWidget *parent = nullptr);

private:
    // ... declaraciones de funciones ...
    QWidget *createPackTab(const QString &targetSubdir, const QString &labelText);
    QWidget *createDevTab();
    QWidget *createWorldTab();
    void installItem(const QString &sourcePath, const QString &targetSubdir);

    // Nuevas funciones para listar y gestionar packs
    QWidget *createManageTab(const QString &packType, const QString &displayName); // Pestaña para gestionar
    void loadPacks(const QString &packType, QListWidget *listWidget); // Cargar packs de un tipo
    void togglePack(const QString &packType, const QString &packName, bool enable); // Activar/desactivar pack

    // Variables para las listas
    QListWidget *modsList = nullptr;
    QListWidget *addonsList = nullptr;
    QListWidget *resourcesList = nullptr;
    QListWidget *mapsList = nullptr;
};

#endif // TRINITOWINDOW_H
