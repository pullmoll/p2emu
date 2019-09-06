/****************************************************************************
 *
 * Propeller2 instructions documentation class
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
#include <QMap>
#include <QHash>
#include "p2docopcode.h"
#include "p2opcode.h"

class P2Doc
{
public:

    P2Doc();

    const QStringList html_opcodes() const;
    const QStringList html_opcode(const p2_LONG opcode) const;

    const QString pattern(p2_LONG opcode) const;
    const QString brief(p2_LONG opcode) const;
    const QString instr(p2_LONG opcode) const;
    const QStringList descr(p2_LONG opcode) const;
    p2_token_e token(p2_LONG opcode);

private:
    static constexpr p2_LONG C_match     = p2_mask_C;                   //!< C == match in pattern
    static constexpr p2_LONG C_neq_Z     = p2_mask_CZ;                  //!< C != Z in pattern
    static constexpr p2_LONG C_equ_Z     = p2_mask_CZ;                  //!< C == Z in pattern
    static constexpr p2_LONG NNN_match   = p2_mask_NNN;                 //!< NNN nibble number in pattern
    static constexpr p2_LONG NN_match    = p2_mask_NN;                  //!< NN byte number in pattern
    static constexpr p2_LONG N_match     = p2_mask_N;                   //!< N word number in pattern
    static constexpr p2_LONG CZI_match   = p2_mask_CZI;                 //!< C,Z,I == match in pattern
    static constexpr p2_LONG CZ_match    = p2_mask_CZ;                  //!< C,Z == match in pattern
    static constexpr p2_LONG I_match     = p2_mask_I;                   //!< I == match in pattern
    static constexpr p2_LONG S_match     = p2_mask_S;                   //!< S == match in pattern
    static constexpr p2_LONG D_match     = p2_mask_D;                   //!< D == match in pattern
    static constexpr p2_LONG I_D_match   = p2_mask_I | p2_mask_D;       //!< I == match and D == match in pattern
    static constexpr p2_LONG D_S_match   = p2_mask_D | p2_mask_S;       //!< D == match and S == match in pattern
    static constexpr p2_LONG D_equ_S     = p2_mask_D | p2_mask_S;       //!< D == S in pattern
    static constexpr p2_LONG INST_5      = p2_mask_inst5;               //!< 5 bit inst in pattern
    static constexpr p2_LONG ALL_match   = FULL;                        //!< NOP

    QHash<P2MatchMask,P2DocOpcode> m_opcodes;                           //!< hash of P2DocOpcodes for P2MatchMask pairs
    QMultiHash<p2_LONG,P2MatchMask> m_matches;                          //!< multi hash of P2MatchMask pairs for masks
    QMap<uint,P2MatchMask> m_masks;                                     //!< map of number of 1 bits to match/mask pairs
    QList<uint> m_ones;                                                 //!< list of number of 1 bits

    static const QString format_pattern(const p2_LONG pattern, const QChar& zero = QChar('0'), const QChar& one = QChar('1'));

    const P2MatchMask opcode_matchmask(const p2_LONG opcode) const;
    const P2DocOpcode opcode_of(const p2_LONG opcode) const;
    const QStringList html_opcode(P2DocOpcode op) const;

    static p2_LONG opcode_inst5(const p2_inst5_e instr);
    static p2_LONG opcode_inst7(const p2_inst7_e instr);
    static p2_LONG opcode_inst8(const p2_inst8_e instr);
    static p2_LONG opcode_inst9(const p2_inst9_e instr);
    static p2_LONG opcode_opdst(const p2_opdst_e instr);
    static p2_LONG opcode_opsrc(const p2_opsrc_e instr);
    static p2_LONG opcode_opx24(const p2_opx24_e instr);

    const QString brief(p2_inst7_e inst) { return brief(opcode_inst7(inst)); }
    const QString brief(p2_inst8_e inst) { return brief(opcode_inst8(inst)); }
    const QString brief(p2_inst9_e inst) { return brief(opcode_inst9(inst)); }
    const QString brief(p2_opdst_e inst) { return brief(opcode_opdst(inst)); }
    const QString brief(p2_opsrc_e inst) { return brief(opcode_opsrc(inst)); }
    const QString brief(p2_opx24_e inst) { return brief(opcode_opx24(inst)); }

    const QString instr(p2_inst7_e inst) { return instr(opcode_inst7(inst)); }
    const QString instr(p2_inst8_e inst) { return instr(opcode_inst8(inst)); }
    const QString instr(p2_inst9_e inst) { return instr(opcode_inst9(inst)); }
    const QString instr(p2_opdst_e inst) { return instr(opcode_opdst(inst)); }
    const QString instr(p2_opsrc_e inst) { return instr(opcode_opsrc(inst)); }
    const QString instr(p2_opx24_e inst) { return instr(opcode_opx24(inst)); }

    const QStringList descr(p2_inst7_e inst) { return descr(opcode_inst7(inst)); }
    const QStringList descr(p2_inst8_e inst) { return descr(opcode_inst8(inst)); }
    const QStringList descr(p2_inst9_e inst) { return descr(opcode_inst9(inst)); }
    const QStringList descr(p2_opdst_e inst) { return descr(opcode_opdst(inst)); }
    const QStringList descr(p2_opsrc_e inst) { return descr(opcode_opsrc(inst)); }
    const QStringList descr(p2_opx24_e inst) { return descr(opcode_opx24(inst)); }

    p2_token_e token(p2_inst7_e inst) { return token(opcode_inst7(inst)); }
    p2_token_e token(p2_inst8_e inst) { return token(opcode_inst8(inst)); }
    p2_token_e token(p2_inst9_e inst) { return token(opcode_inst9(inst)); }
    p2_token_e token(p2_opdst_e inst) { return token(opcode_opdst(inst)); }
    p2_token_e token(p2_opsrc_e inst) { return token(opcode_opsrc(inst)); }
    p2_token_e token(p2_opx24_e inst) { return token(opcode_opx24(inst)); }

    P2DocOpcode make_pattern(const char* _func, p2_LONG instr, const char* src, const p2_LONG instmask, p2_LONG mode = 0);

    P2DocOpcode make_pattern(const char* _func, p2_inst5_e instr, const char* pat, p2_LONG mode = 0);
    P2DocOpcode make_pattern(const char* _func, p2_inst7_e instr, const char* pat, p2_LONG mode = 0);
    P2DocOpcode make_pattern(const char* _func, p2_inst8_e instr, const char* pat, p2_LONG mode = 0);
    P2DocOpcode make_pattern(const char* _func, p2_inst9_e instr, const char* pat, p2_LONG mode = 0);
    P2DocOpcode make_pattern(const char* _func, p2_opdst_e instr, const char* pat, p2_LONG mode = 0);
    P2DocOpcode make_pattern(const char* _func, p2_opsrc_e instr, const char* pat, p2_LONG mode = 0);
    P2DocOpcode make_pattern(const char* _func, p2_opx24_e instr, const char* pat, p2_LONG mode = 0);
    P2DocOpcode make_pattern(const char* _func, p2_opsrc_e instr, p2_inst9_e inst9, const char* pat, p2_LONG mode = 0);

    void doc_nop(p2_inst7_e instr);
    void doc_ror(p2_inst7_e instr);
    void doc_rol(p2_inst7_e instr);
    void doc_shr(p2_inst7_e instr);
    void doc_shl(p2_inst7_e instr);
    void doc_rcr(p2_inst7_e instr);
    void doc_rcl(p2_inst7_e instr);
    void doc_sar(p2_inst7_e instr);
    void doc_sal(p2_inst7_e instr);

    void doc_add(p2_inst7_e instr);
    void doc_addx(p2_inst7_e instr);
    void doc_adds(p2_inst7_e instr);
    void doc_addsx(p2_inst7_e instr);
    void doc_sub(p2_inst7_e instr);
    void doc_subx(p2_inst7_e instr);
    void doc_subs(p2_inst7_e instr);
    void doc_subsx(p2_inst7_e instr);

    void doc_cmp(p2_inst7_e instr);
    void doc_cmpx(p2_inst7_e instr);
    void doc_cmps(p2_inst7_e instr);
    void doc_cmpsx(p2_inst7_e instr);
    void doc_cmpr(p2_inst7_e instr);
    void doc_cmpm(p2_inst7_e instr);
    void doc_subr(p2_inst7_e instr);
    void doc_cmpsub(p2_inst7_e instr);

    void doc_fge(p2_inst7_e instr);
    void doc_fle(p2_inst7_e instr);
    void doc_fges(p2_inst7_e instr);
    void doc_fles(p2_inst7_e instr);
    void doc_sumc(p2_inst7_e instr);
    void doc_sumnc(p2_inst7_e instr);
    void doc_sumz(p2_inst7_e instr);
    void doc_sumnz(p2_inst7_e instr);

    void doc_testb_w(p2_inst9_e instr);
    void doc_testbn_w(p2_inst9_e instr);
    void doc_testb_and(p2_inst9_e instr);
    void doc_testbn_and(p2_inst9_e instr);
    void doc_testb_or(p2_inst9_e instr);
    void doc_testbn_or(p2_inst9_e instr);
    void doc_testb_xor(p2_inst9_e instr);
    void doc_testbn_xor(p2_inst9_e instr);

    void doc_bitl(p2_inst9_e instr);
    void doc_bith(p2_inst9_e instr);
    void doc_bitc(p2_inst9_e instr);
    void doc_bitnc(p2_inst9_e instr);
    void doc_bitz(p2_inst9_e instr);
    void doc_bitnz(p2_inst9_e instr);
    void doc_bitrnd(p2_inst9_e instr);
    void doc_bitnot(p2_inst9_e instr);

    void doc_and(p2_inst7_e instr);
    void doc_andn(p2_inst7_e instr);
    void doc_or(p2_inst7_e instr);
    void doc_xor(p2_inst7_e instr);
    void doc_muxc(p2_inst7_e instr);
    void doc_muxnc(p2_inst7_e instr);
    void doc_muxz(p2_inst7_e instr);
    void doc_muxnz(p2_inst7_e instr);

    void doc_mov(p2_inst7_e instr);
    void doc_not(p2_inst7_e instr);
    void doc_not_d(p2_inst7_e instr);
    void doc_abs(p2_inst7_e instr);
    void doc_neg(p2_inst7_e instr);
    void doc_negc(p2_inst7_e instr);
    void doc_negnc(p2_inst7_e instr);
    void doc_negz(p2_inst7_e instr);
    void doc_negnz(p2_inst7_e instr);

    void doc_incmod(p2_inst7_e instr);
    void doc_decmod(p2_inst7_e instr);
    void doc_zerox(p2_inst7_e instr);
    void doc_signx(p2_inst7_e instr);
    void doc_encod(p2_inst7_e instr);
    void doc_encod_d(p2_inst7_e instr);
    void doc_ones(p2_inst7_e instr);
    void doc_test(p2_inst7_e instr);
    void doc_testn(p2_inst7_e instr);

    void doc_setnib(p2_inst7_e instr);
    void doc_setnib_altsn(p2_inst7_e instr);
    void doc_getnib(p2_inst7_e instr);
    void doc_getnib_altgn(p2_inst7_e instr);
    void doc_rolnib(p2_inst7_e instr);
    void doc_rolnib_altgn(p2_inst7_e instr);
    void doc_setbyte(p2_inst7_e instr);
    void doc_setbyte_altsb(p2_inst7_e instr);
    void doc_getbyte(p2_inst7_e instr);
    void doc_getbyte_altgb(p2_inst7_e instr);
    void doc_rolbyte(p2_inst7_e instr);
    void doc_rolbyte_altgb(p2_inst7_e instr);

    void doc_setword(p2_inst9_e instr);
    void doc_setword_altsw(p2_inst9_e instr);
    void doc_getword(p2_inst9_e instr);
    void doc_getword_altgw(p2_inst9_e instr);
    void doc_rolword(p2_inst9_e instr);
    void doc_rolword_altgw(p2_inst9_e instr);

    void doc_altsn(p2_inst9_e instr);
    void doc_altsn_d(p2_inst9_e instr);
    void doc_altgn(p2_inst9_e instr);
    void doc_altgn_d(p2_inst9_e instr);
    void doc_altsb(p2_inst9_e instr);
    void doc_altsb_d(p2_inst9_e instr);
    void doc_altgb(p2_inst9_e instr);
    void doc_altgb_d(p2_inst9_e instr);

    void doc_altsw(p2_inst9_e instr);
    void doc_altsw_d(p2_inst9_e instr);
    void doc_altgw(p2_inst9_e instr);
    void doc_altgw_d(p2_inst9_e instr);

    void doc_altr(p2_inst9_e instr);
    void doc_altr_d(p2_inst9_e instr);
    void doc_altd(p2_inst9_e instr);
    void doc_altd_d(p2_inst9_e instr);
    void doc_alts(p2_inst9_e instr);
    void doc_alts_d(p2_inst9_e instr);
    void doc_altb(p2_inst9_e instr);
    void doc_altb_d(p2_inst9_e instr);
    void doc_alti(p2_inst9_e instr);
    void doc_alti_d(p2_inst9_e instr);

    void doc_setr(p2_inst9_e instr);
    void doc_setd(p2_inst9_e instr);
    void doc_sets(p2_inst9_e instr);
    void doc_decod(p2_inst9_e instr);
    void doc_decod_d(p2_inst9_e instr);
    void doc_bmask(p2_inst9_e instr);
    void doc_bmask_d(p2_inst9_e instr);
    void doc_crcbit(p2_inst9_e instr);
    void doc_crcnib(p2_inst9_e instr);

    void doc_muxnits(p2_inst9_e instr);
    void doc_muxnibs(p2_inst9_e instr);
    void doc_muxq(p2_inst9_e instr);
    void doc_movbyts(p2_inst9_e instr);
    void doc_mul(p2_inst8_e instr);
    void doc_muls(p2_inst8_e instr);
    void doc_sca(p2_inst8_e instr);
    void doc_scas(p2_inst8_e instr);

    void doc_addpix(p2_inst9_e instr);
    void doc_mulpix(p2_inst9_e instr);
    void doc_blnpix(p2_inst9_e instr);
    void doc_mixpix(p2_inst9_e instr);
    void doc_addct1(p2_inst9_e instr);
    void doc_addct2(p2_inst9_e instr);
    void doc_addct3(p2_inst9_e instr);

    void doc_wmlong(p2_inst9_e instr);
    void doc_rqpin(p2_inst8_e instr);
    void doc_rdpin(p2_inst8_e instr);
    void doc_rdlut(p2_inst7_e instr);
    void doc_rdbyte(p2_inst7_e instr);
    void doc_rdword(p2_inst7_e instr);
    void doc_rdlong(p2_inst7_e instr);

    void doc_popa(p2_inst7_e instr);
    void doc_popb(p2_inst7_e instr);
    void doc_calld(p2_inst7_e instr);
    void doc_resi3(p2_inst9_e instr);
    void doc_resi2(p2_inst9_e instr);
    void doc_resi1(p2_inst9_e instr);
    void doc_resi0(p2_inst9_e instr);
    void doc_reti3(p2_inst7_e instr);
    void doc_reti2(p2_inst7_e instr);
    void doc_reti1(p2_inst7_e instr);
    void doc_reti0(p2_inst7_e instr);
    void doc_callpa(p2_inst8_e instr);
    void doc_callpb(p2_inst8_e instr);

    void doc_djz(p2_inst9_e instr);
    void doc_djnz(p2_inst9_e instr);
    void doc_djf(p2_inst9_e instr);
    void doc_djnf(p2_inst9_e instr);
    void doc_ijz(p2_inst9_e instr);
    void doc_ijnz(p2_inst9_e instr);
    void doc_tjz(p2_inst9_e instr);
    void doc_tjnz(p2_inst9_e instr);
    void doc_tjf(p2_inst9_e instr);
    void doc_tjnf(p2_inst9_e instr);
    void doc_tjs(p2_inst9_e instr);
    void doc_tjns(p2_inst9_e instr);
    void doc_tjv(p2_inst9_e instr);

    void doc_jint(p2_opdst_e instr);
    void doc_jct1(p2_opdst_e instr);
    void doc_jct2(p2_opdst_e instr);
    void doc_jct3(p2_opdst_e instr);
    void doc_jse1(p2_opdst_e instr);
    void doc_jse2(p2_opdst_e instr);
    void doc_jse3(p2_opdst_e instr);
    void doc_jse4(p2_opdst_e instr);
    void doc_jpat(p2_opdst_e instr);
    void doc_jfbw(p2_opdst_e instr);
    void doc_jxmt(p2_opdst_e instr);
    void doc_jxfi(p2_opdst_e instr);
    void doc_jxro(p2_opdst_e instr);
    void doc_jxrl(p2_opdst_e instr);
    void doc_jatn(p2_opdst_e instr);
    void doc_jqmt(p2_opdst_e instr);

    void doc_jnint(p2_opdst_e instr);
    void doc_jnct1(p2_opdst_e instr);
    void doc_jnct2(p2_opdst_e instr);
    void doc_jnct3(p2_opdst_e instr);
    void doc_jnse1(p2_opdst_e instr);
    void doc_jnse2(p2_opdst_e instr);
    void doc_jnse3(p2_opdst_e instr);
    void doc_jnse4(p2_opdst_e instr);
    void doc_jnpat(p2_opdst_e instr);
    void doc_jnfbw(p2_opdst_e instr);
    void doc_jnxmt(p2_opdst_e instr);
    void doc_jnxfi(p2_opdst_e instr);
    void doc_jnxro(p2_opdst_e instr);
    void doc_jnxrl(p2_opdst_e instr);
    void doc_jnatn(p2_opdst_e instr);
    void doc_jnqmt(p2_opdst_e instr);
    void doc_1011110_1(p2_inst8_e instr);
    void doc_1011111_0(p2_inst8_e instr);
    void doc_setpat(p2_inst8_e instr);

    void doc_wrpin(p2_inst8_e instr);
    void doc_akpin(p2_inst8_e instr);
    void doc_wxpin(p2_inst8_e instr);
    void doc_wypin(p2_inst8_e instr);
    void doc_wrlut(p2_inst8_e instr);
    void doc_wrbyte(p2_inst8_e instr);
    void doc_wrword(p2_inst8_e instr);
    void doc_wrlong(p2_inst8_e instr);

    void doc_pusha(p2_inst8_e instr);
    void doc_pushb(p2_inst8_e instr);
    void doc_rdfast(p2_inst8_e instr);
    void doc_wrfast(p2_inst8_e instr);
    void doc_fblock(p2_inst8_e instr);
    void doc_xinit(p2_inst8_e instr);
    void doc_xstop(p2_inst8_e instr);
    void doc_xzero(p2_inst8_e instr);
    void doc_xcont(p2_inst8_e instr);

    void doc_rep(p2_inst8_e instr);
    void doc_coginit(p2_inst7_e instr);

    void doc_qmul(p2_inst8_e instr);
    void doc_qdiv(p2_inst8_e instr);
    void doc_qfrac(p2_inst8_e instr);
    void doc_qsqrt(p2_inst8_e instr);
    void doc_qrotate(p2_inst8_e instr);
    void doc_qvector(p2_inst8_e instr);

    void doc_hubset(p2_opsrc_e instr);
    void doc_cogid(p2_opsrc_e instr);
    void doc_cogstop(p2_opsrc_e instr);
    void doc_locknew(p2_opsrc_e instr);
    void doc_lockret(p2_opsrc_e instr);
    void doc_locktry(p2_opsrc_e instr);
    void doc_lockrel(p2_opsrc_e instr);
    void doc_qlog(p2_opsrc_e instr);
    void doc_qexp(p2_opsrc_e instr);

    void doc_rfbyte(p2_opsrc_e instr);
    void doc_rfword(p2_opsrc_e instr);
    void doc_rflong(p2_opsrc_e instr);
    void doc_rfvar(p2_opsrc_e instr);
    void doc_rfvars(p2_opsrc_e instr);
    void doc_wfbyte(p2_opsrc_e instr);
    void doc_wfword(p2_opsrc_e instr);
    void doc_wflong(p2_opsrc_e instr);

    void doc_getqx(p2_opsrc_e instr);
    void doc_getqy(p2_opsrc_e instr);
    void doc_getct(p2_opsrc_e instr);
    void doc_getrnd(p2_opsrc_e instr);
    void doc_getrnd_cz(p2_opsrc_e instr);

    void doc_setdacs(p2_opsrc_e instr);
    void doc_setxfrq(p2_opsrc_e instr);
    void doc_getxacc(p2_opsrc_e instr);
    void doc_waitx(p2_opsrc_e instr);
    void doc_setse1(p2_opsrc_e instr);
    void doc_setse2(p2_opsrc_e instr);
    void doc_setse3(p2_opsrc_e instr);
    void doc_setse4(p2_opsrc_e instr);

    void doc_pollint(p2_opx24_e instr);
    void doc_pollct1(p2_opx24_e instr);
    void doc_pollct2(p2_opx24_e instr);
    void doc_pollct3(p2_opx24_e instr);
    void doc_pollse1(p2_opx24_e instr);
    void doc_pollse2(p2_opx24_e instr);
    void doc_pollse3(p2_opx24_e instr);
    void doc_pollse4(p2_opx24_e instr);
    void doc_pollpat(p2_opx24_e instr);
    void doc_pollfbw(p2_opx24_e instr);
    void doc_pollxmt(p2_opx24_e instr);
    void doc_pollxfi(p2_opx24_e instr);
    void doc_pollxro(p2_opx24_e instr);
    void doc_pollxrl(p2_opx24_e instr);
    void doc_pollatn(p2_opx24_e instr);
    void doc_pollqmt(p2_opx24_e instr);

    void doc_waitint(p2_opx24_e instr);
    void doc_waitct1(p2_opx24_e instr);
    void doc_waitct2(p2_opx24_e instr);
    void doc_waitct3(p2_opx24_e instr);
    void doc_waitse1(p2_opx24_e instr);
    void doc_waitse2(p2_opx24_e instr);
    void doc_waitse3(p2_opx24_e instr);
    void doc_waitse4(p2_opx24_e instr);
    void doc_waitpat(p2_opx24_e instr);
    void doc_waitfbw(p2_opx24_e instr);
    void doc_waitxmt(p2_opx24_e instr);
    void doc_waitxfi(p2_opx24_e instr);
    void doc_waitxro(p2_opx24_e instr);
    void doc_waitxrl(p2_opx24_e instr);
    void doc_waitatn(p2_opx24_e instr);

    void doc_allowi(p2_opx24_e instr);
    void doc_stalli(p2_opx24_e instr);
    void doc_trgint1(p2_opx24_e instr);
    void doc_trgint2(p2_opx24_e instr);
    void doc_trgint3(p2_opx24_e instr);
    void doc_nixint1(p2_opx24_e instr);
    void doc_nixint2(p2_opx24_e instr);
    void doc_nixint3(p2_opx24_e instr);

    void doc_setint1(p2_opsrc_e instr);
    void doc_setint2(p2_opsrc_e instr);
    void doc_setint3(p2_opsrc_e instr);

    void doc_setq(p2_opsrc_e instr);
    void doc_setq2(p2_opsrc_e instr);

    void doc_push(p2_opsrc_e instr);
    void doc_pop(p2_opsrc_e instr);
    void doc_jmp(p2_opsrc_e instr);
    void doc_call(p2_opsrc_e instr);
    void doc_ret(p2_opsrc_e instr);
    void doc_calla(p2_opsrc_e instr);
    void doc_reta(p2_opsrc_e instr);
    void doc_callb(p2_opsrc_e instr);
    void doc_retb(p2_opsrc_e instr);

    void doc_jmprel(p2_opsrc_e instr);
    void doc_skip(p2_opsrc_e instr);
    void doc_skipf(p2_opsrc_e instr);
    void doc_execf(p2_opsrc_e instr);
    void doc_getptr(p2_opsrc_e instr);
    void doc_getbrk(p2_opsrc_e instr);
    void doc_cogbrk(p2_opsrc_e instr);
    void doc_brk(p2_opsrc_e instr);

    void doc_setluts(p2_opsrc_e instr);
    void doc_setcy(p2_opsrc_e instr);
    void doc_setci(p2_opsrc_e instr);
    void doc_setcq(p2_opsrc_e instr);
    void doc_setcfrq(p2_opsrc_e instr);
    void doc_setcmod(p2_opsrc_e instr);
    void doc_setpiv(p2_opsrc_e instr);
    void doc_setpix(p2_opsrc_e instr);
    void doc_cogatn(p2_opsrc_e instr);

    void doc_testp_wc_wz(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_testpn_wc_wz(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_testp_and(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_testpn_and(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_testp_or(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_testpn_or(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_testp_xor(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_testpn_xor(p2_opsrc_e instr, p2_inst9_e inst9);

    void doc_dirl(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_dirh(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_dirc(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_dirnc(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_dirz(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_dirnz(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_dirrnd(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_dirnot(p2_opsrc_e instr, p2_inst9_e inst9);

    void doc_outl(p2_opsrc_e instr);
    void doc_outh(p2_opsrc_e instr);
    void doc_outc(p2_opsrc_e instr);
    void doc_outnc(p2_opsrc_e instr);
    void doc_outz(p2_opsrc_e instr);
    void doc_outnz(p2_opsrc_e instr);
    void doc_outrnd(p2_opsrc_e instr);
    void doc_outnot(p2_opsrc_e instr);

    void doc_fltl(p2_opsrc_e instr);
    void doc_flth(p2_opsrc_e instr);
    void doc_fltc(p2_opsrc_e instr);
    void doc_fltnc(p2_opsrc_e instr);
    void doc_fltz(p2_opsrc_e instr);
    void doc_fltnz(p2_opsrc_e instr);
    void doc_fltrnd(p2_opsrc_e instr);
    void doc_fltnot(p2_opsrc_e instr);

    void doc_drvl(p2_opsrc_e instr);
    void doc_drvh(p2_opsrc_e instr);
    void doc_drvc(p2_opsrc_e instr);
    void doc_drvnc(p2_opsrc_e instr);
    void doc_drvz(p2_opsrc_e instr);
    void doc_drvnz(p2_opsrc_e instr);
    void doc_drvrnd(p2_opsrc_e instr);
    void doc_drvnot(p2_opsrc_e instr);

    void doc_splitb(p2_opsrc_e instr);
    void doc_mergeb(p2_opsrc_e instr);
    void doc_splitw(p2_opsrc_e instr);
    void doc_mergew(p2_opsrc_e instr);
    void doc_seussf(p2_opsrc_e instr);
    void doc_seussr(p2_opsrc_e instr);
    void doc_rgbsqz(p2_opsrc_e instr);
    void doc_rgbexp(p2_opsrc_e instr);
    void doc_xoro32(p2_opsrc_e instr);

    void doc_rev(p2_opsrc_e instr);
    void doc_rczr(p2_opsrc_e instr);
    void doc_rczl(p2_opsrc_e instr);
    void doc_wrc(p2_opsrc_e instr);
    void doc_wrnc(p2_opsrc_e instr);
    void doc_wrz(p2_opsrc_e instr);
    void doc_wrnz(p2_opsrc_e instr);
    void doc_modcz(p2_opsrc_e instr);
    void doc_setscp(p2_opsrc_e instr);
    void doc_getscp(p2_opsrc_e instr);

    void doc_jmp_abs(p2_inst7_e instr);
    void doc_call_abs(p2_inst7_e instr);
    void doc_calla_abs(p2_inst7_e instr);
    void doc_callb_abs(p2_inst7_e instr);

    void doc_calld_abs(p2_inst5_e instr);
    void doc_loc(p2_inst5_e instr);
    void doc_augs(p2_inst5_e instr);
    void doc_augd(p2_inst5_e instr);
    void params_dst(P2DocOpcode& op, P2Opcode::ImmFlag flag);
    void params_src(P2DocOpcode& op, P2Opcode::ImmFlag flag);
    void mandatory_COMMA(P2DocOpcode& op);
    void optional_COMMA(P2DocOpcode& op);
    void optional_WCZ(P2DocOpcode& op);
    void optional_WC(P2DocOpcode& op);
    void optional_WZ(P2DocOpcode& op);
    void mandatory_ANDC_ANDZ(P2DocOpcode& op);
    void mandatory_ORC_ORZ(P2DocOpcode& op);
    void mandatory_XORC_XORZ(P2DocOpcode& op);
    void params_D_IM_S(P2DocOpcode& op);
    void params_D_WCZ(P2DocOpcode& op);
    void params_WCZ(P2DocOpcode& op);
    void params_D(P2DocOpcode& op);
    void params_WZ_D(P2DocOpcode& op);
    void params_IM_D(P2DocOpcode& op);
    void params_IM_D_WCZ(P2DocOpcode& op);
    void params_IM_D_WC(P2DocOpcode& op);
    void params_IM_D_ANDC_ANDZ(P2DocOpcode& op);
    void params_IM_D_ORC_ORZ(P2DocOpcode& op);
    void params_IM_D_XORC_XORZ(P2DocOpcode& op);
    void params_D_IM_S_WCZ(P2DocOpcode& op);
    void params_D_IM_S_ANDCZ(P2DocOpcode& op);
    void params_D_IM_S_ORCZ(P2DocOpcode& op);
    void params_D_IM_S_XORCZ(P2DocOpcode& op);
    void params_D_IM_S_WC(P2DocOpcode& op);
    void params_D_IM_S_WZ(P2DocOpcode& op);
    void params_WZ_D_IM_S(P2DocOpcode& op);
    void params_WZ_D_IM_S_WC(P2DocOpcode& op);
    void params_D_IM_S_NNN(P2DocOpcode& op, uint max);
    void params_IM_S(P2DocOpcode& op);
    void params_IM_S_WC(P2DocOpcode& op);
    void params_PTRx_PC_A20(P2DocOpcode& op);
    void params_PC_A20(P2DocOpcode& op);
    void params_IMM23(P2DocOpcode& op);
};

extern P2Doc Doc;
