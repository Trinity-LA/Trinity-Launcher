#ifndef INSTANCE_DELEGATE_H
#define INSTANCE_DELEGATE_H

#include <QStyledItemDelegate>

/**
 * Delegate for the instance grid (QListView::IconMode).
 *
 * Paints the default icon+text cell and overlays a small badge in the
 * top-right corner when the instance data is marked as invalid through
 * Qt::UserRole (bool). Valid instances paint normally.
 */
class InstanceDelegate : public QStyledItemDelegate {
        Q_OBJECT

    public:
        explicit InstanceDelegate(QObject *parent = nullptr);

        void paint(QPainter *painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
        QSize sizeHint(const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override;
};

#endif // INSTANCE_DELEGATE_H
