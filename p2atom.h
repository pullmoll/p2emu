#pragma once
#include <QVariant>
#include "p2defs.h"

/**
 * @brief The P2Atom class is used to handle expression "atoms" for P2Asm.
 *
 * The data is stored in an array of bytes which is appended to from the
 * "BYTE", "WORD", or "LONG" definitions in the source code.
 */
class P2Atom
{
public:
    enum Type {
        Invalid, Byte, Word, Long, Quad, String
    };

    explicit P2Atom();
    P2Atom(bool value);
    P2Atom(p2_BYTE value);
    P2Atom(p2_WORD value);
    P2Atom(p2_LONG value);
    P2Atom(p2_QUAD value);
    P2Atom(const P2Atom& other);

    void clear();
    bool isNull() const;
    bool isEmpty() const;
    bool isValid() const;
    Type type() const;
    const QString type_name() const;
    void setType(Type type);

    p2_QUAD value(bool *ok = nullptr) const;

    bool append(Type type, p2_QUAD value);
    bool append(int nbits, p2_QUAD value);
    bool append(p2_QUAD value);
    bool append(const P2Atom& atom);
    bool append(const QByteArray& data);

    bool set(Type type, p2_QUAD value);
    bool set(int nbits, p2_QUAD value);
    bool set(p2_QUAD value);

    void complement1(bool flag);
    void complement2(bool flag);
    void logical_not(bool flag);
    void make_bool(bool flag);
    void unary_dec(bool flag);
    void unary_inc(bool flag);
    void arith_mul(p2_QUAD val);
    void arith_div(p2_QUAD val);
    void arith_mod(p2_QUAD val);
    void arith_add(p2_QUAD val);
    void arith_sub(p2_QUAD val);
    void binary_shl(p2_QUAD bits);
    void binary_shr(p2_QUAD bits);
    void binary_and(p2_QUAD mask);
    void binary_xor(p2_QUAD mask);
    void binary_or(p2_QUAD mask);
    void binary_rev();

    p2_BYTE to_byte(bool *ok = nullptr) const;
    p2_WORD to_word(bool *ok = nullptr) const;
    p2_LONG to_long(bool *ok = nullptr) const;
    p2_QUAD to_quad(bool *ok = nullptr) const;
    QString to_string(bool *ok = nullptr) const;

    p2_BYTES to_bytes() const;
    p2_WORDS to_words() const;
    p2_LONGS to_longs() const;

    template <typename T>
    T operator= (T newval)
    {
        if (typeid(T) == typeid(p2_BYTE)) {
            if (set(Byte, newval))
                return newval;
        } else if (typeid(T) == typeid(p2_WORD)) {
            if (set(Word, newval))
                return newval;
        } else if (typeid(T) == typeid(p2_LONG)) {
            if (set(Long, newval))
                return newval;
        } else if (typeid(T) == typeid(p2_QUAD)) {
            if (set(Quad, newval))
                return newval;
        } else if (set(sizeof(T), newval)) {
            return newval;
        }
        return static_cast<T>(value());
    }

    P2Atom& operator= (const P2Atom& other)
    {
        m_type = other.m_type;
        m_data = other.m_data;
        return *this;
    }


    bool operator== (const P2Atom& other);
    P2Atom& operator~();
    P2Atom& operator-();
    P2Atom& operator!();
    P2Atom& operator++();
    P2Atom& operator--();
    P2Atom& operator+=(const P2Atom& other);
    P2Atom& operator-=(const P2Atom& other);
    P2Atom& operator*=(const P2Atom& other);
    P2Atom& operator/=(const P2Atom& other);
    P2Atom& operator%=(const P2Atom& other);
    P2Atom& operator<<=(const P2Atom& other);
    P2Atom& operator>>=(const P2Atom& other);
    P2Atom& operator&=(const P2Atom& other);
    P2Atom& operator^=(const P2Atom& other);
    P2Atom& operator|=(const P2Atom& other);

private:

    /**
     * @brief Append a number of 8, 16, 32, or 64 bits to the data
     * @return true on success, or false for wrong number of bits
     */
    template <Type type, typename T>
    bool append(T value)
    {
        switch (type) {
        case Byte:
            m_data.append(static_cast<char>(value));
            break;
        case Word:
            m_data.append(static_cast<char>(value>>0));
            m_data.append(static_cast<char>(value>>8));
            break;
        case Long:
            m_data.append(static_cast<char>(value>> 0));
            m_data.append(static_cast<char>(value>> 8));
            m_data.append(static_cast<char>(value>>16));
            m_data.append(static_cast<char>(value>>24));
            break;
        case Quad:
            m_data.append(static_cast<char>(value>> 0));
            m_data.append(static_cast<char>(value>> 8));
            m_data.append(static_cast<char>(value>>16));
            m_data.append(static_cast<char>(value>>24));
            m_data.append(static_cast<char>(value>>32));
            m_data.append(static_cast<char>(value>>40));
            m_data.append(static_cast<char>(value>>48));
            m_data.append(static_cast<char>(value>>56));
            break;
        case String:
            // always append one character
            m_data.append(static_cast<char>(value));
            while (value) {
                // append non-NUL characters
                value >>= 8;
                m_data.append(static_cast<char>(value));
            }
            // always set type to String now
            m_type = String;
            break;
        default:
            return false;
        }
        return m_type != Invalid;
    }

    Type m_type;
    QByteArray m_data;
};
