#include "TrinityLib/ui/widgets/instance_delegate.hpp"

#include <QPainter>

InstanceDelegate::InstanceDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {}

void InstanceDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const {
    // Base rendering: background (hover/selection), icon, label.
    QStyledItemDelegate::paint(painter, option, index);

    // Validity badge: red dot with "!" when the instance is not valid.
    const bool valid = index.data(Qt::UserRole).toBool();
    if (valid)
        return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    const int badgeSize = 16;
    const int margin = 8;
    const QRect badgeRect(option.rect.right() - badgeSize - margin,
                          option.rect.top() + margin,
                          badgeSize, badgeSize);

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor("#B38D97")); // Shinonome mError
    painter->drawEllipse(badgeRect);

    painter->setPen(QColor("#1A1D20")); // Shinonome mOnError
    painter->drawText(badgeRect, Qt::AlignCenter, QStringLiteral("!"));

    painter->restore();
}

QSize InstanceDelegate::sizeHint(const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(104, 104);
}
