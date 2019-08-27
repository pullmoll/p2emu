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
    : m_type(Byte)
    , m_data()
{
    set(Byte, value & 1);
}

P2Atom::P2Atom(p2_BYTE value)
    : m_type(Byte)
    , m_data()
{
    set(Byte, value);
}

P2Atom::P2Atom(p2_WORD value)
    : m_type(Word)
    , m_data()
{
    set(Word, value);
}

P2Atom::P2Atom(p2_LONG value)
    : m_type(Long)
    , m_data()
{
    set(Long, value);
}

P2Atom::P2Atom(p2_QUAD value)
    : m_type(Quad)
    , m_data()
{
    set(Quad, value);
}

/**
 * @brief Clear the atom
 */
void P2Atom::clear()
{
    m_data.clear();
    m_type = Invalid;
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
    case P2Atom::Byte:
        return QStringLiteral("Byte");
    case P2Atom::Word:
        return QStringLiteral("Word");
    case P2Atom::Long:
        return QStringLiteral("Long");
    case P2Atom::Quad:
        return QStringLiteral("Quad");
    case P2Atom::String:
        return QStringLiteral("String");
    }
    return QStringLiteral("<invalid>");
}

/**
 * @brief Set the type of the atom
 * @brie type one of the %Type enumeration values
 */
void P2Atom::setType(Type type)
{
    m_type = type;
    switch (type) {
    case Invalid:
        m_data.clear();
        break;
    case Byte:
        m_data.truncate(1);
        break;
    case Word:
        m_data.truncate(2);
        break;
    case Long:
        m_data.truncate(4);
        break;
    case Quad:
        m_data.truncate(8);
        break;
    case String:
        break;
    }
}

p2_QUAD P2Atom::value(bool* ok) const
{
    p2_QUAD result = 0;
    if (ok)
        *ok = isValid();
    if (m_data.size() > 7)
        return *reinterpret_cast<const p2_QUAD*>(m_data.constData());
    if (m_data.size() > 3)
        return *reinterpret_cast<const p2_LONG*>(m_data.constData());
    if (m_data.size() > 1)
        return *reinterpret_cast<const p2_WORD*>(m_data.constData());
    if (m_data.size() > 0)
        return *reinterpret_cast<const p2_BYTE*>(m_data.constData());

    const uchar* data = reinterpret_cast<const uchar *>(m_data.constData());
    const int bits = 8 * m_data.size();
    const int need = 8 * sizeof(p2_QUAD);
    if (ok)
        *ok = need <= bits;
    for (int i = 0; i < bits && i < need; i += 8)
        result |= static_cast<p2_QUAD>(data[i/8]) << i;
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
    switch (type) {
    case Invalid:
        break;
    case Byte:
        return append<Byte>(value);
    case Word:
        return append<Word>(value);
    case Long:
        return append<Long>(value);
    case Quad:
        return append<Quad>(value);
    case String:
        return append<String>(value);
    }
    return false;
}

bool P2Atom::append(int nbits, p2_QUAD value)
{
    if (nbits <= 0)
        return false;
    if (nbits <= 8)
        return append<Byte>(value);
    if (nbits <= 16)
        return append<Word>(value);
    if (nbits <= 32)
        return append<Long>(value);
    if (nbits <= 64)
        return append<Quad>(value);
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
    m_data.append(atom.m_data);
    return true;
}

/**
 * @brief Append contents of a QByteArray to this atom
 * @param data QByteArray to append
 * @return true on success, or false on error
 */
bool P2Atom::append(const QByteArray& data)
{
    m_data.append(data);
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
    m_data.clear();
    m_type = type;
    return append(m_type, value);
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
    return append(value);
}

/**
 * @brief Set atom to its one's complement (~) if flag is true
 * @param flag one's complement if true, leave unchanged otherwise
 */
void P2Atom::complement1(bool flag)
{
    if (!flag)
        return;
    switch (m_type) {
    case Invalid: return;
    case Byte:
        set(Byte, ~to_byte());
        break;
    case Word:
        set(Word, ~to_word());
        break;
    case Long:
        set(Long, ~to_long());
        break;
    case Quad:
        set(Quad, ~to_quad());
        break;
    case String:
        Q_ASSERT(m_type);
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
        return;
    case Byte:
        set(Byte, ~to_byte() + 1);
        break;
    case Word:
        set(Word, ~to_word() + 1);
        break;
    case Long:
        set(Long, ~to_long() + 1);
        break;
    case Quad:
        set(Quad, ~to_quad() + 1);
        break;
    case String:
        Q_ASSERT(m_type);
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
        return;
    case Byte:
        set(Byte, to_byte() ? 0 : 1);
        break;
    case Word:
        set(Word, to_word() ? 0 : 1);
        break;
    case Long:
        set(Long, to_long() ? 1 : 0);
        break;
    case Quad:
        set(Quad, to_quad() ? 0 : 1);
        break;
    case String:
        Q_ASSERT(m_type);
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
        return;
    case Byte:
        set(Byte, to_byte() ? 1 : 0);
        break;
    case Word:
        set(Word, to_word() ? 1 : 0);
        break;
    case Long:
        set(Long, to_long() ? 1 : 0);
        break;
    case Quad:
        set(Quad, to_quad() ? 1 : 0);
        break;
    case String:
        Q_ASSERT(m_type);
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
    switch (m_type) {
    case Invalid:
        return;
    case Byte:
        set(Byte, to_byte() - 1);
        break;
    case Word:
        set(Word, to_word() - 1);
        break;
    case Long:
        set(Long, to_long() - 1);
        break;
    case Quad:
        set(Quad, to_quad() - 1);
        break;
    case String:
        Q_ASSERT(m_type);
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
    switch (m_type) {
    case Invalid:
        return;
    case Byte:
        set(Byte, to_byte() + 1);
        break;
    case Word:
        set(Word, to_word() + 1);
        break;
    case Long:
        set(Long, to_long() + 1);
        break;
    case Quad:
        set(Quad, to_quad() + 1);
        break;
    case String:
        Q_ASSERT(m_type);
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
        return;
    case Byte:
        set(Byte, to_byte() * val);
        break;
    case Word:
        set(Word, to_word() * val);
        break;
    case Long:
        set(Long, to_long() * val);
        break;
    case Quad:
        set(Quad, to_quad() * val);
        break;
    case String:
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
        return;
    case Byte:
        set(Byte, to_byte() / val);
        break;
    case Word:
        set(Word, to_word() / val);
        break;
    case Long:
        set(Long, to_long() / val);
        break;
    case Quad:
        set(Quad, to_quad() / val);
        break;
    case String:
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
        return;
    case Byte:
        set(Byte, to_byte() % val);
        break;
    case Word:
        set(Word, to_word() % val);
        break;
    case Long:
        set(Long, to_long() % val);
        break;
    case Quad:
        set(Quad, to_quad() % val);
        break;
    case String:
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
    switch (m_type) {
    case Invalid:
        return;
    case Byte:
        set(Byte, to_byte() + val);
        break;
    case Word:
        set(Word, to_word() + val);
        break;
    case Long:
        set(Long, to_long() + val);
        break;
    case Quad:
        set(Quad, to_quad() + val);
        break;
    case String:
        Q_ASSERT(m_type);
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
    switch (m_type) {
    case Invalid:
        return;
    case Byte:
        set(Byte, to_byte() - val);
        break;
    case Word:
        set(Word, to_word() - val);
        break;
    case Long:
        set(Long, to_long() - val);
        break;
    case Quad:
        set(Quad, to_quad() - val);
        break;
    case String:
        Q_ASSERT(m_type);
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
        return;
    case Byte:
        set(to_long() << bits);
        break;
    case Word:
        set(to_long() << bits);
        break;
    case Long:
        set(to_long() << bits);
        break;
    case Quad:
        set(to_quad() << bits);
        break;
    case String:
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
        return;
    case Byte:
        set(to_byte() >> bits);
        break;
    case Word:
        set(to_word() >> bits);
        break;
    case Long:
        set(to_long() >> bits);
        break;
    case Quad:
        set(to_quad() >> bits);
        break;
    case String:
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
        return;
    case Byte:
        set(to_byte() & mask);
        break;
    case Word:
        set(to_word() & mask);
        break;
    case Long:
        set(to_long() & mask);
        break;
    case Quad:
        set(to_quad() & mask);
        break;
    case String:
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
    case Invalid:
        return;
    case Byte:
        set(to_byte() ^ mask);
        break;
    case Word:
        set(to_word() ^ mask);
        break;
    case Long:
        set(to_long() ^ mask);
        break;
    case Quad:
        set(to_quad() ^ mask);
        break;
    case String:
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
    case Invalid:
        return;
    case Byte:
        set(to_byte() | mask);
        break;
    case Word:
        set(to_word() | mask);
        break;
    case Long:
        set(to_long() | mask);
        break;
    case Quad:
        set(to_quad() | mask);
        break;
    case String:
        Q_ASSERT(m_type);
    }
}

void P2Atom::binary_rev()
{
    switch (m_type) {
    case Invalid:
        return;
    case Byte:
        set(P2Util::reverse(to_byte()));
        break;
    case Word:
        set(P2Util::reverse(to_word()));
        break;
    case Long:
        set(P2Util::reverse(to_long()));
        break;
    case Quad:
        set(P2Util::reverse(to_quad()));
        break;
    case String:
        Q_ASSERT(m_type);
    }
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

bool P2Atom::operator==(const P2Atom& other)
{
    return m_data == other.m_data;
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
    arith_add(other.to_quad());
    return *this;
}

P2Atom& P2Atom::operator-=(const P2Atom& other) {
    arith_sub(other.to_quad());
    return *this;
}

P2Atom& P2Atom::operator*=(const P2Atom& other) {
    arith_mul(other.to_quad());
    return *this;
}

P2Atom& P2Atom::operator/=(const P2Atom& other) {
    arith_div(other.to_quad());
    return *this;
}

P2Atom& P2Atom::operator%=(const P2Atom& other) {
    arith_div(other.to_quad());
    return *this;
}

P2Atom& P2Atom::operator<<=(const P2Atom& other) {
    binary_shl(other.to_quad());
    return *this;
}

P2Atom& P2Atom::operator>>=(const P2Atom& other) {
    binary_shr(other.to_quad());
    return *this;
}

P2Atom& P2Atom::operator&=(const P2Atom& other) {
    binary_and(other.to_quad());
    return *this;
}

P2Atom& P2Atom::operator^=(const P2Atom& other) {
    binary_xor(other.to_quad());
    return *this;
}

P2Atom& P2Atom::operator|=(const P2Atom& other) {
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
