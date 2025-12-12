#include "TrinityLib/ui/widgets/version_selector.hpp"
#include "TrinityLib/core/version_manager.hpp"

VersionSelector::VersionSelector(QWidget *parent) : QComboBox(parent) {
    refreshVersions();
}

void VersionSelector::refreshVersions() {
    clear();
    VersionManager vm;
    QStringList versions = vm.getInstalledVersions();

    if (!versions.isEmpty()) {
        addItems(versions);
        setEnabled(true);
    } else {
        addItem("No hay versiones instaladas");
        setEnabled(false);
    }
}

QString VersionSelector::getSelectedVersion() const {
    if (!isEnabled() || currentText() == "No hay versiones instaladas") {
        return QString();
    }
    return currentText();
}
