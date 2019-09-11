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

P2Union::P2Union(p2_LONG o, p2_LONG h)
{
    set_addr(o, h);
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
    case ut_Addr:    return sz_QUAD;
    case ut_Long:    return sz_LONG;
    case ut_Quad:    return sz_QUAD;
    case ut_Real:    return sz_REAL;
    case ut_String:  return sz_BYTE;
    case ut_Invalid: return sz_BYTE;
    }
    return 1;
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
    return true;
}

int P2Union::get_int() const
{
    const P2TypedValue tv = value(0);
    return tv.value._int;
}

bool P2Union::get_bool() const
{
    const P2TypedValue tv = value(0);
    return tv.value._bool;
}

char P2Union::get_char() const
{
    const P2TypedValue tv = value(0);
    return tv.value._char;
}

p2_BYTE P2Union::get_byte() const
{
    const P2TypedValue tv = value(0);
    return tv.value._byte;
}

p2_WORD P2Union::get_word() const
{
    const P2TypedValue tv = value(0);
    return tv.value._word;
}

p2_LONG P2Union::get_long() const
{
    const P2TypedValue tv = value(0);
    return tv.value._long;
}

p2_LONG P2Union::get_addr(bool hub) const
{
    const P2TypedValue tv = value(0);
    if (ut_Addr == tv.type)
        return tv.value._addr[hub & 1];
    qDebug("get_addr(%s) for %s: %#x",
           hub ? "hub" : "cog",
           qPrintable(type_name(tv.type)), tv.value._long);
    return hub ? tv.value._long : tv.value._long * 4;
}

p2_QUAD P2Union::get_quad() const
{
    const P2TypedValue tv = value(0);
    return tv.value._quad;
}

p2_REAL P2Union::get_real() const
{
    const P2TypedValue tv = value(0);
    return tv.value._real;
}

static inline QByteArray one_byte_array(const QVariant& v)
{
    char _char = qvariant_cast<char>(v);
    return QByteArray(1, _char);
}

static inline QByteArray one_word_array(const QVariant& v)
{
    p2_WORD _word = qvariant_cast<p2_WORD>(v);
    char sz[2] = {
        static_cast<char>(_word>>0),
        static_cast<char>(_word>>8)
    };
    return QByteArray(sz, 2);
}

static inline QByteArray one_long_array(const QVariant& v)
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

static inline QByteArray one_quad_array(const QVariant& v)
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

static inline QByteArray one_real_array(const QVariant& v)
{
    union {
        p2_REAL _real;
        p2_QUAD _quad;
    }   u = {qvariant_cast<p2_REAL>(v)};
    return one_quad_array(u._quad);
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
    P2TypedValue v{ut_Long, {0}};
    v.value._int = _int;
    append(v);
}


void P2Union::add_bool(const bool _bool)
{
    P2TypedValue v{ut_Byte, {0}};
    v.value._bool = _bool;
    append(v);
}

void P2Union::add_char(const char _char)
{
    P2TypedValue v{ut_Byte, {0}};
    v.value._char = _char;
    append(v);
}

void P2Union::add_byte(const p2_BYTE _byte)
{
    P2TypedValue v{ut_Byte, {0}};
    v.value._byte = _byte;
    append(v);
}

void P2Union::add_word(const p2_WORD _word)
{
    P2TypedValue v{ut_Word, {0}};
    v.value._word = _word;
    append(v);
}

void P2Union::add_long(const p2_LONG _long)
{
    P2TypedValue v{ut_Long, {0}};
    v.value._long = _long;
    append(v);
}

void P2Union::add_addr(const p2_LONG _cog, const p2_LONG _hub)
{
    P2TypedValue v{ut_Addr, {0}};
    v.value._addr[0] = _cog;
    v.value._addr[1] = _hub;
    append(v);
}

void P2Union::add_quad(const p2_QUAD _quad)
{
    P2TypedValue v{ut_Quad, {0}};
    v.value._quad = _quad;
    append(v);
}

void P2Union::add_real(const p2_REAL _real)
{
    P2TypedValue v{ut_Real, {0}};
    v.value._real = _real;
    append(v);
}

void P2Union::add_chars(const p2_CHARS& _chars)
{
    const int pos = size();
    resize(pos + _chars.size());
    P2TypedValue v{ut_Byte, {0}};
    for (int i = 0; i < _chars.size(); i++) {
        v.value._char = _chars[i];
        replace(pos + i, v);
    }
}

void P2Union::add_bytes(const p2_BYTES& _bytes)
{
    const int pos = size();
    resize(pos + _bytes.size());
    P2TypedValue v{ut_Byte, {0}};
    for (int i = 0; i < _bytes.size(); i++) {
        v.value._byte = _bytes[i];
        replace(pos + i, v);
    }
}

void P2Union::add_words(const p2_WORDS& _words)
{
    const int pos = size();
    resize(pos + _words.size());
    P2TypedValue v{ut_Word, {0}};
    for (int i = 0; i < _words.size(); i++) {
        v.value._word = _words[i];
        replace(pos + i, v);
    }
}

void P2Union::add_longs(const p2_LONGS& _longs)
{
    const int pos = size();
    resize(pos + _longs.size());
    P2TypedValue v{ut_Long, {0}};
    for (int i = 0; i < _longs.size(); i++) {
        v.value._long = _longs[i];
        replace(pos + i, v);
    }
}

void P2Union::add_quads(const p2_QUADS& _quads)
{
    const int pos = size();
    resize(pos + _quads.size());
    P2TypedValue v{ut_Quad, {0}};
    for (int i = 0; i < _quads.size(); i++) {
        v.value._quad = _quads[i];
        replace(pos + i, v);
    }
}

void P2Union::add_reals(const p2_REALS& _reals)
{
    const int pos = size();
    resize(pos + _reals.size());
    P2TypedValue v{ut_Real, {0}};
    for (int i = 0; i < _reals.size(); i++) {
        v.value._real = _reals[i];
        replace(pos + i, v);
    }
}

void P2Union::add_array(const QByteArray& _array)
{
    const int pos = size();
    resize(pos + _array.size());
    P2TypedValue v{ut_Byte, {0}};
    for (int i = 0; i < _array.size(); i++) {
        v.value._char = _array[i];
        replace(pos + i, v);
    }
}

void P2Union::add_string(const QString& _string)
{
    const int pos = size();
    resize(pos + _string.size());
    P2TypedValue v{ut_Byte, {0}};
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
    QByteArray result;
    QVariant list = QVariant::fromValue(pun->toList());
    if (list.canConvert<QVariantList>()) {
        QSequentialIterable it = list.value<QSequentialIterable>();
        if (expand) {
            foreach(const QVariant& v, it) {
                P2TypedValue tv = qvariant_cast<P2TypedValue>(v);
                switch (tv.type) {
                case ut_Invalid:
                    Q_ASSERT(tv.type != ut_Invalid);
                    break;
                case ut_Bool:
                    result += one_byte_array(tv.value._bool);
                    break;
                case ut_Byte:
                    result += one_byte_array(tv.value._byte);
                    break;
                case ut_Word:
                    result += one_word_array(tv.value._word);
                    break;
                case ut_Addr:
                    result += one_long_array(tv.value._long);
                    break;
                case ut_Long:
                    result += one_long_array(tv.value._long);
                    break;
                case ut_Quad:
                    result += one_quad_array(tv.value._quad);
                    break;
                case ut_Real:
                    result += one_real_array(tv.value._real);
                    break;
                case ut_String:
                    result += one_byte_array(tv.value._byte);
                    break;
                }
            }
        } else {
            foreach(const QVariant& v, it) {
                P2TypedValue tv = qvariant_cast<P2TypedValue>(v);
                switch (tv.type) {
                case ut_Invalid:
                    Q_ASSERT(tv.type != ut_Invalid);
                    break;
                case ut_Bool:
                case ut_Byte:
                case ut_Word:
                case ut_Addr:
                case ut_Long:
                case ut_Quad:
                case ut_Real:
                case ut_String:
                    result += tv.value._char;
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
                P2TypedValue tv = qvariant_cast<P2TypedValue>(v);
                switch (tv.type) {
                case ut_Invalid:
                    Q_ASSERT(tv.type != ut_Invalid);
                    break;
                case ut_Bool:
                    result += one_word_array(tv.value._bool);
                    break;
                case ut_Byte:
                    result += one_word_array(tv.value._byte);
                    break;
                case ut_Word:
                    result += one_word_array(tv.value._word);
                    break;
                case ut_Addr:
                    result += one_long_array(tv.value._long);
                    break;
                case ut_Long:
                    result += one_long_array(tv.value._long);
                    break;
                case ut_Quad:
                    result += one_quad_array(tv.value._quad);
                    break;
                case ut_Real:
                    result += one_real_array(tv.value._real);
                    break;
                case ut_String:
                    result += one_word_array(tv.value._byte);
                    break;
                }
            }
        } else {
            foreach(const QVariant& v, it) {
                P2TypedValue tv = qvariant_cast<P2TypedValue>(v);
                switch (tv.type) {
                case ut_Invalid:
                    Q_ASSERT(tv.type != ut_Invalid);
                    break;
                case ut_Bool:
                case ut_Byte:
                case ut_Word:
                case ut_Addr:
                case ut_Long:
                case ut_Quad:
                case ut_Real:
                case ut_String:
                    result += one_word_array(tv.value._word);
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
                P2TypedValue tv = qvariant_cast<P2TypedValue>(v);
                switch (tv.type) {
                case ut_Invalid:
                    Q_ASSERT(tv.type != ut_Invalid);
                    break;
                case ut_Bool:
                    result += one_long_array(tv.value._bool);
                    break;
                case ut_Byte:
                    result += one_long_array(tv.value._byte);
                    break;
                case ut_Word:
                    result += one_long_array(tv.value._word);
                    break;
                case ut_Addr:
                    result += one_long_array(tv.value._long);
                    break;
                case ut_Long:
                    result += one_long_array(tv.value._long);
                    break;
                case ut_Quad:
                    result += one_quad_array(tv.value._quad);
                    break;
                case ut_Real:
                    result += one_real_array(tv.value._real);
                    break;
                case ut_String:
                    result += one_long_array(tv.value._byte);
                    break;
                }
            }
        } else {
            foreach(const QVariant& v, it) {
                P2TypedValue tv = qvariant_cast<P2TypedValue>(v);
                switch (tv.type) {
                case ut_Invalid:
                    Q_ASSERT(tv.type != ut_Invalid);
                    break;
                case ut_Bool:
                case ut_Byte:
                case ut_Word:
                case ut_Addr:
                case ut_Long:
                case ut_Quad:
                case ut_Real:
                case ut_String:
                    result += one_long_array(tv.value._long);
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
                P2TypedValue tv = qvariant_cast<P2TypedValue>(v);
                switch (tv.type) {
                case ut_Invalid:
                    Q_ASSERT(tv.type != ut_Invalid);
                    break;
                case ut_Bool:
                    result += one_quad_array(tv.value._bool);
                    break;
                case ut_Byte:
                    result += one_quad_array(tv.value._byte);
                    break;
                case ut_Word:
                    result += one_quad_array(tv.value._word);
                    break;
                case ut_Addr:
                    result += one_quad_array(tv.value._long);
                    break;
                case ut_Long:
                    result += one_quad_array(tv.value._long);
                    break;
                case ut_Quad:
                    result += one_quad_array(tv.value._quad);
                    break;
                case ut_Real:
                    result += one_real_array(tv.value._real);
                    break;
                case ut_String:
                    result += one_quad_array(tv.value._byte);
                    break;
                }
            }
        } else {
            foreach(const QVariant& v, it) {
                P2TypedValue tv = qvariant_cast<P2TypedValue>(v);
                switch (tv.type) {
                case ut_Invalid:
                    Q_ASSERT(tv.type != ut_Invalid);
                    break;
                case ut_Bool:
                case ut_Byte:
                case ut_Word:
                case ut_Addr:
                case ut_Long:
                case ut_Quad:
                case ut_Real:
                case ut_String:
                    result += one_quad_array(tv.value._long);
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

QString P2Union::str(bool with_type, p2_FORMAT_e fmt) const
{
    return str(*this, with_type, fmt);
}

QString P2Union::str(const P2Union& un, bool with_type, p2_FORMAT_e fmt)
{
    QString result;

    if (with_type)
        result = QString("<%1> ")
                 .arg(un.type_name());
    switch (un.type()) {
    case ut_Invalid:
        result = QLatin1String("<invalid>");
        break;

    case ut_Bool:
        result += QString("%1")
                  .arg(un.get_bool() ? "true" : "false");
        break;

    case ut_Byte:
        {
            p2_BYTE _byte = un.get_byte();
            switch (fmt) {
            case fmt_dec:
                result += QString("%1")
                          .arg(_byte);
                break;
            case fmt_bin:
                result += QString("%%1")
                          .arg(_byte, 8, 2, QChar('0'));
                break;
            case fmt_byt:
            case fmt_hex:
            default:
                result += QString("$%1")
                          .arg(_byte, 2, 16, QChar('0'));
            }
        }
        break;

    case ut_Word:
        {
            p2_WORD _word = un.get_word();
            switch (fmt) {
            case fmt_dec:
                result += QString("%1")
                          .arg(_word);
                break;
            case fmt_bin:
                result += QString("%%1_%2")
                          .arg(_word / 256u, 8, 2, QChar('0'))
                          .arg(_word % 256u, 8, 2, QChar('0'));
                break;
            case fmt_byt:
                result += QString("$%1 $%2")
                          .arg(_word / 256u, 2, 16, QChar('0'))
                          .arg(_word % 256u, 2, 16, QChar('0'));
                break;
            case fmt_hex:
            default:
                result += QString("$%1")
                          .arg(_word, 4, 16, QChar('0'));
            }
        }
        break;

    case ut_Addr:
        {
            p2_LONG _hub = un.get_addr(p2_hub);
            p2_LONG _cog = un.get_addr(p2_cog);
            if (_cog < HUB_ADDR0)
                _cog >>= 2;
            switch (fmt) {
            case fmt_dec:
                result += QString("%1:%2")
                          .arg(_hub)
                          .arg(_cog);
                break;
            case fmt_bin:
                result += QString("%%1_%2_%3_%4:%%5_%6_%7")
                          .arg((_hub >> 24) & 0xff, 8, 2, QChar('0'))
                          .arg((_hub >> 16) & 0xff, 8, 2, QChar('0'))
                          .arg((_hub >>  8) & 0xff, 8, 2, QChar('0'))
                          .arg((_hub >>  0) & 0xff, 8, 2, QChar('0'))
                          .arg((_cog >> 16) & 0xff, 8, 2, QChar('0'))
                          .arg((_cog >>  8) & 0xff, 8, 2, QChar('0'))
                          .arg((_cog >>  0) & 0xff, 8, 2, QChar('0'))
                          ;
                break;
            case fmt_byt:
                result += QString("$%1 $%2 $%3:$%4 $%5 $%6 $%7")
                          .arg((_hub >> 24) & 0xff, 2, 16, QChar('0'))
                          .arg((_hub >> 16) & 0xff, 2, 16, QChar('0'))
                          .arg((_hub >>  8) & 0xff, 2, 16, QChar('0'))
                          .arg((_hub >>  0) & 0xff, 2, 16, QChar('0'))
                          .arg((_cog >> 16) & 0xff, 2, 16, QChar('0'))
                          .arg((_cog >>  8) & 0xff, 2, 16, QChar('0'))
                          .arg((_cog >>  0) & 0xff, 2, 16, QChar('0'))
                          ;
                break;
            case fmt_hex:
            default:
                result += QString("$%1:$%2")
                          .arg(_hub, 5, 16, QChar('0'))
                          .arg(_cog >> 2, 3, 16, QChar('0'))
                          ;
            }
        }
        break;

    case ut_Long:
        {
            p2_LONG _long = un.get_long();
            switch (fmt) {
            case fmt_dec:
                result += QString("%1")
                          .arg(_long);
                break;
            case fmt_bin:
                result += QString("%%1_%2_%3_%4")
                          .arg((_long >> 24) & 0xff, 8, 2, QChar('0'))
                          .arg((_long >> 16) & 0xff, 8, 2, QChar('0'))
                          .arg((_long >>  8) & 0xff, 8, 2, QChar('0'))
                          .arg((_long >>  0) & 0xff, 8, 2, QChar('0'));
                break;
            case fmt_byt:
                result += QString("$%1 $%2 $%3 $%4")
                          .arg((_long >> 24) & 0xff, 2, 16, QChar('0'))
                          .arg((_long >> 16) & 0xff, 2, 16, QChar('0'))
                          .arg((_long >>  8) & 0xff, 2, 16, QChar('0'))
                          .arg((_long >>  0) & 0xff, 2, 16, QChar('0'));
                break;
            case fmt_hex:
            default:
                result += QString("$%1")
                          .arg(_long, 0, 16, QChar('0'));
            }
        }
        break;

    case ut_Quad:
        {
            p2_QUAD _quad = un.get_quad();
            switch (fmt) {
            case fmt_dec:
                result += QString("%1")
                          .arg(_quad);
                break;
            case fmt_bin:
                result += QString("%%1_%2_%3_%4_%5_%6_%7_%8")
                          .arg((_quad >> 56) & 0xff, 8, 2, QChar('0'))
                          .arg((_quad >> 48) & 0xff, 8, 2, QChar('0'))
                          .arg((_quad >> 40) & 0xff, 8, 2, QChar('0'))
                          .arg((_quad >> 32) & 0xff, 8, 2, QChar('0'))
                          .arg((_quad >> 24) & 0xff, 8, 2, QChar('0'))
                          .arg((_quad >> 16) & 0xff, 8, 2, QChar('0'))
                          .arg((_quad >>  8) & 0xff, 8, 2, QChar('0'))
                          .arg((_quad >>  0) & 0xff, 8, 2, QChar('0'));
                break;
            case fmt_byt:
                result += QString("$%1 $%2 $%3 $%4 $%5 $%6 $%7 $%8")
                          .arg((_quad >> 56) & 0xff, 2, 16, QChar('0'))
                          .arg((_quad >> 48) & 0xff, 2, 16, QChar('0'))
                          .arg((_quad >> 40) & 0xff, 2, 16, QChar('0'))
                          .arg((_quad >> 32) & 0xff, 2, 16, QChar('0'))
                          .arg((_quad >> 24) & 0xff, 2, 16, QChar('0'))
                          .arg((_quad >> 16) & 0xff, 2, 16, QChar('0'))
                          .arg((_quad >>  8) & 0xff, 2, 16, QChar('0'))
                          .arg((_quad >>  0) & 0xff, 2, 16, QChar('0'));
                break;
            case fmt_hex:
            default:
                result += QString("$%1")
                          .arg(_quad, 0, 16, QChar('0'));
            }
        }
        break;

    case ut_Real:
        result += QString("%1").arg(un.get_real(), 2, 'f');
        break;

    case ut_String:
        result += un.get_string();
        break;
    }
    return result;
}
