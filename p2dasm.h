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

    bool dasm(p2_LONG addr, QString& opcode, QString& instruction);
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

    void dasm_nop(QString& instruction);
    void dasm_ror(QString& instruction);
    void dasm_rol(QString& instruction);
    void dasm_shr(QString& instruction);
    void dasm_shl(QString& instruction);
    void dasm_rcr(QString& instruction);
    void dasm_rcl(QString& instruction);
    void dasm_sar(QString& instruction);
    void dasm_sal(QString& instruction);

    void dasm_add(QString& instruction);
    void dasm_addx(QString& instruction);
    void dasm_adds(QString& instruction);
    void dasm_addsx(QString& instruction);
    void dasm_sub(QString& instruction);
    void dasm_subx(QString& instruction);
    void dasm_subs(QString& instruction);
    void dasm_subsx(QString& instruction);

    void dasm_cmp(QString& instruction);
    void dasm_cmpx(QString& instruction);
    void dasm_cmps(QString& instruction);
    void dasm_cmpsx(QString& instruction);
    void dasm_cmpr(QString& instruction);
    void dasm_cmpm(QString& instruction);
    void dasm_subr(QString& instruction);
    void dasm_cmpsub(QString& instruction);

    void dasm_fge(QString& instruction);
    void dasm_fle(QString& instruction);
    void dasm_fges(QString& instruction);
    void dasm_fles(QString& instruction);
    void dasm_sumc(QString& instruction);
    void dasm_sumnc(QString& instruction);
    void dasm_sumz(QString& instruction);
    void dasm_sumnz(QString& instruction);

    void dasm_testb_w(QString& instruction);
    void dasm_testbn_w(QString& instruction);
    void dasm_testb_and(QString& instruction);
    void dasm_testbn_and(QString& instruction);
    void dasm_testb_or(QString& instruction);
    void dasm_testbn_or(QString& instruction);
    void dasm_testb_xor(QString& instruction);
    void dasm_testbn_xor(QString& instruction);

    void dasm_bitl(QString& instruction);
    void dasm_bith(QString& instruction);
    void dasm_bitc(QString& instruction);
    void dasm_bitnc(QString& instruction);
    void dasm_bitz(QString& instruction);
    void dasm_bitnz(QString& instruction);
    void dasm_bitrnd(QString& instruction);
    void dasm_bitnot(QString& instruction);

    void dasm_and(QString& instruction);
    void dasm_andn(QString& instruction);
    void dasm_or(QString& instruction);
    void dasm_xor(QString& instruction);
    void dasm_muxc(QString& instruction);
    void dasm_muxnc(QString& instruction);
    void dasm_muxz(QString& instruction);
    void dasm_muxnz(QString& instruction);

    void dasm_mov(QString& instruction);
    void dasm_not(QString& instruction);
    void dasm_not_d(QString& instruction);
    void dasm_abs(QString& instruction);
    void dasm_neg(QString& instruction);
    void dasm_negc(QString& instruction);
    void dasm_negnc(QString& instruction);
    void dasm_negz(QString& instruction);
    void dasm_negnz(QString& instruction);

    void dasm_incmod(QString& instruction);
    void dasm_decmod(QString& instruction);
    void dasm_zerox(QString& instruction);
    void dasm_signx(QString& instruction);
    void dasm_encod(QString& instruction);
    void dasm_ones(QString& instruction);
    void dasm_test(QString& instruction);
    void dasm_testn(QString& instruction);

    void dasm_setnib(QString& instruction);
    void dasm_setnib_altsn(QString& instruction);
    void dasm_getnib(QString& instruction);
    void dasm_getnib_altgn(QString& instruction);
    void dasm_rolnib(QString& instruction);
    void dasm_rolnib_altgn(QString& instruction);
    void dasm_setbyte(QString& instruction);
    void dasm_setbyte_altsb(QString& instruction);
    void dasm_getbyte(QString& instruction);
    void dasm_getbyte_altgb(QString& instruction);
    void dasm_rolbyte(QString& instruction);
    void dasm_rolbyte_altgb(QString& instruction);

    void dasm_setword(QString& instruction);
    void dasm_setword_altsw(QString& instruction);
    void dasm_getword(QString& instruction);
    void dasm_getword_altgw(QString& instruction);
    void dasm_rolword(QString& instruction);
    void dasm_rolword_altgw(QString& instruction);

    void dasm_altsn(QString& instruction);
    void dasm_altsn_d(QString& instruction);
    void dasm_altgn(QString& instruction);
    void dasm_altgn_d(QString& instruction);
    void dasm_altsb(QString& instruction);
    void dasm_altsb_d(QString& instruction);
    void dasm_altgb(QString& instruction);
    void dasm_altgb_d(QString& instruction);

    void dasm_altsw(QString& instruction);
    void dasm_altsw_d(QString& instruction);
    void dasm_altgw(QString& instruction);
    void dasm_altgw_d(QString& instruction);

    void dasm_altr(QString& instruction);
    void dasm_altr_d(QString& instruction);
    void dasm_altd(QString& instruction);
    void dasm_altd_d(QString& instruction);
    void dasm_alts(QString& instruction);
    void dasm_alts_d(QString& instruction);
    void dasm_altb(QString& instruction);
    void dasm_altb_d(QString& instruction);
    void dasm_alti(QString& instruction);
    void dasm_alti_d(QString& instruction);

    void dasm_setr(QString& instruction);
    void dasm_setd(QString& instruction);
    void dasm_sets(QString& instruction);
    void dasm_decod(QString& instruction);
    void dasm_decod_d(QString& instruction);
    void dasm_bmask(QString& instruction);
    void dasm_bmask_d(QString& instruction);
    void dasm_crcbit(QString& instruction);
    void dasm_crcnib(QString& instruction);

    void dasm_muxnits(QString& instruction);
    void dasm_muxnibs(QString& instruction);
    void dasm_muxq(QString& instruction);
    void dasm_movbyts(QString& instruction);
    void dasm_mul(QString& instruction);
    void dasm_muls(QString& instruction);
    void dasm_sca(QString& instruction);
    void dasm_scas(QString& instruction);

    void dasm_addpix(QString& instruction);
    void dasm_mulpix(QString& instruction);
    void dasm_blnpix(QString& instruction);
    void dasm_mixpix(QString& instruction);
    void dasm_addct1(QString& instruction);
    void dasm_addct2(QString& instruction);
    void dasm_addct3(QString& instruction);

    void dasm_wmlong(QString& instruction);
    void dasm_rqpin(QString& instruction);
    void dasm_rdpin(QString& instruction);
    void dasm_rdlut(QString& instruction);
    void dasm_rdbyte(QString& instruction);
    void dasm_rdword(QString& instruction);
    void dasm_rdlong(QString& instruction);

    void dasm_popa(QString& instruction);
    void dasm_popb(QString& instruction);
    void dasm_calld(QString& instruction);
    void dasm_resi3(QString& instruction);
    void dasm_resi2(QString& instruction);
    void dasm_resi1(QString& instruction);
    void dasm_resi0(QString& instruction);
    void dasm_reti3(QString& instruction);
    void dasm_reti2(QString& instruction);
    void dasm_reti1(QString& instruction);
    void dasm_reti0(QString& instruction);
    void dasm_callpa(QString& instruction);
    void dasm_callpb(QString& instruction);

    void dasm_djz(QString& instruction);
    void dasm_djnz(QString& instruction);
    void dasm_djf(QString& instruction);
    void dasm_djnf(QString& instruction);
    void dasm_ijz(QString& instruction);
    void dasm_ijnz(QString& instruction);
    void dasm_tjz(QString& instruction);
    void dasm_tjnz(QString& instruction);
    void dasm_tjf(QString& instruction);
    void dasm_tjnf(QString& instruction);
    void dasm_tjs(QString& instruction);
    void dasm_tjns(QString& instruction);
    void dasm_tjv(QString& instruction);

    void dasm_jint(QString& instruction);
    void dasm_jct1(QString& instruction);
    void dasm_jct2(QString& instruction);
    void dasm_jct3(QString& instruction);
    void dasm_jse1(QString& instruction);
    void dasm_jse2(QString& instruction);
    void dasm_jse3(QString& instruction);
    void dasm_jse4(QString& instruction);
    void dasm_jpat(QString& instruction);
    void dasm_jfbw(QString& instruction);
    void dasm_jxmt(QString& instruction);
    void dasm_jxfi(QString& instruction);
    void dasm_jxro(QString& instruction);
    void dasm_jxrl(QString& instruction);
    void dasm_jatn(QString& instruction);
    void dasm_jqmt(QString& instruction);

    void dasm_jnint(QString& instruction);
    void dasm_jnct1(QString& instruction);
    void dasm_jnct2(QString& instruction);
    void dasm_jnct3(QString& instruction);
    void dasm_jnse1(QString& instruction);
    void dasm_jnse2(QString& instruction);
    void dasm_jnse3(QString& instruction);
    void dasm_jnse4(QString& instruction);
    void dasm_jnpat(QString& instruction);
    void dasm_jnfbw(QString& instruction);
    void dasm_jnxmt(QString& instruction);
    void dasm_jnxfi(QString& instruction);
    void dasm_jnxro(QString& instruction);
    void dasm_jnxrl(QString& instruction);
    void dasm_jnatn(QString& instruction);
    void dasm_jnqmt(QString& instruction);
    void dasm_1011110_1(QString& instruction);
    void dasm_1011111_0(QString& instruction);
    void dasm_setpat(QString& instruction);

    void dasm_wrpin(QString& instruction);
    void dasm_akpin(QString& instruction);
    void dasm_wxpin(QString& instruction);
    void dasm_wypin(QString& instruction);
    void dasm_wrlut(QString& instruction);
    void dasm_wrbyte(QString& instruction);
    void dasm_wrword(QString& instruction);
    void dasm_wrlong(QString& instruction);

    void dasm_pusha(QString& instruction);
    void dasm_pushb(QString& instruction);
    void dasm_rdfast(QString& instruction);
    void dasm_wrfast(QString& instruction);
    void dasm_fblock(QString& instruction);
    void dasm_xinit(QString& instruction);
    void dasm_xstop(QString& instruction);
    void dasm_xzero(QString& instruction);
    void dasm_xcont(QString& instruction);

    void dasm_rep(QString& instruction);
    void dasm_coginit(QString& instruction);

    void dasm_qmul(QString& instruction);
    void dasm_qdiv(QString& instruction);
    void dasm_qfrac(QString& instruction);
    void dasm_qsqrt(QString& instruction);
    void dasm_qrotate(QString& instruction);
    void dasm_qvector(QString& instruction);

    void dasm_hubset(QString& instruction);
    void dasm_cogid(QString& instruction);
    void dasm_cogstop(QString& instruction);
    void dasm_locknew(QString& instruction);
    void dasm_lockret(QString& instruction);
    void dasm_locktry(QString& instruction);
    void dasm_lockrel(QString& instruction);
    void dasm_qlog(QString& instruction);
    void dasm_qexp(QString& instruction);

    void dasm_rfbyte(QString& instruction);
    void dasm_rfword(QString& instruction);
    void dasm_rflong(QString& instruction);
    void dasm_rfvar(QString& instruction);
    void dasm_rfvars(QString& instruction);
    void dasm_wfbyte(QString& instruction);
    void dasm_wfword(QString& instruction);
    void dasm_wflong(QString& instruction);

    void dasm_getqx(QString& instruction);
    void dasm_getqy(QString& instruction);
    void dasm_getct(QString& instruction);
    void dasm_getrnd(QString& instruction);
    void dasm_getrnd_cz(QString& instruction);

    void dasm_setdacs(QString& instruction);
    void dasm_setxfrq(QString& instruction);
    void dasm_getxacc(QString& instruction);
    void dasm_waitx(QString& instruction);
    void dasm_setse1(QString& instruction);
    void dasm_setse2(QString& instruction);
    void dasm_setse3(QString& instruction);
    void dasm_setse4(QString& instruction);
    void dasm_pollint(QString& instruction);
    void dasm_pollct1(QString& instruction);
    void dasm_pollct2(QString& instruction);
    void dasm_pollct3(QString& instruction);
    void dasm_pollse1(QString& instruction);
    void dasm_pollse2(QString& instruction);
    void dasm_pollse3(QString& instruction);
    void dasm_pollse4(QString& instruction);
    void dasm_pollpat(QString& instruction);
    void dasm_pollfbw(QString& instruction);
    void dasm_pollxmt(QString& instruction);
    void dasm_pollxfi(QString& instruction);
    void dasm_pollxro(QString& instruction);
    void dasm_pollxrl(QString& instruction);
    void dasm_pollatn(QString& instruction);
    void dasm_pollqmt(QString& instruction);

    void dasm_waitint(QString& instruction);
    void dasm_waitct1(QString& instruction);
    void dasm_waitct2(QString& instruction);
    void dasm_waitct3(QString& instruction);
    void dasm_waitse1(QString& instruction);
    void dasm_waitse2(QString& instruction);
    void dasm_waitse3(QString& instruction);
    void dasm_waitse4(QString& instruction);
    void dasm_waitpat(QString& instruction);
    void dasm_waitfbw(QString& instruction);
    void dasm_waitxmt(QString& instruction);
    void dasm_waitxfi(QString& instruction);
    void dasm_waitxro(QString& instruction);
    void dasm_waitxrl(QString& instruction);
    void dasm_waitatn(QString& instruction);

    void dasm_allowi(QString& instruction);
    void dasm_stalli(QString& instruction);
    void dasm_trgint1(QString& instruction);
    void dasm_trgint2(QString& instruction);
    void dasm_trgint3(QString& instruction);
    void dasm_nixint1(QString& instruction);
    void dasm_nixint2(QString& instruction);
    void dasm_nixint3(QString& instruction);
    void dasm_setint1(QString& instruction);
    void dasm_setint2(QString& instruction);
    void dasm_setint3(QString& instruction);
    void dasm_setq(QString& instruction);
    void dasm_setq2(QString& instruction);

    void dasm_push(QString& instruction);
    void dasm_pop(QString& instruction);
    void dasm_jmp(QString& instruction);
    void dasm_call(QString& instruction);
    void dasm_ret(QString& instruction);
    void dasm_calla(QString& instruction);
    void dasm_reta(QString& instruction);
    void dasm_callb(QString& instruction);
    void dasm_retb(QString& instruction);

    void dasm_jmprel(QString& instruction);
    void dasm_skip(QString& instruction);
    void dasm_skipf(QString& instruction);
    void dasm_execf(QString& instruction);
    void dasm_getptr(QString& instruction);
    void dasm_getbrk(QString& instruction);
    void dasm_cogbrk(QString& instruction);
    void dasm_brk(QString& instruction);

    void dasm_setluts(QString& instruction);
    void dasm_setcy(QString& instruction);
    void dasm_setci(QString& instruction);
    void dasm_setcq(QString& instruction);
    void dasm_setcfrq(QString& instruction);
    void dasm_setcmod(QString& instruction);
    void dasm_setpiv(QString& instruction);
    void dasm_setpix(QString& instruction);
    void dasm_cogatn(QString& instruction);

    void dasm_testp_w(QString& instruction);
    void dasm_testpn_w(QString& instruction);
    void dasm_testp_and(QString& instruction);
    void dasm_testpn_and(QString& instruction);
    void dasm_testp_or(QString& instruction);
    void dasm_testpn_or(QString& instruction);
    void dasm_testp_xor(QString& instruction);
    void dasm_testpn_xor(QString& instruction);

    void dasm_dirl(QString& instruction);
    void dasm_dirh(QString& instruction);
    void dasm_dirc(QString& instruction);
    void dasm_dirnc(QString& instruction);
    void dasm_dirz(QString& instruction);
    void dasm_dirnz(QString& instruction);
    void dasm_dirrnd(QString& instruction);
    void dasm_dirnot(QString& instruction);

    void dasm_outl(QString& instruction);
    void dasm_outh(QString& instruction);
    void dasm_outc(QString& instruction);
    void dasm_outnc(QString& instruction);
    void dasm_outz(QString& instruction);
    void dasm_outnz(QString& instruction);
    void dasm_outrnd(QString& instruction);
    void dasm_outnot(QString& instruction);

    void dasm_fltl(QString& instruction);
    void dasm_flth(QString& instruction);
    void dasm_fltc(QString& instruction);
    void dasm_fltnc(QString& instruction);
    void dasm_fltz(QString& instruction);
    void dasm_fltnz(QString& instruction);
    void dasm_fltrnd(QString& instruction);
    void dasm_fltnot(QString& instruction);

    void dasm_drvl(QString& instruction);
    void dasm_drvh(QString& instruction);
    void dasm_drvc(QString& instruction);
    void dasm_drvnc(QString& instruction);
    void dasm_drvz(QString& instruction);
    void dasm_drvnz(QString& instruction);
    void dasm_drvrnd(QString& instruction);
    void dasm_drvnot(QString& instruction);

    void dasm_splitb(QString& instruction);
    void dasm_mergeb(QString& instruction);
    void dasm_splitw(QString& instruction);
    void dasm_mergew(QString& instruction);
    void dasm_seussf(QString& instruction);
    void dasm_seussr(QString& instruction);
    void dasm_rgbsqz(QString& instruction);
    void dasm_rgbexp(QString& instruction);
    void dasm_xoro32(QString& instruction);

    void dasm_rev(QString& instruction);
    void dasm_rczr(QString& instruction);
    void dasm_rczl(QString& instruction);
    void dasm_wrc(QString& instruction);
    void dasm_wrnc(QString& instruction);
    void dasm_wrz(QString& instruction);
    void dasm_wrnz(QString& instruction);
    void dasm_modcz(QString& instruction);

    void dasm_jmp_abs(QString& instruction);
    void dasm_call_abs(QString& instruction);
    void dasm_calla_abs(QString& instruction);
    void dasm_callb_abs(QString& instruction);
    void dasm_calld_abs(QString& instruction);
    void dasm_loc_pa(QString& instruction);
    void dasm_loc_pb(QString& instruction);
    void dasm_loc_ptra(QString& instruction);
    void dasm_loc_ptrb(QString& instruction);

    void dasm_augs(QString& instruction);
    void dasm_augd(QString& instruction);
};
