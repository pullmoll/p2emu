#pragma once
#include <QObject>
#include <QStyledItemDelegate>

class P2AsmModel;

class P2SourceDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    P2SourceDelegate(QObject* parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
