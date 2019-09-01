/****************************************************************************
 *
 * Propeller2 assembler atomic data element class
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
#pragma once
#include <QVariant>
#include "p2defs.h"

/**
 * @brief The P2Atom class is used to handle expression "atoms" for the Propeller2 assembler.
 *
 * The data is stored in an array of bytes (QByteArray) which is appended to from
 * the basic BYTE, WORD, LONG, or FILE definitions in the source code.
 * Also the virtual type QUAD (64 bit unsigned) is accessible for intermediate results.
 *
 * The data can grow arbitrary large e.g. for strings or FILE contents.
 *
 * There is a set of operations which can be performed on the atom:
 *
 * <ul>
 *  <li>~: one's complement</li>
 *  <li>-: two's complement (i.e. unary minus)</li>
 *  <li>!: logical not</li>
 *  <li>make_bool</li>
 *  <li>--: unary decrement</li>
 *  <li>++: unary increment</li>
 *  <li>* atom: multiplication</li>
 *  <li>/ atom: division</li>
 *  <li>% atom: modulo</li>
 *  <li>+ atom: addition</li>
 *  <li>- atom: subtraction</li>
 *  <li>&lt</li>&lt</li> atom: binary shift left</li>
 *  <li>&gt</li>&gt</li> atom: binary shift right</li>
 *  <li>&amp</li> atom: binary AND</li>
 *  <li>^ atom: binary XOR</li>
 *  <li>| atom: binary OR</li>
 *  <li>binary_rev</li>
 * </ul>
 */
class P2Atom
{
public:
    enum Type {
        Invalid,
        Bool,
        Byte,
        Word,
        PC,
        Long,
        Quad,
        Real,
        String
    };

    static constexpr int sz_BYTE = sizeof(p2_BYTE);
    static constexpr int sz_WORD = sizeof(p2_WORD);
    static constexpr int sz_LONG = sizeof(p2_LONG);
    static constexpr int sz_QUAD = sizeof(p2_QUAD);
    static constexpr int sz_REAL = sizeof(p2_REAL);

    explicit P2Atom(Type type = Invalid);
    P2Atom(const P2Atom& other);
    P2Atom(bool value, Type type = Bool);
    P2Atom(p2_BYTE value, Type type = Byte);
    P2Atom(p2_WORD value, Type type = Word);
    P2Atom(p2_LONG value, Type type = Long);
    P2Atom(p2_QUAD value, Type type = Quad);
    P2Atom(p2_REAL value, Type type = Real);

    void clear(Type type = Invalid);
    bool isNull() const;
    bool isEmpty() const;
    bool isZero() const;
    bool isValid() const;
    int size() const;
    p2_LONG usize() const;
    int count() const;
    Type type() const;
    const QString type_name() const;
    void set_type(Type type);

    p2_QUAD value(bool *ok = nullptr) const;

    bool append_uint(Type type, p2_QUAD value);
    bool append_real(Type type, p2_REAL value);
    bool append_bits(int nbits, p2_QUAD value);
    bool append(p2_QUAD value);
    bool append(p2_REAL value);
    bool append(const P2Atom& atom);
    bool append(const QByteArray& value);

    bool set_uint(Type type, p2_QUAD value);
    bool set_real(Type type, p2_REAL value);
    bool set(int nbits, p2_QUAD value);
    bool set(p2_QUAD value);
    bool set(p2_REAL value);

    bool to_bool(bool *ok = nullptr) const;
    p2_BYTE to_byte(bool *ok = nullptr) const;
    p2_WORD to_word(bool *ok = nullptr) const;
    p2_LONG to_long(bool *ok = nullptr) const;
    p2_QUAD to_quad(bool *ok = nullptr) const;
    p2_REAL to_real(bool *ok = nullptr) const;
    QString to_string(bool *ok = nullptr) const;
    QByteArray to_array() const;
    p2_BYTES to_bytes() const;
    p2_WORDS to_words() const;
    p2_LONGS to_longs() const;

    void make_real();
    void complement1(bool flag);
    void complement2(bool flag);
    void logical_not(bool flag);
    void make_bool(bool flag);
    void unary_dec(const P2Atom& val);
    void unary_inc(const P2Atom& val);
    void arith_mul(const P2Atom& val);
    void arith_div(const P2Atom& val);
    void arith_mod(const P2Atom& val);
    void arith_add(const P2Atom& val);
    void arith_sub(const P2Atom& val);
    void binary_shl(const P2Atom& val);
    void binary_shr(const P2Atom& val);
    void binary_and(const P2Atom& val);
    void binary_xor(const P2Atom& val);
    void binary_or(const P2Atom& val);
    void binary_rev();
    void reverse(const P2Atom& val);

    P2Atom& operator = (const P2Atom& other);
    bool operator==(const P2Atom& other);
    bool operator!=(const P2Atom& other);
    bool operator<(const P2Atom& other);
    bool operator<=(const P2Atom& other);
    bool operator>(const P2Atom& other);
    bool operator>=(const P2Atom& other);
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

    static QString format_long_mask(const p2_LONG data, const p2_LONG mask);
    static QStringList format_data(const P2Atom& data, const p2_LONG addr);

private:
    Type m_type;
    QVector<p2_BYTE> m_data;
};
