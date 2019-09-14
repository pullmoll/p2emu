/****************************************************************************
 *
 * Propeller2 token and instruction mnemonics class
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
#include <QString>
#include <QMultiHash>
#include <QRegularExpression>
#include "p2defs.h"
#include "p2word.h"
#include "p2tokens.h"

class P2Token
{
public:
    explicit P2Token();

    QString string(p2_TOKEN_e tok, bool lowercase = false) const;
    QString enum_name(p2_TOKEN_e tok) const;
    p2_TOKEN_e token(QStringRef& ref) const;

    bool is_type(p2_TOKEN_e tok, p2_TOKMASK_t typemask) const;
    bool is_type(p2_TOKEN_e tok, p2_TOKTYPE_e type) const;
    bool is_type(const QString& str, p2_TOKTYPE_e type) const;
    QStringList type_names(p2_TOKMASK_t typemask) const;
    QStringList type_names(p2_TOKEN_e tok) const;

    bool is_operation(p2_TOKEN_e tok) const;

    p2_TOKEN_e at_token(int& pos, const QString& str, QList<p2_TOKEN_e> tokens, p2_TOKEN_e dflt = t_invalid) const;
    p2_TOKEN_e at_token(const QString& str, QList<p2_TOKEN_e> tokens, p2_TOKEN_e dflt = t_invalid) const;

    bool is_conditional(p2_TOKEN_e tok) const;
    bool is_modcz_param(p2_TOKEN_e tok) const;

    p2_TOKEN_e at_type(int& pos, const QString& str, p2_TOKMASK_t typemask, p2_TOKEN_e dflt = t_invalid) const;
    p2_TOKEN_e at_type(int& pos, const QString& str, p2_TOKTYPE_e type, p2_TOKEN_e dflt = t_invalid) const;
    p2_TOKEN_e at_type(const QString& str, p2_TOKTYPE_e type, p2_TOKEN_e dflt = t_invalid) const;

    p2_TOKEN_e at_types(int& pos, const QString& str, p2_TOKMASK_t typemask, p2_TOKEN_e dflt = t_invalid) const;
    p2_TOKEN_e at_types(int& pos, const QString& str, const QList<p2_TOKTYPE_e>& types, p2_TOKEN_e dflt = t_invalid) const;
    p2_TOKEN_e at_types(const QString& str, const QList<p2_TOKTYPE_e>& types, p2_TOKEN_e dflt = t_invalid) const;

    p2_TOKEN_e at_conditional(int& pos, const QString& str, p2_TOKEN_e dflt = t_invalid) const;
    p2_TOKEN_e at_conditional(const QString& str, p2_TOKEN_e dflt = t_invalid) const;

    p2_TOKEN_e at_modcz_param(int& pos, const QString& str, p2_TOKEN_e dflt = t_invalid) const;
    p2_TOKEN_e at_modcz_param(const QString& str, p2_TOKEN_e dflt = t_invalid) const;

    p2_Cond_e conditional(p2_TOKEN_e cond, p2_Cond_e dflt = cc_always) const;
    p2_Cond_e conditional(const QString& str, p2_Cond_e dflt = cc_always) const;

    p2_Cond_e modcz_param(p2_TOKEN_e cond, p2_Cond_e dflt = cc_clr) const;
    p2_Cond_e modcz_param(const QString& str, p2_Cond_e dflt = cc_clr) const;

private:
    QRegularExpression m_rx;                            //!< regular expression used to find tokens
    QRegExp m_rx_comment_curly;
    QRegExp m_rx_comment_eol;
    QRegExp m_rx_bin_const;
    QRegExp m_rx_byt_const;
    QRegExp m_rx_dec_const;
    QRegExp m_rx_hex_const;
    QRegExp m_rx_str_const;
    QRegExp m_rx_real_const;
    QRegExp m_rx_locsym;
    QRegExp m_rx_symbol;
    QHash<p2_TOKEN_e, QString> m_token_enum_name;       //!< QHash for token value to enum name lookup
    QHash<p2_TOKEN_e, QString> m_token_string;          //!< QHash for token value to string lookup
    QHash<QString, p2_TOKEN_e> m_string_token;          //!< QHash for token string to value lookup
    QHash<p2_TOKEN_e, p2_TOKMASK_t> m_token_type;       //!< QHash for token value to type mask lookup
    QMultiHash<p2_TOKMASK_t, p2_TOKEN_e> m_type_token;  //!< QMultiHash for token type mask to token(s) lookup
    QHash<p2_TOKEN_e, p2_Cond_e> m_lookup_cond;         //!< QHash for conditionals to condition bits lookup
    QHash<p2_TOKEN_e, p2_Cond_e> m_lookup_modcz;        //!< QHash for MODCZ parameters to condition bits lookup
    QHash<p2_TOKTYPE_e, QString> m_t_type_name;         //!< QHash for token type mask to type name(s) lookup

    void tt_set(p2_TOKEN_e tok, p2_TOKMASK_t typemask);
    void tt_clr(p2_TOKEN_e tok, p2_TOKMASK_t typemask);
    bool tt_chk(p2_TOKEN_e tok, p2_TOKMASK_t typemask) const;
    void tn_add(p2_TOKEN_e tok, const QString& enum_name, p2_TOKMASK_t typemask, const QString& string);
};

extern P2Token Token;
