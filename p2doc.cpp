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

// Global instance
P2Doc Doc;

P2Doc::P2Doc(QObject* parent)
    : QObject(parent)
    , m_pattern()
    , m_brief()
    , m_instr()
    , m_token()
{
    doc_nop(p2_ROR);
    doc_ror(p2_ROR);
    doc_rol(p2_ROL);
    doc_shr(p2_SHR);
    doc_shl(p2_SHL);
    doc_rcr(p2_RCR);
    doc_rcl(p2_RCL);
    doc_sar(p2_SAR);
    doc_sal(p2_SAL);
    doc_add(p2_ADD);
    doc_addx(p2_ADDX);
    doc_adds(p2_ADDS);
    doc_addsx(p2_ADDSX);
    doc_sub(p2_SUB);
    doc_subx(p2_SUBX);
    doc_subs(p2_SUBS);
    doc_subsx(p2_SUBSX);
    doc_cmp(p2_CMP);
    doc_cmpx(p2_CMPX);
    doc_cmps(p2_CMPS);
    doc_cmpsx(p2_CMPSX);
    doc_cmpr(p2_CMPR);
    doc_cmpm(p2_CMPM);
    doc_subr(p2_SUBR);
    doc_cmpsub(p2_CMPSUB);
    doc_fge(p2_FGE);
    doc_fle(p2_FLE);
    doc_fges(p2_FGES);
    doc_fles(p2_FLES);
    doc_sumc(p2_SUMC);
    doc_sumnc(p2_SUMNC);
    doc_sumz(p2_SUMZ);
    doc_sumnz(p2_SUMNZ);

    doc_testb_w(p2_TESTB_WC);
    doc_testb_w(p2_TESTB_WZ);
    doc_testbn_w(p2_TESTBN_WZ);
    doc_testbn_w(p2_TESTBN_WC);
    doc_testb_and(p2_TESTB_ANDZ);
    doc_testb_and(p2_TESTB_ANDC);
    doc_testbn_and(p2_TESTBN_ANDZ);
    doc_testbn_and(p2_TESTBN_ANDC);
    doc_testb_or(p2_TESTB_ORC);
    doc_testb_or(p2_TESTB_ORZ);
    doc_testbn_or(p2_TESTBN_ORC);
    doc_testbn_or(p2_TESTBN_ORZ);
    doc_testb_xor(p2_TESTB_XORC);
    doc_testb_xor(p2_TESTB_XORZ);
    doc_testbn_xor(p2_TESTBN_XORC);
    doc_testbn_xor(p2_TESTBN_XORZ);
    doc_bitl(p2_BITL_eol);
    doc_bitl(p2_BITL_WCZ);
    doc_bith(p2_BITH_eol);
    doc_bith(p2_BITH_WCZ);
    doc_bitc(p2_BITC_eol);
    doc_bitc(p2_BITC_WCZ);
    doc_bitnc(p2_BITNC_eol);
    doc_bitnc(p2_BITNC_WCZ);
    doc_bitz(p2_BITZ_eol);
    doc_bitz(p2_BITZ_WCZ);
    doc_bitnz(p2_BITNZ_eol);
    doc_bitnz(p2_BITNZ_WCZ);
    doc_bitrnd(p2_BITRND_eol);
    doc_bitrnd(p2_BITRND_WCZ);
    doc_bitnot(p2_BITNOT_eol);
    doc_bitnot(p2_BITNOT_WCZ);

    doc_and(p2_AND);
    doc_andn(p2_ANDN);
    doc_or(p2_OR);
    doc_xor(p2_XOR);
    doc_muxc(p2_MUXC);
    doc_muxnc(p2_MUXNC);
    doc_muxz(p2_MUXZ);
    doc_muxnz(p2_MUXNZ);
    doc_mov(p2_MOV);
    doc_not(p2_NOT);
    doc_abs(p2_ABS);
    doc_neg(p2_NEG);
    doc_negc(p2_NEGC);

    doc_negnc(p2_NEGNC);
    doc_negz(p2_NEGZ);
    doc_negnz(p2_NEGNZ);
    doc_incmod(p2_INCMOD);
    doc_decmod(p2_DECMOD);
    doc_zerox(p2_ZEROX);
    doc_signx(p2_SIGNX);
    doc_encod(p2_ENCOD);
    doc_ones(p2_ONES);

    doc_test(p2_TEST);
    doc_testn(p2_TESTN);
    doc_setnib(p2_SETNIB_0);
    doc_setnib(p2_SETNIB_1);
    doc_getnib(p2_GETNIB_0);
    doc_getnib(p2_GETNIB_1);
    doc_rolnib(p2_ROLNIB_0);
    doc_rolnib(p2_ROLNIB_1);
    doc_setbyte(p2_SETBYTE);
    doc_getbyte(p2_GETBYTE);
    doc_rolbyte(p2_ROLBYTE);

    doc_setword_altsw(p2_SETWORD_ALTSW);
    doc_setword(p2_SETWORD);
    doc_getword_altgw(p2_GETWORD_ALTGW);
    doc_getword(p2_GETWORD);

    doc_rolword_altgw(p2_ROLWORD_ALTGW);
    doc_rolword(p2_ROLWORD_ALTGW);
    doc_rolword(p2_ROLWORD);
    doc_altsn_d(p2_ALTSN);
    doc_altsn(p2_ALTSN);
    doc_altgn_d(p2_ALTGN);
    doc_altgn(p2_ALTGN);

    doc_altsb_d(p2_ALTSB);
    doc_altsb(p2_ALTSB);
    doc_altgb_d(p2_ALTGB);
    doc_altgb(p2_ALTGB);
    doc_altsw_d(p2_ALTSW);
    doc_altsw(p2_ALTSW);
    doc_altgw_d(p2_ALTGW);
    doc_altgw(p2_ALTGW);

    doc_altr_d(p2_ALTR);
    doc_altr(p2_ALTR);
    doc_altd_d(p2_ALTD);
    doc_altd(p2_ALTD);
    doc_alts_d(p2_ALTS);
    doc_alts(p2_ALTS);
    doc_altb_d(p2_ALTB);
    doc_altb(p2_ALTB);

    doc_alti_d(p2_ALTI);
    doc_alti(p2_ALTI);
    doc_setr(p2_SETR);
    doc_setd(p2_SETD);
    doc_sets(p2_SETS);

    doc_decod_d(p2_DECOD);
    doc_decod(p2_DECOD);
    doc_bmask_d(p2_BMASK);
    doc_bmask(p2_BMASK);
    doc_crcbit(p2_CRCBIT);
    doc_crcnib(p2_CRCNIB);

    doc_muxnits(p2_MUXNITS);
    doc_muxnibs(p2_MUXNIBS);
    doc_muxq(p2_MUXQ);
    doc_movbyts(p2_MOVBYTS);

    doc_mul(p2_MUL);
    doc_muls(p2_MULS);

    doc_sca(p2_SCA);
    doc_scas(p2_SCAS);

    doc_addpix(p2_ADDPIX);
    doc_mulpix(p2_MULPIX);
    doc_blnpix(p2_BLNPIX);
    doc_mixpix(p2_MIXPIX);

    doc_addct1(p2_ADDCT1);
    doc_addct2(p2_ADDCT2);
    doc_addct3(p2_ADDCT3);
    doc_wmlong(p2_WMLONG);

    doc_rqpin(p2_RQPIN);
    doc_rdpin(p2_RDPIN);

    doc_rdlut(p2_RDLUT);
    doc_rdbyte(p2_RDBYTE);
    doc_rdword(p2_RDWORD);

    doc_rdlong(p2_RDLONG);

    doc_calld(p2_CALLD);

    doc_callpa(p2_CALLPA);
    doc_callpb(p2_CALLPB);

    doc_djz(p2_DJZ);
    doc_djnz(p2_DJNZ);
    doc_djf(p2_DJF);
    doc_djnf(p2_DJNF);
    doc_ijz(p2_IJZ);
    doc_ijnz(p2_IJNZ);
    doc_tjz(p2_TJZ);
    doc_tjnz(p2_TJNZ);
    doc_tjf(p2_TJF);
    doc_tjnf(p2_TJNF);
    doc_tjs(p2_TJS);
    doc_tjns(p2_TJNS);
    doc_tjv(p2_TJV);

    doc_jint(p2_OPDST_JINT);
    doc_jct1(p2_OPDST_JCT1);
    doc_jct2(p2_OPDST_JCT2);
    doc_jct3(p2_OPDST_JCT3);
    doc_jse1(p2_OPDST_JSE1);
    doc_jse2(p2_OPDST_JSE2);
    doc_jse3(p2_OPDST_JSE3);
    doc_jse4(p2_OPDST_JSE4);
    doc_jpat(p2_OPDST_JPAT);
    doc_jfbw(p2_OPDST_JFBW);
    doc_jxmt(p2_OPDST_JXMT);
    doc_jxfi(p2_OPDST_JXFI);
    doc_jxro(p2_OPDST_JXRO);
    doc_jxrl(p2_OPDST_JXRL);
    doc_jatn(p2_OPDST_JATN);
    doc_jqmt(p2_OPDST_JQMT);
    doc_jnint(p2_OPDST_JNINT);
    doc_jnct1(p2_OPDST_JNCT1);
    doc_jnct2(p2_OPDST_JNCT2);
    doc_jnct3(p2_OPDST_JNCT3);
    doc_jnse1(p2_OPDST_JNSE1);
    doc_jnse2(p2_OPDST_JNSE2);
    doc_jnse3(p2_OPDST_JNSE3);
    doc_jnse4(p2_OPDST_JNSE4);
    doc_jnpat(p2_OPDST_JNPAT);
    doc_jnfbw(p2_OPDST_JNFBW);
    doc_jnxmt(p2_OPDST_JNXMT);
    doc_jnxfi(p2_OPDST_JNXFI);
    doc_jnxro(p2_OPDST_JNXRO);
    doc_jnxrl(p2_OPDST_JNXRL);
    doc_jnatn(p2_OPDST_JNATN);
    doc_jnqmt(p2_OPDST_JNQMT);
    doc_1011110_1(p2_1011110_1);
    // doc_1011110_1(p2_1011110_11);

    doc_1011111_0(p2_1011111_0);
    doc_setpat(p2_SETPAT);

    doc_akpin(p2_WRPIN);
    doc_wrpin(p2_WRPIN);
    doc_wxpin(p2_WXPIN);
    doc_wypin(p2_WYPIN);
    doc_wrlut(p2_WRLUT);
    doc_wrbyte(p2_WRBYTE);
    doc_wrword(p2_WRWORD);
    doc_wrlong(p2_WRLONG);
    doc_rdfast(p2_RDFAST);
    doc_wrfast(p2_WRFAST);
    doc_fblock(p2_FBLOCK);
    doc_xstop(p2_XINIT);
    doc_xinit(p2_XINIT);
    doc_xzero(p2_XZERO);
    doc_xcont(p2_XCONT);
    doc_rep(p2_REP);
    doc_coginit(p2_COGINIT);
    doc_qmul(p2_QMUL);
    doc_qdiv(p2_QDIV);
    doc_qfrac(p2_QFRAC);
    doc_qsqrt(p2_QSQRT);
    doc_qrotate(p2_QROTATE);
    doc_qvector(p2_QVECTOR);

    doc_hubset(p2_OPSRC_HUBSET);
    doc_cogid(p2_OPSRC_COGID);
    doc_cogstop(p2_OPSRC_COGSTOP);
    doc_locknew(p2_OPSRC_LOCKNEW);
    doc_lockret(p2_OPSRC_LOCKRET);
    doc_locktry(p2_OPSRC_LOCKTRY);
    doc_lockrel(p2_OPSRC_LOCKREL);
    doc_qlog(p2_OPSRC_QLOG);
    doc_qexp(p2_OPSRC_QEXP);
    doc_rfbyte(p2_OPSRC_RFBYTE);
    doc_rfword(p2_OPSRC_RFWORD);
    doc_rflong(p2_OPSRC_RFLONG);
    doc_rfvar(p2_OPSRC_RFVAR);
    doc_rfvars(p2_OPSRC_RFVARS);
    doc_wfbyte(p2_OPSRC_WFBYTE);
    doc_wfword(p2_OPSRC_WFWORD);
    doc_wflong(p2_OPSRC_WFLONG);
    doc_getqx(p2_OPSRC_GETQX);
    doc_getqy(p2_OPSRC_GETQY);
    doc_getct(p2_OPSRC_GETCT);
    doc_getrnd_cz(p2_OPSRC_GETRND);
    doc_getrnd(p2_OPSRC_GETRND);
    doc_setdacs(p2_OPSRC_SETDACS);
    doc_setxfrq(p2_OPSRC_SETXFRQ);
    doc_getxacc(p2_OPSRC_GETXACC);
    doc_waitx(p2_OPSRC_WAITX);
    doc_setse1(p2_OPSRC_SETSE1);
    doc_setse2(p2_OPSRC_SETSE2);
    doc_setse3(p2_OPSRC_SETSE3);
    doc_setse4(p2_OPSRC_SETSE4);

    doc_pollint(p2_OPX24_POLLINT);
    doc_pollct1(p2_OPX24_POLLCT1);
    doc_pollct2(p2_OPX24_POLLCT2);
    doc_pollct3(p2_OPX24_POLLCT3);
    doc_pollse1(p2_OPX24_POLLSE1);
    doc_pollse2(p2_OPX24_POLLSE2);
    doc_pollse3(p2_OPX24_POLLSE3);
    doc_pollse4(p2_OPX24_POLLSE4);
    doc_pollpat(p2_OPX24_POLLPAT);
    doc_pollfbw(p2_OPX24_POLLFBW);
    doc_pollxmt(p2_OPX24_POLLXMT);
    doc_pollxfi(p2_OPX24_POLLXFI);
    doc_pollxro(p2_OPX24_POLLXRO);
    doc_pollxrl(p2_OPX24_POLLXRL);
    doc_pollatn(p2_OPX24_POLLATN);
    doc_pollqmt(p2_OPX24_POLLQMT);
    doc_waitint(p2_OPX24_WAITINT);
    doc_waitct1(p2_OPX24_WAITCT1);
    doc_waitct2(p2_OPX24_WAITCT2);
    doc_waitct3(p2_OPX24_WAITCT3);
    doc_waitse1(p2_OPX24_WAITSE1);
    doc_waitse2(p2_OPX24_WAITSE2);
    doc_waitse3(p2_OPX24_WAITSE3);
    doc_waitse4(p2_OPX24_WAITSE4);
    doc_waitpat(p2_OPX24_WAITPAT);
    doc_waitfbw(p2_OPX24_WAITFBW);
    doc_waitxmt(p2_OPX24_WAITXMT);
    doc_waitxfi(p2_OPX24_WAITXFI);
    doc_waitxro(p2_OPX24_WAITXRO);
    doc_waitxrl(p2_OPX24_WAITXRL);
    doc_waitatn(p2_OPX24_WAITATN);
    doc_allowi(p2_OPX24_ALLOWI);
    doc_stalli(p2_OPX24_STALLI);
    doc_trgint1(p2_OPX24_TRGINT1);
    doc_trgint2(p2_OPX24_TRGINT2);
    doc_trgint3(p2_OPX24_TRGINT3);
    doc_nixint1(p2_OPX24_NIXINT1);
    doc_nixint2(p2_OPX24_NIXINT2);
    doc_nixint3(p2_OPX24_NIXINT3);

    doc_setint1(p2_OPSRC_SETINT1);
    doc_setint2(p2_OPSRC_SETINT2);
    doc_setint3(p2_OPSRC_SETINT3);
    doc_setq(p2_OPSRC_SETQ);
    doc_setq2(p2_OPSRC_SETQ2);
    doc_push(p2_OPSRC_PUSH);
    doc_pop(p2_OPSRC_POP);
    doc_jmp(p2_OPSRC_JMP);
    doc_call(p2_OPSRC_CALL_RET);
    doc_ret(p2_OPSRC_CALL_RET);
    doc_calla(p2_OPSRC_CALLA_RETA);
    doc_reta(p2_OPSRC_CALLA_RETA);
    doc_callb(p2_OPSRC_CALLB_RETB);
    doc_retb(p2_OPSRC_CALLB_RETB);
    doc_jmprel(p2_OPSRC_JMPREL);
    doc_skip(p2_OPSRC_SKIP);
    doc_skipf(p2_OPSRC_SKIPF);
    doc_execf(p2_OPSRC_EXECF);
    doc_getptr(p2_OPSRC_GETPTR);
    doc_cogbrk(p2_OPSRC_COGBRK);
    doc_getbrk(p2_OPSRC_COGBRK);
    doc_brk(p2_OPSRC_BRK);
    doc_setluts(p2_OPSRC_SETLUTS);
    doc_setcy(p2_OPSRC_SETCY);
    doc_setci(p2_OPSRC_SETCI);
    doc_setcq(p2_OPSRC_SETCQ);
    doc_setcfrq(p2_OPSRC_SETCFRQ);
    doc_setcmod(p2_OPSRC_SETCMOD);
    doc_setpiv(p2_OPSRC_SETPIV);
    doc_setpix(p2_OPSRC_SETPIX);
    doc_cogatn(p2_OPSRC_COGATN);
    doc_testp_w(p2_OPSRC_TESTP_W_DIRL);
    doc_dirl(p2_OPSRC_TESTP_W_DIRL);
    doc_testpn_w(p2_OPSRC_TESTPN_W_DIRH);
    doc_dirh(p2_OPSRC_TESTPN_W_DIRH);
    doc_testp_and(p2_OPSRC_TESTP_AND_DIRC);
    doc_dirc(p2_OPSRC_TESTP_AND_DIRC);
    doc_testpn_and(p2_OPSRC_TESTPN_AND_DIRNC);
    doc_dirnc(p2_OPSRC_TESTPN_AND_DIRNC);
    doc_testp_or(p2_OPSRC_TESTP_OR_DIRZ);
    doc_dirz(p2_OPSRC_TESTP_OR_DIRZ);
    doc_testpn_or(p2_OPSRC_TESTPN_OR_DIRNZ);
    doc_dirnz(p2_OPSRC_TESTPN_OR_DIRNZ);
    doc_testp_xor(p2_OPSRC_TESTP_XOR_DIRRND);
    doc_dirrnd(p2_OPSRC_TESTP_XOR_DIRRND);
    doc_testpn_xor(p2_OPSRC_TESTPN_XOR_DIRNOT);
    doc_dirnot(p2_OPSRC_TESTPN_XOR_DIRNOT);

    doc_outl(p2_OPSRC_OUTL);
    doc_outh(p2_OPSRC_OUTH);
    doc_outc(p2_OPSRC_OUTC);
    doc_outnc(p2_OPSRC_OUTNC);
    doc_outz(p2_OPSRC_OUTZ);
    doc_outnz(p2_OPSRC_OUTNZ);
    doc_outrnd(p2_OPSRC_OUTRND);
    doc_outnot(p2_OPSRC_OUTNOT);
    doc_fltl(p2_OPSRC_FLTL);
    doc_flth(p2_OPSRC_FLTH);
    doc_fltc(p2_OPSRC_FLTC);
    doc_fltnc(p2_OPSRC_FLTNC);
    doc_fltz(p2_OPSRC_FLTZ);
    doc_fltnz(p2_OPSRC_FLTNZ);
    doc_fltrnd(p2_OPSRC_FLTRND);
    doc_fltnot(p2_OPSRC_FLTNOT);
    doc_drvl(p2_OPSRC_DRVL);
    doc_drvh(p2_OPSRC_DRVH);
    doc_drvc(p2_OPSRC_DRVC);
    doc_drvnc(p2_OPSRC_DRVNC);
    doc_drvz(p2_OPSRC_DRVZ);
    doc_drvnz(p2_OPSRC_DRVNZ);
    doc_drvrnd(p2_OPSRC_DRVRND);
    doc_drvnot(p2_OPSRC_DRVNOT);
    doc_splitb(p2_OPSRC_SPLITB);
    doc_mergeb(p2_OPSRC_MERGEB);
    doc_splitw(p2_OPSRC_SPLITW);
    doc_mergew(p2_OPSRC_MERGEW);
    doc_seussf(p2_OPSRC_SEUSSF);
    doc_seussr(p2_OPSRC_SEUSSR);
    doc_rgbsqz(p2_OPSRC_RGBSQZ);
    doc_rgbexp(p2_OPSRC_RGBEXP);
    doc_xoro32(p2_OPSRC_XORO32);
    doc_rev(p2_OPSRC_REV);
    doc_rczr(p2_OPSRC_RCZR);
    doc_rczl(p2_OPSRC_RCZL);
    doc_wrc(p2_OPSRC_WRC);
    doc_wrnc(p2_OPSRC_WRNC);
    doc_wrz(p2_OPSRC_WRZ);
    doc_modcz(p2_OPSRC_WRNZ_MODCZ);
    doc_wrnz(p2_OPSRC_WRNZ_MODCZ);
    doc_setscp(p2_OPSRC_SETSCP);
    doc_getscp(p2_OPSRC_GETSCP);

    doc_jmp_abs(p2_JMP_ABS);
    doc_call_abs(p2_CALL_ABS);
    doc_calla_abs(p2_CALLA_ABS);
    doc_callb_abs(p2_CALLB_ABS);
    doc_calld_pa_abs(p2_CALLD_PA_ABS);
    doc_calld_pb_abs(p2_CALLD_PB_ABS);
    doc_calld_ptra_abs(p2_CALLD_PTRA_ABS);
    doc_calld_ptrb_abs(p2_CALLD_PTRB_ABS);

    doc_loc_pa(p2_LOC_PA);
    doc_loc_pb(p2_LOC_PB);
    doc_loc_ptra(p2_LOC_PTRA);
    doc_loc_ptrb(p2_LOC_PTRB);

    doc_augs(p2_AUGS);
    doc_augs(p2_AUGS_01);
    doc_augs(p2_AUGS_10);
    doc_augs(p2_AUGS_11);

    doc_augd(p2_AUGD);
    doc_augd(p2_AUGD_01);
    doc_augd(p2_AUGD_10);
    doc_augd(p2_AUGD_11);
}

/**
 * @brief Return a mask / match for an instruction's pattern
 * @param instr instruction's opcode
 * @param pat pattern
 * @return mask / match value in 64 bits
 */
quint64 P2Doc::pattern(uint instr, const char* pat, const char* source)
{
    quint64 result = 0;
    QString pack = QString::fromLatin1(pat);

    // without spaces
    pack.remove(QChar(' '));
    Q_ASSERT(pack.size() == 32);        // must be 32 bits

    // replace '0' with '1'
    QString instr_mask = pack;
    instr_mask.replace(QChar('0'), QChar('1'));
    instr_mask.replace(QChar('N'), QChar('1')); // SETNIBGETNIB/ROLNIB/SETWORD/GETWORD/ROLWORD
    instr_mask.replace(QChar('h'), QChar('1'));
    instr_mask.replace(QChar('x'), QChar('1'));

    // replace any special characters with '0'
    QString binary_mask = instr_mask;
    binary_mask.replace(QRegExp("[ECZILNDSczRAW]"), QChar('0'));
    Q_ASSERT(binary_mask.count(QChar('0')) + binary_mask.count(QChar('1')) == 32);

    // put the binary mask in the upper 32 bits
    bool ok;
    quint64 mask = binary_mask.toULongLong(&ok, 2);
    if (ok) {
        result = mask << 32;
    } else {
        qDebug("you missed a special character: %s", qPrintable(binary_mask));
    }

    if (instr & ~mask) {
        QString dbg_mask = binary_mask;
        QString dbg_instr = QString("%1").arg(instr | mask, 32, 2, QChar('0'));
        dbg_mask.replace(QChar('0'), QChar('_'));
        dbg_instr.replace(QChar('0'), QChar('_'));
        qDebug("mask does not cover instr: %s %s (%s)",
               qPrintable(dbg_mask),
               qPrintable(dbg_instr),
               source);
    }

    // conditional mask
    QString cond_mask = instr_mask;
    // replace '1' with '0'
    cond_mask.replace(QChar('1'), QChar('0'));
    cond_mask.replace(QChar('E'), QChar('1'));

    result |= instr;
    m_pattern.insert(result, pat);

    return result;
}

/**
 * @brief Pattern for 7 bit instruction
 * @param instr instruction enum value
 * @param src pattern string
 * @return mask / match value in 64 bits
 */
quint64 P2Doc::pattern(p2_inst7_e instr, const char* pat)
{
    uint inst7 = static_cast<uint>(instr) << 21;
    return pattern(inst7, pat, m_instr.value(instr));
}

/**
 * @brief Pattern for 8 bit instruction
 * @param instr instruction enum value
 * @param src pattern string
 * @return mask / match value in 64 bits
 */
quint64 P2Doc::pattern(p2_inst8_e instr, const char* pat)
{
    uint inst8 = static_cast<uint>(instr) << 20;
    return pattern(inst8, pat, m_instr.value(instr));
}

/**
 * @brief Pattern for 9 bit instruction
 * @param instr instruction enum value
 * @param src pattern string
 * @return mask / match value in 64 bits
 */
quint64 P2Doc::pattern(p2_inst9_e instr, const char* pat)
{
    uint inst9 = static_cast<uint>(instr) << 19;
    return pattern(inst9, pat, m_instr.value(instr));
}

/**
 * @brief Pattern for instruction with dst field enumeration value
 * @param instr instruction enum value
 * @param src pattern string
 * @return mask / match value in 64 bits
 */
quint64 P2Doc::pattern(p2_opdst_e instr, const char* pat)
{
    // p2_inst9_t 1011110 01
    uint opdst = (0x179 << 19) | static_cast<uint>(instr) << 9;
    return pattern(opdst, pat, m_instr.value(instr));
}

/**
 * @brief Pattern for instruction with src field enumeration value
 * @param instr instruction enum value
 * @param src pattern string
 * @return mask / match value in 64 bits
 */
quint64 P2Doc::pattern(p2_opsrc_e instr, const char* pat)
{
    uint opsrc = static_cast<uint>(instr);
    return pattern(opsrc, pat, m_instr.value(instr));
}

/**
 * @brief Pattern for instruction with dst field enumeration value, src = %000100100
 * @param instr instruction enum value
 * @param src pattern string
 * @return mask / match value in 64 bits
 */
quint64 P2Doc::pattern(p2_opx24_e instr, const char* pat)
{
    uint opx24 = static_cast<uint>(instr) << 9 | 0x24;
    return pattern(opx24, pat, m_instr.value(instr));
}

/**
 * @brief No operation.
 *
 * 0000 0000000 000 000000000 000000000
 *
 * NOP
 *
 */
void P2Doc::doc_nop(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("No operation."));
    m_instr.insert(instr, "NOP");
    m_token.insert(instr, t_NOP);
    pattern(instr, "0000 0000000 000 000000000 000000000");
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
void P2Doc::doc_ror(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Rotate right."));
    m_instr.insert(instr, "ROR     D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_ROR);
    pattern(instr, "EEEE 0000000 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_rol(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Rotate left."));
    m_instr.insert(instr, "ROL     D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_ROL);
    pattern(instr, "EEEE 0000001 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_shr(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Shift right."));
    m_instr.insert(instr, "SHR     D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_SHR);
    pattern(instr, "EEEE 0000010 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_shl(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Shift left."));
    m_instr.insert(instr, "SHL     D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_SHL);
    pattern(instr, "EEEE 0000011 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_rcr(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Rotate carry right."));
    m_instr.insert(instr, "RCR     D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_RCR);
    pattern(instr, "EEEE 0000100 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_rcl(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Rotate carry left."));
    m_instr.insert(instr, "RCL     D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_RCL);
    pattern(instr, "EEEE 0000101 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_sar(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Shift arithmetic right."));
    m_instr.insert(instr, "SAR     D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_SAR);
    pattern(instr, "EEEE 0000110 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_sal(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Shift arithmetic left."));
    m_instr.insert(instr, "SAL     D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_SAL);
    pattern(instr, "EEEE 0000111 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_add(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Add S into D."));
    m_instr.insert(instr, "ADD     D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_ADD);
    pattern(instr, "EEEE 0001000 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_addx(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Add (S + C) into D, extended."));
    m_instr.insert(instr, "ADDX    D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_ADDX);
    pattern(instr, "EEEE 0001001 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_adds(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Add S into D, signed."));
    m_instr.insert(instr, "ADDS    D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_ADDS);
    pattern(instr, "EEEE 0001010 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_addsx(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Add (S + C) into D, signed and extended."));
    m_instr.insert(instr, "ADDSX   D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_ADDSX);
    pattern(instr, "EEEE 0001011 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_sub(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Subtract S from D."));
    m_instr.insert(instr, "SUB     D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_SUB);
    pattern(instr, "EEEE 0001100 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_subx(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Subtract (S + C) from D, extended."));
    m_instr.insert(instr, "SUBX    D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_SUBX);
    pattern(instr, "EEEE 0001101 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_subs(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Subtract S from D, signed."));
    m_instr.insert(instr, "SUBS    D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_SUBS);
    pattern(instr, "EEEE 0001110 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_subsx(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Subtract (S + C) from D, signed and extended."));
    m_instr.insert(instr, "SUBSX   D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_SUBSX);
    pattern(instr, "EEEE 0001111 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_cmp(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Compare D to S."));
    m_instr.insert(instr, "CMP     D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_CMP);
    pattern(instr, "EEEE 0010000 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_cmpx(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Compare D to (S + C), extended."));
    m_instr.insert(instr, "CMPX    D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_CMPX);
    pattern(instr, "EEEE 0010001 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_cmps(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Compare D to S, signed."));
    m_instr.insert(instr, "CMPS    D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_CMPS);
    pattern(instr, "EEEE 0010010 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_cmpsx(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Compare D to (S + C), signed and extended."));
    m_instr.insert(instr, "CMPSX   D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_CMPSX);
    pattern(instr, "EEEE 0010011 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_cmpr(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Compare S to D (reverse)."));
    m_instr.insert(instr, "CMPR    D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_CMPR);
    pattern(instr, "EEEE 0010100 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_cmpm(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Compare D to S, get MSB of difference into C."));
    m_instr.insert(instr, "CMPM    D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_CMPM);
    pattern(instr, "EEEE 0010101 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_subr(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Subtract D from S (reverse)."));
    m_instr.insert(instr, "SUBR    D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_SUBR);
    pattern(instr, "EEEE 0010110 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_cmpsub(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Compare and subtract S from D if D >= S."));
    m_instr.insert(instr, "CMPSUB  D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_CMPSUB);
    pattern(instr, "EEEE 0010111 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_fge(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Force D >= S."));
    m_instr.insert(instr, "FGE     D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_FGE);
    pattern(instr, "EEEE 0011000 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_fle(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Force D <= S."));
    m_instr.insert(instr, "FLE     D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_FLE);
    pattern(instr, "EEEE 0011001 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_fges(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Force D >= S, signed."));
    m_instr.insert(instr, "FGES    D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_FGES);
    pattern(instr, "EEEE 0011010 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_fles(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Force D <= S, signed."));
    m_instr.insert(instr, "FLES    D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_FLES);
    pattern(instr, "EEEE 0011011 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_sumc(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Sum +/-S into D by  C."));
    m_instr.insert(instr, "SUMC    D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_SUMC);
    pattern(instr, "EEEE 0011100 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_sumnc(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Sum +/-S into D by !C."));
    m_instr.insert(instr, "SUMNC   D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_SUMNC);
    pattern(instr, "EEEE 0011101 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_sumz(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Sum +/-S into D by  Z."));
    m_instr.insert(instr, "SUMZ    D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_SUMZ);
    pattern(instr, "EEEE 0011110 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_sumnz(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Sum +/-S into D by !Z."));
    m_instr.insert(instr, "SUMNZ   D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_SUMNZ);
    pattern(instr, "EEEE 0011111 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_testb_w(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Test bit S[4:0] of  D, write to C/Z."));
    m_instr.insert(instr, "TESTB   D,{#}S         WC/WZ");
    m_token.insert(instr, t_TESTB);
    pattern(instr, "EEEE 0100000 xxI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_testbn_w(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Test bit S[4:0] of !D, write to C/Z."));
    m_instr.insert(instr, "TESTBN  D,{#}S         WC/WZ");
    m_token.insert(instr, t_TESTBN);
    pattern(instr, "EEEE 0100001 xxI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_testb_and(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Test bit S[4:0] of  D, AND into C/Z."));
    m_instr.insert(instr, "TESTB   D,{#}S     ANDC/ANDZ");
    m_token.insert(instr, t_TESTB);    // t_AND
    pattern(instr, "EEEE 0100010 xxI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_testbn_and(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Test bit S[4:0] of !D, AND into C/Z."));
    m_instr.insert(instr, "TESTBN  D,{#}S     ANDC/ANDZ");
    m_token.insert(instr, t_TESTBN);   // t_AND
    pattern(instr, "EEEE 0100011 xxI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_testb_or(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Test bit S[4:0] of  D, OR  into C/Z."));
    m_instr.insert(instr, "TESTB   D,{#}S       ORC/ORZ");
    m_token.insert(instr, t_TESTB);    // t_OR
    pattern(instr, "EEEE 0100100 xxI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_testbn_or(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Test bit S[4:0] of !D, OR  into C/Z."));
    m_instr.insert(instr, "TESTBN  D,{#}S       ORC/ORZ");
    m_token.insert(instr, t_TESTBN);   // t_OR
    pattern(instr, "EEEE 0100101 xxI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_testb_xor(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Test bit S[4:0] of  D, XOR into C/Z."));
    m_instr.insert(instr, "TESTB   D,{#}S     XORC/XORZ");
    m_token.insert(instr, t_TESTB);    // t_XOR
    pattern(instr, "EEEE 0100110 xxI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_testbn_xor(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Test bit S[4:0] of !D, XOR into C/Z."));
    m_instr.insert(instr, "TESTBN  D,{#}S     XORC/XORZ");
    m_token.insert(instr, t_TESTBN);   // t_XOR
    pattern(instr, "EEEE 0100111 xxI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_bitl(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Bit S[4:0] of D = 0,    C,Z = D[S[4:0]]."));
    m_instr.insert(instr, "BITL    D,{#}S         {WCZ}");
    m_token.insert(instr, t_BITL);
    pattern(instr, "EEEE 0100000 hhI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_bith(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Bit S[4:0] of D = 1,    C,Z = D[S[4:0]]."));
    m_instr.insert(instr, "BITH    D,{#}S         {WCZ}");
    m_token.insert(instr, t_BITH);
    pattern(instr, "EEEE 0100001 hhI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_bitc(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Bit S[4:0] of D = C,    C,Z = D[S[4:0]]."));
    m_instr.insert(instr, "BITC    D,{#}S         {WCZ}");
    m_token.insert(instr, t_BITC);
    pattern(instr, "EEEE 0100010 hhI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_bitnc(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Bit S[4:0] of D = !C,   C,Z = D[S[4:0]]."));
    m_instr.insert(instr, "BITNC   D,{#}S         {WCZ}");
    m_token.insert(instr, t_BITNC);
    pattern(instr, "EEEE 0100011 hhI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_bitz(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Bit S[4:0] of D = Z,    C,Z = D[S[4:0]]."));
    m_instr.insert(instr, "BITZ    D,{#}S         {WCZ}");
    m_token.insert(instr, t_BITZ);
    pattern(instr, "EEEE 0100100 hhI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_bitnz(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Bit S[4:0] of D = !Z,   C,Z = D[S[4:0]]."));
    m_instr.insert(instr, "BITNZ   D,{#}S         {WCZ}");
    m_token.insert(instr, t_BITNZ);
    pattern(instr, "EEEE 0100101 hhI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_bitrnd(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Bit S[4:0] of D = RND,  C,Z = D[S[4:0]]."));
    m_instr.insert(instr, "BITRND  D,{#}S         {WCZ}");
    m_token.insert(instr, t_BITRND);
    pattern(instr, "EEEE 0100110 hhI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_bitnot(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Bit S[4:0] of D = !bit, C,Z = D[S[4:0]]."));
    m_instr.insert(instr, "BITNOT  D,{#}S         {WCZ}");
    m_token.insert(instr, t_BITNOT);
    pattern(instr, "EEEE 0100111 hhI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_and(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("AND S into D."));
    m_instr.insert(instr, "AND     D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_AND);
    pattern(instr, "EEEE 0101000 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_andn(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("AND !S into D."));
    m_instr.insert(instr, "ANDN    D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_ANDN);
    pattern(instr, "EEEE 0101001 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_or(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("OR S into D."));
    m_instr.insert(instr, "OR      D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_OR);
    pattern(instr, "EEEE 0101010 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_xor(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("XOR S into D."));
    m_instr.insert(instr, "XOR     D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_XOR);
    pattern(instr, "EEEE 0101011 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_muxc(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Mux C into each D bit that is '1' in S."));
    m_instr.insert(instr, "MUXC    D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_MUXC);
    pattern(instr, "EEEE 0101100 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_muxnc(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Mux !C into each D bit that is '1' in S."));
    m_instr.insert(instr, "MUXNC   D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_MUXNC);
    pattern(instr, "EEEE 0101101 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_muxz(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Mux Z into each D bit that is '1' in S."));
    m_instr.insert(instr, "MUXZ    D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_MUXZ);
    pattern(instr, "EEEE 0101110 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_muxnz(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Mux !Z into each D bit that is '1' in S."));
    m_instr.insert(instr, "MUXNZ   D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_MUXNZ);
    pattern(instr, "EEEE 0101111 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_mov(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Move S into D."));
    m_instr.insert(instr, "MOV     D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_MOV);
    pattern(instr, "EEEE 0110000 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_not(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Get !S into D."));
    m_instr.insert(instr, "NOT     D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_NOT);
    pattern(instr, "EEEE 0110001 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_abs(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Get absolute value of S into D."));
    m_instr.insert(instr, "ABS     D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_ABS);
    pattern(instr, "EEEE 0110010 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_neg(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Negate S into D."));
    m_instr.insert(instr, "NEG     D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_NEG);
    pattern(instr, "EEEE 0110011 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_negc(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Negate S by  C into D."));
    m_instr.insert(instr, "NEGC    D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_NEGC);
    pattern(instr, "EEEE 0110100 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_negnc(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Negate S by !C into D."));
    m_instr.insert(instr, "NEGNC   D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_NEGNC);
    pattern(instr, "EEEE 0110101 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_negz(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Negate S by  Z into D."));
    m_instr.insert(instr, "NEGZ    D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_NEGZ);
    pattern(instr, "EEEE 0110110 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_negnz(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Negate S by !Z into D."));
    m_instr.insert(instr, "NEGNZ   D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_NEGNZ);
    pattern(instr, "EEEE 0110111 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_incmod(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Increment with modulus."));
    m_instr.insert(instr, "INCMOD  D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_INCMOD);
    pattern(instr, "EEEE 0111000 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_decmod(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Decrement with modulus."));
    m_instr.insert(instr, "DECMOD  D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_DECMOD);
    pattern(instr, "EEEE 0111001 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_zerox(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Zero-extend D above bit S[4:0]."));
    m_instr.insert(instr, "ZEROX   D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_ZEROX);
    pattern(instr, "EEEE 0111010 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_signx(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Sign-extend D from bit S[4:0]."));
    m_instr.insert(instr, "SIGNX   D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_SIGNX);
    pattern(instr, "EEEE 0111011 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_encod(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Get bit position of top-most '1' in S into D."));
    m_instr.insert(instr, "ENCOD   D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_ENCOD);
    pattern(instr, "EEEE 0111100 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_ones(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Get number of '1's in S into D."));
    m_instr.insert(instr, "ONES    D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_ONES);
    pattern(instr, "EEEE 0111101 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_test(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Test D with S."));
    m_instr.insert(instr, "TEST    D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_TEST);
    pattern(instr, "EEEE 0111110 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_testn(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Test D with !S."));
    m_instr.insert(instr, "TESTN   D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_TESTN);
    pattern(instr, "EEEE 0111111 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_setnib(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Set S[3:0] into nibble N in D, keeping rest of D same."));
    m_instr.insert(instr, "SETNIB  D,{#}S,#N");
    m_token.insert(instr, t_SETNIB);
    pattern(instr, "EEEE 100000N NNI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_setnib_altsn(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Set S[3:0] into nibble established by prior ALTSN instruction."));
    m_instr.insert(instr, "SETNIB  {#}S");
    m_token.insert(instr, t_SETNIB);
    pattern(instr, "EEEE 1000000 00I 000000000 SSSSSSSSS");
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
void P2Doc::doc_getnib(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Get nibble N of S into D."));
    m_instr.insert(instr, "GETNIB  D,{#}S,#N");
    m_token.insert(instr, t_GETNIB);
    pattern(instr, "EEEE 100001N NNI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_getnib_altgn(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Get nibble established by prior ALTGN instruction into D."));
    m_instr.insert(instr, "GETNIB  D");
    m_token.insert(instr, t_GETNIB);
    pattern(instr, "EEEE 1000010 000 DDDDDDDDD 000000000");
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
void P2Doc::doc_rolnib(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Rotate-left nibble N of S into D."));
    m_instr.insert(instr, "ROLNIB  D,{#}S,#N");
    m_token.insert(instr, t_ROLNIB);
    pattern(instr, "EEEE 100010N NNI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_rolnib_altgn(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Rotate-left nibble established by prior ALTGN instruction into D."));
    m_instr.insert(instr, "ROLNIB  D");
    m_token.insert(instr, t_ROLNIB);
    pattern(instr, "EEEE 1000100 000 DDDDDDDDD 000000000");
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
void P2Doc::doc_setbyte(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Set S[7:0] into byte N in D, keeping rest of D same."));
    m_instr.insert(instr, "SETBYTE D,{#}S,#N");
    m_token.insert(instr, t_SETBYTE);
    pattern(instr, "EEEE 1000110 NNI DDDDDDDDD SSSSSSSSS");
}

/**
 * @brief Set S[7:0] into byte established by prior ALTSB instruction.
 *
 * EEEE 1000110 00I 000000000 SSSSSSSSS
 *
 * SETBYTE {#}S
 *
 *
 */
void P2Doc::doc_setbyte_altsb(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Set S[7:0] into byte established by prior ALTSB instruction."));
    m_instr.insert(instr, "SETBYTE {#}S");
    m_token.insert(instr, t_SETBYTE);
    pattern(instr, "EEEE 1000110 00I 000000000 SSSSSSSSS");
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
void P2Doc::doc_getbyte(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Get byte N of S into D."));
    m_instr.insert(instr, "GETBYTE D,{#}S,#N");
    m_token.insert(instr, t_GETBYTE);
    pattern(instr, "EEEE 1000111 NNI DDDDDDDDD SSSSSSSSS");
}

/**
 * @brief Get byte established by prior ALTGB instruction into D.
 *
 * EEEE 1000111 000 DDDDDDDDD 000000000
 *
 * GETBYTE D
 *
 *
 */
void P2Doc::doc_getbyte_altgb(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Get byte established by prior ALTGB instruction into D."));
    m_instr.insert(instr, "GETBYTE D");
    m_token.insert(instr, t_GETBYTE);
    pattern(instr, "EEEE 1000111 000 DDDDDDDDD 000000000");
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
void P2Doc::doc_rolbyte(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Rotate-left byte N of S into D."));
    m_instr.insert(instr, "ROLBYTE D,{#}S,#N");
    m_token.insert(instr, t_ROLBYTE);
    pattern(instr, "EEEE 1001000 NNI DDDDDDDDD SSSSSSSSS");
}

/**
 * @brief Rotate-left byte established by prior ALTGB instruction into D.
 *
 * EEEE 1001000 000 DDDDDDDDD 000000000
 *
 * ROLBYTE D
 *
 *
 */
void P2Doc::doc_rolbyte_altgb(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Rotate-left byte established by prior ALTGB instruction into D."));
    m_instr.insert(instr, "ROLBYTE D");
    m_token.insert(instr, t_ROLBYTE);
    pattern(instr, "EEEE 1001000 000 DDDDDDDDD 000000000");
}

/**
 * @brief Set S[15:0] into word N in D, keeping rest of D same.
 *
 * EEEE 1001001 0NI DDDDDDDDD SSSSSSSSS
 *
 * SETWORD D,{#}S,#N
 *
 *
 */
void P2Doc::doc_setword(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Set S[15:0] into word N in D, keeping rest of D same."));
    m_instr.insert(instr, "SETWORD D,{#}S,#N");
    m_token.insert(instr, t_SETWORD);
    pattern(instr, "EEEE 1001001 0NI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_setword_altsw(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Set S[15:0] into word established by prior ALTSW instruction."));
    m_instr.insert(instr, "SETWORD {#}S");
    m_token.insert(instr, t_SETWORD);
    pattern(instr, "EEEE 1001001 00I 000000000 SSSSSSSSS");
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
void P2Doc::doc_getword(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Get word N of S into D."));
    m_instr.insert(instr, "GETWORD D,{#}S,#N");
    m_token.insert(instr, t_GETWORD);
    pattern(instr, "EEEE 1001001 1NI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_getword_altgw(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Get word established by prior ALTGW instruction into D."));
    m_instr.insert(instr, "GETWORD D");
    m_token.insert(instr, t_GETWORD);
    pattern(instr, "EEEE 1001001 100 DDDDDDDDD 000000000");
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
void P2Doc::doc_rolword(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Rotate-left word N of S into D."));
    m_instr.insert(instr, "ROLWORD D,{#}S,#N");
    m_token.insert(instr, t_ROLWORD);
    pattern(instr, "EEEE 1001010 0NI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_rolword_altgw(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Rotate-left word established by prior ALTGW instruction into D."));
    m_instr.insert(instr, "ROLWORD D");
    m_token.insert(instr, t_ROLWORD);
    pattern(instr, "EEEE 1001010 000 DDDDDDDDD 000000000");
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
void P2Doc::doc_altsn(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Alter subsequent SETNIB instruction."));
    m_instr.insert(instr, "ALTSN   D,{#}S");
    m_token.insert(instr, t_ALTSN);
    pattern(instr, "EEEE 1001010 10I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_altsn_d(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Alter subsequent SETNIB instruction."));
    m_instr.insert(instr, "ALTSN   D");
    m_token.insert(instr, t_ALTSN);
    pattern(instr, "EEEE 1001010 101 DDDDDDDDD 000000000");
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
void P2Doc::doc_altgn(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Alter subsequent GETNIB/ROLNIB instruction."));
    m_instr.insert(instr, "ALTGN   D,{#}S");
    m_token.insert(instr, t_ALTGN);
    pattern(instr, "EEEE 1001010 11I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_altgn_d(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Alter subsequent GETNIB/ROLNIB instruction."));
    m_instr.insert(instr, "ALTGN   D");
    m_token.insert(instr, t_ALTGN);
    pattern(instr, "EEEE 1001010 111 DDDDDDDDD 000000000");
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
void P2Doc::doc_altsb(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Alter subsequent SETBYTE instruction."));
    m_instr.insert(instr, "ALTSB   D,{#}S");
    m_token.insert(instr, t_ALTSB);
    pattern(instr, "EEEE 1001011 00I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_altsb_d(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Alter subsequent SETBYTE instruction."));
    m_instr.insert(instr, "ALTSB   D");
    m_token.insert(instr, t_ALTSB);
    pattern(instr, "EEEE 1001011 001 DDDDDDDDD 000000000");
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
void P2Doc::doc_altgb(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Alter subsequent GETBYTE/ROLBYTE instruction."));
    m_instr.insert(instr, "ALTGB   D,{#}S");
    m_token.insert(instr, t_ALTGB);
    pattern(instr, "EEEE 1001011 01I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_altgb_d(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Alter subsequent GETBYTE/ROLBYTE instruction."));
    m_instr.insert(instr, "ALTGB   D");
    m_token.insert(instr, t_ALTGB);
    pattern(instr, "EEEE 1001011 011 DDDDDDDDD 000000000");
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
void P2Doc::doc_altsw(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Alter subsequent SETWORD instruction."));
    m_instr.insert(instr, "ALTSW   D,{#}S");
    m_token.insert(instr, t_ALTSW);
    pattern(instr, "EEEE 1001011 10I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_altsw_d(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Alter subsequent SETWORD instruction."));
    m_instr.insert(instr, "ALTSW   D");
    m_token.insert(instr, t_ALTSW);
    pattern(instr, "EEEE 1001011 101 DDDDDDDDD 000000000");
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
void P2Doc::doc_altgw(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Alter subsequent GETWORD/ROLWORD instruction."));
    m_instr.insert(instr, "ALTGW   D,{#}S");
    m_token.insert(instr, t_ALTGW);
    pattern(instr, "EEEE 1001011 11I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_altgw_d(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Alter subsequent GETWORD/ROLWORD instruction."));
    m_instr.insert(instr, "ALTGW   D");
    m_token.insert(instr, t_ALTGW);
    pattern(instr, "EEEE 1001011 111 DDDDDDDDD 000000000");
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
void P2Doc::doc_altr(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Alter result register address (normally D field) of next instruction to (D + S) & $1FF."));
    m_instr.insert(instr, "ALTR    D,{#}S");
    m_token.insert(instr, t_ALTR);
    pattern(instr, "EEEE 1001100 00I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_altr_d(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Alter result register address (normally D field) of next instruction to D[8:0]."));
    m_instr.insert(instr, "ALTR    D");
    m_token.insert(instr, t_ALTD);
    pattern(instr, "EEEE 1001100 001 DDDDDDDDD 000000000");
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
void P2Doc::doc_altd(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Alter D field of next instruction to (D + S) & $1FF."));
    m_instr.insert(instr, "ALTD    D,{#}S");
    m_token.insert(instr, t_ALTD);
    pattern(instr, "EEEE 1001100 01I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_altd_d(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Alter D field of next instruction to D[8:0]."));
    m_instr.insert(instr, "ALTD    D");
    m_token.insert(instr, t_ALTD);
    pattern(instr, "EEEE 1001100 011 DDDDDDDDD 000000000");
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
void P2Doc::doc_alts(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Alter S field of next instruction to (D + S) & $1FF."));
    m_instr.insert(instr, "ALTS    D,{#}S");
    m_token.insert(instr, t_ALTS);
    pattern(instr, "EEEE 1001100 10I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_alts_d(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Alter S field of next instruction to D[8:0]."));
    m_instr.insert(instr, "ALTS    D");
    m_token.insert(instr, t_ALTS);
    pattern(instr, "EEEE 1001100 101 DDDDDDDDD 000000000");
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
void P2Doc::doc_altb(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Alter D field of next instruction to (D[13:5] + S) & $1FF."));
    m_instr.insert(instr, "ALTB    D,{#}S");
    m_token.insert(instr, t_ALTB);
    pattern(instr, "EEEE 1001100 11I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_altb_d(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Alter D field of next instruction to D[13:5]."));
    m_instr.insert(instr, "ALTB    D");
    m_token.insert(instr, t_ALTB);
    pattern(instr, "EEEE 1001100 111 DDDDDDDDD 000000000");
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
void P2Doc::doc_alti(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Substitute next instruction's I/R/D/S fields with fields from D, per S."));
    m_instr.insert(instr, "ALTI    D,{#}S");
    m_token.insert(instr, t_ALTI);
    pattern(instr, "EEEE 1001101 00I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_alti_d(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Execute D in place of next instruction."));
    m_instr.insert(instr, "ALTI    D");
    m_token.insert(instr, t_ALTI);
    pattern(instr, "EEEE 1001101 001 DDDDDDDDD 101100100");
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
void P2Doc::doc_setr(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Set R field of D to S[8:0]."));
    m_instr.insert(instr, "SETR    D,{#}S");
    m_token.insert(instr, t_SETR);
    pattern(instr, "EEEE 1001101 01I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_setd(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Set D field of D to S[8:0]."));
    m_instr.insert(instr, "SETD    D,{#}S");
    m_token.insert(instr, t_SETD);
    pattern(instr, "EEEE 1001101 10I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_sets(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Set S field of D to S[8:0]."));
    m_instr.insert(instr, "SETS    D,{#}S");
    m_token.insert(instr, t_SETS);
    pattern(instr, "EEEE 1001101 11I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_decod(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Decode S[4:0] into D."));
    m_instr.insert(instr, "DECOD   D,{#}S");
    m_token.insert(instr, t_DECOD);
    pattern(instr, "EEEE 1001110 00I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_decod_d(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Decode D[4:0] into D."));
    m_instr.insert(instr, "DECOD   D");
    m_token.insert(instr, t_DECOD);
    pattern(instr, "EEEE 1001110 000 DDDDDDDDD DDDDDDDDD");
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
void P2Doc::doc_bmask(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Get LSB-justified bit mask of size (S[4:0] + 1) into D."));
    m_instr.insert(instr, "BMASK   D,{#}S");
    m_token.insert(instr, t_BMASK);
    pattern(instr, "EEEE 1001110 01I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_bmask_d(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Get LSB-justified bit mask of size (D[4:0] + 1) into D."));
    m_instr.insert(instr, "BMASK   D");
    m_token.insert(instr, t_BMASK);
    pattern(instr, "EEEE 1001110 010 DDDDDDDDD DDDDDDDDD");
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
void P2Doc::doc_crcbit(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Iterate CRC value in D using C and polynomial in S."));
    m_instr.insert(instr, "CRCBIT  D,{#}S");
    m_token.insert(instr, t_CRCBIT);
    pattern(instr, "EEEE 1001110 10I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_crcnib(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Iterate CRC value in D using Q[31:28] and polynomial in S."));
    m_instr.insert(instr, "CRCNIB  D,{#}S");
    m_token.insert(instr, t_CRCNIB);
    pattern(instr, "EEEE 1001110 11I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_muxnits(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("For each non-zero bit pair in S, copy that bit pair into the corresponding D bits, else leave that D bit pair the same."));
    m_instr.insert(instr, "MUXNITS D,{#}S");
    m_token.insert(instr, t_MUXNITS);
    pattern(instr, "EEEE 1001111 00I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_muxnibs(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("For each non-zero nibble in S, copy that nibble into the corresponding D nibble, else leave that D nibble the same."));
    m_instr.insert(instr, "MUXNIBS D,{#}S");
    m_token.insert(instr, t_MUXNIBS);
    pattern(instr, "EEEE 1001111 01I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_muxq(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Used after SETQ."));
    m_instr.insert(instr, "MUXQ    D,{#}S");
    m_token.insert(instr, t_MUXQ);
    pattern(instr, "EEEE 1001111 10I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_movbyts(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Move bytes within D, per S."));
    m_instr.insert(instr, "MOVBYTS D,{#}S");
    m_token.insert(instr, t_MOVBYTS);
    pattern(instr, "EEEE 1001111 11I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_mul(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("D = unsigned (D[15:0] * S[15:0])."));
    m_instr.insert(instr, "MUL     D,{#}S          {WZ}");
    m_token.insert(instr, t_MUL);
    pattern(instr, "EEEE 1010000 0ZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_muls(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("D = signed (D[15:0] * S[15:0])."));
    m_instr.insert(instr, "MULS    D,{#}S          {WZ}");
    m_token.insert(instr, t_MULS);
    pattern(instr, "EEEE 1010000 1ZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_sca(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Next instruction's S value = unsigned (D[15:0] * S[15:0]) >> 16."));
    m_instr.insert(instr, "SCA     D,{#}S          {WZ}");
    m_token.insert(instr, t_SCA);
    pattern(instr, "EEEE 1010001 0ZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_scas(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Next instruction's S value = signed (D[15:0] * S[15:0]) >> 14."));
    m_instr.insert(instr, "SCAS    D,{#}S          {WZ}");
    m_token.insert(instr, t_SCAS);
    pattern(instr, "EEEE 1010001 1ZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_addpix(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Add bytes of S into bytes of D, with $FF saturation."));
    m_instr.insert(instr, "ADDPIX  D,{#}S");
    m_token.insert(instr, t_ADDPIX);
    pattern(instr, "EEEE 1010010 00I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_mulpix(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Multiply bytes of S into bytes of D, where $FF = 1."));
    m_instr.insert(instr, "MULPIX  D,{#}S");
    m_token.insert(instr, t_MULPIX);
    pattern(instr, "EEEE 1010010 01I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_blnpix(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Alpha-blend bytes of S into bytes of D, using SETPIV value."));
    m_instr.insert(instr, "BLNPIX  D,{#}S");
    m_token.insert(instr, t_BLNPIX);
    pattern(instr, "EEEE 1010010 10I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_mixpix(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Mix bytes of S into bytes of D, using SETPIX and SETPIV values."));
    m_instr.insert(instr, "MIXPIX  D,{#}S");
    m_token.insert(instr, t_MIXPIX);
    pattern(instr, "EEEE 1010010 11I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_addct1(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Set CT1 event to trigger on CT = D + S."));
    m_instr.insert(instr, "ADDCT1  D,{#}S");
    m_token.insert(instr, t_ADDCT1);
    pattern(instr, "EEEE 1010011 00I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_addct2(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Set CT2 event to trigger on CT = D + S."));
    m_instr.insert(instr, "ADDCT2  D,{#}S");
    m_token.insert(instr, t_ADDCT2);
    pattern(instr, "EEEE 1010011 01I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_addct3(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Set CT3 event to trigger on CT = D + S."));
    m_instr.insert(instr, "ADDCT3  D,{#}S");
    m_token.insert(instr, t_ADDCT3);
    pattern(instr, "EEEE 1010011 10I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_wmlong(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Write only non-$00 bytes in D[31:0] to hub address {#}S/PTRx."));
    m_instr.insert(instr, "WMLONG  D,{#}S/P");
    m_token.insert(instr, t_WMLONG);
    pattern(instr, "EEEE 1010011 11I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_rqpin(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Read smart pin S[5:0] result \"Z\" into D, don't acknowledge smart pin (\"Q\" in RQPIN means \"quiet\")."));
    m_instr.insert(instr, "RQPIN   D,{#}S          {WC}");
    m_token.insert(instr, t_RQPIN);
    pattern(instr, "EEEE 1010100 C0I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_rdpin(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Read smart pin S[5:0] result \"Z\" into D, acknowledge smart pin."));
    m_instr.insert(instr, "RDPIN   D,{#}S          {WC}");
    m_token.insert(instr, t_RDPIN);
    pattern(instr, "EEEE 1010100 x1I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_rdlut(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Read LUT data from address S[8:0] into D."));
    m_instr.insert(instr, "RDLUT   D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_RDLUT);
    pattern(instr, "EEEE 1010101 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_rdbyte(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Read zero-extended byte from hub address {#}S/PTRx into D."));
    m_instr.insert(instr, "RDBYTE  D,{#}S/P {WC/WZ/WCZ}");
    m_token.insert(instr, t_RDBYTE);
    pattern(instr, "EEEE 1010110 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_rdword(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Read zero-extended word from hub address {#}S/PTRx into D."));
    m_instr.insert(instr, "RDWORD  D,{#}S/P {WC/WZ/WCZ}");
    m_token.insert(instr, t_RDWORD);
    pattern(instr, "EEEE 1010111 CZI DDDDDDDDD SSSSSSSSS");
}

/**
 * @brief Read long from hub address {#}S/PTRx into D.
 *
 * EEEE 1011000 CZI DDDDDDDDD SSSSSSSSS
 *
 * RDLONG  D,{#}S/P {WC/WZ/WCZ}
 *
 * C = MSB of long.
 * *   Prior SETQ/SETQ2 invokes cog/LUT block transfer.
 *
 */
void P2Doc::doc_rdlong(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Read long from hub address {#}S/PTRx into D."));
    m_instr.insert(instr, "RDLONG  D,{#}S/P {WC/WZ/WCZ}");
    m_token.insert(instr, t_RDLONG);
    pattern(instr, "EEEE 1011000 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_popa(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Read long from hub address --PTRA into D."));
    m_instr.insert(instr, "POPA    D        {WC/WZ/WCZ}");
    m_token.insert(instr, t_POPA);
    pattern(instr, "EEEE 1011000 CZ1 DDDDDDDDD 101011111");
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
void P2Doc::doc_popb(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Read long from hub address --PTRB into D."));
    m_instr.insert(instr, "POPB    D        {WC/WZ/WCZ}");
    m_token.insert(instr, t_POPB);
    pattern(instr, "EEEE 1011000 CZ1 DDDDDDDDD 111011111");
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
void P2Doc::doc_calld(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Call to S** by writing {C, Z, 10'b0, PC[19:0]} to D."));
    m_instr.insert(instr, "CALLD   D,{#}S   {WC/WZ/WCZ}");
    m_token.insert(instr, t_CALLD);
    pattern(instr, "EEEE 1011001 CZI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_resi3(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Resume from INT3."));
    m_instr.insert(instr, "RESI3");
    m_token.insert(instr, t_RESI3);
    pattern(instr, "EEEE 1011001 110 111110000 111110001");
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
void P2Doc::doc_resi2(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Resume from INT2."));
    m_instr.insert(instr, "RESI2");
    m_token.insert(instr, t_RESI2);
    pattern(instr, "EEEE 1011001 110 111110010 111110011");
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
void P2Doc::doc_resi1(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Resume from INT1."));
    m_instr.insert(instr, "RESI1");
    m_token.insert(instr, t_RESI1);
    pattern(instr, "EEEE 1011001 110 111110100 111110101");
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
void P2Doc::doc_resi0(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Resume from INT0."));
    m_instr.insert(instr, "RESI0");
    m_token.insert(instr, t_RESI0);
    pattern(instr, "EEEE 1011001 110 111111110 111111111");
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
void P2Doc::doc_reti3(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Return from INT3."));
    m_instr.insert(instr, "RETI3");
    m_token.insert(instr, t_RETI3);
    pattern(instr, "EEEE 1011001 110 111111111 111110001");
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
void P2Doc::doc_reti2(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Return from INT2."));
    m_instr.insert(instr, "RETI2");
    m_token.insert(instr, t_RETI2);
    pattern(instr, "EEEE 1011001 110 111111111 111110011");
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
void P2Doc::doc_reti1(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Return from INT1."));
    m_instr.insert(instr, "RETI1");
    m_token.insert(instr, t_RETI1);
    pattern(instr, "EEEE 1011001 110 111111111 111110101");
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
void P2Doc::doc_reti0(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Return from INT0."));
    m_instr.insert(instr, "RETI0");
    m_token.insert(instr, t_RETI0);
    pattern(instr, "EEEE 1011001 110 111111111 111111111");
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
void P2Doc::doc_callpa(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Call to S** by pushing {C, Z, 10'b0, PC[19:0]} onto stack, copy D to PA."));
    m_instr.insert(instr, "CALLPA  {#}D,{#}S");
    m_token.insert(instr, t_CALLPA);
    pattern(instr, "EEEE 1011010 0LI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_callpb(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Call to S** by pushing {C, Z, 10'b0, PC[19:0]} onto stack, copy D to PB."));
    m_instr.insert(instr, "CALLPB  {#}D,{#}S");
    m_token.insert(instr, t_CALLPB);
    pattern(instr, "EEEE 1011010 1LI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_djz(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Decrement D and jump to S** if result is zero."));
    m_instr.insert(instr, "DJZ     D,{#}S");
    m_token.insert(instr, t_DJZ);
    pattern(instr, "EEEE 1011011 00I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_djnz(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Decrement D and jump to S** if result is not zero."));
    m_instr.insert(instr, "DJNZ    D,{#}S");
    m_token.insert(instr, t_DJNZ);
    pattern(instr, "EEEE 1011011 01I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_djf(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Decrement D and jump to S** if result is $FFFF_FFFF."));
    m_instr.insert(instr, "DJF     D,{#}S");
    m_token.insert(instr, t_DJF);
    pattern(instr, "EEEE 1011011 10I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_djnf(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Decrement D and jump to S** if result is not $FFFF_FFFF."));
    m_instr.insert(instr, "DJNF    D,{#}S");
    m_token.insert(instr, t_DJNF);
    pattern(instr, "EEEE 1011011 11I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_ijz(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Increment D and jump to S** if result is zero."));
    m_instr.insert(instr, "IJZ     D,{#}S");
    m_token.insert(instr, t_IJZ);
    pattern(instr, "EEEE 1011100 00I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_ijnz(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Increment D and jump to S** if result is not zero."));
    m_instr.insert(instr, "IJNZ    D,{#}S");
    m_token.insert(instr, t_IJNZ);
    pattern(instr, "EEEE 1011100 01I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_tjz(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Test D and jump to S** if D is zero."));
    m_instr.insert(instr, "TJZ     D,{#}S");
    m_token.insert(instr, t_TJZ);
    pattern(instr, "EEEE 1011100 10I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_tjnz(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Test D and jump to S** if D is not zero."));
    m_instr.insert(instr, "TJNZ    D,{#}S");
    m_token.insert(instr, t_TJNZ);
    pattern(instr, "EEEE 1011100 11I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_tjf(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Test D and jump to S** if D is full (D = $FFFF_FFFF)."));
    m_instr.insert(instr, "TJF     D,{#}S");
    m_token.insert(instr, t_TJF);
    pattern(instr, "EEEE 1011101 00I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_tjnf(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Test D and jump to S** if D is not full (D != $FFFF_FFFF)."));
    m_instr.insert(instr, "TJNF    D,{#}S");
    m_token.insert(instr, t_TJNF);
    pattern(instr, "EEEE 1011101 01I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_tjs(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Test D and jump to S** if D is signed (D[31] = 1)."));
    m_instr.insert(instr, "TJS     D,{#}S");
    m_token.insert(instr, t_TJS);
    pattern(instr, "EEEE 1011101 10I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_tjns(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Test D and jump to S** if D is not signed (D[31] = 0)."));
    m_instr.insert(instr, "TJNS    D,{#}S");
    m_token.insert(instr, t_TJNS);
    pattern(instr, "EEEE 1011101 11I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_tjv(p2_inst9_e instr)
{
    m_brief.insert(instr, tr("Test D and jump to S** if D overflowed (D[31] != C, C = 'correct sign' from last addition/subtraction)."));
    m_instr.insert(instr, "TJV     D,{#}S");
    m_token.insert(instr, t_TJV);
    pattern(instr, "EEEE 1011110 00I DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_jint(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if INT event flag is set."));
    m_instr.insert(instr, "JINT    {#}S");
    m_token.insert(instr, t_JINT);
    pattern(instr, "EEEE 1011110 01I 000000000 SSSSSSSSS");
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
void P2Doc::doc_jct1(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if CT1 event flag is set."));
    m_instr.insert(instr, "JCT1    {#}S");
    m_token.insert(instr, t_JCT1);
    pattern(instr, "EEEE 1011110 01I 000000001 SSSSSSSSS");
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
void P2Doc::doc_jct2(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if CT2 event flag is set."));
    m_instr.insert(instr, "JCT2    {#}S");
    m_token.insert(instr, t_JCT2);
    pattern(instr, "EEEE 1011110 01I 000000010 SSSSSSSSS");
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
void P2Doc::doc_jct3(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if CT3 event flag is set."));
    m_instr.insert(instr, "JCT3    {#}S");
    m_token.insert(instr, t_JCT3);
    pattern(instr, "EEEE 1011110 01I 000000011 SSSSSSSSS");
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
void P2Doc::doc_jse1(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if SE1 event flag is set."));
    m_instr.insert(instr, "JSE1    {#}S");
    m_token.insert(instr, t_JSE1);
    pattern(instr, "EEEE 1011110 01I 000000100 SSSSSSSSS");
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
void P2Doc::doc_jse2(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if SE2 event flag is set."));
    m_instr.insert(instr, "JSE2    {#}S");
    m_token.insert(instr, t_JSE2);
    pattern(instr, "EEEE 1011110 01I 000000101 SSSSSSSSS");
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
void P2Doc::doc_jse3(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if SE3 event flag is set."));
    m_instr.insert(instr, "JSE3    {#}S");
    m_token.insert(instr, t_JSE3);
    pattern(instr, "EEEE 1011110 01I 000000110 SSSSSSSSS");
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
void P2Doc::doc_jse4(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if SE4 event flag is set."));
    m_instr.insert(instr, "JSE4    {#}S");
    m_token.insert(instr, t_JSE4);
    pattern(instr, "EEEE 1011110 01I 000000111 SSSSSSSSS");
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
void P2Doc::doc_jpat(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if PAT event flag is set."));
    m_instr.insert(instr, "JPAT    {#}S");
    m_token.insert(instr, t_JPAT);
    pattern(instr, "EEEE 1011110 01I 000001000 SSSSSSSSS");
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
void P2Doc::doc_jfbw(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if FBW event flag is set."));
    m_instr.insert(instr, "JFBW    {#}S");
    m_token.insert(instr, t_JFBW);
    pattern(instr, "EEEE 1011110 01I 000001001 SSSSSSSSS");
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
void P2Doc::doc_jxmt(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if XMT event flag is set."));
    m_instr.insert(instr, "JXMT    {#}S");
    m_token.insert(instr, t_JXMT);
    pattern(instr, "EEEE 1011110 01I 000001010 SSSSSSSSS");
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
void P2Doc::doc_jxfi(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if XFI event flag is set."));
    m_instr.insert(instr, "JXFI    {#}S");
    m_token.insert(instr, t_JXFI);
    pattern(instr, "EEEE 1011110 01I 000001011 SSSSSSSSS");
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
void P2Doc::doc_jxro(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if XRO event flag is set."));
    m_instr.insert(instr, "JXRO    {#}S");
    m_token.insert(instr, t_JXRO);
    pattern(instr, "EEEE 1011110 01I 000001100 SSSSSSSSS");
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
void P2Doc::doc_jxrl(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if XRL event flag is set."));
    m_instr.insert(instr, "JXRL    {#}S");
    m_token.insert(instr, t_JXRL);
    pattern(instr, "EEEE 1011110 01I 000001101 SSSSSSSSS");
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
void P2Doc::doc_jatn(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if ATN event flag is set."));
    m_instr.insert(instr, "JATN    {#}S");
    m_token.insert(instr, t_JATN);
    pattern(instr, "EEEE 1011110 01I 000001110 SSSSSSSSS");
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
void P2Doc::doc_jqmt(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if QMT event flag is set."));
    m_instr.insert(instr, "JQMT    {#}S");
    m_token.insert(instr, t_JQMT);
    pattern(instr, "EEEE 1011110 01I 000001111 SSSSSSSSS");
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
void P2Doc::doc_jnint(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if INT event flag is clear."));
    m_instr.insert(instr, "JNINT   {#}S");
    m_token.insert(instr, t_JNINT);
    pattern(instr, "EEEE 1011110 01I 000010000 SSSSSSSSS");
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
void P2Doc::doc_jnct1(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if CT1 event flag is clear."));
    m_instr.insert(instr, "JNCT1   {#}S");
    m_token.insert(instr, t_JNCT1);
    pattern(instr, "EEEE 1011110 01I 000010001 SSSSSSSSS");
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
void P2Doc::doc_jnct2(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if CT2 event flag is clear."));
    m_instr.insert(instr, "JNCT2   {#}S");
    m_token.insert(instr, t_JNCT2);
    pattern(instr, "EEEE 1011110 01I 000010010 SSSSSSSSS");
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
void P2Doc::doc_jnct3(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if CT3 event flag is clear."));
    m_instr.insert(instr, "JNCT3   {#}S");
    m_token.insert(instr, t_JNCT3);
    pattern(instr, "EEEE 1011110 01I 000010011 SSSSSSSSS");
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
void P2Doc::doc_jnse1(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if SE1 event flag is clear."));
    m_instr.insert(instr, "JNSE1   {#}S");
    m_token.insert(instr, t_JNSE1);
    pattern(instr, "EEEE 1011110 01I 000010100 SSSSSSSSS");
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
void P2Doc::doc_jnse2(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if SE2 event flag is clear."));
    m_instr.insert(instr, "JNSE2   {#}S");
    m_token.insert(instr, t_JNSE2);
    pattern(instr, "EEEE 1011110 01I 000010101 SSSSSSSSS");
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
void P2Doc::doc_jnse3(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if SE3 event flag is clear."));
    m_instr.insert(instr, "JNSE3   {#}S");
    m_token.insert(instr, t_JNSE3);
    pattern(instr, "EEEE 1011110 01I 000010110 SSSSSSSSS");
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
void P2Doc::doc_jnse4(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if SE4 event flag is clear."));
    m_instr.insert(instr, "JNSE4   {#}S");
    m_token.insert(instr, t_JNSE4);
    pattern(instr, "EEEE 1011110 01I 000010111 SSSSSSSSS");
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
void P2Doc::doc_jnpat(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if PAT event flag is clear."));
    m_instr.insert(instr, "JNPAT   {#}S");
    m_token.insert(instr, t_JNPAT);
    pattern(instr, "EEEE 1011110 01I 000011000 SSSSSSSSS");
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
void P2Doc::doc_jnfbw(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if FBW event flag is clear."));
    m_instr.insert(instr, "JNFBW   {#}S");
    m_token.insert(instr, t_JNFBW);
    pattern(instr, "EEEE 1011110 01I 000011001 SSSSSSSSS");
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
void P2Doc::doc_jnxmt(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if XMT event flag is clear."));
    m_instr.insert(instr, "JNXMT   {#}S");
    m_token.insert(instr, t_JNXMT);
    pattern(instr, "EEEE 1011110 01I 000011010 SSSSSSSSS");
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
void P2Doc::doc_jnxfi(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if XFI event flag is clear."));
    m_instr.insert(instr, "JNXFI   {#}S");
    m_token.insert(instr, t_JNXFI);
    pattern(instr, "EEEE 1011110 01I 000011011 SSSSSSSSS");
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
void P2Doc::doc_jnxro(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if XRO event flag is clear."));
    m_instr.insert(instr, "JNXRO   {#}S");
    m_token.insert(instr, t_JNXRO);
    pattern(instr, "EEEE 1011110 01I 000011100 SSSSSSSSS");
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
void P2Doc::doc_jnxrl(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if XRL event flag is clear."));
    m_instr.insert(instr, "JNXRL   {#}S");
    m_token.insert(instr, t_JNXRL);
    pattern(instr, "EEEE 1011110 01I 000011101 SSSSSSSSS");
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
void P2Doc::doc_jnatn(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if ATN event flag is clear."));
    m_instr.insert(instr, "JNATN   {#}S");
    m_token.insert(instr, t_JNATN);
    pattern(instr, "EEEE 1011110 01I 000011110 SSSSSSSSS");
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
void P2Doc::doc_jnqmt(p2_opdst_e instr)
{
    m_brief.insert(instr, tr("Jump to S** if QMT event flag is clear."));
    m_instr.insert(instr, "JNQMT   {#}S");
    m_token.insert(instr, t_JNQMT);
    pattern(instr, "EEEE 1011110 01I 000011111 SSSSSSSSS");
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
void P2Doc::doc_1011110_1(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Undefined instruction <empty> %1011110_1."));
    m_instr.insert(instr, "<empty> {#}D,{#}S");
    m_token.insert(instr, t_empty);
    pattern(instr, "EEEE 1011110 1LI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_1011111_0(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Undefined instruction <empty> %1011111_0."));
    m_instr.insert(instr, "<empty> {#}D,{#}S");
    m_token.insert(instr, t_empty);
    pattern(instr, "EEEE 1011111 0LI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_setpat(p2_inst8_e instr)
{
    m_instr.insert(instr, "SETPAT  {#}D,{#}S");
    pattern(instr, "EEEE 1011111 1LI DDDDDDDDD SSSSSSSSS");
    m_brief.insert(instr, tr("Set pin pattern for PAT event."));
    m_token.insert(instr, t_SETPAT);
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
void P2Doc::doc_wrpin(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Write D to mode register of smart pin S[5:0], acknowledge smart pin."));
    m_instr.insert(instr, "WRPIN   {#}D,{#}S");
    m_token.insert(instr, t_WRPIN);
    pattern(instr, "EEEE 1100000 0LI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_akpin(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Acknowledge smart pin S[5:0]."));
    m_instr.insert(instr, "AKPIN   {#}S");
    m_token.insert(instr, t_AKPIN);
    pattern(instr, "EEEE 1100000 01I 000000001 SSSSSSSSS");
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
void P2Doc::doc_wxpin(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Write D to parameter \"X\" of smart pin S[5:0], acknowledge smart pin."));
    m_instr.insert(instr, "WXPIN   {#}D,{#}S");
    m_token.insert(instr, t_WXPIN);
    pattern(instr, "EEEE 1100000 1LI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_wypin(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Write D to parameter \"Y\" of smart pin S[5:0], acknowledge smart pin."));
    m_instr.insert(instr, "WYPIN   {#}D,{#}S");
    m_token.insert(instr, t_WYPIN);
    pattern(instr, "EEEE 1100001 0LI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_wrlut(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Write D to LUT address S[8:0]."));
    m_instr.insert(instr, "WRLUT   {#}D,{#}S");
    m_token.insert(instr, t_WRLUT);
    pattern(instr, "EEEE 1100001 1LI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_wrbyte(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Write byte in D[7:0] to hub address {#}S/PTRx."));
    m_instr.insert(instr, "WRBYTE  {#}D,{#}S/P");
    m_token.insert(instr, t_WRBYTE);
    pattern(instr, "EEEE 1100010 0LI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_wrword(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Write word in D[15:0] to hub address {#}S/PTRx."));
    m_instr.insert(instr, "WRWORD  {#}D,{#}S/P");
    m_token.insert(instr, t_WRWORD);
    pattern(instr, "EEEE 1100010 1LI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_wrlong(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Write long in D[31:0] to hub address {#}S/PTRx."));
    m_instr.insert(instr, "WRLONG  {#}D,{#}S/P");
    m_token.insert(instr, t_WRLONG);
    pattern(instr, "EEEE 1100011 0LI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_pusha(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Write long in D[31:0] to hub address PTRA++."));
    m_instr.insert(instr, "PUSHA   {#}D");
    m_token.insert(instr, t_PUSHA);
    pattern(instr, "EEEE 1100011 0L1 DDDDDDDDD 101100001");
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
void P2Doc::doc_pushb(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Write long in D[31:0] to hub address PTRB++."));
    m_instr.insert(instr, "PUSHB   {#}D");
    m_token.insert(instr, t_PUSHB);
    pattern(instr, "EEEE 1100011 0L1 DDDDDDDDD 111100001");
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
void P2Doc::doc_rdfast(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Begin new fast hub read via FIFO."));
    m_instr.insert(instr, "RDFAST  {#}D,{#}S");
    m_token.insert(instr, t_RDFAST);
    pattern(instr, "EEEE 1100011 1LI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_wrfast(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Begin new fast hub write via FIFO."));
    m_instr.insert(instr, "WRFAST  {#}D,{#}S");
    m_token.insert(instr, t_WRFAST);
    pattern(instr, "EEEE 1100100 0LI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_fblock(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Set next block for when block wraps."));
    m_instr.insert(instr, "FBLOCK  {#}D,{#}S");
    m_token.insert(instr, t_FBLOCK);
    pattern(instr, "EEEE 1100100 1LI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_xinit(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Issue streamer command immediately, zeroing phase."));
    m_instr.insert(instr, "XINIT   {#}D,{#}S");
    m_token.insert(instr, t_XINIT);
    pattern(instr, "EEEE 1100101 0LI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_xstop(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Stop streamer immediately."));
    m_instr.insert(instr, "XSTOP");
    m_token.insert(instr, t_XSTOP);
    pattern(instr, "EEEE 1100101 011 000000000 000000000");
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
void P2Doc::doc_xzero(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Buffer new streamer command to be issued on final NCO rollover of current command, zeroing phase."));
    m_instr.insert(instr, "XZERO   {#}D,{#}S");
    m_token.insert(instr, t_XZERO);
    pattern(instr, "EEEE 1100101 1LI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_xcont(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Buffer new streamer command to be issued on final NCO rollover of current command, continuing phase."));
    m_instr.insert(instr, "XCONT   {#}D,{#}S");
    m_token.insert(instr, t_XCONT);
    pattern(instr, "EEEE 1100110 0LI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_rep(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Execute next D[8:0] instructions S times."));
    m_instr.insert(instr, "REP     {#}D,{#}S");
    m_token.insert(instr, t_REP);
    pattern(instr, "EEEE 1100110 1LI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_coginit(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Start cog selected by D."));
    m_instr.insert(instr, "COGINIT {#}D,{#}S       {WC}");
    m_token.insert(instr, t_COGINIT);
    pattern(instr, "EEEE 1100111 CLI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_qmul(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Begin CORDIC unsigned multiplication of D * S."));
    m_instr.insert(instr, "QMUL    {#}D,{#}S");
    m_token.insert(instr, t_QMUL);
    pattern(instr, "EEEE 1101000 0LI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_qdiv(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Begin CORDIC unsigned division of {SETQ value or 32'b0, D} / S."));
    m_instr.insert(instr, "QDIV    {#}D,{#}S");
    m_token.insert(instr, t_QDIV);
    pattern(instr, "EEEE 1101000 1LI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_qfrac(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Begin CORDIC unsigned division of {D, SETQ value or 32'b0} / S."));
    m_instr.insert(instr, "QFRAC   {#}D,{#}S");
    m_token.insert(instr, t_QFRAC);
    pattern(instr, "EEEE 1101001 0LI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_qsqrt(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Begin CORDIC square root of {S, D}."));
    m_instr.insert(instr, "QSQRT   {#}D,{#}S");
    m_token.insert(instr, t_QSQRT);
    pattern(instr, "EEEE 1101001 1LI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_qrotate(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Begin CORDIC rotation of point (D, SETQ value or 32'b0) by angle S."));
    m_instr.insert(instr, "QROTATE {#}D,{#}S");
    m_token.insert(instr, t_QROTATE);
    pattern(instr, "EEEE 1101010 0LI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_qvector(p2_inst8_e instr)
{
    m_brief.insert(instr, tr("Begin CORDIC vectoring of point (D, S)."));
    m_instr.insert(instr, "QVECTOR {#}D,{#}S");
    m_token.insert(instr, t_QVECTOR);
    pattern(instr, "EEEE 1101010 1LI DDDDDDDDD SSSSSSSSS");
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
void P2Doc::doc_hubset(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Set hub configuration to D."));
    m_instr.insert(instr, "HUBSET  {#}D");
    m_token.insert(instr, t_HUBSET);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000000000");
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
void P2Doc::doc_cogid(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("If D is register and no WC, get cog ID (0 to 15) into D."));
    m_instr.insert(instr, "COGID   {#}D            {WC}");
    m_token.insert(instr, t_COGID);
    pattern(instr, "EEEE 1101011 C0L DDDDDDDDD 000000001");
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
void P2Doc::doc_cogstop(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Stop cog D[3:0]."));
    m_instr.insert(instr, "COGSTOP {#}D");
    m_token.insert(instr, t_COGSTOP);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000000011");
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
void P2Doc::doc_locknew(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Request a LOCK."));
    m_instr.insert(instr, "LOCKNEW D               {WC}");
    m_token.insert(instr, t_LOCKNEW);
    pattern(instr, "EEEE 1101011 C00 DDDDDDDDD 000000100");
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
void P2Doc::doc_lockret(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Return LOCK D[3:0] for reallocation."));
    m_instr.insert(instr, "LOCKRET {#}D");
    m_token.insert(instr, t_LOCKRET);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000000101");
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
void P2Doc::doc_locktry(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Try to get LOCK D[3:0]."));
    m_instr.insert(instr, "LOCKTRY {#}D            {WC}");
    m_token.insert(instr, t_LOCKTRY);
    pattern(instr, "EEEE 1101011 C0L DDDDDDDDD 000000110");
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
void P2Doc::doc_lockrel(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Release LOCK D[3:0]."));
    m_instr.insert(instr, "LOCKREL {#}D            {WC}");
    m_token.insert(instr, t_LOCKREL);
    pattern(instr, "EEEE 1101011 C0L DDDDDDDDD 000000111");
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
void P2Doc::doc_qlog(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Begin CORDIC number-to-logarithm conversion of D."));
    m_instr.insert(instr, "QLOG    {#}D");
    m_token.insert(instr, t_QLOG);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000001110");
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
void P2Doc::doc_qexp(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Begin CORDIC logarithm-to-number conversion of D."));
    m_instr.insert(instr, "QEXP    {#}D");
    m_token.insert(instr, t_QEXP);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000001111");
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
void P2Doc::doc_rfbyte(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Read zero-extended byte from FIFO into D. Used after RDFAST."));
    m_instr.insert(instr, "RFBYTE  D        {WC/WZ/WCZ}");
    m_token.insert(instr, t_RFBYTE);
    pattern(instr, "EEEE 1101011 CZ0 DDDDDDDDD 000010000");
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
void P2Doc::doc_rfword(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Read zero-extended word from FIFO into D. Used after RDFAST."));
    m_instr.insert(instr, "RFWORD  D        {WC/WZ/WCZ}");
    m_token.insert(instr, t_RFWORD);
    pattern(instr, "EEEE 1101011 CZ0 DDDDDDDDD 000010001");
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
void P2Doc::doc_rflong(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Read long from FIFO into D. Used after RDFAST."));
    m_instr.insert(instr, "RFLONG  D        {WC/WZ/WCZ}");
    m_token.insert(instr, t_RFLONG);
    pattern(instr, "EEEE 1101011 CZ0 DDDDDDDDD 000010010");
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
void P2Doc::doc_rfvar(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Read zero-extended 1..4-byte value from FIFO into D. Used after RDFAST."));
    m_instr.insert(instr, "RFVAR   D        {WC/WZ/WCZ}");
    m_token.insert(instr, t_RFVAR);
    pattern(instr, "EEEE 1101011 CZ0 DDDDDDDDD 000010011");
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
void P2Doc::doc_rfvars(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Read sign-extended 1..4-byte value from FIFO into D. Used after RDFAST."));
    m_instr.insert(instr, "RFVARS  D        {WC/WZ/WCZ}");

    pattern(instr, "EEEE 1101011 CZ0 DDDDDDDDD 000010100");
    m_token.insert(instr, t_RFVARS);
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
void P2Doc::doc_wfbyte(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Write byte in D[7:0] into FIFO. Used after WRFAST."));
    m_instr.insert(instr, "WFBYTE  {#}D");
    m_token.insert(instr, t_WFBYTE);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000010101");
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
void P2Doc::doc_wfword(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Write word in D[15:0] into FIFO. Used after WRFAST."));
    m_instr.insert(instr, "WFWORD  {#}D");
    m_token.insert(instr, t_WFWORD);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000010110");
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
void P2Doc::doc_wflong(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Write long in D[31:0] into FIFO. Used after WRFAST."));
    m_instr.insert(instr, "WFLONG  {#}D");
    m_token.insert(instr, t_WFLONG);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000010111");
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
void P2Doc::doc_getqx(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Retrieve CORDIC result X into D."));
    m_instr.insert(instr, "GETQX   D        {WC/WZ/WCZ}");
    m_token.insert(instr, t_GETQX);
    pattern(instr, "EEEE 1101011 CZ0 DDDDDDDDD 000011000");
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
void P2Doc::doc_getqy(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Retrieve CORDIC result Y into D."));
    m_instr.insert(instr, "GETQY   D        {WC/WZ/WCZ}");
    m_token.insert(instr, t_GETQY);
    pattern(instr, "EEEE 1101011 CZ0 DDDDDDDDD 000011001");
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
void P2Doc::doc_getct(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Get CT into D."));
    m_instr.insert(instr, "GETCT   D");
    m_token.insert(instr, t_GETCT);
    pattern(instr, "EEEE 1101011 000 DDDDDDDDD 000011010");
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
void P2Doc::doc_getrnd(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Get RND into D/C/Z."));
    m_instr.insert(instr, "GETRND  D        {WC/WZ/WCZ}");
    m_token.insert(instr, t_GETRND);
    pattern(instr, "EEEE 1101011 CZ0 DDDDDDDDD 000011011");
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
void P2Doc::doc_getrnd_cz(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Get RND into C/Z."));
    m_instr.insert(instr, "GETRND            WC/WZ/WCZ");
    m_token.insert(instr, t_GETRND);
    pattern(instr, "EEEE 1101011 CZ1 000000000 000011011");
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
void P2Doc::doc_setdacs(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("DAC3 = D[31:24], DAC2 = D[23:16], DAC1 = D[15:8], DAC0 = D[7:0]."));
    m_instr.insert(instr, "SETDACS {#}D");
    m_token.insert(instr, t_SETDACS);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000011100");
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
void P2Doc::doc_setxfrq(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Set streamer NCO frequency to D."));
    m_instr.insert(instr, "SETXFRQ {#}D");
    m_token.insert(instr, t_SETXFRQ);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000011101");
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
void P2Doc::doc_getxacc(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Get the streamer's Goertzel X accumulator into D and the Y accumulator into the next instruction's S, clear accumulators."));
    m_instr.insert(instr, "GETXACC D");
    m_token.insert(instr, t_GETXACC);
    pattern(instr, "EEEE 1101011 000 DDDDDDDDD 000011110");
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
void P2Doc::doc_waitx(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Wait 2 + D clocks if no WC/WZ/WCZ."));
    m_instr.insert(instr, "WAITX   {#}D     {WC/WZ/WCZ}");
    m_token.insert(instr, t_WAITX);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000011111");
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
void P2Doc::doc_setse1(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Set SE1 event configuration to D[8:0]."));
    m_instr.insert(instr, "SETSE1  {#}D");
    m_token.insert(instr, t_SETSE1);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000100000");
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
void P2Doc::doc_setse2(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Set SE2 event configuration to D[8:0]."));
    m_instr.insert(instr, "SETSE2  {#}D");
    m_token.insert(instr, t_SETSE2);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000100001");
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
void P2Doc::doc_setse3(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Set SE3 event configuration to D[8:0]."));
    m_instr.insert(instr, "SETSE3  {#}D");
    m_token.insert(instr, t_SETSE3);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000100010");
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
void P2Doc::doc_setse4(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Set SE4 event configuration to D[8:0]."));
    m_instr.insert(instr, "SETSE4  {#}D");
    m_token.insert(instr, t_SETSE4);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000100011");
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
void P2Doc::doc_pollint(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Get INT event flag into C/Z, then clear it."));
    m_instr.insert(instr, "POLLINT          {WC/WZ/WCZ}");
    m_token.insert(instr, t_POLLINT);
    pattern(instr, "EEEE 1101011 CZ0 000000000 000100100");
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
void P2Doc::doc_pollct1(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Get CT1 event flag into C/Z, then clear it."));
    m_instr.insert(instr, "POLLCT1          {WC/WZ/WCZ}");
    m_token.insert(instr, t_POLLCT1);
    pattern(instr, "EEEE 1101011 CZ0 000000001 000100100");
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
void P2Doc::doc_pollct2(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Get CT2 event flag into C/Z, then clear it."));
    m_instr.insert(instr, "POLLCT2          {WC/WZ/WCZ}");
    m_token.insert(instr, t_POLLCT2);
    pattern(instr, "EEEE 1101011 CZ0 000000010 000100100");
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
void P2Doc::doc_pollct3(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Get CT3 event flag into C/Z, then clear it."));
    m_instr.insert(instr, "POLLCT3          {WC/WZ/WCZ}");
    m_token.insert(instr, t_POLLCT3);
    pattern(instr, "EEEE 1101011 CZ0 000000011 000100100");
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
void P2Doc::doc_pollse1(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Get SE1 event flag into C/Z, then clear it."));
    m_instr.insert(instr, "POLLSE1          {WC/WZ/WCZ}");
    m_token.insert(instr, t_POLLSE1);
    pattern(instr, "EEEE 1101011 CZ0 000000100 000100100");
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
void P2Doc::doc_pollse2(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Get SE2 event flag into C/Z, then clear it."));
    m_instr.insert(instr, "POLLSE2          {WC/WZ/WCZ}");
    m_token.insert(instr, t_POLLSE2);
    pattern(instr, "EEEE 1101011 CZ0 000000101 000100100");
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
void P2Doc::doc_pollse3(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Get SE3 event flag into C/Z, then clear it."));
    m_instr.insert(instr, "POLLSE3          {WC/WZ/WCZ}");
    m_token.insert(instr, t_POLLSE3);
    pattern(instr, "EEEE 1101011 CZ0 000000110 000100100");
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
void P2Doc::doc_pollse4(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Get SE4 event flag into C/Z, then clear it."));
    m_instr.insert(instr, "POLLSE4          {WC/WZ/WCZ}");
    m_token.insert(instr, t_POLLSE4);
    pattern(instr, "EEEE 1101011 CZ0 000000111 000100100");
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
void P2Doc::doc_pollpat(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Get PAT event flag into C/Z, then clear it."));
    m_instr.insert(instr, "POLLPAT          {WC/WZ/WCZ}");
    m_token.insert(instr, t_POLLPAT);
    pattern(instr, "EEEE 1101011 CZ0 000001000 000100100");
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
void P2Doc::doc_pollfbw(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Get FBW event flag into C/Z, then clear it."));
    m_instr.insert(instr, "POLLFBW          {WC/WZ/WCZ}");
    m_token.insert(instr, t_POLLFBW);
    pattern(instr, "EEEE 1101011 CZ0 000001001 000100100");
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
void P2Doc::doc_pollxmt(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Get XMT event flag into C/Z, then clear it."));
    m_instr.insert(instr, "POLLXMT          {WC/WZ/WCZ}");
    m_token.insert(instr, t_POLLXMT);
    pattern(instr, "EEEE 1101011 CZ0 000001010 000100100");
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
void P2Doc::doc_pollxfi(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Get XFI event flag into C/Z, then clear it."));
    m_instr.insert(instr, "POLLXFI          {WC/WZ/WCZ}");
    m_token.insert(instr, t_POLLXFI);
    pattern(instr, "EEEE 1101011 CZ0 000001011 000100100");
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
void P2Doc::doc_pollxro(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Get XRO event flag into C/Z, then clear it."));
    m_instr.insert(instr, "POLLXRO          {WC/WZ/WCZ}");
    m_token.insert(instr, t_POLLXRO);
    pattern(instr, "EEEE 1101011 CZ0 000001100 000100100");
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
void P2Doc::doc_pollxrl(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Get XRL event flag into C/Z, then clear it."));
    m_instr.insert(instr, "POLLXRL          {WC/WZ/WCZ}");
    m_token.insert(instr, t_POLLXRL);
    pattern(instr, "EEEE 1101011 CZ0 000001101 000100100");
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
void P2Doc::doc_pollatn(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Get ATN event flag into C/Z, then clear it."));
    m_instr.insert(instr, "POLLATN          {WC/WZ/WCZ}");
    m_token.insert(instr, t_POLLATN);
    pattern(instr, "EEEE 1101011 CZ0 000001110 000100100");
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
void P2Doc::doc_pollqmt(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Get QMT event flag into C/Z, then clear it."));
    m_instr.insert(instr, "POLLQMT          {WC/WZ/WCZ}");
    m_token.insert(instr, t_POLLQMT);
    pattern(instr, "EEEE 1101011 CZ0 000001111 000100100");
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
void P2Doc::doc_waitint(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Wait for INT event flag, then clear it."));
    m_instr.insert(instr, "WAITINT          {WC/WZ/WCZ}");
    m_token.insert(instr, t_WAITINT);
    pattern(instr, "EEEE 1101011 CZ0 000010000 000100100");
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
void P2Doc::doc_waitct1(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Wait for CT1 event flag, then clear it."));
    m_instr.insert(instr, "WAITCT1          {WC/WZ/WCZ}");
    m_token.insert(instr, t_WAITCT1);
    pattern(instr, "EEEE 1101011 CZ0 000010001 000100100");
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
void P2Doc::doc_waitct2(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Wait for CT2 event flag, then clear it."));
    m_instr.insert(instr, "WAITCT2          {WC/WZ/WCZ}");
    m_token.insert(instr, t_WAITCT2);
    pattern(instr, "EEEE 1101011 CZ0 000010010 000100100");
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
void P2Doc::doc_waitct3(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Wait for CT3 event flag, then clear it."));
    m_instr.insert(instr, "WAITCT3          {WC/WZ/WCZ}");
    m_token.insert(instr, t_WAITCT3);
    pattern(instr, "EEEE 1101011 CZ0 000010011 000100100");
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
void P2Doc::doc_waitse1(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Wait for SE1 event flag, then clear it."));
    m_instr.insert(instr, "WAITSE1          {WC/WZ/WCZ}");
    m_token.insert(instr, t_WAITSE1);
    pattern(instr, "EEEE 1101011 CZ0 000010100 000100100");
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
void P2Doc::doc_waitse2(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Wait for SE2 event flag, then clear it."));
    m_instr.insert(instr, "WAITSE2          {WC/WZ/WCZ}");
    m_token.insert(instr, t_WAITSE2);
    pattern(instr, "EEEE 1101011 CZ0 000010101 000100100");
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
void P2Doc::doc_waitse3(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Wait for SE3 event flag, then clear it."));
    m_instr.insert(instr, "WAITSE3          {WC/WZ/WCZ}");
    m_token.insert(instr, t_WAITSE3);
    pattern(instr, "EEEE 1101011 CZ0 000010110 000100100");
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
void P2Doc::doc_waitse4(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Wait for SE4 event flag, then clear it."));
    m_instr.insert(instr, "WAITSE4          {WC/WZ/WCZ}");
    m_token.insert(instr, t_WAITSE4);
    pattern(instr, "EEEE 1101011 CZ0 000010111 000100100");
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
void P2Doc::doc_waitpat(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Wait for PAT event flag, then clear it."));
    m_instr.insert(instr, "WAITPAT          {WC/WZ/WCZ}");
    m_token.insert(instr, t_WAITPAT);
    pattern(instr, "EEEE 1101011 CZ0 000011000 000100100");
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
void P2Doc::doc_waitfbw(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Wait for FBW event flag, then clear it."));
    m_instr.insert(instr, "WAITFBW          {WC/WZ/WCZ}");
    m_token.insert(instr, t_WAITFBW);
    pattern(instr, "EEEE 1101011 CZ0 000011001 000100100");
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
void P2Doc::doc_waitxmt(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Wait for XMT event flag, then clear it."));
    m_instr.insert(instr, "WAITXMT          {WC/WZ/WCZ}");
    m_token.insert(instr, t_WAITXMT);
    pattern(instr, "EEEE 1101011 CZ0 000011010 000100100");
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
void P2Doc::doc_waitxfi(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Wait for XFI event flag, then clear it."));
    m_instr.insert(instr, "WAITXFI          {WC/WZ/WCZ}");
    m_token.insert(instr, t_WAITXFI);
    pattern(instr, "EEEE 1101011 CZ0 000011011 000100100");
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
void P2Doc::doc_waitxro(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Wait for XRO event flag, then clear it."));
    m_instr.insert(instr, "WAITXRO          {WC/WZ/WCZ}");
    m_token.insert(instr, t_WAITXRO);
    pattern(instr, "EEEE 1101011 CZ0 000011100 000100100");
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
void P2Doc::doc_waitxrl(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Wait for XRL event flag, then clear it."));
    m_instr.insert(instr, "WAITXRL          {WC/WZ/WCZ}");
    m_token.insert(instr, t_WAITXRL);
    pattern(instr, "EEEE 1101011 CZ0 000011101 000100100");
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
void P2Doc::doc_waitatn(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Wait for ATN event flag, then clear it."));
    m_instr.insert(instr, "WAITATN          {WC/WZ/WCZ}");
    m_token.insert(instr, t_WAITATN);
    pattern(instr, "EEEE 1101011 CZ0 000011110 000100100");
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
void P2Doc::doc_allowi(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Allow interrupts (default)."));
    m_instr.insert(instr, "ALLOWI");
    m_token.insert(instr, t_ALLOWI);
    pattern(instr, "EEEE 1101011 000 000100000 000100100");
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
void P2Doc::doc_stalli(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Stall Interrupts."));
    m_instr.insert(instr, "STALLI");
    m_token.insert(instr, t_STALLI);
    pattern(instr, "EEEE 1101011 000 000100001 000100100");
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
void P2Doc::doc_trgint1(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Trigger INT1, regardless of STALLI mode."));
    m_instr.insert(instr, "TRGINT1");
    m_token.insert(instr, t_TRGINT1);
    pattern(instr, "EEEE 1101011 000 000100010 000100100");
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
void P2Doc::doc_trgint2(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Trigger INT2, regardless of STALLI mode."));
    m_instr.insert(instr, "TRGINT2");
    m_token.insert(instr, t_TRGINT2);
    pattern(instr, "EEEE 1101011 000 000100011 000100100");
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
void P2Doc::doc_trgint3(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Trigger INT3, regardless of STALLI mode."));
    m_instr.insert(instr, "TRGINT3");
    m_token.insert(instr, t_TRGINT3);
    pattern(instr, "EEEE 1101011 000 000100100 000100100");
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
void P2Doc::doc_nixint1(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Cancel INT1."));
    m_instr.insert(instr, "NIXINT1");
    m_token.insert(instr, t_NIXINT1);
    pattern(instr, "EEEE 1101011 000 000100101 000100100");
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
void P2Doc::doc_nixint2(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Cancel INT2."));
    m_instr.insert(instr, "NIXINT2");
    m_token.insert(instr, t_NIXINT2);
    pattern(instr, "EEEE 1101011 000 000100110 000100100");
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
void P2Doc::doc_nixint3(p2_opx24_e instr)
{
    m_brief.insert(instr, tr("Cancel INT3."));
    m_instr.insert(instr, "NIXINT3");
    m_token.insert(instr, t_NIXINT3);
    pattern(instr, "EEEE 1101011 000 000100111 000100100");
}

/**
 * @brief Set INT1 source to D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100101
 *
 * SETINT1 {#}D
 *
 *
 */
void P2Doc::doc_setint1(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Set INT1 source to D[3:0]."));
    m_instr.insert(instr, "SETINT1 {#}D");
    m_token.insert(instr, t_SETINT1);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000100101");
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
void P2Doc::doc_setint2(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Set INT2 source to D[3:0]."));
    m_instr.insert(instr, "SETINT2 {#}D");
    m_token.insert(instr, t_SETINT2);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000100110");
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
void P2Doc::doc_setint3(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Set INT3 source to D[3:0]."));
    m_instr.insert(instr, "SETINT3 {#}D");
    m_token.insert(instr, t_SETINT3);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000100111");
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
void P2Doc::doc_setq(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Set Q to D."));
    m_instr.insert(instr, "SETQ    {#}D");
    m_token.insert(instr, t_SETQ);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000101000");
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
void P2Doc::doc_setq2(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Set Q to D."));
    m_instr.insert(instr, "SETQ2   {#}D");
    m_token.insert(instr, t_SETQ2);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000101001");
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
void P2Doc::doc_push(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Push D onto stack."));
    m_instr.insert(instr, "PUSH    {#}D");
    m_token.insert(instr, t_PUSH);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000101010");
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
void P2Doc::doc_pop(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Pop stack (K)."));
    m_instr.insert(instr, "POP     D        {WC/WZ/WCZ}");
    m_token.insert(instr, t_POP);
    pattern(instr, "EEEE 1101011 CZ0 DDDDDDDDD 000101011");
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
void P2Doc::doc_jmp(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Jump to D."));
    m_instr.insert(instr, "JMP     D        {WC/WZ/WCZ}");
    m_token.insert(instr, t_JMP);
    pattern(instr, "EEEE 1101011 CZ0 DDDDDDDDD 000101100");
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
void P2Doc::doc_call(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Call to D by pushing {C, Z, 10'b0, PC[19:0]} onto stack."));
    m_instr.insert(instr, "CALL    D        {WC/WZ/WCZ}");
    m_token.insert(instr, t_CALL);
    pattern(instr, "EEEE 1101011 CZ0 DDDDDDDDD 000101101");
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
void P2Doc::doc_ret(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Return by popping stack (K)."));
    m_instr.insert(instr, "RET              {WC/WZ/WCZ}");
    m_token.insert(instr, t_RET);
    pattern(instr, "EEEE 1101011 CZ1 000000000 000101101");
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
void P2Doc::doc_calla(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Call to D by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRA++."));
    m_instr.insert(instr, "CALLA   D        {WC/WZ/WCZ}");
    m_token.insert(instr, t_CALLA);
    pattern(instr, "EEEE 1101011 CZ0 DDDDDDDDD 000101110");
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
void P2Doc::doc_reta(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Return by reading hub long (L) at --PTRA."));
    m_instr.insert(instr, "RETA             {WC/WZ/WCZ}");
    m_token.insert(instr, t_RETA);
    pattern(instr, "EEEE 1101011 CZ1 000000000 000101110");
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
void P2Doc::doc_callb(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Call to D by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRB++."));
    m_instr.insert(instr, "CALLB   D        {WC/WZ/WCZ}");
    m_token.insert(instr, t_CALLB);
    pattern(instr, "EEEE 1101011 CZ0 DDDDDDDDD 000101111");
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
void P2Doc::doc_retb(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Return by reading hub long (L) at --PTRB."));
    m_instr.insert(instr, "RETB             {WC/WZ/WCZ}");
    m_token.insert(instr, t_RETB);
    pattern(instr, "EEEE 1101011 CZ1 000000000 000101111");
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
void P2Doc::doc_jmprel(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Jump ahead/back by D instructions."));
    m_instr.insert(instr, "JMPREL  {#}D");
    m_token.insert(instr, t_JMPREL);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000110000");
}

/**
 * @brief Skip instructions per D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110001
 *
 * SKIP    {#}D
 *
 * Subsequent instructions 0.
 * 31 get cancelled for each '1' bit in D[0].
 * D[31].
 *
 */
void P2Doc::doc_skip(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Skip instructions per D."));
    m_instr.insert(instr, "SKIP    {#}D");
    m_token.insert(instr, t_SKIP);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000110001");
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
void P2Doc::doc_skipf(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Skip cog/LUT instructions fast per D."));
    m_instr.insert(instr, "SKIPF   {#}D");
    m_token.insert(instr, t_SKIPF);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000110010");
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
void P2Doc::doc_execf(p2_opsrc_e instr)
{
    m_instr.insert(instr, "EXECF   {#}D");
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000110011");
    m_brief.insert(instr, tr("Jump to D[9:0] in cog/LUT and set SKIPF pattern to D[31:10]."));
    m_token.insert(instr, t_EXECF);
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
void P2Doc::doc_getptr(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Get current FIFO hub pointer into D."));
    m_instr.insert(instr, "GETPTR  D");
    m_token.insert(instr, t_GETPTR);
    pattern(instr, "EEEE 1101011 000 DDDDDDDDD 000110100");
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
void P2Doc::doc_getbrk(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Get breakpoint status into D according to WC/WZ/WCZ."));
    m_instr.insert(instr, "GETBRK  D          WC/WZ/WCZ");
    m_token.insert(instr, t_GETBRK);
    pattern(instr, "EEEE 1101011 CZ0 DDDDDDDDD 000110101");
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
void P2Doc::doc_cogbrk(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("If in debug ISR, trigger asynchronous breakpoint in cog D[3:0]."));
    m_instr.insert(instr, "COGBRK  {#}D");
    m_token.insert(instr, t_COGBRK);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000110101");
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
void P2Doc::doc_brk(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("If in debug ISR, set next break condition to D."));
    m_instr.insert(instr, "BRK     {#}D");
    m_token.insert(instr, t_BRK);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000110110");
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
void P2Doc::doc_setluts(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("If D[0] = 1 then enable LUT sharing, where LUT writes within the adjacent odd/even companion cog are copied to this LUT."));
    m_instr.insert(instr, "SETLUTS {#}D");
    m_token.insert(instr, t_SETLUTS);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000110111");
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
void P2Doc::doc_setcy(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Set the colorspace converter \"CY\" parameter to D[31:0]."));
    m_instr.insert(instr, "SETCY   {#}D");
    m_token.insert(instr, t_SETCY);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000111000");
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
void P2Doc::doc_setci(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Set the colorspace converter \"CI\" parameter to D[31:0]."));
    m_instr.insert(instr, "SETCI   {#}D");
    m_token.insert(instr, t_SETCI);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000111001");
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
void P2Doc::doc_setcq(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Set the colorspace converter \"CQ\" parameter to D[31:0]."));
    m_instr.insert(instr, "SETCQ   {#}D");
    m_token.insert(instr, t_SETCQ);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000111010");
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
void P2Doc::doc_setcfrq(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Set the colorspace converter \"CFRQ\" parameter to D[31:0]."));
    m_instr.insert(instr, "SETCFRQ {#}D");
    m_token.insert(instr, t_SETCFRQ);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000111011");
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
void P2Doc::doc_setcmod(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Set the colorspace converter \"CMOD\" parameter to D[6:0]."));
    m_instr.insert(instr, "SETCMOD {#}D");
    m_token.insert(instr, t_SETCMOD);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000111100");
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
void P2Doc::doc_setpiv(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Set BLNPIX/MIXPIX blend factor to D[7:0]."));
    m_instr.insert(instr, "SETPIV  {#}D");
    m_token.insert(instr, t_SETPIV);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000111101");
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
void P2Doc::doc_setpix(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Set MIXPIX mode to D[5:0]."));
    m_instr.insert(instr, "SETPIX  {#}D");
    m_token.insert(instr, t_SETPIX);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000111110");
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
void P2Doc::doc_cogatn(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Strobe \"attention\" of all cogs whose corresponging bits are high in D[15:0]."));
    m_instr.insert(instr, "COGATN  {#}D");
    m_token.insert(instr, t_COGATN);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 000111111");
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
void P2Doc::doc_testp_w(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Test  IN bit of pin D[5:0], write to C/Z."));
    m_instr.insert(instr, "TESTP   {#}D           WC/WZ");
    m_token.insert(instr, t_TESTP);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001000000");
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
void P2Doc::doc_testpn_w(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Test !IN bit of pin D[5:0], write to C/Z."));
    m_instr.insert(instr, "TESTPN  {#}D           WC/WZ");
    m_token.insert(instr, t_TESTPN);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001000001");
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
void P2Doc::doc_testp_and(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Test  IN bit of pin D[5:0], AND into C/Z."));
    m_instr.insert(instr, "TESTP   {#}D       ANDC/ANDZ");
    m_token.insert(instr, t_TESTP);    // t_AND
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001000010");
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
void P2Doc::doc_testpn_and(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Test !IN bit of pin D[5:0], AND into C/Z."));
    m_instr.insert(instr, "TESTPN  {#}D       ANDC/ANDZ");
    m_token.insert(instr, t_TESTPN);   // t_AND
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001000011");
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
void P2Doc::doc_testp_or(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Test  IN bit of pin D[5:0], OR  into C/Z."));
    m_instr.insert(instr, "TESTP   {#}D         ORC/ORZ");
    m_token.insert(instr, t_TESTP);    // t_OR
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001000100");
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
void P2Doc::doc_testpn_or(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Test !IN bit of pin D[5:0], OR  into C/Z."));
    m_instr.insert(instr, "TESTPN  {#}D         ORC/ORZ");
    m_token.insert(instr, t_TESTPN);   // t_OR
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001000101");
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
void P2Doc::doc_testp_xor(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Test  IN bit of pin D[5:0], XOR into C/Z."));
    m_instr.insert(instr, "TESTP   {#}D       XORC/XORZ");
    m_token.insert(instr, t_TESTP);    // t_XOR
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001000110");
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
void P2Doc::doc_testpn_xor(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Test !IN bit of pin D[5:0], XOR into C/Z."));
    m_instr.insert(instr, "TESTPN  {#}D       XORC/XORZ");
    m_token.insert(instr, t_TESTPN);   // t_XOR
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001000111");
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
void P2Doc::doc_dirl(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("DIR bit of pin D[5:0] = 0."));
    m_instr.insert(instr, "DIRL    {#}D           {WCZ}");
    m_token.insert(instr, t_DIRL);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001000000");
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
void P2Doc::doc_dirh(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("DIR bit of pin D[5:0] = 1."));
    m_instr.insert(instr, "DIRH    {#}D           {WCZ}");
    m_token.insert(instr, t_DIRH);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001000001");
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
void P2Doc::doc_dirc(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("DIR bit of pin D[5:0] = C."));
    m_instr.insert(instr, "DIRC    {#}D           {WCZ}");
    m_token.insert(instr, t_DIRC);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001000010");
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
void P2Doc::doc_dirnc(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("DIR bit of pin D[5:0] = !C."));
    m_instr.insert(instr, "DIRNC   {#}D           {WCZ}");
    m_token.insert(instr, t_DIRNC);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001000011");
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
void P2Doc::doc_dirz(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("DIR bit of pin D[5:0] = Z."));
    m_instr.insert(instr, "DIRZ    {#}D           {WCZ}");
    m_token.insert(instr, t_DIRZ);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001000100");
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
void P2Doc::doc_dirnz(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("DIR bit of pin D[5:0] = !Z."));
    m_instr.insert(instr, "DIRNZ   {#}D           {WCZ}");
    m_token.insert(instr, t_DIRNZ);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001000101");
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
void P2Doc::doc_dirrnd(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("DIR bit of pin D[5:0] = RND."));
    m_instr.insert(instr, "DIRRND  {#}D           {WCZ}");
    m_token.insert(instr, t_DIRRND);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001000110");
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
void P2Doc::doc_dirnot(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("DIR bit of pin D[5:0] = !bit."));
    m_instr.insert(instr, "DIRNOT  {#}D           {WCZ}");
    m_token.insert(instr, t_DIRNOT);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001000111");
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
void P2Doc::doc_outl(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("OUT bit of pin D[5:0] = 0."));
    m_instr.insert(instr, "OUTL    {#}D           {WCZ}");
    m_token.insert(instr, t_OUTL);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001001000");
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
void P2Doc::doc_outh(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("OUT bit of pin D[5:0] = 1."));
    m_instr.insert(instr, "OUTH    {#}D           {WCZ}");
    m_token.insert(instr, t_OUTH);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001001001");
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
void P2Doc::doc_outc(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("OUT bit of pin D[5:0] = C."));
    m_instr.insert(instr, "OUTC    {#}D           {WCZ}");
    m_token.insert(instr, t_OUTC);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001001010");
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
void P2Doc::doc_outnc(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("OUT bit of pin D[5:0] = !C."));
    m_instr.insert(instr, "OUTNC   {#}D           {WCZ}");
    m_token.insert(instr, t_OUTNC);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001001011");
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
void P2Doc::doc_outz(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("OUT bit of pin D[5:0] = Z."));
    m_instr.insert(instr, "OUTZ    {#}D           {WCZ}");
    m_token.insert(instr, t_OUTZ);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001001100");
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
void P2Doc::doc_outnz(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("OUT bit of pin D[5:0] = !Z."));
    m_instr.insert(instr, "OUTNZ   {#}D           {WCZ}");
    m_token.insert(instr, t_OUTNZ);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001001101");
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
void P2Doc::doc_outrnd(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("OUT bit of pin D[5:0] = RND."));
    m_instr.insert(instr, "OUTRND  {#}D           {WCZ}");
    m_token.insert(instr, t_OUTRND);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001001110");
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
void P2Doc::doc_outnot(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("OUT bit of pin D[5:0] = !bit."));
    m_instr.insert(instr, "OUTNOT  {#}D           {WCZ}");
    m_token.insert(instr, t_OUTNOT);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001001111");
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
void P2Doc::doc_fltl(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("OUT bit of pin D[5:0] = 0."));
    m_instr.insert(instr, "FLTL    {#}D           {WCZ}");
    m_token.insert(instr, t_FLTL);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001010000");
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
void P2Doc::doc_flth(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("OUT bit of pin D[5:0] = 1."));
    m_instr.insert(instr, "FLTH    {#}D           {WCZ}");
    m_token.insert(instr, t_FLTH);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001010001");
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
void P2Doc::doc_fltc(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("OUT bit of pin D[5:0] = C."));
    m_instr.insert(instr, "FLTC    {#}D           {WCZ}");
    m_token.insert(instr, t_FLTC);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001010010");
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
void P2Doc::doc_fltnc(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("OUT bit of pin D[5:0] = !C."));
    m_instr.insert(instr, "FLTNC   {#}D           {WCZ}");
    m_token.insert(instr, t_FLTNC);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001010011");
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
void P2Doc::doc_fltz(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("OUT bit of pin D[5:0] = Z."));
    m_instr.insert(instr, "FLTZ    {#}D           {WCZ}");
    m_token.insert(instr, t_FLTZ);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001010100");
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
void P2Doc::doc_fltnz(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("OUT bit of pin D[5:0] = !Z."));
    m_instr.insert(instr, "FLTNZ   {#}D           {WCZ}");
    m_token.insert(instr, t_FLTNZ);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001010101");
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
void P2Doc::doc_fltrnd(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("OUT bit of pin D[5:0] = RND."));
    m_instr.insert(instr, "FLTRND  {#}D           {WCZ}");
    m_token.insert(instr, t_FLTRND);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001010110");
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
void P2Doc::doc_fltnot(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("OUT bit of pin D[5:0] = !bit."));
    m_instr.insert(instr, "FLTNOT  {#}D           {WCZ}");
    m_token.insert(instr, t_FLTNOT);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001010111");
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
void P2Doc::doc_drvl(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("OUT bit of pin D[5:0] = 0."));
    m_instr.insert(instr, "DRVL    {#}D           {WCZ}");
    m_token.insert(instr, t_DRVL);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001011000");
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
void P2Doc::doc_drvh(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("OUT bit of pin D[5:0] = 1."));
    m_instr.insert(instr, "DRVH    {#}D           {WCZ}");
    m_token.insert(instr, t_DRVH);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001011001");
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
void P2Doc::doc_drvc(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("OUT bit of pin D[5:0] = C."));
    m_instr.insert(instr, "DRVC    {#}D           {WCZ}");
    m_token.insert(instr, t_DRVC);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001011010");
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
void P2Doc::doc_drvnc(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("OUT bit of pin D[5:0] = !C."));
    m_instr.insert(instr, "DRVNC   {#}D           {WCZ}");
    m_token.insert(instr, t_DRVNC);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001011011");
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
void P2Doc::doc_drvz(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("OUT bit of pin D[5:0] = Z."));
    m_instr.insert(instr, "DRVZ    {#}D           {WCZ}");
    m_token.insert(instr, t_DRVZ);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001011100");
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
void P2Doc::doc_drvnz(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("OUT bit of pin D[5:0] = !Z."));
    m_instr.insert(instr, "DRVNZ   {#}D           {WCZ}");
    m_token.insert(instr, t_DRVNZ);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001011101");
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
void P2Doc::doc_drvrnd(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("OUT bit of pin D[5:0] = RND."));
    m_instr.insert(instr, "DRVRND  {#}D           {WCZ}");
    m_token.insert(instr, t_DRVRND);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001011110");
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
void P2Doc::doc_drvnot(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("OUT bit of pin D[5:0] = !bit."));
    m_instr.insert(instr, "DRVNOT  {#}D           {WCZ}");
    m_token.insert(instr, t_DRVNOT);
    pattern(instr, "EEEE 1101011 CZL DDDDDDDDD 001011111");
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
void P2Doc::doc_splitb(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Split every 4th bit of S into bytes of D."));
    m_instr.insert(instr, "SPLITB  D");
    m_token.insert(instr, t_SPLITB);
    pattern(instr, "EEEE 1101011 000 DDDDDDDDD 001100000");
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
void P2Doc::doc_mergeb(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Merge bits of bytes in S into D."));
    m_instr.insert(instr, "MERGEB  D");
    m_token.insert(instr, t_MERGEB);
    pattern(instr, "EEEE 1101011 000 DDDDDDDDD 001100001");
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
void P2Doc::doc_splitw(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Split bits of S into words of D."));
    m_instr.insert(instr, "SPLITW  D");
    m_token.insert(instr, t_SPLITW);
    pattern(instr, "EEEE 1101011 000 DDDDDDDDD 001100010");
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
void P2Doc::doc_mergew(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Merge bits of words in S into D."));
    m_instr.insert(instr, "MERGEW  D");
    m_token.insert(instr, t_MERGEW);
    pattern(instr, "EEEE 1101011 000 DDDDDDDDD 001100011");
}

/**
 * @brief Relocate and periodically invert bits from S into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100100
 *
 * SEUSSF  D
 *
 * Returns to original value on 32nd iteration.
    m_instr.insert(instr, QStringLiteral("SEUSSF  D;
 *
 * Forward pattern.
 */
void P2Doc::doc_seussf(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Relocate and periodically invert bits from S into D."));
    pattern(instr, "EEEE 1101011 000 DDDDDDDDD 001100100");
    m_token.insert(instr, t_SEUSSF);
}

/**
 * @brief Relocate and periodically invert bits from S into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100101
 *
 * SEUSSR  D
 *
 * Returns to original value on 32nd iteration.
    m_instr.insert(instr, QStringLiteral("SEUSSR  D;
 *
 * Reverse pattern.
 */
void P2Doc::doc_seussr(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Relocate and periodically invert bits from S into D."));
    pattern(instr, "EEEE 1101011 000 DDDDDDDDD 001100101");
    m_token.insert(instr, t_SEUSSR);
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
void P2Doc::doc_rgbsqz(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Squeeze 8:8:8 RGB value in S[31:8] into 5:6:5 value in D[15:0]."));
    m_instr.insert(instr, "RGBSQZ  D");
    m_token.insert(instr, t_RGBSQZ);
    pattern(instr, "EEEE 1101011 000 DDDDDDDDD 001100110");
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
void P2Doc::doc_rgbexp(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Expand 5:6:5 RGB value in S[15:0] into 8:8:8 value in D[31:8]."));
    m_instr.insert(instr, "RGBEXP  D");
    m_token.insert(instr, t_RGBEXP);
    pattern(instr, "EEEE 1101011 000 DDDDDDDDD 001100111");
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
void P2Doc::doc_xoro32(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Iterate D with xoroshiro32+ PRNG algorithm and put PRNG result into next instruction's S."));
    m_instr.insert(instr, "XORO32  D");
    m_token.insert(instr, t_XORO32);
    pattern(instr, "EEEE 1101011 000 DDDDDDDDD 001101000");
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
void P2Doc::doc_rev(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Reverse D bits."));
    m_instr.insert(instr, "REV     D");
    m_token.insert(instr, t_REV);
    pattern(instr, "EEEE 1101011 000 DDDDDDDDD 001101001");
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
void P2Doc::doc_rczr(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Rotate C,Z right through D."));
    m_instr.insert(instr, "RCZR    D        {WC/WZ/WCZ}");
    m_token.insert(instr, t_RCZR);
    pattern(instr, "EEEE 1101011 CZ0 DDDDDDDDD 001101010");
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
void P2Doc::doc_rczl(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Rotate C,Z left through D."));
    m_instr.insert(instr, "RCZL    D        {WC/WZ/WCZ}");
    m_token.insert(instr, t_RCZL);
    pattern(instr, "EEEE 1101011 CZ0 DDDDDDDDD 001101011");
}

/**
 * @brief Write 0 or 1 to D, according to  C.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101100
 *
 * WRC     D
 *
 * D = {31'b0,  C).
 *
 */
void P2Doc::doc_wrc(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Write 0 or 1 to D, according to  C."));
    m_instr.insert(instr, "WRC     D");
    m_token.insert(instr, t_WRC);
    pattern(instr, "EEEE 1101011 000 DDDDDDDDD 001101100");
}

/**
 * @brief Write 0 or 1 to D, according to !C.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101101
 *
 * WRNC    D
 *
 * D = {31'b0, !C).
 *
 */
void P2Doc::doc_wrnc(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Write 0 or 1 to D, according to !C."));
    m_instr.insert(instr, "WRNC    D");
    m_token.insert(instr, t_WRNC);
    pattern(instr, "EEEE 1101011 000 DDDDDDDDD 001101101");
}

/**
 * @brief Write 0 or 1 to D, according to  Z.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101110
 *
 * WRZ     D
 *
 * D = {31'b0,  Z).
 *
 */
void P2Doc::doc_wrz(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Write 0 or 1 to D, according to  Z."));
    m_instr.insert(instr, "WRZ     D");
    m_token.insert(instr, t_WRZ);
    pattern(instr, "EEEE 1101011 000 DDDDDDDDD 001101110");
}

/**
 * @brief Write 0 or 1 to D, according to !Z.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101111
 *
 * WRNZ    D
 *
 * D = {31'b0, !Z).
 *
 */
void P2Doc::doc_wrnz(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Write 0 or 1 to D, according to !Z."));
    m_instr.insert(instr, "WRNZ    D");
    m_token.insert(instr, t_WRNZ);
    pattern(instr, "EEEE 1101011 000 DDDDDDDDD 001101111");
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
void P2Doc::doc_modcz(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Modify C and Z according to cccc and zzzz."));
    m_instr.insert(instr, "MODCZ   c,z      {WC/WZ/WCZ}");
    m_token.insert(instr, t_MODCZ);
    pattern(instr, "EEEE 1101011 CZ1 0cccczzzz 001101111");
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
void P2Doc::doc_setscp(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Set scope mode."));
    m_instr.insert(instr, "SETSCP  {#}D");
    m_token.insert(instr, t_SETSCP);
    pattern(instr, "EEEE 1101011 00L DDDDDDDDD 001110000");
}

/**
 * @brief Get scope values.
 *
 * EEEE 1101011 000 DDDDDDDDD 001110001
 *
 * Any time GETSCP is executed, the lower bytes of those four pins' RDPIN values are returned in D.
 * This feature will mainly be useful on the next silicon, as the FPGAs don't have ADC-capable pins.
 *
 * GETSCP  D
 *
 * C = cccc[{C,Z}], Z = zzzz[{C,Z}].
 *
 */
void P2Doc::doc_getscp(p2_opsrc_e instr)
{
    m_brief.insert(instr, tr("Get four scope values into bytes of D."));
    m_instr.insert(instr, "Any time GETSCP is executed, the lower bytes of those four pins' RDPIN values are returned in D.");
    m_token.insert(instr, t_GETSCP);
    pattern(instr, "EEEE 1101011 000 DDDDDDDDD 001110001");
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
void P2Doc::doc_jmp_abs(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Jump to A."));
    m_instr.insert(instr, "JMP     #A");
    m_token.insert(instr, t_JMP);
    pattern(instr, "EEEE 1101100 RAA AAAAAAAAA AAAAAAAAA");
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
void P2Doc::doc_call_abs(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Call to A by pushing {C, Z, 10'b0, PC[19:0]} onto stack."));
    m_instr.insert(instr, "CALL    #A");
    m_token.insert(instr, t_CALL);
    pattern(instr, "EEEE 1101101 RAA AAAAAAAAA AAAAAAAAA");
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
void P2Doc::doc_calla_abs(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Call to A by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRA++."));
    m_instr.insert(instr, "CALLA   #A");
    m_token.insert(instr, t_CALLA);
    pattern(instr, "EEEE 1101110 RAA AAAAAAAAA AAAAAAAAA");
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
void P2Doc::doc_callb_abs(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Call to A by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRB++."));
    m_instr.insert(instr, "CALLB   #A");
    m_token.insert(instr, t_CALLB);
    pattern(instr, "EEEE 1101111 RAA AAAAAAAAA AAAAAAAAA");
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 1110000 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PA,#A
 *
 * If R = 1, PC += A, else PC = A.
 *
 */
void P2Doc::doc_calld_pa_abs(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Call to A by writing {C, Z, 10'b0, PC[19:0]} to PA (per W)."));
    m_instr.insert(instr, "CALLD   PA,#A");
    m_token.insert(instr, t_CALLD);    // t_PA
    pattern(instr, "EEEE 1110000 RAA AAAAAAAAA AAAAAAAAA");
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 1110001 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PB,#A
 *
 * If R = 1, PC += A, else PC = A.
 *
 */
void P2Doc::doc_calld_pb_abs(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Call to A by writing {C, Z, 10'b0, PC[19:0]} to PB (per W)."));
    m_instr.insert(instr, "CALLD   PB,#A");
    m_token.insert(instr, t_CALLD);    // t_PB
    pattern(instr, "EEEE 1110001 RAA AAAAAAAAA AAAAAAAAA");
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PTRA (per W).
 *
 * EEEE 1110010 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PTRA,#A
 *
 * If R = 1, PC += A, else PC = A.
 *
 */
void P2Doc::doc_calld_ptra_abs(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Call to A by writing {C, Z, 10'b0, PC[19:0]} to PTRA (per W)."));
    m_instr.insert(instr, "CALLD   PTRA,#A");
    m_token.insert(instr, t_CALLD);    // t_PTRA
    pattern(instr, "EEEE 1110010 RAA AAAAAAAAA AAAAAAAAA");
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PTRB (per W).
 *
 * EEEE 1110011 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PTRB,#A
 *
 * If R = 1, PC += A, else PC = A.
 *
 */
void P2Doc::doc_calld_ptrb_abs(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Call to A by writing {C, Z, 10'b0, PC[19:0]} to PTRB (per W)."));
    m_instr.insert(instr, "CALLD   PTRB,#A");
    m_token.insert(instr, t_CALLD);    // t_PTRB
    pattern(instr, "EEEE 1110011 RAA AAAAAAAAA AAAAAAAAA");
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
void P2Doc::doc_loc_pa(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Get {12'b0, address[19:0]} into PA/PB/PTRA/PTRB (per W)."));
    m_instr.insert(instr, "LOC     PA/PB/PTRA/PTRB,#A");
    m_token.insert(instr, t_LOC);  // t_PA
    pattern(instr, "EEEE 1110100 RAA AAAAAAAAA AAAAAAAAA");
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
void P2Doc::doc_loc_pb(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Get {12'b0, address[19:0]} into PA/PB/PTRA/PTRB (per W)."));
    m_instr.insert(instr, "LOC     PA/PB/PTRA/PTRB,#A");
    m_token.insert(instr, t_LOC);  // t_PB
    pattern(instr, "EEEE 1110101 RAA AAAAAAAAA AAAAAAAAA");
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
void P2Doc::doc_loc_ptra(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Get {12'b0, address[19:0]} into PA/PB/PTRA/PTRB (per W)."));
    m_instr.insert(instr, "LOC     PA/PB/PTRA/PTRB,#A");
    m_token.insert(instr, t_LOC);  // t_PTRA
    pattern(instr, "EEEE 1110110 RAA AAAAAAAAA AAAAAAAAA");
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
void P2Doc::doc_loc_ptrb(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Get {12'b0, address[19:0]} into PA/PB/PTRA/PTRB (per W)."));
    m_instr.insert(instr, "LOC     PA/PB/PTRA/PTRB,#A");
    m_token.insert(instr, t_LOC);  // t_PTRB
    pattern(instr, "EEEE 1110111 RAA AAAAAAAAA AAAAAAAAA");
}

/**
 * @brief Queue #N[31:9] to be used as upper 23 bits for next #S occurrence, so that the next 9-bit #S will be augmented to 32 bits.
 *
 * EEEE 11110NN NNN NNNNNNNNN NNNNNNNNN
 *
 * AUGS    #N
 *
 *
 */
void P2Doc::doc_augs(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Queue #N[31:9] to be used as upper 23 bits for next #S occurrence, so that the next 9-bit #S will be augmented to 32 bits."));
    m_instr.insert(instr, "AUGS    #N");
    m_token.insert(instr, t_AUGS);
    pattern(instr, "EEEE 11110NN NNN NNNNNNNNN NNNNNNNNN");
}

/**
 * @brief Queue #N[31:9] to be used as upper 23 bits for next #D occurrence, so that the next 9-bit #D will be augmented to 32 bits.
 *
 * EEEE 11111NN NNN NNNNNNNNN NNNNNNNNN
 *
 * AUGD    #N
 *
 *
 */
void P2Doc::doc_augd(p2_inst7_e instr)
{
    m_brief.insert(instr, tr("Queue #N[31:9] to be used as upper 23 bits for next #D occurrence, so that the next 9-bit #D will be augmented to 32 bits."));
    m_instr.insert(instr, "AUGD    #N");
    m_token.insert(instr, t_AUGD);
    pattern(instr, "EEEE 11111NN NNN NNNNNNNNN NNNNNNNNN");
}
