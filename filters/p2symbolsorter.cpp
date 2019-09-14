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
        const P2Symbol l = qvariant_cast<P2Symbol>(l_data);
        const P2Symbol r = qvariant_cast<P2Symbol>(r_data);
        return l->definition().lineno() < r->definition().lineno();
    }
    if (l_data.userType() == qMetaTypeId<p2_Union_e>()) {
        const p2_Union_e l = qvariant_cast<p2_Union_e>(l_data);
        const p2_Union_e r = qvariant_cast<p2_Union_e>(r_data);
        return l < r;
    }
    if (l_data.userType() == qMetaTypeId<P2Atom>()) {
        const P2Atom l = qvariant_cast<P2Atom>(l_data);
        const P2Atom r = qvariant_cast<P2Atom>(r_data);
        if (l.type() == r.type()) {
            switch (l.type()) {
            case ut_Invalid:
                return false;
            case ut_Bool:
                return l.get_bool() < r.get_bool();
            case ut_Byte:
                return l.get_byte() < r.get_byte();
            case ut_Word:
                return l.get_word() < r.get_word();
            case ut_Addr:
                return l.get_addr(true) < r.get_addr(true);
            case ut_Long:
                return l.get_long() < r.get_long();
            case ut_Quad:
                return l.get_quad() < r.get_quad();
            case ut_Real:
                return l.get_real() < r.get_real();
            case ut_String:
                return l.get_bytes() < r.get_bytes();
            }
        }
        return l.type() < r.type();
    }
    QString l = l_data.toString();
    QString r = r_data.toString();
    return l < r;
}
