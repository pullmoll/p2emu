/****************************************************************************
 *
 * Propeller2 opcode documentation implementation
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
#include "p2docopcode.h"

P2DocOpcodeClass::P2DocOpcodeClass(const P2MatchMask& mm, const char* pat, const char* func)
    : m_matchmask(mm)
    , m_func(func)
    , m_pattern(pat)
    , m_brief(nullptr)
    , m_instr(nullptr)
    , m_descr()
    , m_token(t_invalid)
    , m_params()
{

}

bool P2DocOpcodeClass::isDefined() const
{
    return m_matchmask.first != 0;   // mask is non-zero?
}

p2_LONG P2DocOpcodeClass::mask() const
{
    return m_matchmask.mask();
}

p2_LONG P2DocOpcodeClass::match() const
{
    return m_matchmask.match();
}

const P2MatchMask& P2DocOpcodeClass::matchmask() const
{
    return m_matchmask;
}

const char* P2DocOpcodeClass::func() const
{
    return m_func;
}

const QString P2DocOpcodeClass::pattern() const
{
    return QString::fromLatin1(m_pattern);
}

const QString P2DocOpcodeClass::brief() const
{
    return QString::fromLatin1(m_brief);
}

const QString P2DocOpcodeClass::instr() const
{
    return QString::fromLatin1(m_instr);
}

const QStringList P2DocOpcodeClass::descr() const
{
    QStringList descr;
    foreach(const char* line, m_descr)
        descr += QString::fromLatin1(line);
    return descr;
}

p2_TOKEN_e P2DocOpcodeClass::token() const
{
    return m_token;
}

const P2DocParams& P2DocOpcodeClass::params() const
{
    return m_params;
}

void P2DocOpcodeClass::set_brief(const char* brief)
{
    m_brief = brief;
}

void P2DocOpcodeClass::set_instr(const char* instr)
{
    m_instr = instr;
}

void P2DocOpcodeClass::set_descr(QList<const char*> descr)
{
    m_descr = descr;
}

void P2DocOpcodeClass::add_descr(const char* descr)
{
    m_descr += descr;
}

void P2DocOpcodeClass::set_token(p2_TOKEN_e token)
{
    m_token = token;
}

void P2DocOpcodeClass::set_params(p2_TOKEN_e token, p2_mandatory_e mandatory)
{
    m_params = P2DocParams() = {P2DocParam(token,mandatory)};
}

void P2DocOpcodeClass::add_param(p2_TOKEN_e token, p2_mandatory_e mandatory)
{
    m_params += P2DocParam(token, mandatory);
}
