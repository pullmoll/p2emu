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

    void doc_NOP(p2_inst7_e instr);
    void doc_ROR(p2_inst7_e instr);
    void doc_ROL(p2_inst7_e instr);
    void doc_SHR(p2_inst7_e instr);
    void doc_SHL(p2_inst7_e instr);
    void doc_RCR(p2_inst7_e instr);
    void doc_RCL(p2_inst7_e instr);
    void doc_SAR(p2_inst7_e instr);
    void doc_SAL(p2_inst7_e instr);

    void doc_ADD(p2_inst7_e instr);
    void doc_ADDX(p2_inst7_e instr);
    void doc_ADDS(p2_inst7_e instr);
    void doc_ADDSX(p2_inst7_e instr);
    void doc_SUB(p2_inst7_e instr);
    void doc_SUBX(p2_inst7_e instr);
    void doc_SUBS(p2_inst7_e instr);
    void doc_SUBSX(p2_inst7_e instr);

    void doc_CMP(p2_inst7_e instr);
    void doc_CMPX(p2_inst7_e instr);
    void doc_CMPS(p2_inst7_e instr);
    void doc_CMPSX(p2_inst7_e instr);
    void doc_CMPR(p2_inst7_e instr);
    void doc_CMPM(p2_inst7_e instr);
    void doc_SUBR(p2_inst7_e instr);
    void doc_CMPSUB(p2_inst7_e instr);

    void doc_FGE(p2_inst7_e instr);
    void doc_FLE(p2_inst7_e instr);
    void doc_FGES(p2_inst7_e instr);
    void doc_FLES(p2_inst7_e instr);
    void doc_SUMC(p2_inst7_e instr);
    void doc_SUMNC(p2_inst7_e instr);
    void doc_SUMZ(p2_inst7_e instr);
    void doc_SUMNZ(p2_inst7_e instr);

    void doc_TESTB_W(p2_inst9_e instr);
    void doc_TESTBN_W(p2_inst9_e instr);
    void doc_TESTB_AND(p2_inst9_e instr);
    void doc_TESTBN_AND(p2_inst9_e instr);
    void doc_TESTB_OR(p2_inst9_e instr);
    void doc_TESTBN_OR(p2_inst9_e instr);
    void doc_TESTB_XOR(p2_inst9_e instr);
    void doc_TESTBN_XOR(p2_inst9_e instr);

    void doc_BITL(p2_inst9_e instr);
    void doc_BITH(p2_inst9_e instr);
    void doc_BITC(p2_inst9_e instr);
    void doc_BITNC(p2_inst9_e instr);
    void doc_BITZ(p2_inst9_e instr);
    void doc_BITNZ(p2_inst9_e instr);
    void doc_BITRND(p2_inst9_e instr);
    void doc_BITNOT(p2_inst9_e instr);

    void doc_AND(p2_inst7_e instr);
    void doc_ANDN(p2_inst7_e instr);
    void doc_OR(p2_inst7_e instr);
    void doc_XOR(p2_inst7_e instr);
    void doc_MUXC(p2_inst7_e instr);
    void doc_MUXNC(p2_inst7_e instr);
    void doc_MUXZ(p2_inst7_e instr);
    void doc_MUXNZ(p2_inst7_e instr);

    void doc_MOV(p2_inst7_e instr);
    void doc_NOT(p2_inst7_e instr);
    void doc_ABS(p2_inst7_e instr);
    void doc_NEG(p2_inst7_e instr);
    void doc_NEGC(p2_inst7_e instr);
    void doc_NEGNC(p2_inst7_e instr);
    void doc_NEGZ(p2_inst7_e instr);
    void doc_NEGNZ(p2_inst7_e instr);

    void doc_INCMOD(p2_inst7_e instr);
    void doc_DECMOD(p2_inst7_e instr);
    void doc_ZEROX(p2_inst7_e instr);
    void doc_SIGNX(p2_inst7_e instr);
    void doc_ENCOD(p2_inst7_e instr);
    void doc_ONES(p2_inst7_e instr);
    void doc_TEST(p2_inst7_e instr);
    void doc_TESTN(p2_inst7_e instr);

    void doc_SETNIB(p2_inst7_e instr);
    void doc_SETNIB_ALTSN(p2_inst7_e instr);
    void doc_GETNIB(p2_inst7_e instr);
    void doc_GETNIB_ALTGN(p2_inst7_e instr);
    void doc_ROLNIB(p2_inst7_e instr);
    void doc_ROLNIB_ALTGN(p2_inst7_e instr);
    void doc_SETBYTE(p2_inst7_e instr);
    void doc_SETBYTE_ALTSB(p2_inst7_e instr);
    void doc_GETBYTE(p2_inst7_e instr);
    void doc_GETBYTE_ALTGB(p2_inst7_e instr);
    void doc_ROLBYTE(p2_inst7_e instr);
    void doc_ROLBYTE_ALTGB(p2_inst7_e instr);

    void doc_SETWORD(p2_inst9_e instr);
    void doc_SETWORD_ALTSW(p2_inst9_e instr);
    void doc_GETWORD(p2_inst9_e instr);
    void doc_GETWORD_ALTGW(p2_inst9_e instr);
    void doc_ROLWORD(p2_inst9_e instr);
    void doc_ROLWORD_ALTGW(p2_inst9_e instr);

    void doc_ALTSN(p2_inst9_e instr);
    void doc_ALTSN_D(p2_inst9_e instr);
    void doc_ALTGN(p2_inst9_e instr);
    void doc_ALTGN_D(p2_inst9_e instr);
    void doc_ALTSB(p2_inst9_e instr);
    void doc_ALTSB_D(p2_inst9_e instr);
    void doc_ATLGB(p2_inst9_e instr);
    void doc_ALTGB_D(p2_inst9_e instr);

    void doc_ALTSW(p2_inst9_e instr);
    void doc_ALTSW_D(p2_inst9_e instr);
    void doc_ALTGW(p2_inst9_e instr);
    void doc_ALTGW_D(p2_inst9_e instr);

    void doc_ALTR(p2_inst9_e instr);
    void doc_ALTR_D(p2_inst9_e instr);
    void doc_ALTD(p2_inst9_e instr);
    void doc_ALTD_D(p2_inst9_e instr);
    void doc_ALTS(p2_inst9_e instr);
    void doc_ALTS_D(p2_inst9_e instr);
    void doc_ALTB(p2_inst9_e instr);
    void doc_ALTB_D(p2_inst9_e instr);
    void doc_ALTI(p2_inst9_e instr);
    void doc_ALTI_D(p2_inst9_e instr);

    void doc_SETR(p2_inst9_e instr);
    void doc_SETD(p2_inst9_e instr);
    void doc_SETS(p2_inst9_e instr);
    void doc_DECOD(p2_inst9_e instr);
    void doc_DECOD_D(p2_inst9_e instr);
    void doc_BMASK(p2_inst9_e instr);
    void doc_BMASK_D(p2_inst9_e instr);
    void doc_CRCBIT(p2_inst9_e instr);
    void doc_CRCNIB(p2_inst9_e instr);

    void doc_MUXNITS(p2_inst9_e instr);
    void doc_MUXNIBS(p2_inst9_e instr);
    void doc_MUXQ(p2_inst9_e instr);
    void doc_MOVBYTS(p2_inst9_e instr);
    void doc_MUL(p2_inst8_e instr);
    void doc_MULS(p2_inst8_e instr);
    void doc_SCA(p2_inst8_e instr);
    void doc_SCAS(p2_inst8_e instr);

    void doc_ADDPIX(p2_inst9_e instr);
    void doc_MULPIX(p2_inst9_e instr);
    void doc_BLNPIX(p2_inst9_e instr);
    void doc_MIXPIX(p2_inst9_e instr);
    void doc_ADDCT1(p2_inst9_e instr);
    void doc_ADDCT2(p2_inst9_e instr);
    void doc_ADDCT3(p2_inst9_e instr);

    void doc_WMLONG(p2_inst9_e instr);
    void doc_RQPIN(p2_inst8_e instr);
    void doc_RDPIN(p2_inst8_e instr);
    void doc_RDLUT(p2_inst7_e instr);
    void doc_RDBYTE(p2_inst7_e instr);
    void doc_RDWORD(p2_inst7_e instr);
    void doc_RDLONG(p2_inst7_e instr);

    void doc_POPA(p2_inst7_e instr);
    void doc_POPB(p2_inst7_e instr);
    void doc_CALLD(p2_inst7_e instr);
    void doc_RESI3(p2_inst9_e instr);
    void doc_RESI2(p2_inst9_e instr);
    void doc_RESI1(p2_inst9_e instr);
    void doc_RESI0(p2_inst9_e instr);
    void doc_RETI3(p2_inst7_e instr);
    void doc_RETI2(p2_inst7_e instr);
    void doc_RETI1(p2_inst7_e instr);
    void doc_RETI0(p2_inst7_e instr);
    void doc_CALLPA(p2_inst8_e instr);
    void doc_CALLPB(p2_inst8_e instr);

    void doc_DJZ(p2_inst9_e instr);
    void doc_DJNZ(p2_inst9_e instr);
    void doc_DJF(p2_inst9_e instr);
    void doc_DJNF(p2_inst9_e instr);
    void doc_IJZ(p2_inst9_e instr);
    void doc_IJNZ(p2_inst9_e instr);
    void doc_TJZ(p2_inst9_e instr);
    void doc_TJNZ(p2_inst9_e instr);
    void doc_TJF(p2_inst9_e instr);
    void doc_TJNF(p2_inst9_e instr);
    void doc_TJS(p2_inst9_e instr);
    void doc_TJNS(p2_inst9_e instr);
    void doc_TJV(p2_inst9_e instr);

    void doc_JINT(p2_opdst_e instr);
    void doc_JCT1(p2_opdst_e instr);
    void doc_JCT2(p2_opdst_e instr);
    void doc_JCT3(p2_opdst_e instr);
    void doc_JSE1(p2_opdst_e instr);
    void doc_JSE2(p2_opdst_e instr);
    void doc_JSE3(p2_opdst_e instr);
    void doc_JSE4(p2_opdst_e instr);
    void doc_JPAT(p2_opdst_e instr);
    void doc_JFBW(p2_opdst_e instr);
    void doc_JXMT(p2_opdst_e instr);
    void doc_JXFI(p2_opdst_e instr);
    void doc_JXRO(p2_opdst_e instr);
    void doc_JXRL(p2_opdst_e instr);
    void doc_JATN(p2_opdst_e instr);
    void doc_JQMT(p2_opdst_e instr);

    void doc_JNINT(p2_opdst_e instr);
    void doc_JNCT1(p2_opdst_e instr);
    void doc_JNCT2(p2_opdst_e instr);
    void doc_JNCT3(p2_opdst_e instr);
    void doc_JNSE1(p2_opdst_e instr);
    void doc_JNSE2(p2_opdst_e instr);
    void doc_JNSE3(p2_opdst_e instr);
    void doc_JNSE4(p2_opdst_e instr);
    void doc_JNPAT(p2_opdst_e instr);
    void doc_JNFBW(p2_opdst_e instr);
    void doc_JNXMT(p2_opdst_e instr);
    void doc_JNXFI(p2_opdst_e instr);
    void doc_JNXRO(p2_opdst_e instr);
    void doc_JNXRL(p2_opdst_e instr);
    void doc_JNATN(p2_opdst_e instr);
    void doc_JNQMT(p2_opdst_e instr);
    void doc_1011110_1(p2_inst8_e instr);
    void doc_1011111_0(p2_inst8_e instr);
    void doc_SETPAT(p2_inst8_e instr);

    void doc_WRPIN(p2_inst8_e instr);
    void doc_AKPIN(p2_inst8_e instr);
    void doc_WXPIN(p2_inst8_e instr);
    void doc_WYPIN(p2_inst8_e instr);
    void doc_WRLUT(p2_inst8_e instr);
    void doc_WRBYTE(p2_inst8_e instr);
    void doc_WRWORD(p2_inst8_e instr);
    void doc_WRLONG(p2_inst8_e instr);

    void doc_PUSHA(p2_inst8_e instr);
    void doc_PUSHB(p2_inst8_e instr);
    void doc_RDFAST(p2_inst8_e instr);
    void doc_WRFAST(p2_inst8_e instr);
    void doc_FBLOCK(p2_inst8_e instr);
    void doc_XINIT(p2_inst8_e instr);
    void doc_XSTOP(p2_inst8_e instr);
    void doc_XZERO(p2_inst8_e instr);
    void doc_XCONT(p2_inst8_e instr);

    void doc_REP(p2_inst8_e instr);
    void doc_COGINIT(p2_inst7_e instr);

    void doc_QMUL(p2_inst8_e instr);
    void doc_QDIV(p2_inst8_e instr);
    void doc_QFRAC(p2_inst8_e instr);
    void doc_QSQRT(p2_inst8_e instr);
    void doc_QROTATE(p2_inst8_e instr);
    void doc_QVECTOR(p2_inst8_e instr);

    void doc_HUBSET(p2_opsrc_e instr);
    void doc_COGID(p2_opsrc_e instr);
    void doc_COGSTOP(p2_opsrc_e instr);
    void doc_LOCKNEW(p2_opsrc_e instr);
    void doc_LOCKRET(p2_opsrc_e instr);
    void doc_LOCKTRY(p2_opsrc_e instr);
    void doc_LOCKREL(p2_opsrc_e instr);
    void doc_QLOG(p2_opsrc_e instr);
    void doc_QEXP(p2_opsrc_e instr);

    void doc_RFBYTE(p2_opsrc_e instr);
    void doc_RFWORD(p2_opsrc_e instr);
    void doc_RFLONG(p2_opsrc_e instr);
    void doc_RFVAR(p2_opsrc_e instr);
    void doc_RFVARS(p2_opsrc_e instr);
    void doc_WFBYTE(p2_opsrc_e instr);
    void doc_WFWORD(p2_opsrc_e instr);
    void doc_WFLONG(p2_opsrc_e instr);

    void doc_GETQX(p2_opsrc_e instr);
    void doc_GETQY(p2_opsrc_e instr);
    void doc_GETCT(p2_opsrc_e instr);
    void doc_GETRND(p2_opsrc_e instr);
    void doc_GETRND_CZ(p2_opsrc_e instr);

    void doc_SETDACS(p2_opsrc_e instr);
    void doc_SETXFRQ(p2_opsrc_e instr);
    void doc_GETXACC(p2_opsrc_e instr);
    void doc_WAITX(p2_opsrc_e instr);
    void doc_SETSE1(p2_opsrc_e instr);
    void doc_SETSE2(p2_opsrc_e instr);
    void doc_SETSE3(p2_opsrc_e instr);
    void doc_SETSE4(p2_opsrc_e instr);

    void doc_POLLINT(p2_opx24_e instr);
    void doc_POLLCT1(p2_opx24_e instr);
    void doc_POLLCT2(p2_opx24_e instr);
    void doc_POLLCT3(p2_opx24_e instr);
    void doc_POLLSE1(p2_opx24_e instr);
    void doc_POLLSE2(p2_opx24_e instr);
    void doc_POLLSE3(p2_opx24_e instr);
    void doc_POLLSE4(p2_opx24_e instr);
    void doc_POLLPAT(p2_opx24_e instr);
    void doc_POLLFBW(p2_opx24_e instr);
    void doc_POLLXMT(p2_opx24_e instr);
    void doc_POLLXFI(p2_opx24_e instr);
    void doc_POLLXRO(p2_opx24_e instr);
    void doc_POLLXRL(p2_opx24_e instr);
    void doc_POLLATN(p2_opx24_e instr);
    void doc_POLLQMT(p2_opx24_e instr);

    void doc_WAITINT(p2_opx24_e instr);
    void doc_WAITCT1(p2_opx24_e instr);
    void doc_WAITCT2(p2_opx24_e instr);
    void doc_WAITCT3(p2_opx24_e instr);
    void doc_WAITSE1(p2_opx24_e instr);
    void doc_WAITSE2(p2_opx24_e instr);
    void doc_WAITSE3(p2_opx24_e instr);
    void doc_WAITSE4(p2_opx24_e instr);
    void doc_WAITPAT(p2_opx24_e instr);
    void doc_WAITFBW(p2_opx24_e instr);
    void doc_WAITXMT(p2_opx24_e instr);
    void doc_WAITXFI(p2_opx24_e instr);
    void doc_WAITXRO(p2_opx24_e instr);
    void doc_WAITXRL(p2_opx24_e instr);
    void doc_WAITATN(p2_opx24_e instr);

    void doc_ALLOWI(p2_opx24_e instr);
    void doc_STALLI(p2_opx24_e instr);
    void doc_TRGINT1(p2_opx24_e instr);
    void doc_TRGINT2(p2_opx24_e instr);
    void doc_TRGINT3(p2_opx24_e instr);
    void doc_NIXINT1(p2_opx24_e instr);
    void doc_NIXINT2(p2_opx24_e instr);
    void doc_NIXINT3(p2_opx24_e instr);

    void doc_SETINT1(p2_opsrc_e instr);
    void doc_SETINT2(p2_opsrc_e instr);
    void doc_SETINT3(p2_opsrc_e instr);

    void doc_SETQ(p2_opsrc_e instr);
    void doc_SETQ2(p2_opsrc_e instr);

    void doc_PUSH(p2_opsrc_e instr);
    void doc_POP(p2_opsrc_e instr);
    void doc_JMP(p2_opsrc_e instr);
    void doc_CALL(p2_opsrc_e instr);
    void doc_RET(p2_opsrc_e instr);
    void doc_CALLA(p2_opsrc_e instr);
    void doc_RETA(p2_opsrc_e instr);
    void doc_CALLB(p2_opsrc_e instr);
    void doc_RETB(p2_opsrc_e instr);

    void doc_JMPREL(p2_opsrc_e instr);
    void doc_SKIP(p2_opsrc_e instr);
    void doc_SKIPF(p2_opsrc_e instr);
    void doc_EXECF(p2_opsrc_e instr);
    void doc_GETPTR(p2_opsrc_e instr);
    void doc_GETBRK(p2_opsrc_e instr);
    void doc_COGBRK(p2_opsrc_e instr);
    void doc_BRK(p2_opsrc_e instr);

    void doc_SETLUTS(p2_opsrc_e instr);
    void doc_SETCY(p2_opsrc_e instr);
    void doc_SETCI(p2_opsrc_e instr);
    void doc_SETCQ(p2_opsrc_e instr);
    void doc_SETCFRQ(p2_opsrc_e instr);
    void doc_SETCMOD(p2_opsrc_e instr);
    void doc_SETPIV(p2_opsrc_e instr);
    void doc_SETPIX(p2_opsrc_e instr);
    void doc_COGATN(p2_opsrc_e instr);

    void doc_TESTP_W(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_TESTPN_W(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_TESTP_AND(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_TESTPN_AND(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_TESTP_OR(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_TESTPN_OR(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_TESTP_XOR(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_TESTPN_XOR(p2_opsrc_e instr, p2_inst9_e inst9);

    void doc_DIRL(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_DIRH(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_DIRC(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_DIRNC(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_DIRZ(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_DIRNZ(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_DIRRND(p2_opsrc_e instr, p2_inst9_e inst9);
    void doc_DIRNOT(p2_opsrc_e instr, p2_inst9_e inst9);

    void doc_OUTL(p2_opsrc_e instr);
    void doc_OUTH(p2_opsrc_e instr);
    void doc_OUTC(p2_opsrc_e instr);
    void doc_OUTNC(p2_opsrc_e instr);
    void doc_OUTZ(p2_opsrc_e instr);
    void doc_OUTNZ(p2_opsrc_e instr);
    void doc_OUTRND(p2_opsrc_e instr);
    void doc_OUTNOT(p2_opsrc_e instr);

    void doc_FLTL(p2_opsrc_e instr);
    void doc_FLTH(p2_opsrc_e instr);
    void doc_FLTC(p2_opsrc_e instr);
    void doc_FLTNC(p2_opsrc_e instr);
    void doc_FLTZ(p2_opsrc_e instr);
    void doc_FLTNZ(p2_opsrc_e instr);
    void doc_FLTRND(p2_opsrc_e instr);
    void doc_FLTNOT(p2_opsrc_e instr);

    void doc_DRVL(p2_opsrc_e instr);
    void doc_DRVH(p2_opsrc_e instr);
    void doc_DRVC(p2_opsrc_e instr);
    void doc_DRVNC(p2_opsrc_e instr);
    void doc_DRVZ(p2_opsrc_e instr);
    void doc_DRVNZ(p2_opsrc_e instr);
    void doc_DRVRND(p2_opsrc_e instr);
    void doc_DRVNOT(p2_opsrc_e instr);

    void doc_SPLITB(p2_opsrc_e instr);
    void doc_MERGEB(p2_opsrc_e instr);
    void doc_SPLITW(p2_opsrc_e instr);
    void doc_MERGEW(p2_opsrc_e instr);
    void doc_SEUSSF(p2_opsrc_e instr);
    void doc_SEUSSR(p2_opsrc_e instr);
    void doc_RGBSQZ(p2_opsrc_e instr);
    void doc_RGBEXP(p2_opsrc_e instr);
    void doc_XORO32(p2_opsrc_e instr);

    void doc_REV(p2_opsrc_e instr);
    void doc_RCZR(p2_opsrc_e instr);
    void doc_RCZL(p2_opsrc_e instr);
    void doc_WRC(p2_opsrc_e instr);
    void doc_WRNC(p2_opsrc_e instr);
    void doc_WRZ(p2_opsrc_e instr);
    void doc_WRNZ(p2_opsrc_e instr);
    void doc_MODCZ(p2_opsrc_e instr);
    void doc_SETSCP(p2_opsrc_e instr);
    void doc_GETSCP(p2_opsrc_e instr);

    void doc_JMP_abs(p2_inst7_e instr);
    void doc_CALL_abs(p2_inst7_e instr);
    void doc_CALLA_abs(p2_inst7_e instr);
    void doc_CALLB_abs(p2_inst7_e instr);

    void doc_CALLD_abs(p2_inst5_e instr);
    void doc_LOC(p2_inst5_e instr);
    void doc_AUGS(p2_inst5_e instr);
    void doc_AUGD(p2_inst5_e instr);

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
