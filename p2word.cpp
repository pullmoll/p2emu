#include "p2word.h"

P2Word::P2Word(p2_token_e tok, const QString& str, int line, int pos, int len)
    : m_str(str)
    , m_tok(tok)
    , m_line(line)
    , m_pos(pos)
    , m_len(len)
{}

p2_token_e P2Word::tok() const
{
    return m_tok;
}

const QString& P2Word::str() const
{
    return m_str;
}

int P2Word::line() const
{
    return m_line;
}

int P2Word::pos() const
{
    return m_pos;
}

int P2Word::len() const
{
    return m_len;
}

int P2Word::end() const
{
    return m_pos + m_len - 1;
}

void P2Word::set_tok(p2_token_e tok)
{
    m_tok = tok;
}

void P2Word::set_str(const QString& str)
{
    m_str = str;
    m_len = str.length();
}

void P2Word::set_line(const int line)
{
    m_line = line;
}

void P2Word::set_pos(const int pos)
{
    m_pos = pos;
}

void P2Word::set_len(const int len)
{
    m_len = len;
}

void P2Word::set(p2_token_e tok, const QString& str, const int line, const int pos)
{
    if (pos >= 0)
        set_pos(pos);
    if (line >= 0)
        set_line(line);
    if (!str.isEmpty())
        set_str(str);
    set_tok(tok);
}

bool P2Word::merge(QVector<P2Word>& words, p2_token_e tok1, p2_token_e tok2, p2_token_e tok)
{
    int idx1 = -1;
    int idx2 = -1;
    for (int i = 0; i < words.count() && idx1 < 0; i++) {
        if (tok1 == words[i].tok())
            idx1 = i;
    }
    // did not find tok1?
    if (idx1 < 0)
        return false;

    for (int i = idx1 + 1; i < words.count() && idx2 < 0; i++) {
        if (tok2 == words[i].tok())
            idx2 = i;
    }

    // did not find tok2, or tok2 does not follow tok1?
    if (idx2 < 0 || idx1 + 1 != idx2)
        return false;

    // merge tokens at idx1 and idx2
    words[idx1].set(tok, words[idx1].str() + words[idx2].str());

    // remove token at idx2
    words.removeAt(idx2);

    return true;
}
