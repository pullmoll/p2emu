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
#include "p2asmsymtbl.h"

P2AsmSymTbl::P2AsmSymTbl()
    : m_symbols()
{
}

/**
 * @brief Clear the symbol table
 */
void P2AsmSymTbl::clear()
{
    m_symbols.clear();
    m_references.clear();
}

/**
 * @brief Check if the symbol table contains a name
 * @param name symbol name to check for
 * @return true if known, or false if unknown
 */
bool P2AsmSymTbl::contains(const QString& name)
{
    return m_symbols.contains(name);
}

/**
 * @brief Insert a symbol into the symbol table
 * @param symbol const reference to the symbol to insert
 * @return false if the symbol was already in the table, or true if inserted
 */
bool P2AsmSymTbl::insert(const P2AsmSymbol& symbol)
{
    if (m_symbols.contains(symbol.name()))
        return false;
    m_symbols.insert(symbol.name(), symbol);
    return true;
}

/**
 * @brief Insert a symbol name / value into the symbol table
 * @param name name of the new symbol
 * @param value initial value of the new symbol
 * @return false if the symbol was already in the table, or true if inserted
 */
bool P2AsmSymTbl::insert(const QString& name, const QVariant& value)
{
    if (m_symbols.contains(name))
        return false;
    P2AsmSymbol sym(name, value);
    m_symbols.insert(name, sym);
    return true;
}

/**
 * @brief Set a symbol to a new value
 * @param name symbol name
 * @param value new symbol value
 * @return
 */
bool P2AsmSymTbl::setValue(const QString& name, const QVariant& value)
{
    if (!m_symbols.contains(name))
        return false;
    m_symbols[name].setValue(value);
    return true;
}

/**
 * @brief Return a copy of the P2AsmSymbol with %name
 * @param name name of the symbold
 * @return P2AsmSymbol which may be empty, if the symbol name is not in the table
 */
P2AsmSymbol P2AsmSymTbl::value(const QString& name) const
{
    return m_symbols.value(name);
}

/**
 * @brief Return the QVariant::Type for the value of the symbol with %name
 * @param name of the symbol to check for
 * @return QVariant::Type of the value
 */
QVariant::Type P2AsmSymTbl::type(const QString& name) const
{
    if (!m_symbols.contains(name))
        return QVariant::Invalid;
    return m_symbols[name].type();
}

int P2AsmSymTbl::defined_in(const QString& name) const
{
    if (!m_symbols.contains(name))
        return -1;
    return m_symbols[name].defined_in();
}

QStringList P2AsmSymTbl::defined_in(int lineno) const
{
    return m_references.values(lineno);
}

int P2AsmSymTbl::reference(const QString& name, int idx) const
{
    if (!m_symbols.contains(name))
        return -1;
    return m_symbols[name].reference(idx);
}

bool P2AsmSymTbl::addReference(const QString& name, int lineno)
{
    m_references.insert(lineno, name);
    if (m_symbols.contains(name)) {
        m_symbols[name].addReference(lineno);
        return true;
    }
    // Insert an undefined symbol
    m_symbols.insert(name, P2AsmSymbol(name));
    m_symbols[name].addReference(lineno);
    return false;
}

const QList<int> P2AsmSymTbl::references(const QString& name) const
{
    return m_references.keys(name);
}
