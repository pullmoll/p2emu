#pragma once
#include "p2token.h"

class P2Cog;

class P2Dasm
{
public:
    P2Dasm();
    ~P2Dasm();

    QString dasm(P2Cog* COG, quint32 PC, QString& opcode);

private:
    P2Token* Token;
    int pad_inst;
    int pad_wcz;
    QString m_opcode;
    QString m_string;
    p2_opword_t IR;
    quint32 S;
    quint32 D;

    p2_token_e conditional(p2_cond_e cond);
    p2_token_e conditional(unsigned cond);

    QString format_num(uint num, bool binary = false);
    QString format_bin(uint num, int digits = 0);
    void format_inst(p2_token_e inst);
    void format_d_imm_s_cz(p2_token_e inst, p2_token_e with = t_W);
    void format_d_imm_s_c(p2_token_e inst, p2_token_e with = t_W);
    void format_d_imm_s_z(p2_token_e inst, p2_token_e with = t_W);
    void format_wz_d_imm_s(p2_token_e inst);
    void format_d_imm_s_nnn(p2_token_e inst);
    void format_d_imm_s_n(p2_token_e inst);
    void format_d_imm_s(p2_token_e inst);
    void format_d_cz(p2_token_e inst, p2_token_e with = t_W);
    void format_cz(p2_token_e inst, p2_token_e with = t_W);
    void format_cz_cz(p2_token_e inst, p2_token_e with = t_W);
    void format_d(p2_token_e inst);
    void format_wz_d(p2_token_e inst);
    void format_imm_d(p2_token_e inst);
    void format_imm_d_cz(p2_token_e inst, p2_token_e with = t_W);
    void format_imm_d_c(p2_token_e inst);
    void format_imm_s(p2_token_e inst);
    void format_pc_abs(p2_token_e inst, p2_token_e dest = t_nothing);
    void format_imm23(p2_token_e inst);

    void dasm_nop();
    void dasm_ror();
    void dasm_rol();
    void dasm_shr();
    void dasm_shl();
    void dasm_rcr();
    void dasm_rcl();
    void dasm_sar();
    void dasm_sal();

    void dasm_add();
    void dasm_addx();
    void dasm_adds();
    void dasm_addsx();
    void dasm_sub();
    void dasm_subx();
    void dasm_subs();
    void dasm_subsx();

    void dasm_cmp();
    void dasm_cmpx();
    void dasm_cmps();
    void dasm_cmpsx();
    void dasm_cmpr();
    void dasm_cmpm();
    void dasm_subr();
    void dasm_cmpsub();

    void dasm_fge();
    void dasm_fle();
    void dasm_fges();
    void dasm_fles();
    void dasm_sumc();
    void dasm_sumnc();
    void dasm_sumz();
    void dasm_sumnz();

    void dasm_testb_w();
    void dasm_testbn_w();
    void dasm_testb_and();
    void dasm_testbn_and();
    void dasm_testb_or();
    void dasm_testbn_or();
    void dasm_testb_xor();
    void dasm_testbn_xor();

    void dasm_bitl();
    void dasm_bith();
    void dasm_bitc();
    void dasm_bitnc();
    void dasm_bitz();
    void dasm_bitnz();
    void dasm_bitrnd();
    void dasm_bitnot();

    void dasm_and();
    void dasm_andn();
    void dasm_or();
    void dasm_xor();
    void dasm_muxc();
    void dasm_muxnc();
    void dasm_muxz();
    void dasm_muxnz();

    void dasm_mov();
    void dasm_not();
    void dasm_not_d();
    void dasm_abs();
    void dasm_neg();
    void dasm_negc();
    void dasm_negnc();
    void dasm_negz();
    void dasm_negnz();

    void dasm_incmod();
    void dasm_decmod();
    void dasm_zerox();
    void dasm_signx();
    void dasm_encod();
    void dasm_ones();
    void dasm_test();
    void dasm_testn();

    void dasm_setnib();
    void dasm_setnib_altsn();
    void dasm_getnib();
    void dasm_getnib_altgn();
    void dasm_rolnib();
    void dasm_rolnib_altgn();
    void dasm_setbyte();
    void dasm_setbyte_altsb();
    void dasm_getbyte();
    void dasm_getbyte_altgb();
    void dasm_rolbyte();
    void dasm_rolbyte_altgb();

    void dasm_setword();
    void dasm_setword_altsw();
    void dasm_getword();
    void dasm_getword_altgw();
    void dasm_rolword();
    void dasm_rolword_altgw();

    void dasm_altsn();
    void dasm_altsn_d();
    void dasm_altgn();
    void dasm_altgn_d();
    void dasm_altsb();
    void dasm_altsb_d();
    void dasm_altgb();
    void dasm_altgb_d();

    void dasm_altsw();
    void dasm_altsw_d();
    void dasm_altgw();
    void dasm_altgw_d();

    void dasm_altr();
    void dasm_altr_d();
    void dasm_altd();
    void dasm_altd_d();
    void dasm_alts();
    void dasm_alts_d();
    void dasm_altb();
    void dasm_altb_d();
    void dasm_alti();
    void dasm_alti_d();

    void dasm_setr();
    void dasm_setd();
    void dasm_sets();
    void dasm_decod();
    void dasm_decod_d();
    void dasm_bmask();
    void dasm_bmask_d();
    void dasm_crcbit();
    void dasm_crcnib();

    void dasm_muxnits();
    void dasm_muxnibs();
    void dasm_muxq();
    void dasm_movbyts();
    void dasm_mul();
    void dasm_muls();
    void dasm_sca();
    void dasm_scas();

    void dasm_addpix();
    void dasm_mulpix();
    void dasm_blnpix();
    void dasm_mixpix();
    void dasm_addct1();
    void dasm_addct2();
    void dasm_addct3();

    void dasm_wmlong();
    void dasm_rqpin();
    void dasm_rdpin();
    void dasm_rdlut();
    void dasm_rdbyte();
    void dasm_rdword();
    void dasm_rdlong();

    void dasm_popa();
    void dasm_popb();
    void dasm_calld();
    void dasm_resi3();
    void dasm_resi2();
    void dasm_resi1();
    void dasm_resi0();
    void dasm_reti3();
    void dasm_reti2();
    void dasm_reti1();
    void dasm_reti0();
    void dasm_callpa();
    void dasm_callpb();

    void dasm_djz();
    void dasm_djnz();
    void dasm_djf();
    void dasm_djnf();
    void dasm_ijz();
    void dasm_ijnz();
    void dasm_tjz();
    void dasm_tjnz();
    void dasm_tjf();
    void dasm_tjnf();
    void dasm_tjs();
    void dasm_tjns();
    void dasm_tjv();

    void dasm_jint();
    void dasm_jct1();
    void dasm_jct2();
    void dasm_jct3();
    void dasm_jse1();
    void dasm_jse2();
    void dasm_jse3();
    void dasm_jse4();
    void dasm_jpat();
    void dasm_jfbw();
    void dasm_jxmt();
    void dasm_jxfi();
    void dasm_jxro();
    void dasm_jxrl();
    void dasm_jatn();
    void dasm_jqmt();

    void dasm_jnint();
    void dasm_jnct1();
    void dasm_jnct2();
    void dasm_jnct3();
    void dasm_jnse1();
    void dasm_jnse2();
    void dasm_jnse3();
    void dasm_jnse4();
    void dasm_jnpat();
    void dasm_jnfbw();
    void dasm_jnxmt();
    void dasm_jnxfi();
    void dasm_jnxro();
    void dasm_jnxrl();
    void dasm_jnatn();
    void dasm_jnqmt();
    void dasm_1011110_1();
    void dasm_1011111_0();
    void dasm_setpat();

    void dasm_wrpin();
    void dasm_akpin();
    void dasm_wxpin();
    void dasm_wypin();
    void dasm_wrlut();
    void dasm_wrbyte();
    void dasm_wrword();
    void dasm_wrlong();

    void dasm_pusha();
    void dasm_pushb();
    void dasm_rdfast();
    void dasm_wrfast();
    void dasm_fblock();
    void dasm_xinit();
    void dasm_xstop();
    void dasm_xzero();
    void dasm_xcont();

    void dasm_rep();
    void dasm_coginit();

    void dasm_qmul();
    void dasm_qdiv();
    void dasm_qfrac();
    void dasm_qsqrt();
    void dasm_qrotate();
    void dasm_qvector();

    void dasm_hubset();
    void dasm_cogid();
    void dasm_cogstop();
    void dasm_locknew();
    void dasm_lockret();
    void dasm_locktry();
    void dasm_lockrel();
    void dasm_qlog();
    void dasm_qexp();

    void dasm_rfbyte();
    void dasm_rfword();
    void dasm_rflong();
    void dasm_rfvar();
    void dasm_rfvars();
    void dasm_wfbyte();
    void dasm_wfword();
    void dasm_wflong();

    void dasm_getqx();
    void dasm_getqy();
    void dasm_getct();
    void dasm_getrnd();
    void dasm_getrnd_cz();

    void dasm_setdacs();
    void dasm_setxfrq();
    void dasm_getxacc();
    void dasm_waitx();
    void dasm_setse1();
    void dasm_setse2();
    void dasm_setse3();
    void dasm_setse4();
    void dasm_pollint();
    void dasm_pollct1();
    void dasm_pollct2();
    void dasm_pollct3();
    void dasm_pollse1();
    void dasm_pollse2();
    void dasm_pollse3();
    void dasm_pollse4();
    void dasm_pollpat();
    void dasm_pollfbw();
    void dasm_pollxmt();
    void dasm_pollxfi();
    void dasm_pollxro();
    void dasm_pollxrl();
    void dasm_pollatn();
    void dasm_pollqmt();

    void dasm_waitint();
    void dasm_waitct1();
    void dasm_waitct2();
    void dasm_waitct3();
    void dasm_waitse1();
    void dasm_waitse2();
    void dasm_waitse3();
    void dasm_waitse4();
    void dasm_waitpat();
    void dasm_waitfbw();
    void dasm_waitxmt();
    void dasm_waitxfi();
    void dasm_waitxro();
    void dasm_waitxrl();
    void dasm_waitatn();

    void dasm_allowi();
    void dasm_stalli();
    void dasm_trgint1();
    void dasm_trgint2();
    void dasm_trgint3();
    void dasm_nixint1();
    void dasm_nixint2();
    void dasm_nixint3();
    void dasm_setint1();
    void dasm_setint2();
    void dasm_setint3();
    void dasm_setq();
    void dasm_setq2();

    void dasm_push();
    void dasm_pop();
    void dasm_jmp();
    void dasm_call();
    void dasm_ret();
    void dasm_calla();
    void dasm_reta();
    void dasm_callb();
    void dasm_retb();

    void dasm_jmprel();
    void dasm_skip();
    void dasm_skipf();
    void dasm_execf();
    void dasm_getptr();
    void dasm_getbrk();
    void dasm_cogbrk();
    void dasm_brk();

    void dasm_setluts();
    void dasm_setcy();
    void dasm_setci();
    void dasm_setcq();
    void dasm_setcfrq();
    void dasm_setcmod();
    void dasm_setpiv();
    void dasm_setpix();
    void dasm_cogatn();

    void dasm_testp_w();
    void dasm_testpn_w();
    void dasm_testp_and();
    void dasm_testpn_and();
    void dasm_testp_or();
    void dasm_testpn_or();
    void dasm_testp_xor();
    void dasm_testpn_xor();

    void dasm_dirl();
    void dasm_dirh();
    void dasm_dirc();
    void dasm_dirnc();
    void dasm_dirz();
    void dasm_dirnz();
    void dasm_dirrnd();
    void dasm_dirnot();

    void dasm_outl();
    void dasm_outh();
    void dasm_outc();
    void dasm_outnc();
    void dasm_outz();
    void dasm_outnz();
    void dasm_outrnd();
    void dasm_outnot();

    void dasm_fltl();
    void dasm_flth();
    void dasm_fltc();
    void dasm_fltnc();
    void dasm_fltz();
    void dasm_fltnz();
    void dasm_fltrnd();
    void dasm_fltnot();

    void dasm_drvl();
    void dasm_drvh();
    void dasm_drvc();
    void dasm_drvnc();
    void dasm_drvz();
    void dasm_drvnz();
    void dasm_drvrnd();
    void dasm_drvnot();

    void dasm_splitb();
    void dasm_mergeb();
    void dasm_splitw();
    void dasm_mergew();
    void dasm_seussf();
    void dasm_seussr();
    void dasm_rgbsqz();
    void dasm_rgbexp();
    void dasm_xoro32();

    void dasm_rev();
    void dasm_rczr();
    void dasm_rczl();
    void dasm_wrc();
    void dasm_wrnc();
    void dasm_wrz();
    void dasm_wrnz();
    void dasm_modcz();

    void dasm_jmp_abs();
    void dasm_call_abs();
    void dasm_calla_abs();
    void dasm_callb_abs();
    void dasm_calld_abs();
    void dasm_loc_pa();
    void dasm_loc_pb();
    void dasm_loc_ptra();
    void dasm_loc_ptrb();

    void dasm_augs();
    void dasm_augd();
};
