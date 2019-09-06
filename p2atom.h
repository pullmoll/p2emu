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
#include "p2union.h"

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

    enum Traits {
        None            = 0,        //!< no special trait
        Immediate       = (1 << 0), //!< expression started with '#'
        Augmented       = (1 << 1), //!< expression started with '##'
        Relative        = (1 << 2), //!< expression started with '@'
        Absolute        = (1 << 3), //!< expression contained a '\'
        AddressHub      = (1 << 4), //!< expression started with '#@'
        RelativeHub     = (1 << 5), //!< expression started with '@@@'
    };

    explicit P2Atom(p2_union_e type = ut_Invalid);
    P2Atom(const P2Atom& other);
    P2Atom(bool get);
    P2Atom(p2_BYTE get);
    P2Atom(p2_WORD get);
    P2Atom(p2_LONG get);
    P2Atom(p2_QUAD get);
    P2Atom(p2_REAL get);

    void clear(p2_union_e type = ut_Invalid);
    bool isNull() const;
    bool isEmpty() const;
    bool isZero() const;
    bool isValid() const;
    Traits trait() const;

    int size() const;
    p2_LONG usize() const;
    int count() const;

    p2_union_e type() const;
    const QString type_name() const;
    void set_type(p2_union_e type);

    bool set_trait(Traits trait);
    bool add_trait(Traits trait);

    const P2Union& value() const;

    template <typename T>
    const T get() const { return m_value.get<T>(); }

    template <typename T>
    void set(const T& value) { m_value.set<T>(value); }

    template <typename T>
    void add(const T& value) { m_value.add<T>(value); }

    bool add_atom(const P2Atom& atom);
    bool add_bytes(const p2_BYTES& bytes);
    bool add_words(const p2_WORDS& words);
    bool add_longs(const p2_LONGS& longs);
    bool add_array(const QByteArray& get);

    QString str(p2_format_e fmt = fmt_hex) const;
    bool to_bool() const;
    p2_BYTE to_byte() const;
    p2_WORD to_word() const;
    p2_LONG to_long() const;
    p2_QUAD to_quad() const;
    p2_REAL to_real() const;
    QString to_string() const;
    QByteArray to_array() const;
    p2_BYTES to_bytes() const;
    p2_WORDS to_words() const;
    p2_LONGS to_longs() const;

    void make_real();
    void complement1(bool flag);
    void complement2(bool flag);
    void logical_not(bool flag);
    void make_bool(bool flag);
    void unary_dec(const p2_LONG val);
    void unary_inc(const p2_LONG val);
    void arith_mul(const P2Atom& atom);
    void arith_div(const P2Atom& atom);
    void arith_mod(const P2Atom& atom);
    void arith_add(const P2Atom& atom);
    void arith_sub(const P2Atom& atom);
    void binary_shl(const P2Atom& atom);
    void binary_shr(const P2Atom& atom);
    void binary_and(const P2Atom& atom);
    void binary_xor(const P2Atom& atom);
    void binary_or(const P2Atom& atom);
    void binary_rev();
    void reverse(const P2Atom& atom);
    void encode(const P2Atom& atom);
    void decode(const P2Atom& atom);

    P2Atom& operator=(const P2Atom& other);
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

    static QByteArray to_array(const P2Atom& atom);
    static QString to_string(const P2Atom& atom);

private:
    Traits m_trait;
    P2Union m_value;
};
