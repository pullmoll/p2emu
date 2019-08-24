#pragma once
#include <QObject>
#include <QStyledItemDelegate>

class P2AsmModel;

class P2AsmSourceDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    P2AsmSourceDelegate(QObject* parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
