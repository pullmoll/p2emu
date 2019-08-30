#pragma once
#include "p2defs.h"
#include "p2tokens.h"

class P2Word
{
public:
    explicit P2Word(p2_token_e tok = t_invalid,
                    const QString& str = QString(),
                    int lineno = 0, int pos = 0, int len = 0);

    const QString str() const;
    p2_token_e tok() const;
    int lineno() const;
    int pos() const;
    int len() const;
    int end() const;
    QUrl url() const;

    void set_tok(p2_token_e tok);
    void set_lineno(const int lineno);

    static QUrl url(const P2Word& word);
    static bool remove(QVector<P2Word>& words, p2_token_e tok);
    static bool merge(QVector<P2Word>& words, p2_token_e tok1, p2_token_e tok2, p2_token_e tok);

    bool operator== (const P2Word& other) const;
private:
    QString m_str;
    p2_token_e m_tok;
    int m_lineno;
    int m_pos;
    int m_len;
};

typedef QMultiHash<int,P2Word> p2_lineno_word_hash_t;
typedef QVector<P2Word> P2Words;
Q_DECLARE_METATYPE(P2Word);
Q_DECLARE_METATYPE(P2Words);
