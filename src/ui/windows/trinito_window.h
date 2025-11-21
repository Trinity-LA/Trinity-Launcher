#ifndef TRINITO_WINDOW_H
#define TRINITO_WINDOW_H

#include <QWidget>

class TrinitoWindow : public QWidget {
    Q_OBJECT

public:
    explicit TrinitoWindow(QWidget *parent = nullptr);

private:
    QWidget* createPackTab(const QString &targetSubdir, const QString &labelText);
    QWidget* createDevTab();
    QWidget* createWorldTab();
    void installItem(const QString &sourcePath, const QString &targetSubdir);
};

#endif // TRINITO_WINDOW_H
