#pragma once
#include "p2token.h"

class P2Cog;

class P2Dasm
{
public:
    P2Dasm();
    ~P2Dasm();

    QString dasm(P2Cog* COG, quint32 PC);

private:
    P2Token* p2Token;
    p2_opword_t IR;
    quint32 S;
    quint32 D;

    p2_token_e conditional(p2_cond_e cond);
    p2_token_e conditional(unsigned cond);

    QString format_d_imm_s_cz(p2_token_e inst, p2_token_e with = t_W);
    QString format_d_imm_s_c(p2_token_e inst, p2_token_e with = t_W);
    QString format_d_imm_s_z(p2_token_e inst, p2_token_e with = t_W);
    QString format_wz_d_imm_s(p2_token_e inst);
    QString format_d_imm_s_nnn(p2_token_e inst);
    QString format_d_imm_s_n(p2_token_e inst);
    QString format_d_imm_s(p2_token_e inst);
    QString format_d_cz(p2_token_e inst, p2_token_e with = t_W);
    QString format_cz(p2_token_e inst, p2_token_e with = t_W);
    QString format_cz_cz(p2_token_e inst, p2_token_e with = t_W);
    QString format_d(p2_token_e inst);
    QString format_wz_d(p2_token_e inst);
    QString format_imm_d(p2_token_e inst);
    QString format_imm_d_cz(p2_token_e inst, p2_token_e with = t_W);
    QString format_imm_d_c(p2_token_e inst);
    QString format_imm_s(p2_token_e inst);
    QString format_pc_abs(p2_token_e inst, p2_token_e dst = t_nothing);
    QString format_imm23(p2_token_e inst);

    QString dasm_nop();
    QString dasm_ror();
    QString dasm_rol();
    QString dasm_shr();
    QString dasm_shl();
    QString dasm_rcr();
    QString dasm_rcl();
    QString dasm_sar();
    QString dasm_sal();

    QString dasm_add();
    QString dasm_addx();
    QString dasm_adds();
    QString dasm_addsx();
    QString dasm_sub();
    QString dasm_subx();
    QString dasm_subs();
    QString dasm_subsx();

    QString dasm_cmp();
    QString dasm_cmpx();
    QString dasm_cmps();
    QString dasm_cmpsx();
    QString dasm_cmpr();
    QString dasm_cmpm();
    QString dasm_subr();
    QString dasm_cmpsub();

    QString dasm_fge();
    QString dasm_fle();
    QString dasm_fges();
    QString dasm_fles();
    QString dasm_sumc();
    QString dasm_sumnc();
    QString dasm_sumz();
    QString dasm_sumnz();

    QString dasm_testb_w();
    QString dasm_testbn_w();
    QString dasm_testb_and();
    QString dasm_testbn_and();
    QString dasm_testb_or();
    QString dasm_testbn_or();
    QString dasm_testb_xor();
    QString dasm_testbn_xor();

    QString dasm_bitl();
    QString dasm_bith();
    QString dasm_bitc();
    QString dasm_bitnc();
    QString dasm_bitz();
    QString dasm_bitnz();
    QString dasm_bitrnd();
    QString dasm_bitnot();

    QString dasm_and();
    QString dasm_andn();
    QString dasm_or();
    QString dasm_xor();
    QString dasm_muxc();
    QString dasm_muxnc();
    QString dasm_muxz();
    QString dasm_muxnz();

    QString dasm_mov();
    QString dasm_not();
    QString dasm_not_d();
    QString dasm_abs();
    QString dasm_neg();
    QString dasm_negc();
    QString dasm_negnc();
    QString dasm_negz();
    QString dasm_negnz();

    QString dasm_incmod();
    QString dasm_decmod();
    QString dasm_zerox();
    QString dasm_signx();
    QString dasm_encod();
    QString dasm_ones();
    QString dasm_test();
    QString dasm_testn();

    QString dasm_setnib();
    QString dasm_setnib_altsn();
    QString dasm_getnib();
    QString dasm_getnib_altgn();
    QString dasm_rolnib();
    QString dasm_rolnib_altgn();
    QString dasm_setbyte();
    QString dasm_setbyte_altsb();
    QString dasm_getbyte();
    QString dasm_getbyte_altgb();
    QString dasm_rolbyte();
    QString dasm_rolbyte_altgb();

    QString dasm_setword();
    QString dasm_setword_altsw();
    QString dasm_getword();
    QString dasm_getword_altgw();
    QString dasm_rolword();
    QString dasm_rolword_altgw();

    QString dasm_altsn();
    QString dasm_altsn_d();
    QString dasm_altgn();
    QString dasm_altgn_d();
    QString dasm_altsb();
    QString dasm_altsb_d();
    QString dasm_altgb();
    QString dasm_altgb_d();

    QString dasm_altsw();
    QString dasm_altsw_d();
    QString dasm_altgw();
    QString dasm_altgw_d();

    QString dasm_altr();
    QString dasm_altr_d();
    QString dasm_altd();
    QString dasm_altd_d();
    QString dasm_alts();
    QString dasm_alts_d();
    QString dasm_altb();
    QString dasm_altb_d();
    QString dasm_alti();
    QString dasm_alti_d();

    QString dasm_setr();
    QString dasm_setd();
    QString dasm_sets();
    QString dasm_decod();
    QString dasm_decod_d();
    QString dasm_bmask();
    QString dasm_bmask_d();
    QString dasm_crcbit();
    QString dasm_crcnib();

    QString dasm_muxnits();
    QString dasm_muxnibs();
    QString dasm_muxq();
    QString dasm_movbyts();
    QString dasm_mul();
    QString dasm_muls();
    QString dasm_sca();
    QString dasm_scas();

    QString dasm_addpix();
    QString dasm_mulpix();
    QString dasm_blnpix();
    QString dasm_mixpix();
    QString dasm_addct1();
    QString dasm_addct2();
    QString dasm_addct3();

    QString dasm_wmlong();
    QString dasm_rqpin();
    QString dasm_rdpin();
    QString dasm_rdlut();
    QString dasm_rdbyte();
    QString dasm_rdword();
    QString dasm_rdlong();

    QString dasm_popa();
    QString dasm_popb();
    QString dasm_calld();
    QString dasm_resi3();
    QString dasm_resi2();
    QString dasm_resi1();
    QString dasm_resi0();
    QString dasm_reti3();
    QString dasm_reti2();
    QString dasm_reti1();
    QString dasm_reti0();
    QString dasm_callpa();
    QString dasm_callpb();

    QString dasm_djz();
    QString dasm_djnz();
    QString dasm_djf();
    QString dasm_djnf();
    QString dasm_ijz();
    QString dasm_ijnz();
    QString dasm_tjz();
    QString dasm_tjnz();
    QString dasm_tjf();
    QString dasm_tjnf();
    QString dasm_tjs();
    QString dasm_tjns();
    QString dasm_tjv();

    QString dasm_jint();
    QString dasm_jct1();
    QString dasm_jct2();
    QString dasm_jct3();
    QString dasm_jse1();
    QString dasm_jse2();
    QString dasm_jse3();
    QString dasm_jse4();
    QString dasm_jpat();
    QString dasm_jfbw();
    QString dasm_jxmt();
    QString dasm_jxfi();
    QString dasm_jxro();
    QString dasm_jxrl();
    QString dasm_jatn();
    QString dasm_jqmt();

    QString dasm_jnint();
    QString dasm_jnct1();
    QString dasm_jnct2();
    QString dasm_jnct3();
    QString dasm_jnse1();
    QString dasm_jnse2();
    QString dasm_jnse3();
    QString dasm_jnse4();
    QString dasm_jnpat();
    QString dasm_jnfbw();
    QString dasm_jnxmt();
    QString dasm_jnxfi();
    QString dasm_jnxro();
    QString dasm_jnxrl();
    QString dasm_jnatn();
    QString dasm_jnqmt();
    QString dasm_1011110_1();
    QString dasm_1011111_0();
    QString dasm_setpat();

    QString dasm_wrpin();
    QString dasm_akpin();
    QString dasm_wxpin();
    QString dasm_wypin();
    QString dasm_wrlut();
    QString dasm_wrbyte();
    QString dasm_wrword();
    QString dasm_wrlong();

    QString dasm_pusha();
    QString dasm_pushb();
    QString dasm_rdfast();
    QString dasm_wrfast();
    QString dasm_fblock();
    QString dasm_xinit();
    QString dasm_xstop();
    QString dasm_xzero();
    QString dasm_xcont();

    QString dasm_rep();
    QString dasm_coginit();

    QString dasm_qmul();
    QString dasm_qdiv();
    QString dasm_qfrac();
    QString dasm_qsqrt();
    QString dasm_qrotate();
    QString dasm_qvector();

    QString dasm_hubset();
    QString dasm_cogid();
    QString dasm_cogstop();
    QString dasm_locknew();
    QString dasm_lockret();
    QString dasm_locktry();
    QString dasm_lockrel();
    QString dasm_qlog();
    QString dasm_qexp();

    QString dasm_rfbyte();
    QString dasm_rfword();
    QString dasm_rflong();
    QString dasm_rfvar();
    QString dasm_rfvars();
    QString dasm_wfbyte();
    QString dasm_wfword();
    QString dasm_wflong();

    QString dasm_getqx();
    QString dasm_getqy();
    QString dasm_getct();
    QString dasm_getrnd();
    QString dasm_getrnd_cz();

    QString dasm_setdacs();
    QString dasm_setxfrq();
    QString dasm_getxacc();
    QString dasm_waitx();
    QString dasm_setse1();
    QString dasm_setse2();
    QString dasm_setse3();
    QString dasm_setse4();
    QString dasm_pollint();
    QString dasm_pollct1();
    QString dasm_pollct2();
    QString dasm_pollct3();
    QString dasm_pollse1();
    QString dasm_pollse2();
    QString dasm_pollse3();
    QString dasm_pollse4();
    QString dasm_pollpat();
    QString dasm_pollfbw();
    QString dasm_pollxmt();
    QString dasm_pollxfi();
    QString dasm_pollxro();
    QString dasm_pollxrl();
    QString dasm_pollatn();
    QString dasm_pollqmt();

    QString dasm_waitint();
    QString dasm_waitct1();
    QString dasm_waitct2();
    QString dasm_waitct3();
    QString dasm_waitse1();
    QString dasm_waitse2();
    QString dasm_waitse3();
    QString dasm_waitse4();
    QString dasm_waitpat();
    QString dasm_waitfbw();
    QString dasm_waitxmt();
    QString dasm_waitxfi();
    QString dasm_waitxro();
    QString dasm_waitxrl();
    QString dasm_waitatn();

    QString dasm_allowi();
    QString dasm_stalli();
    QString dasm_trgint1();
    QString dasm_trgint2();
    QString dasm_trgint3();
    QString dasm_nixint1();
    QString dasm_nixint2();
    QString dasm_nixint3();
    QString dasm_setint1();
    QString dasm_setint2();
    QString dasm_setint3();
    QString dasm_setq();
    QString dasm_setq2();

    QString dasm_push();
    QString dasm_pop();
    QString dasm_jmp();
    QString dasm_call();
    QString dasm_ret();
    QString dasm_calla();
    QString dasm_reta();
    QString dasm_callb();
    QString dasm_retb();

    QString dasm_jmprel();
    QString dasm_skip();
    QString dasm_skipf();
    QString dasm_execf();
    QString dasm_getptr();
    QString dasm_getbrk();
    QString dasm_cogbrk();
    QString dasm_brk();

    QString dasm_setluts();
    QString dasm_setcy();
    QString dasm_setci();
    QString dasm_setcq();
    QString dasm_setcfrq();
    QString dasm_setcmod();
    QString dasm_setpiv();
    QString dasm_setpix();
    QString dasm_cogatn();

    QString dasm_testp_w();
    QString dasm_testpn_w();
    QString dasm_testp_and();
    QString dasm_testpn_and();
    QString dasm_testp_or();
    QString dasm_testpn_or();
    QString dasm_testp_xor();
    QString dasm_testpn_xor();

    QString dasm_dirl();
    QString dasm_dirh();
    QString dasm_dirc();
    QString dasm_dirnc();
    QString dasm_dirz();
    QString dasm_dirnz();
    QString dasm_dirrnd();
    QString dasm_dirnot();

    QString dasm_outl();
    QString dasm_outh();
    QString dasm_outc();
    QString dasm_outnc();
    QString dasm_outz();
    QString dasm_outnz();
    QString dasm_outrnd();
    QString dasm_outnot();

    QString dasm_fltl();
    QString dasm_flth();
    QString dasm_fltc();
    QString dasm_fltnc();
    QString dasm_fltz();
    QString dasm_fltnz();
    QString dasm_fltrnd();
    QString dasm_fltnot();

    QString dasm_drvl();
    QString dasm_drvh();
    QString dasm_drvc();
    QString dasm_drvnc();
    QString dasm_drvz();
    QString dasm_drvnz();
    QString dasm_drvrnd();
    QString dasm_drvnot();

    QString dasm_splitb();
    QString dasm_mergeb();
    QString dasm_splitw();
    QString dasm_mergew();
    QString dasm_seussf();
    QString dasm_seussr();
    QString dasm_rgbsqz();
    QString dasm_rgbexp();
    QString dasm_xoro32();

    QString dasm_rev();
    QString dasm_rczr();
    QString dasm_rczl();
    QString dasm_wrc();
    QString dasm_wrnc();
    QString dasm_wrz();
    QString dasm_wrnz();
    QString dasm_modcz();

    QString dasm_jmp_abs();
    QString dasm_call_abs();
    QString dasm_calla_abs();
    QString dasm_callb_abs();
    QString dasm_calld_abs();
    QString dasm_loc_pa();
    QString dasm_loc_pb();
    QString dasm_loc_ptra();
    QString dasm_loc_ptrb();

    QString dasm_augs();
    QString dasm_augd();
};
