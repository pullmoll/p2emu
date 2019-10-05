/****************************************************************************
 *
 * Propeller2 instructions documentation implementation
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
#include "p2doc.h"
#include "p2tokens.h"
#include "p2util.h"
#include "p2doc.h"
#include "p2html.h"

// Global instance
P2Doc Doc;

P2Doc::P2Doc()
    : m_opcodes()
    , m_masks()
{
    doc_NOP(p2_ROR);
    doc_ROR(p2_ROR);
    doc_ROL(p2_ROL);
    doc_SHR(p2_SHR);
    doc_SHL(p2_SHL);
    doc_RCR(p2_RCR);
    doc_RCL(p2_RCL);
    doc_SAR(p2_SAR);
    doc_SAL(p2_SAL);
    doc_ADD(p2_ADD);
    doc_ADDX(p2_ADDX);
    doc_ADDS(p2_ADDS);
    doc_ADDSX(p2_ADDSX);
    doc_SUB(p2_SUB);
    doc_SUBX(p2_SUBX);
    doc_SUBS(p2_SUBS);
    doc_SUBSX(p2_SUBSX);
    doc_CMP(p2_CMP);
    doc_CMPX(p2_CMPX);
    doc_CMPS(p2_CMPS);
    doc_CMPSX(p2_CMPSX);
    doc_CMPR(p2_CMPR);
    doc_CMPM(p2_CMPM);
    doc_SUBR(p2_SUBR);
    doc_CMPSUB(p2_CMPSUB);
    doc_FGE(p2_FGE);
    doc_FLE(p2_FLE);
    doc_FGES(p2_FGES);
    doc_FLES(p2_FLES);
    doc_SUMC(p2_SUMC);
    doc_SUMNC(p2_SUMNC);
    doc_SUMZ(p2_SUMZ);
    doc_SUMNZ(p2_SUMNZ);

    doc_TESTB_W(p2_TESTB_WC);
    doc_TESTB_W(p2_TESTB_WZ);
    doc_TESTBN_W(p2_TESTBN_WZ);
    doc_TESTBN_W(p2_TESTBN_WC);
    doc_TESTB_AND(p2_TESTB_ANDZ);
    doc_TESTB_AND(p2_TESTB_ANDC);
    doc_TESTBN_AND(p2_TESTBN_ANDZ);
    doc_TESTBN_AND(p2_TESTBN_ANDC);
    doc_TESTB_OR(p2_TESTB_ORC);
    doc_TESTB_OR(p2_TESTB_ORZ);
    doc_TESTBN_OR(p2_TESTBN_ORC);
    doc_TESTBN_OR(p2_TESTBN_ORZ);
    doc_TESTB_XOR(p2_TESTB_XORC);
    doc_TESTB_XOR(p2_TESTB_XORZ);
    doc_TESTBN_XOR(p2_TESTBN_XORC);
    doc_TESTBN_XOR(p2_TESTBN_XORZ);

    doc_BITL(p2_BITL);
    doc_BITL(p2_BITL_WCZ);
    doc_BITH(p2_BITH);
    doc_BITH(p2_BITH_WCZ);
    doc_BITC(p2_BITC);
    doc_BITC(p2_BITC_WCZ);
    doc_BITNC(p2_BITNC);
    doc_BITNC(p2_BITNC_WCZ);
    doc_BITZ(p2_BITZ);
    doc_BITZ(p2_BITZ_WCZ);
    doc_BITNZ(p2_BITNZ);
    doc_BITNZ(p2_BITNZ_WCZ);
    doc_BITRND(p2_BITRND);
    doc_BITRND(p2_BITRND_WCZ);
    doc_BITNOT(p2_BITNOT);
    doc_BITNOT(p2_BITNOT_WCZ);

    doc_AND(p2_AND);
    doc_ANDN(p2_ANDN);
    doc_OR(p2_OR);
    doc_XOR(p2_XOR);
    doc_MUXC(p2_MUXC);
    doc_MUXNC(p2_MUXNC);
    doc_MUXZ(p2_MUXZ);
    doc_MUXNZ(p2_MUXNZ);
    doc_MOV(p2_MOV);
    doc_NOT(p2_NOT);
    doc_ABS(p2_ABS);
    doc_NEG(p2_NEG);
    doc_NEGC(p2_NEGC);

    doc_NEGNC(p2_NEGNC);
    doc_NEGZ(p2_NEGZ);
    doc_NEGNZ(p2_NEGNZ);
    doc_INCMOD(p2_INCMOD);
    doc_DECMOD(p2_DECMOD);
    doc_ZEROX(p2_ZEROX);
    doc_SIGNX(p2_SIGNX);
    doc_ENCOD(p2_ENCOD);
    doc_ONES(p2_ONES);

    doc_TEST(p2_TEST);
    doc_TESTN(p2_TESTN);

    doc_SETNIB(p2_SETNIB_0);
    doc_SETNIB(p2_SETNIB_1);
    doc_SETNIB(p2_SETNIB_2);
    doc_SETNIB(p2_SETNIB_3);
    doc_SETNIB(p2_SETNIB_4);
    doc_SETNIB(p2_SETNIB_5);
    doc_SETNIB(p2_SETNIB_6);
    doc_SETNIB(p2_SETNIB_7);
    doc_SETNIB_ALTSN(p2_SETNIB_0_3);

    doc_GETNIB(p2_GETNIB_0);
    doc_GETNIB(p2_GETNIB_1);
    doc_GETNIB(p2_GETNIB_2);
    doc_GETNIB(p2_GETNIB_3);
    doc_GETNIB(p2_GETNIB_4);
    doc_GETNIB(p2_GETNIB_5);
    doc_GETNIB(p2_GETNIB_6);
    doc_GETNIB(p2_GETNIB_7);
    doc_GETNIB_ALTGN(p2_GETNIB_0_3);

    doc_ROLNIB(p2_ROLNIB_0);
    doc_ROLNIB(p2_ROLNIB_1);
    doc_ROLNIB(p2_ROLNIB_2);
    doc_ROLNIB(p2_ROLNIB_3);
    doc_ROLNIB(p2_ROLNIB_4);
    doc_ROLNIB(p2_ROLNIB_5);
    doc_ROLNIB(p2_ROLNIB_6);
    doc_ROLNIB(p2_ROLNIB_7);
    doc_ROLNIB_ALTGN(p2_ROLNIB_0_3);

    doc_SETBYTE(p2_SETBYTE_0);
    doc_SETBYTE(p2_SETBYTE_1);
    doc_SETBYTE(p2_SETBYTE_2);
    doc_SETBYTE(p2_SETBYTE_3);
    doc_SETBYTE_ALTSB(p2_SETBYTE_0_3);

    doc_GETBYTE(p2_GETBYTE_0);
    doc_GETBYTE(p2_GETBYTE_1);
    doc_GETBYTE(p2_GETBYTE_2);
    doc_GETBYTE(p2_GETBYTE_3);
    doc_GETBYTE_ALTGB(p2_GETBYTE_0_3);

    doc_ROLBYTE(p2_ROLBYTE_0);
    doc_ROLBYTE(p2_ROLBYTE_1);
    doc_ROLBYTE(p2_ROLBYTE_2);
    doc_ROLBYTE(p2_ROLBYTE_3);
    doc_ROLBYTE_ALTGB(p2_ROLBYTE_0_3);

    doc_SETWORD_ALTSW(p2_SETWORD_ALTSW);
    doc_SETWORD(p2_SETWORD);
    doc_GETWORD_ALTGW(p2_GETWORD_ALTGW);
    doc_GETWORD(p2_GETWORD);
    doc_ROLWORD_ALTGW(p2_ROLWORD_ALTGW);
    doc_ROLWORD(p2_ROLWORD);
    doc_ALTSN(p2_ALTSN);
    doc_ALTSN_D(p2_ALTSN);
    doc_ALTGN(p2_ALTGN);
    doc_ALTGN_D(p2_ALTGN);

    doc_ALTSB(p2_ALTSB);
    doc_ALTSB_D(p2_ALTSB);
    doc_ATLGB(p2_ALTGB);
    doc_ALTGB_D(p2_ALTGB);
    doc_ALTSW(p2_ALTSW);
    doc_ALTSW_D(p2_ALTSW);
    doc_ALTGW(p2_ALTGW);
    doc_ALTGW_D(p2_ALTGW);

    doc_ALTR(p2_ALTR);
    doc_ALTR_D(p2_ALTR);
    doc_ALTD(p2_ALTD);
    doc_ALTD_D(p2_ALTD);
    doc_ALTS(p2_ALTS);
    doc_ALTS_D(p2_ALTS);
    doc_ALTB(p2_ALTB);
    doc_ALTB_D(p2_ALTB);

    doc_ALTI(p2_ALTI);
    doc_ALTI_D(p2_ALTI);
    doc_SETR(p2_SETR);
    doc_SETD(p2_SETD);
    doc_SETS(p2_SETS);

    // doc_DECOD_D(p2_DECOD);
    doc_DECOD(p2_DECOD);
    // doc_BMASK_D(p2_BMASK);
    doc_BMASK(p2_BMASK);
    doc_CRCBIT(p2_CRCBIT);
    doc_CRCNIB(p2_CRCNIB);

    doc_MUXNITS(p2_MUXNITS);
    doc_MUXNIBS(p2_MUXNIBS);
    doc_MUXQ(p2_MUXQ);
    doc_MOVBYTS(p2_MOVBYTS);

    doc_MUL(p2_MUL);
    doc_MULS(p2_MULS);

    doc_SCA(p2_SCA);
    doc_SCAS(p2_SCAS);

    doc_ADDPIX(p2_ADDPIX);
    doc_MULPIX(p2_MULPIX);
    doc_BLNPIX(p2_BLNPIX);
    doc_MIXPIX(p2_MIXPIX);

    doc_ADDCT1(p2_ADDCT1);
    doc_ADDCT2(p2_ADDCT2);
    doc_ADDCT3(p2_ADDCT3);
    doc_WMLONG(p2_WMLONG);

    doc_RQPIN(p2_RQPIN);
    doc_RDPIN(p2_RDPIN);

    doc_RDLUT(p2_RDLUT);
    doc_RDBYTE(p2_RDBYTE);
    doc_RDWORD(p2_RDWORD);

    doc_RDLONG(p2_RDLONG);

    doc_CALLD(p2_CALLD);
    doc_RESI3(p2_RESI3);
    doc_RESI2(p2_RESI2);
    doc_RESI1(p2_RESI1);
    doc_RESI0(p2_RESI0);

    doc_CALLPA(p2_CALLPA);
    doc_CALLPB(p2_CALLPB);

    doc_DJZ(p2_DJZ);
    doc_DJNZ(p2_DJNZ);
    doc_DJF(p2_DJF);
    doc_DJNF(p2_DJNF);
    doc_IJZ(p2_IJZ);
    doc_IJNZ(p2_IJNZ);
    doc_TJZ(p2_TJZ);
    doc_TJNZ(p2_TJNZ);
    doc_TJF(p2_TJF);
    doc_TJNF(p2_TJNF);
    doc_TJS(p2_TJS);
    doc_TJNS(p2_TJNS);
    doc_TJV(p2_TJV);

    doc_JINT(p2_OPDST_JINT);
    doc_JCT1(p2_OPDST_JCT1);
    doc_JCT2(p2_OPDST_JCT2);
    doc_JCT3(p2_OPDST_JCT3);
    doc_JSE1(p2_OPDST_JSE1);
    doc_JSE2(p2_OPDST_JSE2);
    doc_JSE3(p2_OPDST_JSE3);
    doc_JSE4(p2_OPDST_JSE4);
    doc_JPAT(p2_OPDST_JPAT);
    doc_JFBW(p2_OPDST_JFBW);
    doc_JXMT(p2_OPDST_JXMT);
    doc_JXFI(p2_OPDST_JXFI);
    doc_JXRO(p2_OPDST_JXRO);
    doc_JXRL(p2_OPDST_JXRL);
    doc_JATN(p2_OPDST_JATN);
    doc_JQMT(p2_OPDST_JQMT);

    doc_JNINT(p2_OPDST_JNINT);
    doc_JNCT1(p2_OPDST_JNCT1);
    doc_JNCT2(p2_OPDST_JNCT2);
    doc_JNCT3(p2_OPDST_JNCT3);
    doc_JNSE1(p2_OPDST_JNSE1);
    doc_JNSE2(p2_OPDST_JNSE2);
    doc_JNSE3(p2_OPDST_JNSE3);
    doc_JNSE4(p2_OPDST_JNSE4);
    doc_JNPAT(p2_OPDST_JNPAT);
    doc_JNFBW(p2_OPDST_JNFBW);
    doc_JNXMT(p2_OPDST_JNXMT);
    doc_JNXFI(p2_OPDST_JNXFI);
    doc_JNXRO(p2_OPDST_JNXRO);
    doc_JNXRL(p2_OPDST_JNXRL);
    doc_JNATN(p2_OPDST_JNATN);
    doc_JNQMT(p2_OPDST_JNQMT);
    doc_1011110_1(p2_1011110_1);

    doc_1011111_0(p2_1011111_0);
    doc_SETPAT(p2_SETPAT);

    doc_AKPIN(p2_WRPIN);
    doc_WRPIN(p2_WRPIN);
    doc_WXPIN(p2_WXPIN);
    doc_WYPIN(p2_WYPIN);
    doc_WRLUT(p2_WRLUT);
    doc_WRBYTE(p2_WRBYTE);
    doc_WRWORD(p2_WRWORD);
    doc_WRLONG(p2_WRLONG);
    doc_RDFAST(p2_RDFAST);
    doc_WRFAST(p2_WRFAST);
    doc_FBLOCK(p2_FBLOCK);

    doc_XSTOP(p2_XINIT);
    doc_XINIT(p2_XINIT);
    doc_XZERO(p2_XZERO);
    doc_XCONT(p2_XCONT);
    doc_REP(p2_REP);
    doc_COGINIT(p2_COGINIT);
    doc_QMUL(p2_QMUL);
    doc_QDIV(p2_QDIV);
    doc_QFRAC(p2_QFRAC);
    doc_QSQRT(p2_QSQRT);
    doc_QROTATE(p2_QROTATE);
    doc_QVECTOR(p2_QVECTOR);

    doc_HUBSET(p2_OPSRC_HUBSET);
    doc_COGID(p2_OPSRC_COGID);
    doc_COGSTOP(p2_OPSRC_COGSTOP);
    doc_LOCKNEW(p2_OPSRC_LOCKNEW);
    doc_LOCKRET(p2_OPSRC_LOCKRET);
    doc_LOCKTRY(p2_OPSRC_LOCKTRY);
    doc_LOCKREL(p2_OPSRC_LOCKREL);
    doc_QLOG(p2_OPSRC_QLOG);
    doc_QEXP(p2_OPSRC_QEXP);

    doc_RFBYTE(p2_OPSRC_RFBYTE);
    doc_RFWORD(p2_OPSRC_RFWORD);
    doc_RFLONG(p2_OPSRC_RFLONG);
    doc_RFVAR(p2_OPSRC_RFVAR);
    doc_RFVARS(p2_OPSRC_RFVARS);
    doc_WFBYTE(p2_OPSRC_WFBYTE);
    doc_WFWORD(p2_OPSRC_WFWORD);
    doc_WFLONG(p2_OPSRC_WFLONG);

    doc_GETQX(p2_OPSRC_GETQX);
    doc_GETQY(p2_OPSRC_GETQY);
    doc_GETCT(p2_OPSRC_GETCT);
    doc_GETRND_CZ(p2_OPSRC_GETRND);
    doc_GETRND(p2_OPSRC_GETRND);
    doc_SETDACS(p2_OPSRC_SETDACS);
    doc_SETXFRQ(p2_OPSRC_SETXFRQ);
    doc_GETXACC(p2_OPSRC_GETXACC);
    doc_WAITX(p2_OPSRC_WAITX);
    doc_SETSE1(p2_OPSRC_SETSE1);
    doc_SETSE2(p2_OPSRC_SETSE2);
    doc_SETSE3(p2_OPSRC_SETSE3);
    doc_SETSE4(p2_OPSRC_SETSE4);

    doc_POLLINT(p2_OPX24_POLLINT);
    doc_POLLCT1(p2_OPX24_POLLCT1);
    doc_POLLCT2(p2_OPX24_POLLCT2);
    doc_POLLCT3(p2_OPX24_POLLCT3);
    doc_POLLSE1(p2_OPX24_POLLSE1);
    doc_POLLSE2(p2_OPX24_POLLSE2);
    doc_POLLSE3(p2_OPX24_POLLSE3);
    doc_POLLSE4(p2_OPX24_POLLSE4);
    doc_POLLPAT(p2_OPX24_POLLPAT);
    doc_POLLFBW(p2_OPX24_POLLFBW);
    doc_POLLXMT(p2_OPX24_POLLXMT);
    doc_POLLXFI(p2_OPX24_POLLXFI);
    doc_POLLXRO(p2_OPX24_POLLXRO);
    doc_POLLXRL(p2_OPX24_POLLXRL);
    doc_POLLATN(p2_OPX24_POLLATN);
    doc_POLLQMT(p2_OPX24_POLLQMT);
    doc_WAITINT(p2_OPX24_WAITINT);
    doc_WAITCT1(p2_OPX24_WAITCT1);
    doc_WAITCT2(p2_OPX24_WAITCT2);
    doc_WAITCT3(p2_OPX24_WAITCT3);
    doc_WAITSE1(p2_OPX24_WAITSE1);
    doc_WAITSE2(p2_OPX24_WAITSE2);
    doc_WAITSE3(p2_OPX24_WAITSE3);
    doc_WAITSE4(p2_OPX24_WAITSE4);
    doc_WAITPAT(p2_OPX24_WAITPAT);
    doc_WAITFBW(p2_OPX24_WAITFBW);
    doc_WAITXMT(p2_OPX24_WAITXMT);
    doc_WAITXFI(p2_OPX24_WAITXFI);
    doc_WAITXRO(p2_OPX24_WAITXRO);
    doc_WAITXRL(p2_OPX24_WAITXRL);
    doc_WAITATN(p2_OPX24_WAITATN);
    doc_ALLOWI(p2_OPX24_ALLOWI);
    doc_STALLI(p2_OPX24_STALLI);
    doc_TRGINT1(p2_OPX24_TRGINT1);
    doc_TRGINT2(p2_OPX24_TRGINT2);
    doc_TRGINT3(p2_OPX24_TRGINT3);
    doc_NIXINT1(p2_OPX24_NIXINT1);
    doc_NIXINT2(p2_OPX24_NIXINT2);
    doc_NIXINT3(p2_OPX24_NIXINT3);

    doc_SETINT1(p2_OPSRC_SETINT1);
    doc_SETINT2(p2_OPSRC_SETINT2);
    doc_SETINT3(p2_OPSRC_SETINT3);
    doc_SETQ(p2_OPSRC_SETQ);
    doc_SETQ2(p2_OPSRC_SETQ2);
    doc_PUSH(p2_OPSRC_PUSH);
    doc_POP(p2_OPSRC_POP);
    doc_JMP(p2_OPSRC_JMP);
    doc_CALL(p2_OPSRC_CALL_RET);
    doc_RET(p2_OPSRC_CALL_RET);
    doc_CALLA(p2_OPSRC_CALLA_RETA);
    doc_RETA(p2_OPSRC_CALLA_RETA);
    doc_CALLB(p2_OPSRC_CALLB_RETB);
    doc_RETB(p2_OPSRC_CALLB_RETB);
    doc_JMPREL(p2_OPSRC_JMPREL);
    doc_SKIP(p2_OPSRC_SKIP);
    doc_SKIPF(p2_OPSRC_SKIPF);
    doc_EXECF(p2_OPSRC_EXECF);
    doc_GETPTR(p2_OPSRC_GETPTR);
    doc_COGBRK(p2_OPSRC_COGBRK);
    doc_GETBRK(p2_OPSRC_COGBRK);
    doc_BRK(p2_OPSRC_BRK);
    doc_SETLUTS(p2_OPSRC_SETLUTS);
    doc_SETCY(p2_OPSRC_SETCY);
    doc_SETCI(p2_OPSRC_SETCI);
    doc_SETCQ(p2_OPSRC_SETCQ);
    doc_SETCFRQ(p2_OPSRC_SETCFRQ);
    doc_SETCMOD(p2_OPSRC_SETCMOD);
    doc_SETPIV(p2_OPSRC_SETPIV);
    doc_SETPIX(p2_OPSRC_SETPIX);
    doc_COGATN(p2_OPSRC_COGATN);

    doc_DIRL(p2_OPSRC_DIRL, p2_DIRL);
    doc_DIRL(p2_OPSRC_DIRL, p2_DIRL_WCZ);
    doc_DIRH(p2_OPSRC_DIRH, p2_DIRH);
    doc_DIRH(p2_OPSRC_DIRH, p2_DIRH_WCZ);
    doc_DIRC(p2_OPSRC_DIRC, p2_DIRC);
    doc_DIRC(p2_OPSRC_DIRC, p2_DIRC_WCZ);
    doc_DIRNC(p2_OPSRC_DIRNC, p2_DIRNC);
    doc_DIRNC(p2_OPSRC_DIRNC, p2_DIRNC_WCZ);
    doc_DIRZ(p2_OPSRC_DIRZ, p2_DIRZ);
    doc_DIRZ(p2_OPSRC_DIRZ, p2_DIRZ_WCZ);
    doc_DIRNZ(p2_OPSRC_DIRNZ, p2_DIRNZ);
    doc_DIRNZ(p2_OPSRC_DIRNZ, p2_DIRNZ_WCZ);
    doc_DIRRND(p2_OPSRC_DIRRND, p2_DIRRND);
    doc_DIRRND(p2_OPSRC_DIRRND, p2_DIRRND_WCZ);
    doc_DIRNOT(p2_OPSRC_DIRNOT, p2_DIRNOT);
    doc_DIRNOT(p2_OPSRC_DIRNOT, p2_DIRNOT_WCZ);

    doc_TESTP_W(p2_OPSRC_TESTP_W, p2_TESTP_WC);
    doc_TESTP_W(p2_OPSRC_TESTP_W, p2_TESTP_WZ);
    doc_TESTPN_W(p2_OPSRC_TESTPN_W, p2_TESTPN_WC);
    doc_TESTPN_W(p2_OPSRC_TESTPN_W, p2_TESTPN_WZ);
    doc_TESTP_AND(p2_OPSRC_TESTP_AND, p2_TESTP_ANDC);
    doc_TESTP_AND(p2_OPSRC_TESTP_AND, p2_TESTP_ANDZ);
    doc_TESTPN_AND(p2_OPSRC_TESTPN_AND, p2_TESTPN_ANDC);
    doc_TESTPN_AND(p2_OPSRC_TESTPN_AND, p2_TESTPN_ANDZ);
    doc_TESTP_OR(p2_OPSRC_TESTP_OR, p2_TESTP_ORC);
    doc_TESTP_OR(p2_OPSRC_TESTP_OR, p2_TESTP_ORZ);
    doc_TESTPN_OR(p2_OPSRC_TESTPN_OR, p2_TESTPN_ORC);
    doc_TESTPN_OR(p2_OPSRC_TESTPN_OR, p2_TESTPN_ORZ);
    doc_TESTP_XOR(p2_OPSRC_TESTP_XOR, p2_TESTP_XORC);
    doc_TESTP_XOR(p2_OPSRC_TESTP_XOR, p2_TESTP_XORZ);
    doc_TESTPN_XOR(p2_OPSRC_TESTPN_XOR, p2_TESTPN_XORC);
    doc_TESTPN_XOR(p2_OPSRC_TESTPN_XOR, p2_TESTPN_XORZ);

    doc_OUTL(p2_OPSRC_OUTL);
    doc_OUTH(p2_OPSRC_OUTH);
    doc_OUTC(p2_OPSRC_OUTC);
    doc_OUTNC(p2_OPSRC_OUTNC);
    doc_OUTZ(p2_OPSRC_OUTZ);
    doc_OUTNZ(p2_OPSRC_OUTNZ);
    doc_OUTRND(p2_OPSRC_OUTRND);
    doc_OUTNOT(p2_OPSRC_OUTNOT);

    doc_FLTL(p2_OPSRC_FLTL);
    doc_FLTH(p2_OPSRC_FLTH);
    doc_FLTC(p2_OPSRC_FLTC);
    doc_FLTNC(p2_OPSRC_FLTNC);
    doc_FLTZ(p2_OPSRC_FLTZ);
    doc_FLTNZ(p2_OPSRC_FLTNZ);
    doc_FLTRND(p2_OPSRC_FLTRND);
    doc_FLTNOT(p2_OPSRC_FLTNOT);

    doc_DRVL(p2_OPSRC_DRVL);
    doc_DRVH(p2_OPSRC_DRVH);
    doc_DRVC(p2_OPSRC_DRVC);
    doc_DRVNC(p2_OPSRC_DRVNC);
    doc_DRVZ(p2_OPSRC_DRVZ);
    doc_DRVNZ(p2_OPSRC_DRVNZ);
    doc_DRVRND(p2_OPSRC_DRVRND);
    doc_DRVNOT(p2_OPSRC_DRVNOT);

    doc_SPLITB(p2_OPSRC_SPLITB);
    doc_MERGEB(p2_OPSRC_MERGEB);
    doc_SPLITW(p2_OPSRC_SPLITW);
    doc_MERGEW(p2_OPSRC_MERGEW);
    doc_SEUSSF(p2_OPSRC_SEUSSF);
    doc_SEUSSR(p2_OPSRC_SEUSSR);
    doc_RGBSQZ(p2_OPSRC_RGBSQZ);
    doc_RGBEXP(p2_OPSRC_RGBEXP);

    doc_XORO32(p2_OPSRC_XORO32);
    doc_REV(p2_OPSRC_REV);
    doc_RCZR(p2_OPSRC_RCZR);
    doc_RCZL(p2_OPSRC_RCZL);
    doc_WRC(p2_OPSRC_WRC);
    doc_WRNC(p2_OPSRC_WRNC);
    doc_WRZ(p2_OPSRC_WRZ);
    doc_MODCZ(p2_OPSRC_WRNZ_MODCZ);
    doc_WRNZ(p2_OPSRC_WRNZ_MODCZ);
    doc_SETSCP(p2_OPSRC_SETSCP);
    doc_GETSCP(p2_OPSRC_GETSCP);

    doc_JMP_abs(p2_JMP_ABS);
    doc_CALL_abs(p2_CALL_ABS);
    doc_CALLA_abs(p2_CALLA_ABS);
    doc_CALLB_abs(p2_CALLB_ABS);

    doc_CALLD_abs(p2_CALLD_ABS);
    doc_LOC(p2_LOC);
    doc_AUGS(p2_AUGS);
    doc_AUGD(p2_AUGD);

    foreach(P2DocOpcode op, m_opcodes) {
        P2MatchMask matchmask = op->matchmask();
        uchar ones = P2Util::ones(~matchmask.mask());
        m_masks.insert(ones, matchmask);
    }

    qDebug("%s: opcodes = %d", __func__, m_opcodes.count());
    qDebug("%s: masks = %d", __func__, m_masks.count());
}

static void put_spaces(QString& mask)
{
    mask.insert( 4, QChar(' '));
    mask.insert(12, QChar(' '));
    mask.insert(16, QChar(' '));
    mask.insert(26, QChar(' '));
}

/**
 * @brief Format a 32 bit value pattern according to the Propeller2 format
 *
 * EEEE OOOOOOO CZI DDDDDDDDD SSSSSSSSS
 *
 * @param pattern
 * @return formatted string
 */
const QString P2Doc::format_pattern(const p2_LONG pattern, const QChar& zero, const QChar& one, const p2_LONG ignore)
{
    QString result = QString("%1").arg(pattern, 32, 2, QChar('0'));
    for (int i = 0; i < 32; i++) {
        if ((ignore << i) & MSB)
            result[i] = QChar(L'·');
    }
    put_spaces(result);

    if (zero != QChar('0'))
        result.replace(QChar('0'), zero);
    if (one != QChar('1'))
        result.replace(QChar('1'), one);
    result += QString(" ($%1)").arg(pattern, 8, 16, QChar('0'));
    return result;
}

const P2DocOpcode P2Doc::opcode_of(const p2_LONG opcode) const
{
    foreach(P2MatchMask matchmask, m_masks.values()) {
        matchmask.set_masked_match(opcode);
        P2DocOpcode op = m_opcodes.value(matchmask);
        if (!op.isNull() && op->isDefined())
            return op;
    }
    return P2DocOpcode();
}

const P2MatchMask P2Doc::opcode_matchmask(const p2_LONG opcode) const
{
    P2DocOpcode op = opcode_of(opcode);
    if (!op.isNull() && op->isDefined())
        return op->matchmask();
    return P2MatchMask();
}

QDomDocumentFragment P2Doc::doc_opcode(QDomDocument& doc, P2DocOpcode op) const
{
    QDomDocumentFragment node = doc.createDocumentFragment();
    if (op.isNull() || !op->isDefined())
        return node;

    QDomElement h2, h3, pre, table, tr, td, tt, strong;
    QDomText text;

    h2 = p2_html(doc,"h2");
    h2.setAttribute(attr_style, attr_style_nowrap);
    text = p2_text(doc, op->instr());
    h2.appendChild(text);
    node.appendChild(h2);

    h3 = p2_html(doc,"h3");
    h3.setAttribute(attr_style, attr_style_nowrap);
    text = p2_text(doc, op->brief());
    h3.appendChild(text);
    node.appendChild(h3);

    table = p2_html(doc,"table");
    table.setAttribute(attr_width, attr_95percent);

    tr = p2_html(doc,"tr");
    td = p2_html(doc,"td");
    td.setAttribute(attr_width, attr_10percent);
    text = p2_text(doc, QStringLiteral("Pattern"));
    td.appendChild(text);
    tr.appendChild(td);

    td = p2_html(doc,"td");
    tt = p2_html(doc,"tt");
    text = p2_text(doc, op->pattern());
    tt.appendChild(text);
    td.appendChild(tt);
    tr.appendChild(td);
    table.appendChild(tr);

    tr = p2_html(doc,"tr");
    td = p2_html(doc,"td");
    td.setAttribute(attr_width, attr_10percent);
    text = p2_text(doc, QStringLiteral("Mask"));
    td.appendChild(text);
    tr.appendChild(td);

    td = p2_html(doc,"td");
    tt = p2_html(doc,"tt");
    text = p2_text(doc, format_pattern(op->mask(), QChar(L'·'), QChar('X')));
    tt.appendChild(text);
    td.appendChild(tt);
    tr.appendChild(td);
    table.appendChild(tr);

    tr = p2_html(doc,"tr");
    td = p2_html(doc,"td");
    td.setAttribute(attr_width, attr_10percent);
    text = p2_text(doc, QStringLiteral("Match"));
    td.appendChild(text);
    tr.appendChild(td);

    td = p2_html(doc,"td");
    tt = p2_html(doc,"tt");
    text = p2_text(doc, format_pattern(op->match(), QChar('0'), QChar('1'), ~op->mask()));
    tt.appendChild(text);
    td.appendChild(tt);
    tr.appendChild(td);
    table.appendChild(tr);

    node.appendChild(table);

    strong = p2_html(doc,"strong");
    pre = p2_html(doc,"pre");
    text = p2_text(doc, QString("    %1").arg(op->instr()));
    pre.appendChild(text);
    strong.appendChild(pre);
    node.appendChild(strong);

    if (!op->descr().isEmpty()) {
        pre = p2_html(doc,"pre");
        foreach(const QString& line, op->descr()) {
            text = p2_text(doc, QString("%1\n").arg(line));
            pre.appendChild(text);
        }
        node.appendChild(pre);
    }

    return node;
}

const QStringList P2Doc::html_opcode(const p2_LONG opcode) const
{
    P2DocOpcode op = opcode_of(opcode);
    if (op.isNull() || !op->isDefined())
        return QStringList();

    QDomDocument doc;
    QDomElement root, head, title, body, table;
    QDomElement tr, td, hr;
    QDomText text;

    root = p2_html(doc,"html");
    head = p2_html(doc,"head");
    title = p2_html(doc,"title");
    title.appendChild(p2_text(doc, QStringLiteral("Propeller2 opcode")));
    head.appendChild(title);
    root.appendChild(head);

    body = p2_html(doc,"body");
    table = p2_html(doc,"table");
    table.setAttribute(attr_width, attr_95percent);
    tr = p2_html(doc,"tr");
    td = p2_html(doc,"td");
    td.appendChild(doc_opcode(doc, op));
    tr.appendChild(td);
    table.appendChild(tr);
    body.appendChild(table);
    root.appendChild(body);
    doc.appendChild(root);

    return doc.toString(1).split(QChar::LineFeed);
}

const QString P2Doc::pattern(p2_LONG opcode) const
{
    P2DocOpcode op = opcode_of(opcode);
    if (op.isNull() || !op->isDefined())
        return QString();
    return op->pattern();
}

/**
 * @brief Create a HTML document describing the opcodes
 * @return QStringList with HTML lines
 */
const QStringList P2Doc::html_opcodes() const
{
    QDomDocument doc;
    QDomElement root, head, title, body, table;
    QDomElement tr, td, hr;
    QDomText text;

    root = p2_html(doc,"html");
    head = p2_head(doc);
    title = p2_title(doc);
    text = p2_text(doc, QStringLiteral("Propeller2 opcodes"));
    title.appendChild(text);
    head.appendChild(title);
    root.appendChild(head);

    body = p2_body(doc);
    table = p2_table(doc);
    table.setAttribute(attr_width, attr_95percent);


    foreach(const P2DocOpcode& op, m_opcodes.values()) {

        tr = p2_tr(doc);
        td = p2_td(doc);
        hr = p2_hr(doc);
        // hr.setAttribute(attr_width, attr_95percent);
        td.appendChild(hr);
        tr.appendChild(td);
        table.appendChild(tr);

        tr = p2_tr(doc);
        td = p2_td(doc);
        td.appendChild(doc_opcode(doc, op));
        tr.appendChild(td);
        table.appendChild(tr);
    }
    body.appendChild(table);
    root.appendChild(body);
    doc.appendChild(root);

    return doc.toString(1).split(QChar::LineFeed);
}

/**
 * @brief Return the brief description for an instruction
 * @param instr 32 bit value of the masked opcode
 * @return pointer to brief descr, or nullptr if none exists
 */
const QString P2Doc::brief(p2_LONG opcode) const
{
    P2DocOpcode op = opcode_of(opcode);
    if (!op.isNull() && op->isDefined())
        return op->brief();
    return QString();
}

/**
 * @brief Return the assembler source example for an instruction
 * @param instr 32 bit value of the masked opcode
 * @return pointer to instruction example, or nullptr if none exists
 */
const QString P2Doc::instr(p2_LONG opcode) const
{
    P2DocOpcode op = opcode_of(opcode);
    if (!op.isNull() && op->isDefined())
        return op->instr();
    return QString();
}

/**
 * @brief Return the effects description for an instruction
 * @param instr 32 bit value of the masked opcode
 * @return QList of pointers to the effects description
 */
const QStringList P2Doc::descr(p2_LONG opcode) const
{
    P2DocOpcode op = opcode_of(opcode);
    if (!op.isNull() && op->isDefined())
        return op->descr();
    return QStringList();
}

/**
 * @brief Return the token value for the mnemonic of an instruction
 * @param instr 32 bit value of the masked opcode
 * @return token value for the instruction, or t_invalid if none exists
 */
p2_TOKEN_e P2Doc::token(p2_LONG opcode)
{
    P2DocOpcode op = opcode_of(opcode);
    if (!op.isNull() && op->isDefined())
        return op->token();
    return t_invalid;
}

/**
 * @brief Build the opcode for a p2_inst5_e enumeration value
 * @param instr instruction's enumeration value
 * @return adjusted opcode
 */
p2_LONG P2Doc::opcode_inst5(const p2_INST5_e instr)
{
    p2_opcode_u op = {0};
    op.op5.inst = static_cast<uint>(instr);
    return op.opcode;
}

/**
 * @brief Build the opcode for a p2_inst7_e enumeration value
 * @param instr instruction's enumeration value
 * @return adjusted opcode
 */
p2_LONG P2Doc::opcode_inst7(const p2_INST7_e instr)
{
    p2_opcode_u op = {0};
    op.op7.inst = static_cast<uint>(instr);
    return op.opcode;
}

/**
 * @brief Build the opcode for a p2_inst8_e enumeration value
 * @param instr instruction's enumeration value
 * @return adjusted opcode
 */
p2_LONG P2Doc::opcode_inst8(const p2_INST8_e instr)
{
    p2_opcode_u op = {0};
    op.op8.inst = static_cast<uint>(instr);
    return op.opcode;
}

/**
 * @brief Build the opcode for a p2_inst9_e enumeration value
 * @param instr instruction's enumeration value
 * @return adjusted opcode
 */
p2_LONG P2Doc::opcode_inst9(const p2_INST9_e instr)
{
    p2_opcode_u op = {0};
    op.op9.inst = static_cast<uint>(instr);
    return op.opcode;
}

/**
 * @brief Build the opcode for a p2_opdst_e enumeration value
 * @param instr instruction's enumeration value
 * @return adjusted opcode
 */
p2_LONG P2Doc::opcode_opdst(const p2_OPDST_e instr)
{
    p2_opcode_u op = {0};
    op.op9.inst = static_cast<uint>(p2_OPDST);
    op.op9.dst = static_cast<uint>(instr);
    return op.opcode;
}


/**
 * @brief Build the opcode for a p2_opsrc_e enumeration value
 * @param instr instruction's enumeration value
 * @return adjusted opcode
 */
p2_LONG P2Doc::opcode_opsrc(const p2_OPSRC_e instr)
{
    p2_opcode_u op = {0};
    op.op7.inst = static_cast<uint>(p2_OPSRC);
    op.op7.src = static_cast<uint>(instr);
    return op.opcode;
}

/**
 * @brief Build the opcode for a p2_opx24_e enumeration value
 * @param instr instruction's enumeration value
 * @return adjusted opcode
 */
p2_LONG P2Doc::opcode_opx24(const p2_OPX24_e instr)
{
    p2_opcode_u op = {0};
    op.op7.inst = static_cast<uint>(p2_OPSRC);
    op.op7.dst = static_cast<uint>(instr);
    op.op7.src = static_cast<uint>(p2_OPSRC_X24);
    return op.opcode;
}

/**
 * @brief Return a mask / match for an instruction's pattern
 *
 * Meaning of the special characters in the pattern strings:
 * E    conditional execution flags (EEEE)
 * C    with carry flag (WC)
 * Z    with zero flag (WZ)
 * I    immediate flag for source (#S)
 * L    immediate flag for destination (#D)
 * N    nibble, byte, word index or absolute address (NNN, NN, N, ...)
 * D    destination register / immediate value (DDDDDDDDD)
 * S    source register / immediate value (SSSSSSSSS)
 * c    MODCZ condition for the C flag (cccc)
 * z    MODCZ condition for the Z flag (zzzz)
 * R    relative 20 bit address flag (R)
 * A    20 bit address (AA AAAAAAAAA AAAAAAAAA)
 * W    PTR selection (PA, PB, PTRA, PTRB)
 *
 * Additional characters not used in the original comment:
 * h    for WC + WZ bits which have to be either 00 or 11 (equal)
 * x    for WC + WZ bits which have to be either 10 or 01 (unequal)
 *
 * @param _func calling function's name
 * @param opcode instruction's opcode
 * @param pat instruction's pattern string
 * @param mask1 opcode mask for the instruction class (5,7,8,9 bit, with S, with D, or both)
 * @param mask2 additional mask for instruction bits
 * @return mask / match value in 64 bits
 */
P2DocOpcode P2Doc::make_pattern(const char* _func, p2_LONG opcode, const char* pat, p2_LONG mask1, p2_LONG mask2)
{
    P2MatchMask matchmask(FULL,opcode);
    // pattern without spaces
    QString pack = QString::fromLatin1(pat).remove(QChar(' '));

    // pattern size must be 32 bits
    if (32 != pack.size()) {
        qDebug("pattern is not 32 bits: %s (%s)",
               qPrintable(pack), _func);
        Q_ASSERT(32 == pack.size());
    }

    // replace non-mask bits with '0's
    for (int bit = 0; bit < 32; bit++) {
        if (0 == ((mask1 << bit) & MSB) && pack[bit] != QChar('1'))
            pack[bit] = QChar('0');
    }

    bool ok;
    p2_LONG match = pack.toUInt(&ok, 2);
    if (!ok) {
        qDebug("mask contains non digits: %s (%s)",
               qPrintable(pack), _func);
    }
    p2_LONG mask = mask1 | mask2;
    matchmask = P2MatchMask((opcode | match) & mask, mask);

    if (matchmask.match() & ~matchmask.mask()) {
        QString dbg_instr = QString("%1").arg(opcode | mask, 32, 2, QChar('0'));
        QString dbg_mask = QString("%1").arg(mask, 32, 2, QChar('0'));
        put_spaces(dbg_instr);
        put_spaces(dbg_mask);
        dbg_instr.replace(QChar('0'), chr_centerdot);
        dbg_mask.replace(QChar('0'), chr_centerdot);
        qDebug("miss inst=[%s] mask=[%s] (%s)",
               qPrintable(dbg_mask),
               qPrintable(dbg_instr),
               _func);
    }

    if (m_opcodes.contains(matchmask)) {
        P2DocOpcode op = m_opcodes.value(matchmask);
        QString dbg_match = QString("%1").arg(matchmask.first, 32, 2, QChar('0'));
        QString dbg_mask = QString("%1").arg(matchmask.second, 32, 2, QChar('0'));
        put_spaces(dbg_mask);
        put_spaces(dbg_match);
        qDebug("dupe mask=[%s] match=[%s] for %s (%s)",
               qPrintable(dbg_mask),
               qPrintable(dbg_match),
               op->func(),
               _func);
    }
    P2DocOpcode result = P2DocOpcode(new P2DocOpcodeClass(matchmask, pat, _func));
    m_opcodes.insert(matchmask, result);

    return result;
}

/**
 * @brief Make pattern for 5 bit instruction
 * @param _func calling function's name
 * @param instr instruction enum value
 * @param pat pattern string
 * @return mask / match value pair
 */
P2DocOpcode P2Doc::make_pattern(const char* _func, p2_INST5_e instr, const char* pat, match_flags flags)
{
    p2_LONG mask1 = p2_mask_inst5;
    p2_LONG mask2 = 0;

    Q_ASSERT(match_none == flags);

    return make_pattern(_func, opcode_inst5(instr), pat, mask1, mask2);
}

/**
 * @brief Make pattern for 7 bit instruction
 * @param _func calling function's name
 * @param instr instruction enum value
 * @param pat pattern string
 * @return mask / match value pair
 */
P2DocOpcode P2Doc::make_pattern(const char* _func, p2_INST7_e instr, const char* pat, match_flags flags)
{
    p2_LONG mask1 = p2_mask_inst7;
    p2_LONG mask2 = 0;

    switch (flags) {
    case match_all:
        mask2 = FULL;
        break;
    case match_NNN_D:
        mask2 = p2_mask_NNN | p2_mask_D;
        break;
    case match_NNN_I_S:
        mask2 = p2_mask_NNN | p2_mask_I | p2_mask_S;
        break;
    case match_NN_D:
        mask2 = p2_mask_NN | p2_mask_D;
        break;
    case match_NN_I_S:
        mask2 = p2_mask_NN | p2_mask_I | p2_mask_S;
        break;
    default:
        Q_ASSERT(match_none == flags);
    }

    return make_pattern(_func, opcode_inst7(instr), pat, mask1, mask2);
}

/**
 * @brief Make pattern for 8 bit instruction
 * @param _func calling function's name
 * @param instr instruction enum value
 * @param pat pattern string
 * @return mask / match value pair
 */
P2DocOpcode P2Doc::make_pattern(const char* _func, p2_INST8_e instr, const char* pat, match_flags flags)
{
    p2_LONG mask1 = p2_mask_inst8;
    p2_LONG mask2 = 0;

    switch (flags) {
    case match_C:
        mask2 = p2_mask_C;
        break;
    case match_Z:
        mask1 &= ~p2_mask_C;
        mask2 = p2_mask_CZ;
        break;
    case match_D:
        mask2 = p2_mask_D;
        break;
    case match_D_S:
        mask2 = p2_mask_D | p2_mask_S;
        break;
    default:
        Q_ASSERT(match_none == flags);
    }

    return make_pattern(_func, opcode_inst8(instr), pat, mask1, mask2);
}

/**
 * @brief Pattern for 9 bit instruction
 * @param _func calling function's name
 * @param instr instruction enum value
 * @param pat pattern string
 * @return mask / match value pair
 */
P2DocOpcode P2Doc::make_pattern(const char* _func, p2_INST9_e instr, const char* pat, match_flags flags)
{
    p2_LONG mask1 = p2_mask_inst9;
    p2_LONG mask2 = 0;

    switch (flags) {
    case match_NNN:
        mask1 &= ~p2_mask_NNN;
        mask2 = p2_mask_NNN;
        break;
    case match_NN:
        mask1 &= ~p2_mask_NN;
        mask2 = p2_mask_NN;
        break;
    case match_N:
        mask1 &= ~p2_mask_N;
        mask2 = p2_mask_N;
        break;
    case match_CZI_D_S:
        mask1 &= ~p2_mask_CZI;
        mask2 = p2_mask_CZI | p2_mask_D | p2_mask_S;
        break;
    case match_I_S:
        mask2 = p2_mask_I | p2_mask_S;
        break;
    case match_C_equ_Z:
        mask1 &= ~p2_mask_CZ;
        mask2 = p2_mask_CZ;
        break;
    case match_C_neq_Z:
        mask1 &= ~p2_mask_CZ;
        mask2 = p2_mask_CZ;
        break;
    case match_D:
        mask2 = p2_mask_D;
        break;
    case match_S:
        mask2 = p2_mask_S;
        break;
    default:
        Q_ASSERT(match_none == flags);
    }

    return make_pattern(_func, opcode_inst9(instr), pat, mask1, mask2);
}

/**
 * @brief Pattern for instruction with dst field enumeration value
 * @param _func calling function's name
 * @param instr instruction enum value
 * @param pat pattern string
 * @return mask / match value pair
 */
P2DocOpcode P2Doc::make_pattern(const char* _func, p2_OPDST_e instr, const char* pat, match_flags flags)
{
    p2_LONG mask1 = p2_mask_opdst;
    p2_LONG mask2 = 0;

    Q_ASSERT(match_none == flags);

    return make_pattern(_func, opcode_opdst(instr), pat, mask1, mask2);
}

/**
 * @brief Pattern for instruction with src field enumeration value
 * @param _func calling function's name
 * @param instr instruction enum value
 * @param pat pattern string
 * @return mask / match value pair
 */
P2DocOpcode P2Doc::make_pattern(const char* _func, p2_OPSRC_e instr, const char* pat, match_flags flags)
{
    p2_LONG mask1 = p2_mask_opsrc;
    p2_LONG mask2 = 0;

    switch (flags) {
    case match_D:
        mask2 = p2_mask_D;
        break;
    case match_I:
        mask2 = p2_mask_I;
        break;
    case match_I_D:
        mask2 = p2_mask_I | p2_mask_D;
        break;
    case match_I_S:
        mask2 = p2_mask_I | p2_mask_S;
        break;
    case match_C_equ_Z:
        mask2 = p2_mask_CZ;
        break;
    case match_CZ:
        mask2 = p2_mask_CZ;
        break;
    case match_CZI:
        mask2 = p2_mask_CZI;
        break;
    default:
        Q_ASSERT(match_none == flags);
    }

    return make_pattern(_func, opcode_opsrc(instr), pat, mask1, mask2);
}

/**
 * @brief Pattern for instruction with dst field enumeration value, src = %000100100
 * @param _func calling function's name
 * @param instr instruction enum value
 * @param pat pattern string
 * @return mask / match value pair
 */
P2DocOpcode P2Doc::make_pattern(const char* _func, p2_OPX24_e instr, const char* pat, match_flags flags)
{
    p2_LONG mask1 = p2_mask_opx24;
    p2_LONG mask2 = 0;

    Q_ASSERT(match_none == flags);

    return make_pattern(_func, opcode_opx24(instr), pat, mask1, mask2);
}

/**
 * @brief Pattern for instruction with src field enumeration value
 * @param _func calling function's name
 * @param instr instruction enum value from opsrc
 * @param inst9 instruction enum value from inst9
 * @param pat pattern string
 * @return mask / match value pair
 */
P2DocOpcode P2Doc::make_pattern(const char* _func, p2_OPSRC_e instr, p2_INST9_e inst9, const char* pat, match_flags flags)
{
    p2_LONG mask1 = p2_mask_opsrc;
    p2_LONG mask2 = 0;

    switch (flags) {
    case match_C_equ_Z:
        mask2 |= p2_mask_CZ;
        break;
    case match_C_neq_Z:
        mask2 |= p2_mask_CZ;
        break;
    default:
        Q_ASSERT(match_none == flags);
    }

    return make_pattern(_func, opcode_inst9(inst9) | opcode_opsrc(instr), pat, mask1, mask2);
}


/**
 * @brief Set the destination %dst in the opcode and check for errors
 */
void P2Doc::params_dst(P2DocOpcode& op, P2Opcode::ImmFlag flag)
{
    op->add_param(t_D);
    Q_UNUSED(flag)
}

/**
 * @brief Set the source %src in the opcode and check for errors
 */
void P2Doc::params_src(P2DocOpcode& op, P2Opcode::ImmFlag flag)
{
    op->add_param(t_S);
    Q_UNUSED(flag)
}

/**
 * @brief A comma is expected
 */
void P2Doc::mandatory_COMMA(P2DocOpcode& op)
{
    op->add_param(t_COMMA);
}

/**
 * @brief Expect an optional comma
 */
void P2Doc::optional_COMMA(P2DocOpcode& op)
{
    op->add_param(t_COMMA, p2_optional);
}

/**
 * @brief Expect optional WC, WZ, or WCZ
 */
void P2Doc::optional_WCZ(P2DocOpcode& op)
{
    op->add_param(t_WC, p2_optional);
    op->add_param(t_WZ, p2_optional);
    op->add_param(t_WCZ, p2_optional);
}

/**
 * @brief Expect optional WC
 */
void P2Doc::optional_WC(P2DocOpcode& op)
{
    op->add_param(t_WC, p2_optional);
}

/**
 * @brief Expect optional WZ
 */
void P2Doc::optional_WZ(P2DocOpcode& op)
{
    op->add_param(t_WZ, p2_optional);
}

/**
 * @brief Mandatory ANDC or ANDZ
 */
void P2Doc::mandatory_ANDC_ANDZ(P2DocOpcode& op)
{
    op->add_param(t_ANDC, p2_alternative_1);
    op->add_param(t_ANDZ, p2_alternative_2);
}

/**
 * @brief Mandatory ORC or ORZ
 */
void P2Doc::mandatory_ORC_ORZ(P2DocOpcode& op)
{
    op->add_param(t_ORC, p2_alternative_1);
    op->add_param(t_ORZ, p2_alternative_2);
}

/**
 * @brief Mandatory XORC or XORZ
 */
void P2Doc::mandatory_XORC_XORZ(P2DocOpcode& op)
{
    op->add_param(t_XORC, p2_alternative_1);
    op->add_param(t_XORZ, p2_alternative_2);
}

/**
 * @brief Expect parameters for D and {#}S
 */
void P2Doc::params_D_IM_S(P2DocOpcode& op)
{
    op->add_param(t_D);
    mandatory_COMMA(op);
    op->add_param(t_IMMEDIATE, p2_optional);
    op->add_param(t_S);
}

/**
 * @brief Expect parameters for D and optional WC, WZ, or WCZ
 */
void P2Doc::params_D_WCZ(P2DocOpcode& op)
{
    op->add_param(t_D);
    optional_WCZ(op);
}

/**
 * @brief Expect optional WC, WZ, or WCZ
 */
void P2Doc::params_WCZ(P2DocOpcode& op)
{
    optional_WCZ(op);
}

/**
 * @brief Expect parameters for D
 */
void P2Doc::params_D(P2DocOpcode& op)
{
    op->add_param(t_D);
}

/**
 * @brief Expect parameters for {#}D setting WZ for immediate
 */
void P2Doc::params_WZ_D(P2DocOpcode& op)
{
    op->add_param(t_IMMEDIATE, p2_optional);
    op->add_param(t_D);
}

/**
 * @brief Expect parameters for {#}D
 */
void P2Doc::params_IM_D(P2DocOpcode& op)
{
    op->add_param(t_IMMEDIATE, p2_optional);
    op->add_param(t_D);
}

/**
 * @brief Expect parameters for {#}D, and optional WC, WZ, or WCZ
 */
void P2Doc::params_IM_D_WCZ(P2DocOpcode& op)
{
    params_IM_D(op);
    optional_WCZ(op);
}

/**
 * @brief Expect parameters for {#}D, and optional WC
 */
void P2Doc::params_IM_D_WC(P2DocOpcode& op)
{
    params_IM_D(op);
    optional_WC(op);
}

/**
 * @brief Expect parameters for {#}D, and mandatory ANDC or ANDZ
 */
void P2Doc::params_IM_D_ANDC_ANDZ(P2DocOpcode& op)
{
    params_IM_D(op);
    mandatory_ANDC_ANDZ(op);
}

/**
 * @brief Expect parameters for {#}D, and mandatory ORC or ORZ
 */
void P2Doc::params_IM_D_ORC_ORZ(P2DocOpcode& op)
{
    params_IM_D(op);
    mandatory_ORC_ORZ(op);
}

/**
 * @brief Expect parameters for {#}D, and mandatory XORC or XORZ
 */
void P2Doc::params_IM_D_XORC_XORZ(P2DocOpcode& op)
{
    params_IM_D(op);
    mandatory_XORC_XORZ(op);
}

/**
 * @brief Expect parameters for D, and {#}S, and optional WC, WZ, or WCZ
 */
void P2Doc::params_D_IM_S_WCZ(P2DocOpcode& op)
{
    params_D_IM_S(op);
    optional_WCZ(op);
}

/**
 * @brief Expect parameters for D, and {#}S, and mandatory ANDC or ANDZ
 */
void P2Doc::params_D_IM_S_ANDCZ(P2DocOpcode& op)
{
    params_D_IM_S(op);
    mandatory_ANDC_ANDZ(op);
}

/**
 * @brief Expect parameters for D, and {#}S, and mandatory ORC or ORZ
 */
void P2Doc::params_D_IM_S_ORCZ(P2DocOpcode& op)
{
    params_D_IM_S(op);
    mandatory_ORC_ORZ(op);
}

/**
 * @brief Expect parameters for D, and {#}S, and mandatory XORC or XORZ
 */
void P2Doc::params_D_IM_S_XORCZ(P2DocOpcode& op)
{
    params_D_IM_S(op);
    mandatory_XORC_XORZ(op);
}

/**
 * @brief Expect parameters for D, and {#}S, and optional WC
 */
void P2Doc::params_D_IM_S_WC(P2DocOpcode& op)
{
    params_D_IM_S(op);
    optional_WC(op);
}

/**
 * @brief Expect parameters for D, and {#}S, and optional WZ
 */
void P2Doc::params_D_IM_S_WZ(P2DocOpcode& op)
{
    params_D_IM_S(op);
    optional_WZ(op);
}

/**
 * @brief Expect parameters for {#}D, and {#}S
 */
void P2Doc::params_WZ_D_IM_S(P2DocOpcode& op)
{
    op->add_param(t_IMMEDIATE, p2_optional);
    params_D_IM_S(op);
}

/**
 * @brief Expect parameters for {#}D, and {#}S, followed by an optional WC
 */
void P2Doc::params_WZ_D_IM_S_WC(P2DocOpcode& op)
{
    params_WZ_D_IM_S(op);
    optional_WC(op);
}

/**
 * @brief Expect parameters for D, and {#}S, and #N (0 … 7)
 */
void P2Doc::params_D_IM_S_NNN(P2DocOpcode& op, uint max)
{
    params_D_IM_S(op);
    mandatory_COMMA(op);
    op->add_param(t_IMMEDIATE);
    // TODO: t_NNN, t_NN, t_N
    Q_UNUSED(max)
}

/**
 * @brief Expect parameters for {#}S
 */
void P2Doc::params_IM_S(P2DocOpcode& op)
{
    op->add_param(t_IMMEDIATE, p2_optional);
    op->add_param(t_S);
}

/**
 * @brief Expect parameters for {#}S, and optional WC
 */
void P2Doc::params_IM_S_WC(P2DocOpcode& op)
{
    params_IM_S(op);
    optional_WC(op);
}


/**
 * @brief Expect parameters for #AAAAAAAAAAAA (20 bit address for CALL/CALLA/CALLB/LOC)
 */
void P2Doc::params_PTRx_PC_A20(P2DocOpcode& op)
{
    // TODO: t_ADDR20
    Q_UNUSED(op)
}

/**
 * @brief Expect parameters for #AAAAAAAAAAAA (20 bit address for CALL/CALLA/CALLB)
 */
void P2Doc::params_PC_A20(P2DocOpcode& op)
{
    // TODO: t_ADDR20
    Q_UNUSED(op)
}

/**
 * @brief Expect parameters for #AAAAAAAAAAAA (23 bit address for AUGD/AUGS)
 */
void P2Doc::params_IMM23(P2DocOpcode& op)
{
    // TODO: t_IMM23
    Q_UNUSED(op)
}

/**
 * @brief No operation.
 *
 * 0000 0000000 000 000000000 000000000
 *
 * NOP
 *
 */
void P2Doc::doc_NOP(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "0000 0000000 000 000000000 000000000", FULL, 0);

    op->set_token(t_NOP);
    op->set_brief("No operation.");
    op->set_instr("NOP");
}

/**
 * @brief Rotate right.
 *
 * EEEE 0000000 CZI DDDDDDDDD SSSSSSSSS
 *
 * ROR     D,{#}S   {WC/WZ/WCZ}
 * D = [31:0]  of ({D[31:0], D[31:0]}     >> S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[0].
 * Z = (result == 0).
 *
 */
void P2Doc::doc_ROR(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0000000 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ROR);
    params_D_IM_S_WCZ(op);
    op->set_brief("Rotate right.");
    op->set_instr("ROR     D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = [31:0]  of ({D[31:0], D[31:0]}     >> S[4:0]).");
    op->add_descr("C = last bit shifted out if S[4:0] > 0, else D[0].");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Rotate left.
 *
 * EEEE 0000001 CZI DDDDDDDDD SSSSSSSSS
 *
 * ROL     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [63:32] of ({D[31:0], D[31:0]}     << S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[31].
 * Z = (result == 0).
 *
 */
void P2Doc::doc_ROL(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0000001 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ROL);
    params_D_IM_S_WCZ(op);
    op->set_brief("Rotate left.");
    op->set_instr("ROL     D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = [63:32] of ({D[31:0], D[31:0]}     << S[4:0]).");
    op->add_descr("C = last bit shifted out if S[4:0] > 0, else D[31].");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Shift right.
 *
 * EEEE 0000010 CZI DDDDDDDDD SSSSSSSSS
 *
 * SHR     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [31:0]  of ({32'b0, D[31:0]}       >> S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[0].
 * Z = (result == 0).
 *
 */
void P2Doc::doc_SHR(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0000010 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_SHR);
    params_D_IM_S_WCZ(op);
    op->set_brief("Shift right.");
    op->set_instr("SHR     D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = [31:0]  of ({32'b0, D[31:0]}       >> S[4:0]).");
    op->add_descr("C = last bit shifted out if S[4:0] > 0, else D[0].");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Shift left.
 *
 * EEEE 0000011 CZI DDDDDDDDD SSSSSSSSS
 *
 * SHL     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [63:32] of ({D[31:0], 32'b0}       << S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[31].
 * Z = (result == 0).
 *
 */
void P2Doc::doc_SHL(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0000011 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_SHL);
    params_D_IM_S_WCZ(op);
    op->set_brief("Shift left.");
    op->set_instr("SHL     D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = [63:32] of ({D[31:0], 32'b0}       << S[4:0]).");
    op->add_descr("C = last bit shifted out if S[4:0] > 0, else D[31].");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Rotate carry right.
 *
 * EEEE 0000100 CZI DDDDDDDDD SSSSSSSSS
 *
 * RCR     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [31:0]  of ({{32{C}}, D[31:0]}     >> S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[0].
 * Z = (result == 0).
 *
 */
void P2Doc::doc_RCR(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0000100 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_RCR);
    params_D_IM_S_WCZ(op);
    op->set_brief("Rotate carry right.");
    op->set_instr("RCR     D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = [31:0]  of ({{32{C}}, D[31:0]}     >> S[4:0]).");
    op->add_descr("C = last bit shifted out if S[4:0] > 0, else D[0].");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Rotate carry left.
 *
 * EEEE 0000101 CZI DDDDDDDDD SSSSSSSSS
 *
 * RCL     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [63:32] of ({D[31:0], {32{C}}}     << S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[31].
 * Z = (result == 0).
 *
 */
void P2Doc::doc_RCL(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0000101 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_RCL);
    params_D_IM_S_WCZ(op);
    op->set_brief("Rotate carry left.");
    op->set_instr("RCL     D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = [63:32] of ({D[31:0], {32{C}}}     << S[4:0]).");
    op->add_descr("C = last bit shifted out if S[4:0] > 0, else D[31].");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Shift arithmetic right.
 *
 * EEEE 0000110 CZI DDDDDDDDD SSSSSSSSS
 *
 * SAR     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [31:0]  of ({{32{D[31]}}, D[31:0]} >> S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[0].
 * Z = (result == 0).
 *
 */
void P2Doc::doc_SAR(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0000110 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_SAR);
    params_D_IM_S_WCZ(op);
    op->set_brief("Shift arithmetic right.");
    op->set_instr("SAR     D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = [31:0]  of ({{32{D[31]}}, D[31:0]} >> S[4:0]).");
    op->add_descr("C = last bit shifted out if S[4:0] > 0, else D[0].");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Shift arithmetic left.
 *
 * EEEE 0000111 CZI DDDDDDDDD SSSSSSSSS
 *
 * SAL     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [63:32] of ({D[31:0], {32{D[0]}}}  << S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[31].
 * Z = (result == 0).
 *
 */
void P2Doc::doc_SAL(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0000111 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_SAL);
    params_D_IM_S_WCZ(op);
    op->set_brief("Shift arithmetic left.");
    op->set_instr("SAL     D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = [63:32] of ({D[31:0], {32{D[0]}}}  << S[4:0]).");
    op->add_descr("C = last bit shifted out if S[4:0] > 0, else D[31].");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Add S into D.
 *
 * EEEE 0001000 CZI DDDDDDDDD SSSSSSSSS
 *
 * ADD     D,{#}S   {WC/WZ/WCZ}
 *
 * D = D + S.
 * C = carry of (D + S).
 * Z = (result == 0).
 *
 */
void P2Doc::doc_ADD(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0001000 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ADD);
    params_D_IM_S_WCZ(op);
    op->set_brief("Add S into D.");
    op->set_instr("ADD     D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = D + S.");
    op->add_descr("C = carry of (D + S).");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Add (S + C) into D, extended.
 *
 * EEEE 0001001 CZI DDDDDDDDD SSSSSSSSS
 *
 * ADDX    D,{#}S   {WC/WZ/WCZ}
 *
 * D = D + S + C.
 * C = carry of (D + S + C).
 * Z = Z AND (result == 0).
 *
 */
void P2Doc::doc_ADDX(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0001001 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ADDX);
    params_D_IM_S_WCZ(op);
    op->set_brief("Add (S + C) into D, extended.");
    op->set_instr("ADDX    D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = D + S + C.");
    op->add_descr("C = carry of (D + S + C).");
    op->add_descr("Z = Z AND (result == 0).");
}

/**
 * @brief Add S into D, signed.
 *
 * EEEE 0001010 CZI DDDDDDDDD SSSSSSSSS
 *
 * ADDS    D,{#}S   {WC/WZ/WCZ}
 *
 * D = D + S.
 * C = correct sign of (D + S).
 * Z = (result == 0).
 *
 */
void P2Doc::doc_ADDS(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0001010 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ADDS);
    params_D_IM_S_WCZ(op);
    op->set_brief("Add S into D, signed.");
    op->set_instr("ADDS    D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = D + S.");
    op->add_descr("C = correct sign of (D + S).");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Add (S + C) into D, signed and extended.
 *
 * EEEE 0001011 CZI DDDDDDDDD SSSSSSSSS
 *
 * ADDSX   D,{#}S   {WC/WZ/WCZ}
 *
 * D = D + S + C.
 * C = correct sign of (D + S + C).
 * Z = Z AND (result == 0).
 *
 */
void P2Doc::doc_ADDSX(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0001011 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ADDSX);
    params_D_IM_S_WCZ(op);
    op->set_brief("Add (S + C) into D, signed and extended.");
    op->set_instr("ADDSX   D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = D + S + C.");
    op->add_descr("C = correct sign of (D + S + C).");
    op->add_descr("Z = Z AND (result == 0).");
}

/**
 * @brief Subtract S from D.
 *
 * EEEE 0001100 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUB     D,{#}S   {WC/WZ/WCZ}
 *
 * D = D - S.
 * C = borrow of (D - S).
 * Z = (result == 0).
 *
 */
void P2Doc::doc_SUB(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0001100 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_SUB);
    params_D_IM_S_WCZ(op);
    op->set_brief("Subtract S from D.");
    op->set_instr("SUB     D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = D - S.");
    op->add_descr("C = borrow of (D - S).");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Subtract (S + C) from D, extended.
 *
 * EEEE 0001101 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUBX    D,{#}S   {WC/WZ/WCZ}
 *
 * D = D - (S + C).
 * C = borrow of (D - (S + C)).
 * Z = Z AND (result == 0).
 *
 */
void P2Doc::doc_SUBX(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0001101 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_SUBX);
    params_D_IM_S_WCZ(op);
    op->set_brief("Subtract (S + C) from D, extended.");
    op->set_instr("SUBX    D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = D - (S + C).");
    op->add_descr("C = borrow of (D - (S + C)).");
    op->add_descr("Z = Z AND (result == 0).");
}

/**
 * @brief Subtract S from D, signed.
 *
 * EEEE 0001110 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUBS    D,{#}S   {WC/WZ/WCZ}
 *
 * D = D - S.
 * C = correct sign of (D - S).
 * Z = (result == 0).
 *
 */
void P2Doc::doc_SUBS(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0001110 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_SUBS);
    params_D_IM_S_WCZ(op);
    op->set_brief("Subtract S from D, signed.");
    op->set_instr("SUBS    D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = D - S.");
    op->add_descr("C = correct sign of (D - S).");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Subtract (S + C) from D, signed and extended.
 *
 * EEEE 0001111 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUBSX   D,{#}S   {WC/WZ/WCZ}
 *
 * D = D - (S + C).
 * C = correct sign of (D - (S + C)).
 * Z = Z AND (result == 0).
 *
 */
void P2Doc::doc_SUBSX(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0001111 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_SUBSX);
    params_D_IM_S_WCZ(op);
    op->set_brief("Subtract (S + C) from D, signed and extended.");
    op->set_instr("SUBSX   D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = D - (S + C).");
    op->add_descr("C = correct sign of (D - (S + C)).");
    op->add_descr("Z = Z AND (result == 0).");
}

/**
 * @brief Compare D to S.
 *
 * EEEE 0010000 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMP     D,{#}S   {WC/WZ/WCZ}
 *
 * C = borrow of (D - S).
 * Z = (D == S).
 *
 */
void P2Doc::doc_CMP(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0010000 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_CMP);
    params_D_IM_S_WCZ(op);
    op->set_brief("Compare D to S.");
    op->set_instr("CMP     D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("C = borrow of (D - S).");
    op->add_descr("Z = (D == S).");
}

/**
 * @brief Compare D to (S + C), extended.
 *
 * EEEE 0010001 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMPX    D,{#}S   {WC/WZ/WCZ}
 *
 * C = borrow of (D - (S + C)).
 * Z = Z AND (D == S + C).
 *
 */
void P2Doc::doc_CMPX(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0010001 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_CMPX);
    params_D_IM_S_WCZ(op);
    op->set_brief("Compare D to (S + C), extended.");
    op->set_instr("CMPX    D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("C = borrow of (D - (S + C)).");
    op->add_descr("Z = Z AND (D == S + C).");
}

/**
 * @brief Compare D to S, signed.
 *
 * EEEE 0010010 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMPS    D,{#}S   {WC/WZ/WCZ}
 *
 * C = correct sign of (D - S).
 * Z = (D == S).
 *
 */
void P2Doc::doc_CMPS(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0010010 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_CMPS);
    params_D_IM_S_WCZ(op);
    op->set_brief("Compare D to S, signed.");
    op->set_instr("CMPS    D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("C = correct sign of (D - S).");
    op->add_descr("Z = (D == S).");
}

/**
 * @brief Compare D to (S + C), signed and extended.
 *
 * EEEE 0010011 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMPSX   D,{#}S   {WC/WZ/WCZ}
 *
 * C = correct sign of (D - (S + C)).
 * Z = Z AND (D == S + C).
 *
 */
void P2Doc::doc_CMPSX(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0010011 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_CMPSX);
    params_D_IM_S_WCZ(op);
    op->set_brief("Compare D to (S + C), signed and extended.");
    op->set_instr("CMPSX   D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("C = correct sign of (D - (S + C)).");
    op->add_descr("Z = Z AND (D == S + C).");
}

/**
 * @brief Compare S to D (reverse).
 *
 * EEEE 0010100 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMPR    D,{#}S   {WC/WZ/WCZ}
 *
 * C = borrow of (S - D).
 * Z = (D == S).
 *
 */
void P2Doc::doc_CMPR(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0010100 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_CMPR);
    params_D_IM_S_WCZ(op);
    op->set_brief("Compare S to D (reverse).");
    op->set_instr("CMPR    D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("C = borrow of (S - D).");
    op->add_descr("Z = (D == S).");
}

/**
 * @brief Compare D to S, get MSB of difference into C.
 *
 * EEEE 0010101 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMPM    D,{#}S   {WC/WZ/WCZ}
 *
 * C = MSB of (D - S).
 * Z = (D == S).
 *
 */
void P2Doc::doc_CMPM(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0010101 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_CMPM);
    params_D_IM_S_WCZ(op);
    op->set_brief("Compare D to S, get MSB of difference into C.");
    op->set_instr("CMPM    D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("C = MSB of (D - S).");
    op->add_descr("Z = (D == S).");
}

/**
 * @brief Subtract D from S (reverse).
 *
 * EEEE 0010110 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUBR    D,{#}S   {WC/WZ/WCZ}
 *
 * D = S - D.
 * C = borrow of (S - D).
 * Z = (result == 0).
 *
 */
void P2Doc::doc_SUBR(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0010110 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_SUBR);
    params_D_IM_S_WCZ(op);
    op->set_brief("Subtract D from S (reverse).");
    op->set_instr("SUBR    D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = S - D.");
    op->add_descr("C = borrow of (S - D).");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Compare and subtract S from D if D >= S.
 *
 * EEEE 0010111 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMPSUB  D,{#}S   {WC/WZ/WCZ}
 *
 * If D => S then D = D - S and C = 1, else D same and C = 0.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_CMPSUB(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0010111 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_CMPSUB);
    params_D_IM_S_WCZ(op);
    op->set_brief("Compare and subtract S from D if D >= S.");
    op->set_instr("CMPSUB  D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("If D => S then D = D - S and C = 1, else D same and C = 0.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Force D >= S.
 *
 * EEEE 0011000 CZI DDDDDDDDD SSSSSSSSS
 *
 * FGE     D,{#}S   {WC/WZ/WCZ}
 *
 * If D < S then D = S and C = 1, else D same and C = 0.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_FGE(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0011000 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_FGE);
    params_D_IM_S_WCZ(op);
    op->set_brief("Force D >= S.");
    op->set_instr("FGE     D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("If D < S then D = S and C = 1, else D same and C = 0.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Force D <= S.
 *
 * EEEE 0011001 CZI DDDDDDDDD SSSSSSSSS
 *
 * FLE     D,{#}S   {WC/WZ/WCZ}
 *
 * If D > S then D = S and C = 1, else D same and C = 0.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_FLE(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0011001 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_FLE);
    params_D_IM_S_WCZ(op);
    op->set_brief("Force D <= S.");
    op->set_instr("FLE     D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("If D > S then D = S and C = 1, else D same and C = 0.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Force D >= S, signed.
 *
 * EEEE 0011010 CZI DDDDDDDDD SSSSSSSSS
 *
 * FGES    D,{#}S   {WC/WZ/WCZ}
 *
 * If D < S then D = S and C = 1, else D same and C = 0.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_FGES(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0011010 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_FGES);
    params_D_IM_S_WCZ(op);
    op->set_brief("Force D >= S, signed.");
    op->set_instr("FGES    D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("If D < S then D = S and C = 1, else D same and C = 0.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Force D <= S, signed.
 *
 * EEEE 0011011 CZI DDDDDDDDD SSSSSSSSS
 *
 * FLES    D,{#}S   {WC/WZ/WCZ}
 *
 * If D > S then D = S and C = 1, else D same and C = 0.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_FLES(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0011011 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_FLES);
    params_D_IM_S_WCZ(op);
    op->set_brief("Force D <= S, signed.");
    op->set_instr("FLES    D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("If D > S then D = S and C = 1, else D same and C = 0.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Sum +/-S into D by  C.
 *
 * EEEE 0011100 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUMC    D,{#}S   {WC/WZ/WCZ}
 *
 * If C = 1 then D = D - S, else D = D + S.
 * C = correct sign of (D +/- S).
 * Z = (result == 0).
 *
 */
void P2Doc::doc_SUMC(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0011100 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_SUMC);
    params_D_IM_S_WCZ(op);
    op->set_brief("Sum +/-S into D by  C.");
    op->set_instr("SUMC    D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("If C = 1 then D = D - S, else D = D + S.");
    op->add_descr("C = correct sign of (D +/- S).");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Sum +/-S into D by !C.
 *
 * EEEE 0011101 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUMNC   D,{#}S   {WC/WZ/WCZ}
 *
 * If C = 0 then D = D - S, else D = D + S.
 * C = correct sign of (D +/- S).
 * Z = (result == 0).
 *
 */
void P2Doc::doc_SUMNC(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0011101 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_SUMNC);
    params_D_IM_S_WCZ(op);
    op->set_brief("Sum +/-S into D by !C.");
    op->set_instr("SUMNC   D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("If C = 0 then D = D - S, else D = D + S.");
    op->add_descr("C = correct sign of (D +/- S).");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Sum +/-S into D by  Z.
 *
 * EEEE 0011110 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUMZ    D,{#}S   {WC/WZ/WCZ}
 *
 * If Z = 1 then D = D - S, else D = D + S.
 * C = correct sign of (D +/- S).
 * Z = (result == 0).
 *
 */
void P2Doc::doc_SUMZ(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0011110 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_SUMZ);
    params_D_IM_S_WCZ(op);
    op->set_brief("Sum +/-S into D by  Z.");
    op->set_instr("SUMZ    D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("If Z = 1 then D = D - S, else D = D + S.");
    op->add_descr("C = correct sign of (D +/- S).");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Sum +/-S into D by !Z.
 *
 * EEEE 0011111 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUMNZ   D,{#}S   {WC/WZ/WCZ}
 *
 * If Z = 0 then D = D - S, else D = D + S.
 * C = correct sign of (D +/- S).
 * Z = (result == 0).
 *
 */
void P2Doc::doc_SUMNZ(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0011111 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_SUMNZ);
    params_D_IM_S_WCZ(op);
    op->set_brief("Sum +/-S into D by !Z.");
    op->set_instr("SUMNZ   D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("If Z = 0 then D = D - S, else D = D + S.");
    op->add_descr("C = correct sign of (D +/- S).");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Test bit S[4:0] of  D, write to C/Z.
 *
 * EEEE 0100000 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTB   D,{#}S         WC/WZ
 *
 * C/Z =          D[S[4:0]].
 *
 */
void P2Doc::doc_TESTB_W(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0100000 CZI DDDDDDDDD SSSSSSSSS", match_C_neq_Z);

    op->set_token(t_TESTB);
    op->set_brief("Test bit S[4:0] of  D, write to C/Z.");
    op->set_instr("TESTB   D,{#}S         WC/WZ");
    op->add_descr("C/Z =          D[S[4:0]].");
}

/**
 * @brief Test bit S[4:0] of !D, write to C/Z.
 *
 * EEEE 0100001 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTBN  D,{#}S         WC/WZ
 *
 * C/Z =         !D[S[4:0]].
 *
 */
void P2Doc::doc_TESTBN_W(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0100001 CZI DDDDDDDDD SSSSSSSSS", match_C_neq_Z);

    op->set_token(t_TESTBN);
    op->set_brief("Test bit S[4:0] of !D, write to C/Z.");
    op->set_instr("TESTBN  D,{#}S         WC/WZ");
    op->add_descr("C/Z =         !D[S[4:0]].");
}

/**
 * @brief Test bit S[4:0] of  D, AND into C/Z.
 *
 * EEEE 0100010 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTB   D,{#}S     ANDC/ANDZ
 *
 * C/Z = C/Z AND  D[S[4:0]].
 *
 */
void P2Doc::doc_TESTB_AND(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0100010 CZI DDDDDDDDD SSSSSSSSS", match_C_neq_Z);

    op->set_token(t_TESTB);
    op->set_brief("Test bit S[4:0] of  D, AND into C/Z.");
    op->set_instr("TESTB   D,{#}S     ANDC/ANDZ");
    op->add_descr("C/Z = C/Z AND  D[S[4:0]].");
}

/**
 * @brief Test bit S[4:0] of !D, AND into C/Z.
 *
 * EEEE 0100011 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTBN  D,{#}S     ANDC/ANDZ
 *
 * C/Z = C/Z AND !D[S[4:0]].
 *
 */
void P2Doc::doc_TESTBN_AND(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0100011 CZI DDDDDDDDD SSSSSSSSS", match_C_neq_Z);

    op->set_token(t_TESTBN);
    op->set_brief("Test bit S[4:0] of !D, AND into C/Z.");
    op->set_instr("TESTBN  D,{#}S     ANDC/ANDZ");
    op->add_descr("C/Z = C/Z AND !D[S[4:0]].");
}

/**
 * @brief Test bit S[4:0] of  D, OR  into C/Z.
 *
 * EEEE 0100100 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTB   D,{#}S       ORC/ORZ
 *
 * C/Z = C/Z OR   D[S[4:0]].
 *
 */
void P2Doc::doc_TESTB_OR(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0100100 CZI DDDDDDDDD SSSSSSSSS", match_C_neq_Z);

    op->set_token(t_TESTB);
    op->set_brief("Test bit S[4:0] of  D, OR  into C/Z.");
    op->set_instr("TESTB   D,{#}S       ORC/ORZ");
    op->add_descr("C/Z = C/Z OR   D[S[4:0]].");
}

/**
 * @brief Test bit S[4:0] of !D, OR  into C/Z.
 *
 * EEEE 0100101 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTBN  D,{#}S       ORC/ORZ
 *
 * C/Z = C/Z OR  !D[S[4:0]].
 *
 */
void P2Doc::doc_TESTBN_OR(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0100101 CZI DDDDDDDDD SSSSSSSSS", match_C_neq_Z);

    op->set_token(t_TESTBN);
    op->set_brief("Test bit S[4:0] of !D, OR  into C/Z.");
    op->set_instr("TESTBN  D,{#}S       ORC/ORZ");
    op->add_descr("C/Z = C/Z OR  !D[S[4:0]].");
}

/**
 * @brief Test bit S[4:0] of  D, XOR into C/Z.
 *
 * EEEE 0100110 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTB   D,{#}S     XORC/XORZ
 *
 * C/Z = C/Z XOR  D[S[4:0]].
 *
 */
void P2Doc::doc_TESTB_XOR(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0100110 CZI DDDDDDDDD SSSSSSSSS", match_C_neq_Z);

    op->set_token(t_TESTB);
    op->set_brief("Test bit S[4:0] of  D, XOR into C/Z.");
    op->set_instr("TESTB   D,{#}S     XORC/XORZ");
    op->add_descr("C/Z = C/Z XOR  D[S[4:0]].");
}

/**
 * @brief Test bit S[4:0] of !D, XOR into C/Z.
 *
 * EEEE 0100111 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTBN  D,{#}S     XORC/XORZ
 *
 * C/Z = C/Z XOR !D[S[4:0]].
 *
 */
void P2Doc::doc_TESTBN_XOR(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0100111 CZI DDDDDDDDD SSSSSSSSS", match_C_neq_Z);

    op->set_token(t_TESTBN);
    op->set_brief("Test bit S[4:0] of !D, XOR into C/Z.");
    op->set_instr("TESTBN  D,{#}S     XORC/XORZ");
    op->add_descr("C/Z = C/Z XOR !D[S[4:0]].");
}

/**
 * @brief Bit S[4:0] of D = 0,    C,Z = D[S[4:0]].
 *
 * EEEE 0100000 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITL    D,{#}S         {WCZ}
 *
 *
 */
void P2Doc::doc_BITL(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0100000 CZI DDDDDDDDD SSSSSSSSS", match_C_equ_Z);

    op->set_token(t_BITL);
    op->set_brief("Bit S[4:0] of D = 0,    C,Z = D[S[4:0]].");
    op->set_instr("BITL    D,{#}S         {WCZ}");
}

/**
 * @brief Bit S[4:0] of D = 1,    C,Z = D[S[4:0]].
 *
 * EEEE 0100001 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITH    D,{#}S         {WCZ}
 *
 *
 */
void P2Doc::doc_BITH(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0100001 CZI DDDDDDDDD SSSSSSSSS", match_C_equ_Z);

    op->set_token(t_BITH);
    op->set_brief("Bit S[4:0] of D = 1,    C,Z = D[S[4:0]].");
    op->set_instr("BITH    D,{#}S         {WCZ}");
}

/**
 * @brief Bit S[4:0] of D = C,    C,Z = D[S[4:0]].
 *
 * EEEE 0100010 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITC    D,{#}S         {WCZ}
 *
 *
 */
void P2Doc::doc_BITC(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0100010 CZI DDDDDDDDD SSSSSSSSS", match_C_equ_Z);

    op->set_token(t_BITC);
    op->set_brief("Bit S[4:0] of D = C,    C,Z = D[S[4:0]].");
    op->set_instr("BITC    D,{#}S         {WCZ}");
}

/**
 * @brief Bit S[4:0] of D = !C,   C,Z = D[S[4:0]].
 *
 * EEEE 0100011 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNC   D,{#}S         {WCZ}
 *
 *
 */
void P2Doc::doc_BITNC(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0100011 CZI DDDDDDDDD SSSSSSSSS", match_C_equ_Z);

    op->set_token(t_BITNC);
    op->set_brief("Bit S[4:0] of D = !C,   C,Z = D[S[4:0]].");
    op->set_instr("BITNC   D,{#}S         {WCZ}");
}

/**
 * @brief Bit S[4:0] of D = Z,    C,Z = D[S[4:0]].
 *
 * EEEE 0100100 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITZ    D,{#}S         {WCZ}
 *
 *
 */
void P2Doc::doc_BITZ(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0100100 CZI DDDDDDDDD SSSSSSSSS", match_C_equ_Z);

    op->set_token(t_BITZ);
    op->set_brief("Bit S[4:0] of D = Z,    C,Z = D[S[4:0]].");
    op->set_instr("BITZ    D,{#}S         {WCZ}");
}

/**
 * @brief Bit S[4:0] of D = !Z,   C,Z = D[S[4:0]].
 *
 * EEEE 0100101 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNZ   D,{#}S         {WCZ}
 *
 *
 */
void P2Doc::doc_BITNZ(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0100101 CZI DDDDDDDDD SSSSSSSSS", match_C_equ_Z);

    op->set_token(t_BITNZ);
    op->set_brief("Bit S[4:0] of D = !Z,   C,Z = D[S[4:0]].");
    op->set_instr("BITNZ   D,{#}S         {WCZ}");
}

/**
 * @brief Bit S[4:0] of D = RND,  C,Z = D[S[4:0]].
 *
 * EEEE 0100110 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITRND  D,{#}S         {WCZ}
 *
 *
 */
void P2Doc::doc_BITRND(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0100110 CZI DDDDDDDDD SSSSSSSSS", match_C_equ_Z);

    op->set_token(t_BITRND);
    op->set_brief("Bit S[4:0] of D = RND,  C,Z = D[S[4:0]].");
    op->set_instr("BITRND  D,{#}S         {WCZ}");
}

/**
 * @brief Bit S[4:0] of D = !bit, C,Z = D[S[4:0]].
 *
 * EEEE 0100111 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNOT  D,{#}S         {WCZ}
 *
 *
 */
void P2Doc::doc_BITNOT(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0100111 CZI DDDDDDDDD SSSSSSSSS", match_C_equ_Z);

    op->set_token(t_BITNOT);
    op->set_brief("Bit S[4:0] of D = !bit, C,Z = D[S[4:0]].");
    op->set_instr("BITNOT  D,{#}S         {WCZ}");
}

/**
 * @brief AND S into D.
 *
 * EEEE 0101000 CZI DDDDDDDDD SSSSSSSSS
 *
 * AND     D,{#}S   {WC/WZ/WCZ}
 *
 * D = D & S.
 * C = parity of result.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_AND(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0101000 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_AND);
    params_D_IM_S_WCZ(op);
    op->set_brief("AND S into D.");
    op->set_instr("AND     D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = D & S.");
    op->add_descr("C = parity of result.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief AND !S into D.
 *
 * EEEE 0101001 CZI DDDDDDDDD SSSSSSSSS
 *
 * ANDN    D,{#}S   {WC/WZ/WCZ}
 *
 * D = D & !S.
 * C = parity of result.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_ANDN(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0101001 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ANDN);
    params_D_IM_S_WCZ(op);
    op->set_brief("AND !S into D.");
    op->set_instr("ANDN    D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = D & !S.");
    op->add_descr("C = parity of result.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief OR S into D.
 *
 * EEEE 0101010 CZI DDDDDDDDD SSSSSSSSS
 *
 * OR      D,{#}S   {WC/WZ/WCZ}
 *
 * D = D | S.
 * C = parity of result.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_OR(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0101010 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_OR);
    params_D_IM_S_WCZ(op);
    op->set_brief("OR S into D.");
    op->set_instr("OR      D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = D | S.");
    op->add_descr("C = parity of result.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief XOR S into D.
 *
 * EEEE 0101011 CZI DDDDDDDDD SSSSSSSSS
 *
 * XOR     D,{#}S   {WC/WZ/WCZ}
 *
 * D = D ^ S.
 * C = parity of result.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_XOR(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0101011 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_XOR);
    params_D_IM_S_WCZ(op);
    op->set_brief("XOR S into D.");
    op->set_instr("XOR     D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = D ^ S.");
    op->add_descr("C = parity of result.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Mux C into each D bit that is '1' in S.
 *
 * EEEE 0101100 CZI DDDDDDDDD SSSSSSSSS
 *
 * MUXC    D,{#}S   {WC/WZ/WCZ}
 *
 * D = (!S & D ) | (S & {32{ C}}).
 * C = parity of result.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_MUXC(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0101100 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_MUXC);
    params_D_IM_S_WCZ(op);
    op->set_brief("Mux C into each D bit that is '1' in S.");
    op->set_instr("MUXC    D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = (!S & D ) | (S & {32{ C}}).");
    op->add_descr("C = parity of result.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Mux !C into each D bit that is '1' in S.
 *
 * EEEE 0101101 CZI DDDDDDDDD SSSSSSSSS
 *
 * MUXNC   D,{#}S   {WC/WZ/WCZ}
 *
 * D = (!S & D ) | (S & {32{!C}}).
 * C = parity of result.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_MUXNC(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0101101 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_MUXNC);
    params_D_IM_S_WCZ(op);
    op->set_brief("Mux !C into each D bit that is '1' in S.");
    op->set_instr("MUXNC   D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = (!S & D ) | (S & {32{!C}}).");
    op->add_descr("C = parity of result.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Mux Z into each D bit that is '1' in S.
 *
 * EEEE 0101110 CZI DDDDDDDDD SSSSSSSSS
 *
 * MUXZ    D,{#}S   {WC/WZ/WCZ}
 *
 * D = (!S & D ) | (S & {32{ Z}}).
 * C = parity of result.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_MUXZ(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0101110 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_MUXZ);
    params_D_IM_S_WCZ(op);
    op->set_brief("Mux Z into each D bit that is '1' in S.");
    op->set_instr("MUXZ    D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = (!S & D ) | (S & {32{ Z}}).");
    op->add_descr("C = parity of result.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Mux !Z into each D bit that is '1' in S.
 *
 * EEEE 0101111 CZI DDDDDDDDD SSSSSSSSS
 *
 * MUXNZ   D,{#}S   {WC/WZ/WCZ}
 *
 * D = (!S & D ) | (S & {32{!Z}}).
 * C = parity of result.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_MUXNZ(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0101111 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_MUXNZ);
    params_D_IM_S_WCZ(op);
    op->set_brief("Mux !Z into each D bit that is '1' in S.");
    op->set_instr("MUXNZ   D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = (!S & D ) | (S & {32{!Z}}).");
    op->add_descr("C = parity of result.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Move S into D.
 *
 * EEEE 0110000 CZI DDDDDDDDD SSSSSSSSS
 *
 * MOV     D,{#}S   {WC/WZ/WCZ}
 *
 * D = S.
 * C = S[31].
 * Z = (result == 0).
 *
 */
void P2Doc::doc_MOV(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0110000 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_MOV);
    params_D_IM_S_WCZ(op);
    op->set_brief("Move S into D.");
    op->set_instr("MOV     D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = S.");
    op->add_descr("C = S[31].");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Get !S into D.
 *
 * EEEE 0110001 CZI DDDDDDDDD SSSSSSSSS
 *
 * NOT     D,{#}S   {WC/WZ/WCZ}
 *
 * D = !S.
 * C = !S[31].
 * Z = (result == 0).
 *
 */
void P2Doc::doc_NOT(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0110001 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_NOT);
    params_D_IM_S_WCZ(op);
    op->set_brief("Get !S into D.");
    op->set_instr("NOT     D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = !S.");
    op->add_descr("C = !S[31].");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Get absolute value of S into D.
 *
 * EEEE 0110010 CZI DDDDDDDDD SSSSSSSSS
 *
 * ABS     D,{#}S   {WC/WZ/WCZ}
 *
 * D = ABS(S).
 * C = S[31].
 * Z = (result == 0).
 *
 */
void P2Doc::doc_ABS(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0110010 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ABS);
    params_D_IM_S_WCZ(op);
    op->set_brief("Get absolute value of S into D.");
    op->set_instr("ABS     D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = ABS(S).");
    op->add_descr("C = S[31].");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Negate S into D.
 *
 * EEEE 0110011 CZI DDDDDDDDD SSSSSSSSS
 *
 * NEG     D,{#}S   {WC/WZ/WCZ}
 *
 * D = -S.
 * C = MSB of result.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_NEG(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0110011 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_NEG);
    params_D_IM_S_WCZ(op);
    op->set_brief("Negate S into D.");
    op->set_instr("NEG     D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = -S.");
    op->add_descr("C = MSB of result.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Negate S by  C into D.
 *
 * EEEE 0110100 CZI DDDDDDDDD SSSSSSSSS
 *
 * NEGC    D,{#}S   {WC/WZ/WCZ}
 *
 * If C = 1 then D = -S, else D = S.
 * C = MSB of result.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_NEGC(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0110100 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_NEGC);
    params_D_IM_S_WCZ(op);
    op->set_brief("Negate S by  C into D.");
    op->set_instr("NEGC    D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("If C = 1 then D = -S, else D = S.");
    op->add_descr("C = MSB of result.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Negate S by !C into D.
 *
 * EEEE 0110101 CZI DDDDDDDDD SSSSSSSSS
 *
 * NEGNC   D,{#}S   {WC/WZ/WCZ}
 *
 * If C = 0 then D = -S, else D = S.
 * C = MSB of result.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_NEGNC(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0110101 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_NEGNC);
    params_D_IM_S_WCZ(op);
    op->set_brief("Negate S by !C into D.");
    op->set_instr("NEGNC   D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("If C = 0 then D = -S, else D = S.");
    op->add_descr("C = MSB of result.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Negate S by  Z into D.
 *
 * EEEE 0110110 CZI DDDDDDDDD SSSSSSSSS
 *
 * NEGZ    D,{#}S   {WC/WZ/WCZ}
 *
 * If Z = 1 then D = -S, else D = S.
 * C = MSB of result.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_NEGZ(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0110110 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_NEGZ);
    params_D_IM_S_WCZ(op);
    op->set_brief("Negate S by  Z into D.");
    op->set_instr("NEGZ    D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("If Z = 1 then D = -S, else D = S.");
    op->add_descr("C = MSB of result.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Negate S by !Z into D.
 *
 * EEEE 0110111 CZI DDDDDDDDD SSSSSSSSS
 *
 * NEGNZ   D,{#}S   {WC/WZ/WCZ}
 *
 * If Z = 0 then D = -S, else D = S.
 * C = MSB of result.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_NEGNZ(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0110111 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_NEGNZ);
    params_D_IM_S_WCZ(op);
    op->set_brief("Negate S by !Z into D.");
    op->set_instr("NEGNZ   D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("If Z = 0 then D = -S, else D = S.");
    op->add_descr("C = MSB of result.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Increment with modulus.
 *
 * EEEE 0111000 CZI DDDDDDDDD SSSSSSSSS
 *
 * INCMOD  D,{#}S   {WC/WZ/WCZ}
 *
 * If D = S then D = 0 and C = 1, else D = D + 1 and C = 0.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_INCMOD(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0111000 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_INCMOD);
    params_D_IM_S_WCZ(op);
    op->set_brief("Increment with modulus.");
    op->set_instr("INCMOD  D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("If D = S then D = 0 and C = 1, else D = D + 1 and C = 0.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Decrement with modulus.
 *
 * EEEE 0111001 CZI DDDDDDDDD SSSSSSSSS
 *
 * DECMOD  D,{#}S   {WC/WZ/WCZ}
 *
 * If D = 0 then D = S and C = 1, else D = D - 1 and C = 0.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_DECMOD(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0111001 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_DECMOD);
    params_D_IM_S_WCZ(op);
    op->set_brief("Decrement with modulus.");
    op->set_instr("DECMOD  D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("If D = 0 then D = S and C = 1, else D = D - 1 and C = 0.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Zero-extend D above bit S[4:0].
 *
 * EEEE 0111010 CZI DDDDDDDDD SSSSSSSSS
 *
 * ZEROX   D,{#}S   {WC/WZ/WCZ}
 *
 * C = MSB of result.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_ZEROX(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0111010 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ZEROX);
    params_D_IM_S_WCZ(op);
    op->set_brief("Zero-extend D above bit S[4:0].");
    op->set_instr("ZEROX   D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("C = MSB of result.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Sign-extend D from bit S[4:0].
 *
 * EEEE 0111011 CZI DDDDDDDDD SSSSSSSSS
 *
 * SIGNX   D,{#}S   {WC/WZ/WCZ}
 *
 * C = MSB of result.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_SIGNX(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0111011 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_SIGNX);
    params_D_IM_S_WCZ(op);
    op->set_brief("Sign-extend D from bit S[4:0].");
    op->set_instr("SIGNX   D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("C = MSB of result.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Get bit position of top-most '1' in S into D.
 *
 * EEEE 0111100 CZI DDDDDDDDD SSSSSSSSS
 *
 * ENCOD   D,{#}S   {WC/WZ/WCZ}
 *
 * D = position of top '1' in S (0..31).
 * C = (S != 0).
 * Z = (result == 0).
 *
 */
void P2Doc::doc_ENCOD(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0111100 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ENCOD);
    params_D_IM_S_WCZ(op);
    op->set_brief("Get bit position of top-most '1' in S into D.");
    op->set_instr("ENCOD   D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = position of top '1' in S (0..31).");
    op->add_descr("C = (S != 0).");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Get number of '1's in S into D.
 *
 * EEEE 0111101 CZI DDDDDDDDD SSSSSSSSS
 *
 * ONES    D,{#}S   {WC/WZ/WCZ}
 *
 * D = number of '1's in S (0..32).
 * C = LSB of result.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_ONES(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0111101 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ONES);
    params_D_IM_S_WCZ(op);
    op->set_brief("Get number of '1's in S into D.");
    op->set_instr("ONES    D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("D = number of '1's in S (0..32).");
    op->add_descr("C = LSB of result.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Test D with S.
 *
 * EEEE 0111110 CZI DDDDDDDDD SSSSSSSSS
 *
 * TEST    D,{#}S   {WC/WZ/WCZ}
 *
 * C = parity of (D & S).
 * Z = ((D & S) == 0).
 *
 */
void P2Doc::doc_TEST(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0111110 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_TEST);
    params_D_IM_S_WCZ(op);
    op->set_brief("Test D with S.");
    op->set_instr("TEST    D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("C = parity of (D & S).");
    op->add_descr("Z = ((D & S) == 0).");
}

/**
 * @brief Test D with !S.
 *
 * EEEE 0111111 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTN   D,{#}S   {WC/WZ/WCZ}
 *
 * C = parity of (D & !S).
 * Z = ((D & !S) == 0).
 *
 */
void P2Doc::doc_TESTN(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 0111111 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_TESTN);
    params_D_IM_S_WCZ(op);
    op->set_brief("Test D with !S.");
    op->set_instr("TESTN   D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("C = parity of (D & !S).");
    op->add_descr("Z = ((D & !S) == 0).");
}

/**
 * @brief Set S[3:0] into nibble N in D, keeping rest of D same.
 *
 * EEEE 100000N NNI DDDDDDDDD SSSSSSSSS
 *
 * SETNIB  D,{#}S,#N
 *
 *
 */
void P2Doc::doc_SETNIB(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 100000N NNI DDDDDDDDD SSSSSSSSS", match_NNN);

    op->set_token(t_SETNIB);
    op->set_brief("Set S[3:0] into nibble N in D, keeping rest of D same.");
    op->set_instr("SETNIB  D,{#}S,#N");
}

/**
 * @brief Set S[3:0] into nibble established by prior ALTSN instruction.
 *
 * EEEE 1000000 00I 000000000 SSSSSSSSS
 *
 * SETNIB  {#}S
 *
 *
 */
void P2Doc::doc_SETNIB_ALTSN(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1000000 00I 000000000 SSSSSSSSS", match_NNN_D);

    op->set_token(t_SETNIB);
    op->set_brief("Set S[3:0] into nibble established by prior ALTSN instruction.");
    op->set_instr("SETNIB  {#}S");
}

/**
 * @brief Get nibble N of S into D.
 *
 * EEEE 100001N NNI DDDDDDDDD SSSSSSSSS
 *
 * GETNIB  D,{#}S,#N
 *
 * D = {28'b0, S.NIBBLE[N]).
 *
 */
void P2Doc::doc_GETNIB(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 100001N NNI DDDDDDDDD SSSSSSSSS", match_NNN);

    op->set_token(t_GETNIB);
    op->set_brief("Get nibble N of S into D.");
    op->set_instr("GETNIB  D,{#}S,#N");
    op->add_descr("D = {28'b0, S.NIBBLE[N]).");
}

/**
 * @brief Get nibble established by prior ALTGN instruction into D.
 *
 * EEEE 1000010 000 DDDDDDDDD 000000000
 *
 * GETNIB  D
 *
 *
 */
void P2Doc::doc_GETNIB_ALTGN(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1000010 000 DDDDDDDDD 000000000", match_NNN_I_S);

    op->set_token(t_GETNIB);
    op->set_brief("Get nibble established by prior ALTGN instruction into D.");
    op->set_instr("GETNIB  D");
}

/**
 * @brief Rotate-left nibble N of S into D.
 *
 * EEEE 100010N NNI DDDDDDDDD SSSSSSSSS
 *
 * ROLNIB  D,{#}S,#N
 *
 * D = {D[27:0], S.NIBBLE[N]).
 *
 */
void P2Doc::doc_ROLNIB(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 100010N NNI DDDDDDDDD SSSSSSSSS", match_NNN);

    op->set_token(t_ROLNIB);
    op->set_brief("Rotate-left nibble N of S into D.");
    op->set_instr("ROLNIB  D,{#}S,#N");
    op->add_descr("D = {D[27:0], S.NIBBLE[N]).");
}

/**
 * @brief Rotate-left nibble established by prior ALTGN instruction into D.
 *
 * EEEE 1000100 000 DDDDDDDDD 000000000
 *
 * ROLNIB  D
 *
 *
 */
void P2Doc::doc_ROLNIB_ALTGN(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1000100 000 DDDDDDDDD 000000000", match_NNN_I_S);

    op->set_token(t_ROLNIB);
    op->set_brief("Rotate-left nibble established by prior ALTGN instruction into D.");
    op->set_instr("ROLNIB  D");
}

/**
 * @brief Set S[7:0] into byte N in D, keeping rest of D same.
 *
 * EEEE 1000110 NNI DDDDDDDDD SSSSSSSSS
 *
 * SETBYTE D,{#}S,#N
 *
 *
 */
void P2Doc::doc_SETBYTE(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1000110 NNI DDDDDDDDD SSSSSSSSS", match_NN);

    op->set_token(t_SETBYTE);
    op->set_brief("Set S[7:0] into byte N in D, keeping rest of D same.");
    op->set_instr("SETBYTE D,{#}S,#N");
}

/**
 * @brief Set S[7:0] into byte established by prior ALTSB instruction.
 *
 * EEEE 1000110 00I 000000000 SSSSSSSSS
 *
 * SETBYTE {#}S
 *
 */
void P2Doc::doc_SETBYTE_ALTSB(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1000110 00I 000000000 SSSSSSSSS", match_NN_D);

    op->set_token(t_SETBYTE);
    op->set_brief("Set S[7:0] into byte established by prior ALTSB instruction.");
    op->set_instr("SETBYTE {#}S");
}

/**
 * @brief Get byte N of S into D.
 *
 * EEEE 1000111 NNI DDDDDDDDD SSSSSSSSS
 *
 * GETBYTE D,{#}S,#N
 *
 * D = {24'b0, S.BYTE[N]).
 *
 */
void P2Doc::doc_GETBYTE(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1000111 NNI DDDDDDDDD SSSSSSSSS", match_NN);

    op->set_token(t_GETBYTE);
    op->set_brief("Get byte N of S into D.");
    op->set_instr("GETBYTE D,{#}S,#N");
    op->add_descr("D = {24'b0, S.BYTE[N]).");
}

/**
 * @brief Get byte established by prior ALTGB instruction into D.
 *
 * EEEE 1000111 000 DDDDDDDDD 000000000
 *
 * GETBYTE D
 *
 */
void P2Doc::doc_GETBYTE_ALTGB(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1000111 000 DDDDDDDDD 000000000", match_NN_I_S);

    op->set_token(t_GETBYTE);
    op->set_brief("Get byte established by prior ALTGB instruction into D.");
    op->set_instr("GETBYTE D");
}

/**
 * @brief Rotate-left byte N of S into D.
 *
 * EEEE 1001000 NNI DDDDDDDDD SSSSSSSSS
 *
 * ROLBYTE D,{#}S,#N
 *
 * D = {D[23:0], S.BYTE[N]).
 *
 */
void P2Doc::doc_ROLBYTE(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001000 NNI DDDDDDDDD SSSSSSSSS", match_NN);

    op->set_token(t_ROLBYTE);
    op->set_brief("Rotate-left byte N of S into D.");
    op->set_instr("ROLBYTE D,{#}S,#N");
    op->add_descr("D = {D[23:0], S.BYTE[N]).");
}

/**
 * @brief Rotate-left byte established by prior ALTGB instruction into D.
 *
 * EEEE 1001000 000 DDDDDDDDD 000000000
 *
 * ROLBYTE D
 *
 */
void P2Doc::doc_ROLBYTE_ALTGB(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001000 000 DDDDDDDDD 000000000", match_NN_I_S);

    op->set_token(t_ROLBYTE);
    op->set_brief("Rotate-left byte established by prior ALTGB instruction into D.");
    op->set_instr("ROLBYTE D");
}

/**
 * @brief Set S[15:0] into word N in D, keeping rest of D same.
 *
 * EEEE 1001001 0NI DDDDDDDDD SSSSSSSSS
 *
 * SETWORD D,{#}S,#N
 *
 */
void P2Doc::doc_SETWORD(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001001 0NI DDDDDDDDD SSSSSSSSS", match_N);

    op->set_token(t_SETWORD);
    op->set_brief("Set S[15:0] into word N in D, keeping rest of D same.");
    op->set_instr("SETWORD D,{#}S,#N");
}

/**
 * @brief Set S[15:0] into word established by prior ALTSW instruction.
 *
 * EEEE 1001001 00I 000000000 SSSSSSSSS
 *
 * SETWORD {#}S
 *
 *
 */
void P2Doc::doc_SETWORD_ALTSW(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001001 00I 000000000 SSSSSSSSS", match_D);

    op->set_token(t_SETWORD);
    op->set_brief("Set S[15:0] into word established by prior ALTSW instruction.");
    op->set_instr("SETWORD {#}S");
}

/**
 * @brief Get word N of S into D.
 *
 * EEEE 1001001 1NI DDDDDDDDD SSSSSSSSS
 *
 * GETWORD D,{#}S,#N
 *
 * D = {16'b0, S.WORD[N]).
 *
 */
void P2Doc::doc_GETWORD(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001001 1NI DDDDDDDDD SSSSSSSSS", match_N);

    op->set_token(t_GETWORD);
    op->set_brief("Get word N of S into D.");
    op->set_instr("GETWORD D,{#}S,#N");
    op->add_descr("D = {16'b0, S.WORD[N]).");
}

/**
 * @brief Get word established by prior ALTGW instruction into D.
 *
 * EEEE 1001001 100 DDDDDDDDD 000000000
 *
 * GETWORD D
 *
 *
 */
void P2Doc::doc_GETWORD_ALTGW(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001001 100 DDDDDDDDD 000000000", match_S);

    op->set_token(t_GETWORD);
    op->set_brief("Get word established by prior ALTGW instruction into D.");
    op->set_instr("GETWORD D");
}

/**
 * @brief Rotate-left word N of S into D.
 *
 * EEEE 1001010 0NI DDDDDDDDD SSSSSSSSS
 *
 * ROLWORD D,{#}S,#N
 *
 * D = {D[15:0], S.WORD[N]).
 *
 */
void P2Doc::doc_ROLWORD(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001010 0NI DDDDDDDDD SSSSSSSSS", match_N);

    op->set_token(t_ROLWORD);
    op->set_brief("Rotate-left word N of S into D.");
    op->set_instr("ROLWORD D,{#}S,#N");
    op->add_descr("D = {D[15:0], S.WORD[N]).");
}

/**
 * @brief Rotate-left word established by prior ALTGW instruction into D.
 *
 * EEEE 1001010 000 DDDDDDDDD 000000000
 *
 * ROLWORD D
 *
 *
 */
void P2Doc::doc_ROLWORD_ALTGW(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001010 000 DDDDDDDDD 000000000", match_S);

    op->set_token(t_ROLWORD);
    op->set_brief("Rotate-left word established by prior ALTGW instruction into D.");
    op->set_instr("ROLWORD D");
}

/**
 * @brief Alter subsequent SETNIB instruction.
 *
 * EEEE 1001010 10I DDDDDDDDD SSSSSSSSS
 *
 * ALTSN   D,{#}S
 *
 * Next D field = (D[11:3] + S) & $1FF, N field = D[2:0].
 * D += sign-extended S[17:9].
 *
 */
void P2Doc::doc_ALTSN(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001010 10I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ALTSN);
    op->set_brief("Alter subsequent SETNIB instruction.");
    op->set_instr("ALTSN   D,{#}S");
    op->add_descr("Next D field = (D[11:3] + S) & $1FF, N field = D[2:0].");
    op->add_descr("D += sign-extended S[17:9].");
}

/**
 * @brief Alter subsequent SETNIB instruction.
 *
 * EEEE 1001010 101 DDDDDDDDD 000000000
 *
 * ALTSN   D
 *
 * Next D field = D[11:3], N field = D[2:0].
 *
 */
void P2Doc::doc_ALTSN_D(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001010 101 DDDDDDDDD 000000000", match_I_S);

    op->set_token(t_ALTSN);
    op->set_brief("Alter subsequent SETNIB instruction.");
    op->set_instr("ALTSN   D");
    op->add_descr("Next D field = D[11:3], N field = D[2:0].");
}

/**
 * @brief Alter subsequent GETNIB/ROLNIB instruction.
 *
 * EEEE 1001010 11I DDDDDDDDD SSSSSSSSS
 *
 * ALTGN   D,{#}S
 *
 * Next S field = (D[11:3] + S) & $1FF, N field = D[2:0].
 * D += sign-extended S[17:9].
 *
 */
void P2Doc::doc_ALTGN(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001010 11I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ALTGN);
    op->set_brief("Alter subsequent GETNIB/ROLNIB instruction.");
    op->set_instr("ALTGN   D,{#}S");
    op->add_descr("Next S field = (D[11:3] + S) & $1FF, N field = D[2:0].");
    op->add_descr("D += sign-extended S[17:9].");
}

/**
 * @brief Alter subsequent GETNIB/ROLNIB instruction.
 *
 * EEEE 1001010 111 DDDDDDDDD 000000000
 *
 * ALTGN   D
 *
 * Next S field = D[11:3], N field = D[2:0].
 *
 */
void P2Doc::doc_ALTGN_D(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001010 111 DDDDDDDDD 000000000", match_I_S);

    op->set_token(t_ALTGN);
    op->set_brief("Alter subsequent GETNIB/ROLNIB instruction.");
    op->set_instr("ALTGN   D");
    op->add_descr("Next S field = D[11:3], N field = D[2:0].");
}

/**
 * @brief Alter subsequent SETBYTE instruction.
 *
 * EEEE 1001011 00I DDDDDDDDD SSSSSSSSS
 *
 * ALTSB   D,{#}S
 *
 * Next D field = (D[10:2] + S) & $1FF, N field = D[1:0].
 * D += sign-extended S[17:9].
 *
 */
void P2Doc::doc_ALTSB(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001011 00I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ALTSB);
    op->set_brief("Alter subsequent SETBYTE instruction.");
    op->set_instr("ALTSB   D,{#}S");
    op->add_descr("Next D field = (D[10:2] + S) & $1FF, N field = D[1:0].");
    op->add_descr("D += sign-extended S[17:9].");
}

/**
 * @brief Alter subsequent SETBYTE instruction.
 *
 * EEEE 1001011 001 DDDDDDDDD 000000000
 *
 * ALTSB   D
 *
 * Next D field = D[10:2], N field = D[1:0].
 *
 */
void P2Doc::doc_ALTSB_D(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001011 001 DDDDDDDDD 000000000", match_I_S);

    op->set_token(t_ALTSB);
    op->set_brief("Alter subsequent SETBYTE instruction.");
    op->set_instr("ALTSB   D");
    op->add_descr("Next D field = D[10:2], N field = D[1:0].");
}

/**
 * @brief Alter subsequent GETBYTE/ROLBYTE instruction.
 *
 * EEEE 1001011 01I DDDDDDDDD SSSSSSSSS
 *
 * ALTGB   D,{#}S
 *
 * Next S field = (D[10:2] + S) & $1FF, N field = D[1:0].
 * D += sign-extended S[17:9].
 *
 */
void P2Doc::doc_ATLGB(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001011 01I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ALTGB);
    op->set_brief("Alter subsequent GETBYTE/ROLBYTE instruction.");
    op->set_instr("ALTGB   D,{#}S");
    op->add_descr("Next S field = (D[10:2] + S) & $1FF, N field = D[1:0].");
    op->add_descr("D += sign-extended S[17:9].");
}

/**
 * @brief Alter subsequent GETBYTE/ROLBYTE instruction.
 *
 * EEEE 1001011 011 DDDDDDDDD 000000000
 *
 * ALTGB   D
 *
 * Next S field = D[10:2], N field = D[1:0].
 *
 */
void P2Doc::doc_ALTGB_D(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001011 011 DDDDDDDDD 000000000", match_I_S);

    op->set_token(t_ALTGB);
    op->set_brief("Alter subsequent GETBYTE/ROLBYTE instruction.");
    op->set_instr("ALTGB   D");
    op->add_descr("Next S field = D[10:2], N field = D[1:0].");
}

/**
 * @brief Alter subsequent SETWORD instruction.
 *
 * EEEE 1001011 10I DDDDDDDDD SSSSSSSSS
 *
 * ALTSW   D,{#}S
 *
 * Next D field = (D[9:1] + S) & $1FF, N field = D[0].
 * D += sign-extended S[17:9].
 *
 */
void P2Doc::doc_ALTSW(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001011 10I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ALTSW);
    op->set_brief("Alter subsequent SETWORD instruction.");
    op->set_instr("ALTSW   D,{#}S");
    op->add_descr("Next D field = (D[9:1] + S) & $1FF, N field = D[0].");
    op->add_descr("D += sign-extended S[17:9].");
}

/**
 * @brief Alter subsequent SETWORD instruction.
 *
 * EEEE 1001011 101 DDDDDDDDD 000000000
 *
 * ALTSW   D
 *
 * Next D field = D[9:1], N field = D[0].
 *
 */
void P2Doc::doc_ALTSW_D(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001011 101 DDDDDDDDD 000000000", match_I_S);

    op->set_token(t_ALTSW);
    op->set_brief("Alter subsequent SETWORD instruction.");
    op->set_instr("ALTSW   D");
    op->add_descr("Next D field = D[9:1], N field = D[0].");
}

/**
 * @brief Alter subsequent GETWORD/ROLWORD instruction.
 *
 * EEEE 1001011 11I DDDDDDDDD SSSSSSSSS
 *
 * ALTGW   D,{#}S
 *
 * Next S field = ((D[9:1] + S) & $1FF), N field = D[0].
 * D += sign-extended S[17:9].
 *
 */
void P2Doc::doc_ALTGW(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001011 11I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ALTGW);
    op->set_brief("Alter subsequent GETWORD/ROLWORD instruction.");
    op->set_instr("ALTGW   D,{#}S");
    op->add_descr("Next S field = ((D[9:1] + S) & $1FF), N field = D[0].");
    op->add_descr("D += sign-extended S[17:9].");
}

/**
 * @brief Alter subsequent GETWORD/ROLWORD instruction.
 *
 * EEEE 1001011 111 DDDDDDDDD 000000000
 *
 * ALTGW   D
 *
 * Next S field = D[9:1], N field = D[0].
 *
 */
void P2Doc::doc_ALTGW_D(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001011 111 DDDDDDDDD 000000000", match_I_S);

    op->set_token(t_ALTGW);
    op->set_brief("Alter subsequent GETWORD/ROLWORD instruction.");
    op->set_instr("ALTGW   D");
    op->add_descr("Next S field = D[9:1], N field = D[0].");
}

/**
 * @brief Alter result register address (normally D field) of next instruction to (D + S) & $1FF.
 *
 * EEEE 1001100 00I DDDDDDDDD SSSSSSSSS
 *
 * ALTR    D,{#}S
 *
 * D += sign-extended S[17:9].
 *
 */
void P2Doc::doc_ALTR(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001100 00I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ALTR);
    op->set_brief("Alter result register address (normally D field) of next instruction to (D + S) & $1FF.");
    op->set_instr("ALTR    D,{#}S");
    op->add_descr("D += sign-extended S[17:9].");
}

/**
 * @brief Alter result register address (normally D field) of next instruction to D[8:0].
 *
 * EEEE 1001100 001 DDDDDDDDD 000000000
 *
 * ALTR    D
 *
 *
 */
void P2Doc::doc_ALTR_D(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001100 001 DDDDDDDDD 000000000", match_I_S);

    op->set_token(t_ALTD);
    op->set_brief("Alter result register address (normally D field) of next instruction to D[8:0].");
    op->set_instr("ALTR    D");
}

/**
 * @brief Alter D field of next instruction to (D + S) & $1FF.
 *
 * EEEE 1001100 01I DDDDDDDDD SSSSSSSSS
 *
 * ALTD    D,{#}S
 *
 * D += sign-extended S[17:9].
 *
 */
void P2Doc::doc_ALTD(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001100 01I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ALTD);
    op->set_brief("Alter D field of next instruction to (D + S) & $1FF.");
    op->set_instr("ALTD    D,{#}S");
    op->add_descr("D += sign-extended S[17:9].");
}

/**
 * @brief Alter D field of next instruction to D[8:0].
 *
 * EEEE 1001100 011 DDDDDDDDD 000000000
 *
 * ALTD    D
 *
 *
 */
void P2Doc::doc_ALTD_D(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001100 011 DDDDDDDDD 000000000", match_I_S);

    op->set_token(t_ALTD);
    op->set_brief("Alter D field of next instruction to D[8:0].");
    op->set_instr("ALTD    D");
}

/**
 * @brief Alter S field of next instruction to (D + S) & $1FF.
 *
 * EEEE 1001100 10I DDDDDDDDD SSSSSSSSS
 *
 * ALTS    D,{#}S
 *
 * D += sign-extended S[17:9].
 *
 */
void P2Doc::doc_ALTS(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001100 10I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ALTS);
    op->set_brief("Alter S field of next instruction to (D + S) & $1FF.");
    op->set_instr("ALTS    D,{#}S");
    op->add_descr("D += sign-extended S[17:9].");
}

/**
 * @brief Alter S field of next instruction to D[8:0].
 *
 * EEEE 1001100 101 DDDDDDDDD 000000000
 *
 * ALTS    D
 *
 *
 */
void P2Doc::doc_ALTS_D(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001100 101 DDDDDDDDD 000000000", match_I_S);

    op->set_token(t_ALTS);
    op->set_brief("Alter S field of next instruction to D[8:0].");
    op->set_instr("ALTS    D");
}

/**
 * @brief Alter D field of next instruction to (D[13:5] + S) & $1FF.
 *
 * EEEE 1001100 11I DDDDDDDDD SSSSSSSSS
 *
 * ALTB    D,{#}S
 *
 * D += sign-extended S[17:9].
 *
 */
void P2Doc::doc_ALTB(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001100 11I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ALTB);
    op->set_brief("Alter D field of next instruction to (D[13:5] + S) & $1FF.");
    op->set_instr("ALTB    D,{#}S");
    op->add_descr("D += sign-extended S[17:9].");
}

/**
 * @brief Alter D field of next instruction to D[13:5].
 *
 * EEEE 1001100 111 DDDDDDDDD 000000000
 *
 * ALTB    D
 *
 *
 */
void P2Doc::doc_ALTB_D(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001100 111 DDDDDDDDD 000000000", match_I_S);

    op->set_token(t_ALTB);
    op->set_brief("Alter D field of next instruction to D[13:5].");
    op->set_instr("ALTB    D");
}

/**
 * @brief Substitute next instruction's I/R/D/S fields with fields from D, per S.
 *
 * EEEE 1001101 00I DDDDDDDDD SSSSSSSSS
 *
 * ALTI    D,{#}S
 *
 * Modify D per S.
 *
 */
void P2Doc::doc_ALTI(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001101 00I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ALTI);
    op->set_brief("Substitute next instruction's I/R/D/S fields with fields from D, per S.");
    op->set_instr("ALTI    D,{#}S");
    op->add_descr("Modify D per S.");
}

/**
 * @brief Execute D in place of next instruction.
 *
 * EEEE 1001101 001 DDDDDDDDD 101100100
 *
 * ALTI    D
 *
 * D stays same.
 *
 */
void P2Doc::doc_ALTI_D(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001101 001 DDDDDDDDD 101100100", match_I_S);

    op->set_token(t_ALTI);
    op->set_brief("Execute D in place of next instruction.");
    op->set_instr("ALTI    D");
    op->add_descr("D stays same.");
}

/**
 * @brief Set R field of D to S[8:0].
 *
 * EEEE 1001101 01I DDDDDDDDD SSSSSSSSS
 *
 * SETR    D,{#}S
 *
 * D = {D[31:28], S[8:0], D[18:0]}.
 *
 */
void P2Doc::doc_SETR(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001101 01I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_SETR);
    op->set_brief("Set R field of D to S[8:0].");
    op->set_instr("SETR    D,{#}S");
    op->add_descr("D = {D[31:28], S[8:0], D[18:0]}.");
}

/**
 * @brief Set D field of D to S[8:0].
 *
 * EEEE 1001101 10I DDDDDDDDD SSSSSSSSS
 *
 * SETD    D,{#}S
 *
 * D = {D[31:18], S[8:0], D[8:0]}.
 *
 */
void P2Doc::doc_SETD(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001101 10I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_SETD);
    op->set_brief("Set D field of D to S[8:0].");
    op->set_instr("SETD    D,{#}S");
    op->add_descr("D = {D[31:18], S[8:0], D[8:0]}.");
}

/**
 * @brief Set S field of D to S[8:0].
 *
 * EEEE 1001101 11I DDDDDDDDD SSSSSSSSS
 *
 * SETS    D,{#}S
 *
 * D = {D[31:9], S[8:0]}.
 *
 */
void P2Doc::doc_SETS(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001101 11I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_SETS);
    op->set_brief("Set S field of D to S[8:0].");
    op->set_instr("SETS    D,{#}S");
    op->add_descr("D = {D[31:9], S[8:0]}.");
}

/**
 * @brief Decode S[4:0] into D.
 *
 * EEEE 1001110 00I DDDDDDDDD SSSSSSSSS
 *
 * DECOD   D,{#}S
 *
 * D = 1 << S[4:0].
 *
 */
void P2Doc::doc_DECOD(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001110 00I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_DECOD);
    op->set_brief("Decode S[4:0] into D.");
    op->set_instr("DECOD   D,{#}S");
    op->add_descr("D = 1 << S[4:0].");
}

/**
 * @brief Decode D[4:0] into D.
 *
 * EEEE 1001110 000 DDDDDDDDD DDDDDDDDD
 *
 * DECOD   D
 *
 * D = 1 << D[4:0].
 *
 */
void P2Doc::doc_DECOD_D(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001110 000 DDDDDDDDD DDDDDDDDD", match_D_equ_S);

    op->set_token(t_DECOD);
    op->set_brief("Decode D[4:0] into D.");
    op->set_instr("DECOD   D");
    op->add_descr("D = 1 << D[4:0].");
}

/**
 * @brief Get LSB-justified bit mask of size (S[4:0] + 1) into D.
 *
 * EEEE 1001110 01I DDDDDDDDD SSSSSSSSS
 *
 * BMASK   D,{#}S
 *
 * D = ($0000_0002 << S[4:0]) - 1.
 *
 */
void P2Doc::doc_BMASK(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001110 01I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_BMASK);
    op->set_brief("Get LSB-justified bit mask of size (S[4:0] + 1) into D.");
    op->set_instr("BMASK   D,{#}S");
    op->add_descr("D = ($0000_0002 << S[4:0]) - 1.");
}

/**
 * @brief Get LSB-justified bit mask of size (D[4:0] + 1) into D.
 *
 * EEEE 1001110 010 DDDDDDDDD DDDDDDDDD
 *
 * BMASK   D
 *
 * D = ($0000_0002 << D[4:0]) - 1.
 *
 */
void P2Doc::doc_BMASK_D(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001110 010 DDDDDDDDD DDDDDDDDD", match_D_equ_S);

    op->set_token(t_BMASK);
    op->set_brief("Get LSB-justified bit mask of size (D[4:0] + 1) into D.");
    op->set_instr("BMASK   D");
    op->add_descr("D = ($0000_0002 << D[4:0]) - 1.");
}

/**
 * @brief Iterate CRC value in D using C and polynomial in S.
 *
 * EEEE 1001110 10I DDDDDDDDD SSSSSSSSS
 *
 * CRCBIT  D,{#}S
 *
 * If (C XOR D[0]) then D = (D >> 1) XOR S, else D = (D >> 1).
 *
 */
void P2Doc::doc_CRCBIT(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001110 10I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_CRCBIT);
    op->set_brief("Iterate CRC value in D using C and polynomial in S.");
    op->set_instr("CRCBIT  D,{#}S");
    op->add_descr("If (C XOR D[0]) then D = (D >> 1) XOR S, else D = (D >> 1).");
}

/**
 * @brief Iterate CRC value in D using Q[31:28] and polynomial in S.
 *
 * EEEE 1001110 11I DDDDDDDDD SSSSSSSSS
 *
 * CRCNIB  D,{#}S
 *
 * Like CRCBIT, but 4x.
 * Q = Q << 4.
 * Use SETQ+CRCNIB+CRCNIB+CRCNIB.
 *
 */
void P2Doc::doc_CRCNIB(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001110 11I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_CRCNIB);
    op->set_brief("Iterate CRC value in D using Q[31:28] and polynomial in S.");
    op->set_instr("CRCNIB  D,{#}S");
    op->add_descr("Like CRCBIT, but 4x.");
    op->add_descr("Q = Q << 4.");
    op->add_descr("Use SETQ+CRCNIB+CRCNIB+CRCNIB.");
}

/**
 * @brief For each non-zero bit pair in S, copy that bit pair into the corresponding D bits, else leave that D bit pair the same.
 *
 * EEEE 1001111 00I DDDDDDDDD SSSSSSSSS
 *
 * MUXNITS D,{#}S
 *
 *
 */
void P2Doc::doc_MUXNITS(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001111 00I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_MUXNITS);
    op->set_brief("For each non-zero bit pair in S, copy that bit pair into the corresponding D bits, else leave that D bit pair the same.");
    op->set_instr("MUXNITS D,{#}S");
}

/**
 * @brief For each non-zero nibble in S, copy that nibble into the corresponding D nibble, else leave that D nibble the same.
 *
 * EEEE 1001111 01I DDDDDDDDD SSSSSSSSS
 *
 * MUXNIBS D,{#}S
 *
 *
 */
void P2Doc::doc_MUXNIBS(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001111 01I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_MUXNIBS);
    op->set_brief("For each non-zero nibble in S, copy that nibble into the corresponding D nibble, else leave that D nibble the same.");
    op->set_instr("MUXNIBS D,{#}S");
}

/**
 * @brief Used after SETQ.
 *
 * EEEE 1001111 10I DDDDDDDDD SSSSSSSSS
 *
 * MUXQ    D,{#}S
 *
 * For each '1' bit in Q, copy the corresponding bit in S into D.
 * D = (D & !Q) | (S & Q).
 *
 */
void P2Doc::doc_MUXQ(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001111 10I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_MUXQ);
    op->set_brief("For each '1' bit in Q, copy the corresponding bit in S into D.");
    op->set_instr("MUXQ    D,{#}S");
    op->add_descr("D = (D & !Q) | (S & Q).");
    op->add_descr("Used after SETQ.");
}

/**
 * @brief Move bytes within D, per S.
 *
 * EEEE 1001111 11I DDDDDDDDD SSSSSSSSS
 *
 * MOVBYTS D,{#}S
 *
 * D = {D.BYTE[S[7:6]], D.BYTE[S[5:4]], D.BYTE[S[3:2]], D.BYTE[S[1:0]]}.
 *
 */
void P2Doc::doc_MOVBYTS(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1001111 11I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_MOVBYTS);
    op->set_brief("Move bytes within D, per S.");
    op->set_instr("MOVBYTS D,{#}S");
    op->add_descr("D = {D.BYTE[S[7:6]], D.BYTE[S[5:4]], D.BYTE[S[3:2]], D.BYTE[S[1:0]]}.");
}

/**
 * @brief D = unsigned (D[15:0] * S[15:0]).
 *
 * EEEE 1010000 0ZI DDDDDDDDD SSSSSSSSS
 *
 * MUL     D,{#}S          {WZ}
 *
 * Z = (S == 0) | (D == 0).
 *
 */
void P2Doc::doc_MUL(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1010000 0ZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_MUL);
    op->set_brief("D = unsigned (D[15:0] * S[15:0]).");
    op->set_instr("MUL     D,{#}S          {WZ}");
    op->add_descr("Z = (S == 0) | (D == 0).");
}

/**
 * @brief D = signed (D[15:0] * S[15:0]).
 *
 * EEEE 1010000 1ZI DDDDDDDDD SSSSSSSSS
 *
 * MULS    D,{#}S          {WZ}
 *
 * Z = (S == 0) | (D == 0).
 *
 */
void P2Doc::doc_MULS(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1010000 1ZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_MULS);
    op->set_brief("D = signed (D[15:0] * S[15:0]).");
    op->set_instr("MULS    D,{#}S          {WZ}");
    op->add_descr("Z = (S == 0) | (D == 0).");
}

/**
 * @brief Next instruction's S value = unsigned (D[15:0] * S[15:0]) >> 16.
 *
 * EEEE 1010001 0ZI DDDDDDDDD SSSSSSSSS
 *
 * SCA     D,{#}S          {WZ}
 *
 * Z = (result == 0).
 *
 */
void P2Doc::doc_SCA(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1010001 0ZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_SCA);
    op->set_brief("Next instruction's S value = unsigned (D[15:0] * S[15:0]) >> 16.");
    op->set_instr("SCA     D,{#}S          {WZ}");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Next instruction's S value = signed (D[15:0] * S[15:0]) >> 14.
 *
 * EEEE 1010001 1ZI DDDDDDDDD SSSSSSSSS
 *
 * SCAS    D,{#}S          {WZ}
 *
 * In this scheme, $4000 = 1.0 and $C000 = -1.0.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_SCAS(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1010001 1ZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_SCAS);
    op->set_brief("Next instruction's S value = signed (D[15:0] * S[15:0]) >> 14.");
    op->set_instr("SCAS    D,{#}S          {WZ}");
    op->add_descr("In this scheme, $4000 = 1.0 and $C000 = -1.0.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Add bytes of S into bytes of D, with $FF saturation.
 *
 * EEEE 1010010 00I DDDDDDDDD SSSSSSSSS
 *
 * ADDPIX  D,{#}S
 *
 *
 */
void P2Doc::doc_ADDPIX(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1010010 00I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ADDPIX);
    op->set_brief("Add bytes of S into bytes of D, with $FF saturation.");
    op->set_instr("ADDPIX  D,{#}S");
}

/**
 * @brief Multiply bytes of S into bytes of D, where $FF = 1.
 *
 * EEEE 1010010 01I DDDDDDDDD SSSSSSSSS
 *
 * MULPIX  D,{#}S
 *
 * 0.
 *
 */
void P2Doc::doc_MULPIX(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1010010 01I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_MULPIX);
    op->set_brief("Multiply bytes of S into bytes of D, where $FF = 1.");
    op->set_instr("MULPIX  D,{#}S");
    op->add_descr("0.");
}

/**
 * @brief Alpha-blend bytes of S into bytes of D, using SETPIV value.
 *
 * EEEE 1010010 10I DDDDDDDDD SSSSSSSSS
 *
 * BLNPIX  D,{#}S
 *
 *
 */
void P2Doc::doc_BLNPIX(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1010010 10I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_BLNPIX);
    op->set_brief("Alpha-blend bytes of S into bytes of D, using SETPIV value.");
    op->set_instr("BLNPIX  D,{#}S");
}

/**
 * @brief Mix bytes of S into bytes of D, using SETPIX and SETPIV values.
 *
 * EEEE 1010010 11I DDDDDDDDD SSSSSSSSS
 *
 * MIXPIX  D,{#}S
 *
 *
 */
void P2Doc::doc_MIXPIX(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1010010 11I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_MIXPIX);
    op->set_brief("Mix bytes of S into bytes of D, using SETPIX and SETPIV values.");
    op->set_instr("MIXPIX  D,{#}S");
}

/**
 * @brief Set CT1 event to trigger on CT = D + S.
 *
 * EEEE 1010011 00I DDDDDDDDD SSSSSSSSS
 *
 * ADDCT1  D,{#}S
 *
 * Adds S into D.
 *
 */
void P2Doc::doc_ADDCT1(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1010011 00I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ADDCT1);
    op->set_brief("Set CT1 event to trigger on CT = D + S.");
    op->set_instr("ADDCT1  D,{#}S");
    op->add_descr("Adds S into D.");
}

/**
 * @brief Set CT2 event to trigger on CT = D + S.
 *
 * EEEE 1010011 01I DDDDDDDDD SSSSSSSSS
 *
 * ADDCT2  D,{#}S
 *
 * Adds S into D.
 *
 */
void P2Doc::doc_ADDCT2(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1010011 01I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ADDCT2);
    op->set_brief("Set CT2 event to trigger on CT = D + S.");
    op->set_instr("ADDCT2  D,{#}S");
    op->add_descr("Adds S into D.");
}

/**
 * @brief Set CT3 event to trigger on CT = D + S.
 *
 * EEEE 1010011 10I DDDDDDDDD SSSSSSSSS
 *
 * ADDCT3  D,{#}S
 *
 * Adds S into D.
 *
 */
void P2Doc::doc_ADDCT3(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1010011 10I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_ADDCT3);
    op->set_brief("Set CT3 event to trigger on CT = D + S.");
    op->set_instr("ADDCT3  D,{#}S");
    op->add_descr("Adds S into D.");
}

/**
 * @brief Write only non-$00 bytes in D[31:0] to hub address {#}S/PTRx.
 *
 * EEEE 1010011 11I DDDDDDDDD SSSSSSSSS
 *
 * WMLONG  D,{#}S/P
 *
 * Prior SETQ/SETQ2 invokes cog/LUT block transfer.
 *
 */
void P2Doc::doc_WMLONG(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1010011 11I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_WMLONG);
    op->set_brief("Write only non-$00 bytes in D[31:0] to hub address {#}S/PTRx.");
    op->set_instr("WMLONG  D,{#}S/P");
    op->add_descr("Prior SETQ/SETQ2 invokes cog/LUT block transfer.");
}

/**
 * @brief Read smart pin S[5:0] result "Z" into D, don't acknowledge smart pin ("Q" in RQPIN means "quiet").
 *
 * EEEE 1010100 C0I DDDDDDDDD SSSSSSSSS
 *
 * RQPIN   D,{#}S          {WC}
 *
 * C = modal result.
 *
 */
void P2Doc::doc_RQPIN(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1010100 C0I DDDDDDDDD SSSSSSSSS", match_Z);

    op->set_token(t_RQPIN);
    op->set_brief("Read smart pin S[5:0] result \"Z\" into D, don't acknowledge smart pin (\"Q\" in RQPIN means \"quiet\").");
    op->set_instr("RQPIN   D,{#}S          {WC}");
    op->add_descr("C = modal result.");
}

/**
 * @brief Read smart pin S[5:0] result "Z" into D, acknowledge smart pin.
 *
 * EEEE 1010100 C1I DDDDDDDDD SSSSSSSSS
 *
 * RDPIN   D,{#}S          {WC}
 *
 * C = modal result.
 *
 */
void P2Doc::doc_RDPIN(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1010100 C1I DDDDDDDDD SSSSSSSSS", match_Z);

    op->set_token(t_RDPIN);
    op->set_brief("Read smart pin S[5:0] result \"Z\" into D, acknowledge smart pin.");
    op->set_instr("RDPIN   D,{#}S          {WC}");
    op->add_descr("C = modal result.");
}

/**
 * @brief Read LUT data from address S[8:0] into D.
 *
 * EEEE 1010101 CZI DDDDDDDDD SSSSSSSSS
 *
 * RDLUT   D,{#}S   {WC/WZ/WCZ}
 *
 * C = MSB of data.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_RDLUT(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1010101 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_RDLUT);
    params_D_IM_S_WCZ(op);
    op->set_brief("Read LUT data from address S[8:0] into D.");
    op->set_instr("RDLUT   D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("C = MSB of data.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Read zero-extended byte from hub address {#}S/PTRx into D.
 *
 * EEEE 1010110 CZI DDDDDDDDD SSSSSSSSS
 *
 * RDBYTE  D,{#}S/P {WC/WZ/WCZ}
 *
 * C = MSB of byte.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_RDBYTE(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1010110 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_RDBYTE);
    op->set_brief("Read zero-extended byte from hub address {#}S/PTRx into D.");
    op->set_instr("RDBYTE  D,{#}S/P {WC/WZ/WCZ}");
    op->add_descr("C = MSB of byte.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Read zero-extended word from hub address {#}S/PTRx into D.
 *
 * EEEE 1010111 CZI DDDDDDDDD SSSSSSSSS
 *
 * RDWORD  D,{#}S/P {WC/WZ/WCZ}
 *
 * C = MSB of word.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_RDWORD(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1010111 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_RDWORD);
    op->set_brief("Read zero-extended word from hub address {#}S/PTRx into D.");
    op->set_instr("RDWORD  D,{#}S/P {WC/WZ/WCZ}");
    op->add_descr("C = MSB of word.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Read long from hub address {#}S/PTRx into D.
 *
 * EEEE 1011000 CZI DDDDDDDDD SSSSSSSSS
 *
 * RDLONG  D,{#}S/P {WC/WZ/WCZ}
 *
 * C = MSB of long.
 * Prior SETQ/SETQ2 invokes cog/LUT block transfer.
 *
 */
void P2Doc::doc_RDLONG(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011000 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_RDLONG);
    op->set_brief("Read long from hub address {#}S/PTRx into D.");
    op->set_instr("RDLONG  D,{#}S/P {WC/WZ/WCZ}");
    op->add_descr("C = MSB of long.");
    op->add_descr("Prior SETQ/SETQ2 invokes cog/LUT block transfer.");
}

/**
 * @brief Read long from hub address --PTRA into D.
 *
 * EEEE 1011000 CZ1 DDDDDDDDD 101011111
 *
 * POPA    D        {WC/WZ/WCZ}
 *
 * C = MSB of long.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_POPA(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011000 CZ1 DDDDDDDDD 101011111");

    op->set_token(t_POPA);
    op->set_brief("Read long from hub address --PTRA into D.");
    op->set_instr("POPA    D        {WC/WZ/WCZ}");
    op->add_descr("C = MSB of long.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Read long from hub address --PTRB into D.
 *
 * EEEE 1011000 CZ1 DDDDDDDDD 111011111
 *
 * POPB    D        {WC/WZ/WCZ}
 *
 * C = MSB of long.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_POPB(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011000 CZ1 DDDDDDDDD 111011111");

    op->set_token(t_POPB);
    op->set_brief("Read long from hub address --PTRB into D.");
    op->set_instr("POPB    D        {WC/WZ/WCZ}");
    op->add_descr("C = MSB of long.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Call to S** by writing {C, Z, 10'b0, PC[19:0]} to D.
 *
 * EEEE 1011001 CZI DDDDDDDDD SSSSSSSSS
 *
 * CALLD   D,{#}S   {WC/WZ/WCZ}
 *
 * C = S[31], Z = S[30].
 *
 */
void P2Doc::doc_CALLD(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011001 CZI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_CALLD);
    params_D_IM_S_WCZ(op);
    op->set_brief("Call to S** by writing {C, Z, 10'b0, PC[19:0]} to D.");
    op->set_instr("CALLD   D,{#}S   {WC/WZ/WCZ}");
    op->add_descr("C = S[31], Z = S[30].");
}

/**
 * @brief Resume from INT3.
 *
 * EEEE 1011001 110 111110000 111110001
 *
 * RESI3
 *
 * (CALLD $1F0,$1F1 WC,WZ).
 *
 */
void P2Doc::doc_RESI3(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011001 110 111110000 111110001", match_CZI_D_S);

    op->set_token(t_RESI3);
    op->set_brief("Resume from INT3.");
    op->set_instr("RESI3");
    op->add_descr("(CALLD $1F0,$1F1 WC,WZ).");
}

/**
 * @brief Resume from INT2.
 *
 * EEEE 1011001 110 111110010 111110011
 *
 * RESI2
 *
 * (CALLD $1F2,$1F3 WC,WZ).
 *
 */
void P2Doc::doc_RESI2(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011001 110 111110010 111110011", match_CZI_D_S);

    op->set_token(t_RESI2);
    op->set_brief("Resume from INT2.");
    op->set_instr("RESI2");
    op->add_descr("(CALLD $1F2,$1F3 WC,WZ).");
}

/**
 * @brief Resume from INT1.
 *
 * EEEE 1011001 110 111110100 111110101
 *
 * RESI1
 *
 * (CALLD $1F4,$1F5 WC,WZ).
 *
 */
void P2Doc::doc_RESI1(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011001 110 111110100 111110101", match_CZI_D_S);

    op->set_token(t_RESI1);
    op->set_brief("Resume from INT1.");
    op->set_instr("RESI1");
    op->add_descr("(CALLD $1F4,$1F5 WC,WZ).");
}

/**
 * @brief Resume from INT0.
 *
 * EEEE 1011001 110 111111110 111111111
 *
 * RESI0
 *
 * (CALLD $1FE,$1FF WC,WZ).
 *
 */
void P2Doc::doc_RESI0(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011001 110 111111110 111111111", match_CZI_D_S);

    op->set_token(t_RESI0);
    op->set_brief("Resume from INT0.");
    op->set_instr("RESI0");
    op->add_descr("(CALLD $1FE,$1FF WC,WZ).");
}

/**
 * @brief Return from INT3.
 *
 * EEEE 1011001 110 111111111 111110001
 *
 * RETI3
 *
 * (CALLD $1FF,$1F1 WC,WZ).
 *
 */
void P2Doc::doc_RETI3(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011001 110 111111111 111110001", match_CZI_D_S);

    op->set_token(t_RETI3);
    op->set_brief("Return from INT3.");
    op->set_instr("RETI3");
    op->add_descr("(CALLD $1FF,$1F1 WC,WZ).");
}

/**
 * @brief Return from INT2.
 *
 * EEEE 1011001 110 111111111 111110011
 *
 * RETI2
 *
 * (CALLD $1FF,$1F3 WC,WZ).
 *
 */
void P2Doc::doc_RETI2(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011001 110 111111111 111110011", match_CZI_D_S);

    op->set_token(t_RETI2);
    op->set_brief("Return from INT2.");
    op->set_instr("RETI2");
    op->add_descr("(CALLD $1FF,$1F3 WC,WZ).");
}

/**
 * @brief Return from INT1.
 *
 * EEEE 1011001 110 111111111 111110101
 *
 * RETI1
 *
 * (CALLD $1FF,$1F5 WC,WZ).
 *
 */
void P2Doc::doc_RETI1(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011001 110 111111111 111110101", match_CZI_D_S);

    op->set_token(t_RETI1);
    op->set_brief("Return from INT1.");
    op->set_instr("RETI1");
    op->add_descr("(CALLD $1FF,$1F5 WC,WZ).");
}

/**
 * @brief Return from INT0.
 *
 * EEEE 1011001 110 111111111 111111111
 *
 * RETI0
 *
 * (CALLD $1FF,$1FF WC,WZ).
 *
 */
void P2Doc::doc_RETI0(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011001 110 111111111 111111111", match_CZI_D_S);

    op->set_token(t_RETI0);
    op->set_brief("Return from INT0.");
    op->set_instr("RETI0");
    op->add_descr("(CALLD $1FF,$1FF WC,WZ).");
}

/**
 * @brief Call to S** by pushing {C, Z, 10'b0, PC[19:0]} onto stack, copy D to PA.
 *
 * EEEE 1011010 0LI DDDDDDDDD SSSSSSSSS
 *
 * CALLPA  {#}D,{#}S
 *
 *
 */
void P2Doc::doc_CALLPA(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011010 0LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_CALLPA);
    op->set_brief("Call to S** by pushing {C, Z, 10'b0, PC[19:0]} onto stack, copy D to PA.");
    op->set_instr("CALLPA  {#}D,{#}S");
}

/**
 * @brief Call to S** by pushing {C, Z, 10'b0, PC[19:0]} onto stack, copy D to PB.
 *
 * EEEE 1011010 1LI DDDDDDDDD SSSSSSSSS
 *
 * CALLPB  {#}D,{#}S
 *
 *
 */
void P2Doc::doc_CALLPB(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011010 1LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_CALLPB);
    op->set_brief("Call to S** by pushing {C, Z, 10'b0, PC[19:0]} onto stack, copy D to PB.");
    op->set_instr("CALLPB  {#}D,{#}S");
}

/**
 * @brief Decrement D and jump to S** if result is zero.
 *
 * EEEE 1011011 00I DDDDDDDDD SSSSSSSSS
 *
 * DJZ     D,{#}S
 *
 *
 */
void P2Doc::doc_DJZ(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011011 00I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_DJZ);
    op->set_brief("Decrement D and jump to S** if result is zero.");
    op->set_instr("DJZ     D,{#}S");
}

/**
 * @brief Decrement D and jump to S** if result is not zero.
 *
 * EEEE 1011011 01I DDDDDDDDD SSSSSSSSS
 *
 * DJNZ    D,{#}S
 *
 *
 */
void P2Doc::doc_DJNZ(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011011 01I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_DJNZ);
    op->set_brief("Decrement D and jump to S** if result is not zero.");
    op->set_instr("DJNZ    D,{#}S");
}

/**
 * @brief Decrement D and jump to S** if result is $FFFF_FFFF.
 *
 * EEEE 1011011 10I DDDDDDDDD SSSSSSSSS
 *
 * DJF     D,{#}S
 *
 *
 */
void P2Doc::doc_DJF(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011011 10I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_DJF);
    op->set_brief("Decrement D and jump to S** if result is $FFFF_FFFF.");
    op->set_instr("DJF     D,{#}S");
}

/**
 * @brief Decrement D and jump to S** if result is not $FFFF_FFFF.
 *
 * EEEE 1011011 11I DDDDDDDDD SSSSSSSSS
 *
 * DJNF    D,{#}S
 *
 *
 */
void P2Doc::doc_DJNF(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011011 11I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_DJNF);
    op->set_brief("Decrement D and jump to S** if result is not $FFFF_FFFF.");
    op->set_instr("DJNF    D,{#}S");
}

/**
 * @brief Increment D and jump to S** if result is zero.
 *
 * EEEE 1011100 00I DDDDDDDDD SSSSSSSSS
 *
 * IJZ     D,{#}S
 *
 *
 */
void P2Doc::doc_IJZ(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011100 00I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_IJZ);
    op->set_brief("Increment D and jump to S** if result is zero.");
    op->set_instr("IJZ     D,{#}S");
}

/**
 * @brief Increment D and jump to S** if result is not zero.
 *
 * EEEE 1011100 01I DDDDDDDDD SSSSSSSSS
 *
 * IJNZ    D,{#}S
 *
 *
 */
void P2Doc::doc_IJNZ(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011100 01I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_IJNZ);
    op->set_brief("Increment D and jump to S** if result is not zero.");
    op->set_instr("IJNZ    D,{#}S");
}

/**
 * @brief Test D and jump to S** if D is zero.
 *
 * EEEE 1011100 10I DDDDDDDDD SSSSSSSSS
 *
 * TJZ     D,{#}S
 *
 *
 */
void P2Doc::doc_TJZ(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011100 10I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_TJZ);
    op->set_brief("Test D and jump to S** if D is zero.");
    op->set_instr("TJZ     D,{#}S");
}

/**
 * @brief Test D and jump to S** if D is not zero.
 *
 * EEEE 1011100 11I DDDDDDDDD SSSSSSSSS
 *
 * TJNZ    D,{#}S
 *
 *
 */
void P2Doc::doc_TJNZ(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011100 11I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_TJNZ);
    op->set_brief("Test D and jump to S** if D is not zero.");
    op->set_instr("TJNZ    D,{#}S");
}

/**
 * @brief Test D and jump to S** if D is full (D = $FFFF_FFFF).
 *
 * EEEE 1011101 00I DDDDDDDDD SSSSSSSSS
 *
 * TJF     D,{#}S
 *
 *
 */
void P2Doc::doc_TJF(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011101 00I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_TJF);
    op->set_brief("Test D and jump to S** if D is full (D = $FFFF_FFFF).");
    op->set_instr("TJF     D,{#}S");
}

/**
 * @brief Test D and jump to S** if D is not full (D != $FFFF_FFFF).
 *
 * EEEE 1011101 01I DDDDDDDDD SSSSSSSSS
 *
 * TJNF    D,{#}S
 *
 *
 */
void P2Doc::doc_TJNF(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011101 01I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_TJNF);
    op->set_brief("Test D and jump to S** if D is not full (D != $FFFF_FFFF).");
    op->set_instr("TJNF    D,{#}S");
}

/**
 * @brief Test D and jump to S** if D is signed (D[31] = 1).
 *
 * EEEE 1011101 10I DDDDDDDDD SSSSSSSSS
 *
 * TJS     D,{#}S
 *
 *
 */
void P2Doc::doc_TJS(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011101 10I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_TJS);
    op->set_brief("Test D and jump to S** if D is signed (D[31] = 1).");
    op->set_instr("TJS     D,{#}S");
}

/**
 * @brief Test D and jump to S** if D is not signed (D[31] = 0).
 *
 * EEEE 1011101 11I DDDDDDDDD SSSSSSSSS
 *
 * TJNS    D,{#}S
 *
 *
 */
void P2Doc::doc_TJNS(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011101 11I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_TJNS);
    op->set_brief("Test D and jump to S** if D is not signed (D[31] = 0).");
    op->set_instr("TJNS    D,{#}S");
}

/**
 * @brief Test D and jump to S** if D overflowed (D[31] != C, C = 'correct sign' from last addition/subtraction).
 *
 * EEEE 1011110 00I DDDDDDDDD SSSSSSSSS
 *
 * TJV     D,{#}S
 *
 *
 */
void P2Doc::doc_TJV(p2_INST9_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 00I DDDDDDDDD SSSSSSSSS");

    op->set_token(t_TJV);
    op->set_brief("Test D and jump to S** if D overflowed (D[31] != C, C = 'correct sign' from last addition/subtraction).");
    op->set_instr("TJV     D,{#}S");
}

/**
 * @brief Jump to S** if INT event flag is set.
 *
 * EEEE 1011110 01I 000000000 SSSSSSSSS
 *
 * JINT    {#}S
 *
 *
 */
void P2Doc::doc_JINT(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000000000 SSSSSSSSS");

    op->set_token(t_JINT);
    op->set_brief("Jump to S** if INT event flag is set.");
    op->set_instr("JINT    {#}S");
}

/**
 * @brief Jump to S** if CT1 event flag is set.
 *
 * EEEE 1011110 01I 000000001 SSSSSSSSS
 *
 * JCT1    {#}S
 *
 *
 */
void P2Doc::doc_JCT1(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000000001 SSSSSSSSS");

    op->set_token(t_JCT1);
    op->set_brief("Jump to S** if CT1 event flag is set.");
    op->set_instr("JCT1    {#}S");
}

/**
 * @brief Jump to S** if CT2 event flag is set.
 *
 * EEEE 1011110 01I 000000010 SSSSSSSSS
 *
 * JCT2    {#}S
 *
 *
 */
void P2Doc::doc_JCT2(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000000010 SSSSSSSSS");

    op->set_token(t_JCT2);
    op->set_brief("Jump to S** if CT2 event flag is set.");
    op->set_instr("JCT2    {#}S");
}

/**
 * @brief Jump to S** if CT3 event flag is set.
 *
 * EEEE 1011110 01I 000000011 SSSSSSSSS
 *
 * JCT3    {#}S
 *
 *
 */
void P2Doc::doc_JCT3(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000000011 SSSSSSSSS");

    op->set_token(t_JCT3);
    op->set_brief("Jump to S** if CT3 event flag is set.");
    op->set_instr("JCT3    {#}S");
}

/**
 * @brief Jump to S** if SE1 event flag is set.
 *
 * EEEE 1011110 01I 000000100 SSSSSSSSS
 *
 * JSE1    {#}S
 *
 *
 */
void P2Doc::doc_JSE1(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000000100 SSSSSSSSS");

    op->set_token(t_JSE1);
    op->set_brief("Jump to S** if SE1 event flag is set.");
    op->set_instr("JSE1    {#}S");
}

/**
 * @brief Jump to S** if SE2 event flag is set.
 *
 * EEEE 1011110 01I 000000101 SSSSSSSSS
 *
 * JSE2    {#}S
 *
 *
 */
void P2Doc::doc_JSE2(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000000101 SSSSSSSSS");

    op->set_token(t_JSE2);
    op->set_brief("Jump to S** if SE2 event flag is set.");
    op->set_instr("JSE2    {#}S");
}

/**
 * @brief Jump to S** if SE3 event flag is set.
 *
 * EEEE 1011110 01I 000000110 SSSSSSSSS
 *
 * JSE3    {#}S
 *
 *
 */
void P2Doc::doc_JSE3(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000000110 SSSSSSSSS");

    op->set_token(t_JSE3);
    op->set_brief("Jump to S** if SE3 event flag is set.");
    op->set_instr("JSE3    {#}S");
}

/**
 * @brief Jump to S** if SE4 event flag is set.
 *
 * EEEE 1011110 01I 000000111 SSSSSSSSS
 *
 * JSE4    {#}S
 *
 *
 */
void P2Doc::doc_JSE4(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000000111 SSSSSSSSS");

    op->set_token(t_JSE4);
    op->set_brief("Jump to S** if SE4 event flag is set.");
    op->set_instr("JSE4    {#}S");
}

/**
 * @brief Jump to S** if PAT event flag is set.
 *
 * EEEE 1011110 01I 000001000 SSSSSSSSS
 *
 * JPAT    {#}S
 *
 *
 */
void P2Doc::doc_JPAT(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000001000 SSSSSSSSS");

    op->set_token(t_JPAT);
    op->set_brief("Jump to S** if PAT event flag is set.");
    op->set_instr("JPAT    {#}S");
}

/**
 * @brief Jump to S** if FBW event flag is set.
 *
 * EEEE 1011110 01I 000001001 SSSSSSSSS
 *
 * JFBW    {#}S
 *
 *
 */
void P2Doc::doc_JFBW(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000001001 SSSSSSSSS");

    op->set_token(t_JFBW);
    op->set_brief("Jump to S** if FBW event flag is set.");
    op->set_instr("JFBW    {#}S");
}

/**
 * @brief Jump to S** if XMT event flag is set.
 *
 * EEEE 1011110 01I 000001010 SSSSSSSSS
 *
 * JXMT    {#}S
 *
 *
 */
void P2Doc::doc_JXMT(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000001010 SSSSSSSSS");

    op->set_token(t_JXMT);
    op->set_brief("Jump to S** if XMT event flag is set.");
    op->set_instr("JXMT    {#}S");
}

/**
 * @brief Jump to S** if XFI event flag is set.
 *
 * EEEE 1011110 01I 000001011 SSSSSSSSS
 *
 * JXFI    {#}S
 *
 *
 */
void P2Doc::doc_JXFI(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000001011 SSSSSSSSS");

    op->set_token(t_JXFI);
    op->set_brief("Jump to S** if XFI event flag is set.");
    op->set_instr("JXFI    {#}S");
}

/**
 * @brief Jump to S** if XRO event flag is set.
 *
 * EEEE 1011110 01I 000001100 SSSSSSSSS
 *
 * JXRO    {#}S
 *
 *
 */
void P2Doc::doc_JXRO(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000001100 SSSSSSSSS");

    op->set_token(t_JXRO);
    op->set_brief("Jump to S** if XRO event flag is set.");
    op->set_instr("JXRO    {#}S");
}

/**
 * @brief Jump to S** if XRL event flag is set.
 *
 * EEEE 1011110 01I 000001101 SSSSSSSSS
 *
 * JXRL    {#}S
 *
 *
 */
void P2Doc::doc_JXRL(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000001101 SSSSSSSSS");

    op->set_token(t_JXRL);
    op->set_brief("Jump to S** if XRL event flag is set.");
    op->set_instr("JXRL    {#}S");
}

/**
 * @brief Jump to S** if ATN event flag is set.
 *
 * EEEE 1011110 01I 000001110 SSSSSSSSS
 *
 * JATN    {#}S
 *
 *
 */
void P2Doc::doc_JATN(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000001110 SSSSSSSSS");

    op->set_token(t_JATN);
    op->set_brief("Jump to S** if ATN event flag is set.");
    op->set_instr("JATN    {#}S");
}

/**
 * @brief Jump to S** if QMT event flag is set.
 *
 * EEEE 1011110 01I 000001111 SSSSSSSSS
 *
 * JQMT    {#}S
 *
 *
 */
void P2Doc::doc_JQMT(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000001111 SSSSSSSSS");

    op->set_token(t_JQMT);
    op->set_brief("Jump to S** if QMT event flag is set.");
    op->set_instr("JQMT    {#}S");
}

/**
 * @brief Jump to S** if INT event flag is clear.
 *
 * EEEE 1011110 01I 000010000 SSSSSSSSS
 *
 * JNINT   {#}S
 *
 *
 */
void P2Doc::doc_JNINT(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000010000 SSSSSSSSS");

    op->set_token(t_JNINT);
    op->set_brief("Jump to S** if INT event flag is clear.");
    op->set_instr("JNINT   {#}S");
}

/**
 * @brief Jump to S** if CT1 event flag is clear.
 *
 * EEEE 1011110 01I 000010001 SSSSSSSSS
 *
 * JNCT1   {#}S
 *
 *
 */
void P2Doc::doc_JNCT1(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000010001 SSSSSSSSS");

    op->set_token(t_JNCT1);
    op->set_brief("Jump to S** if CT1 event flag is clear.");
    op->set_instr("JNCT1   {#}S");
}

/**
 * @brief Jump to S** if CT2 event flag is clear.
 *
 * EEEE 1011110 01I 000010010 SSSSSSSSS
 *
 * JNCT2   {#}S
 *
 *
 */
void P2Doc::doc_JNCT2(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000010010 SSSSSSSSS");

    op->set_token(t_JNCT2);
    op->set_brief("Jump to S** if CT2 event flag is clear.");
    op->set_instr("JNCT2   {#}S");
}

/**
 * @brief Jump to S** if CT3 event flag is clear.
 *
 * EEEE 1011110 01I 000010011 SSSSSSSSS
 *
 * JNCT3   {#}S
 *
 *
 */
void P2Doc::doc_JNCT3(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000010011 SSSSSSSSS");

    op->set_token(t_JNCT3);
    op->set_brief("Jump to S** if CT3 event flag is clear.");
    op->set_instr("JNCT3   {#}S");
}

/**
 * @brief Jump to S** if SE1 event flag is clear.
 *
 * EEEE 1011110 01I 000010100 SSSSSSSSS
 *
 * JNSE1   {#}S
 *
 *
 */
void P2Doc::doc_JNSE1(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000010100 SSSSSSSSS");

    op->set_token(t_JNSE1);
    op->set_brief("Jump to S** if SE1 event flag is clear.");
    op->set_instr("JNSE1   {#}S");
}

/**
 * @brief Jump to S** if SE2 event flag is clear.
 *
 * EEEE 1011110 01I 000010101 SSSSSSSSS
 *
 * JNSE2   {#}S
 *
 *
 */
void P2Doc::doc_JNSE2(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000010101 SSSSSSSSS");

    op->set_token(t_JNSE2);
    op->set_brief("Jump to S** if SE2 event flag is clear.");
    op->set_instr("JNSE2   {#}S");
}

/**
 * @brief Jump to S** if SE3 event flag is clear.
 *
 * EEEE 1011110 01I 000010110 SSSSSSSSS
 *
 * JNSE3   {#}S
 *
 *
 */
void P2Doc::doc_JNSE3(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000010110 SSSSSSSSS");

    op->set_token(t_JNSE3);
    op->set_brief("Jump to S** if SE3 event flag is clear.");
    op->set_instr("JNSE3   {#}S");
}

/**
 * @brief Jump to S** if SE4 event flag is clear.
 *
 * EEEE 1011110 01I 000010111 SSSSSSSSS
 *
 * JNSE4   {#}S
 *
 *
 */
void P2Doc::doc_JNSE4(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000010111 SSSSSSSSS");

    op->set_token(t_JNSE4);
    op->set_brief("Jump to S** if SE4 event flag is clear.");
    op->set_instr("JNSE4   {#}S");
}

/**
 * @brief Jump to S** if PAT event flag is clear.
 *
 * EEEE 1011110 01I 000011000 SSSSSSSSS
 *
 * JNPAT   {#}S
 *
 *
 */
void P2Doc::doc_JNPAT(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000011000 SSSSSSSSS");

    op->set_token(t_JNPAT);
    op->set_brief("Jump to S** if PAT event flag is clear.");
    op->set_instr("JNPAT   {#}S");
}

/**
 * @brief Jump to S** if FBW event flag is clear.
 *
 * EEEE 1011110 01I 000011001 SSSSSSSSS
 *
 * JNFBW   {#}S
 *
 *
 */
void P2Doc::doc_JNFBW(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000011001 SSSSSSSSS");

    op->set_token(t_JNFBW);
    op->set_brief("Jump to S** if FBW event flag is clear.");
    op->set_instr("JNFBW   {#}S");
}

/**
 * @brief Jump to S** if XMT event flag is clear.
 *
 * EEEE 1011110 01I 000011010 SSSSSSSSS
 *
 * JNXMT   {#}S
 *
 *
 */
void P2Doc::doc_JNXMT(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000011010 SSSSSSSSS");

    op->set_token(t_JNXMT);
    op->set_brief("Jump to S** if XMT event flag is clear.");
    op->set_instr("JNXMT   {#}S");
}

/**
 * @brief Jump to S** if XFI event flag is clear.
 *
 * EEEE 1011110 01I 000011011 SSSSSSSSS
 *
 * JNXFI   {#}S
 *
 *
 */
void P2Doc::doc_JNXFI(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000011011 SSSSSSSSS");

    op->set_token(t_JNXFI);
    op->set_brief("Jump to S** if XFI event flag is clear.");
    op->set_instr("JNXFI   {#}S");
}

/**
 * @brief Jump to S** if XRO event flag is clear.
 *
 * EEEE 1011110 01I 000011100 SSSSSSSSS
 *
 * JNXRO   {#}S
 *
 *
 */
void P2Doc::doc_JNXRO(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000011100 SSSSSSSSS");

    op->set_token(t_JNXRO);
    op->set_brief("Jump to S** if XRO event flag is clear.");
    op->set_instr("JNXRO   {#}S");
}

/**
 * @brief Jump to S** if XRL event flag is clear.
 *
 * EEEE 1011110 01I 000011101 SSSSSSSSS
 *
 * JNXRL   {#}S
 *
 *
 */
void P2Doc::doc_JNXRL(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000011101 SSSSSSSSS");

    op->set_token(t_JNXRL);
    op->set_brief("Jump to S** if XRL event flag is clear.");
    op->set_instr("JNXRL   {#}S");
}

/**
 * @brief Jump to S** if ATN event flag is clear.
 *
 * EEEE 1011110 01I 000011110 SSSSSSSSS
 *
 * JNATN   {#}S
 *
 *
 */
void P2Doc::doc_JNATN(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000011110 SSSSSSSSS");

    op->set_token(t_JNATN);
    op->set_brief("Jump to S** if ATN event flag is clear.");
    op->set_instr("JNATN   {#}S");
}

/**
 * @brief Jump to S** if QMT event flag is clear.
 *
 * EEEE 1011110 01I 000011111 SSSSSSSSS
 *
 * JNQMT   {#}S
 *
 *
 */
void P2Doc::doc_JNQMT(p2_OPDST_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 01I 000011111 SSSSSSSSS");

    op->set_token(t_JNQMT);
    op->set_brief("Jump to S** if QMT event flag is clear.");
    op->set_instr("JNQMT   {#}S");
}

/**
 * @brief <empty>.
 *
 * EEEE 1011110 1LI DDDDDDDDD SSSSSSSSS
 *
 * <empty> {#}D,{#}S
 *
 *
 */
void P2Doc::doc_1011110_1(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011110 1LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_empty);
    op->set_brief("Undefined instruction <empty> %1011110_1.");
    op->set_instr("<empty> {#}D,{#}S");
}

/**
 * @brief <empty>.
 *
 * EEEE 1011111 0LI DDDDDDDDD SSSSSSSSS
 *
 * <empty> {#}D,{#}S
 *
 *
 */
void P2Doc::doc_1011111_0(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011111 0LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_empty);
    op->set_brief("Undefined instruction <empty> %1011111_0.");
    op->set_instr("<empty> {#}D,{#}S");
}

/**
 * @brief Set pin pattern for PAT event.
 *
 * EEEE 1011111 1LI DDDDDDDDD SSSSSSSSS
 *
 * SETPAT  {#}D,{#}S
 *
 * C selects INA/INB, Z selects =/!=, D provides mask value, S provides match value.
 *
 */
void P2Doc::doc_SETPAT(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1011111 1LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_SETPAT);
    op->set_instr("SETPAT  {#}D,{#}S");
    op->set_brief("Set pin pattern for PAT event.");
    op->add_descr("C selects INA/INB, Z selects =/!=, D provides mask value, S provides match value.");
}

/**
 * @brief Write D to mode register of smart pin S[5:0], acknowledge smart pin.
 *
 * EEEE 1100000 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRPIN   {#}D,{#}S
 *
 *
 */
void P2Doc::doc_WRPIN(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1100000 0LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_WRPIN);
    op->set_brief("Write D to mode register of smart pin S[5:0], acknowledge smart pin.");
    op->set_instr("WRPIN   {#}D,{#}S");
}

/**
 * @brief Acknowledge smart pin S[5:0].
 *
 * EEEE 1100000 01I 000000001 SSSSSSSSS
 *
 * AKPIN   {#}S
 *
 *
 */
void P2Doc::doc_AKPIN(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1100000 01I 000000001 SSSSSSSSS", match_D);

    op->set_token(t_AKPIN);
    op->set_brief("Acknowledge smart pin S[5:0].");
    op->set_instr("AKPIN   {#}S");
}

/**
 * @brief Write D to parameter "X" of smart pin S[5:0], acknowledge smart pin.
 *
 * EEEE 1100000 1LI DDDDDDDDD SSSSSSSSS
 *
 * WXPIN   {#}D,{#}S
 *
 *
 */
void P2Doc::doc_WXPIN(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1100000 1LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_WXPIN);
    op->set_brief("Write D to parameter \"X\" of smart pin S[5:0], acknowledge smart pin.");
    op->set_instr("WXPIN   {#}D,{#}S");
}

/**
 * @brief Write D to parameter "Y" of smart pin S[5:0], acknowledge smart pin.
 *
 * EEEE 1100001 0LI DDDDDDDDD SSSSSSSSS
 *
 * WYPIN   {#}D,{#}S
 *
 *
 */
void P2Doc::doc_WYPIN(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1100001 0LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_WYPIN);
    op->set_brief("Write D to parameter \"Y\" of smart pin S[5:0], acknowledge smart pin.");
    op->set_instr("WYPIN   {#}D,{#}S");
}

/**
 * @brief Write D to LUT address S[8:0].
 *
 * EEEE 1100001 1LI DDDDDDDDD SSSSSSSSS
 *
 * WRLUT   {#}D,{#}S
 *
 *
 */
void P2Doc::doc_WRLUT(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1100001 1LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_WRLUT);
    op->set_brief("Write D to LUT address S[8:0].");
    op->set_instr("WRLUT   {#}D,{#}S");
}

/**
 * @brief Write byte in D[7:0] to hub address {#}S/PTRx.
 *
 * EEEE 1100010 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRBYTE  {#}D,{#}S/P
 *
 *
 */
void P2Doc::doc_WRBYTE(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1100010 0LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_WRBYTE);
    op->set_brief("Write byte in D[7:0] to hub address {#}S/PTRx.");
    op->set_instr("WRBYTE  {#}D,{#}S/P");
}

/**
 * @brief Write word in D[15:0] to hub address {#}S/PTRx.
 *
 * EEEE 1100010 1LI DDDDDDDDD SSSSSSSSS
 *
 * WRWORD  {#}D,{#}S/P
 *
 *
 */
void P2Doc::doc_WRWORD(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1100010 1LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_WRWORD);
    op->set_brief("Write word in D[15:0] to hub address {#}S/PTRx.");
    op->set_instr("WRWORD  {#}D,{#}S/P");
}

/**
 * @brief Write long in D[31:0] to hub address {#}S/PTRx.
 *
 * EEEE 1100011 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRLONG  {#}D,{#}S/P
 *
 * Prior SETQ/SETQ2 invokes cog/LUT block transfer.
 *
 */
void P2Doc::doc_WRLONG(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1100011 0LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_WRLONG);
    op->set_brief("Write long in D[31:0] to hub address {#}S/PTRx.");
    op->set_instr("WRLONG  {#}D,{#}S/P");
    op->add_descr("Prior SETQ/SETQ2 invokes cog/LUT block transfer.");
}

/**
 * @brief Write long in D[31:0] to hub address PTRA++.
 *
 * EEEE 1100011 0L1 DDDDDDDDD 101100001
 *
 * PUSHA   {#}D
 *
 *
 */
void P2Doc::doc_PUSHA(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1100011 0L1 DDDDDDDDD 101100001");

    op->set_token(t_PUSHA);
    op->set_brief("Write long in D[31:0] to hub address PTRA++.");
    op->set_instr("PUSHA   {#}D");
}

/**
 * @brief Write long in D[31:0] to hub address PTRB++.
 *
 * EEEE 1100011 0L1 DDDDDDDDD 111100001
 *
 * PUSHB   {#}D
 *
 *
 */
void P2Doc::doc_PUSHB(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1100011 0L1 DDDDDDDDD 111100001");

    op->set_token(t_PUSHB);
    op->set_brief("Write long in D[31:0] to hub address PTRB++.");
    op->set_instr("PUSHB   {#}D");
}

/**
 * @brief Begin new fast hub read via FIFO.
 *
 * EEEE 1100011 1LI DDDDDDDDD SSSSSSSSS
 *
 * RDFAST  {#}D,{#}S
 *
 * D[31] = no wait, D[13:0] = block size in 64-byte units (0 = max), S[19:0] = block start address.
 *
 */
void P2Doc::doc_RDFAST(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1100011 1LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_RDFAST);
    op->set_brief("Begin new fast hub read via FIFO.");
    op->set_instr("RDFAST  {#}D,{#}S");
    op->add_descr("D[31] = no wait, D[13:0] = block size in 64-byte units (0 = max), S[19:0] = block start address.");
}

/**
 * @brief Begin new fast hub write via FIFO.
 *
 * EEEE 1100100 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRFAST  {#}D,{#}S
 *
 * D[31] = no wait, D[13:0] = block size in 64-byte units (0 = max), S[19:0] = block start address.
 *
 */
void P2Doc::doc_WRFAST(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1100100 0LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_WRFAST);
    op->set_brief("Begin new fast hub write via FIFO.");
    op->set_instr("WRFAST  {#}D,{#}S");
    op->add_descr("D[31] = no wait, D[13:0] = block size in 64-byte units (0 = max), S[19:0] = block start address.");
}

/**
 * @brief Set next block for when block wraps.
 *
 * EEEE 1100100 1LI DDDDDDDDD SSSSSSSSS
 *
 * FBLOCK  {#}D,{#}S
 *
 * D[13:0] = block size in 64-byte units (0 = max), S[19:0] = block start address.
 *
 */
void P2Doc::doc_FBLOCK(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1100100 1LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_FBLOCK);
    op->set_brief("Set next block for when block wraps.");
    op->set_instr("FBLOCK  {#}D,{#}S");
    op->add_descr("D[13:0] = block size in 64-byte units (0 = max), S[19:0] = block start address.");
}

/**
 * @brief Issue streamer command immediately, zeroing phase.
 *
 * EEEE 1100101 0LI DDDDDDDDD SSSSSSSSS
 *
 * XINIT   {#}D,{#}S
 *
 *
 */
void P2Doc::doc_XINIT(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1100101 0LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_XINIT);
    op->set_brief("Issue streamer command immediately, zeroing phase.");
    op->set_instr("XINIT   {#}D,{#}S");
}

/**
 * @brief Stop streamer immediately.
 *
 * EEEE 1100101 011 000000000 000000000
 *
 * XSTOP
 *
 *
 */
void P2Doc::doc_XSTOP(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1100101 011 000000000 000000000", match_D_S);

    op->set_token(t_XSTOP);
    op->set_brief("Stop streamer immediately.");
    op->set_instr("XSTOP");
}

/**
 * @brief Buffer new streamer command to be issued on final NCO rollover of current command, zeroing phase.
 *
 * EEEE 1100101 1LI DDDDDDDDD SSSSSSSSS
 *
 * XZERO   {#}D,{#}S
 *
 *
 */
void P2Doc::doc_XZERO(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1100101 1LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_XZERO);
    op->set_brief("Buffer new streamer command to be issued on final NCO rollover of current command, zeroing phase.");
    op->set_instr("XZERO   {#}D,{#}S");
}

/**
 * @brief Buffer new streamer command to be issued on final NCO rollover of current command, continuing phase.
 *
 * EEEE 1100110 0LI DDDDDDDDD SSSSSSSSS
 *
 * XCONT   {#}D,{#}S
 *
 *
 */
void P2Doc::doc_XCONT(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1100110 0LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_XCONT);
    op->set_brief("Buffer new streamer command to be issued on final NCO rollover of current command, continuing phase.");
    op->set_instr("XCONT   {#}D,{#}S");
}

/**
 * @brief Execute next D[8:0] instructions S times.
 *
 * EEEE 1100110 1LI DDDDDDDDD SSSSSSSSS
 *
 * REP     {#}D,{#}S
 *
 * If S = 0, repeat infinitely.
 * If D[8:0] = 0, nothing repeats.
 *
 */
void P2Doc::doc_REP(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1100110 1LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_REP);
    op->set_brief("Execute next D[8:0] instructions S times.");
    op->set_instr("REP     {#}D,{#}S");
    op->add_descr("If S = 0, repeat infinitely.");
    op->add_descr("If D[8:0] = 0, nothing repeats.");
}

/**
 * @brief Start cog selected by D.
 *
 * EEEE 1100111 CLI DDDDDDDDD SSSSSSSSS
 *
 * COGINIT {#}D,{#}S       {WC}
 *
 * S[19:0] sets hub startup address and PTRB of cog.
 * Prior SETQ sets PTRA of cog.
 *
 */
void P2Doc::doc_COGINIT(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1100111 CLI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_COGINIT);
    op->set_brief("Start cog selected by D.");
    op->set_instr("COGINIT {#}D,{#}S       {WC}");
    op->add_descr("S[19:0] sets hub startup address and PTRB of cog.");
    op->add_descr("Prior SETQ sets PTRA of cog.");
}

/**
 * @brief Begin CORDIC unsigned multiplication of D * S.
 *
 * EEEE 1101000 0LI DDDDDDDDD SSSSSSSSS
 *
 * QMUL    {#}D,{#}S
 *
 * GETQX/GETQY retrieves lower/upper product.
 *
 */
void P2Doc::doc_QMUL(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101000 0LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_QMUL);
    op->set_brief("Begin CORDIC unsigned multiplication of D * S.");
    op->set_instr("QMUL    {#}D,{#}S");
    op->add_descr("GETQX/GETQY retrieves lower/upper product.");
}

/**
 * @brief Begin CORDIC unsigned division of {SETQ value or 32'b0, D} / S.
 *
 * EEEE 1101000 1LI DDDDDDDDD SSSSSSSSS
 *
 * QDIV    {#}D,{#}S
 *
 * GETQX/GETQY retrieves quotient/remainder.
 *
 */
void P2Doc::doc_QDIV(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101000 1LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_QDIV);
    op->set_brief("Begin CORDIC unsigned division of {SETQ value or 32'b0, D} / S.");
    op->set_instr("QDIV    {#}D,{#}S");
    op->add_descr("GETQX/GETQY retrieves quotient/remainder.");
}

/**
 * @brief Begin CORDIC unsigned division of {D, SETQ value or 32'b0} / S.
 *
 * EEEE 1101001 0LI DDDDDDDDD SSSSSSSSS
 *
 * QFRAC   {#}D,{#}S
 *
 * GETQX/GETQY retrieves quotient/remainder.
 *
 */
void P2Doc::doc_QFRAC(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101001 0LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_QFRAC);
    op->set_brief("Begin CORDIC unsigned division of {D, SETQ value or 32'b0} / S.");
    op->set_instr("QFRAC   {#}D,{#}S");
    op->add_descr("GETQX/GETQY retrieves quotient/remainder.");
}

/**
 * @brief Begin CORDIC square root of {S, D}.
 *
 * EEEE 1101001 1LI DDDDDDDDD SSSSSSSSS
 *
 * QSQRT   {#}D,{#}S
 *
 * GETQX retrieves root.
 *
 */
void P2Doc::doc_QSQRT(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101001 1LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_QSQRT);
    op->set_brief("Begin CORDIC square root of {S, D}.");
    op->set_instr("QSQRT   {#}D,{#}S");
    op->add_descr("GETQX retrieves root.");
}

/**
 * @brief Begin CORDIC rotation of point (D, SETQ value or 32'b0) by angle S.
 *
 * EEEE 1101010 0LI DDDDDDDDD SSSSSSSSS
 *
 * QROTATE {#}D,{#}S
 *
 * GETQX/GETQY retrieves X/Y.
 *
 */
void P2Doc::doc_QROTATE(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101010 0LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_QROTATE);
    op->set_brief("Begin CORDIC rotation of point (D, SETQ value or 32'b0) by angle S.");
    op->set_instr("QROTATE {#}D,{#}S");
    op->add_descr("GETQX/GETQY retrieves X/Y.");
}

/**
 * @brief Begin CORDIC vectoring of point (D, S).
 *
 * EEEE 1101010 1LI DDDDDDDDD SSSSSSSSS
 *
 * QVECTOR {#}D,{#}S
 *
 * GETQX/GETQY retrieves length/angle.
 *
 */
void P2Doc::doc_QVECTOR(p2_INST8_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101010 1LI DDDDDDDDD SSSSSSSSS");

    op->set_token(t_QVECTOR);
    op->set_brief("Begin CORDIC vectoring of point (D, S).");
    op->set_instr("QVECTOR {#}D,{#}S");
    op->add_descr("GETQX/GETQY retrieves length/angle.");
}

/**
 * @brief Set hub configuration to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000000000
 *
 * HUBSET  {#}D
 *
 *
 */
void P2Doc::doc_HUBSET(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000000000");

    op->set_token(t_HUBSET);
    op->set_brief("Set hub configuration to D.");
    op->set_instr("HUBSET  {#}D");
}

/**
 * @brief If D is register and no WC, get cog ID (0 to 15) into D.
 *
 * EEEE 1101011 C0L DDDDDDDDD 000000001
 *
 * COGID   {#}D            {WC}
 *
 * If WC, check status of cog D[3:0], C = 1 if on.
 *
 */
void P2Doc::doc_COGID(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 C0L DDDDDDDDD 000000001");

    op->set_token(t_COGID);
    op->set_brief("If D is register and no WC, get cog ID (0 to 15) into D.");
    op->set_instr("COGID   {#}D            {WC}");
    op->add_descr("If WC, check status of cog D[3:0], C = 1 if on.");
}

/**
 * @brief Stop cog D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000000011
 *
 * COGSTOP {#}D
 *
 *
 */
void P2Doc::doc_COGSTOP(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000000011");

    op->set_token(t_COGSTOP);
    op->set_brief("Stop cog D[3:0].");
    op->set_instr("COGSTOP {#}D");
}

/**
 * @brief Request a LOCK.
 *
 * EEEE 1101011 C00 DDDDDDDDD 000000100
 *
 * LOCKNEW D               {WC}
 *
 * D will be written with the LOCK number (0 to 15).
 * C = 1 if no LOCK available.
 *
 */
void P2Doc::doc_LOCKNEW(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 C00 DDDDDDDDD 000000100");

    op->set_token(t_LOCKNEW);
    op->set_brief("Request a LOCK.");
    op->set_instr("LOCKNEW D               {WC}");
    op->add_descr("D will be written with the LOCK number (0 to 15).");
    op->add_descr("C = 1 if no LOCK available.");
}

/**
 * @brief Return LOCK D[3:0] for reallocation.
 *
 * EEEE 1101011 00L DDDDDDDDD 000000101
 *
 * LOCKRET {#}D
 *
 *
 */
void P2Doc::doc_LOCKRET(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000000101");

    op->set_token(t_LOCKRET);
    op->set_brief("Return LOCK D[3:0] for reallocation.");
    op->set_instr("LOCKRET {#}D");
}

/**
 * @brief Try to get LOCK D[3:0].
 *
 * EEEE 1101011 C0L DDDDDDDDD 000000110
 *
 * LOCKTRY {#}D            {WC}
 *
 * C = 1 if got LOCK.
 * LOCKREL releases LOCK.
 * LOCK is also released if owner cog stops or restarts.
 *
 */
void P2Doc::doc_LOCKTRY(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 C0L DDDDDDDDD 000000110");

    op->set_token(t_LOCKTRY);
    op->set_brief("Try to get LOCK D[3:0].");
    op->set_instr("LOCKTRY {#}D            {WC}");
    op->add_descr("C = 1 if got LOCK.");
    op->add_descr("LOCKREL releases LOCK.");
    op->add_descr("LOCK is also released if owner cog stops or restarts.");
}

/**
 * @brief Release LOCK D[3:0].
 *
 * EEEE 1101011 C0L DDDDDDDDD 000000111
 *
 * LOCKREL {#}D            {WC}
 *
 * If D is a register and WC, get current/last cog id of LOCK owner into D and LOCK status into C.
 *
 */
void P2Doc::doc_LOCKREL(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 C0L DDDDDDDDD 000000111");

    op->set_token(t_LOCKREL);
    op->set_brief("Release LOCK D[3:0].");
    op->set_instr("LOCKREL {#}D            {WC}");
    op->add_descr("If D is a register and WC, get current/last cog id of LOCK owner into D and LOCK status into C.");
}

/**
 * @brief Begin CORDIC number-to-logarithm conversion of D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000001110
 *
 * QLOG    {#}D
 *
 * GETQX retrieves log {5'whole_exponent, 27'fractional_exponent}.
 *
 */
void P2Doc::doc_QLOG(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000001110");

    op->set_token(t_QLOG);
    op->set_brief("Begin CORDIC number-to-logarithm conversion of D.");
    op->set_instr("QLOG    {#}D");
    op->add_descr("GETQX retrieves log {5'whole_exponent, 27'fractional_exponent}.");
}

/**
 * @brief Begin CORDIC logarithm-to-number conversion of D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000001111
 *
 * QEXP    {#}D
 *
 * GETQX retrieves number.
 *
 */
void P2Doc::doc_QEXP(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000001111");

    op->set_token(t_QEXP);
    op->set_brief("Begin CORDIC logarithm-to-number conversion of D.");
    op->set_instr("QEXP    {#}D");
    op->add_descr("GETQX retrieves number.");
}

/**
 * @brief Read zero-extended byte from FIFO into D. Used after RDFAST.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000010000
 *
 * RFBYTE  D        {WC/WZ/WCZ}
 *
 * C = MSB of byte.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_RFBYTE(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 DDDDDDDDD 000010000");

    op->set_token(t_RFBYTE);
    op->set_brief("Read zero-extended byte from FIFO into D. Used after RDFAST.");
    op->set_instr("RFBYTE  D        {WC/WZ/WCZ}");
    op->add_descr("C = MSB of byte.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Read zero-extended word from FIFO into D. Used after RDFAST.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000010001
 *
 * RFWORD  D        {WC/WZ/WCZ}
 *
 * C = MSB of word.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_RFWORD(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 DDDDDDDDD 000010001");

    op->set_token(t_RFWORD);
    op->set_brief("Read zero-extended word from FIFO into D. Used after RDFAST.");
    op->set_instr("RFWORD  D        {WC/WZ/WCZ}");
    op->add_descr("C = MSB of word.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Read long from FIFO into D. Used after RDFAST.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000010010
 *
 * RFLONG  D        {WC/WZ/WCZ}
 *
 * C = MSB of long.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_RFLONG(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 DDDDDDDDD 000010010");

    op->set_token(t_RFLONG);
    op->set_brief("Read long from FIFO into D. Used after RDFAST.");
    op->set_instr("RFLONG  D        {WC/WZ/WCZ}");
    op->add_descr("C = MSB of long.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Read zero-extended 1..4-byte value from FIFO into D. Used after RDFAST.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000010011
 *
 * RFVAR   D        {WC/WZ/WCZ}
 *
 * C = 0.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_RFVAR(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 DDDDDDDDD 000010011");

    op->set_token(t_RFVAR);
    op->set_brief("Read zero-extended 1..4-byte value from FIFO into D. Used after RDFAST.");
    op->set_instr("RFVAR   D        {WC/WZ/WCZ}");
    op->add_descr("C = 0.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Read sign-extended 1..4-byte value from FIFO into D. Used after RDFAST.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000010100
 *
 * RFVARS  D        {WC/WZ/WCZ}
 *
 * C = MSB of value.
 * Z = (result == 0).
 *
 */
void P2Doc::doc_RFVARS(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 DDDDDDDDD 000010100");

    op->set_token(t_RFVARS);
    op->set_brief("Read sign-extended 1..4-byte value from FIFO into D. Used after RDFAST.");
    op->set_instr("RFVARS  D        {WC/WZ/WCZ}");
    op->add_descr("C = MSB of value.");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Write byte in D[7:0] into FIFO. Used after WRFAST.
 *
 * EEEE 1101011 00L DDDDDDDDD 000010101
 *
 * WFBYTE  {#}D
 *
 *
 */
void P2Doc::doc_WFBYTE(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000010101");

    op->set_token(t_WFBYTE);
    op->set_brief("Write byte in D[7:0] into FIFO. Used after WRFAST.");
    op->set_instr("WFBYTE  {#}D");
}

/**
 * @brief Write word in D[15:0] into FIFO. Used after WRFAST.
 *
 * EEEE 1101011 00L DDDDDDDDD 000010110
 *
 * WFWORD  {#}D
 *
 *
 */
void P2Doc::doc_WFWORD(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000010110");

    op->set_token(t_WFWORD);
    op->set_brief("Write word in D[15:0] into FIFO. Used after WRFAST.");
    op->set_instr("WFWORD  {#}D");
}

/**
 * @brief Write long in D[31:0] into FIFO. Used after WRFAST.
 *
 * EEEE 1101011 00L DDDDDDDDD 000010111
 *
 * WFLONG  {#}D
 *
 *
 */
void P2Doc::doc_WFLONG(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000010111");

    op->set_token(t_WFLONG);
    op->set_brief("Write long in D[31:0] into FIFO. Used after WRFAST.");
    op->set_instr("WFLONG  {#}D");
}

/**
 * @brief Retrieve CORDIC result X into D.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000011000
 *
 * GETQX   D        {WC/WZ/WCZ}
 *
 * Waits, in case result not ready.
 * C = X[31].
 * Z = (result == 0).
 *
 */
void P2Doc::doc_GETQX(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 DDDDDDDDD 000011000");

    op->set_token(t_GETQX);
    op->set_brief("Retrieve CORDIC result X into D.");
    op->set_instr("GETQX   D        {WC/WZ/WCZ}");
    op->add_descr("Waits, in case result not ready.");
    op->add_descr("C = X[31].");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Retrieve CORDIC result Y into D.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000011001
 *
 * GETQY   D        {WC/WZ/WCZ}
 *
 * Waits, in case result no ready.
 * C = Y[31].
 * Z = (result == 0).
 *
 */
void P2Doc::doc_GETQY(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 DDDDDDDDD 000011001");

    op->set_token(t_GETQY);
    op->set_brief("Retrieve CORDIC result Y into D.");
    op->set_instr("GETQY   D        {WC/WZ/WCZ}");
    op->add_descr("Waits, in case result no ready.");
    op->add_descr("C = Y[31].");
    op->add_descr("Z = (result == 0).");
}

/**
 * @brief Get CT into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 000011010
 *
 * GETCT   D
 *
 * CT is the free-running 32-bit system counter that increments on every clock.
 *
 */
void P2Doc::doc_GETCT(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 DDDDDDDDD 000011010");

    op->set_token(t_GETCT);
    op->set_brief("Get CT into D.");
    op->set_instr("GETCT   D");
    op->add_descr("CT is the free-running 32-bit system counter that increments on every clock.");
}

/**
 * @brief Get RND into D/C/Z.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000011011
 *
 * GETRND  D        {WC/WZ/WCZ}
 *
 * RND is the PRNG that updates on every clock.
 * D = RND[31:0], C = RND[31], Z = RND[30], unique per cog.
 *
 */
void P2Doc::doc_GETRND(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 DDDDDDDDD 000011011");

    op->set_token(t_GETRND);
    op->set_brief("Get RND into D/C/Z.");
    op->set_instr("GETRND  D        {WC/WZ/WCZ}");
    op->add_descr("RND is the PRNG that updates on every clock.");
    op->add_descr("D = RND[31:0], C = RND[31], Z = RND[30], unique per cog.");
}

/**
 * @brief Get RND into C/Z.
 *
 * EEEE 1101011 CZ1 000000000 000011011
 *
 * GETRND            WC/WZ/WCZ
 *
 * C = RND[31], Z = RND[30], unique per cog.
 *
 */
void P2Doc::doc_GETRND_CZ(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ1 000000000 000011011", match_D);

    op->set_token(t_GETRND);
    op->set_brief("Get RND into C/Z.");
    op->set_instr("GETRND            WC/WZ/WCZ");
    op->add_descr("C = RND[31], Z = RND[30], unique per cog.");
}

/**
 * @brief DAC3 = D[31:24], DAC2 = D[23:16], DAC1 = D[15:8], DAC0 = D[7:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000011100
 *
 * SETDACS {#}D
 *
 *
 */
void P2Doc::doc_SETDACS(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000011100");

    op->set_token(t_SETDACS);
    op->set_brief("DAC3 = D[31:24], DAC2 = D[23:16], DAC1 = D[15:8], DAC0 = D[7:0].");
    op->set_instr("SETDACS {#}D");
}

/**
 * @brief Set streamer NCO frequency to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000011101
 *
 * SETXFRQ {#}D
 *
 *
 */
void P2Doc::doc_SETXFRQ(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000011101");

    op->set_token(t_SETXFRQ);
    op->set_brief("Set streamer NCO frequency to D.");
    op->set_instr("SETXFRQ {#}D");
}

/**
 * @brief Get the streamer's Goertzel X accumulator into D and the Y accumulator into the next instruction's S, clear accumulators.
 *
 * EEEE 1101011 000 DDDDDDDDD 000011110
 *
 * GETXACC D
 *
 *
 */
void P2Doc::doc_GETXACC(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 DDDDDDDDD 000011110");

    op->set_token(t_GETXACC);
    op->set_brief("Get the streamer's Goertzel X accumulator into D and the Y accumulator into the next instruction's S, clear accumulators.");
    op->set_instr("GETXACC D");
}

/**
 * @brief Wait 2 + D clocks if no WC/WZ/WCZ.
 *
 * EEEE 1101011 00L DDDDDDDDD 000011111
 *
 * WAITX   {#}D     {WC/WZ/WCZ}
 *
 * If WC/WZ/WCZ, wait 2 + (D & RND) clocks.
 * C/Z = 0.
 *
 */
void P2Doc::doc_WAITX(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000011111");

    op->set_token(t_WAITX);
    op->set_brief("Wait 2 + D clocks if no WC/WZ/WCZ.");
    op->set_instr("WAITX   {#}D     {WC/WZ/WCZ}");
    op->add_descr("If WC/WZ/WCZ, wait 2 + (D & RND) clocks.");
    op->add_descr("C/Z = 0.");
}

/**
 * @brief Set SE1 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100000
 *
 * SETSE1  {#}D
 *
 *
 */
void P2Doc::doc_SETSE1(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000100000");

    op->set_token(t_SETSE1);
    op->set_brief("Set SE1 event configuration to D[8:0].");
    op->set_instr("SETSE1  {#}D");
}

/**
 * @brief Set SE2 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100001
 *
 * SETSE2  {#}D
 *
 *
 */
void P2Doc::doc_SETSE2(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000100001");

    op->set_token(t_SETSE2);
    op->set_brief("Set SE2 event configuration to D[8:0].");
    op->set_instr("SETSE2  {#}D");
}

/**
 * @brief Set SE3 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100010
 *
 * SETSE3  {#}D
 *
 *
 */
void P2Doc::doc_SETSE3(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000100010");

    op->set_token(t_SETSE3);
    op->set_brief("Set SE3 event configuration to D[8:0].");
    op->set_instr("SETSE3  {#}D");
}

/**
 * @brief Set SE4 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100011
 *
 * SETSE4  {#}D
 *
 *
 */
void P2Doc::doc_SETSE4(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000100011");

    op->set_token(t_SETSE4);
    op->set_brief("Set SE4 event configuration to D[8:0].");
    op->set_instr("SETSE4  {#}D");
}

/**
 * @brief Get INT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000000 000100100
 *
 * POLLINT          {WC/WZ/WCZ}
 *
 *
 */
void P2Doc::doc_POLLINT(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000000000 000100100");

    op->set_token(t_POLLINT);
    op->set_brief("Get INT event flag into C/Z, then clear it.");
    op->set_instr("POLLINT          {WC/WZ/WCZ}");
}

/**
 * @brief Get CT1 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000001 000100100
 *
 * POLLCT1          {WC/WZ/WCZ}
 *
 *
 */
void P2Doc::doc_POLLCT1(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000000001 000100100");

    op->set_token(t_POLLCT1);
    op->set_brief("Get CT1 event flag into C/Z, then clear it.");
    op->set_instr("POLLCT1          {WC/WZ/WCZ}");
}

/**
 * @brief Get CT2 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000010 000100100
 *
 * POLLCT2          {WC/WZ/WCZ}
 *
 *
 */
void P2Doc::doc_POLLCT2(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000000010 000100100");

    op->set_token(t_POLLCT2);
    op->set_brief("Get CT2 event flag into C/Z, then clear it.");
    op->set_instr("POLLCT2          {WC/WZ/WCZ}");
}

/**
 * @brief Get CT3 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000011 000100100
 *
 * POLLCT3          {WC/WZ/WCZ}
 *
 *
 */
void P2Doc::doc_POLLCT3(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000000011 000100100");

    op->set_token(t_POLLCT3);
    op->set_brief("Get CT3 event flag into C/Z, then clear it.");
    op->set_instr("POLLCT3          {WC/WZ/WCZ}");
}

/**
 * @brief Get SE1 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000100 000100100
 *
 * POLLSE1          {WC/WZ/WCZ}
 *
 *
 */
void P2Doc::doc_POLLSE1(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000000100 000100100");

    op->set_token(t_POLLSE1);
    op->set_brief("Get SE1 event flag into C/Z, then clear it.");
    op->set_instr("POLLSE1          {WC/WZ/WCZ}");
}

/**
 * @brief Get SE2 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000101 000100100
 *
 * POLLSE2          {WC/WZ/WCZ}
 *
 *
 */
void P2Doc::doc_POLLSE2(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000000101 000100100");

    op->set_token(t_POLLSE2);
    op->set_brief("Get SE2 event flag into C/Z, then clear it.");
    op->set_instr("POLLSE2          {WC/WZ/WCZ}");
}

/**
 * @brief Get SE3 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000110 000100100
 *
 * POLLSE3          {WC/WZ/WCZ}
 *
 *
 */
void P2Doc::doc_POLLSE3(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000000110 000100100");

    op->set_token(t_POLLSE3);
    op->set_brief("Get SE3 event flag into C/Z, then clear it.");
    op->set_instr("POLLSE3          {WC/WZ/WCZ}");
}

/**
 * @brief Get SE4 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000111 000100100
 *
 * POLLSE4          {WC/WZ/WCZ}
 *
 *
 */
void P2Doc::doc_POLLSE4(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000000111 000100100");

    op->set_token(t_POLLSE4);
    op->set_brief("Get SE4 event flag into C/Z, then clear it.");
    op->set_instr("POLLSE4          {WC/WZ/WCZ}");
}

/**
 * @brief Get PAT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001000 000100100
 *
 * POLLPAT          {WC/WZ/WCZ}
 *
 *
 */
void P2Doc::doc_POLLPAT(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000001000 000100100");

    op->set_token(t_POLLPAT);
    op->set_brief("Get PAT event flag into C/Z, then clear it.");
    op->set_instr("POLLPAT          {WC/WZ/WCZ}");
}

/**
 * @brief Get FBW event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001001 000100100
 *
 * POLLFBW          {WC/WZ/WCZ}
 *
 *
 */
void P2Doc::doc_POLLFBW(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000001001 000100100");

    op->set_token(t_POLLFBW);
    op->set_brief("Get FBW event flag into C/Z, then clear it.");
    op->set_instr("POLLFBW          {WC/WZ/WCZ}");
}

/**
 * @brief Get XMT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001010 000100100
 *
 * POLLXMT          {WC/WZ/WCZ}
 *
 *
 */
void P2Doc::doc_POLLXMT(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000001010 000100100");

    op->set_token(t_POLLXMT);
    op->set_brief("Get XMT event flag into C/Z, then clear it.");
    op->set_instr("POLLXMT          {WC/WZ/WCZ}");
}

/**
 * @brief Get XFI event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001011 000100100
 *
 * POLLXFI          {WC/WZ/WCZ}
 *
 *
 */
void P2Doc::doc_POLLXFI(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000001011 000100100");

    op->set_token(t_POLLXFI);
    op->set_brief("Get XFI event flag into C/Z, then clear it.");
    op->set_instr("POLLXFI          {WC/WZ/WCZ}");
}

/**
 * @brief Get XRO event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001100 000100100
 *
 * POLLXRO          {WC/WZ/WCZ}
 *
 *
 */
void P2Doc::doc_POLLXRO(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000001100 000100100");

    op->set_token(t_POLLXRO);
    op->set_brief("Get XRO event flag into C/Z, then clear it.");
    op->set_instr("POLLXRO          {WC/WZ/WCZ}");
}

/**
 * @brief Get XRL event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001101 000100100
 *
 * POLLXRL          {WC/WZ/WCZ}
 *
 *
 */
void P2Doc::doc_POLLXRL(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000001101 000100100");

    op->set_token(t_POLLXRL);
    op->set_brief("Get XRL event flag into C/Z, then clear it.");
    op->set_instr("POLLXRL          {WC/WZ/WCZ}");
}

/**
 * @brief Get ATN event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001110 000100100
 *
 * POLLATN          {WC/WZ/WCZ}
 *
 *
 */
void P2Doc::doc_POLLATN(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000001110 000100100");

    op->set_token(t_POLLATN);
    op->set_brief("Get ATN event flag into C/Z, then clear it.");
    op->set_instr("POLLATN          {WC/WZ/WCZ}");
}

/**
 * @brief Get QMT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001111 000100100
 *
 * POLLQMT          {WC/WZ/WCZ}
 *
 *
 */
void P2Doc::doc_POLLQMT(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000001111 000100100");

    op->set_token(t_POLLQMT);
    op->set_brief("Get QMT event flag into C/Z, then clear it.");
    op->set_instr("POLLQMT          {WC/WZ/WCZ}");
}

/**
 * @brief Wait for INT event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000010000 000100100
 *
 * WAITINT          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 */
void P2Doc::doc_WAITINT(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000010000 000100100");

    op->set_token(t_WAITINT);
    op->set_brief("Wait for INT event flag, then clear it.");
    op->set_instr("WAITINT          {WC/WZ/WCZ}");
    op->add_descr("Prior SETQ sets optional CT timeout value.");
    op->add_descr("C/Z = timeout.");
}

/**
 * @brief Wait for CT1 event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000010001 000100100
 *
 * WAITCT1          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 */
void P2Doc::doc_WAITCT1(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000010001 000100100");

    op->set_token(t_WAITCT1);
    op->set_brief("Wait for CT1 event flag, then clear it.");
    op->set_instr("WAITCT1          {WC/WZ/WCZ}");
    op->add_descr("Prior SETQ sets optional CT timeout value.");
    op->add_descr("C/Z = timeout.");
}

/**
 * @brief Wait for CT2 event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000010010 000100100
 *
 * WAITCT2          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 */
void P2Doc::doc_WAITCT2(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000010010 000100100");

    op->set_token(t_WAITCT2);
    op->set_brief("Wait for CT2 event flag, then clear it.");
    op->set_instr("WAITCT2          {WC/WZ/WCZ}");
    op->add_descr("Prior SETQ sets optional CT timeout value.");
    op->add_descr("C/Z = timeout.");
}

/**
 * @brief Wait for CT3 event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000010011 000100100
 *
 * WAITCT3          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 */
void P2Doc::doc_WAITCT3(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000010011 000100100");

    op->set_token(t_WAITCT3);
    op->set_brief("Wait for CT3 event flag, then clear it.");
    op->set_instr("WAITCT3          {WC/WZ/WCZ}");
    op->add_descr("Prior SETQ sets optional CT timeout value.");
    op->add_descr("C/Z = timeout.");
}

/**
 * @brief Wait for SE1 event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000010100 000100100
 *
 * WAITSE1          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 */
void P2Doc::doc_WAITSE1(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000010100 000100100");

    op->set_token(t_WAITSE1);
    op->set_brief("Wait for SE1 event flag, then clear it.");
    op->set_instr("WAITSE1          {WC/WZ/WCZ}");
    op->add_descr("Prior SETQ sets optional CT timeout value.");
    op->add_descr("C/Z = timeout.");
}

/**
 * @brief Wait for SE2 event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000010101 000100100
 *
 * WAITSE2          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 */
void P2Doc::doc_WAITSE2(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000010101 000100100");

    op->set_token(t_WAITSE2);
    op->set_brief("Wait for SE2 event flag, then clear it.");
    op->set_instr("WAITSE2          {WC/WZ/WCZ}");
    op->add_descr("Prior SETQ sets optional CT timeout value.");
    op->add_descr("C/Z = timeout.");
}

/**
 * @brief Wait for SE3 event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000010110 000100100
 *
 * WAITSE3          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 */
void P2Doc::doc_WAITSE3(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000010110 000100100");

    op->set_token(t_WAITSE3);
    op->set_brief("Wait for SE3 event flag, then clear it.");
    op->set_instr("WAITSE3          {WC/WZ/WCZ}");
    op->add_descr("Prior SETQ sets optional CT timeout value.");
    op->add_descr("C/Z = timeout.");
}

/**
 * @brief Wait for SE4 event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000010111 000100100
 *
 * WAITSE4          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 */
void P2Doc::doc_WAITSE4(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000010111 000100100");

    op->set_token(t_WAITSE4);
    op->set_brief("Wait for SE4 event flag, then clear it.");
    op->set_instr("WAITSE4          {WC/WZ/WCZ}");
    op->add_descr("Prior SETQ sets optional CT timeout value.");
    op->add_descr("C/Z = timeout.");
}

/**
 * @brief Wait for PAT event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000011000 000100100
 *
 * WAITPAT          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 */
void P2Doc::doc_WAITPAT(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000011000 000100100");

    op->set_token(t_WAITPAT);
    op->set_brief("Wait for PAT event flag, then clear it.");
    op->set_instr("WAITPAT          {WC/WZ/WCZ}");
    op->add_descr("Prior SETQ sets optional CT timeout value.");
    op->add_descr("C/Z = timeout.");
}

/**
 * @brief Wait for FBW event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000011001 000100100
 *
 * WAITFBW          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 */
void P2Doc::doc_WAITFBW(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000011001 000100100");

    op->set_token(t_WAITFBW);
    op->set_brief("Wait for FBW event flag, then clear it.");
    op->set_instr("WAITFBW          {WC/WZ/WCZ}");
    op->add_descr("Prior SETQ sets optional CT timeout value.");
    op->add_descr("C/Z = timeout.");
}

/**
 * @brief Wait for XMT event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000011010 000100100
 *
 * WAITXMT          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 */
void P2Doc::doc_WAITXMT(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000011010 000100100");

    op->set_token(t_WAITXMT);
    op->set_brief("Wait for XMT event flag, then clear it.");
    op->set_instr("WAITXMT          {WC/WZ/WCZ}");
    op->add_descr("Prior SETQ sets optional CT timeout value.");
    op->add_descr("C/Z = timeout.");
}

/**
 * @brief Wait for XFI event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000011011 000100100
 *
 * WAITXFI          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 */
void P2Doc::doc_WAITXFI(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000011011 000100100");

    op->set_token(t_WAITXFI);
    op->set_brief("Wait for XFI event flag, then clear it.");
    op->set_instr("WAITXFI          {WC/WZ/WCZ}");
    op->add_descr("Prior SETQ sets optional CT timeout value.");
    op->add_descr("C/Z = timeout.");
}

/**
 * @brief Wait for XRO event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000011100 000100100
 *
 * WAITXRO          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 */
void P2Doc::doc_WAITXRO(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000011100 000100100");

    op->set_token(t_WAITXRO);
    op->set_brief("Wait for XRO event flag, then clear it.");
    op->set_instr("WAITXRO          {WC/WZ/WCZ}");
    op->add_descr("Prior SETQ sets optional CT timeout value.");
    op->add_descr("C/Z = timeout.");
}

/**
 * @brief Wait for XRL event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000011101 000100100
 *
 * WAITXRL          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 */
void P2Doc::doc_WAITXRL(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000011101 000100100");

    op->set_token(t_WAITXRL);
    op->set_brief("Wait for XRL event flag, then clear it.");
    op->set_instr("WAITXRL          {WC/WZ/WCZ}");
    op->add_descr("Prior SETQ sets optional CT timeout value.");
    op->add_descr("C/Z = timeout.");
}

/**
 * @brief Wait for ATN event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000011110 000100100
 *
 * WAITATN          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 */
void P2Doc::doc_WAITATN(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 000011110 000100100");

    op->set_token(t_WAITATN);
    op->set_brief("Wait for ATN event flag, then clear it.");
    op->set_instr("WAITATN          {WC/WZ/WCZ}");
    op->add_descr("Prior SETQ sets optional CT timeout value.");
    op->add_descr("C/Z = timeout.");
}

/**
 * @brief Allow interrupts (default).
 *
 * EEEE 1101011 000 000100000 000100100
 *
 * ALLOWI
 *
 *
 */
void P2Doc::doc_ALLOWI(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 000100000 000100100");

    op->set_token(t_ALLOWI);
    op->set_brief("Allow interrupts (default).");
    op->set_instr("ALLOWI");
}

/**
 * @brief Stall Interrupts.
 *
 * EEEE 1101011 000 000100001 000100100
 *
 * STALLI
 *
 *
 */
void P2Doc::doc_STALLI(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 000100001 000100100");

    op->set_token(t_STALLI);
    op->set_brief("Stall Interrupts.");
    op->set_instr("STALLI");
}

/**
 * @brief Trigger INT1, regardless of STALLI mode.
 *
 * EEEE 1101011 000 000100010 000100100
 *
 * TRGINT1
 *
 *
 */
void P2Doc::doc_TRGINT1(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 000100010 000100100");

    op->set_token(t_TRGINT1);
    op->set_brief("Trigger INT1, regardless of STALLI mode.");
    op->set_instr("TRGINT1");
}

/**
 * @brief Trigger INT2, regardless of STALLI mode.
 *
 * EEEE 1101011 000 000100011 000100100
 *
 * TRGINT2
 *
 *
 */
void P2Doc::doc_TRGINT2(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 000100011 000100100");

    op->set_token(t_TRGINT2);
    op->set_brief("Trigger INT2, regardless of STALLI mode.");
    op->set_instr("TRGINT2");
}

/**
 * @brief Trigger INT3, regardless of STALLI mode.
 *
 * EEEE 1101011 000 000100100 000100100
 *
 * TRGINT3
 *
 *
 */
void P2Doc::doc_TRGINT3(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 000100100 000100100");

    op->set_token(t_TRGINT3);
    op->set_brief("Trigger INT3, regardless of STALLI mode.");
    op->set_instr("TRGINT3");
}

/**
 * @brief Cancel INT1.
 *
 * EEEE 1101011 000 000100101 000100100
 *
 * NIXINT1
 *
 *
 */
void P2Doc::doc_NIXINT1(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 000100101 000100100");

    op->set_token(t_NIXINT1);
    op->set_brief("Cancel INT1.");
    op->set_instr("NIXINT1");
}

/**
 * @brief Cancel INT2.
 *
 * EEEE 1101011 000 000100110 000100100
 *
 * NIXINT2
 *
 *
 */
void P2Doc::doc_NIXINT2(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 000100110 000100100");

    op->set_token(t_NIXINT2);
    op->set_brief("Cancel INT2.");
    op->set_instr("NIXINT2");
}

/**
 * @brief Cancel INT3.
 *
 * EEEE 1101011 000 000100111 000100100
 *
 * NIXINT3
 *
 *
 */
void P2Doc::doc_NIXINT3(p2_OPX24_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 000100111 000100100");

    op->set_token(t_NIXINT3);
    op->set_brief("Cancel INT3.");
    op->set_instr("NIXINT3");
}

/**
 * @brief Set INT1 source to D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100101
 *
 * SETINT1 {#}D
 *
 * 0	<off>, default on cog start for INT1/INT2/INT3 event sources
 * 1	CT-passed-CT1, established by ADDCT1
 * 2	CT-passed-CT2, established by ADDCT2
 * 3	CT-passed-CT3, established by ADDCT3
 * 4	SE1 event occurred, established by SETSE1
 * 5	SE2 event occurred, established by SETSE2
 * 6	SE3 event occurred, established by SETSE3
 * 7	SE4 event occurred, established by SETSE4
 * 8	Pin pattern match or mismatch occurred, established by SETPAT
 * 9	Hub RAM FIFO interface wrapped and reloaded, established by RDFAST/WRFAST/FBLOCK
 * 10	Streamer is ready for another command, established by XINIT/XZERO/ZCONT
 * 11	Streamer ran out of commands, established by XINIT/XZERO/ZCONT
 * 12	Streamer NCO rolled over, established by XINIT/XZERO/XCONT
 * 13	Streamer read location $1FF of lookup RAM
 * 14	Attention requested by other cog(s)
 * 15	GETQX/GETQY executed without any CORDIC results available or in progress
 *
 */
void P2Doc::doc_SETINT1(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000100101");

    op->set_token(t_SETINT1);
    op->set_brief("Set INT1 source to D[3:0].");
    op->set_instr("SETINT1 {#}D");
    op->add_descr("0	<off>, default on cog start for INT1/INT2/INT3 event sources");
    op->add_descr("1	CT-passed-CT1, established by ADDCT1");
    op->add_descr("2	CT-passed-CT2, established by ADDCT2");
    op->add_descr("3	CT-passed-CT3, established by ADDCT3");
    op->add_descr("4	SE1 event occurred, established by SETSE1");
    op->add_descr("5	SE2 event occurred, established by SETSE2");
    op->add_descr("6	SE3 event occurred, established by SETSE3");
    op->add_descr("7	SE4 event occurred, established by SETSE4");
    op->add_descr("8	Pin pattern match or mismatch occurred, established by SETPAT");
    op->add_descr("9	Hub RAM FIFO interface wrapped and reloaded, established by RDFAST/WRFAST/FBLOCK");
    op->add_descr("10	Streamer is ready for another command, established by XINIT/XZERO/ZCONT");
    op->add_descr("11	Streamer ran out of commands, established by XINIT/XZERO/ZCONT");
    op->add_descr("12	Streamer NCO rolled over, established by XINIT/XZERO/XCONT");
    op->add_descr("13	Streamer read location $1FF of lookup RAM");
    op->add_descr("14	Attention requested by other cog(s)");
    op->add_descr("15	GETQX/GETQY executed without any CORDIC results available or in progress");
}

/**
 * @brief Set INT2 source to D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100110
 *
 * SETINT2 {#}D
 *
 *
 */
void P2Doc::doc_SETINT2(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000100110");

    op->set_token(t_SETINT2);
    op->set_brief("Set INT2 source to D[3:0].");
    op->set_instr("SETINT2 {#}D");
    op->add_descr("0	<off>, default on cog start for INT1/INT2/INT3 event sources");
    op->add_descr("1	CT-passed-CT1, established by ADDCT1");
    op->add_descr("2	CT-passed-CT2, established by ADDCT2");
    op->add_descr("3	CT-passed-CT3, established by ADDCT3");
    op->add_descr("4	SE1 event occurred, established by SETSE1");
    op->add_descr("5	SE2 event occurred, established by SETSE2");
    op->add_descr("6	SE3 event occurred, established by SETSE3");
    op->add_descr("7	SE4 event occurred, established by SETSE4");
    op->add_descr("8	Pin pattern match or mismatch occurred, established by SETPAT");
    op->add_descr("9	Hub RAM FIFO interface wrapped and reloaded, established by RDFAST/WRFAST/FBLOCK");
    op->add_descr("10	Streamer is ready for another command, established by XINIT/XZERO/ZCONT");
    op->add_descr("11	Streamer ran out of commands, established by XINIT/XZERO/ZCONT");
    op->add_descr("12	Streamer NCO rolled over, established by XINIT/XZERO/XCONT");
    op->add_descr("13	Streamer read location $1FF of lookup RAM");
    op->add_descr("14	Attention requested by other cog(s)");
    op->add_descr("15	GETQX/GETQY executed without any CORDIC results available or in progress");
}

/**
 * @brief Set INT3 source to D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100111
 *
 * SETINT3 {#}D
 *
 *
 */
void P2Doc::doc_SETINT3(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000100111");

    op->set_token(t_SETINT3);
    op->set_brief("Set INT3 source to D[3:0].");
    op->set_instr("SETINT3 {#}D");
    op->add_descr("0	<off>, default on cog start for INT1/INT2/INT3 event sources");
    op->add_descr("1	CT-passed-CT1, established by ADDCT1");
    op->add_descr("2	CT-passed-CT2, established by ADDCT2");
    op->add_descr("3	CT-passed-CT3, established by ADDCT3");
    op->add_descr("4	SE1 event occurred, established by SETSE1");
    op->add_descr("5	SE2 event occurred, established by SETSE2");
    op->add_descr("6	SE3 event occurred, established by SETSE3");
    op->add_descr("7	SE4 event occurred, established by SETSE4");
    op->add_descr("8	Pin pattern match or mismatch occurred, established by SETPAT");
    op->add_descr("9	Hub RAM FIFO interface wrapped and reloaded, established by RDFAST/WRFAST/FBLOCK");
    op->add_descr("10	Streamer is ready for another command, established by XINIT/XZERO/ZCONT");
    op->add_descr("11	Streamer ran out of commands, established by XINIT/XZERO/ZCONT");
    op->add_descr("12	Streamer NCO rolled over, established by XINIT/XZERO/XCONT");
    op->add_descr("13	Streamer read location $1FF of lookup RAM");
    op->add_descr("14	Attention requested by other cog(s)");
    op->add_descr("15	GETQX/GETQY executed without any CORDIC results available or in progress");
}

/**
 * @brief Set Q to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000101000
 *
 * SETQ    {#}D
 *
 * Use before RDLONG/WRLONG/WMLONG to set block transfer.
 * Also used before MUXQ/COGINIT/QDIV/QFRAC/QROTATE/WAITxxx.
 *
 */
void P2Doc::doc_SETQ(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000101000");

    op->set_token(t_SETQ);
    op->set_brief("Set Q to D.");
    op->set_instr("SETQ    {#}D");
    op->add_descr("Use before RDLONG/WRLONG/WMLONG to set block transfer.");
    op->add_descr("Also used before MUXQ/COGINIT/QDIV/QFRAC/QROTATE/WAITxxx.");
}

/**
 * @brief Set Q to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000101001
 *
 * SETQ2   {#}D
 *
 * Use before RDLONG/WRLONG/WMLONG to set LUT block transfer.
 *
 */
void P2Doc::doc_SETQ2(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000101001");

    op->set_token(t_SETQ2);
    op->set_brief("Set Q to D.");
    op->set_instr("SETQ2   {#}D");
    op->add_descr("Use before RDLONG/WRLONG/WMLONG to set LUT block transfer.");
}

/**
 * @brief Push D onto stack.
 *
 * EEEE 1101011 00L DDDDDDDDD 000101010
 *
 * PUSH    {#}D
 *
 *
 */
void P2Doc::doc_PUSH(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000101010");

    op->set_token(t_PUSH);
    op->set_brief("Push D onto stack.");
    op->set_instr("PUSH    {#}D");
}

/**
 * @brief Pop stack (K).
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101011
 *
 * POP     D        {WC/WZ/WCZ}
 *
 * C = K[31], Z = K[30], D = K.
 *
 */
void P2Doc::doc_POP(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 DDDDDDDDD 000101011");

    op->set_token(t_POP);
    op->set_brief("Pop stack (K).");
    op->set_instr("POP     D        {WC/WZ/WCZ}");
    op->add_descr("C = K[31], Z = K[30], D = K.");
}

/**
 * @brief Jump to D.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101100
 *
 * JMP     D        {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 *
 */
void P2Doc::doc_JMP(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 DDDDDDDDD 000101100");

    op->set_token(t_JMP);
    op->set_brief("Jump to D.");
    op->set_instr("JMP     D        {WC/WZ/WCZ}");
    op->add_descr("C = D[31], Z = D[30], PC = D[19:0].");
}

/**
 * @brief Call to D by pushing {C, Z, 10'b0, PC[19:0]} onto stack.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101101
 *
 * CALL    D        {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 *
 */
void P2Doc::doc_CALL(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 DDDDDDDDD 000101101", match_I_S);

    op->set_token(t_CALL);
    op->set_brief("Call to D by pushing {C, Z, 10'b0, PC[19:0]} onto stack.");
    op->set_instr("CALL    D        {WC/WZ/WCZ}");
    op->add_descr("C = D[31], Z = D[30], PC = D[19:0].");
}

/**
 * @brief Return by popping stack (K).
 *
 * EEEE 1101011 CZ1 000000000 000101101
 *
 * RET              {WC/WZ/WCZ}
 *
 * C = K[31], Z = K[30], PC = K[19:0].
 *
 */
void P2Doc::doc_RET(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ1 000000000 000101101", match_I_D);

    op->set_token(t_RET);
    op->set_brief("Return by popping stack (K).");
    op->set_instr("RET              {WC/WZ/WCZ}");
    op->add_descr("C = K[31], Z = K[30], PC = K[19:0].");
}

/**
 * @brief Call to D by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRA++.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101110
 *
 * CALLA   D        {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 *
 */
void P2Doc::doc_CALLA(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 DDDDDDDDD 000101110", match_I_S);

    op->set_token(t_CALLA);
    op->set_brief("Call to D by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRA++.");
    op->set_instr("CALLA   D        {WC/WZ/WCZ}");
    op->add_descr("C = D[31], Z = D[30], PC = D[19:0].");
}

/**
 * @brief Return by reading hub long (L) at --PTRA.
 *
 * EEEE 1101011 CZ1 000000000 000101110
 *
 * RETA             {WC/WZ/WCZ}
 *
 * C = L[31], Z = L[30], PC = L[19:0].
 *
 */
void P2Doc::doc_RETA(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ1 000000000 000101110", match_I_D);

    op->set_token(t_RETA);
    op->set_brief("Return by reading hub long (L) at --PTRA.");
    op->set_instr("RETA             {WC/WZ/WCZ}");
    op->add_descr("C = L[31], Z = L[30], PC = L[19:0].");
}

/**
 * @brief Call to D by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRB++.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101111
 *
 * CALLB   D        {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 *
 */
void P2Doc::doc_CALLB(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 DDDDDDDDD 000101111", match_I_S);

    op->set_token(t_CALLB);
    op->set_brief("Call to D by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRB++.");
    op->set_instr("CALLB   D        {WC/WZ/WCZ}");
    op->add_descr("C = D[31], Z = D[30], PC = D[19:0].");
}

/**
 * @brief Return by reading hub long (L) at --PTRB.
 *
 * EEEE 1101011 CZ1 000000000 000101111
 *
 * RETB             {WC/WZ/WCZ}
 *
 * C = L[31], Z = L[30], PC = L[19:0].
 *
 */
void P2Doc::doc_RETB(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ1 000000000 000101111", match_I_D);

    op->set_token(t_RETB);
    op->set_brief("Return by reading hub long (L) at --PTRB.");
    op->set_instr("RETB             {WC/WZ/WCZ}");
    op->add_descr("C = L[31], Z = L[30], PC = L[19:0].");
}

/**
 * @brief Jump ahead/back by D instructions.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110000
 *
 * JMPREL  {#}D
 *
 * For cogex, PC += D[19:0].
 * For hubex, PC += D[17:0] << 2.
 *
 */
void P2Doc::doc_JMPREL(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000110000");

    op->set_token(t_JMPREL);
    op->set_brief("Jump ahead/back by D instructions.");
    op->set_instr("JMPREL  {#}D");
    op->add_descr("For cogex, PC += D[19:0].");
    op->add_descr("For hubex, PC += D[17:0] << 2.");
}

/**
 * @brief Skip instructions per D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110001
 *
 * SKIP    {#}D
 *
 * Subsequent instructions 0..31 get cancelled for each '1' bit in D[0]..D[31].
 *
 */
void P2Doc::doc_SKIP(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000110001");

    op->set_token(t_SKIP);
    op->set_brief("Skip instructions per D.");
    op->set_instr("SKIP    {#}D");
    op->add_descr("Subsequent instructions 0..31 get cancelled for each '1' bit in D[0]..D[31].");
}

/**
 * @brief Skip cog/LUT instructions fast per D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110010
 *
 * SKIPF   {#}D
 *
 * Like SKIP, but instead of cancelling instructions, the PC leaps over them.
 *
 */
void P2Doc::doc_SKIPF(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000110010");

    op->set_token(t_SKIPF);
    op->set_brief("Skip cog/LUT instructions fast per D.");
    op->set_instr("SKIPF   {#}D");
    op->add_descr("Like SKIP, but instead of cancelling instructions, the PC leaps over them.");
}

/**
 * @brief Jump to D[9:0] in cog/LUT and set SKIPF pattern to D[31:10].
 *
 * EEEE 1101011 00L DDDDDDDDD 000110011
 *
 * EXECF   {#}D
 *
 * PC = {10'b0, D[9:0]}.
 *
 */
void P2Doc::doc_EXECF(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000110011");

    op->set_token(t_EXECF);
    op->set_instr("EXECF   {#}D");
    op->set_brief("Jump to D[9:0] in cog/LUT and set SKIPF pattern to D[31:10].");
    op->add_descr("PC = {10'b0, D[9:0]}.");
}

/**
 * @brief Get current FIFO hub pointer into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 000110100
 *
 * GETPTR  D
 *
 *
 */
void P2Doc::doc_GETPTR(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 DDDDDDDDD 000110100");

    op->set_token(t_GETPTR);
    op->set_brief("Get current FIFO hub pointer into D.");
    op->set_instr("GETPTR  D");
}

/**
 * @brief Get breakpoint status into D according to WC/WZ/WCZ.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000110101
 *
 * GETBRK  D          WC/WZ/WCZ
 *
 * C = 0.
 * Z = 0.
 *
 */
void P2Doc::doc_GETBRK(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 DDDDDDDDD 000110101", match_I);

    op->set_token(t_GETBRK);
    op->set_brief("Get breakpoint status into D according to WC/WZ/WCZ.");
    op->set_instr("GETBRK  D          WC/WZ/WCZ");
    op->add_descr("C = 0.");
    op->add_descr("Z = 0.");
}

/**
 * @brief If in debug ISR, trigger asynchronous breakpoint in cog D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000110101
 *
 * COGBRK  {#}D
 *
 * Cog D[3:0] must have asynchronous breakpoint enabled.
 *
 */
void P2Doc::doc_COGBRK(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000110101", match_CZ);

    op->set_token(t_COGBRK);
    op->set_brief("If in debug ISR, trigger asynchronous breakpoint in cog D[3:0].");
    op->set_instr("COGBRK  {#}D");
    op->add_descr("Cog D[3:0] must have asynchronous breakpoint enabled.");
}

/**
 * @brief If in debug ISR, set next break condition to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110110
 *
 * BRK     {#}D
 *
 * Else, trigger break if enabled, conditionally write break code to D[7:0].
 *
 */
void P2Doc::doc_BRK(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000110110");

    op->set_token(t_BRK);
    op->set_brief("If in debug ISR, set next break condition to D.");
    op->set_instr("BRK     {#}D");
    op->add_descr("Else, trigger break if enabled, conditionally write break code to D[7:0].");
}

/**
 * @brief If D[0] = 1 then enable LUT sharing, where LUT writes within the adjacent odd/even companion cog are copied to this LUT.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110111
 *
 * SETLUTS {#}D
 *
 *
 */
void P2Doc::doc_SETLUTS(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000110111");

    op->set_token(t_SETLUTS);
    op->set_brief("If D[0] = 1 then enable LUT sharing, where LUT writes within the adjacent odd/even companion cog are copied to this LUT.");
    op->set_instr("SETLUTS {#}D");
}

/**
 * @brief Set the colorspace converter "CY" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111000
 *
 * SETCY   {#}D
 *
 *
 */
void P2Doc::doc_SETCY(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000111000");

    op->set_token(t_SETCY);
    op->set_brief("Set the colorspace converter \"CY\" parameter to D[31:0].");
    op->set_instr("SETCY   {#}D");
}

/**
 * @brief Set the colorspace converter "CI" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111001
 *
 * SETCI   {#}D
 *
 *
 */
void P2Doc::doc_SETCI(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000111001");

    op->set_token(t_SETCI);
    op->set_brief("Set the colorspace converter \"CI\" parameter to D[31:0].");
    op->set_instr("SETCI   {#}D");
}

/**
 * @brief Set the colorspace converter "CQ" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111010
 *
 * SETCQ   {#}D
 *
 *
 */
void P2Doc::doc_SETCQ(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000111010");

    op->set_token(t_SETCQ);
    op->set_brief("Set the colorspace converter \"CQ\" parameter to D[31:0].");
    op->set_instr("SETCQ   {#}D");
}

/**
 * @brief Set the colorspace converter "CFRQ" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111011
 *
 * SETCFRQ {#}D
 *
 *
 */
void P2Doc::doc_SETCFRQ(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000111011");

    op->set_token(t_SETCFRQ);
    op->set_brief("Set the colorspace converter \"CFRQ\" parameter to D[31:0].");
    op->set_instr("SETCFRQ {#}D");
}

/**
 * @brief Set the colorspace converter "CMOD" parameter to D[6:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111100
 *
 * SETCMOD {#}D
 *
 *
 */
void P2Doc::doc_SETCMOD(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000111100");

    op->set_token(t_SETCMOD);
    op->set_brief("Set the colorspace converter \"CMOD\" parameter to D[6:0].");
    op->set_instr("SETCMOD {#}D");
}

/**
 * @brief Set BLNPIX/MIXPIX blend factor to D[7:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111101
 *
 * SETPIV  {#}D
 *
 *
 */
void P2Doc::doc_SETPIV(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000111101");

    op->set_token(t_SETPIV);
    op->set_brief("Set BLNPIX/MIXPIX blend factor to D[7:0].");
    op->set_instr("SETPIV  {#}D");
}

/**
 * @brief Set MIXPIX mode to D[5:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111110
 *
 * SETPIX  {#}D
 *
 *
 */
void P2Doc::doc_SETPIX(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000111110");

    op->set_token(t_SETPIX);
    op->set_brief("Set MIXPIX mode to D[5:0].");
    op->set_instr("SETPIX  {#}D");
}

/**
 * @brief Strobe "attention" of all cogs whose corresponging bits are high in D[15:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111111
 *
 * COGATN  {#}D
 *
 *
 */
void P2Doc::doc_COGATN(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 000111111");

    op->set_token(t_COGATN);
    op->set_brief("Strobe \"attention\" of all cogs whose corresponging bits are high in D[15:0].");
    op->set_instr("COGATN  {#}D");
}

/**
 * @brief Test  IN bit of pin D[5:0], write to C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000000
 *
 * TESTP   {#}D           WC/WZ
 *
 * C/Z =          IN[D[5:0]].
 *
 */
void P2Doc::doc_TESTP_W(p2_OPSRC_e instr, p2_INST9_e inst9)
{
    P2DocOpcode op = make_pattern(__func__, instr, inst9, "EEEE 1101011 CZL DDDDDDDDD 001000000", match_C_neq_Z);

    op->set_token(t_TESTP);
    op->set_brief("Test  IN bit of pin D[5:0], write to C/Z.");
    op->set_instr("TESTP   {#}D           WC/WZ");
    op->add_descr("C/Z =          IN[D[5:0]].");
}


/**
 * @brief Test !IN bit of pin D[5:0], write to C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000001
 *
 * TESTPN  {#}D           WC/WZ
 *
 * C/Z =         !IN[D[5:0]].
 *
 */
void P2Doc::doc_TESTPN_W(p2_OPSRC_e instr, p2_INST9_e inst9)
{
    P2DocOpcode op = make_pattern(__func__, instr, inst9, "EEEE 1101011 CZL DDDDDDDDD 001000001", match_C_neq_Z);

    op->set_token(t_TESTPN);
    op->set_brief("Test !IN bit of pin D[5:0], write to C/Z.");
    op->set_instr("TESTPN  {#}D           WC/WZ");
    op->add_descr("C/Z =         !IN[D[5:0]].");
}

/**
 * @brief Test  IN bit of pin D[5:0], AND into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000010
 *
 * TESTP   {#}D       ANDC/ANDZ
 *
 * C/Z = C/Z AND  IN[D[5:0]].
 *
 */
void P2Doc::doc_TESTP_AND(p2_OPSRC_e instr, p2_INST9_e inst9)
{
    P2DocOpcode op = make_pattern(__func__, instr, inst9, "EEEE 1101011 CZL DDDDDDDDD 001000010", match_C_neq_Z);

    op->set_token(t_TESTP);
    op->set_brief("Test  IN bit of pin D[5:0], AND into C/Z.");
    op->set_instr("TESTP   {#}D       ANDC/ANDZ");
    op->add_descr("C/Z = C/Z AND  IN[D[5:0]].");
}

/**
 * @brief Test !IN bit of pin D[5:0], AND into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000011
 *
 * TESTPN  {#}D       ANDC/ANDZ
 *
 * C/Z = C/Z AND !IN[D[5:0]].
 *
 */
void P2Doc::doc_TESTPN_AND(p2_OPSRC_e instr, p2_INST9_e inst9)
{
    P2DocOpcode op = make_pattern(__func__, instr, inst9, "EEEE 1101011 CZL DDDDDDDDD 001000011", match_C_neq_Z);

    op->set_token(t_TESTPN);
    op->set_brief("Test !IN bit of pin D[5:0], AND into C/Z.");
    op->set_instr("TESTPN  {#}D       ANDC/ANDZ");
    op->add_descr("C/Z = C/Z AND !IN[D[5:0]].");
}

/**
 * @brief Test  IN bit of pin D[5:0], OR  into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000100
 *
 * TESTP   {#}D         ORC/ORZ
 *
 * C/Z = C/Z OR   IN[D[5:0]].
 *
 */
void P2Doc::doc_TESTP_OR(p2_OPSRC_e instr, p2_INST9_e inst9)
{
    P2DocOpcode op = make_pattern(__func__, instr, inst9, "EEEE 1101011 CZL DDDDDDDDD 001000100", match_C_neq_Z);

    op->set_token(t_TESTP);
    op->set_brief("Test  IN bit of pin D[5:0], OR  into C/Z.");
    op->set_instr("TESTP   {#}D         ORC/ORZ");
    op->add_descr("C/Z = C/Z OR   IN[D[5:0]].");
}

/**
 * @brief Test !IN bit of pin D[5:0], OR  into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000101
 *
 * TESTPN  {#}D         ORC/ORZ
 *
 * C/Z = C/Z OR  !IN[D[5:0]].
 *
 */
void P2Doc::doc_TESTPN_OR(p2_OPSRC_e instr, p2_INST9_e inst9)
{
    P2DocOpcode op = make_pattern(__func__, instr, inst9, "EEEE 1101011 CZL DDDDDDDDD 001000101", match_C_neq_Z);

    op->set_token(t_TESTPN);
    op->set_brief("Test !IN bit of pin D[5:0], OR  into C/Z.");
    op->set_instr("TESTPN  {#}D         ORC/ORZ");
    op->add_descr("C/Z = C/Z OR  !IN[D[5:0]].");
}

/**
 * @brief Test  IN bit of pin D[5:0], XOR into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000110
 *
 * TESTP   {#}D       XORC/XORZ
 *
 * C/Z = C/Z XOR  IN[D[5:0]].
 *
 */
void P2Doc::doc_TESTP_XOR(p2_OPSRC_e instr, p2_INST9_e inst9)
{
    P2DocOpcode op = make_pattern(__func__, instr, inst9, "EEEE 1101011 CZL DDDDDDDDD 001000110", match_C_neq_Z);

    op->set_token(t_TESTP);
    op->set_brief("Test  IN bit of pin D[5:0], XOR into C/Z.");
    op->set_instr("TESTP   {#}D       XORC/XORZ");
    op->add_descr("C/Z = C/Z XOR  IN[D[5:0]].");
}

/**
 * @brief Test !IN bit of pin D[5:0], XOR into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000111
 *
 * TESTPN  {#}D       XORC/XORZ
 *
 * C/Z = C/Z XOR !IN[D[5:0]].
 *
 */
void P2Doc::doc_TESTPN_XOR(p2_OPSRC_e instr, p2_INST9_e inst9)
{
    P2DocOpcode op = make_pattern(__func__, instr, inst9, "EEEE 1101011 CZL DDDDDDDDD 001000111", match_C_neq_Z);

    op->set_token(t_TESTPN);
    op->set_brief("Test !IN bit of pin D[5:0], XOR into C/Z.");
    op->set_instr("TESTPN  {#}D       XORC/XORZ");
    op->add_descr("C/Z = C/Z XOR !IN[D[5:0]].");
}

/**
 * @brief DIR bit of pin D[5:0] = 0.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000000
 *
 * DIRL    {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *
 */
void P2Doc::doc_DIRL(p2_OPSRC_e instr, p2_INST9_e inst9)
{
    P2DocOpcode op = make_pattern(__func__, instr, inst9, "EEEE 1101011 CZL DDDDDDDDD 001000000", match_C_equ_Z);

    op->set_token(t_DIRL);
    op->set_brief("DIR bit of pin D[5:0] = 0.");
    op->set_instr("DIRL    {#}D           {WCZ}");
    op->add_descr("C,Z = DIR bit.");
}

/**
 * @brief DIR bit of pin D[5:0] = 1.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000001
 *
 * DIRH    {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *
 */
void P2Doc::doc_DIRH(p2_OPSRC_e instr, p2_INST9_e inst9)
{
    P2DocOpcode op = make_pattern(__func__, instr, inst9, "EEEE 1101011 CZL DDDDDDDDD 001000001", match_C_equ_Z);

    op->set_token(t_DIRH);
    op->set_brief("DIR bit of pin D[5:0] = 1.");
    op->set_instr("DIRH    {#}D           {WCZ}");
    op->add_descr("C,Z = DIR bit.");
}

/**
 * @brief DIR bit of pin D[5:0] = C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000010
 *
 * DIRC    {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *
 */
void P2Doc::doc_DIRC(p2_OPSRC_e instr, p2_INST9_e inst9)
{
    P2DocOpcode op = make_pattern(__func__, instr, inst9, "EEEE 1101011 CZL DDDDDDDDD 001000010", match_C_equ_Z);

    op->set_token(t_DIRC);
    op->set_brief("DIR bit of pin D[5:0] = C.");
    op->set_instr("DIRC    {#}D           {WCZ}");
    op->add_descr("C,Z = DIR bit.");
}

/**
 * @brief DIR bit of pin D[5:0] = !C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000011
 *
 * DIRNC   {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *
 */
void P2Doc::doc_DIRNC(p2_OPSRC_e instr, p2_INST9_e inst9)
{
    P2DocOpcode op = make_pattern(__func__, instr, inst9, "EEEE 1101011 CZL DDDDDDDDD 001000011", match_C_equ_Z);

    op->set_token(t_DIRNC);
    op->set_brief("DIR bit of pin D[5:0] = !C.");
    op->set_instr("DIRNC   {#}D           {WCZ}");
    op->add_descr("C,Z = DIR bit.");
}

/**
 * @brief DIR bit of pin D[5:0] = Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000100
 *
 * DIRZ    {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *
 */
void P2Doc::doc_DIRZ(p2_OPSRC_e instr, p2_INST9_e inst9)
{
    P2DocOpcode op = make_pattern(__func__, instr, inst9, "EEEE 1101011 CZL DDDDDDDDD 001000100", match_C_equ_Z);

    op->set_token(t_DIRZ);
    op->set_brief("DIR bit of pin D[5:0] = Z.");
    op->set_instr("DIRZ    {#}D           {WCZ}");
    op->add_descr("C,Z = DIR bit.");
}

/**
 * @brief DIR bit of pin D[5:0] = !Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000101
 *
 * DIRNZ   {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *
 */
void P2Doc::doc_DIRNZ(p2_OPSRC_e instr, p2_INST9_e inst9)
{
    P2DocOpcode op = make_pattern(__func__, instr, inst9, "EEEE 1101011 CZL DDDDDDDDD 001000101", match_C_equ_Z);

    op->set_token(t_DIRNZ);
    op->set_brief("DIR bit of pin D[5:0] = !Z.");
    op->set_instr("DIRNZ   {#}D           {WCZ}");
    op->add_descr("C,Z = DIR bit.");
}

/**
 * @brief DIR bit of pin D[5:0] = RND.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000110
 *
 * DIRRND  {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *
 */
void P2Doc::doc_DIRRND(p2_OPSRC_e instr, p2_INST9_e inst9)
{
    P2DocOpcode op = make_pattern(__func__, instr, inst9, "EEEE 1101011 CZL DDDDDDDDD 001000110", match_C_equ_Z);

    op->set_token(t_DIRRND);
    op->set_brief("DIR bit of pin D[5:0] = RND.");
    op->set_instr("DIRRND  {#}D           {WCZ}");
    op->add_descr("C,Z = DIR bit.");
}

/**
 * @brief DIR bit of pin D[5:0] = !bit.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000111
 *
 * DIRNOT  {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *
 */
void P2Doc::doc_DIRNOT(p2_OPSRC_e instr, p2_INST9_e inst9)
{
    P2DocOpcode op = make_pattern(__func__, instr, inst9, "EEEE 1101011 CZL DDDDDDDDD 001000111", match_C_equ_Z);

    op->set_token(t_DIRNOT);
    op->set_brief("DIR bit of pin D[5:0] = !bit.");
    op->set_instr("DIRNOT  {#}D           {WCZ}");
    op->add_descr("C,Z = DIR bit.");
}

/**
 * @brief OUT bit of pin D[5:0] = 0.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001000
 *
 * OUTL    {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *
 */
void P2Doc::doc_OUTL(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZL DDDDDDDDD 001001000", match_C_equ_Z);

    op->set_token(t_OUTL);
    op->set_brief("OUT bit of pin D[5:0] = 0.");
    op->set_instr("OUTL    {#}D           {WCZ}");
    op->add_descr("C,Z = OUT bit.");
}

/**
 * @brief OUT bit of pin D[5:0] = 1.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001001
 *
 * OUTH    {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *
 */
void P2Doc::doc_OUTH(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZL DDDDDDDDD 001001001", match_C_equ_Z);

    op->set_token(t_OUTH);
    op->set_brief("OUT bit of pin D[5:0] = 1.");
    op->set_instr("OUTH    {#}D           {WCZ}");
    op->add_descr("C,Z = OUT bit.");
}

/**
 * @brief OUT bit of pin D[5:0] = C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001010
 *
 * OUTC    {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *
 */
void P2Doc::doc_OUTC(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZL DDDDDDDDD 001001010", match_C_equ_Z);

    op->set_token(t_OUTC);
    op->set_brief("OUT bit of pin D[5:0] = C.");
    op->set_instr("OUTC    {#}D           {WCZ}");
    op->add_descr("C,Z = OUT bit.");
}

/**
 * @brief OUT bit of pin D[5:0] = !C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001011
 *
 * OUTNC   {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *
 */
void P2Doc::doc_OUTNC(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZL DDDDDDDDD 001001011", match_C_equ_Z);

    op->set_token(t_OUTNC);
    op->set_brief("OUT bit of pin D[5:0] = !C.");
    op->set_instr("OUTNC   {#}D           {WCZ}");
    op->add_descr("C,Z = OUT bit.");
}

/**
 * @brief OUT bit of pin D[5:0] = Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001100
 *
 * OUTZ    {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *
 */
void P2Doc::doc_OUTZ(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZL DDDDDDDDD 001001100", match_C_equ_Z);

    op->set_token(t_OUTZ);
    op->set_brief("OUT bit of pin D[5:0] = Z.");
    op->set_instr("OUTZ    {#}D           {WCZ}");
    op->add_descr("C,Z = OUT bit.");
}

/**
 * @brief OUT bit of pin D[5:0] = !Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001101
 *
 * OUTNZ   {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *
 */
void P2Doc::doc_OUTNZ(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZL DDDDDDDDD 001001101", match_C_equ_Z);

    op->set_token(t_OUTNZ);
    op->set_brief("OUT bit of pin D[5:0] = !Z.");
    op->set_instr("OUTNZ   {#}D           {WCZ}");
    op->add_descr("C,Z = OUT bit.");
}

/**
 * @brief OUT bit of pin D[5:0] = RND.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001110
 *
 * OUTRND  {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *
 */
void P2Doc::doc_OUTRND(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZL DDDDDDDDD 001001110", match_C_equ_Z);

    op->set_token(t_OUTRND);
    op->set_brief("OUT bit of pin D[5:0] = RND.");
    op->set_instr("OUTRND  {#}D           {WCZ}");
    op->add_descr("C,Z = OUT bit.");
}

/**
 * @brief OUT bit of pin D[5:0] = !bit.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001111
 *
 * OUTNOT  {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *
 */
void P2Doc::doc_OUTNOT(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZL DDDDDDDDD 001001111", match_C_equ_Z);

    op->set_token(t_OUTNOT);
    op->set_brief("OUT bit of pin D[5:0] = !bit.");
    op->set_instr("OUTNOT  {#}D           {WCZ}");
    op->add_descr("C,Z = OUT bit.");
}

/**
 * @brief OUT bit of pin D[5:0] = 0.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001010000
 *
 * FLTL    {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 *
 */
void P2Doc::doc_FLTL(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZL DDDDDDDDD 001010000");

    op->set_token(t_FLTL);
    op->set_brief("OUT bit of pin D[5:0] = 0.");
    op->set_instr("FLTL    {#}D           {WCZ}");
    op->add_descr("DIR bit = 0.");
    op->add_descr("C,Z = OUT bit.");
}

/**
 * @brief OUT bit of pin D[5:0] = 1.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001010001
 *
 * FLTH    {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 *
 */
void P2Doc::doc_FLTH(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZL DDDDDDDDD 001010001");

    op->set_token(t_FLTH);
    op->set_brief("OUT bit of pin D[5:0] = 1.");
    op->set_instr("FLTH    {#}D           {WCZ}");
    op->add_descr("DIR bit = 0.");
    op->add_descr("C,Z = OUT bit.");
}

/**
 * @brief OUT bit of pin D[5:0] = C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001010010
 *
 * FLTC    {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 *
 */
void P2Doc::doc_FLTC(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZL DDDDDDDDD 001010010");

    op->set_token(t_FLTC);
    op->set_brief("OUT bit of pin D[5:0] = C.");
    op->set_instr("FLTC    {#}D           {WCZ}");
    op->add_descr("DIR bit = 0.");
    op->add_descr("C,Z = OUT bit.");
}

/**
 * @brief OUT bit of pin D[5:0] = !C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001010011
 *
 * FLTNC   {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 *
 */
void P2Doc::doc_FLTNC(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZL DDDDDDDDD 001010011");

    op->set_token(t_FLTNC);
    op->set_brief("OUT bit of pin D[5:0] = !C.");
    op->set_instr("FLTNC   {#}D           {WCZ}");
    op->add_descr("DIR bit = 0.");
    op->add_descr("C,Z = OUT bit.");
}

/**
 * @brief OUT bit of pin D[5:0] = Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001010100
 *
 * FLTZ    {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 *
 */
void P2Doc::doc_FLTZ(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZL DDDDDDDDD 001010100");

    op->set_token(t_FLTZ);
    op->set_brief("OUT bit of pin D[5:0] = Z.");
    op->set_instr("FLTZ    {#}D           {WCZ}");
    op->add_descr("DIR bit = 0.");
    op->add_descr("C,Z = OUT bit.");
}

/**
 * @brief OUT bit of pin D[5:0] = !Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001010101
 *
 * FLTNZ   {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 *
 */
void P2Doc::doc_FLTNZ(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZL DDDDDDDDD 001010101");

    op->set_token(t_FLTNZ);
    op->set_brief("OUT bit of pin D[5:0] = !Z.");
    op->set_instr("FLTNZ   {#}D           {WCZ}");
    op->add_descr("DIR bit = 0.");
    op->add_descr("C,Z = OUT bit.");
}

/**
 * @brief OUT bit of pin D[5:0] = RND.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001010110
 *
 * FLTRND  {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 *
 */
void P2Doc::doc_FLTRND(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZL DDDDDDDDD 001010110");

    op->set_token(t_FLTRND);
    op->set_brief("OUT bit of pin D[5:0] = RND.");
    op->set_instr("FLTRND  {#}D           {WCZ}");
    op->add_descr("DIR bit = 0.");
    op->add_descr("C,Z = OUT bit.");
}

/**
 * @brief OUT bit of pin D[5:0] = !bit.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001010111
 *
 * FLTNOT  {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 *
 */
void P2Doc::doc_FLTNOT(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZL DDDDDDDDD 001010111");

    op->set_token(t_FLTNOT);
    op->set_brief("OUT bit of pin D[5:0] = !bit.");
    op->set_instr("FLTNOT  {#}D           {WCZ}");
    op->add_descr("DIR bit = 0.");
    op->add_descr("C,Z = OUT bit.");
}

/**
 * @brief OUT bit of pin D[5:0] = 0.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001011000
 *
 * DRVL    {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 *
 */
void P2Doc::doc_DRVL(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZL DDDDDDDDD 001011000");

    op->set_token(t_DRVL);
    op->set_brief("OUT bit of pin D[5:0] = 0.");
    op->set_instr("DRVL    {#}D           {WCZ}");
    op->add_descr("DIR bit = 1.");
    op->add_descr("C,Z = OUT bit.");
}

/**
 * @brief OUT bit of pin D[5:0] = 1.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001011001
 *
 * DRVH    {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 *
 */
void P2Doc::doc_DRVH(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZL DDDDDDDDD 001011001");

    op->set_token(t_DRVH);
    op->set_brief("OUT bit of pin D[5:0] = 1.");
    op->set_instr("DRVH    {#}D           {WCZ}");
    op->add_descr("DIR bit = 1.");
    op->add_descr("C,Z = OUT bit.");
}

/**
 * @brief OUT bit of pin D[5:0] = C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001011010
 *
 * DRVC    {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 *
 */
void P2Doc::doc_DRVC(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZL DDDDDDDDD 001011010");

    op->set_token(t_DRVC);
    op->set_brief("OUT bit of pin D[5:0] = C.");
    op->set_instr("DRVC    {#}D           {WCZ}");
    op->add_descr("DIR bit = 1.");
    op->add_descr("C,Z = OUT bit.");
}

/**
 * @brief OUT bit of pin D[5:0] = !C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001011011
 *
 * DRVNC   {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 *
 */
void P2Doc::doc_DRVNC(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZL DDDDDDDDD 001011011");

    op->set_token(t_DRVNC);
    op->set_brief("OUT bit of pin D[5:0] = !C.");
    op->set_instr("DRVNC   {#}D           {WCZ}");
    op->add_descr("DIR bit = 1.");
    op->add_descr("C,Z = OUT bit.");
}

/**
 * @brief OUT bit of pin D[5:0] = Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001011100
 *
 * DRVZ    {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 *
 */
void P2Doc::doc_DRVZ(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZL DDDDDDDDD 001011100");

    op->set_token(t_DRVZ);
    op->set_brief("OUT bit of pin D[5:0] = Z.");
    op->set_instr("DRVZ    {#}D           {WCZ}");
    op->add_descr("DIR bit = 1.");
    op->add_descr("C,Z = OUT bit.");
}

/**
 * @brief OUT bit of pin D[5:0] = !Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001011101
 *
 * DRVNZ   {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 *
 */
void P2Doc::doc_DRVNZ(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZL DDDDDDDDD 001011101");

    op->set_token(t_DRVNZ);
    op->set_brief("OUT bit of pin D[5:0] = !Z.");
    op->set_instr("DRVNZ   {#}D           {WCZ}");
    op->add_descr("DIR bit = 1.");
    op->add_descr("C,Z = OUT bit.");
}

/**
 * @brief OUT bit of pin D[5:0] = RND.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001011110
 *
 * DRVRND  {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 *
 */
void P2Doc::doc_DRVRND(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZL DDDDDDDDD 001011110");

    op->set_token(t_DRVRND);
    op->set_brief("OUT bit of pin D[5:0] = RND.");
    op->set_instr("DRVRND  {#}D           {WCZ}");
    op->add_descr("DIR bit = 1.");
    op->add_descr("C,Z = OUT bit.");
}

/**
 * @brief OUT bit of pin D[5:0] = !bit.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001011111
 *
 * DRVNOT  {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 *
 */
void P2Doc::doc_DRVNOT(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZL DDDDDDDDD 001011111");

    op->set_token(t_DRVNOT);
    op->set_brief("OUT bit of pin D[5:0] = !bit.");
    op->set_instr("DRVNOT  {#}D           {WCZ}");
    op->add_descr("DIR bit = 1.");
    op->add_descr("C,Z = OUT bit.");
}

/**
 * @brief Split every 4th bit of S into bytes of D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100000
 *
 * SPLITB  D
 *
 * D = {S[31], S[27], S[23], S[19], ... S[12], S[8], S[4], S[0]}.
 *
 */
void P2Doc::doc_SPLITB(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 DDDDDDDDD 001100000");

    op->set_token(t_SPLITB);
    op->set_brief("Split every 4th bit of S into bytes of D.");
    op->set_instr("SPLITB  D");
    op->add_descr("D = {S[31], S[27], S[23], S[19], ... S[12], S[8], S[4], S[0]}.");
}

/**
 * @brief Merge bits of bytes in S into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100001
 *
 * MERGEB  D
 *
 * D = {S[31], S[23], S[15], S[7], ... S[24], S[16], S[8], S[0]}.
 *
 */
void P2Doc::doc_MERGEB(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 DDDDDDDDD 001100001");

    op->set_token(t_MERGEB);
    op->set_brief("Merge bits of bytes in S into D.");
    op->set_instr("MERGEB  D");
    op->add_descr("D = {S[31], S[23], S[15], S[7], ... S[24], S[16], S[8], S[0]}.");
}

/**
 * @brief Split bits of S into words of D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100010
 *
 * SPLITW  D
 *
 * D = {S[31], S[29], S[27], S[25], ... S[6], S[4], S[2], S[0]}.
 *
 */
void P2Doc::doc_SPLITW(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 DDDDDDDDD 001100010");

    op->set_token(t_SPLITW);
    op->set_brief("Split bits of S into words of D.");
    op->set_instr("SPLITW  D");
    op->add_descr("D = {S[31], S[29], S[27], S[25], ... S[6], S[4], S[2], S[0]}.");
}

/**
 * @brief Merge bits of words in S into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100011
 *
 * MERGEW  D
 *
 * D = {S[31], S[15], S[30], S[14], ... S[17], S[1], S[16], S[0]}.
 *
 */
void P2Doc::doc_MERGEW(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 DDDDDDDDD 001100011");

    op->set_token(t_MERGEW);
    op->set_brief("Merge bits of words in S into D.");
    op->set_instr("MERGEW  D");
    op->add_descr("D = {S[31], S[15], S[30], S[14], ... S[17], S[1], S[16], S[0]}.");
}

/**
 * @brief Relocate and periodically invert bits from S into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100100
 *
 * SEUSSF  D
 *
 * Returns to original value on 32nd iteration.
 * Forward pattern.
 */
void P2Doc::doc_SEUSSF(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 DDDDDDDDD 001100100");

    op->set_token(t_SEUSSF);
    op->set_brief("Relocate and periodically invert bits from S into D.");
    op->set_instr("SEUSSF  D");
    op->add_descr("Returns to original value on 32nd iteration.");
    op->add_descr("Forward pattern.");
}

/**
 * @brief Relocate and periodically invert bits from S into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100101
 *
 * SEUSSR  D
 *
 * Returns to original value on 32nd iteration.
 * Reverse pattern.
 */
void P2Doc::doc_SEUSSR(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 DDDDDDDDD 001100101");

    op->set_token(t_SEUSSR);
    op->set_brief("Relocate and periodically invert bits from S into D.");
    op->set_instr("SEUSSR  D");
    op->add_descr("Returns to original value on 32nd iteration.");
    op->add_descr("Reverse pattern.");
}

/**
 * @brief Squeeze 8:8:8 RGB value in S[31:8] into 5:6:5 value in D[15:0].
 *
 * EEEE 1101011 000 DDDDDDDDD 001100110
 *
 * RGBSQZ  D
 *
 * D = {15'b0, S[31:27], S[23:18], S[15:11]}.
 *
 */
void P2Doc::doc_RGBSQZ(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 DDDDDDDDD 001100110");

    op->set_token(t_RGBSQZ);
    op->set_brief("Squeeze 8:8:8 RGB value in S[31:8] into 5:6:5 value in D[15:0].");
    op->set_instr("RGBSQZ  D");
    op->add_descr("D = {15'b0, S[31:27], S[23:18], S[15:11]}.");
}

/**
 * @brief Expand 5:6:5 RGB value in S[15:0] into 8:8:8 value in D[31:8].
 *
 * EEEE 1101011 000 DDDDDDDDD 001100111
 *
 * RGBEXP  D
 *
 * D = {S[15:11,15:13], S[10:5,10:9], S[4:0,4:2], 8'b0}.
 *
 */
void P2Doc::doc_RGBEXP(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 DDDDDDDDD 001100111");

    op->set_token(t_RGBEXP);
    op->set_brief("Expand 5:6:5 RGB value in S[15:0] into 8:8:8 value in D[31:8].");
    op->set_instr("RGBEXP  D");
    op->add_descr("D = {S[15:11,15:13], S[10:5,10:9], S[4:0,4:2], 8'b0}.");
}

/**
 * @brief Iterate D with xoroshiro32+ PRNG algorithm and put PRNG result into next instruction's S.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101000
 *
 * XORO32  D
 *
 *
 */
void P2Doc::doc_XORO32(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 DDDDDDDDD 001101000");

    op->set_token(t_XORO32);
    op->set_brief("Iterate D with xoroshiro32+ PRNG algorithm and put PRNG result into next instruction's S.");
    op->set_instr("XORO32  D");
}

/**
 * @brief Reverse D bits.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101001
 *
 * REV     D
 *
 * D = D[0:31].
 *
 */
void P2Doc::doc_REV(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 DDDDDDDDD 001101001");

    op->set_token(t_REV);
    op->set_brief("Reverse D bits.");
    op->set_instr("REV     D");
    op->add_descr("D = D[0:31].");
}

/**
 * @brief Rotate C,Z right through D.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 001101010
 *
 * RCZR    D        {WC/WZ/WCZ}
 *
 * D = {C, Z, D[31:2]}.
 * C = D[1],  Z = D[0].
 *
 */
void P2Doc::doc_RCZR(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 DDDDDDDDD 001101010");

    op->set_token(t_RCZR);
    op->set_brief("Rotate C,Z right through D.");
    op->set_instr("RCZR    D        {WC/WZ/WCZ}");
    op->add_descr("D = {C, Z, D[31:2]}.");
    op->add_descr("C = D[1],  Z = D[0].");
}

/**
 * @brief Rotate C,Z left through D.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 001101011
 *
 * RCZL    D        {WC/WZ/WCZ}
 *
 * D = {D[29:0], C, Z}.
 * C = D[31], Z = D[30].
 *
 */
void P2Doc::doc_RCZL(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ0 DDDDDDDDD 001101011");

    op->set_token(t_RCZL);
    op->set_brief("Rotate C,Z left through D.");
    op->set_instr("RCZL    D        {WC/WZ/WCZ}");
    op->add_descr("D = {D[29:0], C, Z}.");
    op->add_descr("C = D[31], Z = D[30].");
}

/**
 * @brief Write 0 or 1 to D, according to  C.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101100
 *
 * WRC     D
 *
 * D = {31'b0,  C}.
 *
 */
void P2Doc::doc_WRC(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 DDDDDDDDD 001101100", match_CZI);

    op->set_token(t_WRC);
    op->set_brief("Write 0 or 1 to D, according to  C.");
    op->set_instr("WRC     D");
    op->add_descr("D = {31'b0,  C}.");
}

/**
 * @brief Write 0 or 1 to D, according to !C.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101101
 *
 * WRNC    D
 *
 * D = {31'b0, !C}.
 *
 */
void P2Doc::doc_WRNC(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 DDDDDDDDD 001101101", match_CZI);

    op->set_token(t_WRNC);
    op->set_brief("Write 0 or 1 to D, according to !C.");
    op->set_instr("WRNC    D");
    op->add_descr("D = {31'b0, !C}.");
}

/**
 * @brief Write 0 or 1 to D, according to  Z.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101110
 *
 * WRZ     D
 *
 * D = {31'b0,  Z}.
 *
 */
void P2Doc::doc_WRZ(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 DDDDDDDDD 001101110", match_CZI);

    op->set_token(t_WRZ);
    op->set_brief("Write 0 or 1 to D, according to  Z.");
    op->set_instr("WRZ     D");
    op->add_descr("D = {31'b0,  Z}.");
}

/**
 * @brief Write 0 or 1 to D, according to !Z.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101111
 *
 * WRNZ    D
 *
 * D = {31'b0, !Z}.
 *
 */
void P2Doc::doc_WRNZ(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 DDDDDDDDD 001101111", match_CZI);

    op->set_token(t_WRNZ);
    op->set_brief("Write 0 or 1 to D, according to !Z.");
    op->set_instr("WRNZ    D");
    op->add_descr("D = {31'b0, !Z}.");
}

/**
 * @brief Modify C and Z according to cccc and zzzz.
 *
 * EEEE 1101011 CZ1 0cccczzzz 001101111
 *
 * MODCZ   c,z      {WC/WZ/WCZ}
 *
 * C = cccc[{C,Z}], Z = zzzz[{C,Z}].
 *
 */
void P2Doc::doc_MODCZ(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 CZ1 0cccczzzz 001101111", match_I);

    op->set_token(t_MODCZ);
    op->set_brief("Modify C and Z according to cccc and zzzz.");
    op->set_instr("MODCZ   c,z      {WC/WZ/WCZ}");
    op->add_descr("C = cccc[{C,Z}], Z = zzzz[{C,Z}].");
}

/**
 * @brief Set scope mode.
 *
 * EEEE 1101011 00L DDDDDDDDD 001110000
 *
 * SETSCP  {#}D
 *
 * Pins D[5:2], enable D[6].
 *
 * SETSCP points the scope mux to a set of four pins starting
 * at (D[5:0] AND $3C), with D[6]=1 to enable scope operation.
 *
 */
void P2Doc::doc_SETSCP(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 00L DDDDDDDDD 001110000");

    op->set_token(t_SETSCP);
    op->set_brief("Set scope mode.");
    op->set_instr("SETSCP  {#}D");
    op->add_descr("Pins D[5:2], enable D[6].");
    op->add_descr("SETSCP points the scope mux to a set of four pins starting");
    op->add_descr("at (D[5:0] AND $3C), with D[6]=1 to enable scope operation.");
}

/**
 * @brief Get scope values.
 *
 * EEEE 1101011 000 DDDDDDDDD 001110001
 *
 * GETSCP  D
 *
 * Any time GETSCP is executed, the lower bytes of those four pins' RDPIN values are returned in D.
 * This feature will mainly be useful on the next silicon, as the FPGAs don't have ADC-capable pins.
 *
 */
void P2Doc::doc_GETSCP(p2_OPSRC_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101011 000 DDDDDDDDD 001110001");

    op->set_token(t_GETSCP);
    op->set_brief("Get four scope values into bytes of D.");
    op->set_instr("GETSCP  D");
    op->add_descr("Any time GETSCP is executed, the lower bytes of those");
    op->add_descr("four pins' RDPIN values are returned in D.");
}

/**
 * @brief Jump to A.
 *
 * EEEE 1101100 RAA AAAAAAAAA AAAAAAAAA
 *
 * JMP     #A
 *
 * If R = 1, PC += A, else PC = A.
 *
 */
void P2Doc::doc_JMP_abs(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101100 RAA AAAAAAAAA AAAAAAAAA");

    op->set_token(t_JMP);
    op->set_brief("Jump to A.");
    op->set_instr("JMP     #A");
    op->add_descr("If R = 1, PC += A, else PC = A.");
}

/**
 * @brief Call to A by pushing {C, Z, 10'b0, PC[19:0]} onto stack.
 *
 * EEEE 1101101 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALL    #A
 *
 * If R = 1, PC += A, else PC = A.
 *
 */
void P2Doc::doc_CALL_abs(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101101 RAA AAAAAAAAA AAAAAAAAA");

    op->set_token(t_CALL);
    op->set_brief("Call to A by pushing {C, Z, 10'b0, PC[19:0]} onto stack.");
    op->set_instr("CALL    #A");
    op->add_descr("If R = 1, PC += A, else PC = A.");
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRA++.
 *
 * EEEE 1101110 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLA   #A
 *
 * If R = 1, PC += A, else PC = A.
 *
 */
void P2Doc::doc_CALLA_abs(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101110 RAA AAAAAAAAA AAAAAAAAA");

    op->set_token(t_CALLA);
    op->set_brief("Call to A by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRA++.");
    op->set_instr("CALLA   #A");
    op->add_descr("If R = 1, PC += A, else PC = A.");
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRB++.
 *
 * EEEE 1101111 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLB   #A
 *
 * If R = 1, PC += A, else PC = A.
 *
 */
void P2Doc::doc_CALLB_abs(p2_INST7_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 1101111 RAA AAAAAAAAA AAAAAAAAA");

    op->set_token(t_CALLB);
    op->set_brief("Call to A by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRB++.");
    op->set_instr("CALLB   #A");
    op->add_descr("If R = 1, PC += A, else PC = A.");
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 11100WW RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PA,#A
 *
 * If R = 1, PC += A, else PC = A.
 *
 */
void P2Doc::doc_CALLD_abs(p2_INST5_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 11100WW RAA AAAAAAAAA AAAAAAAAA");

    op->set_token(t_CALLD);
    op->set_brief("Call to A by writing {C, Z, 10'b0, PC[19:0]} to PA (per W).");
    op->set_instr("CALLD   PA/PB/PTRA/PTRB,#A");
    op->add_descr("If R = 1, PC += A, else PC = A.");
}

/**
 * @brief Get {12'b0, address[19:0]} into PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 11101WW RAA AAAAAAAAA AAAAAAAAA
 *
 * LOC     PA/PB/PTRA/PTRB,#A
 *
 * If R = 1, address = PC + A, else address = A.
 *
 */
void P2Doc::doc_LOC(p2_INST5_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 11101WW RAA AAAAAAAAA AAAAAAAAA");

    op->set_token(t_LOC);
    op->set_brief("Get {12'b0, address[19:0]} into PA/PB/PTRA/PTRB (per W).");
    op->set_instr("LOC     PA/PB/PTRA/PTRB,#A");
    op->add_descr("If R = 1, address = PC + A, else address = A.");
}

/**
 * @brief Queue #N[31:9] to be used as upper 23 bits for next #S occurrence, so that the next 9-bit #S will be augmented to 32 bits.
 *
 * EEEE 11110NN NNN NNNNNNNNN NNNNNNNNN
 *
 * AUGS    #N
 *
 */
void P2Doc::doc_AUGS(p2_INST5_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 11110NN NNN NNNNNNNNN NNNNNNNNN");

    op->set_token(t_AUGS);
    op->set_brief("Augment next #S occurrence with 23 bits #N[31:9].");
    op->set_instr("AUGS    #N");
    op->add_descr("Queue #N[31:9] to be used as upper 23 bits for next #S occurrence,");
    op->add_descr("so that the next 9-bit #S will be augmented to 32 bits.");
}

/**
 * @brief Queue #N[31:9] to be used as upper 23 bits for next #D occurrence, so that the next 9-bit #D will be augmented to 32 bits.
 *
 * EEEE 11111NN NNN NNNNNNNNN NNNNNNNNN
 *
 * AUGD    #N
 *
 */
void P2Doc::doc_AUGD(p2_INST5_e instr)
{
    P2DocOpcode op = make_pattern(__func__, instr, "EEEE 11111NN NNN NNNNNNNNN NNNNNNNNN");

    op->set_token(t_AUGD);
    op->set_brief("Augment next #D occurrence with 23 bits #N[31:9].");
    op->set_instr("AUGD    #N");
    op->add_descr("Queue #N[31:9] to be used as upper 23 bits for next D occurrence,");
    op->add_descr("so that the next 9-bit #D will be augmented to 32 bits.");
}
