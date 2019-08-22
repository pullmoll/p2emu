#include "p2atom.h"

P2Atom::P2Atom()
    : m_type(Invalid)
    , m_data()
{
}

P2Atom::P2Atom(p2_BYTE value)
    : m_type(Invalid)
    , m_data()
{
    append(8, value);
}

P2Atom::P2Atom(p2_LONG value)
    : m_type(Invalid)
    , m_data()
{
    append(32, value);
}

P2Atom::P2Atom(p2_WORD value)
    : m_type(Invalid)
    , m_data()
{
    append(16, value);
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

/**
 * @brief Append a number of bytes to the data
 * @param nbits number of bits to append
 * @param value with lower %nbits valid
 * @return truen on success, or false on error
 */
bool P2Atom::append(int nbits, p2_QUAD value)
{
    if (nbits <= 0)
        return false;
    if (nbits <= 8)
        return append<8>(value);
    if (nbits <= 16)
        return append<16>(value);
    if (nbits <= 32)
        return append<32>(value);
    return append<64>(value);
}

/**
 * @brief Set the data to a new value
 * @param nbits number of bits to set
 * @param value with lower %nbits valid
 * @return true on success, or false on error
 */
bool P2Atom::set(int nbits, p2_QUAD value)
{
    clear();
    if (nbits <= 0)
        return false;
    if (nbits <= 8) {
        return append<8>(value);
    }
    if (nbits <= 16)
        return append<16>(value);
    if (nbits <= 32)
        return append<32>(value);
    return append<64>(value);
}

/**
 * @brief Return data as a single byte
 * @param ok optional pointer to a bool set to true if data is available
 * @return One p2_BYTE
 */
p2_BYTE P2Atom::toBYTE(bool* ok) const
{
    return value<p2_BYTE>(ok);
}

/**
 * @brief Return data as a single word
 * @param ok optional pointer to a bool set to true if data is available
 * @return One p2_WORD
 */
p2_WORD P2Atom::toWORD(bool* ok) const
{
    return value<p2_WORD>(ok);
}

/**
 * @brief Return data as a single long
 * @param ok optional pointer to a bool set to true if data is available
 * @return One p2_LONG
 */
p2_LONG P2Atom::toLONG(bool* ok) const
{
    return value<p2_LONG>(ok);
}

/**
 * @brief Return data as a single quad
 * @param ok optional pointer to a bool set to true if data is available
 * @return One p2_LONG
 */
p2_QUAD P2Atom::toQUAD(bool* ok) const
{
    return value<p2_QUAD>(ok);
}

/**
 * @brief Return data as a vector of bytes
 * @return p2_BYTEs of all data
 */
p2_BYTES P2Atom::toBYTES() const
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
p2_WORDS P2Atom::toWORDS() const
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
p2_LONGS P2Atom::toLONGS() const
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

P2Atom& P2Atom::operator+=(const P2Atom& other)
{
    m_data += other.m_data;
    return *this;
}
