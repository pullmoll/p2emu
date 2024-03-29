/****************************************************************************
 *
 * Propeller2 union of values and types implementation
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
#include "p2union.h"

P2Union::P2Union()
    : QVector<P2TypedValue>()
    , m_type(ut_Invalid)
{
}

P2Union::P2Union(bool b) : P2Union()
{
    set_bool(b);
}

P2Union::P2Union(char c) : P2Union()
{
    set_char(c);
}

P2Union::P2Union(int i) : P2Union()
{
    set_long(static_cast<p2_LONG>(i));
}

P2Union::P2Union(p2_BYTE b) : P2Union()
{
    set_byte(b);
}

P2Union::P2Union(p2_WORD w) : P2Union()
{
    set_word(w);
}

P2Union::P2Union(p2_LONG l) : P2Union()
{
    set_long(l);
}

P2Union::P2Union(p2_QUAD q) : P2Union()
{
    set_quad(q);
}

P2Union::P2Union(p2_REAL r) : P2Union()
{
    set_real(r);
}

P2Union::P2Union(p2_LONG _cog, p2_LONG _hub, bool hubmode)
{
    set_addr(_cog, _hub, hubmode);
}

P2Union::P2Union(const QByteArray& ba) : P2Union()
{
    set_array(ba);
}

P2Union::P2Union(p2_BYTES bv) : P2Union()
{
    set_bytes(bv);
}

P2Union::P2Union(p2_WORDS wv) : P2Union()
{
    set_words(wv);
}

P2Union::P2Union(p2_LONGS lv) : P2Union()
{
    set_longs(lv);
}

int P2Union::unit() const
{
    return unit(m_type);
}

int P2Union::usize() const
{
    int result = 0;
    QVariant list = QVariant::fromValue(toList());
    if (list.canConvert<QVariantList>()) {
        QSequentialIterable it = list.value<QSequentialIterable>();
        foreach(const QVariant& v, it) {
            P2TypedValue tv = qvariant_cast<P2TypedValue>(v);
            switch (tv.type) {
            case ut_Invalid:
                Q_ASSERT(tv.type != ut_Invalid);
                break;
            case ut_Bool:
            case ut_Byte:
                result += sz_BYTE;
                break;
            case ut_Word:
                result += sz_WORD;
                break;
            case ut_Addr:
                result += sz_QUAD;
                break;
            case ut_Long:
                result += sz_LONG;
                break;
            case ut_Quad:
                result += sz_QUAD;
                break;
            case ut_Real:
                result += sz_REAL;
                break;
            case ut_String:
                result += sz_BYTE;
                break;
            }
        }
    }
    return result;
}

p2_Union_e P2Union::type() const
{
    return m_type;
}

QString P2Union::type_name() const
{
    return type_name(m_type);
}

void P2Union::set_type(p2_Union_e type)
{
    m_type = type;
}

bool P2Union::is_zero() const
{
    // FIXME: xxx
    return true;
}

bool P2Union::hubmode() const
{
    const P2TypedValue tv = value(0);
    return tv.hubmode;
}

void P2Union::set_hubmode(bool hubmode)
{
    for (int i = 0; i < count(); i++) {
        P2TypedValue tv = value(i);
        tv.hubmode = hubmode;
        replace(i, tv);
    }
}

int P2Union::get_int() const
{
    const P2TypedValue& tv = value(0);
    int result = 0;
    switch (tv.type) {
    case ut_Invalid:
        break;
    case ut_Bool:
        result = tv.value._bool;
        break;
    case ut_Byte:
        result = tv.value._byte;
        break;
    case ut_Word:
        result = tv.value._word;
        break;
    case ut_Addr:
        result = static_cast<int>(tv.value._addr[tv.hubmode]);
        break;
    case ut_Long:
        result = static_cast<int>(tv.value._long);
        break;
    case ut_Quad:
        result = static_cast<int>(tv.value._quad);
        break;
    case ut_Real:
        result = static_cast<int>(tv.value._real);
        break;
    case ut_String:
        result = tv.value._byte;
        break;
    }
    return result;
}

bool P2Union::get_bool() const
{
    const P2TypedValue& tv = value(0);
    bool result = false;
    switch (tv.type) {
    case ut_Invalid:
        break;
    case ut_Bool:
        result = tv.value._bool;
        break;
    case ut_Byte:
        result = tv.value._byte != 0;
        break;
    case ut_Word:
        result = tv.value._word != 0;
        break;
    case ut_Addr:
        result = tv.value._addr[tv.hubmode] != 0;
        break;
    case ut_Long:
        result = tv.value._long != 0;
        break;
    case ut_Quad:
        result = tv.value._quad != 0;
        break;
    case ut_Real:
        result = !qFuzzyIsNull(tv.value._real);
        break;
    case ut_String:
        result = tv.value._byte ? true : false;
    }
    return result;
}

char P2Union::get_char() const
{
    const P2TypedValue& tv = value(0);
    char result = '\0';
    switch (tv.type) {
    case ut_Invalid:
        break;
    case ut_Bool:
        result = tv.value._bool;
        break;
    case ut_Byte:
        result = tv.value._char;
        break;
    case ut_Word:
        result = static_cast<char>(tv.value._word);
        break;
    case ut_Addr:
        result = static_cast<char>(tv.value._addr[tv.hubmode]);
        break;
    case ut_Long:
        result = static_cast<char>(tv.value._long);
        break;
    case ut_Quad:
        result = static_cast<char>(tv.value._quad);
        break;
    case ut_Real:
        result = static_cast<char>(tv.value._real);
        break;
    case ut_String:
        result = tv.value._char;
        break;
    }
    return result;
}

p2_BYTE P2Union::get_byte() const
{
    const P2TypedValue& tv = value(0);
    p2_BYTE result = 0;
    switch (tv.type) {
    case ut_Invalid:
        break;
    case ut_Bool:
        result = tv.value._bool;
        break;
    case ut_Byte:
        result = tv.value._byte;
        break;
    case ut_Word:
        result = static_cast<p2_BYTE>(tv.value._word);
        break;
    case ut_Addr:
        result = static_cast<p2_BYTE>(tv.value._addr[tv.hubmode]);
        break;
    case ut_Long:
        result = static_cast<p2_BYTE>(tv.value._long);
        break;
    case ut_Quad:
        result = static_cast<p2_BYTE>(tv.value._quad);
        break;
    case ut_Real:
        result = static_cast<p2_BYTE>(tv.value._real);
        break;
    case ut_String:
        result = tv.value._byte;
        break;
    }
    return result;
}

p2_WORD P2Union::get_word() const
{
    const P2TypedValue& tv = value(0);
    p2_WORD result = 0;
    switch (tv.type) {
    case ut_Invalid:
        break;
    case ut_Bool:
        result = tv.value._bool;
        break;
    case ut_Byte:
        result = tv.value._byte;
        break;
    case ut_Word:
        result = tv.value._word;
        break;
    case ut_Addr:
        result = static_cast<p2_WORD>(tv.value._addr[tv.hubmode]);
        break;
    case ut_Long:
        result = static_cast<p2_WORD>(tv.value._long);
        break;
    case ut_Quad:
        result = static_cast<p2_WORD>(tv.value._quad);
        break;
    case ut_Real:
        result = static_cast<p2_WORD>(tv.value._real);
        break;
    case ut_String:
        result = tv.value._byte;
        break;
    }
    return result;
}

p2_LONG P2Union::get_long() const
{
    const P2TypedValue tv = value(0);
    p2_LONG result = 0;
    switch (tv.type) {
    case ut_Invalid:
        break;
    case ut_Bool:
        result = tv.value._bool;
        break;
    case ut_Byte:
        result = tv.value._byte;
        break;
    case ut_Word:
        result = tv.value._word;
        break;
    case ut_Addr:
        result = tv.value._addr[tv.hubmode];
        break;
    case ut_Long:
        result = tv.value._long;
        break;
    case ut_Quad:
        return static_cast<p2_LONG>(tv.value._quad);
    case ut_Real:
        return static_cast<p2_LONG>(tv.value._real);
    case ut_String:
        return tv.value._byte;
    }
    return result;
}

p2_LONG P2Union::get_addr() const
{
    const P2TypedValue& tv = value(0);
    p2_LONG result = 0;
    switch (tv.type) {
    case ut_Invalid:
        break;
    case ut_Bool:
        result = tv.value._bool ? sz_LONG : 0;
        break;
    case ut_Byte:
        result = tv.value._byte * sz_LONG;
        break;
    case ut_Word:
        result = tv.value._word < HUB_ADDR0 ? tv.value._word * sz_LONG
                                           : tv.value._word;
        break;
    case ut_Addr:
        result = tv.value._addr[tv.hubmode];
        break;
    case ut_Long:
        result = tv.value._long < HUB_ADDR0 ? tv.value._long * sz_LONG
                                           : tv.value._long;
        break;
    case ut_Quad:
        result = static_cast<p2_LONG>(tv.value._quad < HUB_ADDR0 ? tv.value._quad * sz_LONG
                                                                : tv.value._quad);
        break;
    case ut_Real:
        result = static_cast<p2_LONG>(tv.value._real * sz_LONG);
        break;
    case ut_String:
        result = tv.value._byte * sz_LONG;
        break;
    }
    return result;
}

p2_LONG P2Union::get_addr(bool hubmode) const
{
    const P2TypedValue& tv = value(0);
    p2_LONG result = 0;
    switch (tv.type) {
    case ut_Addr:
        result = tv.value._addr[hubmode];
        break;
    default:
        result = get_addr();
    }
    return result;
}

p2_QUAD P2Union::get_quad() const
{
    const P2TypedValue tv = value(0);
    p2_QUAD result = 0;
    switch (tv.type) {
    case ut_Invalid:
        break;
    case ut_Bool:
        result = tv.value._bool;
        break;
    case ut_Byte:
        result = tv.value._byte;
        break;
    case ut_Word:
        result = tv.value._word;
        break;
    case ut_Addr:
        result = tv.value._addr[0];
        break;
    case ut_Long:
        result = tv.value._long;
        break;
    case ut_Quad:
        result = tv.value._quad;
        break;
    case ut_Real:
        result = static_cast<p2_QUAD>(tv.value._real);
        break;
    case ut_String:
        result = tv.value._byte;
        break;
    }
    return result;
}

p2_REAL P2Union::get_real() const
{
    const P2TypedValue tv = value(0);
    p2_REAL result = 0.0;
    switch (tv.type) {
    case ut_Invalid:
        break;
    case ut_Bool:
        result = tv.value._bool ? 1.0 : 0.0;
        break;
    case ut_Byte:
        result = tv.value._byte;
        break;
    case ut_Word:
        result = tv.value._word;
        break;
    case ut_Addr:
        result = tv.value._addr[tv.hubmode];
        break;
    case ut_Long:
        result = tv.value._long;
        break;
    case ut_Quad:
        result = tv.value._quad;
        break;
    case ut_Real:
        result = tv.value._real;
        break;
    case ut_String:
        result = tv.value._byte;
        break;
    }
    return result;
}

void P2Union::set_int(const int var)
{
    clear();
    add_int(var);
    m_type = ut_Long;
}

void P2Union::set_bool(const bool var)
{
    clear();
    add_bool(var);
    m_type = ut_Bool;
}

void P2Union::set_char(const char var)
{
    clear();
    add_char(var);
    m_type = ut_Byte;
}

void P2Union::set_byte(const p2_BYTE _byte)
{
    clear();
    add_byte(_byte);
    m_type = ut_Byte;
}

void P2Union::set_word(const p2_WORD _word)
{
    clear();
    add_word(_word);
    m_type = ut_Word;
}

void P2Union::set_long(const p2_LONG var)
{
    clear();
    add_long(var);
    m_type = ut_Long;
}

void P2Union::set_addr(const p2_LONG _cog, const p2_LONG _hub)
{
    clear();
    add_addr(_cog, _hub);
    m_type = ut_Addr;
}

void P2Union::set_addr(const p2_LONG _cog, const p2_LONG _hub, bool hubmode)
{
    clear();
    add_addr(_cog, _hub, hubmode);
    m_type = ut_Addr;
}

void P2Union::set_quad(const p2_QUAD var)
{
    clear();
    add_quad(var);
    m_type = ut_Quad;
}

void P2Union::set_real(const p2_REAL var)
{
    clear();
    add_real(var);
    m_type = ut_Real;
}

void P2Union::set_chars(const p2_CHARS& _chars)
{
    clear();
    add_chars(_chars);
    m_type = ut_String;
}

void P2Union::set_bytes(const p2_BYTES& _bytes)
{
    clear();
    add_bytes(_bytes);
    m_type = ut_String;
}

void P2Union::set_words(const p2_WORDS& _words)
{
    clear();
    add_words(_words);
    m_type = ut_Word;
}

void P2Union::set_longs(const p2_LONGS& _longs)
{
    clear();
    add_longs(_longs);
    m_type = ut_Long;
}

void P2Union::set_quads(const p2_QUADS& _quads)
{
    clear();
    add_quads(_quads);
    m_type = ut_Quad;
}

void P2Union::set_reals(const p2_REALS& _reals)
{
    clear();
    add_reals(_reals);
    m_type = ut_Real;
}

void P2Union::set_array(const QByteArray& _array)
{
    clear();
    add_array(_array);
    m_type = ut_String;
}

void P2Union::set_string(const QString& _string)
{
    clear();
    add_string(_string);
    m_type = ut_String;
}

void P2Union::set_typed_var(const P2TypedValue& _tv)
{
    clear();
    add_typed_var(_tv);
    m_type = _tv.type;
}

void P2Union::add_int(const int _int)
{
    P2TypedValue v{ut_Long, {0}, false};
    v.value._int = _int;
    append(v);
}


void P2Union::add_bool(const bool _bool)
{
    P2TypedValue v{ut_Byte, {0}, false};
    v.value._bool = _bool;
    append(v);
}

void P2Union::add_char(const char _char)
{
    P2TypedValue v{ut_Byte, {0}, false};
    v.value._char = _char;
    append(v);
}

void P2Union::add_byte(const p2_BYTE _byte)
{
    P2TypedValue v{ut_Byte, {0}, false};
    v.value._byte = _byte;
    append(v);
}

void P2Union::add_word(const p2_WORD _word)
{
    P2TypedValue v{ut_Word, {0}, false};
    v.value._word = _word;
    append(v);
}

void P2Union::add_long(const p2_LONG _long)
{
    P2TypedValue v{ut_Long, {0}, false};
    v.value._long = _long;
    append(v);
}

void P2Union::add_addr(const p2_LONG _cog, const p2_LONG _hub)
{
    P2TypedValue v{ut_Addr, {0}, false};
    v.value._addr[0] = _cog;
    v.value._addr[1] = _hub;
    v.hubmode = value(0).hubmode;
    append(v);
}

void P2Union::add_addr(const p2_LONG _cog, const p2_LONG _hub, bool hubmode)
{
    P2TypedValue v{ut_Addr, {0}, false};
    v.value._addr[0] = _cog;
    v.value._addr[1] = _hub;
    v.hubmode = hubmode;
    append(v);
}

void P2Union::add_quad(const p2_QUAD _quad)
{
    P2TypedValue v{ut_Quad, {0}, false};
    v.value._quad = _quad;
    append(v);
}

void P2Union::add_real(const p2_REAL _real)
{
    P2TypedValue v{ut_Real, {0}, false};
    v.value._real = _real;
    append(v);
}

void P2Union::add_chars(const p2_CHARS& _chars)
{
    const int pos = size();
    resize(pos + _chars.size());
    P2TypedValue v{ut_Byte, {0}, false};
    for (int i = 0; i < _chars.size(); i++) {
        v.value._char = _chars[i];
        replace(pos + i, v);
    }
}

void P2Union::add_bytes(const p2_BYTES& _bytes)
{
    const int pos = size();
    resize(pos + _bytes.size());
    P2TypedValue v{ut_Byte, {0}, false};
    for (int i = 0; i < _bytes.size(); i++) {
        v.value._byte = _bytes[i];
        replace(pos + i, v);
    }
}

void P2Union::add_words(const p2_WORDS& _words)
{
    const int pos = size();
    resize(pos + _words.size());
    P2TypedValue v{ut_Word, {0}, false};
    for (int i = 0; i < _words.size(); i++) {
        v.value._word = _words[i];
        replace(pos + i, v);
    }
}

void P2Union::add_longs(const p2_LONGS& _longs)
{
    const int pos = size();
    resize(pos + _longs.size());
    P2TypedValue v{ut_Long, {0}, false};
    for (int i = 0; i < _longs.size(); i++) {
        v.value._long = _longs[i];
        replace(pos + i, v);
    }
}

void P2Union::add_quads(const p2_QUADS& _quads)
{
    const int pos = size();
    resize(pos + _quads.size());
    P2TypedValue v{ut_Quad, {0}, false};
    for (int i = 0; i < _quads.size(); i++) {
        v.value._quad = _quads[i];
        replace(pos + i, v);
    }
}

void P2Union::add_reals(const p2_REALS& _reals)
{
    const int pos = size();
    resize(pos + _reals.size());
    P2TypedValue v{ut_Real, {0}, false};
    for (int i = 0; i < _reals.size(); i++) {
        v.value._real = _reals[i];
        replace(pos + i, v);
    }
}

void P2Union::add_array(const QByteArray& _array)
{
    const int pos = size();
    resize(pos + _array.size());
    P2TypedValue v{ut_Byte, {0}, false};
    for (int i = 0; i < _array.size(); i++) {
        v.value._char = _array[i];
        replace(pos + i, v);
    }
}

void P2Union::add_string(const QString& _string)
{
    const int pos = size();
    resize(pos + _string.size());
    P2TypedValue v{ut_Byte, {0}, false};
    for (int i = 0; i < _string.size(); i++) {
        v.value._char = _string[i].toLatin1();
        replace(pos +i, v);
    }
}

void P2Union::add_typed_var(const P2TypedValue& _tv)
{
    append(_tv);
}

/**
 * @brief Create a QByteArray with all data contained in %pun chained together
 * @param pun const pointer to a P2Union
 * @param expand if true, expand WORD, LONG, QUAD to 2/4/8 BYTEs
 * @return QByteArray with data
 */
QByteArray P2Union::chain_bytes(const P2Union* pun, bool expand)
{
    int size = 4096;
    int pos = 0;
    QByteArray result(size, 0x00);
    QVariant list = QVariant::fromValue(pun->toList());
    if (list.canConvert<QVariantList>()) {
        QSequentialIterable it = list.value<QSequentialIterable>();
        foreach(const QVariant& v, it) {
            P2TypedValue tv = qvariant_cast<P2TypedValue>(v);
            if (expand) {
                switch (tv.type) {
                case ut_Invalid:
                case ut_Bool:
                    result[pos++] = tv.value._bool;
                    break;
                case ut_Byte:
                    result[pos++] = static_cast<char>(tv.value._byte);
                    break;
                case ut_Word:
                    result[pos++] = static_cast<char>(tv.value._word >> 0);
                    result[pos++] = static_cast<char>(tv.value._word >> 8);
                    break;
                case ut_Addr:
                    result[pos++] = static_cast<char>(tv.value._addr[0] >>  0);
                    result[pos++] = static_cast<char>(tv.value._addr[0] >>  8);
                    result[pos++] = static_cast<char>(tv.value._addr[0] >> 16);
                    result[pos++] = static_cast<char>(tv.value._addr[0] >> 24);
                    result[pos++] = static_cast<char>(tv.value._addr[1] >>  0);
                    result[pos++] = static_cast<char>(tv.value._addr[1] >>  8);
                    result[pos++] = static_cast<char>(tv.value._addr[1] >> 16);
                    result[pos++] = static_cast<char>(tv.value._addr[1] >> 24);
                    break;
                case ut_Long:
                    result[pos++] = static_cast<char>(tv.value._long >>  0);
                    result[pos++] = static_cast<char>(tv.value._long >>  8);
                    result[pos++] = static_cast<char>(tv.value._long >> 16);
                    result[pos++] = static_cast<char>(tv.value._long >> 24);
                    break;
                case ut_Quad:
                    result[pos++] = static_cast<char>(tv.value._quad >>  0);
                    result[pos++] = static_cast<char>(tv.value._quad >>  8);
                    result[pos++] = static_cast<char>(tv.value._quad >> 16);
                    result[pos++] = static_cast<char>(tv.value._quad >> 24);
                    result[pos++] = static_cast<char>(tv.value._quad >> 32);
                    result[pos++] = static_cast<char>(tv.value._quad >> 40);
                    result[pos++] = static_cast<char>(tv.value._quad >> 48);
                    result[pos++] = static_cast<char>(tv.value._quad >> 56);
                    break;
                case ut_Real:
                    result[pos++] = static_cast<char>(tv.value._quad >>  0);
                    result[pos++] = static_cast<char>(tv.value._quad >>  8);
                    result[pos++] = static_cast<char>(tv.value._quad >> 16);
                    result[pos++] = static_cast<char>(tv.value._quad >> 24);
                    result[pos++] = static_cast<char>(tv.value._quad >> 32);
                    result[pos++] = static_cast<char>(tv.value._quad >> 40);
                    result[pos++] = static_cast<char>(tv.value._quad >> 48);
                    result[pos++] = static_cast<char>(tv.value._quad >> 56);
                    break;
                case ut_String:
                    result[pos++] = tv.value._char;
                    break;
                }
            } else {
                switch (tv.type) {
                case ut_Invalid:
                case ut_Bool:
                case ut_Byte:
                case ut_Word:
                case ut_Addr:
                case ut_Long:
                case ut_Quad:
                case ut_Real:
                case ut_String:
                    result[pos++] = tv.value._char;
                    break;
                }
            }
            if (pos >= size)
                result.resize(size += 4096);
        }
    } else {
        Q_ASSERT_X(false, "Cannot use QSequentialIterable", "chain_bytes");
    }
    result.truncate(pos);
    return result;
}

/**
 * @brief Create a QByteArray with all data contained in %pun chained together
 * @param pun const pointer to a P2Union
 * @param expand if true, expand LONG, QUAD to 2/4 WORDs
 * @return QByteArray with data
 */
QByteArray P2Union::chain_words(const P2Union* pun, bool expand)
{
    int size = 4096;
    int pos = 0;
    QByteArray result(size, 0x00);
    QVariant list = QVariant::fromValue(pun->toList());
    if (list.canConvert<QVariantList>()) {
        QSequentialIterable it = list.value<QSequentialIterable>();
        foreach(const QVariant& v, it) {
            P2TypedValue tv = qvariant_cast<P2TypedValue>(v);
            if (expand) {
                switch (tv.type) {
                case ut_Invalid:
                case ut_Bool:
                    result[pos++] = tv.value._bool;
                    result[pos++] = 0;
                    break;
                case ut_Byte:
                    result[pos++] = static_cast<char>(tv.value._byte);
                    result[pos++] = 0;
                    break;
                case ut_Word:
                    result[pos++] = static_cast<char>(tv.value._word >> 0);
                    result[pos++] = static_cast<char>(tv.value._word >> 8);
                    break;
                case ut_Addr:
                    result[pos++] = static_cast<char>(tv.value._addr[0] >>  0);
                    result[pos++] = static_cast<char>(tv.value._addr[0] >>  8);
                    result[pos++] = static_cast<char>(tv.value._addr[0] >> 16);
                    result[pos++] = static_cast<char>(tv.value._addr[0] >> 24);
                    result[pos++] = static_cast<char>(tv.value._addr[1] >>  0);
                    result[pos++] = static_cast<char>(tv.value._addr[1] >>  8);
                    result[pos++] = static_cast<char>(tv.value._addr[1] >> 16);
                    result[pos++] = static_cast<char>(tv.value._addr[1] >> 24);
                    break;
                case ut_Long:
                    result[pos++] = static_cast<char>(tv.value._long >>  0);
                    result[pos++] = static_cast<char>(tv.value._long >>  8);
                    result[pos++] = static_cast<char>(tv.value._long >> 16);
                    result[pos++] = static_cast<char>(tv.value._long >> 24);
                    break;
                case ut_Quad:
                    result[pos++] = static_cast<char>(tv.value._quad >>  0);
                    result[pos++] = static_cast<char>(tv.value._quad >>  8);
                    result[pos++] = static_cast<char>(tv.value._quad >> 16);
                    result[pos++] = static_cast<char>(tv.value._quad >> 24);
                    result[pos++] = static_cast<char>(tv.value._quad >> 32);
                    result[pos++] = static_cast<char>(tv.value._quad >> 40);
                    result[pos++] = static_cast<char>(tv.value._quad >> 48);
                    result[pos++] = static_cast<char>(tv.value._quad >> 56);
                    break;
                case ut_Real:
                    result[pos++] = static_cast<char>(tv.value._quad >>  0);
                    result[pos++] = static_cast<char>(tv.value._quad >>  8);
                    result[pos++] = static_cast<char>(tv.value._quad >> 16);
                    result[pos++] = static_cast<char>(tv.value._quad >> 24);
                    result[pos++] = static_cast<char>(tv.value._quad >> 32);
                    result[pos++] = static_cast<char>(tv.value._quad >> 40);
                    result[pos++] = static_cast<char>(tv.value._quad >> 48);
                    result[pos++] = static_cast<char>(tv.value._quad >> 56);
                    break;
                case ut_String:
                    result[pos++] = tv.value._char;
                    result[pos++] = 0;
                    break;
                }
            } else {
                switch (tv.type) {
                case ut_Invalid:
                case ut_Bool:
                case ut_Byte:
                case ut_String:
                    result[pos++] = tv.value._char;
                    result[pos++] = 0;
                    break;
                case ut_Word:
                case ut_Addr:
                case ut_Long:
                case ut_Quad:
                case ut_Real:
                    result[pos++] = static_cast<char>(tv.value._word >> 0);
                    result[pos++] = static_cast<char>(tv.value._word >> 8);
                    break;
                }
            }
        }
        if (pos >= size)
            result.resize(size += 4096);
    } else {
        Q_ASSERT_X(false, "Cannot use QSequentialIterable", "chain_words");
    }
    result.truncate(pos);
    return result;
}

/**
 * @brief Create a QByteArray with all data contained in %tv chained together
 * @param pun const pointer to a P2Union
 * @param expand if true, expand LONG, QUAD to 2/4 WORDs
 * @return QByteArray with data
 */
QByteArray P2Union::chain_longs(const P2Union* pun, bool expand)
{
    int size = 4096;
    int pos = 0;
    QByteArray result(size, 0x00);
    QVariant list = QVariant::fromValue(pun->toList());
    if (list.canConvert<QVariantList>()) {
        QSequentialIterable it = list.value<QSequentialIterable>();
        foreach(const QVariant& v, it) {
            P2TypedValue tv = qvariant_cast<P2TypedValue>(v);
            if (expand) {
                switch (tv.type) {
                case ut_Invalid:
                    Q_ASSERT(tv.type != ut_Invalid);
                    break;
                case ut_Bool:
                    result[pos++] = tv.value._bool;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    break;
                case ut_Byte:
                    result[pos++] = static_cast<char>(tv.value._byte);
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    break;
                case ut_Word:
                    result[pos++] = static_cast<char>(tv.value._word >> 0);
                    result[pos++] = static_cast<char>(tv.value._word >> 8);
                    result[pos++] = 0;
                    result[pos++] = 0;
                    break;
                case ut_Addr:
                    result[pos++] = static_cast<char>(tv.value._addr[0] >>  0);
                    result[pos++] = static_cast<char>(tv.value._addr[0] >>  8);
                    result[pos++] = static_cast<char>(tv.value._addr[0] >> 16);
                    result[pos++] = static_cast<char>(tv.value._addr[0] >> 24);
                    result[pos++] = static_cast<char>(tv.value._addr[1] >>  0);
                    result[pos++] = static_cast<char>(tv.value._addr[1] >>  8);
                    result[pos++] = static_cast<char>(tv.value._addr[1] >> 16);
                    result[pos++] = static_cast<char>(tv.value._addr[1] >> 24);
                    break;
                case ut_Long:
                    result[pos++] = static_cast<char>(tv.value._long >>  0);
                    result[pos++] = static_cast<char>(tv.value._long >>  8);
                    result[pos++] = static_cast<char>(tv.value._long >> 16);
                    result[pos++] = static_cast<char>(tv.value._long >> 24);
                    break;
                case ut_Quad:
                    result[pos++] = static_cast<char>(tv.value._quad >>  0);
                    result[pos++] = static_cast<char>(tv.value._quad >>  8);
                    result[pos++] = static_cast<char>(tv.value._quad >> 16);
                    result[pos++] = static_cast<char>(tv.value._quad >> 24);
                    result[pos++] = static_cast<char>(tv.value._quad >> 32);
                    result[pos++] = static_cast<char>(tv.value._quad >> 40);
                    result[pos++] = static_cast<char>(tv.value._quad >> 48);
                    result[pos++] = static_cast<char>(tv.value._quad >> 56);
                    break;
                case ut_Real:
                    result[pos++] = static_cast<char>(tv.value._quad >>  0);
                    result[pos++] = static_cast<char>(tv.value._quad >>  8);
                    result[pos++] = static_cast<char>(tv.value._quad >> 16);
                    result[pos++] = static_cast<char>(tv.value._quad >> 24);
                    result[pos++] = static_cast<char>(tv.value._quad >> 32);
                    result[pos++] = static_cast<char>(tv.value._quad >> 40);
                    result[pos++] = static_cast<char>(tv.value._quad >> 48);
                    result[pos++] = static_cast<char>(tv.value._quad >> 56);
                    break;
                case ut_String:
                    result[pos++] = tv.value._char;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    break;
                }
            } else {
                switch (tv.type) {
                case ut_Invalid:
                    Q_ASSERT(tv.type != ut_Invalid);
                    break;
                case ut_Bool:
                case ut_Byte:
                case ut_String:
                    result[pos++] = tv.value._char;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    break;
                case ut_Word:
                    result[pos++] = static_cast<char>(tv.value._long >>  0);
                    result[pos++] = static_cast<char>(tv.value._long >>  8);
                    result[pos++] = 0;
                    result[pos++] = 0;
                    break;
                case ut_Addr:
                case ut_Long:
                case ut_Quad:
                case ut_Real:
                    result[pos++] = static_cast<char>(tv.value._long >>  0);
                    result[pos++] = static_cast<char>(tv.value._long >>  8);
                    result[pos++] = static_cast<char>(tv.value._long >> 16);
                    result[pos++] = static_cast<char>(tv.value._long >> 24);
                    break;
                }
            }
            if (pos >= size)
                result.resize(size += 4096);
        }
    } else {
        Q_ASSERT_X(false, "Cannot use QSequentialIterable", "chain_words");
    }
    result.truncate(pos);
    return result;
}

QByteArray P2Union::chain_quads(const P2Union* pun, bool expand)
{
    int size = 4096;
    int pos = 0;
    QByteArray result(size, 0x00);
    QVariant list = QVariant::fromValue(pun->toList());
    if (list.canConvert<QVariantList>()) {
        QSequentialIterable it = list.value<QSequentialIterable>();
        foreach(const QVariant& v, it) {
            P2TypedValue tv = qvariant_cast<P2TypedValue>(v);
            if (expand) {
                switch (tv.type) {
                case ut_Invalid:
                case ut_Bool:
                    result[pos++] = tv.value._bool;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    break;
                case ut_Byte:
                    result[pos++] = static_cast<char>(tv.value._byte);
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    break;
                case ut_Word:
                    result[pos++] = static_cast<char>(tv.value._word >> 0);
                    result[pos++] = static_cast<char>(tv.value._word >> 8);
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    break;
                case ut_Addr:
                    result[pos++] = static_cast<char>(tv.value._addr[0] >>  0);
                    result[pos++] = static_cast<char>(tv.value._addr[0] >>  8);
                    result[pos++] = static_cast<char>(tv.value._addr[0] >> 16);
                    result[pos++] = static_cast<char>(tv.value._addr[0] >> 24);
                    result[pos++] = static_cast<char>(tv.value._addr[1] >>  0);
                    result[pos++] = static_cast<char>(tv.value._addr[1] >>  8);
                    result[pos++] = static_cast<char>(tv.value._addr[1] >> 16);
                    result[pos++] = static_cast<char>(tv.value._addr[1] >> 24);
                    break;
                case ut_Long:
                    result[pos++] = static_cast<char>(tv.value._long >>  0);
                    result[pos++] = static_cast<char>(tv.value._long >>  8);
                    result[pos++] = static_cast<char>(tv.value._long >> 16);
                    result[pos++] = static_cast<char>(tv.value._long >> 24);
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    break;
                case ut_Quad:
                    result[pos++] = static_cast<char>(tv.value._quad >>  0);
                    result[pos++] = static_cast<char>(tv.value._quad >>  8);
                    result[pos++] = static_cast<char>(tv.value._quad >> 16);
                    result[pos++] = static_cast<char>(tv.value._quad >> 24);
                    result[pos++] = static_cast<char>(tv.value._quad >> 32);
                    result[pos++] = static_cast<char>(tv.value._quad >> 40);
                    result[pos++] = static_cast<char>(tv.value._quad >> 48);
                    result[pos++] = static_cast<char>(tv.value._quad >> 56);
                    break;
                case ut_Real:
                    result[pos++] = static_cast<char>(tv.value._quad >>  0);
                    result[pos++] = static_cast<char>(tv.value._quad >>  8);
                    result[pos++] = static_cast<char>(tv.value._quad >> 16);
                    result[pos++] = static_cast<char>(tv.value._quad >> 24);
                    result[pos++] = static_cast<char>(tv.value._quad >> 32);
                    result[pos++] = static_cast<char>(tv.value._quad >> 40);
                    result[pos++] = static_cast<char>(tv.value._quad >> 48);
                    result[pos++] = static_cast<char>(tv.value._quad >> 56);
                    break;
                case ut_String:
                    result[pos++] = tv.value._char;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    break;
                }
            } else {
                switch (tv.type) {
                case ut_Invalid:
                case ut_Bool:
                case ut_Byte:
                case ut_String:
                    result[pos++] = tv.value._char;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    break;
                case ut_Word:
                    result[pos++] = static_cast<char>(tv.value._word >> 0);
                    result[pos++] = static_cast<char>(tv.value._word >> 8);
                    break;
                case ut_Addr:
                    result[pos++] = static_cast<char>(tv.value._addr[0] >>  0);
                    result[pos++] = static_cast<char>(tv.value._addr[0] >>  8);
                    result[pos++] = static_cast<char>(tv.value._addr[0] >> 16);
                    result[pos++] = static_cast<char>(tv.value._addr[0] >> 24);
                    result[pos++] = static_cast<char>(tv.value._addr[1] >>  0);
                    result[pos++] = static_cast<char>(tv.value._addr[1] >>  8);
                    result[pos++] = static_cast<char>(tv.value._addr[1] >> 16);
                    result[pos++] = static_cast<char>(tv.value._addr[1] >> 24);
                    break;
                case ut_Long:
                    result[pos++] = static_cast<char>(tv.value._long >>  0);
                    result[pos++] = static_cast<char>(tv.value._long >>  8);
                    result[pos++] = static_cast<char>(tv.value._long >> 16);
                    result[pos++] = static_cast<char>(tv.value._long >> 24);
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    result[pos++] = 0;
                    break;
                case ut_Quad:
                case ut_Real:
                    result[pos++] = static_cast<char>(tv.value._quad >>  0);
                    result[pos++] = static_cast<char>(tv.value._quad >>  8);
                    result[pos++] = static_cast<char>(tv.value._quad >> 16);
                    result[pos++] = static_cast<char>(tv.value._quad >> 24);
                    result[pos++] = static_cast<char>(tv.value._quad >> 32);
                    result[pos++] = static_cast<char>(tv.value._quad >> 40);
                    result[pos++] = static_cast<char>(tv.value._quad >> 48);
                    result[pos++] = static_cast<char>(tv.value._quad >> 56);
                    break;
                }
            }
            if (pos >= size)
                result.resize(size += 4096);
        }
    } else {
        Q_ASSERT_X(false, "Cannot use QSequentialIterable", "chain_words");
    }
    result.truncate(pos);
    return result;
}

p2_CHARS P2Union::get_chars(bool expand) const
{
    p2_CHARS result;
    QByteArray chars = chain_bytes(this, expand);
    size_t size = static_cast<size_t>(chars.count());
    result.resize(chars.size());
    if (size > 0)
        memcpy(result.data(), chars.constData(), size);
    return result;
}

p2_BYTES P2Union::get_bytes(bool expand) const
{
    p2_BYTES result;
    QByteArray bytes = chain_bytes(this, expand);
    size_t size = static_cast<size_t>(bytes.size());
    result.resize(bytes.size() / sz_BYTE);
    if (size > 0)
        memcpy(result.data(), bytes.constData(), size);
    return result;
}

p2_WORDS P2Union::get_words(bool expand) const
{
    p2_WORDS result;
    QByteArray words = chain_words(this, expand);
    size_t size = static_cast<size_t>(words.size());
    result.resize((words.size() + sz_WORD - 1) / sz_WORD);
    if (size > 0) {
        memcpy(result.data(), words.constData(), size);
    }
    return result;
}

p2_LONGS P2Union::get_longs(bool expand) const
{
    p2_LONGS result;
    QByteArray longs = chain_longs(this, expand);
    size_t size = static_cast<size_t>(longs.size());
    result.resize((longs.size() + sz_LONG - 1) / sz_LONG);
    if (size > 0) {
        memcpy(result.data(), longs.constData(), size);
    }
    return result;
}

p2_QUADS P2Union::get_quads(bool expand) const
{
    p2_QUADS result;
    QByteArray quads = chain_quads(this, expand);
    size_t size = static_cast<size_t>(quads.size());
    result.resize((quads.size() + sz_QUAD - 1) / sz_QUAD);
    if (size > 0) {
        memcpy(result.data(), quads.constData(), size);
    }
    return result;
}

p2_REALS P2Union::get_reals(bool expand) const
{
    p2_REALS result;
    QByteArray reals = chain_bytes(this, expand);
    size_t size = static_cast<size_t>(reals.size());
    result.resize((reals.size() + sz_REAL - 1) / sz_REAL);
    if (size > 0) {
        memcpy(result.data(), reals.constData(), size);
    }
    return result;
}

QString P2Union::get_string(bool expand) const
{
    QByteArray chars = chain_bytes(this, expand);
    return QString::fromUtf8(chars);
}

QString P2Union::str(bool with_type, p2_FORMAT_e fmt) const
{
    return str(*this, with_type, fmt);
}

/**
 * @brief Return the size of a unit of %type
 * @param type one of p2_Union_e
 * @return size in bytes
 */
int P2Union::unit(p2_Union_e type)
{
    switch (type) {
    case ut_Bool:    return sz_BYTE;
    case ut_Byte:    return sz_BYTE;
    case ut_Word:    return sz_WORD;
    case ut_Addr:    return sz_QUAD;
    case ut_Long:    return sz_LONG;
    case ut_Quad:    return sz_QUAD;
    case ut_Real:    return sz_REAL;
    case ut_String:  return sz_BYTE;
    case ut_Invalid: return sz_BYTE;
    }
    return 1;
}

/**
 * @brief Return the (technical) name of a unit of %type
 * @param type one of p2_Union_e
 * @return QString with the type name
 */
QString P2Union::type_name(p2_Union_e type)
{
    switch (type) {
    case ut_Invalid:
        return QStringLiteral("Invalid");
    case ut_Bool:
        return QStringLiteral("Bool");
    case ut_Byte:
        return QStringLiteral("Byte");
    case ut_Word:
        return QStringLiteral("Word");
    case ut_Addr:
        return QStringLiteral("Addr");
    case ut_Long:
        return QStringLiteral("Long");
    case ut_Quad:
        return QStringLiteral("Quad");
    case ut_Real:
        return QStringLiteral("Real");
    case ut_String:
        return QStringLiteral("String");
    }
    return QStringLiteral("<invalid>");
}

static inline QString bin(const p2_QUAD val, int digits = 0)
{
    return QString("%1").arg(val, digits, 2, QChar('0'));
}

static inline QString dec(const p2_QUAD val, int digits = 0)
{
    return QString("%1").arg(val, digits, 10, QChar('0'));
}

static inline QString hex(const p2_QUAD val, int digits = 0)
{
    return QString("%1").arg(val, digits, 16, QChar('0'));
}

QString P2Union::str(const P2Union& u, bool with_type, p2_FORMAT_e fmt)
{
    QString result;

    if (with_type)
        result = QString("<%1> ")
                 .arg(u.type_name());
    switch (u.type()) {
    case ut_Invalid:
        result = QLatin1String("<invalid>");
        break;

    case ut_Bool:
        result += QString("%1")
                  .arg(u.get_bool() ? "true" : "false");
        break;

    case ut_Byte:
        {
            p2_BYTE _byte = u.get_byte();
            switch (fmt) {
            case fmt_dec:
                result += dec(_byte,0);
                break;
            case fmt_bin:
                result += chr_percent;
                result += bin(_byte, 8);
                break;
            case fmt_bit:
                result += chr_percent;
                result += hex(_byte, 2);
                break;
            case fmt_hex:
            default:
                result += chr_percent;
                result += hex(_byte, 2);
            }
        }
        break;

    case ut_Word:
        {
            p2_WORD _word = u.get_word();
            switch (fmt) {
            case fmt_dec:
                result += dec(_word,0);
                break;
            case fmt_bin:
                result += chr_percent;
                result += bin(_word >> 8, 8);
                result += chr_skip_digit;
                result += bin(_word & 0xff, 8);
                break;
            case fmt_bit:
                result += chr_percent;
                result += hex(_word >> 8, 2);
                result += chr_skip_digit;
                result += hex(_word & 0xff, 2);
                break;
            case fmt_hex:
            default:
                result += chr_percent;
                result += hex(_word, 4);
            }
        }
        break;

    case ut_Addr:
        {
            p2_LONG _cog = u.get_addr(p2_cog) / sz_LONG;
            p2_LONG _hub = u.get_addr(p2_hub);
            switch (fmt) {
            case fmt_dec:
                result += dec(_hub,0);
                if (!u.hubmode()) {
                    result += chr_colon;
                    result += dec(_cog,0);
                }
                break;
            case fmt_bin:
                result += chr_percent;
                result += bin((_hub >> 24) & 0xff, 8);
                result += chr_skip_digit;
                result += bin((_hub >> 16) & 0xff, 8);
                result += chr_skip_digit;
                result += bin((_hub >>  8) & 0xff, 8);
                result += chr_skip_digit;
                result += bin((_hub >>  0) & 0xff, 8);
                if (!u.hubmode()) {
                    result += chr_colon;
                    result += bin((_cog >> 16) & 0xff, 2);
                    result += chr_skip_digit;
                    result += bin((_cog>>  8) & 0xff, 8);
                    result += chr_skip_digit;
                    result += bin((_cog >> 0) & 0xff, 8);
                }
                break;
            case fmt_bit:
                result += chr_dollar;
                result += hex((_hub >> 24) & 0xff, 2);
                result += chr_skip_digit;
                result += hex((_hub >> 16) & 0xff, 2);
                result += chr_skip_digit;
                result += hex((_hub >>  8) & 0xff, 2);
                result += chr_skip_digit;
                result += hex((_hub >>  0) & 0xff, 2);
                if (!u.hubmode()) {
                    result += chr_colon;
                    result += hex((_cog >> 16) & 0xff, 1);
                    result += chr_skip_digit;
                    result += hex((_cog >>  8) & 0xff, 2);
                    result += chr_skip_digit;
                    result += hex((_cog >>  0) & 0xff, 2);
                }
                break;
            case fmt_hex:
            default:
                result += chr_dollar;
                result += hex(_hub, 6);
                if (!u.hubmode()) {
                    result += chr_colon;
                    result += hex(_cog, 3);
                }
            }
        }
        break;

    case ut_Long:
        {
            p2_LONG _long = u.get_long();
            switch (fmt) {
            case fmt_dec:
                result += QString("%1")
                          .arg(dec(_long, 0));
                break;
            case fmt_bin:
                result += chr_percent;
                result += bin((_long >> 24) & 0xff, 8);
                result += chr_skip_digit;
                result += bin((_long >> 16) & 0xff, 8);
                result += chr_skip_digit;
                result += bin((_long >>  8) & 0xff, 8);
                result += chr_skip_digit;
                result += bin((_long >>  0) & 0xff, 8);
                break;
            case fmt_bit:
                result += chr_dollar;
                result += hex((_long >> 24) & 0xff, 2);
                result += chr_skip_digit;
                result += hex((_long >> 16) & 0xff, 2);
                result += chr_skip_digit;
                result += hex((_long >>  8) & 0xff, 2);
                result += chr_skip_digit;
                result += hex((_long >>  0) & 0xff, 2);
                break;
            case fmt_hex:
            default:
                result += chr_dollar;
                result += hex(_long, 0);
            }
        }
        break;

    case ut_Quad:
        {
            p2_QUAD _quad = u.get_quad();
            switch (fmt) {
            case fmt_dec:
                result += dec(_quad);
                break;
            case fmt_bin:
                result += chr_percent;
                if (_quad & HMAX) {
                    result += bin((_quad >> 56) & 0xff, 8);
                    result += chr_skip_digit;
                    result += bin((_quad >> 48) & 0xff, 8);
                    result += chr_skip_digit;
                    result += bin((_quad >> 40) & 0xff, 8);
                    result += chr_skip_digit;
                    result += bin((_quad >> 32) & 0xff, 8);
                    result += chr_skip_digit;
                }
                result += bin((_quad >> 24) & 0xff, 8);
                result += chr_skip_digit;
                result += bin((_quad >> 16) & 0xff, 8);
                result += chr_skip_digit;
                result += bin((_quad >>  8) & 0xff, 8);
                result += chr_skip_digit;
                result += bin((_quad >>  0) & 0xff, 8);
                break;
            case fmt_bit:
                result += chr_dollar;
                if (_quad & HMAX) {
                    result += hex((_quad >> 56) & 0xff, 2);
                    result += chr_skip_digit;
                    result += hex((_quad >> 48) & 0xff, 2);
                    result += chr_skip_digit;
                    result += hex((_quad >> 40) & 0xff, 2);
                    result += chr_skip_digit;
                    result += hex((_quad >> 32) & 0xff, 2);
                    result += chr_skip_digit;
                }
                result += hex((_quad >> 24) & 0xff, 2);
                result += chr_skip_digit;
                result += hex((_quad >> 16) & 0xff, 2);
                result += chr_skip_digit;
                result += hex((_quad >>  8) & 0xff, 2);
                result += chr_skip_digit;
                result += hex((_quad >>  0) & 0xff, 2);
                break;
            case fmt_hex:
            default:
                result += chr_dollar;
                result += hex(_quad);
            }
        }
        break;

    case ut_Real:
        result += QString("%1")
                  .arg(u.get_real(), 2, 'f');
        break;

    case ut_String:
        result += u.get_string();
        break;
    }
    return result;
}
