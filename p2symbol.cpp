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
#include <QUrl>
#include "p2symbol.h"

/**
 * @brief P2SymbolClass constructor
 * @param name optional initial name
 * @param value optional initial value
 */
P2SymbolClass::P2SymbolClass(const QString& name, const P2Union& value)
    : m_name(name)
    , m_value(value)
    , m_references()
{
}

/**
 * @brief P2SymbolClass constructor
 * @param name optional initial name
 * @param atom optional initial atom
 */
P2SymbolClass::P2SymbolClass(const QString& name, const P2Atom& atom)
    : m_name(name)
    , m_value(atom.value())
    , m_references()
{
}

/**
 * @brief Return true, if the symbol value is null (undefined)
 * @return true if empty, false otherwise
 */
bool P2SymbolClass::isNull() const
{
    return ut_Invalid == m_value.type() || m_value.isEmpty();
}

/**
 * @brief Return true, if the symbol is empty (undefined)
 * @return true if empty, false otherwise
 */
bool P2SymbolClass::isEmpty() const
{
    return m_name.isEmpty();
}

/**
 * @brief Return the symbol name
 * @return const reference to QString with the name
 */
const QString& P2SymbolClass::name() const
{
    return m_name;
}

/**
 * @brief Return the symbol's value
 * @return const reference to the value
 */
const P2Union& P2SymbolClass::value() const
{
    return m_value;
}

/**
 * @brief Return the %idx'th reference line number
 * @param idx index into references; 0 == line number where defined
 * @return line number, or -1 if not referenced / defined or %idx >= number of references
 */
P2Word P2SymbolClass::reference(int idx) const
{
    return m_references.value(idx);
}

/**
 * @brief Return the word where the symbol was defined
 * @return line number, or -1 if not defined
 */
P2Word P2SymbolClass::definition() const
{
    return m_definition;
}

/**
 * @brief Set a new value for the symbol
 * @param value new value
 */
void P2SymbolClass::set_value(const P2Union& value)
{
    m_value = value;
}

/**
 * @brief Return the type of the value in this symbol
 * @return QVariant::Type of the value
 */
p2_union_e P2SymbolClass::type() const
{
    return m_value.type();
}

/**
 * @brief Return the name for the type of the value in this symbol
 * @return QString with name
 */
const QString P2SymbolClass::type_name() const
{
    return m_value.type_name();
}

/**
 * @brief Add another line number to the hash of references
 * @param lineno line number
 * @param word word which references this symbol
 */
void P2SymbolClass::add_reference(int lineno, const P2Word& word)
{
    if (m_references.isEmpty())
        m_definition = word;
    m_references.insert(lineno, word);
}

/**
 * @brief Return the hash of references to the symbol
 *
 * NB: Index %idx == 0 is the line number where the symbol was defined.
 *
 * @return QList<int> with line numbers
 */
const p2_lineno_word_hash_t& P2SymbolClass::references() const
{
    return m_references;
}

/**
 * @brief Return the hash of references to the symbol
 *
 * NB: Index %idx == 0 is the line number where the symbol was defined.
 *
 * @return QList<int> with line numbers
 */
QList<P2Word> P2SymbolClass::references(const P2SymbolClass& sym) const
{
    return m_references.values(sym.definition().lineno());
}

/**
 * @brief Return the QUrl for this symbol's reference in word
 * @param word const reference to the P2Word referencing the symbol
 * @return QUrl with path, query, and fragment
 */
QUrl P2SymbolClass::url(const P2Word& word) const
{
    return url(*this, word);
}

QUrl P2SymbolClass::url(const P2SymbolClass& symbol, const P2Word& word)
{
    QUrl url;
    const int lineno = word.lineno();
    const int pos = word.pos();
    const int len = word.len();
    url.setPath(key_tv_asm);
    url.setQuery(symbol.m_name);
    url.setFragment(QString("%1,%2,%3")
                    .arg(lineno)
                    .arg(pos)
                    .arg(len));
    return url;
}
