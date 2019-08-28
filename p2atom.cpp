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
#include "p2atom.h"
#include "p2util.h"

P2Atom::P2Atom()
    : m_type(Invalid)
    , m_data()
{
}

P2Atom::P2Atom(const P2Atom& other)
    : m_type(other.m_type)
    , m_data(other.m_data)
{
}

P2Atom::P2Atom(bool value)
    : m_type(Bool)
    , m_data()
{
    set(value & 1);
}

P2Atom::P2Atom(p2_BYTE value)
    : m_type(Byte)
    , m_data()
{
    set(value);
}

P2Atom::P2Atom(p2_WORD value)
    : m_type(Word)
    , m_data()
{
    set(value);
}

P2Atom::P2Atom(p2_LONG value)
    : m_type(Long)
    , m_data()
{
    set(value);
}

P2Atom::P2Atom(p2_QUAD value)
    : m_type(Quad)
    , m_data()
{
    set(value);
}

/**
 * @brief Clear the atom
 */
void P2Atom::clear(Type type)
{
    m_data.clear();
    m_type = type;
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
 * @brief Return true, if the atom is valid, i.e. contains data
 * @return true if valid, or false otherwise
 */
bool P2Atom::isValid() const
{
    return m_type != Invalid;
}

/**
 * @brief Return the size of the data in the atom
 * @return size of m_data
 */
int P2Atom::size() const
{
    return m_data.size();
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
        count = m_data.size();
        break;
    case Word:
        count = (m_data.size() + 1) / 2;
        break;
    case PC:
        count = (m_data.size() + 3) / 4;
        break;
    case Long:
        count = (m_data.size() + 3) / 4;
        break;
    case Quad:
        count = (m_data.size() + 7) / 8;
        break;
    }
    return count;
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
    case P2Atom::Invalid:
        return QStringLiteral("Invalid");
    case P2Atom::Bool:
        return QStringLiteral("Bool");
    case P2Atom::Byte:
        return QStringLiteral("Byte");
    case P2Atom::Word:
        return QStringLiteral("Word");
    case P2Atom::Long:
        return QStringLiteral("Long");
    case P2Atom::Quad:
        return QStringLiteral("Quad");
    case P2Atom::PC:
        return QStringLiteral("PC");
    case P2Atom::String:
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

p2_QUAD P2Atom::value(bool* ok) const
{
    const p2_BWLQ* data = reinterpret_cast<const p2_BWLQ*>(m_data.constData());
    p2_QUAD result = 0;
    if (ok)
        *ok = isValid();
    if (m_data.size() > 7)
        return data->q;
    if (m_data.size() > 3)
        return data->l0;
    if (m_data.size() > 1)
        return data->w0;
    if (m_data.size() > 0)
        return data->b0;

    const int bits = 8 * m_data.size();
    const int need = 8 * sizeof(p2_QUAD);
    if (ok)
        *ok = need <= bits;
    for (int i = 0; i < bits && i < need; i += 8)
        result |= static_cast<p2_QUAD>(data->b[i/8]) << i;
    return result;
}

/**
 * @brief Append a number of bytes to the data
 * @param type new type of the atom
 * @param value with lower %nbits valid
 * @return truen on success, or false on error
 */
bool P2Atom::append(Type type, p2_QUAD value)
{
    const char* data = reinterpret_cast<const char *>(&value);
    switch (type) {
    case Invalid:
        break;
    case Bool:
        return append(value ? false : true);
    case Byte:
        m_data += QByteArray::fromRawData(data, 1);
        break;
    case Word:
        m_data += QByteArray::fromRawData(data, 2);
        break;
    case PC:
        m_data += QByteArray::fromRawData(data, 4);
        break;
    case Long:
        m_data += QByteArray::fromRawData(data, 4);
        break;
    case Quad:
        m_data += QByteArray::fromRawData(data, 8);
        break;
    case String:
        m_data += *data;
        for (int i = 1; i < 8 && value != 0; i++, value >>= 8)
            m_data += data[i];
        return true;
    }
    return false;
}

bool P2Atom::append(int nbits, p2_QUAD value)
{
    if (nbits <= 0)
        return false;
    if (nbits <= 8)
        return append(Byte, value);
    if (nbits <= 16)
        return append(Word, value);
    if (nbits <= 32)
        return append(Long, value);
    if (nbits <= 64)
        return append(Quad, value);
    return false;
}

/**
 * @brief Append a value using the current m_type
 * @param value value to append
 * @return true on success, or false on error
 */
bool P2Atom::append(p2_QUAD value)
{
    return append(m_type, value);
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
bool P2Atom::append(const QByteArray& data)
{
    m_data += data;
    return true;
}

/**
 * @brief Set the m_data to a new value
 * @param type Type of the data to set
 * @param value with lower bits depending on type
 * @return true on success, or false on error
 */
bool P2Atom::set(Type type, p2_QUAD value)
{
    const char* data = reinterpret_cast<const char *>(&value);
    switch (type) {
    case Invalid:
        return false;
    case Bool:
        m_data.fill(*data & 1, 1);
        break;
    case Byte:
        m_data = QByteArray::fromRawData(data, 1);
        break;
    case Word:
        m_data = QByteArray::fromRawData(data, 2);
        break;
    case PC:
        m_data = QByteArray::fromRawData(data, 4);
        break;
    case Long:
        m_data = QByteArray::fromRawData(data, 4);
        break;
    case Quad:
        m_data = QByteArray::fromRawData(data, 8);
        break;
    case String:
        m_data.fill(*data, 1);
        value >>= 8;
        for (int i = 1; i < 8 && value != 0; i++, value >>= 8)
            m_data += data[i];
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
    return append(nbits, value);
}

/**
 * @brief Set the data to a new value keeping the %m_type
 * @param value new value
 * @return true on success, or false on error
 */
bool P2Atom::set(p2_QUAD value)
{
    m_data.clear();
    return append(m_type, value);
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
        set(!to_bool());
        break;
    case Byte:
        set(~to_byte());
        break;
    case Word:
        set(~to_word());
        break;
    case PC:
        set(~to_long());
        break;
    case Long:
        set(~to_long());
        break;
    case Quad:
        set(~to_quad());
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

    uchar* data = reinterpret_cast<uchar *>(m_data.data());
    uint cy = 0;

    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set(!to_bool());
        break;
    case Byte:
        set(~to_byte() + 1);
        break;
    case Word:
        set(~to_word() + 1);
        break;
    case PC:
        set(~to_long() + 1);
        break;
    case Long:
        set(~to_long() + 1);
        break;
    case Quad:
        set(~to_quad() + 1);
        break;
    case String:
        // 1's complement
        for (int i = 0; i < m_data.size(); i++)
            data[i] ^= 0xff;
        // increment
        for (int i = 0; i < m_data.size(); i++) {
            const uint byte = data[i] + cy;
            cy = static_cast<uint>(byte >> 8);
            data[i] = static_cast<uchar>(byte);
        }
        if (cy)
            m_data.append(static_cast<char>(cy));
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

    uchar* data = reinterpret_cast<uchar *>(m_data.data());
    bool zero = true;

    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set(!to_bool());
        break;
    case Byte:
        set(to_byte() ? 0 : 1);
        break;
    case Word:
        set(to_word() ? 0 : 1);
        break;
    case PC:
        set(to_long() ? 1 : 0);
        break;
    case Long:
        set(to_long() ? 1 : 0);
        break;
    case Quad:
        set(to_quad() ? 0 : 1);
        break;
    case String:
        for (int i = 0; zero && i < m_data.size(); i++)
            if (data[i])
                zero = false;
        m_data.fill(zero, 1);
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
        set(Bool, to_byte() ? 1 : 0);
        break;
    case Word:
        set(Bool, to_word() ? 1 : 0);
        break;
    case PC:
        set(Bool, to_long() ? 1 : 0);
        break;
    case Long:
        set(Bool, to_long() ? 1 : 0);
        break;
    case Quad:
        set(Bool, to_quad() ? 1 : 0);
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
void P2Atom::unary_dec(bool flag)
{
    if (!flag)
        return;

    uchar* data = reinterpret_cast<uchar *>(m_data.data());
    uint cy = 0;

    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set(!to_bool());
        break;
    case Byte:
        set(to_byte() - 1);
        break;
    case Word:
        set(to_word() - 1);
        break;
    case PC:
        set(to_long() - 1);
        break;
    case Long:
        set(to_long() - 1);
        break;
    case Quad:
        set(to_quad() - 1);
        break;
    case String:
        for (int i = 0; i < m_data.size(); i++) {
            const uint byte = data[i] - cy;
            cy = static_cast<uint>(byte >> 8);
            data[i] = static_cast<uchar>(byte);
        }
        if (cy)
            m_data.append(static_cast<char>(cy));
        break;
    }
}

/**
 * @brief Set atom to its pre/post increment value
 * @param flag do the increment if true, leave unchanged otherwise
 */
void P2Atom::unary_inc(bool flag)
{
    if (!flag)
        return;

    uchar* data = reinterpret_cast<uchar *>(m_data.data());
    uint cy = 0;

    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set(!to_bool());
        break;
    case Byte:
        set(to_byte() + 1);
        break;
    case Word:
        set(to_word() + 1);
        break;
    case PC:
        set(to_long() + 1);
        break;
    case Long:
        set(to_long() + 1);
        break;
    case Quad:
        set(to_quad() + 1);
        break;
    case String:
        for (int i = 0; i < m_data.size(); i++) {
            const uint byte = data[i] + cy;
            cy = static_cast<uint>(byte >> 8);
            data[i] = static_cast<uchar>(byte);
        }
        if (cy)
            m_data.append(static_cast<char>(cy));
        break;
    }
}

/**
 * @brief Set atom to its multiplication result value
 * @param val value to multiply by
 */
void P2Atom::arith_mul(p2_QUAD val)
{
    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set(to_bool() ? val ? true : false : false);
        break;
    case Byte:
        set(to_byte() * val);
        break;
    case Word:
        set(to_word() * val);
        break;
    case PC:
        set(to_long() * val);
        break;
    case Long:
        set(to_long() * val);
        break;
    case Quad:
        set(to_quad() * val);
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
void P2Atom::arith_div(p2_QUAD val)
{
    if (!val) {
        // Division by zero
        set(~0u);
        return;
    }
    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set(false);
        break;
    case Byte:
        set(to_byte() / val);
        break;
    case Word:
        set(to_word() / val);
        break;
    case PC:
    case Long:
        set(to_long() / val);
        break;
    case Quad:
        set(to_quad() / val);
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
void P2Atom::arith_mod(p2_QUAD val)
{
    if (!val) {
        // Division by zero
        set(~0u);
        return;
    }
    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set(to_bool() ? val & 1 ? true : false : false );
        break;
    case Byte:
        set(to_byte() % val);
        break;
    case Word:
        set(to_word() % val);
        break;
    case PC:
        set(to_long() % val);
        break;
    case Long:
        set(to_long() % val);
        break;
    case Quad:
        set(to_quad() % val);
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
void P2Atom::arith_add(p2_QUAD val)
{
    if (!val)
        return;

    uchar* data = reinterpret_cast<uchar *>(m_data.data());
    uint cy = 0;

    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set((to_bool() + val) & 1 ? true : false);
        break;
    case Byte:
        set(to_byte() + val);
        break;
    case Word:
        set(to_word() + val);
        break;
    case PC:
        set(to_long() + val);
        break;
    case Long:
        set(to_long() + val);
        break;
    case Quad:
        set(to_quad() + val);
        break;
    case String:
        for (int i = 0; i < m_data.size(); i++, val >>= 8) {
            const uint byte = data[i] + static_cast<uchar>(val) + cy;
            cy = static_cast<uint>(byte >> 8);
            data[i] = static_cast<uchar>(byte);
        }
        if (cy)
            m_data += static_cast<char>(cy);
    }
}

/**
 * @brief Set atom to its subtraction result value
 * @param val value to subtract
 */
void P2Atom::arith_sub(p2_QUAD val)
{
    if (!val)
        return;

    uchar* data = reinterpret_cast<uchar *>(m_data.data());
    uint cy = 0;

    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set((to_bool() - val) & 1 ? true : false);
        break;
    case Byte:
        set(to_byte() - val);
        break;
    case Word:
        set(to_word() - val);
        break;
    case PC:
        set(to_long() - val);
        break;
    case Long:
        set(to_long() - val);
        break;
    case Quad:
        set(to_quad() - val);
        break;
    case String:
        for (int i = 0; i < m_data.size(); i++, val >>= 8) {
            const uint byte = data[i] - static_cast<uchar>(val) - cy;
            cy = static_cast<uint>(byte >> 8);
            data[i] = static_cast<uchar>(byte);
        }
        if (cy)
            m_data += static_cast<char>(cy);
    }
}

/**
 * @brief Set atom to its left shifted value
 * @param bits number of bits to shift left
 */
void P2Atom::binary_shl(p2_QUAD bits)
{
    if (!bits)
        return;
    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set(false);
        break;
    case Byte:
        set(to_long() << bits);
        break;
    case Word:
        set(to_long() << bits);
        break;
    case PC:
        set(to_long() << bits);
        break;
    case Long:
        set(to_long() << bits);
        break;
    case Quad:
        set(to_quad() << bits);
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
void P2Atom::binary_shr(p2_QUAD bits)
{
    if (!bits)
        return;
    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set(false);
        break;
    case Byte:
        set(to_byte() >> bits);
        break;
    case Word:
        set(to_word() >> bits);
        break;
    case PC:
        set(to_long() >> bits);
        break;
    case Long:
        set(to_long() >> bits);
        break;
    case Quad:
        set(to_quad() >> bits);
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
void P2Atom::binary_and(p2_QUAD mask)
{
    switch (m_type) {
    case Invalid:
        break;
    case Bool:
        set((to_bool() & mask) & 1 ? true : false);
        break;
    case Byte:
        set(to_byte() & mask);
        break;
    case Word:
        set(to_word() & mask);
        break;
    case PC:
        set(to_long() & mask);
        break;
    case Long:
        set(to_long() & mask);
        break;
    case Quad:
        set(to_quad() & mask);
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
void P2Atom::binary_xor(p2_QUAD mask)
{
    switch (m_type) {
    case Invalid: break;
    case Bool:
        set((to_bool() ^ mask) & 1 ? true : false);
        break;
    case Byte:
        set(to_byte() ^ mask);
        break;
    case Word:
        set(to_word() ^ mask);
        break;
    case PC:
        set(to_long() ^ mask);
        break;
    case Long:
        set(to_long() ^ mask);
        break;
    case Quad:
        set(to_quad() ^ mask);
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
void P2Atom::binary_or(p2_QUAD mask)
{
    switch (m_type) {
    case Invalid: break;
    case Bool:
        set((to_bool() | mask) & 1 ? true : false);
        break;
    case Byte:
        set(to_byte() | mask);
        break;
    case Word:
        set(to_word() | mask);
        break;
    case PC:
        set(to_long() | mask);
        break;
    case Long:
        set(to_long() | mask);
        break;
    case Quad:
        set(to_quad() | mask);
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
        set(P2Util::reverse(to_byte()));
        break;
    case Word:
        set(P2Util::reverse(to_word()));
        break;
    case PC:
        set(P2Util::reverse(to_long()));
        break;
    case Long:
        set(P2Util::reverse(to_long()));
        break;
    case Quad:
        set(P2Util::reverse(to_quad()));
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
    if (Invalid == m_type) {
        if (ok)
            *ok = false;
        return false;
    }
    if (ok)
        *ok = true;
    return m_data.count('\0') != m_data.size();
}

/**
 * @brief Return data as a single byte
 * @param ok optional pointer to a bool set to true if data is available
 * @return One p2_BYTE
 */
p2_BYTE P2Atom::to_byte(bool* ok) const
{
    return static_cast<p2_BYTE>(value(ok));
}

/**
 * @brief Return data as a single word
 * @param ok optional pointer to a bool set to true if data is available
 * @return One p2_WORD
 */
p2_WORD P2Atom::to_word(bool* ok) const
{
    return static_cast<p2_WORD>(value(ok));
}

/**
 * @brief Return data as a single long
 * @param ok optional pointer to a bool set to true if data is available
 * @return One p2_LONG
 */
p2_LONG P2Atom::to_long(bool* ok) const
{
    return static_cast<p2_LONG>(value(ok));
}

/**
 * @brief Return data as a single quad
 * @param ok optional pointer to a bool set to true if data is available
 * @return One p2_LONG
 */
p2_QUAD P2Atom::to_quad(bool* ok) const
{
    return static_cast<p2_QUAD>(value(ok));
}

/**
 * @brief Return data as a QString
 * @param ok optional pointer to a bool set to true if data is available
 * @return QString with the data
 */
QString P2Atom::to_string(bool* ok) const
{
    QString data = QString::fromUtf8(m_data);
    if (ok)
        *ok = !data.isEmpty();
    return data;
}

QByteArray P2Atom::to_array() const
{
    return m_data;
}

/**
 * @brief Return data as a vector of bytes
 * @return p2_BYTEs of all data
 */
p2_BYTES P2Atom::to_bytes() const
{
    const int bytes = m_data.size();
    p2_BYTES result(bytes, 0);
    if (bytes > 0)
        memcpy(result.data(), m_data.constData(), static_cast<size_t>(bytes));
    return result;
}

/**
 * @brief Return data as a vector of words
 * @return p2_WORDs of all data
 */
p2_WORDS P2Atom::to_words() const
{
    const int bytes = m_data.size();
    p2_WORDS result((bytes + 1)  / 2, 0);
    if (bytes > 0)
        memcpy(result.data(), m_data.constData(), static_cast<size_t>(bytes));
    return result;
}

/**
 * @brief Return data as a vector of longs
 * @return p2_LONGs of all data
 */
p2_LONGS P2Atom::to_longs() const
{
    const int bytes = m_data.size();
    p2_LONGS result((bytes + 3) / 4);
    if (bytes > 0)
        memcpy(result.data(), m_data.constData(), static_cast<size_t>(bytes));
    return result;
}

P2Atom& P2Atom::operator = (const P2Atom& other)
{
    m_type = other.m_type;
    m_data = other.m_data;
    return *this;
}

bool P2Atom::operator == (const P2Atom& other)
{
    return m_data == other.m_data;
}

bool P2Atom::operator != (const P2Atom& other)
{
    return m_data != other.m_data;
}

bool P2Atom::operator < (const P2Atom& other)
{
    return m_data < other.m_data;
}

bool P2Atom::operator <= (const P2Atom& other)
{
    return m_data <= other.m_data;
}

bool P2Atom::operator > (const P2Atom& other)
{
    return m_data > other.m_data;
}

bool P2Atom::operator >= (const P2Atom& other)
{
    return m_data >= other.m_data;
}

P2Atom& P2Atom::operator ~ ()
{
    complement1(true);
    return *this;
}

P2Atom& P2Atom::operator - () {
    complement2(true);
    return *this;
}

P2Atom& P2Atom::operator ! () {
    logical_not(true);
    return *this;
}

P2Atom& P2Atom::operator ++ () {
    unary_inc(true);
    return *this;
}

P2Atom& P2Atom::operator -- () {
    unary_dec(true);
    return *this;
}

P2Atom& P2Atom::operator += (const P2Atom& other) {
    arith_add(other.to_quad());
    return *this;
}

P2Atom& P2Atom::operator -= (const P2Atom& other) {
    arith_sub(other.to_quad());
    return *this;
}

P2Atom& P2Atom::operator *= (const P2Atom& other) {
    arith_mul(other.to_quad());
    return *this;
}

P2Atom& P2Atom::operator /= (const P2Atom& other) {
    arith_div(other.to_quad());
    return *this;
}

P2Atom& P2Atom::operator %= (const P2Atom& other) {
    arith_div(other.to_quad());
    return *this;
}

P2Atom& P2Atom::operator <<= (const P2Atom& other) {
    binary_shl(other.to_quad());
    return *this;
}

P2Atom& P2Atom::operator >>= (const P2Atom& other) {
    binary_shr(other.to_quad());
    return *this;
}

P2Atom& P2Atom::operator &= (const P2Atom& other) {
    binary_and(other.to_quad());
    return *this;
}

P2Atom& P2Atom::operator ^= (const P2Atom& other) {
    binary_xor(other.to_quad());
    return *this;
}

P2Atom& P2Atom::operator |= (const P2Atom& other) {
    binary_or(other.to_quad());
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
