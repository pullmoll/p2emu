#include "p2symbolsorter.h"
#include "p2symboltable.h"
#include "p2symbolsmodel.h"

P2SymbolSorter::P2SymbolSorter(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

bool P2SymbolSorter::lessThan(const QModelIndex& l_index, const QModelIndex& r_index) const
{
    QVariant l_data = sourceModel()->data(l_index, Qt::EditRole);
    QVariant r_data = sourceModel()->data(r_index, Qt::EditRole);
    if (l_data.userType() == qMetaTypeId<P2Word>()) {
        const P2Word& l = qvariant_cast<P2Word>(l_data);
        const P2Word& r = qvariant_cast<P2Word>(r_data);
        return l.lineno() < r.lineno();
    }
    if (l_data.userType() == qMetaTypeId<P2Symbol>()) {
        P2Symbol l = qvariant_cast<P2Symbol>(l_data);
        P2Symbol r = qvariant_cast<P2Symbol>(r_data);
        return l->definition().lineno() < r->definition().lineno();
    }
    QString l = l_data.toString();
    QString r = r_data.toString();
    return l < r;
}
