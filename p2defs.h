/****************************************************************************
 *
 * Propeller2 enumerations and opcode structure
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
#include <QtEndian>

typedef quint8 p2_BYTE;
typedef quint16 p2_WORD;
typedef quint32 p2_LONG;

typedef union {
#if (Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
    p2_BYTE b0,b1,b2,b3;
    p2_BYTE b[4];
    p2_WORD w0, w1;
    p2_WORD w[2];
    p2_LONG l;
#elif (Q_BYTE_ORDER == Q_BIG_ENDIAN)
    p2_BYTE b3,b2,b1,b0;
    p2_BYTE b[4];
    p2_WORD w1, w0;
    p2_WORD w[2];
    p2_LONG l;
#else
#error "Unknown byte order!"
#endif
}   p2_BWL;

/**
 * @brief Enumeration of the 16 conditional execution modes
 */
typedef enum {
    cond__ret_,             //!< execute always; if no branch is taken, return
    cond_nc_and_nz,         //!< execute if C == 0 and Z == 0
    cond_nc_and_z,          //!< execute if C == 0 and Z == 1
    cond_nc,                //!< execute if C == 0
    cond_c_and_nz,          //!< execute if C == 1 and Z == 0
    cond_nz,                //!< execute if Z == 0
    cond_c_ne_z,            //!< execute if C != Z
    cond_nc_or_nz,          //!< execute if C == 0 or Z == 0
    cond_c_and_z,           //!< execute if C == 1 and Z == 1
    cond_c_eq_z,            //!< execute if C == Z
    cond_z,                 //!< execute if Z == 1
    cond_nc_or_z,           //!< execute if C == 0 or Z == 1
    cond_c,                 //!< execute if C == 1
    cond_c_or_nz,           //!< execute if C == 1 or Z == 0
    cond_c_or_z,            //!< execute if C == 1 or Z == 1
    cond_always             //!< execute always (default)
}   p2_cond_e;

/**
 * @brief Enumeration of the 128 possible instruction types
 * Note: some instructions use more bits from the opcode word
 */
typedef enum {
    p2_ror = 0u,        //!< 0000000
    p2_rol,             //!< 0000001
    p2_shr,             //!< 0000010
    p2_shl,             //!< 0000011
    p2_rcr,             //!< 0000100
    p2_rcl,             //!< 0000101
    p2_sar,             //!< 0000110
    p2_sal,             //!< 0000111

    p2_add,             //!< 0001000
    p2_addx,            //!< 0001001
    p2_adds,            //!< 0001010
    p2_addsx,           //!< 0001011
    p2_sub,             //!< 0001100
    p2_subx,            //!< 0001101
    p2_subs,            //!< 0001110
    p2_subsx,           //!< 0001111

    p2_cmp,             //!< 0010000
    p2_cmpx,            //!< 0010001
    p2_cmps,            //!< 0010010
    p2_cmpsx,           //!< 0010011
    p2_cmpr,            //!< 0010100
    p2_cmpm,            //!< 0010101
    p2_subr,            //!< 0010110
    p2_cmpsub,          //!< 0010111

    p2_fge,             //!< 0011000
    p2_fle,             //!< 0011001
    p2_fges,            //!< 0011010
    p2_fles,            //!< 0011011
    p2_sumc,            //!< 0011100
    p2_sumnc,           //!< 0011101
    p2_sumz,            //!< 0011110
    p2_sumnz,           //!< 0011111

    p2_testb_w,         //!< 0100000    also p2_bitl
    p2_testbn_w,        //!< 0100001    also p2_bith
    p2_testb_and,       //!< 0100010    also p2_bitc
    p2_testbn_and,      //!< 0100011    also p2_bitnc
    p2_testb_or,        //!< 0100100    also p2_bitz
    p2_testbn_or,       //!< 0100101    also p2_bitnz
    p2_testb_xor,       //!< 0100110    also p2_bitrnd
    p2_testbn_xor,      //!< 0100111    also p2_bitnot

    p2_and,             //!< 0101000
    p2_andn,            //!< 0101001
    p2_or,              //!< 0101010
    p2_xor,             //!< 0101011
    p2_muxc,            //!< 0101100
    p2_muxnc,           //!< 0101101
    p2_muxz,            //!< 0101110
    p2_muxnz,           //!< 0101111

    p2_mov,             //!< 0110000
    p2_not,             //!< 0110001
    p2_abs,             //!< 0110010
    p2_neg,             //!< 0110011
    p2_negc,            //!< 0110100
    p2_negnc,           //!< 0110101
    p2_negz,            //!< 0110110
    p2_negnz,           //!< 0110111

    p2_incmod,          //!< 0111000
    p2_decmod,          //!< 0111001
    p2_zerox,           //!< 0111010
    p2_signx,           //!< 0111011
    p2_encod,           //!< 0111100
    p2_ones,            //!< 0111101
    p2_test,            //!< 0111110
    p2_testn,           //!< 0111111

    p2_setnib_0,        //!< 1000000
    p2_setnib_1,        //!< 1000001
    p2_getnib_0,        //!< 1000010
    p2_getnib_1,        //!< 1000011
    p2_rolnib_0,        //!< 1000100
    p2_rolnib_1,        //!< 1000101
    p2_setbyte,         //!< 1000110
    p2_getbyte,         //!< 1000111

    p2_rolbyte,         //!< 1001000
    p2_1001001,         //!< 1001001
    p2_1001010,         //!< 1001010
    p2_1001011,         //!< 1001011
    p2_1001100,         //!< 1001100
    p2_1001101,         //!< 1001101
    p2_1001110,         //!< 1001110
    p2_1001111,         //!< 1001111

    p2_1010000,         //!< 1010000
    p2_1010001,         //!< 1010001
    p2_1010010,         //!< 1010010
    p2_1010011,         //!< 1010011
    p2_1010100,         //!< 1010100
    p2_rdlut,           //!< 1010101
    p2_rdbyte,          //!< 1010110
    p2_rdword,          //!< 1010111

    p2_rdlong,          //!< 1011000
    p2_calld,           //!< 1011001
    p2_callp,           //!< 1011010
    p2_1011011,         //!< 1011011
    p2_1011100,         //!< 1011100
    p2_1011101,         //!< 1011101
    p2_1011110,         //!< 1011110
    p2_1011111,         //!< 1011111

    p2_1100000,         //!< 1100000
    p2_1100001,         //!< 1100001
    p2_1100010,         //!< 1100010
    p2_1100011,         //!< 1100011
    p2_1100100,         //!< 1100100
    p2_1100101,         //!< 1100101
    p2_1100110,         //!< 1100110
    p2_coginit,         //!< 1100111

    p2_1101000,         //!< 1101000
    p2_1101001,         //!< 1101001
    p2_1101010,         //!< 1101010
    p2_1101011,         //!< 1101011

    p2_jmp_abs,         //!< 1101100
    p2_call_abs,        //!< 1101101
    p2_calla_abs,       //!< 1101110
    p2_callb_abs,       //!< 1101111

    p2_calld_pa_abs,    //!< 1110000
    p2_calld_pb_abs,    //!< 1110001
    p2_calld_ptra_abs,  //!< 1110010
    p2_calld_ptrb_abs,  //!< 1110011

    p2_loc_pa,          //!< 1110100
    p2_loc_pb,          //!< 1110101
    p2_loc_ptra,        //!< 1110110
    p2_loc_ptrb,        //!< 1110111

    p2_augs_00,         //!< 1111000
    p2_augs_01,         //!< 1111001
    p2_augs_10,         //!< 1111010
    p2_augs_11,         //!< 1111011

    p2_augd_00,         //!< 1111100
    p2_augd_01,         //!< 1111101
    p2_augd_10,         //!< 1111110
    p2_augd_11          //!< 1111111
}   p2_inst_e;

Q_STATIC_ASSERT(p2_augd_11 == 127);

/**
 * @brief Structure of the Propeller2 opcode words
 *
 * Note: To have the bit fields work for both, little
 * and big endian machines, the order has to match
 * the native order of bits in 32 bit words of the target.
 */
typedef struct {
#if (Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
    unsigned src:9;             //!< source (S or #S)
    unsigned dst:9;             //!< destination (D or #D)
    bool imm:1;                 //!< immediate flag
    bool wz:1;                  //!< update Z flag
    bool wc:1;                  //!< update C flag
    unsigned inst:7;            //!< instruction type
    unsigned cond:4;            //!< conditional execution
#elif (Q_BYTE_ORDER == Q_BIG_ENDIAN)
    unsigned cond:4;            //!< conditional execution
    unsigned inst:7;            //!< instruction type
    bool uc:1;                  //!< update C flag
    bool uz:1;                  //!< update Z flag
    bool imm:1;                 //!< immediate flag
    unsigned dst:9;             //!< destination (D or #D)
    unsigned src:9;             //!< source (S or #S)
#else
#error "Unknown byte order!"
#endif
}   p2_opcode_t;

/**
 * @brief union of a 32 bit word and the opcode bit fields
 */
typedef union {
    unsigned int word;          //!< opcode as 32 bit word
    p2_opcode_t op;             //!< ocpode as bit fields
}   p2_opword_t;

typedef struct {
    p2_LONG RAM[512-16];        //!< general-use code/data registers
    p2_LONG IJMP3;              //!< interrupt call address for INT3
    p2_LONG IRET3;              //!< interrupt return address for INT3
    p2_LONG IJMP2;              //!< interrupt call address for INT2
    p2_LONG IRET2;              //!< interrupt return address for INT2
    p2_LONG IJMP1;              //!< interrupt call address for INT1
    p2_LONG IRET1;              //!< interrupt return address for INT1
    p2_LONG PA;                 //!< CALLD-imm return, CALLPA parameter, or LOC address
    p2_LONG PB;                 //!< CALLD-imm return, CALLPB parameter, or LOC address
    p2_LONG PTRA;               //!< pointer A to hub RAM
    p2_LONG PTRB;               //!< pointer B to hub RAM
    p2_LONG DIRA;               //!< output enables for P31 ... P0
    p2_LONG DIRB;               //!< output enables for P63 ... P32
    p2_LONG OUTA;               //!< output states for P31 ... P0
    p2_LONG OUTB;               //!< output states for P63 ... P32
    p2_LONG INA;                //!< input states for P31 ... P0
    p2_LONG INB;                //!< input states for P63 ... P32
}   p2_lutregs_t;

typedef enum {
    offs_IJMP3 = offsetof(p2_lutregs_t, IJMP3) / sizeof(p2_LONG),
    offs_IRET3 = offsetof(p2_lutregs_t, IRET3) / sizeof(p2_LONG),
    offs_IJMP2 = offsetof(p2_lutregs_t, IJMP2) / sizeof(p2_LONG),
    offs_IRET2 = offsetof(p2_lutregs_t, IRET2) / sizeof(p2_LONG),
    offs_IJMP1 = offsetof(p2_lutregs_t, IJMP1) / sizeof(p2_LONG),
    offs_IRET1 = offsetof(p2_lutregs_t, IRET1) / sizeof(p2_LONG),
    offs_PA    = offsetof(p2_lutregs_t,    PA) / sizeof(p2_LONG),
    offs_PB    = offsetof(p2_lutregs_t,    PB) / sizeof(p2_LONG),
    offs_PTRA  = offsetof(p2_lutregs_t,  PTRA) / sizeof(p2_LONG),
    offs_PTRB  = offsetof(p2_lutregs_t,  PTRB) / sizeof(p2_LONG),
    offs_DIRA  = offsetof(p2_lutregs_t,  DIRA) / sizeof(p2_LONG),
    offs_DIRB  = offsetof(p2_lutregs_t,  DIRB) / sizeof(p2_LONG),
    offs_OUTA  = offsetof(p2_lutregs_t,  OUTA) / sizeof(p2_LONG),
    offs_OUTB  = offsetof(p2_lutregs_t,  OUTB) / sizeof(p2_LONG),
    offs_INA   = offsetof(p2_lutregs_t,   INA) / sizeof(p2_LONG),
    offs_INB   = offsetof(p2_lutregs_t,   INB) / sizeof(p2_LONG)
}   p2_lutregs_e;

Q_STATIC_ASSERT(offs_INB == 0x1ff);

/**
 * @brief union of the COG memory (shadow registers ?)
 */
typedef union {
    p2_LONG RAM[512];
}   p2_cog_t;

/**
 * @brief union of the LUT memory and shadow registers
 */
typedef union {
    p2_LONG RAM[512];
    p2_lutregs_t REG;
}   p2_lut_t;

typedef struct {
    bool    f_INT:1;            //!< INT interrupt flag
    bool    f_CT1:1;            //!< CT1 counter 1 flag
    bool    f_CT2:1;            //!< CT2 counter 2 flag
    bool    f_CT3:1;            //!< CT3 counter 3 flag
    bool    f_SE1:1;            //!< SE1 set event 1 flag
    bool    f_SE2:1;            //!< SE2 set event 2 flag
    bool    f_SE3:1;            //!< SE3 set event 3 flag
    bool    f_SE4:1;            //!< SE4 set event 4 flag
    bool    f_PAT:1;            //!< PAT pattern flag
    bool    f_FBW:1;            //!< FBW flag
    bool    f_XMT:1;            //!< XMT flag
    bool    f_XFI:1;            //!< XFI flag
    bool    f_XRO:1;            //!< XRO flag
    bool    f_XRL:1;            //!< XRL flag
    bool    f_ATN:1;            //!< ATN COG attention flag
    bool    f_QMT:1;            //!< QMT Q empty flag
}   p2_flags_t;
