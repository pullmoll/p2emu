#include "p2union.h"

P2Union::P2Union()
    : QVector<TypedVar>()
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
    return true;
}

QVariant P2Union::get() const
{
    if (isEmpty())
        return QVariant();
    switch (m_type) {
    case ut_Invalid:
        return QVariant();
    case ut_Bool:
        return get_bool();
    case ut_Byte:
        return get_byte();
    case ut_Word:
        return get_word();
    case ut_Addr:
    case ut_Long:
        return get_long();
    case ut_Quad:
        return get_quad();
    case ut_Real:
        return get_real();
    case ut_String:
        return QVariant::fromValue(get_bytes());
    }
    return QVariant();
}

void P2Union::set(const QVariant& var)
{
    if (var.canConvert(mt_P2Union)) {
        *this = qvariant_cast<P2Union>(var);
        return;
    }
    if (var.canConvert(mt_CHARS)) {
        set_chars(qvariant_cast<p2_CHARS>(var));
        return;
    }
    if (var.canConvert(mt_BYTES)) {
        set_bytes(qvariant_cast<p2_BYTES>(var));
        return;
    }
    if (var.canConvert(mt_WORDS)) {
        set_words(qvariant_cast<p2_WORDS>(var));
        return;
    }
    if (var.canConvert(mt_QUADS)) {
        set_quads(qvariant_cast<p2_QUADS>(var));
        return;
    }
    if (var.canConvert(mt_REALS)) {
        set_reals(qvariant_cast<p2_REALS>(var));
        return;
    }
    if (var.canConvert(mt_BYTE)) {
        set_byte(qvariant_cast<p2_BYTE>(var));
        return;
    }
    if (var.canConvert(mt_WORD)) {
        set_word(qvariant_cast<p2_WORD>(var));
        return;
    }
    if (var.canConvert(mt_LONG)) {
        set_long(qvariant_cast<p2_LONG>(var));
        return;
    }
    if (var.canConvert(mt_QUAD)) {
        set_quad(qvariant_cast<p2_QUAD>(var));
        return;
    }
    if (var.canConvert(mt_REAL)) {
        set_real(qvariant_cast<p2_REAL>(var));
        return;
    }
    if (var.canConvert(QVariant::Bool)) {
        set_bool(qvariant_cast<p2_BYTE>(var));
        return;
    }
    Q_ASSERT(var.isNull());
}

void P2Union::add(const QVariant& var)
{
    if (var.canConvert(mt_CHARS)) {
        add_chars(qvariant_cast<p2_CHARS>(var));
        return;
    }
    if (var.canConvert(mt_BYTES)) {
        add_bytes(qvariant_cast<p2_BYTES>(var));
        return;
    }
    if (var.canConvert(mt_WORDS)) {
        add_words(qvariant_cast<p2_WORDS>(var));
        return;
    }
    if (var.canConvert(mt_QUADS)) {
        add_quads(qvariant_cast<p2_QUADS>(var));
        return;
    }
    if (var.canConvert(mt_REALS)) {
        add_reals(qvariant_cast<p2_REALS>(var));
        return;
    }
    if (var.canConvert(mt_BYTE)) {
        add_byte(qvariant_cast<p2_BYTE>(var));
        return;
    }
    if (var.canConvert(mt_WORD)) {
        add_word(qvariant_cast<p2_WORD>(var));
        return;
    }
    if (var.canConvert(mt_LONG)) {
        add_long(qvariant_cast<p2_LONG>(var));
        return;
    }
    if (var.canConvert(mt_QUAD)) {
        add_quad(qvariant_cast<p2_QUAD>(var));
        return;
    }
    if (var.canConvert(mt_REAL)) {
        add_real(qvariant_cast<p2_REAL>(var));
        return;
    }
    if (var.canConvert(QVariant::Bool)) {
        add_bool(qvariant_cast<p2_BYTE>(var));
        return;
    }
    Q_ASSERT(var.isNull());
}

#if 0
void P2Union::set_value(const QVariant& var)
{
    p2_union_e type = m_type;
    if (var.canConvert(mt_TypedVar)) {
        const TypedVar& tv = at(0);
        set_value(QVariant::fromValue(tv.second));
        m_type = type;
        return;
    }
    if (var.canConvert(mt_CHARS)) {
        set_chars(qvariant_cast<p2_CHARS>(var));
        m_type = type;
        return;
    }
    if (var.canConvert(mt_BYTES)) {
        set_bytes(qvariant_cast<p2_BYTES>(var));
        m_type = type;
        return;
    }
    if (var.canConvert(mt_WORDS)) {
        set_words(qvariant_cast<p2_WORDS>(var));
        m_type = type;
        return;
    }
    if (var.canConvert(mt_QUADS)) {
        set_quads(qvariant_cast<p2_QUADS>(var));
        m_type = type;
        return;
    }
    if (var.canConvert(mt_REALS)) {
        set_reals(qvariant_cast<p2_REALS>(var));
        m_type = type;
        return;
    }

    switch (type) {
    case ut_Invalid:
        set_real(qvariant_cast<p2_REAL>(var));
        type = ut_Real;
        break;
    case ut_Bool:
        set_bool(qvariant_cast<bool>(var));
        break;
    case ut_Byte:
        set_byte(qvariant_cast<p2_BYTE>(var));
        break;
    case ut_Word:
        set_word(qvariant_cast<p2_WORD>(var));
        break;
    case ut_Addr:
        set_long(qvariant_cast<p2_LONG>(var));
        break;
    case ut_Long:
        set_long(qvariant_cast<p2_LONG>(var));
        break;
    case ut_Quad:
        set_quad(qvariant_cast<p2_QUAD>(var));
        break;
    case ut_Real:
        set_real(qvariant_cast<p2_REAL>(var));
        break;
    case ut_String:
        set_byte(qvariant_cast<p2_BYTE>(var));
        break;
    }
    m_type = type;
}
#endif

bool P2Union::get_bool() const
{
    if (isEmpty())
        return false;
    const TypedVar& tv = at(0);
    return tv.second._bool;
}

char P2Union::get_char() const
{
    if (isEmpty())
        return char(0);
    if (isEmpty())
        return false;
    const TypedVar& tv = at(0);
    return tv.second._char;
}

p2_BYTE P2Union::get_byte() const
{
    if (isEmpty())
        return null_BYTE;
    const TypedVar& tv = at(0);
    return tv.second._byte;
}

p2_WORD P2Union::get_word() const
{
    if (isEmpty())
        return null_WORD;
    const TypedVar& tv = at(0);
    return tv.second._word;
}

p2_LONG P2Union::get_long() const
{
    if (isEmpty())
        return null_LONG;
    const TypedVar& tv = at(0);
    return tv.second._long;
}

p2_QUAD P2Union::get_quad() const
{
    if (isEmpty())
        return null_QUAD;
    const TypedVar& tv = at(0);
    return tv.second._quad;
}

p2_REAL P2Union::get_real() const
{
    if (isEmpty())
        return null_REAL;
    const TypedVar& tv = at(0);
    return tv.second._real;
}

static inline QByteArray byte_array(const QVariant& v)
{
    char _char = qvariant_cast<char>(v);
    return QByteArray(1, _char);
}

static inline QByteArray word_array(const QVariant& v)
{
    p2_WORD _word = qvariant_cast<p2_WORD>(v);
    char sz[2] = {
        static_cast<char>(_word>>0),
        static_cast<char>(_word>>8)
    };
    return QByteArray(sz, 2);
}

static inline QByteArray long_array(const QVariant& v)
{
    p2_LONG _long = qvariant_cast<p2_LONG>(v);
    char sz[4] = {
        static_cast<char>(_long>>0),
        static_cast<char>(_long>>8),
        static_cast<char>(_long>>16),
        static_cast<char>(_long>>24)
    };
    return QByteArray(sz, 4);
}

static inline QByteArray quad_array(const QVariant& v)
{
    p2_QUAD _quad = qvariant_cast<p2_QUAD>(v);
    char sz[8] = {
        static_cast<char>(_quad>>0),
        static_cast<char>(_quad>>8),
        static_cast<char>(_quad>>16),
        static_cast<char>(_quad>>24),
        static_cast<char>(_quad>>32),
        static_cast<char>(_quad>>40),
        static_cast<char>(_quad>>48),
        static_cast<char>(_quad>>56)
    };
    return QByteArray(sz, 8);
}

static inline QByteArray real_array(const QVariant& v)
{
    union {
        p2_REAL _real;
        p2_QUAD _quad;
    }   u = {qvariant_cast<p2_REAL>(v)};
    return quad_array(u._quad);
}

void P2Union::set_bool(const bool& var)
{
    clear();
    add_bool(var);
    m_type = ut_Bool;
}

void P2Union::set_char(const char& var)
{
    clear();
    add_char(var);
    m_type = ut_Byte;
}

void P2Union::set_byte(const p2_BYTE& var)
{
    clear();
    add_byte(var);
    m_type = ut_Byte;
}

void P2Union::set_word(const p2_WORD& var)
{
    clear();
    add_word(var);
    m_type = ut_Word;
}

void P2Union::set_addr(const p2_LONG& var)
{
    clear();
    add_long(var);
    m_type = ut_Addr;
}

void P2Union::set_long(const p2_LONG& var)
{
    clear();
    add_long(var);
    m_type = ut_Long;
}

void P2Union::set_quad(const p2_QUAD& var)
{
    clear();
    add_quad(var);
    m_type = ut_Quad;
}

void P2Union::set_real(const p2_REAL& var)
{
    clear();
    add_real(var);
    m_type = ut_Real;
}

void P2Union::set_chars(const p2_CHARS& var)
{
    clear();
    add_chars(var);
    m_type = ut_String;
}

void P2Union::set_bytes(const p2_BYTES& var)
{
    clear();
    add_bytes(var);
    m_type = ut_String;
}

void P2Union::set_words(const p2_WORDS& var)
{
    clear();
    add_words(var);
    m_type = ut_Word;
}

void P2Union::set_longs(const p2_LONGS& var)
{
    clear();
    add_longs(var);
    m_type = ut_Long;
}

void P2Union::set_quads(const p2_QUADS& var)
{
    clear();
    add_quads(var);
    m_type = ut_Quad;
}

void P2Union::set_reals(const p2_REALS& var)
{
    clear();
    add_reals(var);
    m_type = ut_Real;
}

void P2Union::set_array(const QByteArray& var)
{
    clear();
    add_array(var);
    m_type = ut_String;
}

void P2Union::set_string(const QString& var)
{
    clear();
    add_string(var);
    m_type = ut_String;
}

void P2Union::set_typed_var(const TypedVar& var)
{
    const p2_union_e type = m_type;
    set(QVariant::fromValue(var.second));
    m_type = type;
}

void P2Union::add_bool(const bool& var)
{
    TypedVar v{ut_Byte, {0}};
    v.second._bool = var;
    append(v);
}

void P2Union::add_char(const char& var)
{
    TypedVar v{ut_Byte, {0}};
    v.second._char = var;
    append(v);
}

void P2Union::add_byte(const p2_BYTE& var)
{
    TypedVar v{ut_Byte, {0}};
    v.second._byte = var;
    append(v);
}

void P2Union::add_word(const p2_WORD& var)
{
    TypedVar v{ut_Word, {0}};
    v.second._word = var;
    append(v);
}

void P2Union::add_long(const p2_LONG& var)
{
    TypedVar v{ut_Long, {0}};
    v.second._long = var;
    append(v);
}

void P2Union::add_quad(const p2_QUAD& var)
{
    TypedVar v{ut_Quad, {0}};
    v.second._quad = var;
    append(v);
}

void P2Union::add_real(const p2_REAL& var)
{
    TypedVar v{ut_Real, {0}};
    v.second._real = var;
    append(v);
}

void P2Union::add_chars(const p2_CHARS& var)
{
    const int pos = size();
    resize(pos + var.size());
    TypedVar v{ut_Byte, {0}};
    for (int i = 0; i < var.size(); i++) {
        v.second._char = var[i];
        replace(pos + i, v);
    }
}

void P2Union::add_bytes(const p2_BYTES& var)
{
    const int pos = size();
    resize(pos + var.size());
    TypedVar v{ut_Byte, {0}};
    for (int i = 0; i < var.size(); i++) {
        v.second._byte = var[i];
        replace(pos + i, v);
    }
}

void P2Union::add_words(const p2_WORDS& var)
{
    const int pos = size();
    resize(pos + var.size());
    TypedVar v{ut_Word, {0}};
    for (int i = 0; i < var.size(); i++) {
        v.second._word = var[i];
        replace(pos + i, v);
    }
}

void P2Union::add_longs(const p2_LONGS& var)
{
    const int pos = size();
    resize(pos + var.size());
    TypedVar v{ut_Long, {0}};
    for (int i = 0; i < var.size(); i++) {
        v.second._long = var[i];
        replace(pos + i, v);
    }
}

void P2Union::add_quads(const p2_QUADS& var)
{
    const int pos = size();
    resize(pos + var.size());
    TypedVar v{ut_Quad, {0}};
    for (int i = 0; i < var.size(); i++) {
        v.second._quad = var[i];
        replace(pos + i, v);
    }
}

void P2Union::add_reals(const p2_REALS& var)
{
    const int pos = size();
    resize(pos + var.size());
    TypedVar v{ut_Real, {0}};
    for (int i = 0; i < var.size(); i++) {
        v.second._real = var[i];
        replace(pos + i, v);
    }
}

void P2Union::add_array(const QByteArray& var)
{
    const int pos = size();
    resize(pos + var.size());
    TypedVar v{ut_Byte, {0}};
    for (int i = 0; i < var.size(); i++) {
        v.second._char = var[i];
        replace(pos + i, v);
    }
}

void P2Union::add_string(const QString& var)
{
    const int pos = size();
    resize(pos + var.size());
    TypedVar v{ut_Byte, {0}};
    for (int i = 0; i < var.size(); i++) {
        v.second._char = var[i].toLatin1();
        replace(pos +i, v);
    }
}

/**
 * @brief Create a QByteArray with all data contained in %pun chained together
 * @param pun const pointer to a P2Union
 * @param expand if true, expand WORD, LONG, QUAD to 2/4/8 BYTEs
 * @return QByteArray with data
 */
QByteArray P2Union::chain_bytes(const P2Union* pun, bool expand)
{
    QByteArray result;
    QVariant list = QVariant::fromValue(pun->toList());
    if (list.canConvert<QVariantList>()) {
        QSequentialIterable it = list.value<QSequentialIterable>();
        if (expand) {
            foreach(const QVariant& v, it) {
                TypedVar tv = qvariant_cast<TypedVar>(v);
                switch (tv.first) {
                case ut_Invalid:
                    Q_ASSERT(tv.first != ut_Invalid);
                    break;
                case ut_Bool:
                    result += byte_array(tv.second._bool);
                    break;
                case ut_Byte:
                    result += byte_array(tv.second._byte);
                    break;
                case ut_Word:
                    result += word_array(tv.second._word);
                    break;
                case ut_Addr:
                    result += long_array(tv.second._long);
                    break;
                case ut_Long:
                    result += long_array(tv.second._long);
                    break;
                case ut_Quad:
                    result += quad_array(tv.second._quad);
                    break;
                case ut_Real:
                    result += real_array(tv.second._real);
                    break;
                case ut_String:
                    result += byte_array(tv.second._byte);
                    break;
                }
            }
        } else {
            foreach(const QVariant& v, it) {
                TypedVar tv = qvariant_cast<TypedVar>(v);
                switch (tv.first) {
                case ut_Invalid:
                    Q_ASSERT(tv.first != ut_Invalid);
                    break;
                case ut_Bool:
                case ut_Byte:
                case ut_Word:
                case ut_Addr:
                case ut_Long:
                case ut_Quad:
                case ut_Real:
                case ut_String:
                    result += tv.second._char;
                    break;
                }
            }
        }
    } else {
        Q_ASSERT_X(false, "Cannot use QSequentialIterable", "chain_bytes");
    }
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
    QByteArray result;
    QVariant list = QVariant::fromValue(pun->toList());
    if (list.canConvert<QVariantList>()) {
        QSequentialIterable it = list.value<QSequentialIterable>();
        if (expand) {
            foreach(const QVariant& v, it) {
                TypedVar tv = qvariant_cast<TypedVar>(v);
                switch (tv.first) {
                case ut_Invalid:
                    Q_ASSERT(tv.first != ut_Invalid);
                    break;
                case ut_Bool:
                    result += word_array(tv.second._bool);
                    break;
                case ut_Byte:
                    result += word_array(tv.second._byte);
                    break;
                case ut_Word:
                    result += word_array(tv.second._word);
                    break;
                case ut_Addr:
                    result += long_array(tv.second._long);
                    break;
                case ut_Long:
                    result += long_array(tv.second._long);
                    break;
                case ut_Quad:
                    result += quad_array(tv.second._quad);
                    break;
                case ut_Real:
                    result += real_array(tv.second._real);
                    break;
                case ut_String:
                    result += word_array(tv.second._byte);
                    break;
                }
            }
        } else {
            foreach(const QVariant& v, it) {
                TypedVar tv = qvariant_cast<TypedVar>(v);
                switch (tv.first) {
                case ut_Invalid:
                    Q_ASSERT(tv.first != ut_Invalid);
                    break;
                case ut_Bool:
                case ut_Byte:
                case ut_Word:
                case ut_Addr:
                case ut_Long:
                case ut_Quad:
                case ut_Real:
                case ut_String:
                    result += word_array(tv.second._word);
                    break;
                }
            }
        }
    } else {
        Q_ASSERT_X(false, "Cannot use QSequentialIterable", "chain_words");
    }
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
    QByteArray result;
    QVariant list = QVariant::fromValue(pun->toList());
    if (list.canConvert<QVariantList>()) {
        QSequentialIterable it = list.value<QSequentialIterable>();
        if (expand) {
            foreach(const QVariant& v, it) {
                TypedVar tv = qvariant_cast<TypedVar>(v);
                switch (tv.first) {
                case ut_Invalid:
                    Q_ASSERT(tv.first != ut_Invalid);
                    break;
                case ut_Bool:
                    result += long_array(tv.second._bool);
                    break;
                case ut_Byte:
                    result += long_array(tv.second._byte);
                    break;
                case ut_Word:
                    result += long_array(tv.second._word);
                    break;
                case ut_Addr:
                    result += long_array(tv.second._long);
                    break;
                case ut_Long:
                    result += long_array(tv.second._long);
                    break;
                case ut_Quad:
                    result += quad_array(tv.second._quad);
                    break;
                case ut_Real:
                    result += real_array(tv.second._real);
                    break;
                case ut_String:
                    result += long_array(tv.second._byte);
                    break;
                }
            }
        } else {
            foreach(const QVariant& v, it) {
                TypedVar tv = qvariant_cast<TypedVar>(v);
                switch (tv.first) {
                case ut_Invalid:
                    Q_ASSERT(tv.first != ut_Invalid);
                    break;
                case ut_Bool:
                case ut_Byte:
                case ut_Word:
                case ut_Addr:
                case ut_Long:
                case ut_Quad:
                case ut_Real:
                case ut_String:
                    result += long_array(tv.second._long);
                    break;
                }
            }
        }
    } else {
        Q_ASSERT_X(false, "Cannot use QSequentialIterable", "chain_longs");
    }
    return result;
}

QByteArray P2Union::chain_quads(const P2Union* pun, bool expand)
{
    QByteArray result;
    QVariant list = QVariant::fromValue(pun->toList());
    if (list.canConvert<QVariantList>()) {
        QSequentialIterable it = list.value<QSequentialIterable>();
        if (expand) {
            foreach(const QVariant& v, it) {
                TypedVar tv = qvariant_cast<TypedVar>(v);
                switch (tv.first) {
                case ut_Invalid:
                    Q_ASSERT(tv.first != ut_Invalid);
                    break;
                case ut_Bool:
                    result += quad_array(tv.second._bool);
                    break;
                case ut_Byte:
                    result += quad_array(tv.second._byte);
                    break;
                case ut_Word:
                    result += quad_array(tv.second._word);
                    break;
                case ut_Addr:
                    result += quad_array(tv.second._long);
                    break;
                case ut_Long:
                    result += quad_array(tv.second._long);
                    break;
                case ut_Quad:
                    result += quad_array(tv.second._quad);
                    break;
                case ut_Real:
                    result += real_array(tv.second._real);
                    break;
                case ut_String:
                    result += quad_array(tv.second._byte);
                    break;
                }
            }
        } else {
            foreach(const QVariant& v, it) {
                TypedVar tv = qvariant_cast<TypedVar>(v);
                switch (tv.first) {
                case ut_Invalid:
                    Q_ASSERT(tv.first != ut_Invalid);
                    break;
                case ut_Bool:
                case ut_Byte:
                case ut_Word:
                case ut_Addr:
                case ut_Long:
                case ut_Quad:
                case ut_Real:
                case ut_String:
                    result += quad_array(tv.second._long);
                    break;
                }
            }
        }
    } else {
        Q_ASSERT_X(false, "Cannot use QSequentialIterable", "chain_quads");
    }
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
