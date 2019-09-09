#pragma once
#include <QVariant>
#include "p2defs.h"

static constexpr p2_BYTE null_BYTE = 0x00u;
static constexpr p2_WORD null_WORD = 0x0000u;
static constexpr p2_LONG null_LONG = 0x00000000u;
static constexpr p2_QUAD null_QUAD = Q_UINT64_C(0);
static constexpr p2_REAL null_REAL = 0.0;

class P2Union : public QVector<TypedVar>
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
    explicit P2Union(const QByteArray& ba);
    explicit P2Union(p2_BYTES vb);
    explicit P2Union(p2_WORDS vw);
    explicit P2Union(p2_LONGS vl);
    explicit P2Union(p2_QUADS vq);

    int unit() const;
    p2_union_e type() const;
    QString type_name() const;
    void set_type(p2_union_e type);
    bool is_zero() const;

    QVariant get() const;
    void set(const QVariant& var);
    void add(const QVariant& var);

    bool get_bool() const;
    char get_char() const;
    p2_BYTE get_byte() const;
    p2_WORD get_word() const;
    p2_LONG get_long() const;
    p2_QUAD get_quad() const;
    p2_REAL get_real() const;

    p2_CHARS get_chars(bool expand = false) const;
    p2_BYTES get_bytes(bool expand = false) const;
    p2_WORDS get_words(bool expand = false) const;
    p2_LONGS get_longs(bool expand = false) const;
    p2_QUADS get_quads(bool expand = false) const;
    p2_REALS get_reals(bool expand = false) const;
    QString get_string(bool expand = false) const;

    void set_bool(const bool& var);
    void set_char(const char& var);
    void set_byte(const p2_BYTE& var);
    void set_word(const p2_WORD& var);
    void set_addr(const p2_LONG& var);
    void set_long(const p2_LONG& var);
    void set_quad(const p2_QUAD& var);
    void set_real(const p2_REAL& var);

    void set_chars(const p2_CHARS& var);
    void set_bytes(const p2_BYTES& var);
    void set_words(const p2_WORDS& var);
    void set_longs(const p2_LONGS& var);
    void set_quads(const p2_QUADS& var);
    void set_reals(const p2_REALS& var);
    void set_array(const QByteArray& var);
    void set_string(const QString& var);

    void set_typed_var(const TypedVar& var);

    void add_bool(const bool& var);
    void add_char(const char& var);
    void add_byte(const p2_BYTE& var);
    void add_word(const p2_WORD& var);
    void add_long(const p2_LONG& var);
    void add_quad(const p2_QUAD& var);
    void add_real(const p2_REAL& var);

    void add_chars(const p2_CHARS& var);
    void add_bytes(const p2_BYTES& var);
    void add_words(const p2_WORDS& var);
    void add_longs(const p2_LONGS& var);
    void add_quads(const p2_QUADS& var);
    void add_reals(const p2_REALS& var);
    void add_array(const QByteArray& var);
    void add_string(const QString& var);

    static QString type_name(p2_union_e type);

private:
    p2_union_e m_type;

    static QByteArray chain_bytes(const P2Union* pun, bool expand = false);
    static QByteArray chain_words(const P2Union* pun, bool expand = false);
    static QByteArray chain_longs(const P2Union* pun, bool expand = false);
    static QByteArray chain_quads(const P2Union* pun, bool expand = false);

    static p2_CHARS get_chars(const TypedVar& tv, bool expand = false);
    static p2_BYTES get_bytes(const TypedVar& tv, bool expand = false);
    static p2_WORDS get_words(const TypedVar& tv, bool expand = false);
    static p2_LONGS get_longs(const TypedVar& tv, bool expand = false);
    static p2_QUADS get_quads(const TypedVar& tv, bool expand = false);
    static p2_REALS get_reals(const TypedVar& tv, bool expand = false);
};

Q_DECLARE_METATYPE(P2Union);
static const int mt_P2Union = qMetaTypeId<P2Union>();
