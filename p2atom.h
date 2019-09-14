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
    explicit P2Atom(p2_Union_e type = ut_Invalid);
    P2Atom(const P2Atom& other);
    P2Atom(bool _bool);
    P2Atom(const p2_BYTE _byte);
    P2Atom(const p2_WORD _word);
    P2Atom(const p2_LONG _long);
    P2Atom(const p2_QUAD _quad);
    P2Atom(const p2_REAL _real);
    P2Atom(const p2_LONG _cog, const p2_LONG _hub);

    void clear(p2_Union_e type = ut_Invalid);
    bool isNull() const;
    bool isEmpty() const;
    bool isZero() const;
    bool isValid() const;

    int size() const;
    int usize() const;
    int count() const;

    p2_Union_e type() const;
    const QString type_name() const;
    void set_type(p2_Union_e type);

    p2_Traits_e traits() const;
    bool set_traits(p2_Traits_e traits);
    bool add_trait(p2_Traits_e traits);
    bool clr_trait(p2_Traits_e traits);
    bool has_trait(const p2_Traits_e trait) const;
    bool has_trait(const int trait) const;

    const P2Union value() const;
    void set_value(const P2Union value);

    const P2Union index() const;
    void set_index(const QVariant val);
    p2_LONG index_long() const;

    void set_int(const int _int);
    void set_bool(const bool _bool);
    void set_char(const char _char);
    void set_byte(const p2_BYTE _byte);
    void set_word(const p2_WORD _word);
    void set_long(const p2_LONG _long);
    void set_addr(const p2_LONG _cog, const p2_LONG _hub);
    void set_quad(const p2_QUAD _quad);
    void set_real(const p2_REAL _real);
    void set_chars(const p2_CHARS& _chars);
    void set_bytes(const p2_BYTES& _bytes);
    void set_words(const p2_WORDS& _words);
    void set_longs(const p2_LONGS& _longs);
    void set_array(const QByteArray& _array);

    void add_int(const int _int);
    void add_bool(const bool _bool);
    void add_char(const char _char);
    void add_byte(const p2_BYTE _byte);
    void add_word(const p2_WORD _word);
    void add_long(const p2_LONG _long);
    void add_addr(const p2_LONG _cog, const p2_LONG _hub);
    void add_quad(const p2_QUAD _quad);
    void add_real(const p2_REAL _real);
    void add_chars(const p2_CHARS& _chars);
    void add_bytes(const p2_BYTES& _bytes);
    void add_words(const p2_WORDS& _words);
    void add_longs(const p2_LONGS& _longs);
    void add_array(const QByteArray& get);

    QString str(bool with_type = false, p2_FORMAT_e fmt = fmt_hex) const;
    bool get_bool() const;
    int get_int() const;
    p2_BYTE get_byte() const;
    p2_WORD get_word() const;
    p2_LONG get_long() const;
    p2_LONG get_addr(bool hub) const;
    p2_QUAD get_quad() const;
    p2_REAL get_real() const;
    QString string(bool expand = false) const;
    QByteArray array(bool expand = false) const;
    p2_BYTES get_bytes(bool expand = false) const;
    p2_WORDS get_words(bool expand = false) const;
    p2_LONGS get_longs(bool expand = false) const;

    void make_real();
    void complement1(bool flag);
    void complement2(bool flag);
    void logical_not(bool flag);
    void make_bool(bool flag);
    void unary_dec(const p2_LONG val = 1);
    void unary_inc(const p2_LONG val = 1);
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

    static QByteArray array(const P2Atom& atom);
    static QString string(const P2Atom& atom);

private:
    p2_Traits_e m_trait;        //!< Traits for this atom
    P2Union m_value;            //!< Actual value of this atom
    P2Union m_index;            //!< Optional index value of this atom
};

Q_DECLARE_METATYPE(P2Atom);
static const int mt_P2Atom = qMetaTypeId<P2Atom>();
