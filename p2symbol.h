/****************************************************************************
 *
 * Propeller2 assembler symbol
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
#include <QString>
#include <QVariant>
#include <QSharedPointer>
#include "p2atom.h"
#include "p2word.h"

/**
 * @brief The P2Symbol class is a wrapper for one symbolic name for a value
 * The P2Symbol is implemented as a QSharedPointer<P2SymbolClass> to avoid
 * duplication of symbols in other containers and when passing as values.
 */
class P2SymbolClass
{
public:
    explicit P2SymbolClass(const QString& name, const P2Atom& atom);

    bool isNull() const;
    bool isEmpty() const;
    const QString& name() const;
    const P2Atom& atom() const;
    void set_atom(const P2Atom& value);
    P2Union value() const;
    void set_value(const P2Union& value);
    p2_Union_e type() const;
    const QString type_name() const;

    P2Word definition() const;
    P2Word reference(int lineno = 0) const;
    void add_reference(int lineno, const P2Word& word);
    const p2_word_hash_t& references() const;
    QList<int> references(const P2Word& word) const;
    QList<P2Word> references(const P2SymbolClass& sym) const;
    QUrl url(const P2Word& word) const;

    static QUrl url(const P2SymbolClass& symbol, const P2Word& word);

private:
    QString m_name;
    P2Atom m_atom;
    P2Word m_definition;
    p2_word_hash_t m_references;
};

typedef QSharedPointer<P2SymbolClass> P2Symbol;
Q_DECLARE_METATYPE(P2Symbol);
