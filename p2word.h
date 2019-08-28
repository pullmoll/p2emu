#pragma once
#include "p2defs.h"
#include "p2tokens.h"

class P2Word
{
public:
    explicit P2Word(p2_token_e tok = t_invalid,
                    const QString& str = QString(),
                    int line = 0, int pos = 0, int len = 0);

    p2_token_e tok() const;
    const QString& str() const;
    int line() const;
    int pos() const;
    int len() const;
    int end() const;
    void set_tok(p2_token_e tok);
    void set_str(const QString& str);
    void set_line(const int line);
    void set_pos(const int pos);
    void set_len(const int len);
    void set(p2_token_e tok, const QString& str = QString(), const int line = -1, const int pos = -1);
    static bool merge(QVector<P2Word>& words, p2_token_e tok1, p2_token_e tok2, p2_token_e tok);

private:
    QString m_str;
    p2_token_e m_tok;
    int m_line;
    int m_pos;
    int m_len;
};

typedef QVector<P2Word> P2Words;
Q_DECLARE_METATYPE(P2Word);
Q_DECLARE_METATYPE(P2Words);
