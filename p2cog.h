/****************************************************************************
 *
 * P2 emulator Cog class
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
#include <QVariant>
#include "p2defs.h"
#include "p2hub.h"

class P2Cog : public QObject
{
    Q_OBJECT
public:
    P2Cog(int cog_id = 0, P2Hub* hub = nullptr, QObject* parent = nullptr);

    int decode();

    p2_opcode_u rd_IR() const { return IR; }
    p2_LONG rd_ID() const { return ID; }
    p2_LONG rd_PC() const { return PC; }
    p2_wait_t rd_WAIT() const { return WAIT; }
    p2_flags_t rd_FLAGS() const { return FLAGS; }
    p2_LONG rd_CT1() const { return CT1; }
    p2_LONG rd_CT2() const { return CT2; }
    p2_LONG rd_CT3() const { return CT3; }
    p2_pat_t rd_PAT() const { return PAT; }
    p2_pin_t rd_PIN() const { return PIN; }
    p2_int_bits_u rd_INT() const { return INT; }
    p2_lock_t rd_LOCK() const { return LOCK; }
    p2_LONG rd_D() const { return D; }
    p2_LONG rd_S() const { return S; }
    p2_LONG rd_Q() const { return Q; }
    p2_LONG rd_C() const { return C; }
    p2_LONG rd_Z() const { return Z; }
    QVariant rd_D_aug() const { return D_aug; }
    QVariant rd_S_aug() const { return S_aug; }
    QVariant rd_R_aug() const { return R_aug; }
    p2_LONG rd_cog(p2_LONG addr) const;
    p2_LONG rd_lut(p2_LONG addr) const;
    p2_LONG rd_mem(p2_LONG addr) const;

public slots:
    void wr_cog(p2_LONG addr, p2_LONG val);
    void wr_lut(p2_LONG addr, p2_LONG val);
    void wr_mem(p2_LONG addr, p2_LONG val);
    void wr_PC(p2_LONG addr);
    void wr_PTRA(p2_LONG addr);
    void wr_PTRB(p2_LONG addr);

private:
    P2Hub* HUB;             //!< pointer to the HUB, i.e. the parent of this P2Cog
    p2_LONG ID;             //!< COG ID (0 … number of COGs - 1)
    p2_LONG PC;             //!< program counter
    p2_wait_t WAIT;         //!< waiting conidition
    p2_flags_t FLAGS;       //!< flags register
    p2_LONG CT1;            //!< counter CT1 value
    p2_LONG CT2;            //!< counter CT2 value
    p2_LONG CT3;            //!< counter CT3 value
    p2_pat_t PAT;           //!< PAT mode, mask, and match
    p2_pin_t PIN;           //!< PIN mode, mask, and match
    p2_int_bits_u INT;      //!< INT disable / active / source bits union
    p2_lock_t LOCK;         //!<
    p2_opcode_u IR;         //!< instruction register
    p2_LONG D;              //!< value of D
    p2_LONG S;              //!< value of S
    p2_LONG Q;              //!< value of Q
    p2_LONG C;              //!< current carry flag
    p2_LONG Z;              //!< current zero flag
    p2_fifo_t FIFO;         //!< stream FIFO
    p2_LONG K;              //!< stack pointer (0 … 7)
    p2_LONG STACK[8];       //!< stack of 8 levels
    QVariant S_next;        //!< next instruction's S value
    QVariant S_aug;         //!< augment next S with this value, if set
    QVariant D_aug;         //!< augment next D with this value, if set
    QVariant R_aug;         //!< augment next R with this value, if set
    QVariant IR_aug;        //!< augment next IR with this value, if set
    QVariant REP_instr;     //!< if REP is active, number of instructions to repeat
    p2_LONG REP_offset;     //!< if REP is active, current instruction to repeat
    p2_LONG REP_times;      //!< if REP is active, number of times to repeat
    p2_LONG SKIP;           //!< if SKIP is active, then if b0 is set, the current instruction is cancelled
    p2_LONG SKIPF;          //!< if SKIPF is active, then if b0 is set, the current instruction is skipped
    p2_LONG PTRA0;          //!< actual pointer A to hub RAM
    p2_LONG PTRB0;          //!< actual pointer B to hub RAM
    p2_LONG HUBOP;          //!< non-zero if HUB operation
    p2_LONG CORDIC_count;   //!< non-zero if CORDIC solver running
    bool QX_posted;         //!< true if CORDIC solver X is posted
    bool QY_posted;         //!< true if CORDIC solver Y is posted
    bool RW_repeat;         //!< true if read/write HUB repeated
    p2_LONG RDL_mask;       //!<
    p2_LONG RDL_flags0;     //!<
    p2_LONG RDL_flags1;     //!<
    p2_LONG WRL_mask;       //!<
    p2_LONG WRL_flags0;     //!<
    p2_LONG WRL_flags1;     //!<
    p2_cog_t COG;           //!< COG memory (512 longs)
    p2_lut_t LUT;           //!< LUT memory (512 longs) and shadow registers
    uchar *MEM;             //!< HUB memory pointer
    p2_LONG MEMSIZE;        //!< HUB memory size

    //! return a signed 32 bit value for val[15:0]
    template <typename T>
    qint32 S16(T val) {
        return static_cast<qint32>((val^0x8000)-0x8000);
    }

    //! return the usigned 32 bit value for val[15:0]
    template <typename T>
    p2_LONG U16(T val) {
        return static_cast<p2_LONG>(static_cast<p2_WORD>(val));
    }

    //! return the signed 32 bit value for val[31:0]
    template <typename T>
    qint32 S32(T val) {
        return static_cast<qint32>((val^0x80000000)-0x80000000);
    }

    //! return the usigned 32 bit value for val[31:0]
    template <typename T>
    p2_LONG U32(T val) {
        return static_cast<p2_LONG>(val);
    }

    //! return the 64 bit sign extended value
    template <typename T>
    qint64 SX64(T val) {
        return static_cast<qint64>((val^0x80000000)-0x80000000);
    }

    //! return the usigned 64 bit value
    template <typename T>
    p2_QUAD U64(T val) {
        return static_cast<p2_QUAD>(val);
    }

    //! return the upper half of a 64 bit value as 32 bit unsigned
    template <typename T>
    p2_LONG U32H(T val) {
        return static_cast<p2_LONG>(static_cast<p2_QUAD>(val) >> 32);
    }

    //! return the lower half of a 64 bit value as 32 bit unsigned
    template <typename T>
    p2_LONG U32L(T val) {
        return static_cast<p2_LONG>(val);
    }

    bool conditional(p2_cond_e cond);
    bool conditional(unsigned cond);
    p2_LONG fifo_level();
    void check_interrupt_flags();
    void check_wait_int_state();
    p2_LONG check_wait_flag(p2_opcode_u IR, p2_LONG value1, p2_LONG value2, bool streamflag);
    p2_LONG get_pointer(p2_LONG inst, p2_LONG size);
    void save_regs();
    void update_regs();
    void updateC(bool c);
    void updateZ(bool c);
    void updateD(p2_LONG d);
    void updatePC(p2_LONG d);
    void updateLUT(p2_LONG addr, p2_LONG d);
    void updateSKIP(p2_LONG d);
    void updateSKIPF(p2_LONG d);
    void pushK(p2_LONG val);
    p2_LONG popK();
    void pushPA(p2_LONG val);
    void pushPB(p2_LONG val);
    void pushPTRA(p2_LONG val);
    p2_LONG popPTRA();
    void pushPTRB(p2_LONG val);
    p2_LONG popPTRB();
    void augmentS(bool f);
    void augmentD(bool f);
    void updatePA(p2_LONG d);
    void updatePB(p2_LONG d);
    void updatePTRA(p2_LONG d);
    void updatePTRB(p2_LONG d);
    void updateDIR(p2_LONG pin, bool v);
    void updateOUT(p2_LONG pin, bool v);
    void updateREP(p2_LONG instr, p2_LONG times);

    int op_nop();
    int op_ror();
    int op_rol();
    int op_shr();
    int op_shl();
    int op_rcr();
    int op_rcl();
    int op_sar();
    int op_sal();

    int op_add();
    int op_addx();
    int op_adds();
    int op_addsx();
    int op_sub();
    int op_subx();
    int op_subs();
    int op_subsx();

    int op_cmp();
    int op_cmpx();
    int op_cmps();
    int op_cmpsx();
    int op_cmpr();
    int op_cmpm();
    int op_subr();
    int op_cmpsub();

    int op_fge();
    int op_fle();
    int op_fges();
    int op_fles();
    int op_sumc();
    int op_sumnc();
    int op_sumz();
    int op_sumnz();

    int op_testb_w();
    int op_testbn_w();
    int op_testb_and();
    int op_testbn_and();
    int op_testb_or();
    int op_testbn_or();
    int op_testb_xor();
    int op_testbn_xor();

    int op_bitl();
    int op_bith();
    int op_bitc();
    int op_bitnc();
    int op_bitz();
    int op_bitnz();
    int op_bitrnd();
    int op_bitnot();

    int op_and();
    int op_andn();
    int op_or();
    int op_xor();
    int op_muxc();
    int op_muxnc();
    int op_muxz();
    int op_muxnz();

    int op_mov();
    int op_not();
    int op_not_d();
    int op_abs();
    int op_neg();
    int op_negc();
    int op_negnc();
    int op_negz();
    int op_negnz();

    int op_incmod();
    int op_decmod();
    int op_zerox();
    int op_signx();
    int op_encod();
    int op_ones();
    int op_test();
    int op_testn();

    int op_setnib();
    int op_setnib_altsn();
    int op_getnib();
    int op_getnib_altgn();
    int op_rolnib();
    int op_rolnib_altgn();
    int op_setbyte();
    int op_setbyte_altsb();
    int op_getbyte();
    int op_getbyte_altgb();
    int op_rolbyte();
    int op_rolbyte_altgb();

    int op_setword();
    int op_setword_altsw();
    int op_getword();
    int op_getword_altgw();
    int op_rolword();
    int op_rolword_altgw();

    int op_altsn();
    int op_altsn_d();
    int op_altgn();
    int op_altgn_d();
    int op_altsb();
    int op_altsb_d();
    int op_altgb();
    int op_altgb_d();

    int op_altsw();
    int op_altsw_d();
    int op_altgw();
    int op_altgw_d();

    int op_altr();
    int op_altr_d();
    int op_altd();
    int op_altd_d();
    int op_alts();
    int op_alts_d();
    int op_altb();
    int op_altb_d();
    int op_alti();
    int op_alti_d();

    int op_setr();
    int op_setd();
    int op_sets();
    int op_decod();
    int op_decod_d();
    int op_bmask();
    int op_bmask_d();
    int op_crcbit();
    int op_crcnib();

    int op_muxnits();
    int op_muxnibs();
    int op_muxq();
    int op_movbyts();
    int op_mul();
    int op_muls();
    int op_sca();
    int op_scas();

    int op_addpix();
    int op_mulpix();
    int op_blnpix();
    int op_mixpix();
    int op_addct1();
    int op_addct2();
    int op_addct3();

    int op_wmlong();
    int op_rqpin();
    int op_rdpin();
    int op_rdlut();
    int op_rdbyte();
    int op_rdword();
    int op_rdlong();

    int op_popa();
    int op_popb();
    int op_calld();
    int op_resi3();
    int op_resi2();
    int op_resi1();
    int op_resi0();
    int op_reti3();
    int op_reti2();
    int op_reti1();
    int op_reti0();
    int op_callpa();
    int op_callpb();

    int op_djz();
    int op_djnz();
    int op_djf();
    int op_djnf();
    int op_ijz();
    int op_ijnz();
    int op_tjz();
    int op_tjnz();
    int op_tjf();
    int op_tjnf();
    int op_tjs();
    int op_tjns();
    int op_tjv();

    int op_jint();
    int op_jct1();
    int op_jct2();
    int op_jct3();
    int op_jse1();
    int op_jse2();
    int op_jse3();
    int op_jse4();
    int op_jpat();
    int op_jfbw();
    int op_jxmt();
    int op_jxfi();
    int op_jxro();
    int op_jxrl();
    int op_jatn();
    int op_jqmt();

    int op_jnint();
    int op_jnct1();
    int op_jnct2();
    int op_jnct3();
    int op_jnse1();
    int op_jnse2();
    int op_jnse3();
    int op_jnse4();
    int op_jnpat();
    int op_jnfbw();
    int op_jnxmt();
    int op_jnxfi();
    int op_jnxro();
    int op_jnxrl();
    int op_jnatn();
    int op_jnqmt();
    int op_1011110_1();
    int op_1011111_0();
    int op_setpat();

    int op_wrpin();
    int op_akpin();
    int op_wxpin();
    int op_wypin();
    int op_wrlut();
    int op_wrbyte();
    int op_wrword();
    int op_wrlong();

    int op_pusha();
    int op_pushb();
    int op_rdfast();
    int op_wrfast();
    int op_fblock();
    int op_xinit();
    int op_xstop();
    int op_xzero();
    int op_xcont();

    int op_rep();
    int op_coginit();

    int op_qmul();
    int op_qdiv();
    int op_qfrac();
    int op_qsqrt();
    int op_qrotate();
    int op_qvector();

    int op_hubset();
    int op_cogid();
    int op_cogstop();
    int op_locknew();
    int op_lockret();
    int op_locktry();
    int op_lockrel();
    int op_qlog();
    int op_qexp();

    int op_rfbyte();
    int op_rfword();
    int op_rflong();
    int op_rfvar();
    int op_rfvars();
    int op_wfbyte();
    int op_wfword();
    int op_wflong();

    int op_getqx();
    int op_getqy();
    int op_getct();
    int op_getrnd();
    int op_getrnd_cz();

    int op_setdacs();
    int op_setxfrq();
    int op_getxacc();
    int op_waitx();
    int op_setse1();
    int op_setse2();
    int op_setse3();
    int op_setse4();
    int op_pollint();
    int op_pollct1();
    int op_pollct2();
    int op_pollct3();
    int op_pollse1();
    int op_pollse2();
    int op_pollse3();
    int op_pollse4();
    int op_pollpat();
    int op_pollfbw();
    int op_pollxmt();
    int op_pollxfi();
    int op_pollxro();
    int op_pollxrl();
    int op_pollatn();
    int op_pollqmt();

    int op_waitint();
    int op_waitct1();
    int op_waitct2();
    int op_waitct3();
    int op_waitse1();
    int op_waitse2();
    int op_waitse3();
    int op_waitse4();
    int op_waitpat();
    int op_waitfbw();
    int op_waitxmt();
    int op_waitxfi();
    int op_waitxro();
    int op_waitxrl();
    int op_waitatn();

    int op_allowi();
    int op_stalli();
    int op_trgint1();
    int op_trgint2();
    int op_trgint3();
    int op_nixint1();
    int op_nixint2();
    int op_nixint3();
    int op_setint1();
    int op_setint2();
    int op_setint3();
    int op_setq();
    int op_setq2();

    int op_push();
    int op_pop();
    int op_jmp();
    int op_call();
    int op_ret();
    int op_calla();
    int op_reta();
    int op_callb();
    int op_retb();

    int op_jmprel();
    int op_skip();
    int op_skipf();
    int op_execf();
    int op_getptr();
    int op_getbrk();
    int op_cogbrk();
    int op_brk();

    int op_setluts();
    int op_setcy();
    int op_setci();
    int op_setcq();
    int op_setcfrq();
    int op_setcmod();
    int op_setpiv();
    int op_setpix();
    int op_cogatn();

    int op_testp_w();
    int op_testpn_w();
    int op_testp_and();
    int op_testpn_and();
    int op_testp_or();
    int op_testpn_or();
    int op_testp_xor();
    int op_testpn_xor();

    int op_dirl();
    int op_dirh();
    int op_dirc();
    int op_dirnc();
    int op_dirz();
    int op_dirnz();
    int op_dirrnd();
    int op_dirnot();

    int op_outl();
    int op_outh();
    int op_outc();
    int op_outnc();
    int op_outz();
    int op_outnz();
    int op_outrnd();
    int op_outnot();

    int op_fltl();
    int op_flth();
    int op_fltc();
    int op_fltnc();
    int op_fltz();
    int op_fltnz();
    int op_fltrnd();
    int op_fltnot();

    int op_drvl();
    int op_drvh();
    int op_drvc();
    int op_drvnc();
    int op_drvz();
    int op_drvnz();
    int op_drvrnd();
    int op_drvnot();

    int op_splitb();
    int op_mergeb();
    int op_splitw();
    int op_mergew();
    int op_seussf();
    int op_seussr();
    int op_rgbsqz();
    int op_rgbexp();
    int op_xoro32();

    int op_rev();
    int op_rczr();
    int op_rczl();
    int op_wrc();
    int op_wrnc();
    int op_wrz();
    int op_wrnz();
    int op_modcz();
    int op_setscp();
    int op_getscp();

    int op_jmp_abs();
    int op_call_abs();
    int op_calla_abs();
    int op_callb_abs();

    int op_calld_pa_abs();
    int op_calld_pb_abs();
    int op_calld_ptra_abs();
    int op_calld_ptrb_abs();

    int op_loc_pa();
    int op_loc_pb();
    int op_loc_ptra();
    int op_loc_ptrb();

    int op_augs();
    int op_augd();
};
