/****************************************************************************
 *
 * Propeller2 word for lexer results class
 *
 * Copyright (C) 2019 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ****************************************************************************/
#pragma once
#include "p2defs.h"
#include "p2tokens.h"

class P2Word
{
public:
    explicit P2Word(int lineno = 0,
                    p2_TOKEN_e tok = t_invalid,
                    const QStringRef& ref = QStringRef());

    bool isValid() const;
    const QStringRef ref() const;
    const QString str() const;
    p2_TOKEN_e tok() const;
    int lineno() const;
    int pos() const;
    int len() const;
    int end() const;

    void set_tok(p2_TOKEN_e tok);
    void set_lineno(const int lineno);

    static bool remove(QVector<P2Word>& words, p2_TOKEN_e tok);

    bool operator==(const P2Word& other) const;
    bool operator!=(const P2Word& other) const;
private:
    int m_lineno;
    QStringRef m_ref;
    p2_TOKEN_e m_tok;
};
Q_DECLARE_METATYPE(P2Word)

typedef QList<P2Word> P2Words;
Q_DECLARE_METATYPE(P2Words)

//! A QHash of (multiple) P2Word per line number
typedef QMultiHash<int,P2Word> p2_word_hash_t;
Q_DECLARE_METATYPE(p2_word_hash_t)

//! A QHash of the list of P2Word per line number
typedef QHash<int,P2Words> p2_words_hash_t;
Q_DECLARE_METATYPE(p2_words_hash_t)
