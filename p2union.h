#pragma once
#include <QVariant>
#include "p2defs.h"

static constexpr p2_BYTE null_BYTE = 0x00u;
static constexpr p2_WORD null_WORD = 0x0000u;
static constexpr p2_LONG null_LONG = 0x00000000u;
static constexpr p2_QUAD null_QUAD = Q_UINT64_C(0);
static constexpr p2_REAL null_REAL = 0.0;

class P2Union : public QVector<P2TypedValue>
{
public:
    explicit P2Union();
    explicit P2Union(bool b);
    explicit P2Union(char c);
    explicit P2Union(int i);
    explicit P2Union(p2_BYTE b);
    explicit P2Union(p2_WORD w);
    explicit P2Union(p2_LONG l);
    explicit P2Union(p2_QUAD q);
    explicit P2Union(p2_REAL r);
    explicit P2Union(p2_LONG o, p2_LONG h);
    explicit P2Union(const QByteArray& ba);
    explicit P2Union(p2_BYTES vb);
    explicit P2Union(p2_WORDS vw);
    explicit P2Union(p2_LONGS vl);
    explicit P2Union(p2_QUADS vq);

    int unit() const;
    int usize() const;
    p2_Union_e type() const;
    QString type_name() const;
    void set_type(p2_Union_e type);
    bool is_zero() const;

    int get_int() const;
    bool get_bool() const;
    char get_char() const;
    p2_BYTE get_byte() const;
    p2_WORD get_word() const;
    p2_LONG get_long() const;
    p2_LONG get_addr(bool p2_hub) const;
    p2_QUAD get_quad() const;
    p2_REAL get_real() const;

    p2_CHARS get_chars(bool expand = false) const;
    p2_BYTES get_bytes(bool expand = false) const;
    p2_WORDS get_words(bool expand = false) const;
    p2_LONGS get_longs(bool expand = false) const;
    p2_QUADS get_quads(bool expand = false) const;
    p2_REALS get_reals(bool expand = false) const;
    QString get_string(bool expand = false) const;

    void set_int(const int var);
    void set_bool(const bool var);
    void set_char(const char var);
    void set_byte(const p2_BYTE _byte);
    void set_word(const p2_WORD _word);
    void set_addr(const p2_LONG _cog, const p2_LONG _hub);
    void set_long(const p2_LONG _long);
    void set_quad(const p2_QUAD _quad);
    void set_real(const p2_REAL _real);

    void set_chars(const p2_CHARS& _chars);
    void set_bytes(const p2_BYTES& _bytes);
    void set_words(const p2_WORDS& _words);
    void set_longs(const p2_LONGS& _longs);
    void set_quads(const p2_QUADS& _quads);
    void set_reals(const p2_REALS& _reals);
    void set_array(const QByteArray& _array);
    void set_string(const QString& _string);

    void set_typed_var(const P2TypedValue& var);

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
    void add_quads(const p2_QUADS& _quads);
    void add_reals(const p2_REALS& _reals);
    void add_array(const QByteArray& _array);
    void add_string(const QString& _string);

    void add_typed_var(const P2TypedValue& var);

    QString str(bool with_type = false, p2_FORMAT_e fmt = fmt_hex) const;

    static QString type_name(p2_Union_e type);
    static QString str(const P2Union& un, bool with_type = false, p2_FORMAT_e fmt = fmt_hex);

private:
    p2_Union_e m_type;

    static QByteArray chain_bytes(const P2Union* pun, bool expand = false);
    static QByteArray chain_words(const P2Union* pun, bool expand = false);
    static QByteArray chain_longs(const P2Union* pun, bool expand = false);
    static QByteArray chain_quads(const P2Union* pun, bool expand = false);

    static p2_CHARS get_chars(const P2TypedValue& tv, bool expand = false);
    static p2_BYTES get_bytes(const P2TypedValue& tv, bool expand = false);
    static p2_WORDS get_words(const P2TypedValue& tv, bool expand = false);
    static p2_LONGS get_longs(const P2TypedValue& tv, bool expand = false);
    static p2_QUADS get_quads(const P2TypedValue& tv, bool expand = false);
    static p2_REALS get_reals(const P2TypedValue& tv, bool expand = false);
};

Q_DECLARE_METATYPE(P2Union);
static const int mt_P2Union = qMetaTypeId<P2Union>();
