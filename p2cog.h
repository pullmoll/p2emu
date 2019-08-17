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

    p2_opword_t rd_IR() const { return IR; }
    P2LONG rd_ID() const { return ID; }
    P2LONG rd_PC() const { return PC; }
    p2_wait_t rd_WAIT() const { return WAIT; }
    p2_flags_t rd_FLAGS() const { return FLAGS; }
    P2LONG rd_CT1() const { return CT1; }
    P2LONG rd_CT2() const { return CT2; }
    P2LONG rd_CT3() const { return CT3; }
    p2_pat_t rd_PAT() const { return PAT; }
    p2_pin_t rd_PIN() const { return PIN; }
    p2_int_bits_u rd_INT() const { return INT; }
    p2_lock_t rd_LOCK() const { return LOCK; }
    P2LONG rd_D() const { return D; }
    P2LONG rd_S() const { return S; }
    P2LONG rd_Q() const { return Q; }
    P2LONG rd_C() const { return C; }
    P2LONG rd_Z() const { return Z; }

    P2LONG rd_cog(P2LONG addr) const;
    void wr_cog(P2LONG addr, P2LONG val);

    P2LONG rd_lut(P2LONG addr) const;
    void wr_lut(P2LONG addr, P2LONG val);

    P2LONG rd_mem(P2LONG addr) const;
    void wr_mem(P2LONG addr, P2LONG val);

private:
    P2Hub* HUB;             //!< pointer to the HUB, i.e. the parent of this P2Cog
    P2LONG ID;             //!< COG ID (0 … number of COGs - 1)
    P2LONG PC;             //!< program counter
    p2_wait_t WAIT;         //!< waiting conidition
    p2_flags_t FLAGS;       //!< flags register
    P2LONG CT1;            //!< counter CT1 value
    P2LONG CT2;            //!< counter CT2 value
    P2LONG CT3;            //!< counter CT3 value
    p2_pat_t PAT;           //!< PAT mode, mask, and match
    p2_pin_t PIN;           //!< PIN mode, mask, and match
    p2_int_bits_u INT;      //!< INT disable / active / source bits union
    p2_lock_t LOCK;         //!<
    p2_opword_t IR;         //!< instruction register
    P2LONG D;              //!< value of D
    P2LONG S;              //!< value of S
    P2LONG Q;              //!< value of Q
    P2LONG C;              //!< current carry flag
    P2LONG Z;              //!< current zero flag
    p2_fifo_t FIFO;         //!< stream FIFO
    P2LONG K;              //!< stack pointer (0 … 7)
    P2LONG STACK[8];       //!< stack of 8 levels
    QVariant S_next;        //!< next instruction's S value
    QVariant S_aug;         //!< augment next S with this value, if set
    QVariant D_aug;         //!< augment next D with this value, if set
    QVariant R_aug;         //!< augment next R with this value, if set
    QVariant IR_aug;        //!< augment next IR with this value, if set
    P2LONG PTRA0;          //!< actual pointer A to hub RAM
    P2LONG PTRB0;          //!< actual pointer B to hub RAM
    P2LONG HUBOP;          //!< non-zero if HUB operation
    P2LONG CORDIC_count;   //!< non-zero if CORDIC solver running
    bool QX_posted;         //!< true if CORDIC solver X is posted
    bool QY_posted;         //!< true if CORDIC solver Y is posted
    bool RW_repeat;         //!< true if read/write HUB repeated
    P2LONG RDL_mask;       //!<
    P2LONG RDL_flags0;     //!<
    P2LONG RDL_flags1;     //!<
    P2LONG WRL_mask;       //!<
    P2LONG WRL_flags0;     //!<
    P2LONG WRL_flags1;     //!<

    p2_cog_t COG;           //!< COG memory (512 longs)
    p2_lut_t LUT;           //!< LUT memory (512 longs)
    uchar *MEM;             //!< HUB memory pointer
    P2LONG MEMSIZE;        //!< HUB memory size

    //! update C if the WC flag bit is set
    template <typename T>
    void updateC(T c) {
        if (IR.op.wc)
            C = static_cast<P2LONG>(c) & 1;
    }

    //! update Z if the WC flag bit is set
    template <typename T>
    void updateZ(T z) {
        if (IR.op.wz)
            Z = static_cast<P2LONG>(z) & 1;
    }

    //! update D, i.e. write result to COG
    template <typename T>
    void updateD(T d) {
        COG.RAM[D] = static_cast<P2LONG>(d);
    }

    //! update PC (program counter)
    template <typename T>
    void updatePC(T d) {
        // TODO: handle switch between COG, LUT, and HUB ?
        PC = static_cast<P2LONG>(d);
    }

    //! update PC (program counter)
    template <typename T>
    void updateLUT(P2LONG addr, T d) {
        LUT.RAM[addr & 0x1ff] = static_cast<P2LONG>(d);
    }

    //! push val to the 8 level stack
    template <typename T>
    void pushK(T val) {
        K = (K - 1) & 7;
        STACK[K] = static_cast<P2LONG>(val);
    }

    //! pop val from the 8 level stack
    template <typename T>
    T popK() {
        P2LONG val = static_cast<T>(STACK[K]);
        K = (K + 1) & 7;
        return static_cast<T>(val);
    }

    //! push val to PA
    template <typename T>
    void pushPA(T val) {
        LUT.REG.PA = static_cast<P2LONG>(val);
    }

    //! push val to PB
    template <typename T>
    void pushPB(T val) {
        LUT.REG.PB = static_cast<P2LONG>(val);
    }

    //! push val to PTRA++
    template <typename T>
    void pushPTRA(T val) {
        HUB->wr_LONG(LUT.REG.PTRA, static_cast<P2LONG>(val));
        LUT.REG.PTRA = (LUT.REG.PTRA + 4) & A20MASK;
    }

    //! push val to PTRB++
    template <typename T>
    void pushPTRB(T val) {
        HUB->wr_LONG(LUT.REG.PTRB, static_cast<P2LONG>(val));
        LUT.REG.PTRB = (LUT.REG.PTRB + 4) & A20MASK;
    }

    //! augment #S or use #S, if the flag bit is set, then possibly set S from next_S
    template <typename T>
    void augmentS(T f) {
        if (f) {
            if (S_aug.isValid()) {
                S = S_aug.toUInt() | IR.op.src;
                S_aug.clear();
            } else {
                S = IR.op.src;
            }
        }
        if (S_next.isValid()) {
            // set S to next_S
            S = S_next.toUInt();
            S_next.clear();
        }
    }

    //! augment #D or use #D, if the flag bit is set
    template <typename T>
    void augmentD(T f) {
        if (f) {
            if (D_aug.isValid()) {
                D = D_aug.toUInt() | IR.op.dst;
                D_aug.clear();
            } else {
                D = IR.op.dst;
            }
        }
    }

    //! update PA, i.e. write result to port A
    template <typename T>
    void updatePA(T d) {
        Q_ASSERT(HUB);
        LUT.REG.PA = static_cast<P2LONG>(d);
        HUB->wr_PA(LUT.REG.PA);
    }

    //! update PB, i.e. write result to port A
    template <typename T>
    void updatePB(T d) {
        Q_ASSERT(HUB);
        LUT.REG.PB = static_cast<P2LONG>(d);
        HUB->wr_PB(LUT.REG.PA);
    }

    //! update PTRA, i.e. write result to pointer A
    template <typename T>
    void updatePTRA(T d) {
        Q_ASSERT(HUB);
        HUB->wr_LONG(LUT.REG.PTRA, static_cast<P2LONG>(d));
    }

    //! update PTRB, i.e. write result to pointer B
    template <typename T>
    void updatePTRB(T d) {
        Q_ASSERT(HUB);
        HUB->wr_LONG(LUT.REG.PTRB, static_cast<P2LONG>(d));
    }

    //! update DIR bit
    template <typename T>
    void updateDIR(P2LONG port, T v)
    {
        HUB->wr_DIR(port, static_cast<P2LONG>(v) & 1);
    }

    //! update OUT bit
    template <typename T>
    void updateOUT(P2LONG port, T v)
    {
        HUB->wr_OUT(port, static_cast<P2LONG>(v) & 1);
    }

    //! return a signed 32 bit value for val[15:0]
    template <typename T>
    qint32 S16(T val) {
        return static_cast<qint32>(static_cast<qint16>(val));
    }

    //! return the usigned 32 bit value for val[15:0]
    template <typename T>
    P2LONG U16(T val) {
        return static_cast<P2LONG>(static_cast<P2WORD>(val));
    }

    //! return the signed 32 bit value for val[31:0]
    template <typename T>
    qint32 S32(T val) {
        return static_cast<qint32>(val);
    }

    //! return the usigned 32 bit value for val[31:0]
    template <typename T>
    P2LONG U32(T val) {
        return static_cast<P2LONG>(val);
    }

    //! return the 64 bit sign extended value
    template <typename T>
    qint64 SX64(T val) {
        return static_cast<qint64>(static_cast<qint32>(val));
    }

    //! return the usigned 64 bit value
    template <typename T>
    quint64 U64(T val) {
        return static_cast<quint64>(val);
    }

    //! return the upper half of a 64 bit value as 32 bit unsigned
    template <typename T>
    P2LONG U32H(T val) {
        return static_cast<P2LONG>(static_cast<quint64>(val) >> 32);
    }

    //! return the lower half of a 64 bit value as 32 bit unsigned
    template <typename T>
    P2LONG U32L(T val) {
        return static_cast<P2LONG>(val);
    }

    //! most significant bit in a 32 bit word
    static const P2LONG MSB = 1u << 31;

    //! least significant bit in a 32 bit word
    static const P2LONG LSB = 1u;

    //! least significant nibble in a 32 bit word
    static const P2LONG LNIBBLE = 0x0000000fu;

    //! least significant byte in a 32 bit word
    static const P2LONG LBYTE = 0x000000ffu;

    //! least significant word in a 32 bit word
    static const P2LONG LWORD = 0x0000ffffu;

    //! most significant word in a 32 bit word
    static const P2LONG HWORD = 0xffff0000u;

    //! bits without sign bit in a 32 bit word
    static const P2LONG IMAX = 0x7fffffffu;

    //! all bits in a 32 bit word
    static const P2LONG UMAX = 0xffffffffu;

    //! least significant 20 bits for an address value
    static const P2LONG A20MASK = (1u << 20) - 1;

    //! most significant 23 bits for an augmentation value
    static const P2LONG AUG = 0xfffffe00;

    //! upper word max / mask in a 64 bit unsigned
    static const quint64 HMAX = Q_UINT64_C(0xffffffff00000000);

    //! lower word max / mask in a 64 bit unsigned
    static const quint64 LMAX = Q_UINT64_C(0x00000000ffffffff);

    bool conditional(p2_cond_e cond);
    bool conditional(unsigned cond);
    P2BYTE msbit(P2LONG val);
    P2BYTE ones(P2LONG val);
    P2BYTE parity(P2LONG val);
    P2LONG seuss(P2LONG val, bool forward = true);
    P2LONG reverse(P2LONG val);
    P2LONG fifo_level();
    void check_interrupt_flags();
    void check_wait_int_state();
    P2LONG check_wait_flag(p2_opword_t IR, P2LONG value1, P2LONG value2, bool streamflag);
    P2LONG get_pointer(P2LONG inst, P2LONG size);
    void save_regs();
    void update_regs();

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
