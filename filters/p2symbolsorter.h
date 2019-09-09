#pragma once
#include <QSortFilterProxyModel>

class P2SymbolSorter : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    P2SymbolSorter(QObject *parent = nullptr);

protected:
    bool lessThan(const QModelIndex &l_index, const QModelIndex &r_index) const override;
};
