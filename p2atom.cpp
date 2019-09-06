/****************************************************************************
 *
 * Propeller2 assembler atomic data element implementation
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
#include <cmath>
#include "p2atom.h"
#include "p2util.h"

P2Atom::P2Atom(p2_union_e type)
    : m_trait(None)
    , m_value()
{
    m_value.set_type(type);
}

P2Atom::P2Atom(const P2Atom& other)
    : m_trait(None)
    , m_value(other.value())
{
}

P2Atom::P2Atom(bool value)
    : m_trait(None)
    , m_value(value)
{
}

P2Atom::P2Atom(p2_BYTE value)
    : m_trait(None)
    , m_value(value)
{
}

P2Atom::P2Atom(p2_WORD value)
    : m_trait(None)
    , m_value(value)
{
}

P2Atom::P2Atom(p2_LONG value)
    : m_trait(None)
    , m_value(value)
{
}

P2Atom::P2Atom(p2_QUAD value)
    : m_trait(None)
    , m_value(value)
{
}

P2Atom::P2Atom(p2_REAL value)
    : m_trait(None)
    , m_value(value)
{
}

/**
 * @brief Clear the atom
 */
void P2Atom::clear(p2_union_e type)
{
    m_trait = None;
    m_value.clear();
    m_value.set_type(type);
}

/**
 * @brief Return true, if the atom is invalid
 * @return true if empty, or false otherwise
 */
bool P2Atom::isNull() const
{
    return ut_Invalid == m_value.type();
}

/**
 * @brief Return true, if the atom contains no data
 * @return true if empty, or false otherwise
 */
bool P2Atom::isEmpty() const
{
    bool result = false;
    switch (m_value.type()) {
    case ut_Invalid:
        result = true;
        break;
    case ut_Bool:
    case ut_Byte:
    case ut_Word:
    case ut_Addr:
    case ut_Long:
    case ut_Quad:
    case ut_Real:
        break;
    case ut_String:
        result = m_value.isEmpty();
    }
    return result;
}

/**
 * @brief Return true, if the atom's value is zero
 * @return true if zero, or false otherwise
 */
bool P2Atom::isZero() const
{
    return m_value.is_zero();
}

/**
 * @brief Return true, if the atom is valid, i.e. contains data
 * @return true if valid, or false otherwise
 */
bool P2Atom::isValid() const
{
    return ut_Invalid != m_value.type();
}

/**
 * @brief Return the trait of the atom
 * @return Enumeration value from Trait
 */
P2Atom::Traits P2Atom::trait() const
{
    return m_trait;
}

/**
 * @brief Return the size of the data in the atom
 * @return size of m_data
 */
int P2Atom::size() const
{
    return m_value.size();
}

p2_LONG P2Atom::usize() const
{
    return static_cast<p2_LONG>(m_value.size());
}

int P2Atom::count() const
{
    return m_value.size() / m_value.unit();
}

/**
 * @brief Return the type of the atom, i.e. max size inserted
 * @return One of the %Type enumeration values
 */
p2_union_e P2Atom::type() const
{
    return m_value.type();
}

const QString P2Atom::type_name() const
{
    return m_value.type_name();
}

/**
 * @brief Set the type of the atom
 * @brie type one of the %Type enumeration values
 */
void P2Atom::set_type(p2_union_e type)
{
    m_value.set_type(type);
}

/**
 * @brief Set the atom's trait
 * @param trait new trait to set
 * @return true if set, false if not changed
 */
bool P2Atom::set_trait(P2Atom::Traits trait)
{
    if (trait == m_trait)
        return false;
    m_trait = trait;
    return true;
}

/**
 * @brief Add a trait to the atom's trait
 * @param trait trait to add
 * @return true if added, false if not changed
 */
bool P2Atom::add_trait(P2Atom::Traits trait)
{
    if (m_trait & trait)
        return false;
    m_trait = static_cast<Traits>(m_trait | trait);
    return true;
}

const P2Union& P2Atom::value() const
{
    return m_value;
}

/**
 * @brief Append another P2Atom to this atom
 * @param atom to append
 * @return true on success, or false on error
 */
bool P2Atom::add_atom(const P2Atom& atom)
{
    const p2_union_e type = atom.type();
    m_value.add(atom.value());
    m_value.set_type(type);
    return true;
}

/**
 * @brief Append a QVector<p2_BYTE> to this atom
 * @param bytes to append
 * @return true on success, or false on error
 */
bool P2Atom::add_bytes(const p2_BYTES& bytes)
{
    m_value.add_bytes(QVariant::fromValue(bytes));
    m_value.set_type(ut_String);
    return true;
}

/**
 * @brief Append a QVector<p2_WORD> to this atom
 * @param words to append
 * @return true on success, or false on error
 */
bool P2Atom::add_words(const p2_WORDS& words)
{
    m_value.add_words(QVariant::fromValue(words));
    m_value.set_type(ut_String);
    return true;
}

/**
 * @brief Append a QVector<p2_LONG> to this atom
 * @param longs to append
 * @return true on success, or false on error
 */
bool P2Atom::add_longs(const p2_LONGS& longs)
{
    m_value.add_longs(QVariant::fromValue(longs));
    m_value.set_type(ut_String);
    return true;
}

/**
 * @brief Append contents of a QByteArray to this atom
 * @param data QByteArray to append
 * @return true on success, or false on error
 */
bool P2Atom::add_array(const QByteArray& value)
{
    p2_BYTES bytes(value.size());
    memcpy(bytes.data(), value.constData(), static_cast<size_t>(value.size()));
    m_value.set_bytes(QVariant::fromValue(bytes));
    return true;
}

/**
 * @brief Format the atom's value as a string respecting the type
 * @return QString with value
 */
QString P2Atom::str(p2_format_e fmt) const
{
    QString result;
    switch (m_value.type()) {
    case ut_Invalid:
        result = QLatin1String("<invalid>");
        break;
    case ut_Bool:
        result = QString("<%1> %2")
                 .arg(type_name())
                 .arg(get<bool>() ? "true" : "false");
        break;
    case ut_Byte:
        switch (fmt) {
        case fmt_dec:
            result = QString("<%1> %2")
                     .arg(type_name())
                     .arg(get<p2_BYTE>());
            break;
        case fmt_bin:
            result = QString("<%1> %%2")
                     .arg(type_name())
                     .arg(get<p2_BYTE>(), 8, 2, QChar('0'));
            break;
        case fmt_byt:
        case fmt_hex:
        default:
            result = QString("<%1> $%2")
                     .arg(type_name())
                     .arg(get<p2_BYTE>(), 2, 16, QChar('0'));
        }
        break;
    case ut_Word:
        switch (fmt) {
        case fmt_dec:
            result = QString("<%1> %2")
                     .arg(type_name())
                     .arg(get<p2_WORD>());
            break;
        case fmt_bin:
            result = QString("<%1> %%2_%3")
                     .arg(type_name())
                     .arg(get<p2_WORD>() >> 8, 8, 2, QChar('0'))
                     .arg(get<p2_WORD>() & 0xff, 8, 2, QChar('0'));
            break;
        case fmt_byt:
            result = QString("<%1> $%2 $%3")
                     .arg(type_name())
                     .arg(get<p2_WORD>() >> 8, 2, 16, QChar('0'))
                     .arg(get<p2_WORD>() & 0xff, 2, 16, QChar('0'));
            break;
        case fmt_hex:
        default:
            result = QString("<%1> $%2")
                     .arg(type_name())
                     .arg(get<p2_WORD>(), 4, 16, QChar('0'));
        }
        break;
    case ut_Addr:
    case ut_Long:
        switch (fmt) {
        case fmt_dec:
            result = QString("<%1> %2")
                     .arg(type_name())
                     .arg(get<p2_LONG>());
            break;
        case fmt_bin:
            result = QString("<%1> %%2_%3_%4_%5")
                     .arg(type_name())
                     .arg((get<p2_LONG>() >> 24) & 0xff, 8, 2, QChar('0'))
                     .arg((get<p2_LONG>() >> 16) & 0xff, 8, 2, QChar('0'))
                     .arg((get<p2_LONG>() >>  8) & 0xff, 8, 2, QChar('0'))
                     .arg((get<p2_LONG>() >>  0) & 0xff, 8, 2, QChar('0'));
            break;
        case fmt_byt:
            result = QString("<%1> $%2 $%3 $%4 $%5")
                     .arg(type_name())
                     .arg((get<p2_LONG>() >> 24) & 0xff, 2, 16, QChar('0'))
                     .arg((get<p2_LONG>() >> 16) & 0xff, 2, 16, QChar('0'))
                     .arg((get<p2_LONG>() >>  8) & 0xff, 2, 16, QChar('0'))
                     .arg((get<p2_LONG>() >>  0) & 0xff, 2, 16, QChar('0'));
            break;
        case fmt_hex:
        default:
            result = QString("<%1> $%2")
                     .arg(type_name())
                     .arg(get<p2_LONG>(), 8, 16, QChar('0'));
        }
        break;
    case ut_Quad:
        switch (fmt) {
        case fmt_dec:
            result = QString("<%1> %2")
                     .arg(type_name())
                     .arg(get<p2_QUAD>());
            break;
        case fmt_bin:
            result = QString("<%1> %%2_%3_%4_%5_%7_%8_%9_%10")
                     .arg(type_name())
                     .arg((get<p2_QUAD>() >> 56) & 0xff, 8, 2, QChar('0'))
                     .arg((get<p2_QUAD>() >> 48) & 0xff, 8, 2, QChar('0'))
                     .arg((get<p2_QUAD>() >> 40) & 0xff, 8, 2, QChar('0'))
                     .arg((get<p2_QUAD>() >> 32) & 0xff, 8, 2, QChar('0'))
                     .arg((get<p2_QUAD>() >> 24) & 0xff, 8, 2, QChar('0'))
                     .arg((get<p2_QUAD>() >> 16) & 0xff, 8, 2, QChar('0'))
                     .arg((get<p2_QUAD>() >>  8) & 0xff, 8, 2, QChar('0'))
                     .arg((get<p2_QUAD>() >>  0) & 0xff, 8, 2, QChar('0'));
            break;
        case fmt_byt:
            result = QString("<%1> $%2 $%3 $%4 $%5 $%7 $%8 $%9 $%10")
                     .arg(type_name())
                     .arg((get<p2_QUAD>() >> 56) & 0xff, 2, 16, QChar('0'))
                     .arg((get<p2_QUAD>() >> 48) & 0xff, 2, 16, QChar('0'))
                     .arg((get<p2_QUAD>() >> 40) & 0xff, 2, 16, QChar('0'))
                     .arg((get<p2_QUAD>() >> 32) & 0xff, 2, 16, QChar('0'))
                     .arg((get<p2_QUAD>() >> 24) & 0xff, 2, 16, QChar('0'))
                     .arg((get<p2_QUAD>() >> 16) & 0xff, 2, 16, QChar('0'))
                     .arg((get<p2_QUAD>() >>  8) & 0xff, 2, 16, QChar('0'))
                     .arg((get<p2_QUAD>() >>  0) & 0xff, 2, 16, QChar('0'));
            break;
        case fmt_hex:
        default:
            result = QString("<%1> $%2")
                     .arg(type_name())
                     .arg(get<p2_QUAD>(), 16, 16, QChar('0'));
        }
        break;
    case ut_Real:
        result = QString("<Real> %1").arg(to_real(), 4, 'f');
        break;
    case ut_String:
        result = QString("<String> %1").arg(to_string());
        break;
    }
    return result;
}

/**
 * @brief Set atom to its one's complement (~) if flag is true
 * @param flag one's complement if true, leave unchanged otherwise
 */
void P2Atom::complement1(bool flag)
{
    if (!flag)
        return;

    switch (m_value.type()) {
    case ut_Invalid:
        break;
    case ut_Bool:
        m_value.set_bool(!m_value.get<bool>());
        break;
    case ut_Byte:
        m_value.set_byte(~m_value.get<p2_BYTE>());
        break;
    case ut_Word:
        m_value.set_word(~m_value.get<p2_WORD>());
        break;
    case ut_Addr:
        m_value.set_addr(~m_value.get<p2_LONG>());
        break;
    case ut_Long:
        m_value.set_long(~m_value.get<p2_LONG>());
        break;
    case ut_Quad:
        m_value.set_quad(~m_value.get<p2_QUAD>());
        break;
    case ut_Real:
    case ut_String:
        Q_ASSERT(m_value.type() != ut_Invalid);
        break;
    }
}

/**
 * @brief Set atom to its two's complement (-) if flag is true
 * @param flag two's complement if true, leave unchanged otherwise
 */
void P2Atom::complement2(bool flag)
{
    if (!flag)
        return;

    switch (m_value.type()) {
    case ut_Invalid:
        break;
    case ut_Bool:
        m_value.set_bool(!m_value.get<bool>());
        break;
    case ut_Byte:
        m_value.set_byte(-m_value.get<p2_BYTE>());
        break;
    case ut_Word:
        m_value.set_word(-m_value.get<p2_WORD>());
        break;
    case ut_Addr:
        m_value.set_addr(-m_value.get<p2_LONG>());
        break;
    case ut_Long:
        m_value.set_long(-m_value.get<p2_LONG>());
        break;
    case ut_Quad:
        m_value.set_quad(-m_value.get<p2_QUAD>());
        break;
    case ut_Real:
        m_value.set_real(-m_value.get<p2_REAL>());
        break;
    case ut_String:
        complement1(true);
        unary_inc(1);
        break;
    }
}

/**
 * @brief Set atom to its logical not (!) value
 * @param flag do the not if true, leave unchanged otherwise
 */
void P2Atom::logical_not(bool flag)
{
    if (!flag)
        return;

    switch (m_value.type()) {
    case ut_Invalid:
        break;
    case ut_Bool:
        m_value.set_bool(!m_value.get<bool>());
        break;
    case ut_Byte:
        m_value.set_byte(!m_value.get<p2_BYTE>());
        break;
    case ut_Word:
        m_value.set_word(!m_value.get<p2_WORD>());
        break;
    case ut_Addr:
        m_value.set_addr(!m_value.get<p2_LONG>());
        break;
    case ut_Long:
        m_value.set_long(!m_value.get<p2_LONG>());
        break;
    case ut_Quad:
        m_value.set_quad(!m_value.get<p2_QUAD>());
        break;
    case ut_Real:
        m_value.set_real(!qRound(m_value.get<p2_REAL>()));
        break;
    case ut_String:
        Q_ASSERT(m_value.type() != ut_Invalid);
        break;
    }
}

/**
 * @brief Set atom to its bool value
 * @param flag do the conversion if true, leave unchanged otherwise
 */
void P2Atom::make_bool(bool flag)
{
    if (!flag)
        return;
    m_value.set(!m_value.is_zero());
}

/**
 * @brief Set atom to its pre/post decrement value
 * @param flag do the decrement if true, leave unchanged otherwise
 */
void P2Atom::unary_dec(const p2_LONG val)
{
    if (0 == val)
        return;

    switch (m_value.type()) {
    case ut_Invalid:
        break;
    case ut_Bool:
        m_value.set_bool((m_value.get<bool>() - val) & 1 ? true : false);
        break;
    case ut_Byte:
        m_value.set_byte(m_value.get<p2_BYTE>() - val);
        break;
    case ut_Word:
        m_value.set_word(m_value.get<p2_WORD>() - val);
        break;
    case ut_Addr:
        m_value.set_addr(m_value.get<p2_LONG>() - val);
        break;
    case ut_Long:
        m_value.set_long(m_value.get<p2_LONG>() - val);
        break;
    case ut_Quad:
        m_value.set_quad(m_value.get<p2_QUAD>() - val);
        break;
    case ut_Real:
        m_value.set_real(m_value.get<p2_REAL>() - val);
        break;
    case ut_String:
        Q_ASSERT(m_value.type() != ut_Invalid);
        break;
    }
}

/**
 * @brief Set atom to its pre/post increment value
 * @param flag do the increment if true, leave unchanged otherwise
 */
void P2Atom::unary_inc(const p2_LONG val)
{
    if (0 == val)
        return;

    switch (m_value.type()) {
    case ut_Invalid:
        break;
    case ut_Bool:
        m_value.set_bool((m_value.get<bool>() + val) & 1 ? true : false);
        break;
    case ut_Byte:
        m_value.set_byte(m_value.get<p2_BYTE>() + val);
        break;
    case ut_Word:
        m_value.set_word(m_value.get<p2_WORD>() + val);
        break;
    case ut_Addr:
        m_value.set_addr(m_value.get<p2_LONG>() + val);
        break;
    case ut_Long:
        m_value.set_long(m_value.get<p2_LONG>() + val);
        break;
    case ut_Quad:
        m_value.set_quad(m_value.get<p2_QUAD>() + val);
        break;
    case ut_Real:
        m_value.set_real(m_value.get<p2_REAL>() + val);
        break;
    case ut_String:
        Q_ASSERT(m_value.type() != ut_Invalid);
        break;
    }
}

/**
 * @brief Set atom to its multiplication result value
 * @param val value to multiply by
 */
void P2Atom::arith_mul(const P2Atom& atom)
{
    if (atom.isNull())
        return;

    switch (m_value.type()) {
    case ut_Invalid:
        break;
    case ut_Bool:
        m_value.set_bool((m_value.get<bool>() * atom.get<p2_BYTE>()) & 1 ? true : false);
        break;
    case ut_Byte:
        m_value.set_byte(m_value.get<p2_BYTE>() * atom.get<p2_BYTE>());
        break;
    case ut_Word:
        m_value.set_word(m_value.get<p2_WORD>() * atom.get<p2_WORD>());
        break;
    case ut_Addr:
        m_value.set_addr(m_value.get<p2_LONG>() * atom.get<p2_LONG>());
        break;
    case ut_Long:
        m_value.set_long(m_value.get<p2_LONG>() * atom.get<p2_LONG>());
        break;
    case ut_Quad:
        m_value.set(m_value.get<p2_QUAD>() * atom.get<p2_QUAD>());
        break;
    case ut_Real:
        m_value.set(m_value.get<p2_REAL>() * atom.get<p2_REAL>());
        break;
    case ut_String:
        Q_ASSERT(m_value.type() != ut_Invalid);
        break;
    }
}

/**
 * @brief Set atom to its division result value
 * @param val value to multiply by
 */
void P2Atom::arith_div(const P2Atom& atom)
{
    switch (m_value.type()) {
    case ut_Invalid:
        break;
    case ut_Bool:
        {
            p2_BYTE divisor = atom.get<p2_BYTE>();
            if (0 != divisor)
                m_value.set_bool((m_value.get<bool>() / divisor) & 1 ? true : false);
        }
        break;
    case ut_Byte:
        {
            p2_BYTE divisor = atom.get<p2_BYTE>();
            if (0 != divisor)
                m_value.set_byte(m_value.get<p2_BYTE>() / divisor);
        }
        break;
    case ut_Word:
        {
            p2_WORD divisor = atom.get<p2_WORD>();
            if (0 != divisor)
                m_value.set_word(m_value.get<p2_WORD>() / divisor);
        }
        break;
    case ut_Addr:
        {
            p2_LONG divisor = atom.get<p2_LONG>();
            if (0 != divisor)
                m_value.set_addr(m_value.get<p2_LONG>() / divisor);
        }
        break;
    case ut_Long:
        {
            p2_LONG divisor = atom.get<p2_LONG>();
            if (0 != divisor)
                m_value.set_long(m_value.get<p2_LONG>() / divisor);
        }
        break;
    case ut_Quad:
        {
            p2_QUAD divisor = atom.get<p2_QUAD>();
            if (0 != divisor)
                m_value.set_quad(m_value.get<p2_QUAD>() / divisor);
        }
        break;
    case ut_Real:
        {
            p2_REAL divisor = atom.get<p2_REAL>();
            if (!qFuzzyIsNull(divisor))
                m_value.set_real(m_value.get<p2_REAL>() / divisor);
        }
        break;
    case ut_String:
        Q_ASSERT(m_value.type() != ut_Invalid);
        break;
    }
}

/**
 * @brief Set atom to its modulo result value
 * @param val value to do modulo by
 */
void P2Atom::arith_mod(const P2Atom& atom)
{
    switch (m_value.type()) {
    case ut_Invalid:
        break;
    case ut_Bool:
        {
            p2_BYTE divisor = atom.get<p2_BYTE>();
            if (0 != divisor)
                m_value.set_bool((m_value.get<bool>() % divisor) & 1 ? true : false);
        }
        break;
    case ut_Byte:
        {
            p2_BYTE divisor = atom.get<p2_BYTE>();
            if (0 != divisor)
                m_value.set_byte(m_value.get<p2_BYTE>() % divisor);
        }
        break;
    case ut_Word:
        {
            p2_WORD divisor = atom.get<p2_WORD>();
            if (0 != divisor)
                m_value.set_word(m_value.get<p2_WORD>() % divisor);
        }
        break;
    case ut_Addr:
        {
            p2_LONG divisor = atom.get<p2_LONG>();
            if (0 != divisor)
                m_value.set_addr(m_value.get<p2_LONG>() % divisor);
        }
        break;
    case ut_Long:
        {
            p2_LONG divisor = atom.get<p2_LONG>();
            if (0 != divisor)
                m_value.set_long(m_value.get<p2_LONG>() % divisor);
        }
        break;
    case ut_Quad:
        {
            p2_QUAD divisor = atom.get<p2_QUAD>();
            if (0 != divisor)
                m_value.set_quad(m_value.get<p2_QUAD>() % divisor);
        }
        break;
    case ut_Real:
        {
            p2_REAL divisor = atom.get<p2_REAL>();
            if (!qFuzzyIsNull(divisor))
                m_value.set_real(fmod(m_value.get<p2_REAL>(), divisor));
        }
        break;
    case ut_String:
        Q_ASSERT(m_value.type() != ut_Invalid);
        break;
    }
}

/**
 * @brief Set atom to its addition result value
 * @param val value to add
 */
void P2Atom::arith_add(const P2Atom& atom)
{
    switch (m_value.type()) {
    case ut_Invalid:
        break;
    case ut_Bool:
        m_value.set_bool((m_value.get<p2_BYTE>() + atom.get<p2_BYTE>()) & 1 ? true : false);
        break;
    case ut_Byte:
        m_value.set_byte(m_value.get<p2_BYTE>() + atom.get<p2_BYTE>());
        break;
    case ut_Word:
        m_value.set_word(m_value.get<p2_WORD>() + atom.get<p2_WORD>());
        break;
    case ut_Addr:
        m_value.set_addr(m_value.get<p2_LONG>() + atom.get<p2_LONG>());
        break;
    case ut_Long:
        m_value.set_long(m_value.get<p2_LONG>() + atom.get<p2_LONG>());
        break;
    case ut_Quad:
        m_value.set_quad(m_value.get<p2_QUAD>() + atom.get<p2_QUAD>());
        break;
    case ut_Real:
        m_value.set_real(m_value.get<p2_REAL>() + atom.get<p2_REAL>());
        break;
    case ut_String:
        Q_ASSERT(m_value.type() != ut_Invalid);
        break;
    }
}

/**
 * @brief Set atom to its subtraction result value
 * @param val value to subtract
 */
void P2Atom::arith_sub(const P2Atom& atom)
{
    switch (m_value.type()) {
    case ut_Invalid:
        break;
    case ut_Bool:
        m_value.set_bool((m_value.get<p2_BYTE>() - atom.get<p2_BYTE>()) & 1 ? true : false);
        break;
    case ut_Byte:
        m_value.set_byte(m_value.get<p2_BYTE>() - atom.get<p2_BYTE>());
        break;
    case ut_Word:
        m_value.set_word(m_value.get<p2_WORD>() - atom.get<p2_WORD>());
        break;
    case ut_Addr:
        m_value.set_addr(m_value.get<p2_LONG>() - atom.get<p2_LONG>());
        break;
    case ut_Long:
        m_value.set_long(m_value.get<p2_LONG>() - atom.get<p2_LONG>());
        break;
    case ut_Quad:
        m_value.set_quad(m_value.get<p2_QUAD>() - atom.get<p2_QUAD>());
        break;
    case ut_Real:
        m_value.set_real(m_value.get<p2_REAL>() - atom.get<p2_REAL>());
        break;
    case ut_String:
        Q_ASSERT(m_value.type() != ut_Invalid);
        break;
    }
}

/**
 * @brief Set atom to its left shifted value
 * @param bits number of bits to shift left
 */
void P2Atom::binary_shl(const P2Atom& atom)
{
    switch (m_value.type()) {
    case ut_Invalid:
        break;
    case ut_Bool:
        m_value.set_bool(false);
        break;
    case ut_Byte:
        m_value.set_byte(m_value.get<p2_BYTE>() << atom.get<p2_BYTE>());
        break;
    case ut_Word:
        m_value.set_word(m_value.get<p2_WORD>() << atom.get<p2_WORD>());
        break;
    case ut_Addr:
        m_value.set_addr(m_value.get<p2_LONG>() << atom.get<p2_LONG>());
        break;
    case ut_Long:
        m_value.set_long(m_value.get<p2_LONG>() << atom.get<p2_LONG>());
        break;
    case ut_Quad:
        m_value.set_quad(m_value.get<p2_QUAD>() << atom.get<p2_LONG>());
        break;
    case ut_Real:
        m_value.set_real(qRound(m_value.get<p2_REAL>()) << atom.get<p2_LONG>());
        break;
    case ut_String:
        Q_ASSERT(m_value.type() != ut_Invalid);
        break;
    }
}

/**
 * @brief Set atom to its right shifted value
 * @param bits number of bits to shift right
 */
void P2Atom::binary_shr(const P2Atom& atom)
{
    switch (m_value.type()) {
    case ut_Invalid:
        break;
    case ut_Bool:
        m_value.set_bool(false);
        break;
    case ut_Byte:
        m_value.set_byte(m_value.get<p2_BYTE>() >> atom.get<p2_BYTE>());
        break;
    case ut_Word:
        m_value.set_word(m_value.get<p2_WORD>() >> atom.get<p2_WORD>());
        break;
    case ut_Addr:
        m_value.set_addr(m_value.get<p2_LONG>() >> atom.get<p2_LONG>());
        break;
    case ut_Long:
        m_value.set_long(m_value.get<p2_LONG>() >> atom.get<p2_LONG>());
        break;
    case ut_Quad:
        m_value.set_quad(m_value.get<p2_QUAD>() >> atom.get<p2_LONG>());
        break;
    case ut_Real:
        m_value.set_real(qRound(m_value.get<p2_REAL>()) >> atom.get<p2_LONG>());
        break;
    case ut_String:
        Q_ASSERT(m_value.type() != ut_Invalid);
        break;
    }
}

/**
 * @brief Set atom to its binary AND value
 * @param mask value to AND with
 */
void P2Atom::binary_and(const P2Atom& atom)
{
    switch (m_value.type()) {
    case ut_Invalid:
        break;
    case ut_Bool:
        m_value.set_bool(m_value.get<bool>() & atom.get<bool>());
        break;
    case ut_Byte:
        m_value.set_byte(m_value.get<p2_BYTE>() & atom.get<p2_BYTE>());
        break;
    case ut_Word:
        m_value.set_word(m_value.get<p2_WORD>() & atom.get<p2_WORD>());
        break;
    case ut_Addr:
        m_value.set_addr(m_value.get<p2_LONG>() & atom.get<p2_LONG>());
        break;
    case ut_Long:
        m_value.set_long(m_value.get<p2_LONG>() & atom.get<p2_LONG>());
        break;
    case ut_Quad:
        m_value.set_quad(m_value.get<p2_QUAD>() & atom.get<p2_QUAD>());
        break;
    case ut_Real:
        m_value.set_real(m_value.get<p2_QUAD>() & atom.get<p2_QUAD>());
        break;
    case ut_String:
        Q_ASSERT(m_value.type() != ut_Invalid);
        break;
    }
}

/**
 * @brief Set atom to its binary XOR value
 * @param mask value to XOR with
 */
void P2Atom::binary_xor(const P2Atom& atom)
{
    switch (m_value.type()) {
    case ut_Invalid:
        break;
    case ut_Bool:
        m_value.set_bool(m_value.get<bool>() ^ atom.get<bool>());
        break;
    case ut_Byte:
        m_value.set_byte(m_value.get<p2_BYTE>() ^ atom.get<p2_BYTE>());
        break;
    case ut_Word:
        m_value.set_word(m_value.get<p2_WORD>() ^ atom.get<p2_WORD>());
        break;
    case ut_Addr:
        m_value.set_addr(m_value.get<p2_LONG>() ^ atom.get<p2_LONG>());
        break;
    case ut_Long:
        m_value.set_long(m_value.get<p2_LONG>() ^ atom.get<p2_LONG>());
        break;
    case ut_Quad:
        m_value.set_quad(m_value.get<p2_QUAD>() ^ atom.get<p2_QUAD>());
        break;
    case ut_Real:
        m_value.set_real(m_value.get<p2_QUAD>() ^ atom.get<p2_QUAD>());
        break;
    case ut_String:
        Q_ASSERT(m_value.type() != ut_Invalid);
        break;
    }
}

/**
 * @brief Set atom to its binary OR value
 * @param mask value to OR with
 */
void P2Atom::binary_or(const P2Atom& atom)
{
    switch (m_value.type()) {
    case ut_Invalid:
        break;
    case ut_Bool:
        m_value.set_bool(m_value.get<bool>() | atom.get<bool>());
        break;
    case ut_Byte:
        m_value.set_byte(m_value.get<p2_BYTE>() | atom.get<p2_BYTE>());
        break;
    case ut_Word:
        m_value.set_word(m_value.get<p2_WORD>() | atom.get<p2_WORD>());
        break;
    case ut_Addr:
        m_value.set_addr(m_value.get<p2_LONG>() | atom.get<p2_LONG>());
        break;
    case ut_Long:
        m_value.set_long(m_value.get<p2_LONG>() | atom.get<p2_LONG>());
        break;
    case ut_Quad:
        m_value.set_quad(m_value.get<p2_QUAD>() | atom.get<p2_QUAD>());
        break;
    case ut_Real:
        m_value.set_real(m_value.get<p2_QUAD>() | atom.get<p2_QUAD>());
        break;
    case ut_String:
        Q_ASSERT(m_value.type() != ut_Invalid);
        break;
    }
}

void P2Atom::binary_rev()
{
    switch (m_value.type()) {
    case ut_Invalid:
        break;
    case ut_Bool:
        m_value.set_bool(m_value.get<bool>());
        break;
    case ut_Byte:
        m_value.set_byte(P2Util::reverse(m_value.get<p2_BYTE>()));
        break;
    case ut_Word:
        m_value.set_word(P2Util::reverse(m_value.get<p2_WORD>()));
        break;
    case ut_Addr:
        m_value.set_addr(P2Util::reverse(m_value.get<p2_LONG>()));
        break;
    case ut_Long:
        m_value.set_long(P2Util::reverse(m_value.get<p2_LONG>()));
        break;
    case ut_Quad:
        m_value.set_quad(P2Util::reverse(m_value.get<p2_QUAD>()));
        break;
    case ut_Real:
        m_value.set_real(P2Util::reverse(m_value.get<p2_QUAD>()));
        break;
    case ut_String:
        Q_ASSERT(m_value.type() != ut_Invalid);
        break;
    }
}

void P2Atom::reverse(const P2Atom& atom)
{
    switch (m_value.type()) {
    case ut_Invalid:
        break;
    case ut_Bool:
        m_value.set_bool(m_value.get<bool>());
        break;
    case ut_Byte:
        m_value.set_byte(P2Util::reverse(m_value.get<p2_QUAD>(), atom.get<p2_LONG>()));
        break;
    case ut_Word:
        m_value.set_word(P2Util::reverse(m_value.get<p2_QUAD>(), atom.get<p2_LONG>()));
        break;
    case ut_Addr:
        m_value.set_addr(P2Util::reverse(m_value.get<p2_QUAD>(), atom.get<p2_LONG>()));
        break;
    case ut_Long:
        m_value.set_long(P2Util::reverse(m_value.get<p2_QUAD>(), atom.get<p2_LONG>()));
        break;
    case ut_Quad:
        m_value.set_quad(P2Util::reverse(m_value.get<p2_QUAD>(), atom.get<p2_LONG>()));
        break;
    case ut_Real:
        m_value.set_real(P2Util::reverse(m_value.get<p2_QUAD>(), atom.get<p2_LONG>()));
        break;
    case ut_String:
        Q_ASSERT(m_value.type() != ut_Invalid);
        break;
    }
}

void P2Atom::encode(const P2Atom& atom)
{
    switch (m_value.type()) {
    case ut_Invalid:
        break;
    case ut_Bool:
        m_value.set_bool(m_value.get<bool>());
        break;
    case ut_Byte:
        m_value.set_byte(P2Util::encode(atom.get<p2_BYTE>()));
        break;
    case ut_Word:
        m_value.set_word(P2Util::encode(atom.get<p2_WORD>()));
        break;
    case ut_Addr:
        m_value.set_addr(P2Util::encode(atom.get<p2_LONG>()));
        break;
    case ut_Long:
        m_value.set_long(P2Util::encode(atom.get<p2_LONG>()));
        break;
    case ut_Quad:
        m_value.set_quad(P2Util::encode(atom.get<p2_QUAD>()));
        break;
    case ut_Real:
        m_value.set_real(P2Util::encode(atom.get<p2_QUAD>()));
        break;
    case ut_String:
        Q_ASSERT(m_value.type() != ut_Invalid);
        break;
    }
}

void P2Atom::decode(const P2Atom& atom)
{
    switch (m_value.type()) {
    case ut_Invalid:
        break;
    case ut_Bool:
        m_value.set_bool(P2Util::lzc(atom.get<p2_BYTE>()));
        break;
    case ut_Byte:
        m_value.set_byte(P2Util::lzc(atom.get<p2_BYTE>()));
        break;
    case ut_Word:
        m_value.set_word(P2Util::lzc(atom.get<p2_WORD>()));
        break;
    case ut_Addr:
        m_value.set_addr(P2Util::lzc(atom.get<p2_LONG>()));
        break;
    case ut_Long:
        m_value.set_long(P2Util::lzc(atom.get<p2_LONG>()));
        break;
    case ut_Quad:
        m_value.set_quad(P2Util::lzc(atom.get<p2_QUAD>()));
        break;
    case ut_Real:
        m_value.set_real(P2Util::lzc(atom.get<p2_QUAD>()));
        break;
    case ut_String:
        Q_ASSERT(m_value.type() != ut_Invalid);
        break;
    }
}

/**
 * @brief Return data as a single bool
 * @return One bool
 */
bool P2Atom::to_bool() const
{
    return m_value.get<bool>();
}

/**
 * @brief Return data as a single byte
 * @return One p2_BYTE
 */
p2_BYTE P2Atom::to_byte() const
{
    return m_value.get<p2_BYTE>();
}

/**
 * @brief Return data as a single word
 * @return One p2_WORD
 */
p2_WORD P2Atom::to_word() const
{
    return m_value.get<p2_WORD>();
}

/**
 * @brief Return data as a single long
 * @param ok optional pointer to a bool set to true if data is available
 * @return One p2_LONG
 */
p2_LONG P2Atom::to_long() const
{
    return m_value.get<p2_LONG>();
}

/**
 * @brief Return data as a single quad
 * @param ok optional pointer to a bool set to true if data is available
 * @return One p2_LONG
 */
p2_QUAD P2Atom::to_quad() const
{
    return m_value.get<p2_QUAD>();
}

/**
 * @brief Return data as a single quad
 * @param ok optional pointer to a bool set to true if data is available
 * @return One p2_LONG
 */
p2_REAL P2Atom::to_real() const
{
    return m_value.get<p2_REAL>();
}

/**
 * @brief Return data as a QString
 * @param ok optional pointer to a bool set to true if data is available
 * @return QString with the data
 */
QString P2Atom::to_string() const
{
    QString result;
    p2_CHARS chars = m_value.get<p2_CHARS>();
    foreach(char ch, chars)
        result += ch;
    return result;
}

/**
 * @brief Return data as a QByteArray
 * @param ok optional pointer to a bool set to true if data is available
 * @return QString with the data
 */
QByteArray P2Atom::to_array() const
{
    QByteArray result;
    p2_CHARS chars = m_value.get<p2_CHARS>();
    foreach(char ch, chars)
        result += ch;
    return result;
}

/**
 * @brief Return data as a vector of bytes
 * @return p2_BYTEs of all data
 */
p2_BYTES P2Atom::to_bytes() const
{
    p2_BYTES result = m_value.get<p2_BYTES>();
    return result;
}

/**
 * @brief Return data as a vector of words
 * @return p2_WORDs of all data
 */
p2_WORDS P2Atom::to_words() const
{
    p2_WORDS result = m_value.get<p2_WORDS>();
    return result;
}

/**
 * @brief Return data as a vector of longs
 * @return p2_LONGs of all data
 */
p2_LONGS P2Atom::to_longs() const
{
    p2_LONGS result = m_value.get<p2_LONGS>();
    return result;
}

void P2Atom::make_real()
{
    m_value.set_type(ut_Real);
}

P2Atom& P2Atom::operator=(const P2Atom& other)
{
    m_trait = other.m_trait;
    m_value = other.m_value;
    return *this;
}

bool P2Atom::operator==(const P2Atom& other)
{
    switch (m_value.type()) {
    case ut_Invalid:
        return false;
    case ut_Bool:
        return m_value.get<bool>() == other.get<bool>();
    case ut_Byte:
        return m_value.get<p2_BYTE>() == other.get<p2_BYTE>();
    case ut_Word:
        return m_value.get<p2_WORD>() == other.get<p2_WORD>();
    case ut_Addr:
    case ut_Long:
        return m_value.get<p2_LONG>() == other.get<p2_LONG>();
    case ut_Quad:
        return m_value.get<p2_QUAD>() == other.get<p2_QUAD>();
    case ut_Real:
        return qFuzzyCompare(m_value.get<p2_REAL>(), other.get<p2_REAL>());
    case ut_String:
        return m_value.get<p2_CHARS>() == other.get<p2_CHARS>();
    }
    return false;
}

bool P2Atom::operator!=(const P2Atom& other)
{
    return !(*this == other);
}

bool P2Atom::operator<(const P2Atom& other)
{
    switch (m_value.type()) {
    case ut_Invalid:
        return false;
    case ut_Bool:
        return m_value.get<bool>() < other.get<bool>();
    case ut_Byte:
        return m_value.get<p2_BYTE>() < other.get<p2_BYTE>();
    case ut_Word:
        return m_value.get<p2_WORD>() < other.get<p2_WORD>();
    case ut_Addr:
    case ut_Long:
        return m_value.get<p2_LONG>() < other.get<p2_LONG>();
    case ut_Quad:
        return m_value.get<p2_QUAD>() < other.get<p2_QUAD>();
    case ut_Real:
        return m_value.get<p2_REAL>() < other.get<p2_REAL>();
    case ut_String:
        return m_value.get<p2_CHARS>() < other.get<p2_CHARS>();
    }
    return false;
}

bool P2Atom::operator<=(const P2Atom& other)
{
    switch (m_value.type()) {
    case ut_Invalid:
        return false;
    case ut_Bool:
        return m_value.get<bool>() <= other.get<bool>();
    case ut_Byte:
        return m_value.get<p2_BYTE>() <= other.get<p2_BYTE>();
    case ut_Word:
        return m_value.get<p2_WORD>() <= other.get<p2_WORD>();
    case ut_Addr:
    case ut_Long:
        return m_value.get<p2_LONG>() <= other.get<p2_LONG>();
    case ut_Quad:
        return m_value.get<p2_QUAD>() <= other.get<p2_QUAD>();
    case ut_Real:
        return m_value.get<p2_REAL>() <= other.get<p2_REAL>();
    case ut_String:
        return m_value.get<p2_CHARS>() <= other.get<p2_CHARS>();
    }
    return false;
}

bool P2Atom::operator>(const P2Atom& other)
{
    switch (m_value.type()) {
    case ut_Invalid:
        return false;
    case ut_Bool:
        return m_value.get<bool>() > other.get<bool>();
    case ut_Byte:
        return m_value.get<p2_BYTE>() > other.get<p2_BYTE>();
    case ut_Word:
        return m_value.get<p2_WORD>() > other.get<p2_WORD>();
    case ut_Addr:
    case ut_Long:
        return m_value.get<p2_LONG>() > other.get<p2_LONG>();
    case ut_Quad:
        return m_value.get<p2_QUAD>() > other.get<p2_QUAD>();
    case ut_Real:
        return m_value.get<p2_REAL>() > other.get<p2_REAL>();
    case ut_String:
        return m_value.get<p2_CHARS>() > other.get<p2_CHARS>();
    }
    return false;
}

bool P2Atom::operator>=(const P2Atom& other)
{
    switch (m_value.type()) {
    case ut_Invalid:
        return false;
    case ut_Bool:
        return m_value.get<bool>() >= other.get<bool>();
    case ut_Byte:
        return m_value.get<p2_BYTE>() >= other.get<p2_BYTE>();
    case ut_Word:
        return m_value.get<p2_WORD>() >= other.get<p2_WORD>();
    case ut_Addr:
    case ut_Long:
        return m_value.get<p2_LONG>() >= other.get<p2_LONG>();
    case ut_Quad:
        return m_value.get<p2_QUAD>() >= other.get<p2_QUAD>();
    case ut_Real:
        return m_value.get<p2_REAL>() >= other.get<p2_REAL>();
    case ut_String:
        return m_value.get<p2_CHARS>() >= other.get<p2_CHARS>();
    }
    return false;
}

P2Atom& P2Atom::operator~()
{
    complement1(true);
    return *this;
}

P2Atom& P2Atom::operator-() {
    complement2(true);
    return *this;
}

P2Atom& P2Atom::operator!() {
    logical_not(true);
    return *this;
}

P2Atom& P2Atom::operator++() {
    unary_inc(true);
    return *this;
}

P2Atom& P2Atom::operator--() {
    unary_dec(true);
    return *this;
}

P2Atom& P2Atom::operator+=(const P2Atom& other) {
    arith_add(other);
    return *this;
}

P2Atom& P2Atom::operator-=(const P2Atom& other) {
    arith_sub(other);
    return *this;
}

P2Atom& P2Atom::operator*=(const P2Atom& other) {
    arith_mul(other);
    return *this;
}

P2Atom& P2Atom::operator /= (const P2Atom& other) {
    arith_div(other);
    return *this;
}

P2Atom& P2Atom::operator %= (const P2Atom& other) {
    arith_div(other);
    return *this;
}

P2Atom& P2Atom::operator <<= (const P2Atom& other) {
    binary_shl(other);
    return *this;
}

P2Atom& P2Atom::operator >>= (const P2Atom& other) {
    binary_shr(other);
    return *this;
}

P2Atom& P2Atom::operator &= (const P2Atom& other) {
    binary_and(other);
    return *this;
}

P2Atom& P2Atom::operator ^= (const P2Atom& other) {
    binary_xor(other);
    return *this;
}

P2Atom& P2Atom::operator |= (const P2Atom& other) {
    binary_or(other);
    return *this;
}


/**
 * @brief Format a LONG %data as hex digits according to %mask
 * @param data p2_LONG with data
 * @param mask p2_LONG with mask
 * @return formatted string
 */
QString P2Atom::format_long_mask(const p2_LONG data, const p2_LONG mask)
{
    QString result;

    // for each nibble
    for (int shift = 32-4; shift >=0; shift -= 4) {
        if (0x0f == ((mask >> shift) & 0x0f)) {
            // the mask is set: append the nibble
            result += QString("%1").arg((data >> shift) & 0x0f, 1, 16);
        } else {
            // the mask is not set: append a dash
            result += QChar('-');
        }
    }
    return result;
}

/**
 * @brief Format a P2Atom's data as string list of longs in hex
 * @param atom const reference to the P2Atom with data
 * @param addr starting address
 * @return QStringList with one or more formatted long values
 */
QStringList P2Atom::format_data(const P2Atom& atom, const p2_LONG addr)
{
    QStringList result;
    QString line;
    const p2_BYTES bytes = atom.to_bytes();
    p2_LONG offset = addr;
    p2_LONG data = 0;
    p2_LONG mask = 0;

    for (int i = 0; i < bytes.count(); i++) {
        const int shift = 8 * (offset & 3);
        data |= static_cast<p2_LONG>(bytes[i]) << shift;
        mask |= 0xffu << shift;
        if (0 == (++offset & 3)) {
            result += format_long_mask(data, mask);
            mask = 0;
        }
    }

    if (mask)
        result += format_long_mask(data, mask);
    return result;
}

QByteArray P2Atom::to_array(const P2Atom& atom)
{
    return atom.to_array();
}

QString P2Atom::to_string(const P2Atom& atom)
{
    return atom.to_string();
}
