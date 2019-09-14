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
#include "p2opcode.h"
#include "p2tokens.h"

class P2Cog;

class P2Dasm : public QObject
{
    Q_OBJECT
public:
    P2Dasm(const P2Cog* cog, QObject* parent = nullptr);

    bool dasm(p2_LONG addr, QString* opcode = nullptr, QString* instruction = nullptr, QString* description = nullptr);
    p2_LONG rd_mem(p2_LONG addr);
    p2_LONG memsize() const;
    bool lowercase() const;

public slots:
    void set_lowercase(bool flag);

private:
    const P2Cog* COG;
    bool m_lowercase;
    int pad_opcode;
    int pad_inst;
    int pad_wcz;
    P2Opcode IR;
    quint32 PC;
    quint32 S;
    quint32 D;

    p2_TOKEN_e conditional(p2_Cond_e cond);
    p2_TOKEN_e conditional(unsigned cond);

    static QString format_num(uint num, bool binary = false);
    static QString format_bin(uint num, int digits = 0);

    QString format_imm(bool im = false);
    void format_WCZ(QString* instruction, p2_TOKEN_e with = t_WCZ);
    void format_WC(QString* instruction, p2_TOKEN_e with = t_WC);
    void format_WZ(QString* instruction, p2_TOKEN_e with = t_WZ);
    void format_inst(QString* instruction, p2_TOKEN_e inst);
    void format_D_IM_S_WCZ(QString* instruction, p2_TOKEN_e inst, p2_TOKEN_e wcz = t_WCZ);
    void format_D_IM_S_WC(QString* instruction, p2_TOKEN_e inst);
    void format_D_IM_S_WZ(QString* instruction, p2_TOKEN_e inst, p2_TOKEN_e with = t_WZ);
    void format_WZ_D_IM_S(QString* instruction, p2_TOKEN_e inst);
    void format_WZ_D_IM_S_WC(QString* instruction, p2_TOKEN_e inst);
    void format_D_IM_S_NNN(QString* instruction, p2_TOKEN_e inst, uint max = 7);
    void format_D_IMM_S(QString* instruction, p2_TOKEN_e inst);
    void format_D_CZ(QString* instruction, p2_TOKEN_e inst, p2_TOKEN_e with = t_WCZ);
    void format_CZ(QString* instruction, p2_TOKEN_e inst, p2_TOKEN_e with = t_WCZ);
    void format_MODCZ_WCZ(QString* instruction, p2_TOKEN_e inst, p2_TOKEN_e with = t_WCZ);
    void format_D(QString* instruction, p2_TOKEN_e inst);
    void format_WZ_D(QString* instruction, p2_TOKEN_e inst);
    void format_IM_D(QString* instruction, p2_TOKEN_e inst);
    void format_IM_D_WCZ(QString* instruction, p2_TOKEN_e inst, p2_TOKEN_e with = t_WCZ);
    void format_IM_D_WC(QString* instruction, p2_TOKEN_e inst);
    void format_IM_S(QString* instruction, p2_TOKEN_e inst);
    void format_IM_S_WC(QString* instruction, p2_TOKEN_e inst);
    void format_PC_A20(QString* instruction, p2_TOKEN_e inst, p2_TOKEN_e dest = t_none);
    void format_IMM23(QString* instruction, p2_TOKEN_e inst);

    void da_NOP(QString* instruction = nullptr);
    void da_ROR(QString* instruction = nullptr);
    void da_ROL(QString* instruction = nullptr);
    void da_SHR(QString* instruction = nullptr);
    void da_SHL(QString* instruction = nullptr);
    void da_RCR(QString* instruction = nullptr);
    void da_RCL(QString* instruction = nullptr);
    void da_SAR(QString* instruction = nullptr);
    void da_SAL(QString* instruction = nullptr);

    void da_ADD(QString* instruction = nullptr);
    void da_ADDX(QString* instruction = nullptr);
    void da_ADDS(QString* instruction = nullptr);
    void da_ADDSX(QString* instruction = nullptr);
    void da_SUB(QString* instruction = nullptr);
    void da_SUBX(QString* instruction = nullptr);
    void da_subs(QString* instruction = nullptr);
    void da_subsx(QString* instruction = nullptr);

    void da_cmp(QString* instruction = nullptr);
    void da_cmpx(QString* instruction = nullptr);
    void da_cmps(QString* instruction = nullptr);
    void da_cmpsx(QString* instruction = nullptr);
    void da_cmpr(QString* instruction = nullptr);
    void da_cmpm(QString* instruction = nullptr);
    void da_subr(QString* instruction = nullptr);
    void da_cmpsub(QString* instruction = nullptr);

    void da_fge(QString* instruction = nullptr);
    void da_fle(QString* instruction = nullptr);
    void da_fges(QString* instruction = nullptr);
    void da_fles(QString* instruction = nullptr);
    void da_sumc(QString* instruction = nullptr);
    void da_sumnc(QString* instruction = nullptr);
    void da_sumz(QString* instruction = nullptr);
    void da_sumnz(QString* instruction = nullptr);

    void da_testb_w(QString* instruction = nullptr);
    void da_testbn_w(QString* instruction = nullptr);
    void da_testb_and(QString* instruction = nullptr);
    void da_testbn_and(QString* instruction = nullptr);
    void da_testb_or(QString* instruction = nullptr);
    void da_testbn_or(QString* instruction = nullptr);
    void da_testb_xor(QString* instruction = nullptr);
    void da_testbn_xor(QString* instruction = nullptr);

    void da_bitl(QString* instruction = nullptr);
    void da_bith(QString* instruction = nullptr);
    void da_bitc(QString* instruction = nullptr);
    void da_bitnc(QString* instruction = nullptr);
    void da_bitz(QString* instruction = nullptr);
    void da_bitnz(QString* instruction = nullptr);
    void da_bitrnd(QString* instruction = nullptr);
    void da_bitnot(QString* instruction = nullptr);

    void da_and(QString* instruction = nullptr);
    void da_andn(QString* instruction = nullptr);
    void da_or(QString* instruction = nullptr);
    void da_xor(QString* instruction = nullptr);
    void da_muxc(QString* instruction = nullptr);
    void da_muxnc(QString* instruction = nullptr);
    void da_muxz(QString* instruction = nullptr);
    void da_muxnz(QString* instruction = nullptr);

    void da_mov(QString* instruction = nullptr);
    void da_not(QString* instruction = nullptr);
    void da_not_d(QString* instruction = nullptr);
    void da_abs(QString* instruction = nullptr);
    void da_neg(QString* instruction = nullptr);
    void da_negc(QString* instruction = nullptr);
    void da_negnc(QString* instruction = nullptr);
    void da_negz(QString* instruction = nullptr);
    void da_negnz(QString* instruction = nullptr);

    void da_incmod(QString* instruction = nullptr);
    void da_decmod(QString* instruction = nullptr);
    void da_zerox(QString* instruction = nullptr);
    void da_signx(QString* instruction = nullptr);
    void da_encod(QString* instruction = nullptr);
    void da_ones(QString* instruction = nullptr);
    void da_test(QString* instruction = nullptr);
    void da_testn(QString* instruction = nullptr);

    void da_setnib(QString* instruction = nullptr);
    void da_setnib_altsn(QString* instruction = nullptr);
    void da_getnib(QString* instruction = nullptr);
    void da_getnib_altgn(QString* instruction = nullptr);
    void da_rolnib(QString* instruction = nullptr);
    void da_rolnib_altgn(QString* instruction = nullptr);
    void da_setbyte(QString* instruction = nullptr);
    void da_setbyte_altsb(QString* instruction = nullptr);
    void da_getbyte(QString* instruction = nullptr);
    void da_getbyte_altgb(QString* instruction = nullptr);
    void da_rolbyte(QString* instruction = nullptr);
    void da_rolbyte_altgb(QString* instruction = nullptr);

    void da_setword(QString* instruction = nullptr);
    void da_setword_altsw(QString* instruction = nullptr);
    void da_getword(QString* instruction = nullptr);
    void da_getword_altgw(QString* instruction = nullptr);
    void da_rolword(QString* instruction = nullptr);
    void da_rolword_altgw(QString* instruction = nullptr);

    void da_altsn(QString* instruction = nullptr);
    void da_altsn_d(QString* instruction = nullptr);
    void da_altgn(QString* instruction = nullptr);
    void da_altgn_d(QString* instruction = nullptr);
    void da_altsb(QString* instruction = nullptr);
    void da_altsb_d(QString* instruction = nullptr);
    void da_altgb(QString* instruction = nullptr);
    void da_altgb_d(QString* instruction = nullptr);

    void da_altsw(QString* instruction = nullptr);
    void da_altsw_d(QString* instruction = nullptr);
    void da_altgw(QString* instruction = nullptr);
    void da_altgw_d(QString* instruction = nullptr);

    void da_altr(QString* instruction = nullptr);
    void da_altr_d(QString* instruction = nullptr);
    void da_altd(QString* instruction = nullptr);
    void da_altd_d(QString* instruction = nullptr);
    void da_alts(QString* instruction = nullptr);
    void da_alts_d(QString* instruction = nullptr);
    void da_altb(QString* instruction = nullptr);
    void da_altb_d(QString* instruction = nullptr);
    void da_alti(QString* instruction = nullptr);
    void da_alti_d(QString* instruction = nullptr);

    void da_setr(QString* instruction = nullptr);
    void da_setd(QString* instruction = nullptr);
    void da_sets(QString* instruction = nullptr);
    void da_decod(QString* instruction = nullptr);
    void da_decod_d(QString* instruction = nullptr);
    void da_bmask(QString* instruction = nullptr);
    void da_bmask_d(QString* instruction = nullptr);
    void da_crcbit(QString* instruction = nullptr);
    void da_crcnib(QString* instruction = nullptr);

    void da_muxnits(QString* instruction = nullptr);
    void da_muxnibs(QString* instruction = nullptr);
    void da_muxq(QString* instruction = nullptr);
    void da_movbyts(QString* instruction = nullptr);
    void da_mul(QString* instruction = nullptr);
    void da_muls(QString* instruction = nullptr);
    void da_sca(QString* instruction = nullptr);
    void da_scas(QString* instruction = nullptr);

    void da_addpix(QString* instruction = nullptr);
    void da_mulpix(QString* instruction = nullptr);
    void da_blnpix(QString* instruction = nullptr);
    void da_mixpix(QString* instruction = nullptr);
    void da_addct1(QString* instruction = nullptr);
    void da_addct2(QString* instruction = nullptr);
    void da_addct3(QString* instruction = nullptr);

    void da_wmlong(QString* instruction = nullptr);
    void da_rqpin(QString* instruction = nullptr);
    void da_rdpin(QString* instruction = nullptr);
    void da_rdlut(QString* instruction = nullptr);
    void da_rdbyte(QString* instruction = nullptr);
    void da_rdword(QString* instruction = nullptr);
    void da_rdlong(QString* instruction = nullptr);

    void da_popa(QString* instruction = nullptr);
    void da_popb(QString* instruction = nullptr);
    void da_calld(QString* instruction = nullptr);
    void da_resi3(QString* instruction = nullptr);
    void da_resi2(QString* instruction = nullptr);
    void da_resi1(QString* instruction = nullptr);
    void da_resi0(QString* instruction = nullptr);
    void da_reti3(QString* instruction = nullptr);
    void da_reti2(QString* instruction = nullptr);
    void da_reti1(QString* instruction = nullptr);
    void da_reti0(QString* instruction = nullptr);
    void da_callpa(QString* instruction = nullptr);
    void da_callpb(QString* instruction = nullptr);

    void da_djz(QString* instruction = nullptr);
    void da_djnz(QString* instruction = nullptr);
    void da_djf(QString* instruction = nullptr);
    void da_djnf(QString* instruction = nullptr);
    void da_ijz(QString* instruction = nullptr);
    void da_ijnz(QString* instruction = nullptr);
    void da_tjz(QString* instruction = nullptr);
    void da_tjnz(QString* instruction = nullptr);
    void da_tjf(QString* instruction = nullptr);
    void da_tjnf(QString* instruction = nullptr);
    void da_tjs(QString* instruction = nullptr);
    void da_tjns(QString* instruction = nullptr);
    void da_tjv(QString* instruction = nullptr);

    void da_jint(QString* instruction = nullptr);
    void da_jct1(QString* instruction = nullptr);
    void da_jct2(QString* instruction = nullptr);
    void da_jct3(QString* instruction = nullptr);
    void da_jse1(QString* instruction = nullptr);
    void da_jse2(QString* instruction = nullptr);
    void da_jse3(QString* instruction = nullptr);
    void da_jse4(QString* instruction = nullptr);
    void da_jpat(QString* instruction = nullptr);
    void da_jfbw(QString* instruction = nullptr);
    void da_jxmt(QString* instruction = nullptr);
    void da_jxfi(QString* instruction = nullptr);
    void da_jxro(QString* instruction = nullptr);
    void da_jxrl(QString* instruction = nullptr);
    void da_jatn(QString* instruction = nullptr);
    void da_jqmt(QString* instruction = nullptr);

    void da_jnint(QString* instruction = nullptr);
    void da_jnct1(QString* instruction = nullptr);
    void da_jnct2(QString* instruction = nullptr);
    void da_jnct3(QString* instruction = nullptr);
    void da_jnse1(QString* instruction = nullptr);
    void da_jnse2(QString* instruction = nullptr);
    void da_jnse3(QString* instruction = nullptr);
    void da_jnse4(QString* instruction = nullptr);
    void da_jnpat(QString* instruction = nullptr);
    void da_jnfbw(QString* instruction = nullptr);
    void da_jnxmt(QString* instruction = nullptr);
    void da_jnxfi(QString* instruction = nullptr);
    void da_jnxro(QString* instruction = nullptr);
    void da_jnxrl(QString* instruction = nullptr);
    void da_jnatn(QString* instruction = nullptr);
    void da_jnqmt(QString* instruction = nullptr);
    void da_1011110_1(QString* instruction = nullptr);
    void da_1011111_0(QString* instruction = nullptr);
    void da_setpat(QString* instruction = nullptr);

    void da_wrpin(QString* instruction = nullptr);
    void da_akpin(QString* instruction = nullptr);
    void da_wxpin(QString* instruction = nullptr);
    void da_wypin(QString* instruction = nullptr);
    void da_wrlut(QString* instruction = nullptr);
    void da_wrbyte(QString* instruction = nullptr);
    void da_wrword(QString* instruction = nullptr);
    void da_wrlong(QString* instruction = nullptr);

    void da_pusha(QString* instruction = nullptr);
    void da_pushb(QString* instruction = nullptr);
    void da_rdfast(QString* instruction = nullptr);
    void da_wrfast(QString* instruction = nullptr);
    void da_fblock(QString* instruction = nullptr);
    void da_xinit(QString* instruction = nullptr);
    void da_xstop(QString* instruction = nullptr);
    void da_xzero(QString* instruction = nullptr);
    void da_xcont(QString* instruction = nullptr);

    void da_rep(QString* instruction = nullptr);
    void da_coginit(QString* instruction = nullptr);

    void da_qmul(QString* instruction = nullptr);
    void da_qdiv(QString* instruction = nullptr);
    void da_qfrac(QString* instruction = nullptr);
    void da_qsqrt(QString* instruction = nullptr);
    void da_qrotate(QString* instruction = nullptr);
    void da_qvector(QString* instruction = nullptr);

    void da_hubset(QString* instruction = nullptr);
    void da_cogid(QString* instruction = nullptr);
    void da_cogstop(QString* instruction = nullptr);
    void da_locknew(QString* instruction = nullptr);
    void da_lockret(QString* instruction = nullptr);
    void da_locktry(QString* instruction = nullptr);
    void da_lockrel(QString* instruction = nullptr);
    void da_qlog(QString* instruction = nullptr);
    void da_qexp(QString* instruction = nullptr);

    void da_rfbyte(QString* instruction = nullptr);
    void da_rfword(QString* instruction = nullptr);
    void da_rflong(QString* instruction = nullptr);
    void da_rfvar(QString* instruction = nullptr);
    void da_rfvars(QString* instruction = nullptr);
    void da_wfbyte(QString* instruction = nullptr);
    void da_wfword(QString* instruction = nullptr);
    void da_wflong(QString* instruction = nullptr);

    void da_getqx(QString* instruction = nullptr);
    void da_getqy(QString* instruction = nullptr);
    void da_getct(QString* instruction = nullptr);
    void da_getrnd(QString* instruction = nullptr);
    void da_getrnd_cz(QString* instruction = nullptr);

    void da_setdacs(QString* instruction = nullptr);
    void da_setxfrq(QString* instruction = nullptr);
    void da_getxacc(QString* instruction = nullptr);
    void da_waitx(QString* instruction = nullptr);
    void da_setse1(QString* instruction = nullptr);
    void da_setse2(QString* instruction = nullptr);
    void da_setse3(QString* instruction = nullptr);
    void da_setse4(QString* instruction = nullptr);
    void da_pollint(QString* instruction = nullptr);
    void da_pollct1(QString* instruction = nullptr);
    void da_pollct2(QString* instruction = nullptr);
    void da_pollct3(QString* instruction = nullptr);
    void da_pollse1(QString* instruction = nullptr);
    void da_pollse2(QString* instruction = nullptr);
    void da_pollse3(QString* instruction = nullptr);
    void da_pollse4(QString* instruction = nullptr);
    void da_pollpat(QString* instruction = nullptr);
    void da_pollfbw(QString* instruction = nullptr);
    void da_pollxmt(QString* instruction = nullptr);
    void da_pollxfi(QString* instruction = nullptr);
    void da_pollxro(QString* instruction = nullptr);
    void da_pollxrl(QString* instruction = nullptr);
    void da_pollatn(QString* instruction = nullptr);
    void da_pollqmt(QString* instruction = nullptr);

    void da_waitint(QString* instruction = nullptr);
    void da_waitct1(QString* instruction = nullptr);
    void da_waitct2(QString* instruction = nullptr);
    void da_waitct3(QString* instruction = nullptr);
    void da_waitse1(QString* instruction = nullptr);
    void da_waitse2(QString* instruction = nullptr);
    void da_waitse3(QString* instruction = nullptr);
    void da_waitse4(QString* instruction = nullptr);
    void da_waitpat(QString* instruction = nullptr);
    void da_waitfbw(QString* instruction = nullptr);
    void da_waitxmt(QString* instruction = nullptr);
    void da_waitxfi(QString* instruction = nullptr);
    void da_waitxro(QString* instruction = nullptr);
    void da_waitxrl(QString* instruction = nullptr);
    void da_waitatn(QString* instruction = nullptr);

    void da_allowi(QString* instruction = nullptr);
    void da_stalli(QString* instruction = nullptr);
    void da_trgint1(QString* instruction = nullptr);
    void da_trgint2(QString* instruction = nullptr);
    void da_trgint3(QString* instruction = nullptr);
    void da_nixint1(QString* instruction = nullptr);
    void da_nixint2(QString* instruction = nullptr);
    void da_nixint3(QString* instruction = nullptr);
    void da_setint1(QString* instruction = nullptr);
    void da_setint2(QString* instruction = nullptr);
    void da_setint3(QString* instruction = nullptr);
    void da_setq(QString* instruction = nullptr);
    void da_setq2(QString* instruction = nullptr);

    void da_push(QString* instruction = nullptr);
    void da_pop(QString* instruction = nullptr);
    void da_jmp(QString* instruction = nullptr);
    void da_call(QString* instruction = nullptr);
    void da_ret(QString* instruction = nullptr);
    void da_calla(QString* instruction = nullptr);
    void da_reta(QString* instruction = nullptr);
    void da_callb(QString* instruction = nullptr);
    void da_retb(QString* instruction = nullptr);

    void da_jmprel(QString* instruction = nullptr);
    void da_skip(QString* instruction = nullptr);
    void da_skipf(QString* instruction = nullptr);
    void da_execf(QString* instruction = nullptr);
    void da_getptr(QString* instruction = nullptr);
    void da_getbrk(QString* instruction = nullptr);
    void da_cogbrk(QString* instruction = nullptr);
    void da_brk(QString* instruction = nullptr);

    void da_setluts(QString* instruction = nullptr);
    void da_setcy(QString* instruction = nullptr);
    void da_setci(QString* instruction = nullptr);
    void da_setcq(QString* instruction = nullptr);
    void da_setcfrq(QString* instruction = nullptr);
    void da_setcmod(QString* instruction = nullptr);
    void da_setpiv(QString* instruction = nullptr);
    void da_setpix(QString* instruction = nullptr);
    void da_cogatn(QString* instruction = nullptr);

    void da_testp_w(QString* instruction = nullptr);
    void da_testpn_w(QString* instruction = nullptr);
    void da_testp_and(QString* instruction = nullptr);
    void da_testpn_and(QString* instruction = nullptr);
    void da_testp_or(QString* instruction = nullptr);
    void da_testpn_or(QString* instruction = nullptr);
    void da_testp_xor(QString* instruction = nullptr);
    void da_testpn_xor(QString* instruction = nullptr);

    void da_dirl(QString* instruction = nullptr);
    void da_dirh(QString* instruction = nullptr);
    void da_dirc(QString* instruction = nullptr);
    void da_dirnc(QString* instruction = nullptr);
    void da_dirz(QString* instruction = nullptr);
    void da_dirnz(QString* instruction = nullptr);
    void da_dirrnd(QString* instruction = nullptr);
    void da_dirnot(QString* instruction = nullptr);

    void da_outl(QString* instruction = nullptr);
    void da_outh(QString* instruction = nullptr);
    void da_outc(QString* instruction = nullptr);
    void da_outnc(QString* instruction = nullptr);
    void da_outz(QString* instruction = nullptr);
    void da_outnz(QString* instruction = nullptr);
    void da_outrnd(QString* instruction = nullptr);
    void da_outnot(QString* instruction = nullptr);

    void da_fltl(QString* instruction = nullptr);
    void da_flth(QString* instruction = nullptr);
    void da_fltc(QString* instruction = nullptr);
    void da_fltnc(QString* instruction = nullptr);
    void da_fltz(QString* instruction = nullptr);
    void da_fltnz(QString* instruction = nullptr);
    void da_fltrnd(QString* instruction = nullptr);
    void da_fltnot(QString* instruction = nullptr);

    void da_drvl(QString* instruction = nullptr);
    void da_drvh(QString* instruction = nullptr);
    void da_drvc(QString* instruction = nullptr);
    void da_drvnc(QString* instruction = nullptr);
    void da_drvz(QString* instruction = nullptr);
    void da_drvnz(QString* instruction = nullptr);
    void da_drvrnd(QString* instruction = nullptr);
    void da_drvnot(QString* instruction = nullptr);

    void da_splitb(QString* instruction = nullptr);
    void da_mergeb(QString* instruction = nullptr);
    void da_splitw(QString* instruction = nullptr);
    void da_mergew(QString* instruction = nullptr);
    void da_seussf(QString* instruction = nullptr);
    void da_seussr(QString* instruction = nullptr);
    void da_rgbsqz(QString* instruction = nullptr);
    void da_rgbexp(QString* instruction = nullptr);
    void da_xoro32(QString* instruction = nullptr);

    void da_rev(QString* instruction = nullptr);
    void da_rczr(QString* instruction = nullptr);
    void da_rczl(QString* instruction = nullptr);
    void da_wrc(QString* instruction = nullptr);
    void da_wrnc(QString* instruction = nullptr);
    void da_wrz(QString* instruction = nullptr);
    void da_wrnz(QString* instruction = nullptr);
    void da_modcz(QString* instruction = nullptr);
    void da_setscp(QString* instruction = nullptr);
    void da_getscp(QString* instruction = nullptr);

    void da_jmp_abs(QString* instruction = nullptr);
    void da_call_abs(QString* instruction = nullptr);
    void da_calla_abs(QString* instruction = nullptr);
    void da_callb_abs(QString* instruction = nullptr);
    void da_calld_abs_pa(QString* instruction = nullptr);
    void da_calld_abs_pb(QString* instruction = nullptr);
    void da_calld_abs_ptra(QString* instruction = nullptr);
    void da_calld_abs_ptrb(QString* instruction = nullptr);
    void da_loc_pa(QString* instruction = nullptr);
    void da_loc_pb(QString* instruction = nullptr);
    void da_loc_ptra(QString* instruction = nullptr);
    void da_loc_ptrb(QString* instruction = nullptr);

    void da_augs(QString* instruction = nullptr);
    void da_AUGD(QString* instruction = nullptr);
};
