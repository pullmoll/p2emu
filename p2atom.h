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
 * + a single byte: P2BYTE B(bool *ok = nullptr) const;
 * + a single word: P2WORD W(bool *ok = nullptr) const;
 * + a single long: P2WORD L(bool *ok = nullptr) const;
 * + a vector of all bytes: QVector<P2BYTE> B() const;
 * + a vector of all words: QVector<P2WORD> W() const;
 * + a vector of all longs: QVector<P2LONG> L() const;
 */
class P2Atom
{
public:
    P2Atom();

    void clear();
    bool isValid() const;
    bool isEmpty() const;

    bool append(int nbits, P2QUAD value);
    bool set(int nbits, P2QUAD value);

    P2BYTE toBYTE(bool *ok = nullptr) const;
    P2WORD toWORD(bool *ok = nullptr) const;
    P2LONG toLONG(bool *ok = nullptr) const;
    quint64 toQUAD(bool *ok = nullptr) const;

    QVector<P2BYTE> BYTES() const;
    QVector<P2WORD> WORDS() const;
    QVector<P2LONG> LONGS() const;

    template <typename T>
    T operator= (T newval)
    {
        if (set(sizeof(T), newval))
            return newval;
        return value<T>();
    }

private:

    /**
     * @brief Append a number of 8, 16, or 32 bits to the data
     * @return true on success, or false for wrong number of bytes
     */
    template <int bits>
    bool append(P2QUAD value)
    {
        switch (bits) {
        case  8: // append a byte
            m_data.append(static_cast<char>(value));
            return true;
        case 16: // append a word
            m_data.append(static_cast<char>(value>>0));
            m_data.append(static_cast<char>(value>>8));
            return true;
        case 32: // append a long
            m_data.append(static_cast<char>(value>> 0));
            m_data.append(static_cast<char>(value>> 8));
            m_data.append(static_cast<char>(value>>16));
            m_data.append(static_cast<char>(value>>24));
            return true;
        case 64: // append a long
            m_data.append(static_cast<char>(value>> 0));
            m_data.append(static_cast<char>(value>> 8));
            m_data.append(static_cast<char>(value>>16));
            m_data.append(static_cast<char>(value>>24));
            m_data.append(static_cast<char>(value>>32));
            m_data.append(static_cast<char>(value>>40));
            m_data.append(static_cast<char>(value>>48));
            m_data.append(static_cast<char>(value>>56));
            return true;
        }
        return false;
    }

    template<typename T>
    T value(bool *ok = nullptr) const
    {
        const uchar* data = reinterpret_cast<const uchar *>(m_data.constData());
        const int need = sizeof(T) * 8;
        const int bits = m_data.size() * 8;
        P2QUAD result = 0;
        if (ok)
            *ok = need <= bits;
        for (int i = 0; i < bits && i < need; i += 8)
            result |= static_cast<P2QUAD>(data[i/8]) << i;
        return static_cast<T>(result);
    }

    template<typename T>
    T take(bool *ok = nullptr)
    {
        const uchar* data = reinterpret_cast<const uchar *>(m_data.constData());
        const int need = sizeof(T) * 8;
        const int bits = m_data.size() * 8;
        P2QUAD result = 0;
        if (ok)
            *ok = need <= bits;
        int i = 0;
        for (i = 0; i < bits && i < need; i += 8)
            result |= static_cast<P2QUAD>(data[i/8]) << i;
        m_data.remove(0, (i+7)/8);
        return static_cast<T>(result);
    }

    template<typename T>
    T at(const int offs, bool *ok = nullptr) const
    {
        const uchar* data = reinterpret_cast<const uchar *>(m_data.constData());
        const int need = sizeof(T) * 8;
        const int bits = m_data.size() * 8;
        P2QUAD result = 0;
        if (ok)
            *ok = need <= bits && offs < bits;
        for (int i = offs; i < bits && i < need; i += 8)
            result = (result << 8) | data[i/8];
        return static_cast<T>(result);
    }

    QByteArray m_data;
};
