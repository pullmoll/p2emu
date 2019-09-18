/****************************************************************************
 *
 * Propeller2 opcode documentation class
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
#include <QSharedPointer>
#include "p2token.h"

typedef enum {
    p2_optional,
    p2_mandatory,
    p2_alternative_1,
    p2_alternative_2
}   p2_mandatory_e;

typedef QPair<p2_TOKEN_e,bool> P2DocParam;
typedef QList<P2DocParam> P2DocParams;

class P2DocOpcodeClass
{
public:
    explicit P2DocOpcodeClass(const P2MatchMask &mm = P2MatchMask(), const char* pat = nullptr, const char* _func = nullptr);

    bool isDefined() const;
    p2_LONG mask() const;
    p2_LONG match() const;
    const P2MatchMask& matchmask() const;
    const char* func() const;
    const QString pattern() const;
    const QString brief() const;
    const QString instr() const;
    const QStringList descr() const;
    p2_TOKEN_e token() const;
    const P2DocParams& params() const;

    void set_brief(const char* brief = nullptr);
    void set_instr(const char* instr = nullptr);
    void set_descr(QList<const char*> descr = QList<const char*>());
    void add_descr(const char* descr = nullptr);
    void set_token(p2_TOKEN_e token);
    void set_params(p2_TOKEN_e token, p2_mandatory_e mandatory = p2_mandatory);
    void add_param(p2_TOKEN_e token, p2_mandatory_e mandatory = p2_mandatory);

private:
    P2MatchMask m_matchmask;            //!< mask and match pair
    const char * m_func;                //!< name of the fuction which created this
    const char * m_pattern;             //!< opcode pattern
    const char * m_brief;               //!< opcode brief description
    const char * m_instr;               //!< opcode instruction example
    QList<const char *> m_descr;        //!< opcode description
    p2_TOKEN_e m_token;                 //!< opcode instruction token
    P2DocParams m_params;               //!< opcode parameters (may be optional)
};

typedef QSharedPointer<P2DocOpcodeClass> P2DocOpcode;

