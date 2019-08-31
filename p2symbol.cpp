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
#include "p2symbol.h"

/**
 * @brief P2Symbol constructo
 * @param name optional initial name
 * @param value optional initial value
 */
P2Symbol::P2Symbol(const QString& name, const P2Atom& value)
    : m_name(name)
    , m_atom(value)
    , m_references()
{}

/**
 * @brief Return true, if the symbol value is null (undefined)
 * @return true if empty, false otherwise
 */
bool P2Symbol::isNull() const
{
    return m_atom.isEmpty();
}

/**
 * @brief Return true, if the symbol is empty (undefined)
 * @return true if empty, false otherwise
 */
bool P2Symbol::isEmpty() const
{
    return m_name.isEmpty();
}

/**
 * @brief Return the symbol name
 * @return const reference to QString with the name
 */
const QString& P2Symbol::name() const
{
    return m_name;
}

/**
 * @brief Return the symbol's atom
 * @return const reference to the value
 */
const P2Atom& P2Symbol::atom() const
{
    return m_atom;
}

/**
 * @brief Return the %idx'th reference line number
 * @param idx index into references; 0 == line number where defined
 * @return line number, or -1 if not referenced / defined or %idx >= number of references
 */
P2Word P2Symbol::reference(int idx) const
{
    return m_references.value(idx);
}

/**
 * @brief Return the word where the symbol was defined
 * @return line number, or -1 if not defined
 */
P2Word P2Symbol::definition() const
{
    return m_definition;
}

/**
 * @brief Set a new value for the symbol
 * @param value new value
 * @return true if modified, false if unmodified
 */
bool P2Symbol::set_atom(const P2Atom& value)
{
    if (m_atom == value)
        return false;
    m_atom = value;
    return true;
}

/**
 * @brief Return the type of the value in this symbol
 * @return QVariant::Type of the value
 */
P2Atom::Type P2Symbol::type() const
{
    return m_atom.type();
}

/**
 * @brief Return the name for the type of the value in this symbol
 * @return QString with name
 */
const QString P2Symbol::type_name() const
{
    return m_atom.type_name();
}

/**
 * @brief Add another line number to the hash of references
 * @param lineno line number
 * @param word word which references this symbol
 */
void P2Symbol::add_reference(int lineno, const P2Word& word)
{
    if (m_references.isEmpty())
        m_definition = word;
    else
        m_references.insert(lineno, word);
}

/**
 * @brief Return the hash of references to the symbol
 *
 * NB: Index %idx == 0 is the line number where the symbol was defined.
 *
 * @return QList<int> with line numbers
 */
const p2_lineno_word_hash_t& P2Symbol::references() const
{
    return m_references;
}

/**
 * @brief Return the hash of references to the symbol
 *
 * NB: Index %idx == 0 is the line number where the symbold was defined.
 *
 * @return QList<int> with line numbers
 */
QList<P2Word> P2Symbol::references(const P2Symbol& sym) const
{
    return m_references.values(sym.definition().lineno());
}
