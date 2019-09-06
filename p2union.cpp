#include "p2union.h"

P2Union::P2Union()
    : QVariantList()
    , m_type(ut_Invalid)
{
    mt_TypedVar = QMetaType::type("TypedVar");
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
    set_long(i);
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

P2Union::P2Union(const QByteArray& ba) : P2Union()
{
    set_array(ba);
}

P2Union::P2Union(p2_BYTES bv) : P2Union()
{
    set_bytes(QVariant::fromValue(bv));
}

P2Union::P2Union(p2_WORDS wv) : P2Union()
{
    set_words(QVariant::fromValue(wv));
}

P2Union::P2Union(p2_LONGS lv) : P2Union()
{
    set_longs(QVariant::fromValue(lv));
}

int P2Union::unit() const
{
    switch (m_type) {
    case ut_Bool:    return sz_BYTE;
    case ut_Byte:    return sz_BYTE;
    case ut_Word:    return sz_WORD;
    case ut_Addr:    return sz_LONG;
    case ut_Long:    return sz_LONG;
    case ut_Quad:    return sz_QUAD;
    case ut_Real:    return sz_REAL;
    case ut_String:  return sz_BYTE;
    case ut_Invalid: return sz_BYTE;
    }
    return 1;
}

p2_union_e P2Union::type() const
{
    return m_type;
}

QString P2Union::type_name() const
{
    return type_name(m_type);
}

void P2Union::set_type(p2_union_e type)
{
    m_type = type;
}

bool P2Union::is_zero() const
{
    foreach(const QVariant& v, QVariantList::toVector()) {
        if (v.canConvert(mt_TypedVar)) {
            TypedVar tv = qvariant_cast<TypedVar>(v);
            switch (tv.first) {
            case ut_Invalid:
                return true;
            case ut_Bool:
                if (false != qvariant_cast<bool>(tv.second))
                    return false;
                break;
            case ut_Byte:
                if (null_BYTE != qvariant_cast<p2_BYTE>(tv.second))
                    return false;
                break;
            case ut_Word:
                if (null_WORD != qvariant_cast<p2_WORD>(tv.second))
                    return false;
                break;
            case ut_Addr:
            case ut_Long:
                if (null_LONG != qvariant_cast<p2_LONG>(tv.second))
                    return false;
                break;
            case ut_Quad:
                if (null_QUAD != qvariant_cast<p2_QUAD>(tv.second))
                    return false;
                break;
            case ut_Real:
                if (!qFuzzyIsNull(qvariant_cast<p2_REAL>(tv.second)))
                    return false;
                break;
            case ut_String:
                Q_ASSERT(tv.first != ut_Invalid);
                break;
            }
        } else {
            switch (m_type) {
            case ut_Invalid:
                return true;
            case ut_Bool:
                if (qvariant_cast<bool>(v))
                    return false;
                break;
            case ut_Byte:
                if (null_BYTE != qvariant_cast<p2_BYTE>(v))
                    return false;
                break;
            case ut_Word:
                if (null_WORD != qvariant_cast<p2_WORD>(v))
                    return false;
                break;
            case ut_Addr:
            case ut_Long:
                if (null_LONG != qvariant_cast<p2_LONG>(v))
                    return false;
                break;
            case ut_Quad:
                if (null_QUAD != qvariant_cast<p2_QUAD>(v))
                    return false;
                break;
            case ut_Real:
                if (!qFuzzyIsNull(qvariant_cast<p2_REAL>(v)))
                    return false;
                break;
            case ut_String:
                Q_ASSERT(m_type != ut_Invalid);
                break;
            }
        }
    }
    return true;
}

QVariant P2Union::get_bool() const
{
    if (isEmpty())
        return false;
    const QVariant& v = QVariantList::at(0);
    if (v.canConvert(QMetaType::type("TypedVar"))) {
        const TypedVar& tv = qvariant_cast<TypedVar>(v);
        return qvariant_cast<bool>(tv.second);
    }
    return false;
}

QVariant P2Union::get_char() const
{
    if (isEmpty())
        return char(0);
    const QVariant& v = QVariantList::at(0);
    if (v.canConvert(QMetaType::type("TypedVar"))) {
        const TypedVar& tv = qvariant_cast<TypedVar>(v);
        return qvariant_cast<char>(tv.second);
    }
    return char(0);
}

QVariant P2Union::get_byte() const
{
    if (isEmpty())
        return null_BYTE;
    const QVariant& v = QVariantList::at(0);
    if (v.canConvert(QMetaType::type("TypedVar"))) {
        const TypedVar& tv = qvariant_cast<TypedVar>(v);
        return qvariant_cast<p2_BYTE>(tv.second);
    }
    return null_BYTE;
}

QVariant P2Union::get_word() const
{
    if (isEmpty())
        return null_WORD;
    const QVariant& v = QVariantList::at(0);
    if (v.canConvert(QMetaType::type("TypedVar"))) {
        const TypedVar& tv = qvariant_cast<TypedVar>(v);
        return qvariant_cast<p2_WORD>(tv.second);
    }
    return null_WORD;
}

QVariant P2Union::get_long() const
{
    if (isEmpty())
        return null_LONG;
    const QVariant& v = QVariantList::at(0);
    if (v.canConvert(QMetaType::type("TypedVar"))) {
        const TypedVar& tv = qvariant_cast<TypedVar>(v);
        return qvariant_cast<p2_LONG>(tv.second);
    }
    return null_LONG;
}

QVariant P2Union::get_quad() const
{
    if (isEmpty())
        return null_QUAD;
    const QVariant& v = QVariantList::at(0);
    if (v.canConvert(QMetaType::type("TypedVar"))) {
        const TypedVar& tv = qvariant_cast<TypedVar>(v);
        return qvariant_cast<p2_QUAD>(tv.second);
    }
    return null_QUAD;
}

QVariant P2Union::get_real() const
{
    if (isEmpty())
        return null_REAL;
    const QVariant& v = QVariantList::at(0);
    if (v.canConvert(QMetaType::type("TypedVar"))) {
        const TypedVar& tv = qvariant_cast<TypedVar>(v);
        return qvariant_cast<p2_REAL>(tv.second);
    }
    return null_REAL;
}

p2_CHARS P2Union::to_chars(const TypedVar& tv)
{
    p2_CHARS result;
    switch (tv.first) {
    case ut_Invalid:
        break;
    case ut_Bool:
        result += qvariant_cast<bool>(tv.second);
        break;
    case ut_Byte:
        {
            p2_BYTE val = qvariant_cast<p2_BYTE>(tv.second);
            result += static_cast<char>(val);
        }
        break;
    case ut_Word:
        {
            p2_WORD val = qvariant_cast<p2_WORD>(tv.second);
            result += static_cast<char>(val >> 0);
            result += static_cast<char>(val >> 8);
        }
        break;
    case ut_Addr:
    case ut_Long:
        {
            p2_LONG val = qvariant_cast<p2_LONG>(tv.second);
            result += static_cast<char>(val >> 0);
            result += static_cast<char>(val >> 8);
            result += static_cast<char>(val >> 16);
            result += static_cast<char>(val >> 24);
        }
        break;
    case ut_Quad:
        {
            p2_QUAD val = qvariant_cast<p2_QUAD>(tv.second);
            result += static_cast<char>(val >> 0);
            result += static_cast<char>(val >> 8);
            result += static_cast<char>(val >> 16);
            result += static_cast<char>(val >> 24);
            result += static_cast<char>(val >> 32);
            result += static_cast<char>(val >> 40);
            result += static_cast<char>(val >> 48);
            result += static_cast<char>(val >> 56);
        }
        break;
    case ut_Real:
        {
            p2_REAL val = qvariant_cast<p2_REAL>(tv.second);
            const char* ptr = reinterpret_cast<const char *>(&val);
            result += static_cast<char>(ptr[0]);
            result += static_cast<char>(ptr[1]);
            result += static_cast<char>(ptr[2]);
            result += static_cast<char>(ptr[3]);
            result += static_cast<char>(ptr[4]);
            result += static_cast<char>(ptr[5]);
            result += static_cast<char>(ptr[6]);
            result += static_cast<char>(ptr[7]);
        }
        break;
    case ut_String:
        {
            p2_BYTES val = qvariant_cast<p2_BYTES>(tv.second);
            foreach(const p2_BYTE b, val)
                result += static_cast<char>(b);
        }
        break;
    }
    return result;
}

QVariant P2Union::get_chars() const
{
    p2_CHARS result;
    const_iterator it = constBegin();
    while (it != constEnd()) {
        const QVariant& v = it->value<QVariant>();
        if (v.canConvert(QMetaType::type("TypedVar"))) {
            const TypedVar& tv = qvariant_cast<TypedVar>(v);
            result += to_chars(tv);
        }
        it++;
    }
    return QVariant::fromValue(result);
}

p2_BYTES P2Union::to_bytes(const TypedVar& tv)
{
    p2_BYTES result;
    switch (tv.first) {
    case ut_Invalid:
        break;
    case ut_Bool:
        result += qvariant_cast<bool>(tv.second);
        break;
    case ut_Byte:
        {
            p2_BYTE val = qvariant_cast<p2_BYTE>(tv.second);
            result += static_cast<p2_BYTE>(val);
        }
        break;
    case ut_Word:
        {
            p2_WORD val = qvariant_cast<p2_WORD>(tv.second);
            result += static_cast<p2_BYTE>(val >> 0);
            result += static_cast<p2_BYTE>(val >> 8);
        }
        break;
    case ut_Addr:
    case ut_Long:
        {
            p2_LONG val = qvariant_cast<p2_LONG>(tv.second);
            result += static_cast<p2_BYTE>(val >> 0);
            result += static_cast<p2_BYTE>(val >> 8);
            result += static_cast<p2_BYTE>(val >> 16);
            result += static_cast<p2_BYTE>(val >> 24);
        }
        break;
    case ut_Quad:
        {
            p2_QUAD val = qvariant_cast<p2_QUAD>(tv.second);
            result += static_cast<p2_BYTE>(val >> 0);
            result += static_cast<p2_BYTE>(val >> 8);
            result += static_cast<p2_BYTE>(val >> 16);
            result += static_cast<p2_BYTE>(val >> 24);
            result += static_cast<p2_BYTE>(val >> 32);
            result += static_cast<p2_BYTE>(val >> 40);
            result += static_cast<p2_BYTE>(val >> 48);
            result += static_cast<p2_BYTE>(val >> 56);
        }
        break;
    case ut_Real:
        {
            p2_REAL val = qvariant_cast<p2_REAL>(tv.second);
            const p2_BYTE* ptr = reinterpret_cast<const p2_BYTE*>(&val);
            result += static_cast<p2_BYTE>(ptr[0]);
            result += static_cast<p2_BYTE>(ptr[1]);
            result += static_cast<p2_BYTE>(ptr[2]);
            result += static_cast<p2_BYTE>(ptr[3]);
            result += static_cast<p2_BYTE>(ptr[4]);
            result += static_cast<p2_BYTE>(ptr[5]);
            result += static_cast<p2_BYTE>(ptr[6]);
            result += static_cast<p2_BYTE>(ptr[7]);
        }
        break;
    case ut_String:
        {
            p2_BYTES val = qvariant_cast<p2_BYTES>(tv.second);
            foreach(const p2_BYTE b, val)
                result += b;
        }
        break;
    }
    return result;
}

QVariant P2Union::get_bytes() const
{
    p2_BYTES result;
    const_iterator it = constBegin();
    while (it != constEnd()) {
        const QVariant& v = it->value<QVariant>();
        if (v.canConvert(QMetaType::type("TypedVar"))) {
            const TypedVar& tv = qvariant_cast<TypedVar>(v);
            result += to_bytes(tv);
        }
        it++;
    }
    return QVariant::fromValue(result);
}

p2_WORDS P2Union::to_words(const TypedVar& tv)
{
    p2_WORDS result;
    switch (tv.first) {
    case ut_Invalid:
        break;
    case ut_Bool:
        result += qvariant_cast<bool>(tv.second);
        break;
    case ut_Byte:
        {
            p2_BYTE val = qvariant_cast<p2_BYTE>(tv.second);
            result += static_cast<p2_WORD>(val);
        }
        break;
    case ut_Word:
        {
            p2_WORD val = qvariant_cast<p2_WORD>(tv.second);
            result += val;
        }
        break;
    case ut_Addr:
    case ut_Long:
        {
            p2_LONG val = qvariant_cast<p2_LONG>(tv.second);
            result += static_cast<p2_WORD>(val >> 0);
            result += static_cast<p2_WORD>(val >> 16);
        }
        break;
    case ut_Quad:
        {
            p2_QUAD val = qvariant_cast<p2_QUAD>(tv.second);
            result += static_cast<p2_WORD>(val >> 0);
            result += static_cast<p2_WORD>(val >> 16);
            result += static_cast<p2_WORD>(val >> 32);
            result += static_cast<p2_WORD>(val >> 48);
        }
        break;
    case ut_Real:
        {
            p2_REAL val = qvariant_cast<p2_REAL>(tv.second);
            const p2_BYTE* ptr = reinterpret_cast<const p2_BYTE *>(&val);
            result += static_cast<p2_WORD>(ptr[0] | (ptr[1] << 8));
            result += static_cast<p2_WORD>(ptr[2] | (ptr[3] << 8));
            result += static_cast<p2_WORD>(ptr[4] | (ptr[5] << 8));
            result += static_cast<p2_WORD>(ptr[6] | (ptr[7] << 8));
        }
        break;
    case ut_String:
        {
            p2_BYTES val = qvariant_cast<p2_BYTES>(tv.second);
            p2_WORD res = 0;
            int full = 0;
            foreach(const p2_BYTE b, val) {
                res |= static_cast<p2_WORD>(b) << (8*full);
                if (++full == 2) {
                    result += res;
                    full = 0;
                }
            }
            if (full)
                result += res;
        }
        break;
    }
    return result;
}

QVariant P2Union::get_words() const
{
    p2_WORDS result;
    const_iterator it = constBegin();
    while (it != constEnd()) {
        const QVariant& v = it->value<QVariant>();
        if (v.canConvert(QMetaType::type("TypedVar"))) {
            const TypedVar& tv = qvariant_cast<TypedVar>(v);
            result += to_words(tv);
        }
        it++;
    }
    return QVariant::fromValue(result);
}

p2_LONGS P2Union::to_longs(const TypedVar& tv)
{
    p2_LONGS result;
    switch (tv.first) {
    case ut_Invalid:
        break;
    case ut_Bool:
        result += qvariant_cast<bool>(tv.second);
        break;
    case ut_Byte:
        {
            p2_BYTE val = qvariant_cast<p2_BYTE>(tv.second);
            result += static_cast<p2_LONG>(val);
        }
        break;
    case ut_Word:
        {
            p2_WORD val = qvariant_cast<p2_WORD>(tv.second);
            result += val;
        }
        break;
    case ut_Addr:
    case ut_Long:
        {
            p2_LONG val = qvariant_cast<p2_LONG>(tv.second);
            result += val;
        }
        break;
    case ut_Quad:
        {
            p2_QUAD val = qvariant_cast<p2_QUAD>(tv.second);
            result += static_cast<p2_LONG>(val >> 0);
            result += static_cast<p2_LONG>(val >> 32);
        }
        break;
    case ut_Real:
        {
            p2_REAL val = qvariant_cast<p2_REAL>(tv.second);
            const p2_BYTE* ptr = reinterpret_cast<const p2_BYTE *>(&val);
            result += static_cast<p2_LONG>(
                    (static_cast<p2_LONG>(ptr[0]) <<  0) |
                    (static_cast<p2_LONG>(ptr[1]) <<  8) |
                    (static_cast<p2_LONG>(ptr[2]) << 16) |
                    (static_cast<p2_LONG>(ptr[3]) << 24)
                    );
            result += static_cast<p2_LONG>(
                    (static_cast<p2_LONG>(ptr[4]) <<  0) |
                    (static_cast<p2_LONG>(ptr[5]) <<  8) |
                    (static_cast<p2_LONG>(ptr[6]) << 16) |
                    (static_cast<p2_LONG>(ptr[7]) << 24)
                    );
        }
        break;
    case ut_String:
        {
            p2_BYTES val = qvariant_cast<p2_BYTES>(tv.second);
            p2_LONG res = 0;
            int full = 0;
            foreach(const p2_BYTE b, val) {
                res |= static_cast<p2_LONG>(b) << (8*full);
                if (++full == 4) {
                    result += res;
                    full = 0;
                }
            }
            if (full)
                result += res;
        }
        break;
    }
    return result;
}

QVariant P2Union::get_longs() const
{
    p2_LONGS result;
    const_iterator it = constBegin();
    while (it != constEnd()) {
        const QVariant& v = it->value<QVariant>();
        if (v.canConvert(QMetaType::type("TypedVar"))) {
            const TypedVar& tv = qvariant_cast<TypedVar>(v);
            result += to_longs(tv);
        }
        it++;
    }
    return QVariant::fromValue(result);
}

p2_QUADS P2Union::to_quads(const TypedVar& tv)
{
    p2_QUADS result;
    switch (tv.first) {
    case ut_Invalid:
        break;
    case ut_Bool:
        result += qvariant_cast<bool>(tv.second);
        break;
    case ut_Byte:
        {
            p2_BYTE val = qvariant_cast<p2_BYTE>(tv.second);
            result += static_cast<p2_LONG>(val);
        }
        break;
    case ut_Word:
        {
            p2_WORD val = qvariant_cast<p2_WORD>(tv.second);
            result += val;
        }
        break;
    case ut_Addr:
    case ut_Long:
        {
            p2_LONG val = qvariant_cast<p2_LONG>(tv.second);
            result += val;
        }
        break;
    case ut_Quad:
        {
            p2_QUAD val = qvariant_cast<p2_QUAD>(tv.second);
            result += val;
        }
        break;
    case ut_Real:
        {
            p2_REAL val = qvariant_cast<p2_REAL>(tv.second);
            const p2_QUAD* ptr = reinterpret_cast<const p2_QUAD *>(&val);
            result += *ptr;
        }
        break;
    case ut_String:
        {
            p2_BYTES val = qvariant_cast<p2_BYTES>(tv.second);
            p2_QUAD res = 0;
            int full = 0;
            foreach(const p2_BYTE b, val) {
                res |= static_cast<p2_QUAD>(b << (8*full));
                if (++full == 8) {
                    result += res;
                    full = 0;
                }
            }
            if (full)
                result += res;
        }
        break;
    }
    return result;
}

QVariant P2Union::get_quads() const
{
    p2_QUADS result;
    const_iterator it = constBegin();
    while (it != constEnd()) {
        const QVariant& v = it->value<QVariant>();
        if (v.canConvert(QMetaType::type("TypedVar"))) {
            const TypedVar& tv = qvariant_cast<TypedVar>(v);
            result += to_quads(tv);
        }
        it++;
    }
    return QVariant::fromValue(result);
}

QVariant P2Union::get_reals() const
{
    p2_REALS result;
    const_iterator it = constBegin();
    while (it != constEnd()) {
        const QVariant& v = it->value<QVariant>();
        if (v.canConvert(QMetaType::type("TypedVar"))) {
            const TypedVar& tv = qvariant_cast<TypedVar>(v);
            result += qvariant_cast<p2_REAL>(tv.second);
        }
    }
    return QVariant::fromValue(result);
}

void P2Union::set_bool(const QVariant& var)
{
    QVariantList::clear();
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Bool,var)));
    m_type = ut_Bool;
}

void P2Union::set_char(const QVariant& var)
{
    QVariantList::clear();
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Byte,var)));
    m_type = ut_Byte;
}

void P2Union::set_byte(const QVariant& var)
{
    QVariantList::clear();
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Byte,var)));
    m_type = ut_Byte;
}

void P2Union::set_word(const QVariant& var)
{
    QVariantList::clear();
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Word,var)));
    m_type = ut_Word;
}

void P2Union::set_addr(const QVariant& var)
{
    QVariantList::clear();
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Long,var)));
    m_type = ut_Addr;
}

void P2Union::set_long(const QVariant& var)
{
    QVariantList::clear();
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Long,var)));
    m_type = ut_Long;
}

void P2Union::set_quad(const QVariant& var)
{
    QVariantList::clear();
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Quad,var)));
    m_type = ut_Quad;
}

void P2Union::set_real(const QVariant& var)
{
    QVariantList::clear();
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Real,var)));
    m_type = ut_Real;
}

void P2Union::set_chars(const QVariant& var)
{
    QVariantList::clear();
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Byte,var)));
    m_type = ut_String;
}

void P2Union::set_bytes(const QVariant& var)
{
    QVariantList::clear();
    QVariantList::append(var.toList());
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Byte,var)));
    m_type = ut_String;
}

void P2Union::set_words(const QVariant& var)
{
    m_type = ut_Word;
    QVariantList::clear();
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Word,var)));
    m_type = ut_String;
}

void P2Union::set_longs(const QVariant& var)
{
    m_type = ut_Long;
    QVariantList::clear();
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Long,var)));
    m_type = ut_String;
}

void P2Union::set_quads(const QVariant& var)
{
    m_type = ut_Quad;
    QVariantList::clear();
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Quad,var)));
    m_type = ut_String;
}

void P2Union::set_reals(const QVariant& var)
{
    m_type = ut_Real;
    QVariantList::clear();
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Real,var)));
    m_type = ut_String;
}

void P2Union::set_array(const QVariant& var)
{
    QVariantList::clear();
    QVariantList::append(QVariant::fromValue(TypedVar(ut_String,var)));
    m_type = ut_String;
}

void P2Union::compact()
{
}

void P2Union::add_bool(const QVariant& var)
{
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Bool,var)));
}

void P2Union::add_char(const QVariant& var)
{
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Byte,var)));
}

void P2Union::add_byte(const QVariant& var)
{
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Byte,var)));
}

void P2Union::add_word(const QVariant& var)
{
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Word,var)));
}

void P2Union::add_long(const QVariant& var)
{
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Long,var)));
}

void P2Union::add_quad(const QVariant& var)
{
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Quad,var)));
}

void P2Union::add_real(const QVariant& var)
{
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Real,var)));
}

void P2Union::add_chars(const QVariant& var)
{
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Byte,var)));
}

void P2Union::add_bytes(const QVariant& var)
{
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Byte,var)));
}

void P2Union::add_words(const QVariant& var)
{
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Word,var)));
}

void P2Union::add_longs(const QVariant& var)
{
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Long,var)));
}

void P2Union::add_quads(const QVariant& var)
{
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Quad,var)));
}

void P2Union::add_reals(const QVariant& var)
{
    QVariantList::append(QVariant::fromValue(TypedVar(ut_Real,var)));
}

QString P2Union::type_name(p2_union_e type)
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
