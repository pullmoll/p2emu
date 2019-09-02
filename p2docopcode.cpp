#include "p2docopcode.h"

P2DocOpcode::P2DocOpcode(P2MatchMask mm, const char* pat, const char* func)
    : m_matchmask(mm)
    , m_func(func)
    , m_pattern(pat)
    , m_brief(nullptr)
    , m_instr(nullptr)
    , m_descr()
    , m_token(t_invalid)
{

}

bool P2DocOpcode::isValid() const
{
    return m_matchmask.first != 0;
}

P2MatchMask P2DocOpcode::matchmask() const
{
    return m_matchmask;
}

const char* P2DocOpcode::func() const
{
    return m_func;
}

const QString P2DocOpcode::pattern() const
{
    return QString::fromLatin1(m_pattern);
}

const QString P2DocOpcode::brief() const
{
    return QString::fromLatin1(m_brief);
}

const QString P2DocOpcode::instr() const
{
    return QString::fromLatin1(m_instr);
}

const QStringList P2DocOpcode::descr() const
{
    QStringList descr;
    foreach(const char* line, m_descr)
        descr += QString::fromLatin1(line);
    return descr;
}

p2_token_e P2DocOpcode::token() const
{
    return m_token;
}

void P2DocOpcode::set_brief(const char* brief)
{
    m_brief = brief;
}

void P2DocOpcode::set_instr(const char* instr)
{
    m_instr = instr;
}

void P2DocOpcode::set_descr(QList<const char*> descr)
{
    m_descr = descr;
}

void P2DocOpcode::add_descr(const char* descr)
{
    m_descr += descr;
}

void P2DocOpcode::set_token(p2_token_e token)
{
    m_token = token;
}
