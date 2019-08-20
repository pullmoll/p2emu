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
    p2_opcode_u IR;                 //!< current instruction register
    QStringList words;              //!< current list of words on the line
    QVector<p2_token_e> tokens;     //!< current vector of tokens found on the line
    QString symbol;                 //!< currently defined symbol (first name on the line before an instruction token)
    P2AsmSymTbl symbols;            //!< symbol table
    int cnt;                        //!< count of (relevant) words
    int idx;                        //!< token (and word) index
    QString error;                  //!< error message from parameters parser
    union {
        P2BYTE b[1024*1024];        //!< as bytes
        P2WORD w[1024*1024/2];      //!< as words
        P2LONG l[1024*1024/4];      //!< as longs
    } binary;                       //!< binary data
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
    static void skip_spc(int& pos, const QString& str);
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

    static bool end_of_line(P2Params& params, bool binary = true);
    static bool optional_wcz(P2Params& params);
    static bool optional_wc(P2Params& params);
    static bool optional_wz(P2Params& params);

    static bool asm_assign(P2Params &params);
    static bool asm_org(P2Params &params);
    static bool asm_orgh(P2Params &params);

    static bool parse_with_cz(P2Params &params);
    static bool parse_with_c(P2Params &params);
    static bool parse_with_z(P2Params &params);
    static bool parse_inst(P2Params &params);
    static bool parse_d_imm_s_cz(P2Params &params);
    static bool parse_d_imm_s_c(P2Params &params);
    static bool parse_d_imm_s_z(P2Params &params);
    static bool parse_wz_d_imm_s(P2Params &params);
    static bool parse_d_imm_s_nnn(P2Params &params);
    static bool parse_d_imm_s_n(P2Params &params);
    static bool parse_d_imm_s(P2Params &params);
    static bool parse_d_cz(P2Params &params);
    static bool parse_cz(P2Params &params);
    static bool parse_cccc_zzzz_cz(P2Params &params);
    static bool parse_d(P2Params &params);
    static bool parse_wz_d(P2Params &params);
    static bool parse_imm_d(P2Params &params);
    static bool parse_imm_d_cz(P2Params &params);
    static bool parse_imm_d_c(P2Params &params);
    static bool parse_imm_s(P2Params &params);
    static bool parse_imm_s_cz(P2Params &params);
    static bool parse_pc_abs(P2Params &params);
    static bool parse_ptr_pc_abs(P2Params &params);
    static bool parse_imm23(P2Params &params, QVector<p2_inst_e> aug);

    static bool asm_byte(P2Params &params);
    static bool asm_word(P2Params &params);
    static bool asm_long(P2Params &params);
    static bool asm_res(P2Params &params);
    static bool asm_fit(P2Params &params);

    static bool asm_nop(P2Params& params);
    static bool asm_ror(P2Params& params);
    static bool asm_rol(P2Params& params);
    static bool asm_shr(P2Params& params);
    static bool asm_shl(P2Params& params);
    static bool asm_rcr(P2Params& params);
    static bool asm_rcl(P2Params& params);
    static bool asm_sar(P2Params& params);
    static bool asm_sal(P2Params& params);

    static bool asm_add(P2Params& params);
    static bool asm_addx(P2Params& params);
    static bool asm_adds(P2Params& params);
    static bool asm_addsx(P2Params& params);
    static bool asm_sub(P2Params& params);
    static bool asm_subx(P2Params& params);
    static bool asm_subs(P2Params& params);
    static bool asm_subsx(P2Params& params);

    static bool asm_cmp(P2Params& params);
    static bool asm_cmpx(P2Params& params);
    static bool asm_cmps(P2Params& params);
    static bool asm_cmpsx(P2Params& params);
    static bool asm_cmpr(P2Params& params);
    static bool asm_cmpm(P2Params& params);
    static bool asm_subr(P2Params& params);
    static bool asm_cmpsub(P2Params& params);

    static bool asm_fge(P2Params& params);
    static bool asm_fle(P2Params& params);
    static bool asm_fges(P2Params& params);
    static bool asm_fles(P2Params& params);
    static bool asm_sumc(P2Params& params);
    static bool asm_sumnc(P2Params& params);
    static bool asm_sumz(P2Params& params);
    static bool asm_sumnz(P2Params& params);

    static bool asm_testb_w(P2Params& params);
    static bool asm_testbn_w(P2Params& params);
    static bool asm_testb_and(P2Params& params);
    static bool asm_testbn_and(P2Params& params);
    static bool asm_testb_or(P2Params& params);
    static bool asm_testbn_or(P2Params& params);
    static bool asm_testb_xor(P2Params& params);
    static bool asm_testbn_xor(P2Params& params);

    static bool asm_bitl(P2Params& params);
    static bool asm_bith(P2Params& params);
    static bool asm_bitc(P2Params& params);
    static bool asm_bitnc(P2Params& params);
    static bool asm_bitz(P2Params& params);
    static bool asm_bitnz(P2Params& params);
    static bool asm_bitrnd(P2Params& params);
    static bool asm_bitnot(P2Params& params);

    static bool asm_and(P2Params& params);
    static bool asm_andn(P2Params& params);
    static bool asm_or(P2Params& params);
    static bool asm_xor(P2Params& params);
    static bool asm_muxc(P2Params& params);
    static bool asm_muxnc(P2Params& params);
    static bool asm_muxz(P2Params& params);
    static bool asm_muxnz(P2Params& params);

    static bool asm_mov(P2Params& params);
    static bool asm_not(P2Params& params);
    static bool asm_not_d(P2Params& params);
    static bool asm_abs(P2Params& params);
    static bool asm_neg(P2Params& params);
    static bool asm_negc(P2Params& params);
    static bool asm_negnc(P2Params& params);
    static bool asm_negz(P2Params& params);
    static bool asm_negnz(P2Params& params);

    static bool asm_incmod(P2Params& params);
    static bool asm_decmod(P2Params& params);
    static bool asm_zerox(P2Params& params);
    static bool asm_signx(P2Params& params);
    static bool asm_encod(P2Params& params);
    static bool asm_ones(P2Params& params);
    static bool asm_test(P2Params& params);
    static bool asm_testn(P2Params& params);

    static bool asm_setnib(P2Params& params);
    static bool asm_setnib_altsn(P2Params& params);
    static bool asm_getnib(P2Params& params);
    static bool asm_getnib_altgn(P2Params& params);
    static bool asm_rolnib(P2Params& params);
    static bool asm_rolnib_altgn(P2Params& params);
    static bool asm_setbyte(P2Params& params);
    static bool asm_setbyte_altsb(P2Params& params);
    static bool asm_getbyte(P2Params& params);
    static bool asm_getbyte_altgb(P2Params& params);
    static bool asm_rolbyte(P2Params& params);
    static bool asm_rolbyte_altgb(P2Params& params);

    static bool asm_setword(P2Params& params);
    static bool asm_setword_altsw(P2Params& params);
    static bool asm_getword(P2Params& params);
    static bool asm_getword_altgw(P2Params& params);
    static bool asm_rolword(P2Params& params);
    static bool asm_rolword_altgw(P2Params& params);

    static bool asm_altsn(P2Params& params);
    static bool asm_altsn_d(P2Params& params);
    static bool asm_altgn(P2Params& params);
    static bool asm_altgn_d(P2Params& params);
    static bool asm_altsb(P2Params& params);
    static bool asm_altsb_d(P2Params& params);
    static bool asm_altgb(P2Params& params);
    static bool asm_altgb_d(P2Params& params);

    static bool asm_altsw(P2Params& params);
    static bool asm_altsw_d(P2Params& params);
    static bool asm_altgw(P2Params& params);
    static bool asm_altgw_d(P2Params& params);

    static bool asm_altr(P2Params& params);
    static bool asm_altr_d(P2Params& params);
    static bool asm_altd(P2Params& params);
    static bool asm_altd_d(P2Params& params);
    static bool asm_alts(P2Params& params);
    static bool asm_alts_d(P2Params& params);
    static bool asm_altb(P2Params& params);
    static bool asm_altb_d(P2Params& params);
    static bool asm_alti(P2Params& params);
    static bool asm_alti_d(P2Params& params);

    static bool asm_setr(P2Params& params);
    static bool asm_setd(P2Params& params);
    static bool asm_sets(P2Params& params);
    static bool asm_decod(P2Params& params);
    static bool asm_decod_d(P2Params& params);
    static bool asm_bmask(P2Params& params);
    static bool asm_bmask_d(P2Params& params);
    static bool asm_crcbit(P2Params& params);
    static bool asm_crcnib(P2Params& params);

    static bool asm_muxnits(P2Params& params);
    static bool asm_muxnibs(P2Params& params);
    static bool asm_muxq(P2Params& params);
    static bool asm_movbyts(P2Params& params);
    static bool asm_mul(P2Params& params);
    static bool asm_muls(P2Params& params);
    static bool asm_sca(P2Params& params);
    static bool asm_scas(P2Params& params);

    static bool asm_addpix(P2Params& params);
    static bool asm_mulpix(P2Params& params);
    static bool asm_blnpix(P2Params& params);
    static bool asm_mixpix(P2Params& params);
    static bool asm_addct1(P2Params& params);
    static bool asm_addct2(P2Params& params);
    static bool asm_addct3(P2Params& params);

    static bool asm_wmlong(P2Params& params);
    static bool asm_rqpin(P2Params& params);
    static bool asm_rdpin(P2Params& params);
    static bool asm_rdlut(P2Params& params);
    static bool asm_rdbyte(P2Params& params);
    static bool asm_rdword(P2Params& params);
    static bool asm_rdlong(P2Params& params);

    static bool asm_popa(P2Params& params);
    static bool asm_popb(P2Params& params);
    static bool asm_calld(P2Params& params);
    static bool asm_resi3(P2Params& params);
    static bool asm_resi2(P2Params& params);
    static bool asm_resi1(P2Params& params);
    static bool asm_resi0(P2Params& params);
    static bool asm_reti3(P2Params& params);
    static bool asm_reti2(P2Params& params);
    static bool asm_reti1(P2Params& params);
    static bool asm_reti0(P2Params& params);
    static bool asm_callpa(P2Params& params);
    static bool asm_callpb(P2Params& params);

    static bool asm_djz(P2Params& params);
    static bool asm_djnz(P2Params& params);
    static bool asm_djf(P2Params& params);
    static bool asm_djnf(P2Params& params);
    static bool asm_ijz(P2Params& params);
    static bool asm_ijnz(P2Params& params);
    static bool asm_tjz(P2Params& params);
    static bool asm_tjnz(P2Params& params);
    static bool asm_tjf(P2Params& params);
    static bool asm_tjnf(P2Params& params);
    static bool asm_tjs(P2Params& params);
    static bool asm_tjns(P2Params& params);
    static bool asm_tjv(P2Params& params);

    static bool asm_jint(P2Params& params);
    static bool asm_jct1(P2Params& params);
    static bool asm_jct2(P2Params& params);
    static bool asm_jct3(P2Params& params);
    static bool asm_jse1(P2Params& params);
    static bool asm_jse2(P2Params& params);
    static bool asm_jse3(P2Params& params);
    static bool asm_jse4(P2Params& params);
    static bool asm_jpat(P2Params& params);
    static bool asm_jfbw(P2Params& params);
    static bool asm_jxmt(P2Params& params);
    static bool asm_jxfi(P2Params& params);
    static bool asm_jxro(P2Params& params);
    static bool asm_jxrl(P2Params& params);
    static bool asm_jatn(P2Params& params);
    static bool asm_jqmt(P2Params& params);

    static bool asm_jnint(P2Params& params);
    static bool asm_jnct1(P2Params& params);
    static bool asm_jnct2(P2Params& params);
    static bool asm_jnct3(P2Params& params);
    static bool asm_jnse1(P2Params& params);
    static bool asm_jnse2(P2Params& params);
    static bool asm_jnse3(P2Params& params);
    static bool asm_jnse4(P2Params& params);
    static bool asm_jnpat(P2Params& params);
    static bool asm_jnfbw(P2Params& params);
    static bool asm_jnxmt(P2Params& params);
    static bool asm_jnxfi(P2Params& params);
    static bool asm_jnxro(P2Params& params);
    static bool asm_jnxrl(P2Params& params);
    static bool asm_jnatn(P2Params& params);
    static bool asm_jnqmt(P2Params& params);
    static bool asm_1011110_1(P2Params& params);
    static bool asm_1011111_0(P2Params& params);
    static bool asm_setpat(P2Params& params);

    static bool asm_wrpin(P2Params& params);
    static bool asm_akpin(P2Params& params);
    static bool asm_wxpin(P2Params& params);
    static bool asm_wypin(P2Params& params);
    static bool asm_wrlut(P2Params& params);
    static bool asm_wrbyte(P2Params& params);
    static bool asm_wrword(P2Params& params);
    static bool asm_wrlong(P2Params& params);

    static bool asm_pusha(P2Params& params);
    static bool asm_pushb(P2Params& params);
    static bool asm_rdfast(P2Params& params);
    static bool asm_wrfast(P2Params& params);
    static bool asm_fblock(P2Params& params);
    static bool asm_xinit(P2Params& params);
    static bool asm_xstop(P2Params& params);
    static bool asm_xzero(P2Params& params);
    static bool asm_xcont(P2Params& params);

    static bool asm_rep(P2Params& params);
    static bool asm_coginit(P2Params& params);

    static bool asm_qmul(P2Params& params);
    static bool asm_qdiv(P2Params& params);
    static bool asm_qfrac(P2Params& params);
    static bool asm_qsqrt(P2Params& params);
    static bool asm_qrotate(P2Params& params);
    static bool asm_qvector(P2Params& params);

    static bool asm_hubset(P2Params& params);
    static bool asm_cogid(P2Params& params);
    static bool asm_cogstop(P2Params& params);
    static bool asm_locknew(P2Params& params);
    static bool asm_lockret(P2Params& params);
    static bool asm_locktry(P2Params& params);
    static bool asm_lockrel(P2Params& params);
    static bool asm_qlog(P2Params& params);
    static bool asm_qexp(P2Params& params);

    static bool asm_rfbyte(P2Params& params);
    static bool asm_rfword(P2Params& params);
    static bool asm_rflong(P2Params& params);
    static bool asm_rfvar(P2Params& params);
    static bool asm_rfvars(P2Params& params);
    static bool asm_wfbyte(P2Params& params);
    static bool asm_wfword(P2Params& params);
    static bool asm_wflong(P2Params& params);

    static bool asm_getqx(P2Params& params);
    static bool asm_getqy(P2Params& params);
    static bool asm_getct(P2Params& params);
    static bool asm_getrnd(P2Params& params);
    static bool asm_getrnd_cz(P2Params& params);

    static bool asm_setdacs(P2Params& params);
    static bool asm_setxfrq(P2Params& params);
    static bool asm_getxacc(P2Params& params);
    static bool asm_waitx(P2Params& params);
    static bool asm_setse1(P2Params& params);
    static bool asm_setse2(P2Params& params);
    static bool asm_setse3(P2Params& params);
    static bool asm_setse4(P2Params& params);
    static bool asm_pollint(P2Params& params);
    static bool asm_pollct1(P2Params& params);
    static bool asm_pollct2(P2Params& params);
    static bool asm_pollct3(P2Params& params);
    static bool asm_pollse1(P2Params& params);
    static bool asm_pollse2(P2Params& params);
    static bool asm_pollse3(P2Params& params);
    static bool asm_pollse4(P2Params& params);
    static bool asm_pollpat(P2Params& params);
    static bool asm_pollfbw(P2Params& params);
    static bool asm_pollxmt(P2Params& params);
    static bool asm_pollxfi(P2Params& params);
    static bool asm_pollxro(P2Params& params);
    static bool asm_pollxrl(P2Params& params);
    static bool asm_pollatn(P2Params& params);
    static bool asm_pollqmt(P2Params& params);

    static bool asm_waitint(P2Params& params);
    static bool asm_waitct1(P2Params& params);
    static bool asm_waitct2(P2Params& params);
    static bool asm_waitct3(P2Params& params);
    static bool asm_waitse1(P2Params& params);
    static bool asm_waitse2(P2Params& params);
    static bool asm_waitse3(P2Params& params);
    static bool asm_waitse4(P2Params& params);
    static bool asm_waitpat(P2Params& params);
    static bool asm_waitfbw(P2Params& params);
    static bool asm_waitxmt(P2Params& params);
    static bool asm_waitxfi(P2Params& params);
    static bool asm_waitxro(P2Params& params);
    static bool asm_waitxrl(P2Params& params);
    static bool asm_waitatn(P2Params& params);

    static bool asm_allowi(P2Params& params);
    static bool asm_stalli(P2Params& params);
    static bool asm_trgint1(P2Params& params);
    static bool asm_trgint2(P2Params& params);
    static bool asm_trgint3(P2Params& params);
    static bool asm_nixint1(P2Params& params);
    static bool asm_nixint2(P2Params& params);
    static bool asm_nixint3(P2Params& params);
    static bool asm_setint1(P2Params& params);
    static bool asm_setint2(P2Params& params);
    static bool asm_setint3(P2Params& params);
    static bool asm_setq(P2Params& params);
    static bool asm_setq2(P2Params& params);

    static bool asm_push(P2Params& params);
    static bool asm_pop(P2Params& params);
    static bool asm_jmp(P2Params& params);
    static bool asm_call(P2Params& params);
    static bool asm_ret(P2Params& params);
    static bool asm_calla(P2Params& params);
    static bool asm_reta(P2Params& params);
    static bool asm_callb(P2Params& params);
    static bool asm_retb(P2Params& params);

    static bool asm_jmprel(P2Params& params);
    static bool asm_skip(P2Params& params);
    static bool asm_skipf(P2Params& params);
    static bool asm_execf(P2Params& params);
    static bool asm_getptr(P2Params& params);
    static bool asm_getbrk(P2Params& params);
    static bool asm_cogbrk(P2Params& params);
    static bool asm_brk(P2Params& params);

    static bool asm_setluts(P2Params& params);
    static bool asm_setcy(P2Params& params);
    static bool asm_setci(P2Params& params);
    static bool asm_setcq(P2Params& params);
    static bool asm_setcfrq(P2Params& params);
    static bool asm_setcmod(P2Params& params);
    static bool asm_setpiv(P2Params& params);
    static bool asm_setpix(P2Params& params);
    static bool asm_cogatn(P2Params& params);

    static bool asm_testp_w(P2Params& params);
    static bool asm_testpn_w(P2Params& params);
    static bool asm_testp_and(P2Params& params);
    static bool asm_testpn_and(P2Params& params);
    static bool asm_testp_or(P2Params& params);
    static bool asm_testpn_or(P2Params& params);
    static bool asm_testp_xor(P2Params& params);
    static bool asm_testpn_xor(P2Params& params);

    static bool asm_dirl(P2Params& params);
    static bool asm_dirh(P2Params& params);
    static bool asm_dirc(P2Params& params);
    static bool asm_dirnc(P2Params& params);
    static bool asm_dirz(P2Params& params);
    static bool asm_dirnz(P2Params& params);
    static bool asm_dirrnd(P2Params& params);
    static bool asm_dirnot(P2Params& params);

    static bool asm_outl(P2Params& params);
    static bool asm_outh(P2Params& params);
    static bool asm_outc(P2Params& params);
    static bool asm_outnc(P2Params& params);
    static bool asm_outz(P2Params& params);
    static bool asm_outnz(P2Params& params);
    static bool asm_outrnd(P2Params& params);
    static bool asm_outnot(P2Params& params);

    static bool asm_fltl(P2Params& params);
    static bool asm_flth(P2Params& params);
    static bool asm_fltc(P2Params& params);
    static bool asm_fltnc(P2Params& params);
    static bool asm_fltz(P2Params& params);
    static bool asm_fltnz(P2Params& params);
    static bool asm_fltrnd(P2Params& params);
    static bool asm_fltnot(P2Params& params);

    static bool asm_drvl(P2Params& params);
    static bool asm_drvh(P2Params& params);
    static bool asm_drvc(P2Params& params);
    static bool asm_drvnc(P2Params& params);
    static bool asm_drvz(P2Params& params);
    static bool asm_drvnz(P2Params& params);
    static bool asm_drvrnd(P2Params& params);
    static bool asm_drvnot(P2Params& params);

    static bool asm_splitb(P2Params& params);
    static bool asm_mergeb(P2Params& params);
    static bool asm_splitw(P2Params& params);
    static bool asm_mergew(P2Params& params);
    static bool asm_seussf(P2Params& params);
    static bool asm_seussr(P2Params& params);
    static bool asm_rgbsqz(P2Params& params);
    static bool asm_rgbexp(P2Params& params);
    static bool asm_xoro32(P2Params& params);

    static bool asm_rev(P2Params& params);
    static bool asm_rczr(P2Params& params);
    static bool asm_rczl(P2Params& params);
    static bool asm_wrc(P2Params& params);
    static bool asm_wrnc(P2Params& params);
    static bool asm_wrz(P2Params& params);
    static bool asm_wrnz(P2Params& params);
    static bool asm_modcz(P2Params& params);
    static bool asm_setscp(P2Params& params);
    static bool asm_getscp(P2Params& params);

    static bool asm_jmp_abs(P2Params& params);
    static bool asm_call_abs(P2Params& params);
    static bool asm_calla_abs(P2Params& params);
    static bool asm_callb_abs(P2Params& params);
    static bool asm_calld_pa_abs(P2Params& params);
    static bool asm_calld_pb_abs(P2Params& params);
    static bool asm_calld_ptra_abs(P2Params& params);
    static bool asm_calld_ptrb_abs(P2Params& params);
    static bool asm_loc(P2Params& params);
    static bool asm_loc_pa(P2Params& params);
    static bool asm_loc_pb(P2Params& params);
    static bool asm_loc_ptra(P2Params& params);
    static bool asm_loc_ptrb(P2Params& params);

    static bool asm_augs(P2Params& params);
    static bool asm_augd(P2Params& params);
};
