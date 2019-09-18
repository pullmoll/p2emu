/****************************************************************************
 *
 * P2 emulator symbol table sorting filter
 *
 * Copyright (C) 2019 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ****************************************************************************/
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
    if (l_data.canConvert<P2Word>()) {
        const P2Word& l = qvariant_cast<P2Word>(l_data);
        const P2Word& r = qvariant_cast<P2Word>(r_data);
        return l.lineno() < r.lineno();
    }
    if (l_data.canConvert<P2Symbol>()) {
        const P2Symbol l = qvariant_cast<P2Symbol>(l_data);
        const P2Symbol r = qvariant_cast<P2Symbol>(r_data);
        if (l_index.column() == P2SymbolsModel::c_Definition)
            return l->definition().lineno() < r->definition().lineno();
        return l->name() < r->name();
    }
    if (l_data.canConvert<p2_Union_e>()) {
        const p2_Union_e l = qvariant_cast<p2_Union_e>(l_data);
        const p2_Union_e r = qvariant_cast<p2_Union_e>(r_data);
        return l < r;
    }
    if (l_data.canConvert<P2Atom>()) {
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
