#pragma once
#include <QVariant>
#include "p2defs.h"

typedef QPair<p2_union_e,QVariant> TypedVar;
Q_DECLARE_METATYPE(TypedVar);

class P2Union;
Q_DECLARE_METATYPE(P2Union);

static constexpr int sz_BYTE = sizeof(p2_BYTE);
static constexpr int sz_WORD = sizeof(p2_WORD);
static constexpr int sz_LONG = sizeof(p2_LONG);
static constexpr int sz_QUAD = sizeof(p2_QUAD);
static constexpr int sz_REAL = sizeof(p2_REAL);

static constexpr char null_char = '\0';
static constexpr p2_BYTE null_BYTE = 0x00u;
static constexpr p2_WORD null_WORD = 0x0000u;
static constexpr p2_LONG null_LONG = 0x00000000u;
static constexpr p2_QUAD null_QUAD = Q_UINT64_C(0);
static constexpr p2_REAL null_REAL = 0.0;

class P2Union : public QVariantList
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

    template<typename T>
    T get() const
    {
        if (typeid(bool) == typeid(T)) {
            return get_bool().value<T>();
        }
        if (typeid(char) == typeid(T)) {
            return get_char().value<T>();
        }
        if (typeid(p2_BYTE) == typeid(T)) {
            return get_byte().value<T>();
        }
        if (typeid(p2_WORD) == typeid(T)) {
            return get_word().value<T>();
        }
        if (typeid(p2_LONG) == typeid(T)) {
            return get_long().value<T>();
        }
        if (typeid(p2_QUAD) == typeid(T)) {
            return get_quad().value<T>();
        }
        if (typeid(p2_REAL) == typeid(T)) {
            return get_real().value<T>();
        }
        if (typeid(p2_CHARS) == typeid(T)) {
            return get_chars().value<T>();
        }
        if (typeid(p2_BYTES) == typeid(T)) {
            return get_bytes().value<T>();
        }
        if (typeid(p2_WORDS) == typeid(T)) {
            return get_words().value<T>();
        }
        if (typeid(p2_LONGS) == typeid(T)) {
            return get_longs().value<T>();
        }
        if (typeid(p2_QUADS) == typeid(T)) {
            return get_quads().value<T>();
        }
        if (typeid(p2_REALS) == typeid(T)) {
            return get_reals().value<T>();
        }
        return qvariant_cast<T>(value(0));
    }

    template<typename T>
    void set(T val)
    {
        if (typeid(bool) == typeid(T)) {
            set_bool(val);
            return;
        }
        if (typeid(char) == typeid(T)) {
            set_char(val);
            return;
        }
        if (typeid(p2_BYTE) == typeid(T)) {
            set_byte(val);
            return;
        }
        if (typeid(p2_WORD) == typeid(T)) {
            set_word(val);
            return;
        }
        if (typeid(p2_LONG) == typeid(T)) {
            set_long(val);
            return;
        }
        if (typeid(p2_QUAD) == typeid(T)) {
            set_quad(val);
            return;
        }
        if (typeid(p2_REAL) == typeid(T)) {
            set_real(val);
            return;
        }
        if (typeid(p2_CHARS) == typeid(T)) {
            set_chars(val);
            return;
        }
        if (typeid(p2_BYTES) == typeid(T)) {
            set_bytes(val);
            return;
        }
        if (typeid(p2_WORDS) == typeid(T)) {
            set_words(val);
            return;
        }
        if (typeid(p2_LONGS) == typeid(T)) {
            set_longs(val);
            return;
        }
        if (typeid(p2_QUADS) == typeid(T)) {
            set_quads(val);
            return;
        }
        if (typeid(p2_REALS) == typeid(T)) {
            set_reals(val);
            return;
        }
        set_real(val);
    }

    template <typename T>
    void add(const T& val)
    {
        if (typeid(bool) == typeid(T)) {
            add_bool(val);
        }
        if (typeid(char) == typeid(T)) {
            add_char(val);
        }
        if (typeid(p2_BYTE) == typeid(T)) {
            add_byte(val);
        }
        if (typeid(p2_WORD) == typeid(T)) {
            add_word(val);
        }
        if (typeid(p2_LONG) == typeid(T)) {
            add_long(val);
        }
        if (typeid(p2_QUAD) == typeid(T)) {
            add_quad(val);
        }
        if (typeid(p2_REAL) == typeid(T)) {
            add_real(val);
        }
        if (typeid(p2_CHARS) == typeid(T)) {
            add_chars(val);
        }
        if (typeid(p2_BYTES) == typeid(T)) {
            add_bytes(val);
        }
        if (typeid(p2_WORDS) == typeid(T)) {
            add_words(val);
        }
        if (typeid(p2_LONGS) == typeid(T)) {
            add_longs(val);
        }
        if (typeid(p2_QUADS) == typeid(T)) {
            add_quads(val);
        }
        if (typeid(p2_REALS) == typeid(T)) {
            add_reals(val);
        }
    }

    QVariant get_bool() const;
    QVariant get_char() const;
    QVariant get_byte() const;
    QVariant get_word() const;
    QVariant get_long() const;
    QVariant get_quad() const;
    QVariant get_real() const;

    QVariant get_chars() const;
    QVariant get_bytes() const;
    QVariant get_words() const;
    QVariant get_longs() const;
    QVariant get_quads() const;
    QVariant get_reals() const;

    void set_bool(const QVariant& var);
    void set_char(const QVariant& var);
    void set_byte(const QVariant& var);
    void set_word(const QVariant& var);
    void set_addr(const QVariant& var);
    void set_long(const QVariant& var);
    void set_quad(const QVariant& var);
    void set_real(const QVariant& var);

    void set_chars(const QVariant& var);
    void set_bytes(const QVariant& var);
    void set_words(const QVariant& var);
    void set_longs(const QVariant& var);
    void set_quads(const QVariant& var);
    void set_reals(const QVariant& var);
    void set_array(const QVariant& var);

    void add_bool(const QVariant& var);
    void add_char(const QVariant& var);
    void add_byte(const QVariant& var);
    void add_word(const QVariant& var);
    void add_long(const QVariant& var);
    void add_quad(const QVariant& var);
    void add_real(const QVariant& var);

    void add_chars(const QVariant& var);
    void add_bytes(const QVariant& var);
    void add_words(const QVariant& var);
    void add_longs(const QVariant& var);
    void add_quads(const QVariant& var);
    void add_reals(const QVariant& var);

    void compact();

    static QString type_name(p2_union_e type);

private:

    int mt_TypedVar;
    p2_union_e m_type;
    static p2_CHARS to_chars(const TypedVar& tv);
    static p2_BYTES to_bytes(const TypedVar& tv);
    static p2_WORDS to_words(const TypedVar& tv);
    static p2_LONGS to_longs(const TypedVar& tv);
    static p2_QUADS to_quads(const TypedVar& tv);
};
