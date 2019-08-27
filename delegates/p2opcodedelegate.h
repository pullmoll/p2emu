#pragma once
#include <QStyledItemDelegate>

class P2OpcodeDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    P2OpcodeDelegate(QObject* parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
