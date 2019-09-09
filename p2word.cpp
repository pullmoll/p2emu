#include "p2word.h"

P2Word::P2Word(p2_token_e tok, const QStringRef& ref, int lineno)
    : m_ref(ref)
    , m_tok(tok)
    , m_lineno(lineno)
{}

bool P2Word::isValid() const
{
    return t_invalid != m_tok;
}

p2_token_e P2Word::tok() const
{
    return m_tok;
}

const QStringRef P2Word::ref() const
{
    return m_ref;
}

const QString P2Word::str() const
{
    return m_ref.toString();
}

int P2Word::lineno() const
{
    return m_lineno;
}

int P2Word::pos() const
{
    return m_ref.position();
}

int P2Word::len() const
{
    return m_ref.length();
}

int P2Word::end() const
{
    return m_ref.position() + m_ref.length();
}

void P2Word::set_tok(p2_token_e tok)
{
    m_tok = tok;
}

void P2Word::set_lineno(const int line)
{
    m_lineno = line;
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

bool P2Word::operator==(const P2Word& other) const
{
    if (m_lineno != other.m_lineno)
        return false;
    if (m_ref.position() != other.m_ref.position())
        return false;
    return true;
}
