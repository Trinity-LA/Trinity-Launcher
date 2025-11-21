#ifndef VERSION_SELECTOR_H
#define VERSION_SELECTOR_H

#include <QComboBox>

class VersionSelector : public QComboBox {
    Q_OBJECT

public:
    explicit VersionSelector(QWidget *parent = nullptr);
    void refreshVersions();
    QString getSelectedVersion() const;
};

#endif // VERSION_SELECTOR_H
