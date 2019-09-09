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

    //! return the %n bit sign extended value for val[n:0]
    template <typename T, int n>
    T SXn(T val) {
        Q_STATIC_ASSERT(n > 0 && n <= 64);
        return static_cast<T>((val ^ (Q_UINT64_C(1)<<(n-1))) - (Q_UINT64_C(1)<<(n-1)));
    }

    //! return the %n bit zero extended value for val[n:0]
    template <typename T, int n>
    T ZXn(T val) {
        Q_STATIC_ASSERT(n > 0 && n <= 64);
        return static_cast<T>(val & (Q_UINT64_C(1)<<(n-1)));
    }

    //! return a signed 32 bit value for val[15:0]
    template <typename T>
    qint32 S16(T val) {
        return static_cast<qint32>(SXn<T,16>(val));
    }

    //! return the usigned 32 bit value for val[15:0]
    template <typename T>
    p2_LONG U16(T val) {
        return static_cast<p2_LONG>(ZXn<T,16>(val));
    }

    //! return the signed 32 bit value for val[31:0]
    template <typename T>
    qint32 S32(T val) {
        return static_cast<qint32>(SXn<T,32>(val));
    }

    //! return the usigned 32 bit value for val[31:0]
    template <typename T>
    p2_LONG U32(T val) {
        return static_cast<p2_LONG>(ZXn<T,32>(val));
    }

    //! return the 64 bit sign extended value
    template <typename T>
    qint64 SX64(T val) {
        return static_cast<qint64>(SXn<T,64>(val));
    }

    //! return the usigned 64 bit value
    template <typename T>
    p2_QUAD U64(T val) {
        return static_cast<p2_QUAD>(ZXn<T,64>(val));
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
    void updateQ(p2_LONG d);
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

    int op_NOP();
    int op_ROR();
    int op_ROL();
    int op_SHR();
    int op_SHL();
    int op_RCR();
    int op_RCL();
    int op_SAR();
    int op_SAL();

    int op_ADD();
    int op_ADDX();
    int op_ADDS();
    int op_ADDSX();
    int op_SUB();
    int op_SUBX();
    int op_SUBS();
    int op_SUBSX();

    int op_CMP();
    int op_CMPX();
    int op_CMPS();
    int op_CMPSX();
    int op_CMPR();
    int op_CMPM();
    int op_SUBR();
    int op_CMPSUB();

    int op_FGE();
    int op_FLE();
    int op_FGES();
    int op_FLES();
    int op_SUMC();
    int op_SUMNC();
    int op_SUMZ();
    int op_SUMNZ();

    int op_TESTB_W();
    int op_TESTBN_W();
    int op_TESTB_AND();
    int op_TESTBN_AND();
    int op_TESTB_OR();
    int op_TESTBN_OR();
    int op_TESTB_XOR();
    int op_TESTBN_XOR();

    int op_BITL();
    int op_BITH();
    int op_BITC();
    int op_BITNC();
    int op_BITZ();
    int op_BITNZ();
    int op_BITRND();
    int op_BITNOT();

    int op_AND();
    int op_ANDN();
    int op_OR();
    int op_XOR();
    int op_MUXC();
    int op_MUXNC();
    int op_MUXZ();
    int op_MUXNZ();

    int op_MOV();
    int op_NOT();
    int op_ABS();
    int op_NEG();
    int op_NEGC();
    int op_NEGNC();
    int op_NEGZ();
    int op_NEGNZ();

    int op_INCMOD();
    int op_DECMOD();
    int op_ZEROX();
    int op_SIGNX();
    int op_ENCOD();
    int op_ONES();
    int op_TEST();
    int op_TESTN();

    int op_SETNIB();
    int op_SETNIB_ALTSN();
    int op_GETNIB();
    int op_GETNIB_ALTGN();
    int op_ROLNIB();
    int op_ROLNIB_ALTGN();
    int op_SETBYTE();
    int op_SETBYTE_ALTSB();
    int op_GETBYTE();
    int op_GETBYTE_ALTGB();
    int op_ROLBYTE();
    int op_ROLBYTE_ALTGB();

    int op_SETWORD();
    int op_SETWORD_ALTSW();
    int op_GETWORD();
    int op_GETWORD_ALTGW();
    int op_ROLWORD();
    int op_ROLWORD_ALTGW();

    int op_ALTSN();
    int op_ALTSN_D();
    int op_ALTGN();
    int op_ALTGN_D();
    int op_ALTSB();
    int op_ALTSB_D();
    int op_ALTGB();
    int op_ALTGB_D();

    int op_ALTSW();
    int op_ALTSW_D();
    int op_ALTGW();
    int op_ALTGW_D();

    int op_ALTR();
    int op_ALTR_D();
    int op_ALTD();
    int op_ALTD_D();
    int op_ALTS();
    int op_ALTS_D();
    int op_ALTB();
    int op_ALTB_D();
    int op_ALTI();
    int op_ALTI_D();

    int op_SETR();
    int op_SETD();
    int op_SETS();
    int op_DECOD();
    int op_DECOD_D();
    int op_BMASK();
    int op_BMASK_D();
    int op_CRCBIT();
    int op_CRCNIB();

    int op_MUXNITS();
    int op_MUXNIBS();
    int op_MUXQ();
    int op_MOVBYTS();
    int op_MUL();
    int op_MULS();
    int op_SCA();
    int op_SCAS();

    int op_ADDPIX();
    int op_MULPIX();
    int op_BLNPIX();
    int op_MIXPIX();
    int op_ADDCT1();
    int op_ADDCT2();
    int op_ADDCT3();

    int op_WMLONG();
    int op_RQPIN();
    int op_RDPIN();
    int op_RDLUT();
    int op_RDBYTE();
    int op_RDWORD();
    int op_RDLONG();

    int op_POPA();
    int op_POPB();
    int op_CALLD();
    int op_RESI3();
    int op_RESI2();
    int op_RESI1();
    int op_RESI0();
    int op_RETI3();
    int op_RETI2();
    int op_RETI1();
    int op_RETI0();
    int op_CALLPA();
    int op_CALLPB();

    int op_DJZ();
    int op_DJNZ();
    int op_DJF();
    int op_DJNF();
    int op_IJZ();
    int op_IJNZ();
    int op_TJZ();
    int op_TJNZ();
    int op_TJF();
    int op_TJNF();
    int op_TJS();
    int op_TJNS();
    int op_TJV();

    int op_JINT();
    int op_JCT1();
    int op_JCT2();
    int op_JCT3();
    int op_JSE1();
    int op_JSE2();
    int op_JSE3();
    int op_JSE4();
    int op_JPAT();
    int op_JFBW();
    int op_JXMT();
    int op_JXFI();
    int op_JXRO();
    int op_JXRL();
    int op_JATN();
    int op_JQMT();

    int op_JNINT();
    int op_JNCT1();
    int op_JNCT2();
    int op_JNCT3();
    int op_JNSE1();
    int op_JNSE2();
    int op_JNSE3();
    int op_JNSE4();
    int op_JNPAT();
    int op_JNFBW();
    int op_JNXMT();
    int op_JNXFI();
    int op_JNXRO();
    int op_JNXRL();
    int op_JNATN();
    int op_JNQMT();
    int op_1011110_1();
    int op_1011111_0();
    int op_SETPAT();

    int op_WRPIN();
    int op_AKPIN();
    int op_WXPIN();
    int op_WYPIN();
    int op_WRLUT();
    int op_WRBYTE();
    int op_WRWORD();
    int op_WRLONG();

    int op_PUSHA();
    int op_PUSHB();
    int op_RDFAST();
    int op_WRFAST();
    int op_FBLOCK();
    int op_XINIT();
    int op_XSTOP();
    int op_XZERO();
    int op_XCONT();

    int op_REP();
    int op_COGINIT();

    int op_QMUL();
    int op_QDIV();
    int op_QFRAC();
    int op_QSQRT();
    int op_QROTATE();
    int op_QVECTOR();

    int op_HUBSET();
    int op_COGID();
    int op_COGSTOP();
    int op_LOCKNEW();
    int op_LOCKRET();
    int op_LOCKTRY();
    int op_LOCKREL();
    int op_QLOG();
    int op_QEXP();

    int op_RFBYTE();
    int op_RFWORD();
    int op_RFLONG();
    int op_RFVAR();
    int op_RFVARS();
    int op_WFBYTE();
    int op_WFWORD();
    int op_WFLONG();

    int op_GETQX();
    int op_GETQY();
    int op_GETCT();
    int op_GETRND();
    int op_GETRND_CZ();

    int op_SETDACS();
    int op_SETXFRQ();
    int op_GETACC();
    int op_WAITX();
    int op_SETSE1();
    int op_SETSE2();
    int op_SETSE3();
    int op_SETSE4();
    int op_POLLINT();
    int op_POLLCT1();
    int op_POLLCT2();
    int op_POLLCT3();
    int op_POLLSE1();
    int op_POLLSE2();
    int op_POLLSE3();
    int op_POLLSE4();
    int op_POLLPAT();
    int op_POLLFBW();
    int op_POLLXMT();
    int op_POLLXFI();
    int op_POLLXRO();
    int op_POLLXRL();
    int op_POLLATN();
    int op_POLLQMT();

    int op_WAITINT();
    int op_WAITCT1();
    int op_WAITCT2();
    int op_WAITCT3();
    int op_WAITSE1();
    int op_WAITSE2();
    int op_WAITSE3();
    int op_WAITSE4();
    int op_WAITPAT();
    int op_WAITFBW();
    int op_WAITXMT();
    int op_WAITXFI();
    int op_WAITXRO();
    int op_WAITXRL();
    int op_WAITATN();

    int op_ALLOWI();
    int op_STALLI();
    int op_TRGINT1();
    int op_TRGINT2();
    int op_TRGINT3();
    int op_NIXINT1();
    int op_NIXINT2();
    int op_NIXINT3();
    int op_SETINT1();
    int op_SETINT2();
    int op_SETINT3();
    int op_SETQ();
    int op_SETQ2();

    int op_PUSH();
    int op_POP();
    int op_JMP();
    int op_CALL();
    int op_RET();
    int op_CALLA();
    int op_RETA();
    int op_CALLB();
    int op_RETB();

    int op_JMPREL();
    int op_SKIP();
    int op_SKIPF();
    int op_EXECF();
    int op_GETPTR();
    int op_GETBRK();
    int op_COGBRK();
    int op_BRK();

    int op_SETLUTS();
    int op_SETCY();
    int op_SETCI();
    int op_SETCQ();
    int op_SETCFRQ();
    int op_SETCMOD();
    int op_SETPIV();
    int op_SETPIX();
    int op_COGATN();

    int op_TESTP_W();
    int op_TESTPN_W();
    int op_TESTP_AND();
    int op_TESTPN_AND();
    int op_TESTP_OR();
    int op_TESTPN_OR();
    int op_TESTP_XOR();
    int op_TESTPN_XOR();

    int op_DIRL();
    int op_DIRH();
    int op_DIRC();
    int op_DIRNC();
    int op_DIRZ();
    int op_DIRNZ();
    int op_DIRRND();
    int op_DIRNOT();

    int op_OUTL();
    int op_OUTH();
    int op_OUTC();
    int op_OUTNC();
    int op_OUTZ();
    int op_OUTNZ();
    int op_OUTRND();
    int op_OUTNOT();

    int op_FLTL();
    int op_FLTH();
    int op_FLTC();
    int op_FLTNC();
    int op_FLTZ();
    int op_FLTNZ();
    int op_FLTRND();
    int op_FLTNOT();

    int op_DRVL();
    int op_DRVH();
    int op_DRVC();
    int op_DRVNC();
    int op_DRVZ();
    int op_DRVNZ();
    int op_DRVRND();
    int op_DRVNOT();

    int op_SPLITB();
    int op_MERGEB();
    int op_SPLITW();
    int op_MERGEW();
    int op_SEUSSF();
    int op_SEUSSR();
    int op_RGBSQZ();
    int op_RGBEXP();
    int op_XORO32();

    int op_REV();
    int op_RCZR();
    int op_RCZL();
    int op_WRC();
    int op_WRNC();
    int op_WRZ();
    int op_WRNZ();
    int op_MODCZ();
    int op_SETSCP();
    int op_GETSCP();

    int op_JMP_ABS();
    int op_CALL_ABS();
    int op_CALLA_ABS();
    int op_CALLB_ABS();

    int op_CALLD_ABS_PA();
    int op_CALLD_ABS_PB();
    int op_CALLD_ABS_PTRA();
    int op_CALLD_ABS_PTRB();

    int op_LOC_PA();
    int op_LOC_PB();
    int op_LOC_PTRA();
    int op_LOC_PTRB();

    int op_AUGS();
    int op_AUGD();
};
