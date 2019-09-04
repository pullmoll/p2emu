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

P2Atom::P2Atom(P2Atom::Type type)
    : m_type(type)
    , m_trait(None)
    , m_data()
{
}

P2Atom::P2Atom(const P2Atom& other)
    : m_type(other.m_type)
    , m_trait(None)
    , m_data(other.m_data)
{
}

P2Atom::P2Atom(bool value, Type type)
    : m_type(type)
    , m_trait(None)
    , m_data()
{
    set_uint(m_type, value & 1);
}

P2Atom::P2Atom(p2_BYTE value, Type type)
    : m_type(type)
    , m_trait(None)
    , m_data()
{
    set_uint(m_type, value);
}

P2Atom::P2Atom(p2_WORD value, Type type)
    : m_type(type)
    , m_trait(None)
    , m_data()
{
    set_uint(m_type, value);
}

P2Atom::P2Atom(p2_LONG value, Type type)
    : m_type(type)
    , m_trait(None)
    , m_data()
{
    set_uint(m_type, value);
}

P2Atom::P2Atom(p2_QUAD value, Type type)
    : m_type(type)
    , m_trait(None)
    , m_data()
{
    set_uint(m_type, value);
}

P2Atom::P2Atom(p2_REAL value, P2Atom::Type type)
    : m_type(type)
    , m_trait(None)
    , m_data()
{
    set_real(m_type, value);
}

/**
 * @brief Clear the atom
 */
void P2Atom::clear(Type type)
{
    m_data.clear();
    m_type = type;
    m_trait = None;
}

/**
 * @brief Return true, if the atom is invalid
 * @return true if empty, or false otherwise
 */
bool P2Atom::isNull() const
{
    return m_type == Invalid;
}

/**
 * @brief Return true, if the atom contains no data
 * @return true if empty, or false otherwise
 */
bool P2Atom::isEmpty() const
{
    return m_data.isEmpty();
}

/**
 * @brief Return true, if the atom's value is zero
 * @return true if zero, or false otherwise
 */
bool P2Atom::isZero() const
{
    if (isNull() || isEmpty())
        return true;
    switch (m_type) {
    case Invalid:
        return true;
    case Bool:
        return false == to_bool();
    case Byte:
        return 0 == to_byte();
    case Word:
        return 0 == to_word();
    case Addr:
        return 0 == to_long();
    case Long:
        return 0 == to_long();
    case Quad:
        return 0 == to_quad();
    case Real:
        return qFuzzyIsNull(to_real());
    case String:
        break;
    }
    return false;
}

/**
 * @brief Return true, if the atom is valid, i.e. contains data
 * @return true if valid, or false otherwise
 */
bool P2Atom::isValid() const
{
    return m_type != Invalid;
}

/**
 * @brief Return the trait of the atom
 * @return Enumeration value from Trait
 */
P2Atom::Trait P2Atom::trait() const
{
    return m_trait;
}

/**
 * @brief Return the size of the data in the atom
 * @return size of m_data
 */
int P2Atom::size() const
{
    return m_data.size();
}

p2_LONG P2Atom::usize() const
{
    return static_cast<p2_LONG>(m_data.size());
}

int P2Atom::count() const
{
    int count = 0;
    switch (m_type) {
    case Invalid:
        break;
    case Bool:
    case Byte:
    case String:
        count = (m_data.size() + sz_BYTE - 1) / sz_BYTE;
        break;
    case Word:
        count = (m_data.size() + sz_WORD - 1) / sz_WORD;
        break;
    case Addr:
        count = (m_data.size() + sz_LONG - 1) / sz_LONG;
        break;
    case Long:
        count = (m_data.size() + sz_LONG - 1) / sz_LONG;
        break;
    case Quad:
        count = (m_data.size() + sz_QUAD - 1) / sz_QUAD;
        break;
    case Real:
        count = (m_data.size() + sz_REAL - 1) / sz_REAL;
        break;
    }
    return static_cast<int>(count);
}

/**
 * @brief Return the type of the atom, i.e. max size inserted
 * @return One of the %Type enumeration values
 */
P2Atom::Type P2Atom::type() const
{
    return m_type;
}

const QString P2Atom::type_name() const
{
    switch (m_type) {
    case Invalid:
        return QStringLiteral("Invalid");
    case Bool:
        return QStringLiteral("Bool");
    case Byte:
        return QStringLiteral("Byte");
    case Word:
        return QStringLiteral("Word");
    case Addr:
        return QStringLiteral("PC");
    case Long:
        return QStringLiteral("Long");
    case Quad:
        return QStringLiteral("Quad");
    case Real:
        return QStringLiteral("Real");
    case String:
        return QStringLiteral("String");
    }
    return QStringLiteral("<invalid>");
}

/**
 * @brief Set the type of the atom
 * @brie type one of the %Type enumeration values
 */
void P2Atom::set_type(Type type)
{
    m_type = type;
}

/**
 * @brief Set the atom's trait
 * @param trait new trait to set
 * @return true if set, false if not changed
 */
bool P2Atom::set_trait(P2Atom::Trait trait)
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
bool P2Atom::add_trait(P2Atom::Trait trait)
{
    if (m_trait & trait)
        return false;
    m_trait = static_cast<Trait>(m_trait | trait);
    return true;
}

/**
 * @brief Append a number of bytes to the data
 * @param type new type of the atom
 * @param value with lower %nbits valid
 * @return truen on success, or false on error
 */
bool P2Atom::append_uint(Type type, p2_QUAD value)
{
    const int pos = m_data.size();
    switch (type) {
    case Invalid:
        return false;
    case Bool:
        m_data.resize(pos+1);
        m_data[pos] = static_cast<p2_BYTE>(value & 1);
        break;
    case Byte:
        m_data.resize(pos+1);
        m_data[pos] = static_cast<p2_BYTE>(value);
        break;
    case Word:
        m_data.resize(pos+2);
        m_data[pos+0] = static_cast<p2_BYTE>(value >>  0);
        m_data[pos+1] = static_cast<p2_BYTE>(value >>  8);
        break;
    case Addr:
        m_data.resize(pos+4);
        m_data[pos+0] = static_cast<p2_BYTE>(value >>  0);
        m_data[pos+1] = static_cast<p2_BYTE>(value >>  8);
        m_data[pos+2] = static_cast<p2_BYTE>(value >> 16);
        m_data[pos+3] = static_cast<p2_BYTE>(value >> 24);
        break;
    case Long:
        m_data.resize(pos+4);
        m_data[pos+0] = static_cast<p2_BYTE>(value >>  0);
        m_data[pos+1] = static_cast<p2_BYTE>(value >>  8);
        m_data[pos+2] = static_cast<p2_BYTE>(value >> 16);
        m_data[pos+3] = static_cast<p2_BYTE>(value >> 24);
        break;
    case Quad:
        m_data.resize(pos+8);
        m_data[pos+0] = static_cast<p2_BYTE>(value >>  0);
        m_data[pos+1] = static_cast<p2_BYTE>(value >>  8);
        m_data[pos+2] = static_cast<p2_BYTE>(value >> 16);
        m_data[pos+3] = static_cast<p2_BYTE>(value >> 24);
        m_data[pos+4] = static_cast<p2_BYTE>(value >> 32);
        m_data[pos+5] = static_cast<p2_BYTE>(value >> 40);
        m_data[pos+6] = static_cast<p2_BYTE>(value >> 48);
        m_data[pos+7] = static_cast<p2_BYTE>(value >> 56);
        break;
    case Real:
        return append_real(Real, static_cast<p2_REAL>(value));
    case String:
        m_data += static_cast<p2_BYTE>(value);
        value >>= 8;
        for (int i = 1; i < 8 && value != 0; i++, value >>= 8)
            m_data += static_cast<p2_BYTE>(value);
        break;
    }
    return true;
}

/**
 * @brief Append a number of bytes to the data
 * @param type new type of the atom
 * @param value with lower %nbits valid
 * @return truen on success, or false on error
 */
bool P2Atom::append_real(Type type, p2_REAL value)
{
    const int pos = m_data.size();
    p2_QUAD quad = static_cast<p2_QUAD>(value);
    switch (type) {
    case Invalid:
        return false;
    case Bool: case Byte: case Word: case Addr:
    case Long: case Quad: case String:
        return append_uint(type, quad);
    case Real:
        {
            const p2_BYTE* data = reinterpret_cast<const p2_BYTE*>(&value);
            m_data.resize(pos + sz_REAL);
            for (int i = 0; i < sz_REAL; i++)
                m_data[pos+i] = data[i];
        }
        break;
    }
    return true;
}

bool P2Atom::append_bits(int nbits, p2_QUAD value)
{
    if (nbits <= 0)
        return false;
    if (nbits <= 8)
        return append_uint(Byte, value);
    if (nbits <= 16)
        return append_uint(Word, value);
    if (nbits <= 32)
        return append_uint(Long, value);
    if (nbits <= 64)
        return append_uint(Quad, value);
    return false;
}

/**
 * @brief Append a value using the current m_type
 * @param value value to append
 * @return true on success, or false on error
 */
bool P2Atom::append(p2_QUAD value)
{
    return append_uint(m_type, value);
}

/**
 * @brief Append a value using the current m_type
 * @param value value to append
 * @return true on success, or false on error
 */
bool P2Atom::append(p2_REAL value)
{
    return append_real(m_type, value);
}

/**
 * @brief Append another P2Atom to this atom
 * @param atom atom to append
 * @return true on success, or false on error
 */
bool P2Atom::append(const P2Atom& atom)
{
    m_data += atom.m_data;
    return true;
}

/**
 * @brief Append contents of a QByteArray to this atom
 * @param data QByteArray to append
 * @return true on success, or false on error
 */
bool P2Atom::append(const QByteArray& value)
{
    const int pos = m_data.size();
    const int size = value.size();
    m_data.resize(pos+size);
    if (size > 0)
        memcpy(m_data.data(), value.constData(), static_cast<size_t>(size));
    return true;
}

/**
 * @brief Set the m_data to a new value
 * @param type Type of the data to set
 * @param value with lower bits depending on type
 * @return true on success, or false on error
 */
bool P2Atom::set_uint(Type type, p2_QUAD value)
{
    switch (type) {
    case Invalid:
        return false;
    case Bool:
        m_type = type;
        m_data.resize(1);
        m_data[0] = static_cast<p2_BYTE>(value & 1);
        break;
    case Byte:
        m_type = type;
        m_data.resize(1);
        m_data[0] = static_cast<p2_BYTE>(value);
        break;
    case Word:
        m_type = type;
        m_data.resize(2);
        m_data[0] = static_cast<p2_BYTE>(value >> 0);
        m_data[1] = static_cast<p2_BYTE>(value >> 8);
        break;
    case Addr:
        m_type = type;
        m_data.resize(4);
        m_data[0] = static_cast<p2_BYTE>(value >>  0);
        m_data[1] = static_cast<p2_BYTE>(value >>  8);
        m_data[2] = static_cast<p2_BYTE>(value >> 16);
        m_data[3] = static_cast<p2_BYTE>(value >> 24);
        break;
    case Long:
        m_type = type;
        m_data.resize(4);
        m_data[0] = static_cast<p2_BYTE>(value >>  0);
        m_data[1] = static_cast<p2_BYTE>(value >>  8);
        m_data[2] = static_cast<p2_BYTE>(value >> 16);
        m_data[3] = static_cast<p2_BYTE>(value >> 24);
        break;
    case Quad:
        m_type = type;
        m_data.resize(8);
        m_data[0] = static_cast<p2_BYTE>(value >>  0);
        m_data[1] = static_cast<p2_BYTE>(value >>  8);
        m_data[2] = static_cast<p2_BYTE>(value >> 16);
        m_data[3] = static_cast<p2_BYTE>(value >> 24);
        m_data[4] = static_cast<p2_BYTE>(value >> 32);
        m_data[5] = static_cast<p2_BYTE>(value >> 40);
        m_data[6] = static_cast<p2_BYTE>(value >> 48);
        m_data[7] = static_cast<p2_BYTE>(value >> 56);
        break;
    case Real:
        return set_real(Real, static_cast<p2_REAL>(value));
    case String:
        m_type = type;
        m_data.resize(1);
        m_data[0] = static_cast<p2_BYTE>(value);
        value >>= 8;
        for (int i = 1; i < 8 && value != 0; i++, value >>= 8) {
            m_data.resize(i+1);
            m_data[i] = static_cast<p2_BYTE>(value);
        }
        break;
    }
    return true;
}

bool P2Atom::set_real(P2Atom::Type type, p2_REAL value)
{
    p2_QUAD quad = static_cast<p2_QUAD>(value);
    switch (type) {
    case Invalid:
        return false;
    case Bool: case Byte: case Word: case Addr:
    case Long: case Quad: case String:
        return set_uint(type, quad);
    case Real:
        // FIXME: endianness independent float values?
        m_type = Real;
        m_data.resize(sz_REAL);
        memcpy(m_data.data(), reinterpret_cast<p2_BYTE*>(&value), sz_REAL);
        break;
    }
    return true;
}

/**
 * @brief Set the data to a new value
 * @param nbits number of bits to set
 * @param value with lower %nbits valid
 * @return true on success, or false on error
 */
bool P2Atom::set(int nbits, p2_QUAD value)
{
    m_data.clear();
    return append_bits(nbits, value);
}

/**
 * @brief Set the data to a new value keeping the %m_type
 * @param value new value
 * @return true on success, or false on error
 */
bool P2Atom::set(p2_QUAD value)
{
    m_data.clear();
    return append_uint(m_type, value);
}

/**
 * @brief Set the data to a new value keeping the %m_type
 * @param value new value
 * @return true on success, or false on error
 */
bool P2Atom::set(p2_REAL value)
{
    m_data.clear();
    return append_real(m_type, value);
}

/**
 * @brief Format the atom's value as a string respecting the type
 * @return QString with value
 */
QString P2Atom::str(p2_format_e fmt) const
{
    QString result;
    switch (m_type) {
    case Invalid:
        result = QLatin1String("<invalid>");
        break;
    case Bool:
        result = QString("<%1> %1")
                 .arg(type_name())
                 .arg(to_bool() ? "true" : "false");
        break;
    case Byte:
        switch (fmt) {
        case fmt_dec:
            result = QString("<%1> %2")
                     .arg(type_name())
                     .arg(to_byte());
            break;
        case fmt_bin:
            result = QString("<%1> %%2")
                     .arg(type_name())
                     .arg(to_byte(), 8, 2, QChar('0'));
            break;
        case fmt_byt:
        case fmt_hex:
            result = QString("<%1> $%2")
                     .arg(type_name())
                     .arg(to_byte(), 2, 16, QChar('0'));
        }
        break;
    case Word:
        switch (fmt) {
        case fmt_dec:
            result = QString("<%1> %2")
                     .arg(type_name())
                     .arg(to_word());
            break;
        case fmt_bin:
            result = QString("<%1> %%2_%3")
                     .arg(type_name())
                     .arg(to_word() >> 8, 8, 2, QChar('0'))
                     .arg(to_word() & 0xff, 8, 2, QChar('0'));
            break;
        case fmt_byt:
            result = QString("<%1> $%2 $%3")
                     .arg(type_name())
                     .arg(to_word() >> 8, 2, 16, QChar('0'))
                     .arg(to_word() & 0xff, 2, 16, QChar('0'));
            break;
        case fmt_hex:
            result = QString("<%1> $%2")
                     .arg(type_name())
                     .arg(to_word(), 4, 16, QChar('0'));
        }
        break;
    case Addr:
    case Long:
        switch (fmt) {
        case fmt_dec:
            result = QString("<%1> %2")
                     .arg(type_name())
                     .arg(to_long());
            break;
        case fmt_bin:
            result = QString("<%1> %%2_%3_%4_%5")
                     .arg(type_name())
                     .arg((to_long() >> 24) & 0xff, 8, 2, QChar('0'))
                     .arg((to_long() >> 16) & 0xff, 8, 2, QChar('0'))
                     .arg((to_long() >>  8) & 0xff, 8, 2, QChar('0'))
                     .arg((to_long() >>  0) & 0xff, 8, 2, QChar('0'));
            break;
        case fmt_byt:
            result = QString("<%1> $%2 $%3 $%4 $%5")
                     .arg(type_name())
                     .arg((to_long() >> 24) & 0xff, 2, 16, QChar('0'))
                     .arg((to_long() >> 16) & 0xff, 2, 16, QChar('0'))
                     .arg((to_long() >>  8) & 0xff, 2, 16, QChar('0'))
                     .arg((to_long() >>  0) & 0xff, 2, 16, QChar('0'));
            break;
        case fmt_hex:
            result = QString("<%1> $%2")
                     .arg(type_name())
                     .arg(to_long(), 8, 16, QChar('0'));
        }
        break;
    case Quad:
        switch (fmt) {
        case fmt_dec:
            result = QString("<%1> %2")
                     .arg(type_name())
                     .arg(to_quad());
            break;
        case fmt_bin:
            result = QString("<%1> %%2_%3_%4_%5_%7_%8_%9_%10")
                     .arg(type_name())
                     .arg((to_quad() >> 56) & 0xff, 8, 2, QChar('0'))
                     .arg((to_quad() >> 48) & 0xff, 8, 2, QChar('0'))
                     .arg((to_quad() >> 40) & 0xff, 8, 2, QChar('0'))
                     .arg((to_quad() >> 32) & 0xff, 8, 2, QChar('0'))
                     .arg((to_quad() >> 24) & 0xff, 8, 2, QChar('0'))
                     .arg((to_quad() >> 16) & 0xff, 8, 2, QChar('0'))
                     .arg((to_quad() >>  8) & 0xff, 8, 2, QChar('0'))
                     .arg((to_quad() >>  0) & 0xff, 8, 2, QChar('0'));
            break;
        case fmt_byt:
            result = QString("<%1> $%2 $%3 $%4 $%5 $%7 $%8 $%9 $%10")
                     .arg(type_name())
                     .arg((to_quad() >> 56) & 0xff, 2, 16, QChar('0'))
                     .arg((to_quad() >> 48) & 0xff, 2, 16, QChar('0'))
                     .arg((to_quad() >> 40) & 0xff, 2, 16, QChar('0'))
                     .arg((to_quad() >> 32) & 0xff, 2, 16, QChar('0'))
                     .arg((to_quad() >> 24) & 0xff, 2, 16, QChar('0'))
                     .arg((to_quad() >> 16) & 0xff, 2, 16, QChar('0'))
                     .arg((to_quad() >>  8) & 0xff, 2, 16, QChar('0'))
                     .arg((to_quad() >>  0) & 0xff, 2, 16, QChar('0'));
            break;
        case fmt_hex:
            result = QString("<%1> $%2")
                     .arg(type_name())
                     .arg(to_quad(), 16, 16, QChar('0'));
        }
        break;
    case Real:
        result = QString("<Real> %1").arg(to_real(), 0, 'f');
        break;
    case String:
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

    uchar* data = reinterpret_cast<uchar *>(m_data.data());

    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set_uint(m_type, !to_bool());
        break;
    case Byte:
        set_uint(m_type, ~to_byte());
        break;
    case Word:
        set_uint(m_type, ~to_word());
        break;
    case Addr:
        set_uint(m_type, ~to_long());
        break;
    case Long:
        set_uint(m_type, ~to_long());
        break;
    case Quad:
        set_uint(m_type, ~to_quad());
        break;
    case Real:
        break;
    case String:
        for (int i = 0; i < m_data.size(); i++)
            data[i] ^= 0xff;
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

    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set_uint(m_type, !to_bool());
        break;
    case Byte:
        set_uint(m_type, ~to_byte() + 1);
        break;
    case Word:
        set_uint(m_type, ~to_word() + 1);
        break;
    case Addr:
        set_uint(m_type, ~to_long() + 1);
        break;
    case Long:
        set_uint(m_type, ~to_long() + 1);
        break;
    case Quad:
        set_uint(m_type, ~to_quad() + 1);
        break;
    case Real:
        set_real(m_type, -to_real());
        break;
    case String:
        // 1's complement
        complement1(true);
        // add 1
        arith_add(P2Atom(1u, m_type));
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

    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set_uint(m_type, !to_bool());
        break;
    case Byte:
        set_uint(m_type, 0 == to_byte() ? 1 : 0);
        break;
    case Word:
        set_uint(m_type, 0 == to_word() ? 1 : 0);
        break;
    case Addr:
        set_uint(m_type, 0 == to_long() ? 1 : 0);
        break;
    case Long:
        set_uint(m_type, 0 == to_long() ? 1 : 0);
        break;
    case Quad:
        set_uint(m_type, 0 == to_quad() ? 1 : 0);
        break;
    case Real:
        set_real(m_type, qFuzzyIsNull(to_real()) ? 1.0 : 0.0);
        break;
    case String:
        {
            bool zero = true;
            for (int i = 0; zero && i < m_data.size(); i++)
                if (m_data[i])
                    zero = false;
            m_data.resize(1);
            m_data[0] = zero;
        }
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

    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set(Bool, to_bool());
        break;
    case Byte:
        set(Bool, 0 != to_byte() ? 1 : 0);
        break;
    case Word:
        set(Bool, 0 != to_word() ? 1 : 0);
        break;
    case Addr:
        set(Bool, 0 != to_long() ? 1 : 0);
        break;
    case Long:
        set(Bool, 0 != to_long() ? 1 : 0);
        break;
    case Quad:
        set(Bool, 0 != to_quad() ? 1 : 0);
        break;
    case Real:
        set_real(m_type, qFuzzyIsNull(to_real()) ? 0.0 : 1.0);
        break;
    case String:
        set(Bool, to_bool());
        break;
    }
}

/**
 * @brief Set atom to its pre/post decrement value
 * @param flag do the decrement if true, leave unchanged otherwise
 */
void P2Atom::unary_dec(const P2Atom& val)
{
    if (val.isNull())
        return;

    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set_uint(m_type, !to_bool());
        break;
    case Byte:
        set_uint(m_type, to_byte() - val.to_byte());
        break;
    case Word:
        set_uint(m_type, to_word() - val.to_word());
        break;
    case Addr:
        set_uint(m_type, to_long() - val.to_long());
        break;
    case Long:
        set_uint(m_type, to_long() - val.to_long());
        break;
    case Quad:
        set_uint(m_type, to_quad() - val.to_quad());
        break;
    case Real:
        set_real(m_type, to_real() - val.to_real());
        break;
    case String:
        {
            p2_LONG value = val.to_long();
            uint cy = 0;
            for (int i = 0; i < size(); i++) {
                const uint byte = m_data[i] - value - cy;
                cy = static_cast<uint>(byte >> 8);
                m_data[i] = static_cast<p2_BYTE>(byte);
                value >>= 8;
            }
            if (cy)
                m_data.append(static_cast<p2_BYTE>(cy));
        }
        break;
    }
}

/**
 * @brief Set atom to its pre/post increment value
 * @param flag do the increment if true, leave unchanged otherwise
 */
void P2Atom::unary_inc(const P2Atom& val)
{
    if (val.isNull())
        return;

    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set_uint(m_type, !to_bool());
        break;
    case Byte:
        set_uint(m_type, to_byte() + val.to_byte());
        break;
    case Word:
        set_uint(m_type, to_word() + val.to_word());
        break;
    case Addr:
        set_uint(m_type, to_long() + val.to_long());
        break;
    case Long:
        set_uint(m_type, to_long() + val.to_long());
        break;
    case Quad:
        set_uint(m_type, to_quad() + val.to_quad());
        break;
    case Real:
        set_real(m_type, to_real() + val.to_real());
        break;
    case String:
        {
            p2_LONG value = val.to_long();
            uint cy = 0;
            for (int i = 0; i < size(); i++) {
                const uint byte = m_data[i] + value + cy;
                cy = static_cast<uint>(byte >> 8);
                m_data[i] = static_cast<p2_BYTE>(byte);
                value >>= 8;
            }
            if (cy)
                m_data.append(static_cast<p2_BYTE>(cy));
        }
        break;
    }
}

/**
 * @brief Set atom to its multiplication result value
 * @param val value to multiply by
 */
void P2Atom::arith_mul(const P2Atom& val)
{
    if (val.isNull())
        return;

    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set_uint(m_type, false);
        break;
    case Byte:
        set_uint(m_type, to_byte() * val.to_byte());
        break;
    case Word:
        set_uint(m_type, to_word() * val.to_word());
        break;
    case Addr:
        set_uint(m_type, to_long() * val.to_long());
        break;
    case Long:
        set_uint(m_type, to_long() * val.to_long());
        break;
    case Quad:
        set_uint(m_type, to_quad() * val.to_quad());
        break;
    case Real:
        set_real(m_type, to_real() * val.to_real());
        break;
    case String:
        // TODO: implement
        Q_ASSERT(m_type);
    }

}

/**
 * @brief Set atom to its division result value
 * @param val value to multiply by
 */
void P2Atom::arith_div(const P2Atom& val)
{
    if (val.isZero()) {
        // Division by zero
        if (Real == m_type)
            set_real(Real, nan("qreal"));
        else
            set_uint(m_type, ~0u);
        return;
    }
    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set_uint(m_type, false);
        break;
    case Byte:
        set_uint(m_type, to_byte() / val.to_byte());
        break;
    case Word:
        set_uint(m_type, to_word() / val.to_word());
        break;
    case Addr:
        set_uint(m_type, to_long() / val.to_long());
        break;
    case Long:
        set_uint(m_type, to_long() / val.to_long());
        break;
    case Quad:
        set_uint(m_type, to_quad() / val.to_quad());
        break;
    case Real:
        set_real(m_type, to_real() / val.to_real());
        break;
    case String:
        // TODO: implement
        Q_ASSERT(m_type);
    }

}

/**
 * @brief Set atom to its modulo result value
 * @param val value to do modulo by
 */
void P2Atom::arith_mod(const P2Atom& val)
{
    if (val.isZero()) {
        // Modulus with zero
        if (Real == m_type)
            set_real(Real, nan("qreal"));
        else
            set_uint(m_type, ~0u);
        return;
    }
    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set_uint(m_type, val.to_bool() ? false : true );
        break;
    case Byte:
        set_uint(m_type, to_byte() % val.to_byte());
        break;
    case Word:
        set_uint(m_type, to_word() % val.to_word());
        break;
    case Addr:
        set_uint(m_type, to_long() % val.to_long());
        break;
    case Long:
        set_uint(m_type, to_long() % val.to_long());
        break;
    case Quad:
        set_uint(m_type, to_quad() % val.to_quad());
        break;
    case Real:
        set_real(m_type, fmod(to_real(), val.to_real()));
        break;
    case String:
        // TODO: implement
        Q_ASSERT(m_type);
    }

}

/**
 * @brief Set atom to its addition result value
 * @param val value to add
 */
void P2Atom::arith_add(const P2Atom& val)
{
    if (val.isZero())
        return;

    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set_uint(m_type, (to_bool() + val.to_bool()) & 1 ? true : false);
        break;
    case Byte:
        set_uint(m_type, to_byte() + val.to_byte());
        break;
    case Word:
        set_uint(m_type, to_word() + val.to_word());
        break;
    case Addr:
        set_uint(m_type, to_long() + val.to_long());
        break;
    case Long:
        set_uint(m_type, to_long() + val.to_long());
        break;
    case Quad:
        set_uint(m_type, to_quad() + val.to_quad());
        break;
    case Real:
        set_real(m_type, to_real() + val.to_real());
        break;
    case String:
        {
            p2_QUAD value = val.to_long();
            uint cy = 0;
            for (int i = 0; i < size(); i++) {
                const p2_QUAD byte = m_data[i] + value + cy;
                cy = static_cast<uint>(byte >> 8);
                m_data[i] = static_cast<p2_BYTE>(byte);
                value >>= 8;
            }
            if (cy)
                m_data.append(static_cast<p2_BYTE>(cy));
        }
        break;
    }
}

/**
 * @brief Set atom to its subtraction result value
 * @param val value to subtract
 */
void P2Atom::arith_sub(const P2Atom& val)
{
    if (val.isZero())
        return;

    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set_uint(m_type, (to_bool() - val.to_bool()) & 1 ? true : false);
        break;
    case Byte:
        set_uint(m_type, to_byte() - val.to_byte());
        break;
    case Word:
        set_uint(m_type, to_word() - val.to_word());
        break;
    case Addr:
        set_uint(m_type, to_long() - val.to_long());
        break;
    case Long:
        set_uint(m_type, to_long() - val.to_long());
        break;
    case Quad:
        set_uint(m_type, to_quad() - val.to_quad());
        break;
    case Real:
        set_real(m_type, to_real() - val.to_real());
        break;
    case String:
        {
            p2_QUAD value = val.to_quad();
            uint cy = 0;
            for (int i = 0; i < size(); i++) {
                const p2_QUAD byte = m_data[i] - value - cy;
                cy = static_cast<uint>(byte >> 8);
                m_data[i] = static_cast<p2_BYTE>(byte);
                value >>= 8;
            }
            if (cy)
                m_data.append(static_cast<p2_BYTE>(cy));
        }
        break;
    }
}

/**
 * @brief Set atom to its left shifted value
 * @param bits number of bits to shift left
 */
void P2Atom::binary_shl(const P2Atom& val)
{
    if (val.isZero())
        return;
    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set_uint(m_type, false);
        break;
    case Byte:
        set_uint(m_type, to_long() << val.to_byte());
        break;
    case Word:
        set_uint(m_type, to_long() << val.to_byte());
        break;
    case Addr:
        set_uint(m_type, to_long() << val.to_byte());
        break;
    case Long:
        set_uint(m_type, to_long() << val.to_byte());
        break;
    case Quad:
        set_uint(m_type, to_quad() << val.to_byte());
        break;
    case Real:
        set_real(m_type, to_real() * (1u << val.to_byte()));
        break;
    case String:
        // TODO: implement
        Q_ASSERT(m_type);
    }
}

/**
 * @brief Set atom to its right shifted value
 * @param bits number of bits to shift right
 */
void P2Atom::binary_shr(const P2Atom& val)
{
    if (val.isZero())
        return;
    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set_uint(m_type, false);
        break;
    case Byte:
        set_uint(m_type, to_byte() >> val.to_byte());
        break;
    case Word:
        set_uint(m_type, to_word() >> val.to_byte());
        break;
    case Addr:
        set_uint(m_type, to_long() >> val.to_byte());
        break;
    case Long:
        set_uint(m_type, to_long() >> val.to_byte());
        break;
    case Quad:
        set_uint(m_type, to_quad() >> val.to_byte());
        break;
    case Real:
        set_real(m_type, to_real() / (1u << val.to_byte()));
        break;
    case String:
        // TODO: implement
        Q_ASSERT(m_type);
    }
}

/**
 * @brief Set atom to its binary AND value
 * @param mask value to AND with
 */
void P2Atom::binary_and(const P2Atom& val)
{
    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set_uint(m_type, (to_bool() & val.to_bool()) & 1 ? true : false);
        break;
    case Byte:
        set_uint(m_type, to_byte() & val.to_byte());
        break;
    case Word:
        set_uint(m_type, to_word() & val.to_word());
        break;
    case Addr:
        set_uint(m_type, to_long() & val.to_word());
        break;
    case Long:
        set_uint(m_type, to_long() & val.to_long());
        break;
    case Quad:
        set_uint(m_type, to_quad() & val.to_quad());
        break;
    case Real:
        set_real(m_type, static_cast<p2_REAL>(to_quad() & val.to_quad()));
        break;
    case String:
        // TODO: implement
        Q_ASSERT(m_type);
    }
}

/**
 * @brief Set atom to its binary XOR value
 * @param mask value to XOR with
 */
void P2Atom::binary_xor(const P2Atom& val)
{
    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set_uint(m_type, (to_bool() ^ val.to_bool()) & 1 ? true : false);
        break;
    case Byte:
        set_uint(m_type, to_byte() ^ val.to_byte());
        break;
    case Word:
        set_uint(m_type, to_word() ^ val.to_word());
        break;
    case Addr:
        set_uint(m_type, to_long() ^ val.to_long());
        break;
    case Long:
        set_uint(m_type, to_long() ^ val.to_long());
        break;
    case Quad:
        set_uint(m_type, to_quad() ^ val.to_quad());
        break;
    case Real:
        set_real(m_type, static_cast<p2_REAL>(to_quad() ^ val.to_quad()));
        break;
    case String:
        // TODO: implement
        Q_ASSERT(m_type);
    }
}

/**
 * @brief Set atom to its binary OR value
 * @param mask value to OR with
 */
void P2Atom::binary_or(const P2Atom& val)
{
    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set_uint(m_type, (to_bool() | val.to_bool()) & 1 ? true : false);
        break;
    case Byte:
        set_uint(m_type, to_byte() | val.to_byte());
        break;
    case Word:
        set_uint(m_type, to_word() | val.to_word());
        break;
    case Addr:
        set_uint(m_type, to_long() | val.to_long());
        break;
    case Long:
        set_uint(m_type, to_long() | val.to_long());
        break;
    case Quad:
        set_uint(m_type, to_quad() | val.to_quad());
        break;
    case Real:
        set_real(m_type, static_cast<p2_REAL>(to_quad() ^ val.to_quad()));
        break;
    case String:
        // TODO: implement
        Q_ASSERT(m_type);
    }
}

void P2Atom::binary_rev()
{
    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        break;
    case Byte:
        set_uint(m_type, P2Util::reverse(to_byte()));
        break;
    case Word:
        set_uint(m_type, P2Util::reverse(to_word()));
        break;
    case Addr:
        set_uint(m_type, P2Util::reverse(to_long()));
        break;
    case Long:
        set_uint(m_type, P2Util::reverse(to_long()));
        break;
    case Quad:
        set_uint(m_type, P2Util::reverse(to_quad()));
        break;
    case Real:
        set_real(m_type, static_cast<p2_REAL>(P2Util::reverse(to_quad())));
        break;
    case String:
        // TODO: implement
        Q_ASSERT(m_type);
    }
}

void P2Atom::reverse(const P2Atom& val)
{
    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        break;
    case Byte:
        set_uint(m_type, P2Util::reverse(to_byte(), val.to_byte()));
        break;
    case Word:
        set_uint(m_type, P2Util::reverse(to_word(), val.to_word()));
        break;
    case Addr:
        set_uint(m_type, P2Util::reverse(to_long(), val.to_long()));
        break;
    case Long:
        set_uint(m_type, P2Util::reverse(to_long(), val.to_long()));
        break;
    case Quad:
        set_uint(m_type, P2Util::reverse(to_quad(), val.to_long()));
        break;
    case Real:
        set_real(m_type, static_cast<p2_REAL>(P2Util::reverse(to_quad(), val.to_long())));
        break;
    case String:
        // TODO: implement
        Q_ASSERT(m_type);
    }
}

void P2Atom::encode(const P2Atom& val)
{
    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        break;
    case Byte:
        set_uint(m_type, P2Util::encode(val.to_byte()));
        break;
    case Word:
        set_uint(m_type, P2Util::encode(val.to_word()));
        break;
    case Addr:
        set_uint(m_type, P2Util::encode(val.to_long()));
        break;
    case Long:
        set_uint(m_type, P2Util::encode(val.to_long()));
        break;
    case Quad:
        set_uint(m_type, P2Util::encode(val.to_quad()));
        break;
    case Real:
        set_real(m_type, static_cast<p2_REAL>(P2Util::encode(val.to_quad())));
        break;
    case String:
        // TODO: implement
        Q_ASSERT(m_type);
    }
}

void P2Atom::decode(const P2Atom& val)
{
    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        break;
    case Byte:
        set_uint(m_type, P2Util::lzc(val.to_byte()));
        break;
    case Word:
        set_uint(m_type, P2Util::lzc(val.to_word()));
        break;
    case Addr:
        set_uint(m_type, P2Util::lzc(val.to_long()));
        break;
    case Long:
        set_uint(m_type, P2Util::lzc(val.to_long()));
        break;
    case Quad:
        set_uint(m_type, P2Util::lzc(val.to_quad()));
        break;
    case Real:
        set_real(m_type, static_cast<p2_REAL>(P2Util::lzc(val.to_quad())));
        break;
    case String:
        // TODO: implement
        Q_ASSERT(m_type);
    }
}

/**
 * @brief Return data as a single byte
 * @param ok optional pointer to a bool set to true if data is available
 * @return One p2_BYTE
 */
bool P2Atom::to_bool(bool* ok) const
{
    if (ok)
        *ok = m_data.size() > 0;
    return m_data.count(0x00u) > 0;
}

/**
 * @brief Return data as a single byte
 * @param ok optional pointer to a bool set to true if data is available
 * @return One p2_BYTE
 */
p2_BYTE P2Atom::to_byte(bool* ok) const
{
    return private_val<p2_BYTE>(ok);
}

/**
 * @brief Return data as a single word
 * @param ok optional pointer to a bool set to true if data is available
 * @return One p2_WORD
 */
p2_WORD P2Atom::to_word(bool* ok) const
{
    return private_val<p2_WORD>(ok);
}

/**
 * @brief Return data as a single long
 * @param ok optional pointer to a bool set to true if data is available
 * @return One p2_LONG
 */
p2_LONG P2Atom::to_long(bool* ok) const
{
    return private_val<p2_LONG>(ok);
}

/**
 * @brief Return data as a single quad
 * @param ok optional pointer to a bool set to true if data is available
 * @return One p2_LONG
 */
p2_QUAD P2Atom::to_quad(bool* ok) const
{
    return private_val<p2_QUAD>(ok);
}

/**
 * @brief Return data as a single quad
 * @param ok optional pointer to a bool set to true if data is available
 * @return One p2_LONG
 */
p2_REAL P2Atom::to_real(bool* ok) const
{
    return private_val<p2_REAL>(ok);
}

/**
 * @brief Return data as a QString
 * @param ok optional pointer to a bool set to true if data is available
 * @return QString with the data
 */
QString P2Atom::to_string(bool* ok) const
{
    if (ok)
        *ok = m_data.size() > 0;
    return QString::fromUtf8(reinterpret_cast<const char *>(m_data.constData()), m_data.size());
}

/**
 * @brief Return data as a QByteArray
 * @param ok optional pointer to a bool set to true if data is available
 * @return QString with the data
 */
QByteArray P2Atom::to_array(bool* ok) const
{
    if (ok)
        *ok = m_data.size() > 0;
    QByteArray ba = QByteArray::fromRawData(reinterpret_cast<const char *>(m_data.constData()), m_data.size());
    return ba;
}

/**
 * @brief Return data as a vector of bytes
 * @return p2_BYTEs of all data
 */
p2_BYTES P2Atom::to_bytes(bool* ok) const
{
    if (ok)
        *ok = m_data.size() > 0;
    return p2_BYTES(m_data);
}

/**
 * @brief Return data as a vector of words
 * @return p2_WORDs of all data
 */
p2_WORDS P2Atom::to_words(bool* ok) const
{
    // reassemble BYTEs into WORDs (little endian)
    constexpr int sz = sz_WORD;
    const int size = m_data.size();
    p2_WORDS result;
    p2_WORD value = 0;
    int filled = 0;

    for (int i = 0; i < size; i++) {
        value |= static_cast<p2_WORD>(m_data[i]) << (8 * filled);
        if (++filled == sz) {
            result += value;
            filled = 0;
            value = 0;
        }
    }

    if (filled)
        result += value;

    if (nullptr != ok)
        *ok = true;
    return result;
}

/**
 * @brief Return data as a vector of longs
 * @return p2_LONGs of all data
 */
p2_LONGS P2Atom::to_longs(bool* ok) const
{
    // reassemble BYTEs into WORDs (little endian)
    constexpr int sz = sz_LONG;
    const int size = m_data.size();
    p2_LONGS result;
    p2_LONG value = 0;
    int filled = 0;

    for (int i = 0; i < size; i++) {
        value |= static_cast<p2_LONG>(m_data[i]) << (8 * filled);
        if (++filled == sz) {
            result += value;
            filled = 0;
            value = 0;
        }
    }

    if (filled)
        result += value;

    if (nullptr != ok)
        *ok = true;

    return result;
}

void P2Atom::make_real()
{
    p2_REAL real = private_val<p2_REAL>(nullptr);
    set_real(Real, real);
}

P2Atom& P2Atom::operator = (const P2Atom& other)
{
    m_type = other.m_type;
    m_trait = other.m_trait;
    m_data = other.m_data;
    return *this;
}

bool P2Atom::operator==(const P2Atom& other)
{
    return m_type == other.m_type && m_trait == other.m_trait && m_data == other.m_data;
}

bool P2Atom::operator!=(const P2Atom& other)
{
    return m_type != other.m_type || m_trait != other.m_trait || m_data != other.m_data;
}

bool P2Atom::operator<(const P2Atom& other)
{
    return m_data < other.m_data;
}

bool P2Atom::operator<=(const P2Atom& other)
{
    return m_data <= other.m_data;
}

bool P2Atom::operator>(const P2Atom& other)
{
    return m_data > other.m_data;
}

bool P2Atom::operator>=(const P2Atom& other)
{
    return m_data >= other.m_data;
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
