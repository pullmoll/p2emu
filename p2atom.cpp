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
    : m_trait(tr_none)
    , m_value()
    , m_index()
{
    m_value.set_type(type);
}

P2Atom::P2Atom(const P2Atom& other)
    : m_trait(other.m_trait)
    , m_value(other.m_value)
    , m_index(other.m_index)
{
}

P2Atom::P2Atom(bool value)
    : m_trait(tr_none)
    , m_value(value)
    , m_index()
{
}

P2Atom::P2Atom(p2_BYTE value)
    : m_trait(tr_none)
    , m_value(value)
    , m_index()
{
}

P2Atom::P2Atom(p2_WORD value)
    : m_trait(tr_none)
    , m_value(value)
    , m_index()
{
}

P2Atom::P2Atom(p2_LONG value)
    : m_trait(tr_none)
    , m_value(value)
    , m_index()
{
}

P2Atom::P2Atom(p2_QUAD value)
    : m_trait(tr_none)
    , m_value(value)
    , m_index()
{
}

P2Atom::P2Atom(p2_REAL value)
    : m_trait(tr_none)
    , m_value(value)
    , m_index()
{
}

/**
 * @brief Clear the atom
 */
void P2Atom::clear(p2_union_e type)
{
    m_trait = tr_none;
    m_value.clear();
    m_value.set_type(type);
    m_index.clear();
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
    return m_value.isEmpty();
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
 * @brief Return the traits of the atom
 * @return Enumeration value from p2_traits_e
 */
p2_traits_e P2Atom::traits() const
{
    return m_trait;
}

/**
 * @brief Return true, if the traits of the atom contain %trait
 * @param trait Enumeration value from p2_traits_e
 * @return true if set, or false otherwise
 */
bool P2Atom::has_trait(const p2_traits_e trait) const
{
    return p2_has_trait(m_trait, trait);
}

/**
 * @brief Return the size of the data in the atom in bytes
 * @return size of m_value in bytes
 */
int P2Atom::size() const
{
    return m_value.usize();
}

/**
 * @brief Return the size of the data in the atom in units
 * @return size of m_value in units
 */
int P2Atom::usize() const
{
    return m_value.usize() / m_value.unit();
}

/**
 * @brief Return the number of elements in the atom's value
 * @return number of elements in m_value
 */
int P2Atom::count() const
{
    return m_value.size();
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
 * @brief Set the index of the atom
 * @param val new index value
 */
void P2Atom::set_index(const QVariant& val)
{
    if (val.canConvert(mt_P2Atom)) {
        P2Atom index = qvariant_cast<P2Atom>(val);
        m_index = index.m_value;
    } else {
        m_index.set_long(val.toUInt());
    }
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
bool P2Atom::set_traits(p2_traits_e trait)
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
bool P2Atom::add_trait(p2_traits_e trait)
{
    if (p2_has_trait(m_trait, trait))
        return false;
    p2_set_trait(m_trait, trait);
    return true;
}

/**
 * @brief Clear a trait from the atom's trait
 * @param trait trait to clear
 * @return true if added, false if not changed
 */
bool P2Atom::clr_trait(p2_traits_e trait)
{
    if (!p2_has_trait(m_trait, trait))
        return false;
    p2_clr_trait(m_trait, trait);
    return true;
}

/**
 * @brief Return the atom's value
 * @return const reference to the value
 */
const P2Union& P2Atom::value() const
{
    return m_value;
}

/**
 * @brief Set the atom's value
 * @param value const reference to a new value
 */
void P2Atom::set_value(const P2Union& value)
{
    m_value = value;
}

/**
 * @brief Return the atom's index
 * @return const reference to the index
 */
const P2Union& P2Atom::index() const
{
    return m_index;
}

/**
 * @brief Return the atom's index as p2_LONG value
 * @return value of the index (0 if not set)
 */
p2_LONG P2Atom::index_long() const
{
    return m_index.get_long();
}

/**
 * @brief Set the atom value to %_int
 * NB: The atom's value type becomes ut_Long
 * @param _int to set
 */
void P2Atom::set_int(const int& _int)
{
    m_value.set_int(_int);
}

/**
 * @brief Set the atom value to %_bool
 * NB: The atom's value type becomes ut_Bool
 * @param _bool to set
 */
void P2Atom::set_bool(const bool& _bool)
{
    m_value.set_bool(_bool);
}

/**
 * @brief Set the atom value to %_char
 * NB: The atom's value type becomes ut_Byte
 * @param _char to set
 */
void P2Atom::set_char(const char& _char)
{
    m_value.set_char(_char);
}

/**
 * @brief Set the atom value to %_byte
 * NB: The atom's value type becomes ut_Byte
 * @param _byte to set
 */
void P2Atom::set_byte(const p2_BYTE& _byte)
{
    m_value.set_byte(_byte);
}

/**
 * @brief Set the atom value to %_word
 * NB: The atom's value type becomes ut_Word
 * @param _word to set
 */
void P2Atom::set_word(const p2_WORD& _word)
{
    m_value.set_word(_word);
}

/**
 * @brief Set the atom value to %_add
 * NB: The atom's value type becomes ut_Addr
 * @param _addr to set
 */
void P2Atom::set_addr(const p2_LONG& _addr)
{
    m_value.set_addr(_addr);
}

/**
 * @brief Set the atom value to %_long
 * NB: The atom's value type becomes ut_Long
 * @param _long to set
 */
void P2Atom::set_long(const p2_LONG& _long)
{
    m_value.set_long(_long);
}

/**
 * @brief Set the atom value to %_real
 * NB: The atom's value type becomes ut_Real
 * @param _real to set
 */
void P2Atom::set_real(const p2_REAL& _real)
{
    m_value.set_real(_real);
}

/**
 * @brief Set the atom value to %_bytes
 * NB: The atom's value type becomes ut_String
 * @param _bytes to set
 */
void P2Atom::set_chars(const p2_CHARS& _chars)
{
    m_value.set_chars(_chars);
}

/**
 * @brief Set the atom value to %_bytes
 * NB: The atom's value type becomes ut_String
 * @param _bytes to set
 */
void P2Atom::set_bytes(const p2_BYTES& _bytes)
{
    m_value.set_bytes(_bytes);
}

/**
 * @brief Set the atom value to %_words
 * NB: The atom's value type becomes ut_Word
 * @param _bytes to set
 */
void P2Atom::set_words(const p2_WORDS& _words)
{
    m_value.set_words(_words);
}

/**
 * @brief Set the atom value to %_longs
 * NB: The atom's value type becomes ut_Long
 * @param _longs to set
 */
void P2Atom::set_longs(const p2_LONGS& _longs)
{
    m_value.set_longs(_longs);
}

/**
 * @brief Set the atom value to %_array
 * NB: The atom's value type becomes ut_String
 * @param _array to set
 */
void P2Atom::set_array(const QByteArray& _array)
{
    m_value.set_array(_array);
}

/**
 * @brief Append a single int to this atom
 * @param _int to append
 */
void P2Atom::add_int(const int& _int)
{
    m_value.add_int(_int);
}

/**
 * @brief Append a single bool to this atom
 * @param _bool to append
 */
void P2Atom::add_bool(const bool& _bool)
{
    m_value.add_bool(_bool);
}

/**
 * @brief Append a single char to this atom
 * @param _char to append
 */
void P2Atom::add_char(const char& _char)
{
    m_value.add_char(_char);
}

/**
 * @brief Append a single p2_BYTE to this atom
 * @param _byte to append
 */
void P2Atom::add_byte(const p2_BYTE& _byte)
{
    m_value.add_byte(_byte);
}

/**
 * @brief Append a single p2_WORD to this atom
 * @param _word to append
 */
void P2Atom::add_word(const p2_WORD& _word)
{
    m_value.add_word(_word);
}

/**
 * @brief Append a single p2_WORD to this atom
 * @param _long to append
 */
void P2Atom::add_long(const p2_LONG& _long)
{
    m_value.add_long(_long);
}

/**
 * @brief Append a QVector<char> to this atom
 * @param _chars to append
 */
void P2Atom::add_chars(const p2_CHARS& _chars)
{
    m_value.add_chars(_chars);
}

/**
 * @brief Append a QVector<p2_BYTE> to this atom
 * @param _bytes to append
 */
void P2Atom::add_bytes(const p2_BYTES& _bytes)
{
    m_value.add_bytes(_bytes);
}

/**
 * @brief Append a QVector<p2_WORD> to this atom
 * @param _words to append
 */
void P2Atom::add_words(const p2_WORDS& _words)
{
    m_value.add_words(_words);
}

/**
 * @brief Append a QVector<p2_LONG> to this atom
 * @param _longs to append
 */
void P2Atom::add_longs(const p2_LONGS& _longs)
{
    m_value.add_longs(_longs);
}

/**
 * @brief Append contents of a QByteArray to this atom
 * @param data QByteArray to append
 */
void P2Atom::add_array(const QByteArray& value)
{
    p2_BYTES bytes(value.size());
    memcpy(bytes.data(), value.constData(), static_cast<size_t>(value.size()));
    m_value.add_bytes(bytes);
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
                 .arg(m_value.get_bool() ? "true" : "false");
        break;
    case ut_Byte:
        switch (fmt) {
        case fmt_dec:
            result = QString("<%1> %2")
                     .arg(type_name())
                     .arg(m_value.get_byte());
            break;
        case fmt_bin:
            result = QString("<%1> %%2")
                     .arg(type_name())
                     .arg(m_value.get_byte(), 8, 2, QChar('0'));
            break;
        case fmt_byt:
        case fmt_hex:
        default:
            result = QString("<%1> $%2")
                     .arg(type_name())
                     .arg(m_value.get_byte(), 2, 16, QChar('0'));
        }
        break;
    case ut_Word:
        switch (fmt) {
        case fmt_dec:
            result = QString("<%1> %2")
                     .arg(type_name())
                     .arg(m_value.get_word());
            break;
        case fmt_bin:
            result = QString("<%1> %%2_%3")
                     .arg(type_name())
                     .arg(m_value.get_word() >> 8, 8, 2, QChar('0'))
                     .arg(m_value.get_word() & 0xff, 8, 2, QChar('0'));
            break;
        case fmt_byt:
            result = QString("<%1> $%2 $%3")
                     .arg(type_name())
                     .arg(m_value.get_word() >> 8, 2, 16, QChar('0'))
                     .arg(m_value.get_word() & 0xff, 2, 16, QChar('0'));
            break;
        case fmt_hex:
        default:
            result = QString("<%1> $%2")
                     .arg(type_name())
                     .arg(m_value.get_word(), 4, 16, QChar('0'));
        }
        break;
    case ut_Addr:
    case ut_Long:
        switch (fmt) {
        case fmt_dec:
            result = QString("<%1> %2")
                     .arg(type_name())
                     .arg(m_value.get_long());
            break;
        case fmt_bin:
            result = QString("<%1> %%2_%3_%4_%5")
                     .arg(type_name())
                     .arg((m_value.get_long() >> 24) & 0xff, 8, 2, QChar('0'))
                     .arg((m_value.get_long() >> 16) & 0xff, 8, 2, QChar('0'))
                     .arg((m_value.get_long() >>  8) & 0xff, 8, 2, QChar('0'))
                     .arg((m_value.get_long() >>  0) & 0xff, 8, 2, QChar('0'));
            break;
        case fmt_byt:
            result = QString("<%1> $%2 $%3 $%4 $%5")
                     .arg(type_name())
                     .arg((m_value.get_long() >> 24) & 0xff, 2, 16, QChar('0'))
                     .arg((m_value.get_long() >> 16) & 0xff, 2, 16, QChar('0'))
                     .arg((m_value.get_long() >>  8) & 0xff, 2, 16, QChar('0'))
                     .arg((m_value.get_long() >>  0) & 0xff, 2, 16, QChar('0'));
            break;
        case fmt_hex:
        default:
            result = QString("<%1> $%2")
                     .arg(type_name())
                     .arg(m_value.get_long(), 8, 16, QChar('0'));
        }
        break;
    case ut_Quad:
        switch (fmt) {
        case fmt_dec:
            result = QString("<%1> %2")
                     .arg(type_name())
                     .arg(m_value.get_quad());
            break;
        case fmt_bin:
            result = QString("<%1> %%2_%3_%4_%5_%7_%8_%9_%10")
                     .arg(type_name())
                     .arg((m_value.get_quad() >> 56) & 0xff, 8, 2, QChar('0'))
                     .arg((m_value.get_quad() >> 48) & 0xff, 8, 2, QChar('0'))
                     .arg((m_value.get_quad() >> 40) & 0xff, 8, 2, QChar('0'))
                     .arg((m_value.get_quad() >> 32) & 0xff, 8, 2, QChar('0'))
                     .arg((m_value.get_quad() >> 24) & 0xff, 8, 2, QChar('0'))
                     .arg((m_value.get_quad() >> 16) & 0xff, 8, 2, QChar('0'))
                     .arg((m_value.get_quad() >>  8) & 0xff, 8, 2, QChar('0'))
                     .arg((m_value.get_quad() >>  0) & 0xff, 8, 2, QChar('0'));
            break;
        case fmt_byt:
            result = QString("<%1> $%2 $%3 $%4 $%5 $%7 $%8 $%9 $%10")
                     .arg(type_name())
                     .arg((m_value.get_quad() >> 56) & 0xff, 2, 16, QChar('0'))
                     .arg((m_value.get_quad() >> 48) & 0xff, 2, 16, QChar('0'))
                     .arg((m_value.get_quad() >> 40) & 0xff, 2, 16, QChar('0'))
                     .arg((m_value.get_quad() >> 32) & 0xff, 2, 16, QChar('0'))
                     .arg((m_value.get_quad() >> 24) & 0xff, 2, 16, QChar('0'))
                     .arg((m_value.get_quad() >> 16) & 0xff, 2, 16, QChar('0'))
                     .arg((m_value.get_quad() >>  8) & 0xff, 2, 16, QChar('0'))
                     .arg((m_value.get_quad() >>  0) & 0xff, 2, 16, QChar('0'));
            break;
        case fmt_hex:
        default:
            result = QString("<%1> $%2")
                     .arg(type_name())
                     .arg(m_value.get_quad(), 16, 16, QChar('0'));
        }
        break;
    case ut_Real:
        result = QString("<Real> %1").arg(get_real(), 4, 'f');
        break;
    case ut_String:
        result = QString("<String> %1").arg(string());
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
        m_value.set_bool(!m_value.get_bool());
        break;
    case ut_Byte:
        m_value.set_byte(~m_value.get_byte());
        break;
    case ut_Word:
        m_value.set_word(~m_value.get_word());
        break;
    case ut_Addr:
        m_value.set_addr(~m_value.get_long());
        break;
    case ut_Long:
        m_value.set_long(~m_value.get_long());
        break;
    case ut_Quad:
        m_value.set_quad(~m_value.get_quad());
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
        m_value.set_bool(!m_value.get_bool());
        break;
    case ut_Byte:
        m_value.set_byte(-m_value.get_byte());
        break;
    case ut_Word:
        m_value.set_word(-m_value.get_word());
        break;
    case ut_Addr:
        m_value.set_addr(-m_value.get_long());
        break;
    case ut_Long:
        m_value.set_long(-m_value.get_long());
        break;
    case ut_Quad:
        m_value.set_quad(-m_value.get_quad());
        break;
    case ut_Real:
        m_value.set_real(-m_value.get_real());
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
        m_value.set_bool(!m_value.get_bool());
        break;
    case ut_Byte:
        m_value.set_byte(!m_value.get_byte());
        break;
    case ut_Word:
        m_value.set_word(!m_value.get_word());
        break;
    case ut_Addr:
        m_value.set_addr(!m_value.get_long());
        break;
    case ut_Long:
        m_value.set_long(!m_value.get_long());
        break;
    case ut_Quad:
        m_value.set_quad(!m_value.get_quad());
        break;
    case ut_Real:
        m_value.set_real(!qRound(m_value.get_real()));
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
    p2_BYTES bytes = m_value.get_bytes(true);
    m_value.set_bool(bytes.count(0) != bytes.size());
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
        m_value.set_bool((m_value.get_bool() - val) & true);
        break;
    case ut_Byte:
        m_value.set_byte(static_cast<p2_BYTE>(m_value.get_byte() - val));
        break;
    case ut_Word:
        m_value.set_word(static_cast<p2_WORD>(m_value.get_word() - val));
        break;
    case ut_Addr:
        m_value.set_addr(m_value.get_long() - val);
        break;
    case ut_Long:
        m_value.set_long(m_value.get_long() - val);
        break;
    case ut_Quad:
        m_value.set_quad(m_value.get_quad() - val);
        break;
    case ut_Real:
        m_value.set_real(m_value.get_real() - val);
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
        m_value.set_bool((m_value.get_bool() + val) & true);
        break;
    case ut_Byte:
        m_value.set_byte(static_cast<p2_BYTE>(m_value.get_byte() + val));
        break;
    case ut_Word:
        m_value.set_word(static_cast<p2_WORD>(m_value.get_word() + val));
        break;
    case ut_Addr:
        m_value.set_addr(m_value.get_long() + val);
        break;
    case ut_Long:
        m_value.set_long(m_value.get_long() + val);
        break;
    case ut_Quad:
        m_value.set_quad(m_value.get_quad() + val);
        break;
    case ut_Real:
        m_value.set_real(m_value.get_real() + val);
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
        m_value.set_bool((m_value.get_bool() * atom.get_byte()) & true);
        break;
    case ut_Byte:
        m_value.set_byte(m_value.get_byte() * atom.get_byte());
        break;
    case ut_Word:
        m_value.set_word(m_value.get_word() * atom.get_word());
        break;
    case ut_Addr:
        m_value.set_addr(m_value.get_long() * atom.get_long());
        break;
    case ut_Long:
        m_value.set_long(m_value.get_long() * atom.get_long());
        break;
    case ut_Quad:
        m_value.set_quad(m_value.get_quad() * atom.get_quad());
        break;
    case ut_Real:
        m_value.set_real(m_value.get_real() * atom.get_real());
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
            p2_BYTE divisor = atom.get_byte();
            if (0 != divisor)
                m_value.set_bool((m_value.get_bool() / divisor) & true);
        }
        break;
    case ut_Byte:
        {
            p2_BYTE divisor = atom.get_byte();
            if (0 != divisor)
                m_value.set_byte(m_value.get_byte() / divisor);
        }
        break;
    case ut_Word:
        {
            p2_WORD divisor = atom.get_word();
            if (0 != divisor)
                m_value.set_word(m_value.get_word() / divisor);
        }
        break;
    case ut_Addr:
        {
            p2_LONG divisor = atom.get_long();
            if (0 != divisor)
                m_value.set_addr(m_value.get_long() / divisor);
        }
        break;
    case ut_Long:
        {
            p2_LONG divisor = atom.get_long();
            if (0 != divisor)
                m_value.set_long(m_value.get_long() / divisor);
        }
        break;
    case ut_Quad:
        {
            p2_QUAD divisor = atom.get_quad();
            if (0 != divisor)
                m_value.set_quad(m_value.get_quad() / divisor);
        }
        break;
    case ut_Real:
        {
            p2_REAL divisor = atom.get_real();
            if (!qFuzzyIsNull(divisor))
                m_value.set_real(m_value.get_real() / divisor);
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
            p2_BYTE divisor = atom.get_byte();
            if (0 != divisor)
                m_value.set_bool((m_value.get_bool() % divisor) & true);
        }
        break;
    case ut_Byte:
        {
            p2_BYTE divisor = atom.get_byte();
            if (0 != divisor)
                m_value.set_byte(m_value.get_byte() % divisor);
        }
        break;
    case ut_Word:
        {
            p2_WORD divisor = atom.get_word();
            if (0 != divisor)
                m_value.set_word(m_value.get_word() % divisor);
        }
        break;
    case ut_Addr:
        {
            p2_LONG divisor = atom.get_long();
            if (0 != divisor)
                m_value.set_addr(m_value.get_long() % divisor);
        }
        break;
    case ut_Long:
        {
            p2_LONG divisor = atom.get_long();
            if (0 != divisor)
                m_value.set_long(m_value.get_long() % divisor);
        }
        break;
    case ut_Quad:
        {
            p2_QUAD divisor = atom.get_quad();
            if (0 != divisor)
                m_value.set_quad(m_value.get_quad() % divisor);
        }
        break;
    case ut_Real:
        {
            p2_REAL divisor = atom.get_real();
            if (!qFuzzyIsNull(divisor))
                m_value.set_real(fmod(m_value.get_real(), divisor));
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
        m_value.set_bool((m_value.get_bool() + atom.get_bool()) & true);
        break;
    case ut_Byte:
        m_value.set_byte(m_value.get_byte() + atom.get_byte());
        break;
    case ut_Word:
        m_value.set_word(m_value.get_word() + atom.get_word());
        break;
    case ut_Addr:
        m_value.set_addr(m_value.get_long() + atom.get_long());
        break;
    case ut_Long:
        m_value.set_long(m_value.get_long() + atom.get_long());
        break;
    case ut_Quad:
        m_value.set_quad(m_value.get_quad() + atom.get_quad());
        break;
    case ut_Real:
        m_value.set_real(m_value.get_real() + atom.get_real());
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
        m_value.set_bool((m_value.get_bool() - atom.get_bool()) & true);
        break;
    case ut_Byte:
        m_value.set_byte(m_value.get_byte() - atom.get_byte());
        break;
    case ut_Word:
        m_value.set_word(m_value.get_word() - atom.get_word());
        break;
    case ut_Addr:
        m_value.set_addr(m_value.get_long() - atom.get_long());
        break;
    case ut_Long:
        m_value.set_long(m_value.get_long() - atom.get_long());
        break;
    case ut_Quad:
        m_value.set_quad(m_value.get_quad() - atom.get_quad());
        break;
    case ut_Real:
        m_value.set_real(m_value.get_real() - atom.get_real());
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
        m_value.set_byte(static_cast<p2_BYTE>(m_value.get_byte() << atom.get_byte()));
        break;
    case ut_Word:
        m_value.set_word(static_cast<p2_WORD>(m_value.get_word() << atom.get_word()));
        break;
    case ut_Addr:
        m_value.set_addr(m_value.get_long() << atom.get_long());
        break;
    case ut_Long:
        m_value.set_long(m_value.get_long() << atom.get_long());
        break;
    case ut_Quad:
        m_value.set_quad(m_value.get_quad() << atom.get_long());
        break;
    case ut_Real:
        m_value.set_real(qRound(m_value.get_real()) << atom.get_long());
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
        m_value.set_byte(m_value.get_byte() >> atom.get_byte());
        break;
    case ut_Word:
        m_value.set_word(m_value.get_word() >> atom.get_word());
        break;
    case ut_Addr:
        m_value.set_addr(m_value.get_long() >> atom.get_long());
        break;
    case ut_Long:
        m_value.set_long(m_value.get_long() >> atom.get_long());
        break;
    case ut_Quad:
        m_value.set_quad(m_value.get_quad() >> atom.get_long());
        break;
    case ut_Real:
        m_value.set_real(qRound(m_value.get_real()) >> atom.get_long());
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
        m_value.set_bool(m_value.get_bool() & atom.get_bool());
        break;
    case ut_Byte:
        m_value.set_byte(m_value.get_byte() & atom.get_byte());
        break;
    case ut_Word:
        m_value.set_word(m_value.get_word() & atom.get_word());
        break;
    case ut_Addr:
        m_value.set_addr(m_value.get_long() & atom.get_long());
        break;
    case ut_Long:
        m_value.set_long(m_value.get_long() & atom.get_long());
        break;
    case ut_Quad:
        m_value.set_quad(m_value.get_quad() & atom.get_quad());
        break;
    case ut_Real:
        m_value.set_real(m_value.get_quad() & atom.get_quad());
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
        m_value.set_bool(m_value.get_bool() ^ atom.get_bool());
        break;
    case ut_Byte:
        m_value.set_byte(m_value.get_byte() ^ atom.get_byte());
        break;
    case ut_Word:
        m_value.set_word(m_value.get_word() ^ atom.get_word());
        break;
    case ut_Addr:
        m_value.set_addr(m_value.get_long() ^ atom.get_long());
        break;
    case ut_Long:
        m_value.set_long(m_value.get_long() ^ atom.get_long());
        break;
    case ut_Quad:
        m_value.set_quad(m_value.get_quad() ^ atom.get_quad());
        break;
    case ut_Real:
        m_value.set_real(m_value.get_quad() ^ atom.get_quad());
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
        m_value.set_bool(m_value.get_bool() | atom.get_bool());
        break;
    case ut_Byte:
        m_value.set_byte(m_value.get_byte() | atom.get_byte());
        break;
    case ut_Word:
        m_value.set_word(m_value.get_word() | atom.get_word());
        break;
    case ut_Addr:
        m_value.set_addr(m_value.get_long() | atom.get_long());
        break;
    case ut_Long:
        m_value.set_long(m_value.get_long() | atom.get_long());
        break;
    case ut_Quad:
        m_value.set_quad(m_value.get_quad() | atom.get_quad());
        break;
    case ut_Real:
        m_value.set_real(m_value.get_quad() | atom.get_quad());
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
        m_value.set_bool(m_value.get_bool());
        break;
    case ut_Byte:
        m_value.set_byte(P2Util::reverse(m_value.get_byte()));
        break;
    case ut_Word:
        m_value.set_word(P2Util::reverse(m_value.get_word()));
        break;
    case ut_Addr:
        m_value.set_addr(P2Util::reverse(m_value.get_long()));
        break;
    case ut_Long:
        m_value.set_long(P2Util::reverse(m_value.get_long()));
        break;
    case ut_Quad:
        m_value.set_quad(P2Util::reverse(m_value.get_quad()));
        break;
    case ut_Real:
        m_value.set_real(P2Util::reverse(m_value.get_quad()));
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
        m_value.set_bool(m_value.get_bool());
        break;
    case ut_Byte:
        m_value.set_byte(static_cast<p2_BYTE>(P2Util::reverse(m_value.get_quad(), atom.get_long())));
        break;
    case ut_Word:
        m_value.set_word(static_cast<p2_WORD>(P2Util::reverse(m_value.get_quad(), atom.get_long())));
        break;
    case ut_Addr:
        m_value.set_addr(static_cast<p2_LONG>(P2Util::reverse(m_value.get_quad(), atom.get_long())));
        break;
    case ut_Long:
        m_value.set_long(static_cast<p2_LONG>(P2Util::reverse(m_value.get_quad(), atom.get_long())));
        break;
    case ut_Quad:
        m_value.set_quad(P2Util::reverse(m_value.get_quad(), atom.get_long()));
        break;
    case ut_Real:
        m_value.set_real(P2Util::reverse(m_value.get_quad(), atom.get_long()));
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
        m_value.set_bool(m_value.get_bool());
        break;
    case ut_Byte:
        m_value.set_byte(static_cast<p2_BYTE>(P2Util::encode(atom.get_byte())));
        break;
    case ut_Word:
        m_value.set_word(static_cast<p2_WORD>(P2Util::encode(atom.get_word())));
        break;
    case ut_Addr:
        m_value.set_addr(P2Util::encode(atom.get_long()));
        break;
    case ut_Long:
        m_value.set_long(P2Util::encode(atom.get_long()));
        break;
    case ut_Quad:
        m_value.set_quad(P2Util::encode(atom.get_quad()));
        break;
    case ut_Real:
        m_value.set_real(P2Util::encode(atom.get_quad()));
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
        m_value.set_bool(P2Util::lzc(atom.get_byte()));
        break;
    case ut_Byte:
        m_value.set_byte(static_cast<p2_BYTE>(P2Util::lzc(atom.get_byte())));
        break;
    case ut_Word:
        m_value.set_word(static_cast<p2_WORD>(P2Util::lzc(atom.get_word())));
        break;
    case ut_Addr:
        m_value.set_addr(P2Util::lzc(atom.get_long()));
        break;
    case ut_Long:
        m_value.set_long(P2Util::lzc(atom.get_long()));
        break;
    case ut_Quad:
        m_value.set_quad(P2Util::lzc(atom.get_quad()));
        break;
    case ut_Real:
        m_value.set_real(P2Util::lzc(atom.get_quad()));
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
bool P2Atom::get_bool() const
{
    return m_value.get_bool();
}

/**
 * @brief Return data as a single int
 * @return One int
 */
int P2Atom::get_int() const
{
    return m_value.get_int();
}

/**
 * @brief Return data as a single byte
 * @return One p2_BYTE
 */
p2_BYTE P2Atom::get_byte() const
{
    return m_value.get_byte();
}

/**
 * @brief Return data as a single word
 * @return One p2_WORD
 */
p2_WORD P2Atom::get_word() const
{
    return m_value.get_word();
}

/**
 * @brief Return data as a single long
 * @return One p2_LONG
 */
p2_LONG P2Atom::get_long() const
{
    return m_value.get_long();
}

/**
 * @brief Return data as a single quad
 * @return One p2_QUAD
 */
p2_QUAD P2Atom::get_quad() const
{
    return m_value.get_quad();
}

/**
 * @brief Return data as a single quad
 * @return One p2_REAL
 */
p2_REAL P2Atom::get_real() const
{
    return m_value.get_real();
}

/**
 * @brief Return data as a QString
 * @return QString with the data
 */
QString P2Atom::string(bool expand) const
{
    p2_CHARS chars = m_value.get_chars(expand);
    return QString::fromLatin1(chars.data(), chars.size());
}

/**
 * @brief Return data as a QByteArray
 * @return QString with the data
 */
QByteArray P2Atom::array(bool expand) const
{
    p2_CHARS bytes = m_value.get_chars(expand);
    return QByteArray(bytes.data(), bytes.size());
}

/**
 * @brief Return data as a vector of bytes
 * @return p2_BYTEs of all data
 */
p2_BYTES P2Atom::get_bytes(bool expand) const
{
    return m_value.get_bytes(expand);
}

/**
 * @brief Return data as a vector of words
 * @return p2_WORDs of all data
 */
p2_WORDS P2Atom::get_words(bool expand) const
{
    return m_value.get_words(expand);
}

/**
 * @brief Return data as a vector of longs
 * @return p2_LONGs of all data
 */
p2_LONGS P2Atom::get_longs(bool expand) const
{
    return m_value.get_longs(expand);
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
        return m_value.get_bool() == other.m_value.get_bool();
    case ut_Byte:
        return m_value.get_byte() == other.m_value.get_byte();
    case ut_Word:
        return m_value.get_word() == other.m_value.get_word();
    case ut_Addr:
    case ut_Long:
        return m_value.get_long() == other.m_value.get_long();
    case ut_Quad:
        return m_value.get_quad() == other.m_value.get_quad();
    case ut_Real:
        return qFuzzyCompare(m_value.get_real(), other.m_value.get_real());
    case ut_String:
        return m_value.get_chars() == other.m_value.get_chars();
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
        return m_value.get_bool() < other.m_value.get_bool();
    case ut_Byte:
        return m_value.get_byte() < other.m_value.get_byte();
    case ut_Word:
        return m_value.get_word() < other.m_value.get_word();
    case ut_Addr:
    case ut_Long:
        return m_value.get_long() < other.m_value.get_long();
    case ut_Quad:
        return m_value.get_quad() < other.m_value.get_quad();
    case ut_Real:
        return m_value.get_real() < other.m_value.get_real();
    case ut_String:
        return m_value.get_chars() < other.m_value.get_chars();
    }
    return false;
}

bool P2Atom::operator<=(const P2Atom& other)
{
    switch (m_value.type()) {
    case ut_Invalid:
        return false;
    case ut_Bool:
        return m_value.get_bool() <= other.m_value.get_bool();
    case ut_Byte:
        return m_value.get_byte() <= other.m_value.get_byte();
    case ut_Word:
        return m_value.get_word() <= other.m_value.get_word();
    case ut_Addr:
    case ut_Long:
        return m_value.get_long() <= other.m_value.get_long();
    case ut_Quad:
        return m_value.get_quad() <= other.m_value.get_quad();
    case ut_Real:
        return m_value.get_real() <= other.m_value.get_real();
    case ut_String:
        return m_value.get_chars() <= other.m_value.get_chars();
    }
    return false;
}

bool P2Atom::operator>(const P2Atom& other)
{
    switch (m_value.type()) {
    case ut_Invalid:
        return false;
    case ut_Bool:
        return m_value.get_bool() > other.m_value.get_bool();
    case ut_Byte:
        return m_value.get_byte() > other.m_value.get_byte();
    case ut_Word:
        return m_value.get_word() > other.m_value.get_word();
    case ut_Addr:
    case ut_Long:
        return m_value.get_long() > other.m_value.get_long();
    case ut_Quad:
        return m_value.get_quad() > other.m_value.get_quad();
    case ut_Real:
        return m_value.get_real() > other.m_value.get_real();
    case ut_String:
        return m_value.get_chars() > other.m_value.get_chars();
    }
    return false;
}

bool P2Atom::operator>=(const P2Atom& other)
{
    switch (m_value.type()) {
    case ut_Invalid:
        return false;
    case ut_Bool:
        return m_value.get_bool() >= other.m_value.get_bool();
    case ut_Byte:
        return m_value.get_byte() >= other.m_value.get_byte();
    case ut_Word:
        return m_value.get_word() >= other.m_value.get_word();
    case ut_Addr:
    case ut_Long:
        return m_value.get_long() >= other.m_value.get_long();
    case ut_Quad:
        return m_value.get_quad() >= other.m_value.get_quad();
    case ut_Real:
        return m_value.get_real() >= other.m_value.get_real();
    case ut_String:
        return m_value.get_chars() >= other.m_value.get_chars();
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

QByteArray P2Atom::array(const P2Atom& atom)
{
    return atom.array();
}

QString P2Atom::string(const P2Atom& atom)
{
    return atom.string();
}
