/****************************************************************************
 *
 * Propeller2 disassembler
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
#include "p2token.h"

class P2Cog;

class P2Dasm : public QObject
{
    Q_OBJECT
public:
    P2Dasm(const P2Cog* cog, QObject* parent = nullptr);
    ~P2Dasm();

    bool dasm(p2_LONG addr, QString& opcode, QString& instruction, QString* description = nullptr);
    p2_LONG memsize() const;

public slots:
    void setLowercase(bool flag);

private:
    const P2Cog* COG;
    P2Token* Token;
    int pad_opcode;
    int pad_inst;
    int pad_wcz;
    p2_opword_t IR;
    quint32 PC;
    quint32 S;
    quint32 D;

    p2_token_e conditional(p2_cond_e cond);
    p2_token_e conditional(unsigned cond);

    QString format_num(uint num, bool binary = false);
    QString format_bin(uint num, int digits = 0);
    void format_inst(QString& instruction, p2_token_e inst);
    void format_d_imm_s_cz(QString& instruction, p2_token_e inst, p2_token_e with = t_W);
    void format_d_imm_s_c(QString& instruction, p2_token_e inst, p2_token_e with = t_W);
    void format_d_imm_s_z(QString& instruction, p2_token_e inst, p2_token_e with = t_W);
    void format_wz_d_imm_s(QString& instruction, p2_token_e inst);
    void format_d_imm_s_nnn(QString& instruction, p2_token_e inst);
    void format_d_imm_s_n(QString& instruction, p2_token_e inst);
    void format_d_imm_s(QString& instruction, p2_token_e inst);
    void format_d_cz(QString& instruction, p2_token_e inst, p2_token_e with = t_W);
    void format_cz(QString& instruction, p2_token_e inst, p2_token_e with = t_W);
    void format_cz_cz(QString& instruction, p2_token_e inst, p2_token_e with = t_W);
    void format_d(QString& instruction, p2_token_e inst);
    void format_wz_d(QString& instruction, p2_token_e inst);
    void format_imm_d(QString& instruction, p2_token_e inst);
    void format_imm_d_cz(QString& instruction, p2_token_e inst, p2_token_e with = t_W);
    void format_imm_d_c(QString& instruction, p2_token_e inst);
    void format_imm_s(QString& instruction, p2_token_e inst);
    void format_pc_abs(QString& instruction, p2_token_e inst, p2_token_e dest = t_nothing);
    void format_imm23(QString& instruction, p2_token_e inst);

    void dasm_nop(QString& instruction, QString* description = nullptr);
    void dasm_ror(QString& instruction, QString* description = nullptr);
    void dasm_rol(QString& instruction, QString* description = nullptr);
    void dasm_shr(QString& instruction, QString* description = nullptr);
    void dasm_shl(QString& instruction, QString* description = nullptr);
    void dasm_rcr(QString& instruction, QString* description = nullptr);
    void dasm_rcl(QString& instruction, QString* description = nullptr);
    void dasm_sar(QString& instruction, QString* description = nullptr);
    void dasm_sal(QString& instruction, QString* description = nullptr);

    void dasm_add(QString& instruction, QString* description = nullptr);
    void dasm_addx(QString& instruction, QString* description = nullptr);
    void dasm_adds(QString& instruction, QString* description = nullptr);
    void dasm_addsx(QString& instruction, QString* description = nullptr);
    void dasm_sub(QString& instruction, QString* description = nullptr);
    void dasm_subx(QString& instruction, QString* description = nullptr);
    void dasm_subs(QString& instruction, QString* description = nullptr);
    void dasm_subsx(QString& instruction, QString* description = nullptr);

    void dasm_cmp(QString& instruction, QString* description = nullptr);
    void dasm_cmpx(QString& instruction, QString* description = nullptr);
    void dasm_cmps(QString& instruction, QString* description = nullptr);
    void dasm_cmpsx(QString& instruction, QString* description = nullptr);
    void dasm_cmpr(QString& instruction, QString* description = nullptr);
    void dasm_cmpm(QString& instruction, QString* description = nullptr);
    void dasm_subr(QString& instruction, QString* description = nullptr);
    void dasm_cmpsub(QString& instruction, QString* description = nullptr);

    void dasm_fge(QString& instruction, QString* description = nullptr);
    void dasm_fle(QString& instruction, QString* description = nullptr);
    void dasm_fges(QString& instruction, QString* description = nullptr);
    void dasm_fles(QString& instruction, QString* description = nullptr);
    void dasm_sumc(QString& instruction, QString* description = nullptr);
    void dasm_sumnc(QString& instruction, QString* description = nullptr);
    void dasm_sumz(QString& instruction, QString* description = nullptr);
    void dasm_sumnz(QString& instruction, QString* description = nullptr);

    void dasm_testb_w(QString& instruction, QString* description = nullptr);
    void dasm_testbn_w(QString& instruction, QString* description = nullptr);
    void dasm_testb_and(QString& instruction, QString* description = nullptr);
    void dasm_testbn_and(QString& instruction, QString* description = nullptr);
    void dasm_testb_or(QString& instruction, QString* description = nullptr);
    void dasm_testbn_or(QString& instruction, QString* description = nullptr);
    void dasm_testb_xor(QString& instruction, QString* description = nullptr);
    void dasm_testbn_xor(QString& instruction, QString* description = nullptr);

    void dasm_bitl(QString& instruction, QString* description = nullptr);
    void dasm_bith(QString& instruction, QString* description = nullptr);
    void dasm_bitc(QString& instruction, QString* description = nullptr);
    void dasm_bitnc(QString& instruction, QString* description = nullptr);
    void dasm_bitz(QString& instruction, QString* description = nullptr);
    void dasm_bitnz(QString& instruction, QString* description = nullptr);
    void dasm_bitrnd(QString& instruction, QString* description = nullptr);
    void dasm_bitnot(QString& instruction, QString* description = nullptr);

    void dasm_and(QString& instruction, QString* description = nullptr);
    void dasm_andn(QString& instruction, QString* description = nullptr);
    void dasm_or(QString& instruction, QString* description = nullptr);
    void dasm_xor(QString& instruction, QString* description = nullptr);
    void dasm_muxc(QString& instruction, QString* description = nullptr);
    void dasm_muxnc(QString& instruction, QString* description = nullptr);
    void dasm_muxz(QString& instruction, QString* description = nullptr);
    void dasm_muxnz(QString& instruction, QString* description = nullptr);

    void dasm_mov(QString& instruction, QString* description = nullptr);
    void dasm_not(QString& instruction, QString* description = nullptr);
    void dasm_not_d(QString& instruction, QString* description = nullptr);
    void dasm_abs(QString& instruction, QString* description = nullptr);
    void dasm_neg(QString& instruction, QString* description = nullptr);
    void dasm_negc(QString& instruction, QString* description = nullptr);
    void dasm_negnc(QString& instruction, QString* description = nullptr);
    void dasm_negz(QString& instruction, QString* description = nullptr);
    void dasm_negnz(QString& instruction, QString* description = nullptr);

    void dasm_incmod(QString& instruction, QString* description = nullptr);
    void dasm_decmod(QString& instruction, QString* description = nullptr);
    void dasm_zerox(QString& instruction, QString* description = nullptr);
    void dasm_signx(QString& instruction, QString* description = nullptr);
    void dasm_encod(QString& instruction, QString* description = nullptr);
    void dasm_ones(QString& instruction, QString* description = nullptr);
    void dasm_test(QString& instruction, QString* description = nullptr);
    void dasm_testn(QString& instruction, QString* description = nullptr);

    void dasm_setnib(QString& instruction, QString* description = nullptr);
    void dasm_setnib_altsn(QString& instruction, QString* description = nullptr);
    void dasm_getnib(QString& instruction, QString* description = nullptr);
    void dasm_getnib_altgn(QString& instruction, QString* description = nullptr);
    void dasm_rolnib(QString& instruction, QString* description = nullptr);
    void dasm_rolnib_altgn(QString& instruction, QString* description = nullptr);
    void dasm_setbyte(QString& instruction, QString* description = nullptr);
    void dasm_setbyte_altsb(QString& instruction, QString* description = nullptr);
    void dasm_getbyte(QString& instruction, QString* description = nullptr);
    void dasm_getbyte_altgb(QString& instruction, QString* description = nullptr);
    void dasm_rolbyte(QString& instruction, QString* description = nullptr);
    void dasm_rolbyte_altgb(QString& instruction, QString* description = nullptr);

    void dasm_setword(QString& instruction, QString* description = nullptr);
    void dasm_setword_altsw(QString& instruction, QString* description = nullptr);
    void dasm_getword(QString& instruction, QString* description = nullptr);
    void dasm_getword_altgw(QString& instruction, QString* description = nullptr);
    void dasm_rolword(QString& instruction, QString* description = nullptr);
    void dasm_rolword_altgw(QString& instruction, QString* description = nullptr);

    void dasm_altsn(QString& instruction, QString* description = nullptr);
    void dasm_altsn_d(QString& instruction, QString* description = nullptr);
    void dasm_altgn(QString& instruction, QString* description = nullptr);
    void dasm_altgn_d(QString& instruction, QString* description = nullptr);
    void dasm_altsb(QString& instruction, QString* description = nullptr);
    void dasm_altsb_d(QString& instruction, QString* description = nullptr);
    void dasm_altgb(QString& instruction, QString* description = nullptr);
    void dasm_altgb_d(QString& instruction, QString* description = nullptr);

    void dasm_altsw(QString& instruction, QString* description = nullptr);
    void dasm_altsw_d(QString& instruction, QString* description = nullptr);
    void dasm_altgw(QString& instruction, QString* description = nullptr);
    void dasm_altgw_d(QString& instruction, QString* description = nullptr);

    void dasm_altr(QString& instruction, QString* description = nullptr);
    void dasm_altr_d(QString& instruction, QString* description = nullptr);
    void dasm_altd(QString& instruction, QString* description = nullptr);
    void dasm_altd_d(QString& instruction, QString* description = nullptr);
    void dasm_alts(QString& instruction, QString* description = nullptr);
    void dasm_alts_d(QString& instruction, QString* description = nullptr);
    void dasm_altb(QString& instruction, QString* description = nullptr);
    void dasm_altb_d(QString& instruction, QString* description = nullptr);
    void dasm_alti(QString& instruction, QString* description = nullptr);
    void dasm_alti_d(QString& instruction, QString* description = nullptr);

    void dasm_setr(QString& instruction, QString* description = nullptr);
    void dasm_setd(QString& instruction, QString* description = nullptr);
    void dasm_sets(QString& instruction, QString* description = nullptr);
    void dasm_decod(QString& instruction, QString* description = nullptr);
    void dasm_decod_d(QString& instruction, QString* description = nullptr);
    void dasm_bmask(QString& instruction, QString* description = nullptr);
    void dasm_bmask_d(QString& instruction, QString* description = nullptr);
    void dasm_crcbit(QString& instruction, QString* description = nullptr);
    void dasm_crcnib(QString& instruction, QString* description = nullptr);

    void dasm_muxnits(QString& instruction, QString* description = nullptr);
    void dasm_muxnibs(QString& instruction, QString* description = nullptr);
    void dasm_muxq(QString& instruction, QString* description = nullptr);
    void dasm_movbyts(QString& instruction, QString* description = nullptr);
    void dasm_mul(QString& instruction, QString* description = nullptr);
    void dasm_muls(QString& instruction, QString* description = nullptr);
    void dasm_sca(QString& instruction, QString* description = nullptr);
    void dasm_scas(QString& instruction, QString* description = nullptr);

    void dasm_addpix(QString& instruction, QString* description = nullptr);
    void dasm_mulpix(QString& instruction, QString* description = nullptr);
    void dasm_blnpix(QString& instruction, QString* description = nullptr);
    void dasm_mixpix(QString& instruction, QString* description = nullptr);
    void dasm_addct1(QString& instruction, QString* description = nullptr);
    void dasm_addct2(QString& instruction, QString* description = nullptr);
    void dasm_addct3(QString& instruction, QString* description = nullptr);

    void dasm_wmlong(QString& instruction, QString* description = nullptr);
    void dasm_rqpin(QString& instruction, QString* description = nullptr);
    void dasm_rdpin(QString& instruction, QString* description = nullptr);
    void dasm_rdlut(QString& instruction, QString* description = nullptr);
    void dasm_rdbyte(QString& instruction, QString* description = nullptr);
    void dasm_rdword(QString& instruction, QString* description = nullptr);
    void dasm_rdlong(QString& instruction, QString* description = nullptr);

    void dasm_popa(QString& instruction, QString* description = nullptr);
    void dasm_popb(QString& instruction, QString* description = nullptr);
    void dasm_calld(QString& instruction, QString* description = nullptr);
    void dasm_resi3(QString& instruction, QString* description = nullptr);
    void dasm_resi2(QString& instruction, QString* description = nullptr);
    void dasm_resi1(QString& instruction, QString* description = nullptr);
    void dasm_resi0(QString& instruction, QString* description = nullptr);
    void dasm_reti3(QString& instruction, QString* description = nullptr);
    void dasm_reti2(QString& instruction, QString* description = nullptr);
    void dasm_reti1(QString& instruction, QString* description = nullptr);
    void dasm_reti0(QString& instruction, QString* description = nullptr);
    void dasm_callpa(QString& instruction, QString* description = nullptr);
    void dasm_callpb(QString& instruction, QString* description = nullptr);

    void dasm_djz(QString& instruction, QString* description = nullptr);
    void dasm_djnz(QString& instruction, QString* description = nullptr);
    void dasm_djf(QString& instruction, QString* description = nullptr);
    void dasm_djnf(QString& instruction, QString* description = nullptr);
    void dasm_ijz(QString& instruction, QString* description = nullptr);
    void dasm_ijnz(QString& instruction, QString* description = nullptr);
    void dasm_tjz(QString& instruction, QString* description = nullptr);
    void dasm_tjnz(QString& instruction, QString* description = nullptr);
    void dasm_tjf(QString& instruction, QString* description = nullptr);
    void dasm_tjnf(QString& instruction, QString* description = nullptr);
    void dasm_tjs(QString& instruction, QString* description = nullptr);
    void dasm_tjns(QString& instruction, QString* description = nullptr);
    void dasm_tjv(QString& instruction, QString* description = nullptr);

    void dasm_jint(QString& instruction, QString* description = nullptr);
    void dasm_jct1(QString& instruction, QString* description = nullptr);
    void dasm_jct2(QString& instruction, QString* description = nullptr);
    void dasm_jct3(QString& instruction, QString* description = nullptr);
    void dasm_jse1(QString& instruction, QString* description = nullptr);
    void dasm_jse2(QString& instruction, QString* description = nullptr);
    void dasm_jse3(QString& instruction, QString* description = nullptr);
    void dasm_jse4(QString& instruction, QString* description = nullptr);
    void dasm_jpat(QString& instruction, QString* description = nullptr);
    void dasm_jfbw(QString& instruction, QString* description = nullptr);
    void dasm_jxmt(QString& instruction, QString* description = nullptr);
    void dasm_jxfi(QString& instruction, QString* description = nullptr);
    void dasm_jxro(QString& instruction, QString* description = nullptr);
    void dasm_jxrl(QString& instruction, QString* description = nullptr);
    void dasm_jatn(QString& instruction, QString* description = nullptr);
    void dasm_jqmt(QString& instruction, QString* description = nullptr);

    void dasm_jnint(QString& instruction, QString* description = nullptr);
    void dasm_jnct1(QString& instruction, QString* description = nullptr);
    void dasm_jnct2(QString& instruction, QString* description = nullptr);
    void dasm_jnct3(QString& instruction, QString* description = nullptr);
    void dasm_jnse1(QString& instruction, QString* description = nullptr);
    void dasm_jnse2(QString& instruction, QString* description = nullptr);
    void dasm_jnse3(QString& instruction, QString* description = nullptr);
    void dasm_jnse4(QString& instruction, QString* description = nullptr);
    void dasm_jnpat(QString& instruction, QString* description = nullptr);
    void dasm_jnfbw(QString& instruction, QString* description = nullptr);
    void dasm_jnxmt(QString& instruction, QString* description = nullptr);
    void dasm_jnxfi(QString& instruction, QString* description = nullptr);
    void dasm_jnxro(QString& instruction, QString* description = nullptr);
    void dasm_jnxrl(QString& instruction, QString* description = nullptr);
    void dasm_jnatn(QString& instruction, QString* description = nullptr);
    void dasm_jnqmt(QString& instruction, QString* description = nullptr);
    void dasm_1011110_1(QString& instruction, QString* description = nullptr);
    void dasm_1011111_0(QString& instruction, QString* description = nullptr);
    void dasm_setpat(QString& instruction, QString* description = nullptr);

    void dasm_wrpin(QString& instruction, QString* description = nullptr);
    void dasm_akpin(QString& instruction, QString* description = nullptr);
    void dasm_wxpin(QString& instruction, QString* description = nullptr);
    void dasm_wypin(QString& instruction, QString* description = nullptr);
    void dasm_wrlut(QString& instruction, QString* description = nullptr);
    void dasm_wrbyte(QString& instruction, QString* description = nullptr);
    void dasm_wrword(QString& instruction, QString* description = nullptr);
    void dasm_wrlong(QString& instruction, QString* description = nullptr);

    void dasm_pusha(QString& instruction, QString* description = nullptr);
    void dasm_pushb(QString& instruction, QString* description = nullptr);
    void dasm_rdfast(QString& instruction, QString* description = nullptr);
    void dasm_wrfast(QString& instruction, QString* description = nullptr);
    void dasm_fblock(QString& instruction, QString* description = nullptr);
    void dasm_xinit(QString& instruction, QString* description = nullptr);
    void dasm_xstop(QString& instruction, QString* description = nullptr);
    void dasm_xzero(QString& instruction, QString* description = nullptr);
    void dasm_xcont(QString& instruction, QString* description = nullptr);

    void dasm_rep(QString& instruction, QString* description = nullptr);
    void dasm_coginit(QString& instruction, QString* description = nullptr);

    void dasm_qmul(QString& instruction, QString* description = nullptr);
    void dasm_qdiv(QString& instruction, QString* description = nullptr);
    void dasm_qfrac(QString& instruction, QString* description = nullptr);
    void dasm_qsqrt(QString& instruction, QString* description = nullptr);
    void dasm_qrotate(QString& instruction, QString* description = nullptr);
    void dasm_qvector(QString& instruction, QString* description = nullptr);

    void dasm_hubset(QString& instruction, QString* description = nullptr);
    void dasm_cogid(QString& instruction, QString* description = nullptr);
    void dasm_cogstop(QString& instruction, QString* description = nullptr);
    void dasm_locknew(QString& instruction, QString* description = nullptr);
    void dasm_lockret(QString& instruction, QString* description = nullptr);
    void dasm_locktry(QString& instruction, QString* description = nullptr);
    void dasm_lockrel(QString& instruction, QString* description = nullptr);
    void dasm_qlog(QString& instruction, QString* description = nullptr);
    void dasm_qexp(QString& instruction, QString* description = nullptr);

    void dasm_rfbyte(QString& instruction, QString* description = nullptr);
    void dasm_rfword(QString& instruction, QString* description = nullptr);
    void dasm_rflong(QString& instruction, QString* description = nullptr);
    void dasm_rfvar(QString& instruction, QString* description = nullptr);
    void dasm_rfvars(QString& instruction, QString* description = nullptr);
    void dasm_wfbyte(QString& instruction, QString* description = nullptr);
    void dasm_wfword(QString& instruction, QString* description = nullptr);
    void dasm_wflong(QString& instruction, QString* description = nullptr);

    void dasm_getqx(QString& instruction, QString* description = nullptr);
    void dasm_getqy(QString& instruction, QString* description = nullptr);
    void dasm_getct(QString& instruction, QString* description = nullptr);
    void dasm_getrnd(QString& instruction, QString* description = nullptr);
    void dasm_getrnd_cz(QString& instruction, QString* description = nullptr);

    void dasm_setdacs(QString& instruction, QString* description = nullptr);
    void dasm_setxfrq(QString& instruction, QString* description = nullptr);
    void dasm_getxacc(QString& instruction, QString* description = nullptr);
    void dasm_waitx(QString& instruction, QString* description = nullptr);
    void dasm_setse1(QString& instruction, QString* description = nullptr);
    void dasm_setse2(QString& instruction, QString* description = nullptr);
    void dasm_setse3(QString& instruction, QString* description = nullptr);
    void dasm_setse4(QString& instruction, QString* description = nullptr);
    void dasm_pollint(QString& instruction, QString* description = nullptr);
    void dasm_pollct1(QString& instruction, QString* description = nullptr);
    void dasm_pollct2(QString& instruction, QString* description = nullptr);
    void dasm_pollct3(QString& instruction, QString* description = nullptr);
    void dasm_pollse1(QString& instruction, QString* description = nullptr);
    void dasm_pollse2(QString& instruction, QString* description = nullptr);
    void dasm_pollse3(QString& instruction, QString* description = nullptr);
    void dasm_pollse4(QString& instruction, QString* description = nullptr);
    void dasm_pollpat(QString& instruction, QString* description = nullptr);
    void dasm_pollfbw(QString& instruction, QString* description = nullptr);
    void dasm_pollxmt(QString& instruction, QString* description = nullptr);
    void dasm_pollxfi(QString& instruction, QString* description = nullptr);
    void dasm_pollxro(QString& instruction, QString* description = nullptr);
    void dasm_pollxrl(QString& instruction, QString* description = nullptr);
    void dasm_pollatn(QString& instruction, QString* description = nullptr);
    void dasm_pollqmt(QString& instruction, QString* description = nullptr);

    void dasm_waitint(QString& instruction, QString* description = nullptr);
    void dasm_waitct1(QString& instruction, QString* description = nullptr);
    void dasm_waitct2(QString& instruction, QString* description = nullptr);
    void dasm_waitct3(QString& instruction, QString* description = nullptr);
    void dasm_waitse1(QString& instruction, QString* description = nullptr);
    void dasm_waitse2(QString& instruction, QString* description = nullptr);
    void dasm_waitse3(QString& instruction, QString* description = nullptr);
    void dasm_waitse4(QString& instruction, QString* description = nullptr);
    void dasm_waitpat(QString& instruction, QString* description = nullptr);
    void dasm_waitfbw(QString& instruction, QString* description = nullptr);
    void dasm_waitxmt(QString& instruction, QString* description = nullptr);
    void dasm_waitxfi(QString& instruction, QString* description = nullptr);
    void dasm_waitxro(QString& instruction, QString* description = nullptr);
    void dasm_waitxrl(QString& instruction, QString* description = nullptr);
    void dasm_waitatn(QString& instruction, QString* description = nullptr);

    void dasm_allowi(QString& instruction, QString* description = nullptr);
    void dasm_stalli(QString& instruction, QString* description = nullptr);
    void dasm_trgint1(QString& instruction, QString* description = nullptr);
    void dasm_trgint2(QString& instruction, QString* description = nullptr);
    void dasm_trgint3(QString& instruction, QString* description = nullptr);
    void dasm_nixint1(QString& instruction, QString* description = nullptr);
    void dasm_nixint2(QString& instruction, QString* description = nullptr);
    void dasm_nixint3(QString& instruction, QString* description = nullptr);
    void dasm_setint1(QString& instruction, QString* description = nullptr);
    void dasm_setint2(QString& instruction, QString* description = nullptr);
    void dasm_setint3(QString& instruction, QString* description = nullptr);
    void dasm_setq(QString& instruction, QString* description = nullptr);
    void dasm_setq2(QString& instruction, QString* description = nullptr);

    void dasm_push(QString& instruction, QString* description = nullptr);
    void dasm_pop(QString& instruction, QString* description = nullptr);
    void dasm_jmp(QString& instruction, QString* description = nullptr);
    void dasm_call(QString& instruction, QString* description = nullptr);
    void dasm_ret(QString& instruction, QString* description = nullptr);
    void dasm_calla(QString& instruction, QString* description = nullptr);
    void dasm_reta(QString& instruction, QString* description = nullptr);
    void dasm_callb(QString& instruction, QString* description = nullptr);
    void dasm_retb(QString& instruction, QString* description = nullptr);

    void dasm_jmprel(QString& instruction, QString* description = nullptr);
    void dasm_skip(QString& instruction, QString* description = nullptr);
    void dasm_skipf(QString& instruction, QString* description = nullptr);
    void dasm_execf(QString& instruction, QString* description = nullptr);
    void dasm_getptr(QString& instruction, QString* description = nullptr);
    void dasm_getbrk(QString& instruction, QString* description = nullptr);
    void dasm_cogbrk(QString& instruction, QString* description = nullptr);
    void dasm_brk(QString& instruction, QString* description = nullptr);

    void dasm_setluts(QString& instruction, QString* description = nullptr);
    void dasm_setcy(QString& instruction, QString* description = nullptr);
    void dasm_setci(QString& instruction, QString* description = nullptr);
    void dasm_setcq(QString& instruction, QString* description = nullptr);
    void dasm_setcfrq(QString& instruction, QString* description = nullptr);
    void dasm_setcmod(QString& instruction, QString* description = nullptr);
    void dasm_setpiv(QString& instruction, QString* description = nullptr);
    void dasm_setpix(QString& instruction, QString* description = nullptr);
    void dasm_cogatn(QString& instruction, QString* description = nullptr);

    void dasm_testp_w(QString& instruction, QString* description = nullptr);
    void dasm_testpn_w(QString& instruction, QString* description = nullptr);
    void dasm_testp_and(QString& instruction, QString* description = nullptr);
    void dasm_testpn_and(QString& instruction, QString* description = nullptr);
    void dasm_testp_or(QString& instruction, QString* description = nullptr);
    void dasm_testpn_or(QString& instruction, QString* description = nullptr);
    void dasm_testp_xor(QString& instruction, QString* description = nullptr);
    void dasm_testpn_xor(QString& instruction, QString* description = nullptr);

    void dasm_dirl(QString& instruction, QString* description = nullptr);
    void dasm_dirh(QString& instruction, QString* description = nullptr);
    void dasm_dirc(QString& instruction, QString* description = nullptr);
    void dasm_dirnc(QString& instruction, QString* description = nullptr);
    void dasm_dirz(QString& instruction, QString* description = nullptr);
    void dasm_dirnz(QString& instruction, QString* description = nullptr);
    void dasm_dirrnd(QString& instruction, QString* description = nullptr);
    void dasm_dirnot(QString& instruction, QString* description = nullptr);

    void dasm_outl(QString& instruction, QString* description = nullptr);
    void dasm_outh(QString& instruction, QString* description = nullptr);
    void dasm_outc(QString& instruction, QString* description = nullptr);
    void dasm_outnc(QString& instruction, QString* description = nullptr);
    void dasm_outz(QString& instruction, QString* description = nullptr);
    void dasm_outnz(QString& instruction, QString* description = nullptr);
    void dasm_outrnd(QString& instruction, QString* description = nullptr);
    void dasm_outnot(QString& instruction, QString* description = nullptr);

    void dasm_fltl(QString& instruction, QString* description = nullptr);
    void dasm_flth(QString& instruction, QString* description = nullptr);
    void dasm_fltc(QString& instruction, QString* description = nullptr);
    void dasm_fltnc(QString& instruction, QString* description = nullptr);
    void dasm_fltz(QString& instruction, QString* description = nullptr);
    void dasm_fltnz(QString& instruction, QString* description = nullptr);
    void dasm_fltrnd(QString& instruction, QString* description = nullptr);
    void dasm_fltnot(QString& instruction, QString* description = nullptr);

    void dasm_drvl(QString& instruction, QString* description = nullptr);
    void dasm_drvh(QString& instruction, QString* description = nullptr);
    void dasm_drvc(QString& instruction, QString* description = nullptr);
    void dasm_drvnc(QString& instruction, QString* description = nullptr);
    void dasm_drvz(QString& instruction, QString* description = nullptr);
    void dasm_drvnz(QString& instruction, QString* description = nullptr);
    void dasm_drvrnd(QString& instruction, QString* description = nullptr);
    void dasm_drvnot(QString& instruction, QString* description = nullptr);

    void dasm_splitb(QString& instruction, QString* description = nullptr);
    void dasm_mergeb(QString& instruction, QString* description = nullptr);
    void dasm_splitw(QString& instruction, QString* description = nullptr);
    void dasm_mergew(QString& instruction, QString* description = nullptr);
    void dasm_seussf(QString& instruction, QString* description = nullptr);
    void dasm_seussr(QString& instruction, QString* description = nullptr);
    void dasm_rgbsqz(QString& instruction, QString* description = nullptr);
    void dasm_rgbexp(QString& instruction, QString* description = nullptr);
    void dasm_xoro32(QString& instruction, QString* description = nullptr);

    void dasm_rev(QString& instruction, QString* description = nullptr);
    void dasm_rczr(QString& instruction, QString* description = nullptr);
    void dasm_rczl(QString& instruction, QString* description = nullptr);
    void dasm_wrc(QString& instruction, QString* description = nullptr);
    void dasm_wrnc(QString& instruction, QString* description = nullptr);
    void dasm_wrz(QString& instruction, QString* description = nullptr);
    void dasm_wrnz(QString& instruction, QString* description = nullptr);
    void dasm_modcz(QString& instruction, QString* description = nullptr);

    void dasm_jmp_abs(QString& instruction, QString* description = nullptr);
    void dasm_call_abs(QString& instruction, QString* description = nullptr);
    void dasm_calla_abs(QString& instruction, QString* description = nullptr);
    void dasm_callb_abs(QString& instruction, QString* description = nullptr);
    void dasm_calld_abs(QString& instruction, QString* description = nullptr);
    void dasm_loc_pa(QString& instruction, QString* description = nullptr);
    void dasm_loc_pb(QString& instruction, QString* description = nullptr);
    void dasm_loc_ptra(QString& instruction, QString* description = nullptr);
    void dasm_loc_ptrb(QString& instruction, QString* description = nullptr);

    void dasm_augs(QString& instruction, QString* description = nullptr);
    void dasm_augd(QString& instruction, QString* description = nullptr);
};
