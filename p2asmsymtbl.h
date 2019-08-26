/****************************************************************************
 *
 * Propeller2 assembler symbol table
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
#pragma once
#include "p2asmsym.h"

/**
 * @brief The SymbolTable class is a QHash<QString,Symbol>, i.e. a hash
 * of symbol names containing their definitions.
 */
class P2AsmSymTbl
{
public:
    explicit P2AsmSymTbl();

    void clear();
    bool contains(const QString& name);
    bool insert(const P2AsmSymbol& symbol);
    bool insert(const QString& name, const P2Atom& value);
    bool setValue(const QString& name, const P2Atom& value);
    P2AsmSymbol value(const QString& name) const;
    P2Atom::Type type(const QString& name) const;
    int defined_where(const QString& name) const;
    QStringList references_in(int lineno) const;
    int reference(const QString& name, int idx = 0) const;
    bool addReference(const QString& name, int lineno);
    const QList<int> references(const QString& name) const;

    const QHash<QString,P2AsmSymbol>& symbols() const;
    const QMultiHash<int,QString>& references() const;

    /**
     * @brief Return a symbol value cast to the type T
     * The symbol values are stored as QVariant and this
     * template maps to the qvariant_cast<T> for the value.
     */
    template <typename T>
    T value(const QString& name) const
    {
        const P2AsmSymbol sym = m_symbols.value(name);
        return sym.value<T>();
    }

private:
    QHash<QString,P2AsmSymbol> m_symbols;
    QMultiHash<int,QString> m_references;
};
