/****************************************************************************
 *
 * Propeller2 assembler
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
#include <QObject>
#include <QString>
#include <QVariant>
#include <QHash>
#include "p2defs.h"
#include "p2token.h"
#include "p2asmsymtbl.h"

/**
 * @brief The P2Params class is used to pass the current state
 * of the assembler to parameter parsing functions
 */
class P2Params
{
public:
    P2Params()
        : lineno(0)
        , line()
        , next_pc(0)
        , curr_pc(0)
        , last_pc(0)
        , IR({0,})
        , words()
        , tokens()
        , symbols()
        , cnt(0)
        , idx(0)
        , binary()
    {}

    int lineno;                     //!< current line number
    QString line;                   //!< current line
    P2LONG next_pc;                 //!< next program counter
    P2LONG curr_pc;                 //!< current program counter (origin of the instruction)
    P2LONG last_pc;                 //!< last program counter (maximum of next_pc)
    p2_opword_t IR;                 //!< current instruction register
    QStringList words;              //!< current list of words on the line
    QVector<p2_token_e> tokens;     //!< current vector of tokens found on the line
    P2AsmSymTbl symbols;            //!< symbol table
    int cnt;                        //!< count of (relevant) words
    int idx;                        //!< token (and word) index
    QString error;                  //!< error message from parameters parser
    QByteArray binary;              //!< binary data
};

/**
 * @brief The P2Asm class implements an Propeller2 assembler
 */
class P2Asm : public QObject
{
    Q_OBJECT

public:
    explicit P2Asm(QObject *parent = nullptr);
    ~P2Asm();

    bool assemble(P2Params& params, const QStringList& source);
    bool assemble(P2Params& params, const QString& filename);

signals:
    void Error(int lineno, QString message);

private:
    enum imm_to_e {
        immediate_none,
        immediate_imm,
        immediate_wz,
        immediate_wc
    };

    static bool split_and_tokenize(P2Params& params, const QString& line);
    static p2_cond_e conditional(P2Params& params, p2_token_e cond);
    static quint64 from_bin(int& pos, const QString& str, const QString& stop);
    static quint64 from_oct(int& pos, const QString& str, const QString& stop);
    static quint64 from_dec(int& pos, const QString& str, const QString& stop);
    static quint64 from_hex(int& pos, const QString& str, const QString& stop);
    static quint64 from_str(int& pos, const QString& str, const QString& stop);
    static QVariant parse_atom(P2Params& params, int& pos, const QString& word, p2_token_e tok);
    static QVariant parse_factors(P2Params& params, int& pos, const QString& str, p2_token_e tok);
    static QVariant parse_summands(P2Params& params, int& pos, const QString& str, p2_token_e tok);
    static QVariant parse_binops(P2Params& params, int& pos, const QString& str, p2_token_e tok);
    static QVariant expression(P2Params& params, imm_to_e imm_to = immediate_none);

    static bool end_of_line(P2Params& params);
    static bool optional_wcz(P2Params& params);
    static bool optional_wc(P2Params& params);
    static bool optional_wz(P2Params& params);

    static bool params_assign(P2Params &params, P2AsmSymbol& sym);
    static bool params_org(P2Params &params, P2AsmSymbol& sym);
    static bool params_orgh(P2Params &params, P2AsmSymbol& sym);

    static bool params_with_cz(P2Params &params);
    static bool params_with_c(P2Params &params);
    static bool params_with_z(P2Params &params);
    static bool params_inst(P2Params &params);
    static bool params_d_imm_s_cz(P2Params &params);
    static bool params_d_imm_s_c(P2Params &params);
    static bool params_d_imm_s_z(P2Params &params);
    static bool params_wz_d_imm_s(P2Params &params);
    static bool params_d_imm_s_nnn(P2Params &params);
    static bool params_d_imm_s_n(P2Params &params);
    static bool params_d_imm_s(P2Params &params);
    static bool params_d_cz(P2Params &params);
    static bool params_cz(P2Params &params);
    static bool params_cccc_zzzz_cz(P2Params &params);
    static bool params_d(P2Params &params);
    static bool params_wz_d(P2Params &params);
    static bool params_imm_d(P2Params &params);
    static bool params_imm_d_cz(P2Params &params);
    static bool params_imm_d_c(P2Params &params);
    static bool params_imm_s(P2Params &params);
    static bool params_pc_abs(P2Params &params);
    static bool params_ptr_pc_abs(P2Params &params);
    static bool params_imm23(P2Params &params);

    static bool params_byte(P2Params &params);
    static bool params_word(P2Params &params);
    static bool params_long(P2Params &params);
    static bool params_res(P2Params &params);
};
