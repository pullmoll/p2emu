#include <QUrl>
#include "p2word.h"

P2Word::P2Word(p2_token_e tok, const QString& str, int lineno, int pos, int len)
    : m_str(str)
    , m_tok(tok)
    , m_lineno(lineno)
    , m_pos(pos)
    , m_len(len)
{}

p2_token_e P2Word::tok() const
{
    return m_tok;
}

const QString P2Word::str() const
{
    return m_str;
}

int P2Word::lineno() const
{
    return m_lineno;
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

QUrl P2Word::url() const
{
    return url(*this);
}

void P2Word::set_tok(p2_token_e tok)
{
    m_tok = tok;
}

void P2Word::set_lineno(const int line)
{
    m_lineno = line;
}

QUrl P2Word::url(const P2Word& word)
{
    QUrl url;
    url.setPath(key_tv_asm);
    url.setQuery(word.str());
    url.setFragment(QString::number(word.lineno()));
    return url;
}

bool P2Word::remove(QVector<P2Word>& words, p2_token_e tok)
{
    int count = words.count();
    for (int i = 0; i < words.count(); i++) {
        if (tok == words[i].tok()) {
            words.removeAt(i);
            --i;
        }
    }
    return count != words.count();
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
    words[idx1].m_tok = tok;
    words[idx1].m_len += words[idx2].m_len;
    words[idx1].m_str = words[idx1].m_str + words[idx2].m_str;

    // remove token at idx2
    words.removeAt(idx2);

    return true;
}

bool P2Word::operator==(const P2Word& other) const
{
    if (m_len != other.m_len)
        return false;
    if (m_pos != other.m_pos)
        return false;
    if (m_lineno != other.m_lineno)
        return false;
    if (m_tok != other.m_tok)
        return false;
    if (m_str != other.m_str)
        return false;
    return true;
}
