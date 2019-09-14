#include "p2word.h"

/**
 * @brief P2Word constructor
 * @param lineno line number where the word is defined
 * @param tok word's token value
 * @param ref word's string reference
 */
P2Word::P2Word(int lineno, p2_TOKEN_e tok, const QStringRef& ref)
    : m_lineno(lineno)
    , m_ref(ref)
    , m_tok(tok)
{}

/**
 * @brief Return true, if the word is value
 * @return true if valid, or false otherwise
 */
bool P2Word::isValid() const
{
    return t_invalid != m_tok;
}

/**
 * @brief Return the word's token
 * @return token value
 */
p2_TOKEN_e P2Word::tok() const
{
    return m_tok;
}

/**
 * @brief Return the word's string reference
 * @return string reference
 */
const QStringRef P2Word::ref() const
{
    return m_ref;
}

/**
 * @brief Return the word's string
 * @return string
 */
const QString P2Word::str() const
{
    return m_ref.toString();
}

/**
 * @brief Return the word's line number
 * @return line number
 */
int P2Word::lineno() const
{
    return m_lineno;
}

/**
 * @brief Return the word's position in the line
 * @return position of the string reference
 */
int P2Word::pos() const
{
    return m_ref.position();
}

/**
 * @brief Return the word's length
 * @return length of the string reference
 */
int P2Word::len() const
{
    return m_ref.length();
}

/**
 * @brief Return the word's end position
 * @return position after the string reference
 */
int P2Word::end() const
{
    return m_ref.position() + m_ref.length();
}

/**
 * @brief Set a new token
 * @param tok new token value
 */
void P2Word::set_tok(p2_TOKEN_e tok)
{
    m_tok = tok;
}

/**
 * @brief Set a new line number
 * @param lineno line number
 */
void P2Word::set_lineno(const int lineno)
{
    m_lineno = lineno;
}

/**
 * @brief Remove words with token %tok for a vector
 * @param words vector of words
 * @param tok token value to remove
 * @return true if any word was removed, or false otherwise
 */
bool P2Word::remove(QVector<P2Word>& words, p2_TOKEN_e tok)
{
    const int count = words.count();
    for (int i = 0; i < words.count(); i++) {
        if (tok == words[i].tok()) {
            words.removeAt(i);
            --i;
        }
    }
    return count != words.count();
}

/**
 * @brief Return true if this word is equal to another
 * @param other const reference to the other wird
 * @return true if equal, or false otherwise
 */
bool P2Word::operator==(const P2Word& other) const
{
    if (m_lineno != other.m_lineno)
        return false;
    if (m_ref.position() != other.m_ref.position())
        return false;
    if (m_ref.length() != other.m_ref.length())
        return false;
    return true;
}

bool P2Word::operator!=(const P2Word& other) const
{
    if (m_lineno != other.m_lineno)
        return true;
    if (m_ref.position() != other.m_ref.position())
        return true;
    if (m_ref.length() != other.m_ref.length())
        return true;
    return false;
}
