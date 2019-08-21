#include "p2atom.h"

P2Atom::P2Atom() : m_data()
{
}

/**
 * @brief Clear the atom
 */
void P2Atom::clear()
{
    m_data.clear();
}

bool P2Atom::isValid() const
{
    return !m_data.isEmpty();
}

bool P2Atom::isEmpty() const
{
    return m_data.isEmpty();
}

bool P2Atom::append(int nbits, P2QUAD value)
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

bool P2Atom::set(int nbits, P2QUAD value)
{
    clear();
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
 * @brief Return data as a single byte
 * @param ok optional pointer to a bool set to true if data is available
 * @return One P2BYTE
 */
P2BYTE P2Atom::toBYTE(bool* ok) const
{
    return value<P2BYTE>(ok);
}

/**
 * @brief Return data as a single word
 * @param ok optional pointer to a bool set to true if data is available
 * @return One P2WORD
 */
P2WORD P2Atom::toWORD(bool* ok) const
{
    return value<P2WORD>(ok);
}

/**
 * @brief Return data as a single long
 * @param ok optional pointer to a bool set to true if data is available
 * @return One P2LONG
 */
P2LONG P2Atom::toLONG(bool* ok) const
{
    return value<P2LONG>(ok);
}

/**
 * @brief Return data as a single quad
 * @param ok optional pointer to a bool set to true if data is available
 * @return One P2LONG
 */
P2QUAD P2Atom::toQUAD(bool* ok) const
{
    return value<P2QUAD>(ok);
}

/**
 * @brief Return data as a vector of bytes
 * @return QVector<P2BYTE> of all data
 */
QVector<P2BYTE> P2Atom::BYTES() const
{
    const int bytes = m_data.size();
    QVector<P2BYTE> result(bytes);
    for (int i = 0; i < bytes; i++)
        result[i] = at<P2BYTE>(i*8);
    return result;
}

/**
 * @brief Return data as a vector of words
 * @return QVector<P2WORD> of all data
 */
QVector<P2WORD> P2Atom::WORDS() const
{
    const int bytes = m_data.size();
    QVector<P2WORD> result(bytes/2);
    for (int i = 0; i < bytes; i += 2)
        result[i] = at<P2WORD>(i*16);
    return result;
}

/**
 * @brief Return data as a vector of longs
 * @return QVector<P2LONG> of all data
 */
QVector<P2LONG> P2Atom::LONGS() const
{
    const int bytes = m_data.size();
    QVector<P2LONG> result(bytes/4);
    for (int i = 0; i < bytes; i += 4)
        result[i] = at<P2LONG>(i*32);
    return result;
}
