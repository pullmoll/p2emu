#pragma once
#include <QVariant>
#include "p2defs.h"

/**
 * @brief The P2Atom class is used to handle expression "atoms" for P2Asm.
 *
 * The data is stored in an array of bytes which is appended to from the
 * "byte", "word", or "long" definitions in the source code.
 *
 * Then values can be returned as:
 * + a single byte: p2_BYTE B(bool *ok = nullptr) const;
 * + a single word: p2_WORD W(bool *ok = nullptr) const;
 * + a single long: p2_WORD L(bool *ok = nullptr) const;
 * + a vector of all bytes: QVector<p2_BYTE> B() const;
 * + a vector of all words: QVector<p2_WORD> W() const;
 * + a vector of all longs: QVector<p2_LONG> L() const;
 */
class P2Atom
{
public:
    enum Type {
        Invalid, Byte, Word, Long, Quad
    };

    P2Atom();
    P2Atom(p2_BYTE value);
    P2Atom(p2_WORD value);
    P2Atom(p2_LONG value);

    void clear();
    bool isNull() const;
    bool isEmpty() const;
    bool isValid() const;
    Type type() const;
    template<typename T>
    T value(bool *ok = nullptr) const { return at<T>(0, ok); }

    bool append(int nbits, p2_QUAD value);
    bool set(int nbits, p2_QUAD value);

    p2_BYTE toBYTE(bool *ok = nullptr) const;
    p2_WORD toWORD(bool *ok = nullptr) const;
    p2_LONG toLONG(bool *ok = nullptr) const;
    p2_QUAD toQUAD(bool *ok = nullptr) const;

    p2_BYTEs toBYTES() const;
    p2_WORDs toWORDS() const;
    p2_LONGs toLONGS() const;

    template <typename T>
    T operator= (T newval)
    {
        if (set(sizeof(T), newval))
            return newval;
        return value<T>();
    }

    bool operator== (const P2Atom& other)
    {
        return m_data == other.m_data;
    }

private:

    /**
     * @brief Append a number of 8, 16, or 32 bits to the data
     * @return true on success, or false for wrong number of bytes
     */
    template <int bits>
    bool append(p2_QUAD value)
    {
        switch (bits) {
        case  8: // append a byte
            m_data.append(static_cast<char>(value));
            break;
        case 16: // append a word
            m_data.append(static_cast<char>(value>>0));
            m_data.append(static_cast<char>(value>>8));
            break;
        case 32: // append a long
            m_data.append(static_cast<char>(value>> 0));
            m_data.append(static_cast<char>(value>> 8));
            m_data.append(static_cast<char>(value>>16));
            m_data.append(static_cast<char>(value>>24));
            break;
        case 64: // append a long
            m_data.append(static_cast<char>(value>> 0));
            m_data.append(static_cast<char>(value>> 8));
            m_data.append(static_cast<char>(value>>16));
            m_data.append(static_cast<char>(value>>24));
            m_data.append(static_cast<char>(value>>32));
            m_data.append(static_cast<char>(value>>40));
            m_data.append(static_cast<char>(value>>48));
            m_data.append(static_cast<char>(value>>56));
            break;
        }
        switch (m_type) {
        case Quad:
            break;
        case Long:
            if (m_data.size() > 4)
                m_type = Quad;
            break;
        case Word:
            if (m_data.size() > 2)
                m_type = Long;
            break;
        case Byte:
            if (m_data.size() > 0)
                m_type = Word;
            break;
        default:
            if (m_data.size() > 0)
                m_type = Byte;
        }
        return m_type != Invalid;
    }

    template<typename T>
    T at(const int offs, bool *ok = nullptr) const
    {
        const uchar* data = reinterpret_cast<const uchar *>(m_data.constData());
        const int need = sizeof(T) * 8;
        const int bits = m_data.size() * 8;
        p2_QUAD result = 0;
        if (ok)
            *ok = need <= bits && offs < bits;
        for (int i = offs; i < bits && i < need; i += 8)
            result |= static_cast<p2_QUAD>(data[i/8]) << (i - offs);
        return static_cast<T>(result);
    }

    Type m_type;
    QByteArray m_data;
};
