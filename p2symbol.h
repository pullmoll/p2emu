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
#include "p2atom.h"
#include "p2word.h"

/**
 * @brief The P2Symbol class is a wrapper for one symbolic name for a value
 */
class P2Symbol
{
public:
    explicit P2Symbol(const QString& name = QString(), const P2Atom& value = P2Atom());

    bool isNull() const;
    bool isEmpty() const;
    const QString& name() const;
    const P2Atom& atom() const;
    bool set_atom(const P2Atom& value);
    P2Atom::Type type() const;
    const QString type_name() const;
    P2Word definition() const;
    P2Word reference(int idx = 0) const;
    void add_reference(int lineno, const P2Word& word);
    const p2_lineno_word_hash_t& references() const;
    QList<P2Word> references(const P2Symbol& sym) const;

private:
    QString m_name;
    P2Atom m_atom;
    P2Word m_definition;
    p2_lineno_word_hash_t m_references;
};

Q_DECLARE_METATYPE(P2Symbol);
