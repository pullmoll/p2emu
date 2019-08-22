/****************************************************************************
 *
 * Propeller2 constants, enumerations, and opcode structure
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

//!< Type of the Propeller2 BYTE
typedef quint8 p2_BYTE;

//!< Type of the Propeller2 WORD
typedef quint16 p2_WORD;

//!< Type of the Propeller2 LONG
typedef quint32 p2_LONG;

//!< Intermediate type (unsigned long long); not a Propeller2 type
typedef quint64 p2_QUAD;

//!< Type for an array (vector) of BYTEs
typedef QVector<p2_BYTE> p2_BYTEs;

//!< Type for an array (vector) of WORDs
typedef QVector<p2_WORD> p2_WORDs;

//!< Type for an array (vector) of LONGs
typedef QVector<p2_LONG> p2_LONGs;

/// Union of bytes, words, and a long in endianess aware ordering
typedef union {
#if (Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
    p2_LONG l;               //!< long
    p2_WORD w0, w1;          //!< words 0, 1 as least significant word first
    p2_WORD w[2];            //!< 2 words as an array
    p2_BYTE b0, b1, b2, b3;  //!< bytes 0, 1, 2, 3 as least significant byte first
    p2_BYTE b[4];            //!< 4 bytes as an array
#elif (Q_BYTE_ORDER == Q_BIG_ENDIAN)
    p2_LONG l;               //!< long
    p2_WORD w1, w0;          //!< words 1, 0 as most significant word first
    p2_WORD w[2];            //!< 2 words as an array
    p2_BYTE b3, b2, b1, b0;  //!< bytes 3, 2, 1, 0 as most significant byte first
    p2_BYTE b[4];            //!< 4 bytes as an array
#else
#error "Unknown byte order!"
#endif
}   p2_BWL;

//! Size of the HUB memory in bytes (1MiB)
constexpr p2_LONG MEM_SIZE = 0x100000;

extern const p2_LONG COG_SIZE;
extern const p2_LONG COG_MASK;
extern const p2_LONG LUT_SIZE;
extern const p2_LONG LUT_MASK;

extern const p2_LONG PC_LONGS;
extern const p2_LONG HUB_ADDR;
extern const p2_LONG MSB;
extern const p2_LONG LSB;
extern const p2_LONG LNIBBLE;
extern const p2_LONG LBYTE;
extern const p2_LONG LWORD;
extern const p2_LONG HWORD;
extern const p2_LONG IMAX;
extern const p2_LONG ZERO;
extern const p2_LONG FULL;
extern const p2_LONG A20MASK;
extern const p2_LONG AUGMASK;
extern const p2_QUAD HMAX;
extern const p2_QUAD LMAX;

extern const char bin_digits[];
extern const char byt_digits[];
extern const char oct_digits[];
extern const char dec_digits[];
extern const char hex_digits[];

/**
 * @brief Enumeration of the 16 conditional execution modes
 */
typedef enum {
    cc_clr,                     //!< clear (never)
    cc__ret_  = cc_clr,         //!< actually always and, if no branch is taken, return
    cc_nc_and_nz,               //!< if C == 0 and Z == 0
    cc_nc_and_z,                //!< if C == 0 and Z == 1
    cc_nc,                      //!< if C == 0
    cc_c_and_nz,                //!< if C == 1 and Z == 0
    cc_nz,                      //!< if Z == 0
    cc_c_ne_z,                  //!< if C != Z
    cc_nc_or_nz,                //!< if C == 0 or Z == 0
    cc_c_and_z,                 //!< if C == 1 and Z == 1
    cc_c_eq_z,                  //!< if C == Z
    cc_z,                       //!< if Z == 1
    cc_nc_or_z,                 //!< if C == 0 or Z == 1
    cc_c,                       //!< if C == 1
    cc_c_or_nz,                 //!< if C == 1 or Z == 0
    cc_c_or_z,                  //!< if C == 1 or Z == 1
    cc_always                   //!< always (default)
}   p2_cond_e;

//! define an instruction with 7 bits
#define INST7(b6,b5,b4,b3,b2,b1,b0) ((b6<<6)|(b5<<5)|(b4<<4)|(b3<<3)|(b2<<2)|(b1<<1)|(b0))

//! extend an instruction to 8 bits using wc
#define INST8(inst,b0)              ((inst<<1)|(b0))

//! extend an instruction to 9 bits using wc and wz
#define INST9(inst,b1,b0)           ((inst<<2)|(b1<<1)|(b0))

/**
 * @brief Enumeration of the 128 possible instruction types
 */
typedef enum {
    p2_ROR                      = INST7(0,0,0,0,0,0,0),
    p2_ROL                      = INST7(0,0,0,0,0,0,1),
    p2_SHR                      = INST7(0,0,0,0,0,1,0),
    p2_SHL                      = INST7(0,0,0,0,0,1,1),
    p2_RCR                      = INST7(0,0,0,0,1,0,0),
    p2_RCL                      = INST7(0,0,0,0,1,0,1),
    p2_SAR                      = INST7(0,0,0,0,1,1,0),
    p2_SAL                      = INST7(0,0,0,0,1,1,1),

    p2_ADD                      = INST7(0,0,0,1,0,0,0),
    p2_ADDX                     = INST7(0,0,0,1,0,0,1),
    p2_ADDS                     = INST7(0,0,0,1,0,1,0),
    p2_ADDSX                    = INST7(0,0,0,1,0,1,1),
    p2_SUB                      = INST7(0,0,0,1,1,0,0),
    p2_SUBX                     = INST7(0,0,0,1,1,0,1),
    p2_SUBS                     = INST7(0,0,0,1,1,1,0),
    p2_SUBSX                    = INST7(0,0,0,1,1,1,1),

    p2_CMP                      = INST7(0,0,1,0,0,0,0),
    p2_CMPX                     = INST7(0,0,1,0,0,0,1),
    p2_CMPS                     = INST7(0,0,1,0,0,1,0),
    p2_CMPSX                    = INST7(0,0,1,0,0,1,1),
    p2_CMPR                     = INST7(0,0,1,0,1,0,0),
    p2_CMPM                     = INST7(0,0,1,0,1,0,1),
    p2_SUBR                     = INST7(0,0,1,0,1,1,0),
    p2_CMPSUB                   = INST7(0,0,1,0,1,1,1),

    p2_FGE                      = INST7(0,0,1,1,0,0,0),
    p2_FLE                      = INST7(0,0,1,1,0,0,1),
    p2_FGES                     = INST7(0,0,1,1,0,1,0),
    p2_FLES                     = INST7(0,0,1,1,0,1,1),
    p2_SUMC                     = INST7(0,0,1,1,1,0,0),
    p2_SUMNC                    = INST7(0,0,1,1,1,0,1),
    p2_SUMZ                     = INST7(0,0,1,1,1,1,0),
    p2_SUMNZ                    = INST7(0,0,1,1,1,1,1),

    p2_TESTB_W_BITL             = INST7(0,1,0,0,0,0,0),
        p2_TESTB_W              = p2_TESTB_W_BITL,
        p2_BITL                 = p2_TESTB_W_BITL,
    p2_TESTBN_W_BITH            = INST7(0,1,0,0,0,0,1),
        p2_TESTBN_W             = p2_TESTBN_W_BITH,
        p2_BITH                 = p2_TESTBN_W_BITH,
    p2_TESTB_AND_BITC           = INST7(0,1,0,0,0,1,0),
        p2_TESTB_AND            = p2_TESTB_AND_BITC,
        p2_BITC                 = p2_TESTB_AND_BITC,
    p2_TESTBN_AND_BITNC         = INST7(0,1,0,0,0,1,1),
        p2_TESTBN_AND           = p2_TESTBN_AND_BITNC,
        p2_BITNC                = p2_TESTBN_AND_BITNC,
    p2_TESTB_OR_BITZ            = INST7(0,1,0,0,1,0,0),
        p2_TESTB_OR             = p2_TESTB_OR_BITZ,
        p2_BITZ                 = p2_TESTB_OR_BITZ,
    p2_TESTBN_OR_BITNZ          = INST7(0,1,0,0,1,0,1),
        p2_TESTBN_OR            = p2_TESTBN_OR_BITNZ,
        p2_BITNZ                = p2_TESTBN_OR_BITNZ,
    p2_TESTB_XOR_BITRND         = INST7(0,1,0,0,1,1,0),
        p2_TESTB_XOR            = p2_TESTB_XOR_BITRND,
        p2_BITRND               = p2_TESTB_XOR_BITRND,
    p2_TESTBN_XOR_BITNOT        = INST7(0,1,0,0,1,1,1),
        p2_TESTBN_XOR           = p2_TESTBN_XOR_BITNOT,
        p2_BITNOT               = p2_TESTBN_XOR_BITNOT,

    p2_AND                      = INST7(0,1,0,1,0,0,0),
    p2_ANDN                     = INST7(0,1,0,1,0,0,1),
    p2_OR                       = INST7(0,1,0,1,0,1,0),
    p2_XOR                      = INST7(0,1,0,1,0,1,1),
    p2_MUXC                     = INST7(0,1,0,1,1,0,0),
    p2_MUXNC                    = INST7(0,1,0,1,1,0,1),
    p2_MUXZ                     = INST7(0,1,0,1,1,1,0),
    p2_MUXNZ                    = INST7(0,1,0,1,1,1,1),

    p2_MOV                      = INST7(0,1,1,0,0,0,0),
    p2_NOT                      = INST7(0,1,1,0,0,0,1),
    p2_ABS                      = INST7(0,1,1,0,0,1,0),
    p2_NEG                      = INST7(0,1,1,0,0,1,1),
    p2_NEGC                     = INST7(0,1,1,0,1,0,0),
    p2_NEGNC                    = INST7(0,1,1,0,1,0,1),
    p2_NEGZ                     = INST7(0,1,1,0,1,1,0),
    p2_NEGNZ                    = INST7(0,1,1,0,1,1,1),

    p2_INCMOD                   = INST7(0,1,1,1,0,0,0),
    p2_DECMOD                   = INST7(0,1,1,1,0,0,1),
    p2_ZEROX                    = INST7(0,1,1,1,0,1,0),
    p2_SIGNX                    = INST7(0,1,1,1,0,1,1),
    p2_ENCOD                    = INST7(0,1,1,1,1,0,0),
    p2_ONES                     = INST7(0,1,1,1,1,0,1),
    p2_TEST                     = INST7(0,1,1,1,1,1,0),
    p2_TESTN                    = INST7(0,1,1,1,1,1,1),

    p2_SETNIB_0                 = INST7(1,0,0,0,0,0,0),
    p2_SETNIB_1                 = INST7(1,0,0,0,0,0,1),
    p2_GETNIB_0                 = INST7(1,0,0,0,0,1,0),
    p2_GETNIB_1                 = INST7(1,0,0,0,0,1,1),
    p2_ROLNIB_0                 = INST7(1,0,0,0,1,0,0),
    p2_ROLNIB_1                 = INST7(1,0,0,0,1,0,1),
    p2_SETBYTE                  = INST7(1,0,0,0,1,1,0),
    p2_GETBYTE                  = INST7(1,0,0,0,1,1,1),

    p2_ROLBYTE                  = INST7(1,0,0,1,0,0,0),
    p2_SETWORD_GETWORD          = INST7(1,0,0,1,0,0,1),
    p2_ROLWORD_ALTSN_ALTGN      = INST7(1,0,0,1,0,1,0),
    p2_ALTSB_ALTGB_ALTSW_ALTGW  = INST7(1,0,0,1,0,1,1),
    p2_ALTR_ALTD_ALTS_ALTB      = INST7(1,0,0,1,1,0,0),
    p2_ALTI_SETR_SETD_SETS      = INST7(1,0,0,1,1,0,1),
    p2_DECOD_BMASK_CRCBIT_CRCNIB= INST7(1,0,0,1,1,1,0),
    p2_MUX_NITS_NIBS_Q_MOVBYTS  = INST7(1,0,0,1,1,1,1),

    p2_MUL_MULS                 = INST7(1,0,1,0,0,0,0),
    p2_SCA_SCAS                 = INST7(1,0,1,0,0,0,1),
    p2_XXXPIX                   = INST7(1,0,1,0,0,1,0),
    p2_WMLONG_ADDCTx            = INST7(1,0,1,0,0,1,1),
    p2_RQPIN_RDPIN              = INST7(1,0,1,0,1,0,0),
    p2_RDLUT                    = INST7(1,0,1,0,1,0,1),
    p2_RDBYTE                   = INST7(1,0,1,0,1,1,0),
    p2_RDWORD                   = INST7(1,0,1,0,1,1,1),

    p2_RDLONG                   = INST7(1,0,1,1,0,0,0),
    p2_CALLD                    = INST7(1,0,1,1,0,0,1),
    p2_CALLPA_CALLPB            = INST7(1,0,1,1,0,1,0),
    p2_DJZ_DJNZ_DJF_DJNF        = INST7(1,0,1,1,0,1,1),
    p2_IJZ_IJNZ_TJZ_TJNZ        = INST7(1,0,1,1,1,0,0),
    p2_TJF_TJNF_TJS_TJNS        = INST7(1,0,1,1,1,0,1),
    p2_TJV_OPDST                = INST7(1,0,1,1,1,1,0),
    p2_empty_SETPAT             = INST7(1,0,1,1,1,1,1),

    p2_WRPIN_AKPIN_WXPIN        = INST7(1,1,0,0,0,0,0),
    p2_WYPIN_WRLUT              = INST7(1,1,0,0,0,0,1),
    p2_WRBYTE_WRWORD            = INST7(1,1,0,0,0,1,0),
    p2_WRLONG_RDFAST            = INST7(1,1,0,0,0,1,1),
    p2_WRFAST_FBLOCK            = INST7(1,1,0,0,1,0,0),
    p2_XINIT_XSTOP_XZERO        = INST7(1,1,0,0,1,0,1),
    p2_XCONT_REP                = INST7(1,1,0,0,1,1,0),
    p2_COGINIT                  = INST7(1,1,0,0,1,1,1),

    p2_QMUL_QDIV                = INST7(1,1,0,1,0,0,0),
    p2_QFRAC_QSQRT              = INST7(1,1,0,1,0,0,1),
    p2_QROTATE_QVECTOR          = INST7(1,1,0,1,0,1,0),
    p2_OPSRC                    = INST7(1,1,0,1,0,1,1),

    p2_JMP_ABS                  = INST7(1,1,0,1,1,0,0),
    p2_CALL_ABS                 = INST7(1,1,0,1,1,0,1),
    p2_CALLA_ABS                = INST7(1,1,0,1,1,1,0),
    p2_CALLB_ABS                = INST7(1,1,0,1,1,1,1),

    p2_CALLD_PA_ABS             = INST7(1,1,1,0,0,0,0),
    p2_CALLD_PB_ABS             = INST7(1,1,1,0,0,0,1),
    p2_CALLD_PTRA_ABS           = INST7(1,1,1,0,0,1,0),
    p2_CALLD_PTRB_ABS           = INST7(1,1,1,0,0,1,1),

    p2_LOC_PA                   = INST7(1,1,1,0,1,0,0),
    p2_LOC_PB                   = INST7(1,1,1,0,1,0,1),
    p2_LOC_PTRA                 = INST7(1,1,1,0,1,1,0),
    p2_LOC_PTRB                 = INST7(1,1,1,0,1,1,1),

    p2_AUGS_00                  = INST7(1,1,1,1,0,0,0),
    p2_AUGS_01                  = INST7(1,1,1,1,0,0,1),
    p2_AUGS_10                  = INST7(1,1,1,1,0,1,0),
    p2_AUGS_11                  = INST7(1,1,1,1,0,1,1),

    p2_AUGD_00                  = INST7(1,1,1,1,1,0,0),
    p2_AUGD_01                  = INST7(1,1,1,1,1,0,1),
    p2_AUGD_10                  = INST7(1,1,1,1,1,1,0),
    p2_AUGD_11                  = INST7(1,1,1,1,1,1,1),

}   p2_inst7_e;


typedef enum {

    p2_OPDST_JINT               = 0x00,
    p2_OPDST_JCT1               = 0x01,
    p2_OPDST_JCT2               = 0x02,
    p2_OPDST_JCT3               = 0x03,
    p2_OPDST_JSE1               = 0x04,
    p2_OPDST_JSE2               = 0x05,
    p2_OPDST_JSE3               = 0x06,
    p2_OPDST_JSE4               = 0x07,
    p2_OPDST_JPAT               = 0x08,
    p2_OPDST_JFBW               = 0x09,
    p2_OPDST_JXMT               = 0x0a,
    p2_OPDST_JXFI               = 0x0b,
    p2_OPDST_JXRO               = 0x0c,
    p2_OPDST_JXRL               = 0x0d,
    p2_OPDST_JATN               = 0x0e,
    p2_OPDST_JQMT               = 0x0f,

    p2_OPDST_JNINT              = 0x10,
    p2_OPDST_JNCT1              = 0x11,
    p2_OPDST_JNCT2              = 0x12,
    p2_OPDST_JNCT3              = 0x13,
    p2_OPDST_JNSE1              = 0x14,
    p2_OPDST_JNSE2              = 0x15,
    p2_OPDST_JNSE3              = 0x16,
    p2_OPDST_JNSE4              = 0x17,
    p2_OPDST_JNPAT              = 0x18,
    p2_OPDST_JNFBW              = 0x19,
    p2_OPDST_JNXMT              = 0x1a,
    p2_OPDST_JNXFI              = 0x1b,
    p2_OPDST_JNXRO              = 0x1c,
    p2_OPDST_JNXRL              = 0x1d,
    p2_OPDST_JNATN              = 0x1e,
    p2_OPDST_JNQMT              = 0x1f,
}   p2_opdst_e;

typedef enum {
    p2_OPSRC_HUBSET             = 0x00,
    p2_OPSRC_COGID              = 0x01,
    p2_OPSRC_COGSTOP            = 0x03,
    p2_OPSRC_LOCKNEW            = 0x04,
    p2_OPSRC_LOCKRET            = 0x05,
    p2_OPSRC_LOCKTRY            = 0x06,
    p2_OPSRC_LOCKREL            = 0x07,
    p2_OPSRC_QLOG               = 0x0e,
    p2_OPSRC_QEXP               = 0x0f,

    p2_OPSRC_RFBYTE             = 0x10,
    p2_OPSRC_RFWORD             = 0x11,
    p2_OPSRC_RFLONG             = 0x12,
    p2_OPSRC_RFVAR              = 0x13,
    p2_OPSRC_RFVARS             = 0x14,
    p2_OPSRC_WFBYTE             = 0x15,
    p2_OPSRC_WFWORD             = 0x16,
    p2_OPSRC_WFLONG             = 0x17,

    p2_OPSRC_GETQX              = 0x18,
    p2_OPSRC_GETQY              = 0x19,
    p2_OPSRC_GETCT              = 0x1a,
    p2_OPSRC_GETRND             = 0x1b,
    p2_OPSRC_SETDACS            = 0x1c,
    p2_OPSRC_SETXFRQ            = 0x1d,
    p2_OPSRC_GETXACC            = 0x1e,
    p2_OPSRC_WAITX              = 0x1f,

    p2_OPSRC_SETSE1             = 0x20,
    p2_OPSRC_SETSE2             = 0x21,
    p2_OPSRC_SETSE3             = 0x22,
    p2_OPSRC_SETSE4             = 0x23,
    p2_OPSRC_X24                = 0x24,
    p2_OPSRC_SETINT1            = 0x25,
    p2_OPSRC_SETINT2            = 0x26,
    p2_OPSRC_SETINT3            = 0x27,

    p2_OPSRC_SETQ               = 0x28,
    p2_OPSRC_SETQ2              = 0x29,
    p2_OPSRC_PUSH               = 0x2a,
    p2_OPSRC_POP                = 0x2b,
    p2_OPSRC_JMP                = 0x2c,
    p2_OPSRC_CALL_RET           = 0x2d,
    p2_OPSRC_CALLA_RETA         = 0x2e,
    p2_OPSRC_CALLB_RETB         = 0x2f,

    p2_OPSRC_JMPREL             = 0x30,
    p2_OPSRC_SKIP               = 0x31,
    p2_OPSRC_SKIPF              = 0x32,
    p2_OPSRC_EXECF              = 0x33,
    p2_OPSRC_GETPTR             = 0x34,
    p2_OPSRC_COGBRK             = 0x35,
    p2_OPSRC_BRK                = 0x36,
    p2_OPSRC_SETLUTS            = 0x37,

    p2_OPSRC_SETCY              = 0x38,
    p2_OPSRC_SETCI              = 0x39,
    p2_OPSRC_SETCQ              = 0x3a,
    p2_OPSRC_SETCFRQ            = 0x3b,
    p2_OPSRC_SETCMOD            = 0x3c,
    p2_OPSRC_SETPIV             = 0x3d,
    p2_OPSRC_SETPIX             = 0x3e,
    p2_OPSRC_COGATN             = 0x3f,

    p2_OPSRC_TESTP_W_DIRL       = 0x40,
        p2_OPSRC_TESTP_W        = p2_OPSRC_TESTP_W_DIRL,
        p2_OPSRC_DIRL           = p2_OPSRC_TESTP_W_DIRL,
    p2_OPSRC_TESTPN_W_DIRH      = 0x41,
        p2_OPSRC_TESTPN_W       = p2_OPSRC_TESTPN_W_DIRH,
        p2_OPSRC_DIRH           = p2_OPSRC_TESTPN_W_DIRH,
    p2_OPSRC_TESTP_AND_DIRC     = 0x42,
        p2_OPSRC_TESTP_AND      = p2_OPSRC_TESTP_AND_DIRC,
        p2_OPSRC_DIRC           = p2_OPSRC_TESTP_AND_DIRC,
    p2_OPSRC_TESTPN_AND_DIRNC   = 0x43,
        p2_OPSRC_TESTPN_AND     = p2_OPSRC_TESTPN_AND_DIRNC,
        p2_OPSRC_DIRNC          = p2_OPSRC_TESTPN_AND_DIRNC,
    p2_OPSRC_TESTP_OR_DIRZ      = 0x44,
        p2_OPSRC_TESTP_OR       = p2_OPSRC_TESTP_OR_DIRZ,
        p2_OPSRC_DIRZ           = p2_OPSRC_TESTP_OR_DIRZ,
    p2_OPSRC_TESTPN_OR_DIRNZ    = 0x45,
        p2_OPSRC_TESTPN_OR      = p2_OPSRC_TESTPN_OR_DIRNZ,
        p2_OPSRC_DIRNZ          = p2_OPSRC_TESTPN_OR_DIRNZ,
    p2_OPSRC_TESTP_XOR_DIRRND   = 0x46,
        p2_OPSRC_TESTP_XOR      = p2_OPSRC_TESTP_XOR_DIRRND,
        p2_OPSRC_DIRRND         = p2_OPSRC_TESTP_XOR_DIRRND,
    p2_OPSRC_TESTPN_XOR_DIRNOT  = 0x47,
        p2_OPSRC_TESTPN_XOR     = p2_OPSRC_TESTPN_XOR_DIRNOT,
        p2_OPSRC_DIRNOT         = p2_OPSRC_TESTPN_XOR_DIRNOT,

    p2_OPSRC_OUTL               = 0x48,
    p2_OPSRC_OUTH               = 0x49,
    p2_OPSRC_OUTC               = 0x4a,
    p2_OPSRC_OUTNC              = 0x4b,
    p2_OPSRC_OUTZ               = 0x4c,
    p2_OPSRC_OUTNZ              = 0x4d,
    p2_OPSRC_OUTRND             = 0x4e,
    p2_OPSRC_OUTNOT             = 0x4f,

    p2_OPSRC_FLTL               = 0x50,
    p2_OPSRC_FLTH               = 0x51,
    p2_OPSRC_FLTC               = 0x52,
    p2_OPSRC_FLTNC              = 0x53,
    p2_OPSRC_FLTZ               = 0x54,
    p2_OPSRC_FLTNZ              = 0x55,
    p2_OPSRC_FLTRND             = 0x56,
    p2_OPSRC_FLTNOT             = 0x57,

    p2_OPSRC_DRVL               = 0x58,
    p2_OPSRC_DRVH               = 0x59,
    p2_OPSRC_DRVC               = 0x5a,
    p2_OPSRC_DRVNC              = 0x5b,
    p2_OPSRC_DRVZ               = 0x5c,
    p2_OPSRC_DRVNZ              = 0x5d,
    p2_OPSRC_DRVRND             = 0x5e,
    p2_OPSRC_DRVNOT             = 0x5f,

    p2_OPSRC_SPLITB             = 0x60,
    p2_OPSRC_MERGEB             = 0x61,
    p2_OPSRC_SPLITW             = 0x62,
    p2_OPSRC_MERGEW             = 0x63,
    p2_OPSRC_SEUSSF             = 0x64,
    p2_OPSRC_SEUSSR             = 0x65,
    p2_OPSRC_RGBSQZ             = 0x66,
    p2_OPSRC_RGBEXP             = 0x67,

    p2_OPSRC_XORO32             = 0x68,
    p2_OPSRC_REV                = 0x69,
    p2_OPSRC_RCZR               = 0x6a,
    p2_OPSRC_RCZL               = 0x6b,
    p2_OPSRC_WRC                = 0x6c,
    p2_OPSRC_WRNC               = 0x6d,
    p2_OPSRC_WRZ                = 0x6e,
    p2_OPSRC_WRNZ_MODCZ         = 0x6f,

    p2_OPSRC_SETSCP             = 0x70,
    p2_OPSRC_GETSCP             = 0x71,
}   p2_opsrc_e;

typedef enum {
    p2_OPX24_POLLINT            = 0x00,
    p2_OPX24_POLLCT1            = 0x01,
    p2_OPX24_POLLCT2            = 0x02,
    p2_OPX24_POLLCT3            = 0x03,
    p2_OPX24_POLLSE1            = 0x04,
    p2_OPX24_POLLSE2            = 0x05,
    p2_OPX24_POLLSE3            = 0x06,
    p2_OPX24_POLLSE4            = 0x07,
    p2_OPX24_POLLPAT            = 0x08,
    p2_OPX24_POLLFBW            = 0x09,
    p2_OPX24_POLLXMT            = 0x0a,
    p2_OPX24_POLLXFI            = 0x0b,
    p2_OPX24_POLLXRO            = 0x0c,
    p2_OPX24_POLLXRL            = 0x0d,
    p2_OPX24_POLLATN            = 0x0e,
    p2_OPX24_POLLQMT            = 0x0f,

    p2_OPX24_WAITINT            = 0x10,
    p2_OPX24_WAITCT1            = 0x11,
    p2_OPX24_WAITCT2            = 0x12,
    p2_OPX24_WAITCT3            = 0x13,
    p2_OPX24_WAITSE1            = 0x14,
    p2_OPX24_WAITSE2            = 0x15,
    p2_OPX24_WAITSE3            = 0x16,
    p2_OPX24_WAITSE4            = 0x17,
    p2_OPX24_WAITPAT            = 0x18,
    p2_OPX24_WAITFBW            = 0x19,
    p2_OPX24_WAITXMT            = 0x1a,
    p2_OPX24_WAITXFI            = 0x1b,
    p2_OPX24_WAITXRO            = 0x1c,
    p2_OPX24_WAITXRL            = 0x1d,
    p2_OPX24_WAITATN            = 0x1e,

    p2_OPX24_ALLOWI             = 0x20,
    p2_OPX24_STALLI             = 0x21,
    p2_OPX24_TRGINT1            = 0x22,
    p2_OPX24_TRGINT2            = 0x23,
    p2_OPX24_TRGINT3            = 0x24,
    p2_OPX24_NIXINT1            = 0x25,
    p2_OPX24_NIXINT2            = 0x26,
    p2_OPX24_NIXINT3            = 0x27,
}   p2_opx24_e;

typedef enum {
    p2_OPCODE_WMLONG            = 0x53,
    p2_OPCODE_RDBYTE            = 0x56,
    p2_OPCODE_RDWORD            = 0x57,
    p2_OPCODE_RDLONG            = 0x58,
    p2_OPCODE_CALLD             = 0x59,
    p2_OPCODE_CALLPB            = 0x5e,
    p2_OPCODE_JINT              = 0x5f,
    p2_OPCODE_WRBYTE            = 0x62,
    p2_OPCODE_WRWORD            = 0x62,
    p2_OPCODE_WRLONG            = 0x63,
    p2_OPCODE_QMUL              = 0x68,
    p2_OPCODE_QVECTOR           = 0x6a,
}   p2_opcode_e;

typedef enum {
    p2_INSTR_MASK1              = 0x0fe001ff,
    p2_INSTR_LOCKNEW            = 0x0d600004,
    p2_INSTR_LOCKSET            = 0x0d600007,
    p2_INSTR_QLOG               = 0x0d60000e,
    p2_INSTR_QEXP               = 0x0d60000f,
    p2_INSTR_RFBYTE             = 0x0d600010,
    p2_INSTR_RFWORD             = 0x0d600011,
    p2_INSTR_RFLONG             = 0x0d600012,
    p2_INSTR_WFBYTE             = 0x0d600015,
    p2_INSTR_WFWORD             = 0x0d600016,
    p2_INSTR_WFLONG             = 0x0d600017,
    p2_INSTR_GETQX              = 0x0d600018,
    p2_INSTR_GETQY              = 0x0d600019,
    p2_INSTR_WAITX              = 0x0d60001f,
}   p2_instx1_e;

typedef enum {
    p2_INSTR_MASK2              = 0x0fe3e1ff,
    p2_INSTR_WAITXXX            = 0x0d602024,
}   p2_instx2_e;

/**
 * @brief Enumeration of the 8 bit instruction types including WC
 */
typedef enum {
    p2_MUL                      = INST8(p2_MUL_MULS,0),
    p2_MULS                     = INST8(p2_MUL_MULS,1),

    p2_SCA                      = INST8(p2_SCA_SCAS,0),
    p2_SCAS                     = INST8(p2_SCA_SCAS,1),

    p2_CALLPA                   = INST8(p2_CALLPA_CALLPB,0),
    p2_CALLPB                   = INST8(p2_CALLPA_CALLPB,1),

    p2_QMUL                     = INST8(p2_QMUL_QDIV,0),
    p2_QDIV                     = INST8(p2_QMUL_QDIV,1),

    p2_QFRAC                    = INST8(p2_QFRAC_QSQRT,0),
    p2_QSQRT                    = INST8(p2_QFRAC_QSQRT,1),

    p2_QROTATE                  = INST8(p2_QROTATE_QVECTOR,0),
    p2_QVECTOR                  = INST8(p2_QROTATE_QVECTOR,1),

    p2_1011111_0                = INST8(p2_empty_SETPAT,0),
    p2_SETPAT                   = INST8(p2_empty_SETPAT,1),

    p2_RQPIN                    = INST8(p2_RQPIN_RDPIN,0),
    p2_RDPIN                    = INST8(p2_RQPIN_RDPIN,1),

    p2_WRPIN                    = INST8(p2_WRPIN_AKPIN_WXPIN, 0),
    p2_WXPIN                    = INST8(p2_WRPIN_AKPIN_WXPIN, 1),

    p2_WYPIN                    = INST8(p2_WYPIN_WRLUT, 0),
    p2_WRLUT                    = INST8(p2_WYPIN_WRLUT, 1),

    p2_WRBYTE                   = INST8(p2_WRBYTE_WRWORD, 0),
    p2_WRWORD                   = INST8(p2_WRBYTE_WRWORD, 1),

    p2_WRLONG                   = INST8(p2_WRLONG_RDFAST, 0),
    p2_RDFAST                   = INST8(p2_WRLONG_RDFAST, 1),

    p2_WRFAST                   = INST8(p2_WRFAST_FBLOCK, 0),
    p2_FBLOCK                   = INST8(p2_WRFAST_FBLOCK, 1),

    p2_XINIT                    = INST8(p2_XINIT_XSTOP_XZERO, 0),
    p2_XZERO                    = INST8(p2_XINIT_XSTOP_XZERO, 1),

    p2_XCONT                    = INST8(p2_XCONT_REP, 0),
    p2_REP                      = INST8(p2_XCONT_REP, 1),

}   p2_inst8_e;

/**
 * @brief Enumeration of the 9 bit instruction types including WC and WZ
 */
typedef enum {
    p2_BITL_eol                 = INST9(p2_TESTB_W_BITL,0,0),
    p2_TESTB_WZ                 = INST9(p2_TESTB_W_BITL,0,1),
    p2_TESTB_WC                 = INST9(p2_TESTB_W_BITL,1,0),
    p2_BITL_WCZ                 = INST9(p2_TESTB_W_BITL,1,1),

    p2_BITH_eol                 = INST9(p2_TESTBN_W_BITH,0,0),
    p2_TESTBN_WZ                = INST9(p2_TESTBN_W_BITH,0,1),
    p2_TESTBN_WC                = INST9(p2_TESTBN_W_BITH,1,0),
    p2_BITH_WCZ                 = INST9(p2_TESTBN_W_BITH,1,1),

    p2_BITC_eol                 = INST9(p2_TESTB_AND_BITC,0,0),
    p2_TESTB_ANDZ               = INST9(p2_TESTB_AND_BITC,0,1),
    p2_TESTB_ANDC               = INST9(p2_TESTB_AND_BITC,1,0),
    p2_BITC_WCZ                 = INST9(p2_TESTB_AND_BITC,1,1),

    p2_BITNC_eol                = INST9(p2_TESTBN_AND_BITNC,0,0),
    p2_TESTBN_ANDZ              = INST9(p2_TESTBN_AND_BITNC,0,1),
    p2_TESTBN_ANDC              = INST9(p2_TESTBN_AND_BITNC,1,0),
    p2_BITNC_WCZ                = INST9(p2_TESTBN_AND_BITNC,1,1),

    p2_BITZ_eol                 = INST9(p2_TESTB_OR_BITZ,0,0),
    p2_TESTB_ORZ                = INST9(p2_TESTB_OR_BITZ,0,1),
    p2_TESTB_ORC                = INST9(p2_TESTB_OR_BITZ,1,0),
    p2_BITZ_WCZ                 = INST9(p2_TESTB_OR_BITZ,1,1),

    p2_BITNZ_eol                = INST9(p2_TESTBN_OR_BITNZ,0,0),
    p2_TESTBN_ORZ               = INST9(p2_TESTBN_OR_BITNZ,0,1),
    p2_TESTBN_ORC               = INST9(p2_TESTBN_OR_BITNZ,1,0),
    p2_BITNZ_WCZ                = INST9(p2_TESTBN_OR_BITNZ,1,1),

    p2_BITRND_eol               = INST9(p2_TESTB_XOR_BITRND,0,0),
    p2_TESTB_XORZ               = INST9(p2_TESTB_XOR_BITRND,0,1),
    p2_TESTB_XORC               = INST9(p2_TESTB_XOR_BITRND,1,0),
    p2_BITRND_WCZ               = INST9(p2_TESTB_XOR_BITRND,1,1),

    p2_BITNOT_eol               = INST9(p2_TESTBN_XOR_BITNOT,0,0),
    p2_TESTBN_XORZ              = INST9(p2_TESTBN_XOR_BITNOT,0,1),
    p2_TESTBN_XORC              = INST9(p2_TESTBN_XOR_BITNOT,1,0),
    p2_BITNOT_WCZ               = INST9(p2_TESTBN_XOR_BITNOT,1,1),

    p2_SETWORD_ALTSW            = INST9(p2_SETWORD_GETWORD,0,0),
    p2_SETWORD                  = INST9(p2_SETWORD_GETWORD,0,1),
    p2_GETWORD_ALTGW            = INST9(p2_SETWORD_GETWORD,1,0),
    p2_GETWORD                  = INST9(p2_SETWORD_GETWORD,1,1),

    p2_ROLWORD_ALTGW            = INST9(p2_ROLWORD_ALTSN_ALTGN,0,0),
    p2_ROLWORD                  = INST9(p2_ROLWORD_ALTSN_ALTGN,0,1),
    p2_ALTSN                    = INST9(p2_ROLWORD_ALTSN_ALTGN,1,0),
    p2_ALTGN                    = INST9(p2_ROLWORD_ALTSN_ALTGN,1,1),

    p2_ALTSB                    = INST9(p2_ALTSB_ALTGB_ALTSW_ALTGW,0,0),
    p2_ALTGB                    = INST9(p2_ALTSB_ALTGB_ALTSW_ALTGW,0,1),
    p2_ALTSW                    = INST9(p2_ALTSB_ALTGB_ALTSW_ALTGW,1,0),
    p2_ALTGW                    = INST9(p2_ALTSB_ALTGB_ALTSW_ALTGW,1,1),

    p2_ALTR                     = INST9(p2_ALTR_ALTD_ALTS_ALTB,0,0),
    p2_ALTD                     = INST9(p2_ALTR_ALTD_ALTS_ALTB,0,1),
    p2_ALTS                     = INST9(p2_ALTR_ALTD_ALTS_ALTB,1,0),
    p2_ALTB                     = INST9(p2_ALTR_ALTD_ALTS_ALTB,1,1),

    p2_ALTI                     = INST9(p2_ALTI_SETR_SETD_SETS,0,0),
    p2_SETR                     = INST9(p2_ALTI_SETR_SETD_SETS,0,1),
    p2_SETD                     = INST9(p2_ALTI_SETR_SETD_SETS,1,0),
    p2_SETS                     = INST9(p2_ALTI_SETR_SETD_SETS,1,1),

    p2_DECOD                    = INST9(p2_DECOD_BMASK_CRCBIT_CRCNIB,0,0),
    p2_BMASK                    = INST9(p2_DECOD_BMASK_CRCBIT_CRCNIB,0,1),
    p2_CRCBIT                   = INST9(p2_DECOD_BMASK_CRCBIT_CRCNIB,1,0),
    p2_CRCNIB                   = INST9(p2_DECOD_BMASK_CRCBIT_CRCNIB,1,1),

    p2_MUXNITS                  = INST9(p2_MUX_NITS_NIBS_Q_MOVBYTS,0,0),
    p2_MUXNIBS                  = INST9(p2_MUX_NITS_NIBS_Q_MOVBYTS,0,1),
    p2_MUXQ                     = INST9(p2_MUX_NITS_NIBS_Q_MOVBYTS,1,0),
    p2_MOVBYTS                  = INST9(p2_MUX_NITS_NIBS_Q_MOVBYTS,1,1),

    p2_ADDCT1                   = INST9(p2_WMLONG_ADDCTx,0,0),
    p2_ADDCT2                   = INST9(p2_WMLONG_ADDCTx,0,1),
    p2_ADDCT3                   = INST9(p2_WMLONG_ADDCTx,1,0),
    p2_WMLONG                   = INST9(p2_WMLONG_ADDCTx,1,1),

    p2_ADDPIX                   = INST9(p2_XXXPIX,0,0),
    p2_MULPIX                   = INST9(p2_XXXPIX,0,1),
    p2_BLNPIX                   = INST9(p2_XXXPIX,1,0),
    p2_MIXPIX                   = INST9(p2_XXXPIX,1,1),

    p2_DJZ                      = INST9(p2_DJZ_DJNZ_DJF_DJNF,0,0),
    p2_DJNZ                     = INST9(p2_DJZ_DJNZ_DJF_DJNF,0,1),
    p2_DJF                      = INST9(p2_DJZ_DJNZ_DJF_DJNF,1,0),
    p2_DJNF                     = INST9(p2_DJZ_DJNZ_DJF_DJNF,1,1),

    p2_IJZ                      = INST9(p2_IJZ_IJNZ_TJZ_TJNZ,0,0),
    p2_IJNZ                     = INST9(p2_IJZ_IJNZ_TJZ_TJNZ,0,1),
    p2_TJZ                      = INST9(p2_IJZ_IJNZ_TJZ_TJNZ,1,0),
    p2_TJNZ                     = INST9(p2_IJZ_IJNZ_TJZ_TJNZ,1,1),

    p2_TJF                      = INST9(p2_TJF_TJNF_TJS_TJNS,0,0),
    p2_TJNF                     = INST9(p2_TJF_TJNF_TJS_TJNS,0,1),
    p2_TJS                      = INST9(p2_TJF_TJNF_TJS_TJNS,1,0),
    p2_TJNS                     = INST9(p2_TJF_TJNF_TJS_TJNS,1,1),

    p2_TJV                      = INST9(p2_TJV_OPDST,0,0),
    p2_OPDST                    = INST9(p2_TJV_OPDST,0,1),
    p2_1011110_10               = INST9(p2_TJV_OPDST,1,0),
    p2_1011110_11               = INST9(p2_TJV_OPDST,1,1),

    p2_COGBRK                   = INST9(p2_OPSRC_COGBRK,0,0),
    p2_GETBRK_WZ                = INST9(p2_OPSRC_COGBRK,0,1),
    p2_GETBRK_WC                = INST9(p2_OPSRC_COGBRK,1,0),
    p2_GETBRK_WCZ               = INST9(p2_OPSRC_COGBRK,1,1),

}   p2_inst9_e;

Q_STATIC_ASSERT(p2_AUGD_11 == 127u);

/**
 * @brief Structure of the Propeller2 opcode words with 7 bits instruction, wc, wz, and imm
 *
 * Note: To have the bit fields work for both, little
 * and big endian machines, the order has to match
 * the native order of bits in 32 bit words of the target.
 */
typedef struct {
#if (Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
    unsigned src:9;             //!< source (S or #S)
    unsigned dst:9;             //!< destination (D or #D)
    bool im:1;                  //!< immediate flag
    bool wz:1;                  //!< update Z flag
    bool wc:1;                  //!< update C flag
    unsigned inst:7;           //!< instruction type
    unsigned cond:4;            //!< conditional execution
#elif (Q_BYTE_ORDER == Q_BIG_ENDIAN)
    unsigned cond:4;            //!< conditional execution
    unsigned inst:7;            //!< instruction type
    bool uc:1;                  //!< update C flag
    bool uz:1;                  //!< update Z flag
    bool im:1;                  //!< immediate flag
    unsigned dst:9;             //!< destination (D or #D)
    unsigned src:9;             //!< source (S or #S)
#else
#error "Unknown byte order!"
#endif
}   p2_opcode7_t;

/**
 * @brief Structure of the Propeller2 opcode words with 8 bits of instruction, wz, and imm
 */
typedef struct {
#if (Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
    unsigned src:9;             //!< source (S or #S)
    unsigned dst:9;             //!< destination (D or #D)
    bool im:1;                  //!< immediate flag
    bool wz:1;                  //!< update Z flag
    unsigned inst:8;            //!< instruction type including WC
    unsigned cond:4;            //!< conditional execution
#elif (Q_BYTE_ORDER == Q_BIG_ENDIAN)
    unsigned cond:4;            //!< conditional execution
    unsigned inst:8;            //!< instruction type including WC
    bool wz:1;                  //!< update Z flag
    bool im:1;                  //!< immediate flag
    unsigned dst:9;             //!< destination (D or #D)
    unsigned src:9;             //!< source (S or #S)
#else
#error "Unknown byte order!"
#endif
}   p2_opcode8_t;

/**
 * @brief Structure of the Propeller2 opcode words with 9 bits of instruction and imm
 */
typedef struct {
#if (Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
    unsigned src:9;             //!< source (S or #S)
    unsigned dst:9;             //!< destination (D or #D)
    bool im:1;                  //!< immediate flag
    unsigned inst:9;            //!< instruction type including WC and WZ
    unsigned cond:4;            //!< conditional execution
#elif (Q_BYTE_ORDER == Q_BIG_ENDIAN)
    unsigned cond:4;            //!< conditional execution
    unsigned inst:9;            //!< instruction type including WC and WZ
    bool im:1;                  //!< immediate flag
    unsigned dst:9;             //!< destination (D or #D)
    unsigned src:9;             //!< source (S or #S)
#else
#error "Unknown byte order!"
#endif
}   p2_opcode9_t;

/**
 * @brief union of a 32 bit word and the opcode bit fields
 */
typedef union {
    p2_LONG opcode;              //!< opcode as 32 bit word
    p2_opcode7_t op;            //!< ocpode as bit fields (version with 7 bits instruction)
    p2_opcode8_t op8;           //!< ocpode as bit fields (version including WC)
    p2_opcode9_t op9;           //!< ocpode as bit fields (version including WC and WZ)
}   p2_opcode_u;

/**
 * @brief structure of the shadow registers at the end of LUT
 */
typedef struct {
    p2_LONG RAM[512-16];         //!< general-use code/data registers
    p2_LONG IJMP3;               //!< interrupt call address for INT3
    p2_LONG IRET3;               //!< interrupt return address for INT3
    p2_LONG IJMP2;               //!< interrupt call address for INT2
    p2_LONG IRET2;               //!< interrupt return address for INT2
    p2_LONG IJMP1;               //!< interrupt call address for INT1
    p2_LONG IRET1;               //!< interrupt return address for INT1
    p2_LONG PA;                  //!< CALLD-imm return, CALLPA parameter, or LOC address
    p2_LONG PB;                  //!< CALLD-imm return, CALLPB parameter, or LOC address
    p2_LONG PTRA;                //!< pointer A to hub RAM
    p2_LONG PTRB;                //!< pointer B to hub RAM
    p2_LONG DIRA;                //!< output enables for P31 ... P0
    p2_LONG DIRB;                //!< output enables for P63 ... P32
    p2_LONG OUTA;                //!< output states for P31 ... P0
    p2_LONG OUTB;                //!< output states for P63 ... P32
    p2_LONG INA;                 //!< input states for P31 ... P0
    p2_LONG INB;                 //!< input states for P63 ... P32
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
 * @brief Structure of the COG memory
 */
typedef union {
    p2_LONG RAM[512];
}   p2_cog_t;

/**
 * @brief Union of the LUT memory and shadow registers
 */
typedef union {
    p2_LONG RAM[512];
    p2_lutregs_t REG;
}   p2_lut_t;

/**
 * @brief flag bits per COG
 */
typedef struct {
    bool    f_INT:1;            //!< INT interrupt flag
    bool    f_CT1:1;            //!< CT1 counter flag
    bool    f_CT2:1;            //!< CT2 counter flag
    bool    f_CT3:1;            //!< CT3 counter flag
    bool    f_SE1:1;            //!< SE1 event flag
    bool    f_SE2:1;            //!< SE2 event flag
    bool    f_SE3:1;            //!< SE3 event flag
    bool    f_SE4:1;            //!< SE4 event flag
    bool    f_PAT:1;            //!< PAT pattern matching flag
    bool    f_FBW:1;            //!< FBW flag
    bool    f_XMT:1;            //!< XMT flag
    bool    f_XFI:1;            //!< XFI flag
    bool    f_XRO:1;            //!< XRO flag
    bool    f_XRL:1;            //!< XRL flag
    bool    f_ATN:1;            //!< ATN COG attention flag
    bool    f_QMT:1;            //!< QMT Q empty flag
}   p2_flags_t;

/**
 * @brief pattern matching mode enum
 */
typedef enum {
    p2_PAT_NONE,                //!< no pattern matching
    p2_PAT_PA_EQ,               //!< match if (PA & mask) == match
    p2_PAT_PA_NE,               //!< match if (PA & mask) != match
    p2_PAT_PB_EQ,               //!< match if (PB & mask) == match
    p2_PAT_PB_NE                //!< match if (PB & mask) != match
}   p2_pat_mode_e;

/**
 * @brief PAT pattern matching data
 */
typedef struct {
    p2_pat_mode_e mode;         //!< pattern matching mode
    p2_LONG mask;               //!< mask value
    p2_LONG match;              //!< match value
}   p2_pat_t;

/**
 * @brief pin edge mode enum
 */
typedef enum {
    p2_PIN_NONE,                //!< no pin edge
    p2_PIN_CHANGED_LO,          //!< match if pin changed to lo
    p2_PIN_CHANGED_HI,          //!< match if pin changed to hi
    p2_PIN_CHANGED              //!< match if pin changed
}   p2_pin_mode_e;

/**
 * @brief PIN pin edge data
 */
typedef struct {
    p2_pin_mode_e mode;         //!< pin edge mode
    p2_LONG edge;               //!< pin edge type
    p2_LONG num;                //!< pin number
}   p2_pin_t;

/**
 * @brief INT disable / active / source data
 */
typedef struct {
    bool disabled:1;            //!< interrupts disabled
    bool INT1_active:1;         //!< INT1 is active
    bool INT2_active:1;         //!< INT2 is active
    bool INT3_active:1;         //!< INT3 is active
    bool PAT_active:1;          //!< PAT pattern matching active
    bool PIN_active:1;          //!< PIN edged interrupt active
    bool RDL_active:1;          //!< RDLONG active
    bool WRL_active:1;          //!< WRLONG active
    bool LOCK_active:1;         //!< LOCK active
    uint INT1_source:2;         //!< INT1 enabled source
    uint INT2_source:2;         //!< INT2 enabled source
    uint INT3_source:2;         //!< INT3 enabled source
}   p2_int_flags_t;

/**
 * @brief union of INT flags and bits
 */
typedef union {
    p2_LONG bits;               //!< interrupt flags as LONG
    p2_int_flags_t flags;
}   p2_int_bits_u;

/**
 * @brief LOCK edge mode enum
 */
typedef enum {
    p2_LOCK_NONE,               //!< no LOCK edge
    p2_LOCK_CHANGED_LO,         //!< match if LOCK changed to lo
    p2_LOCK_CHANGED_HI,         //!< match if LOCK changed to hi
    p2_LOCK_CHANGED             //!< match if LOCK changed
}   p2_lock_mode_e;

/**
 * @brief LOCK edge data
 */
typedef struct {
    p2_LONG num:4;              //!< LOCK COG id
    p2_lock_mode_e mode:2;      //!< LOCK edge mode
    p2_LONG edge:2;             //!< LOCK edge
    p2_LONG prev:1;             //!< LOCK previous state
}   p2_lock_t;

/**
 * @brief wait reason enum
 */
typedef enum {
    p2_WAIT_NONE,               //!< not waiting on anything
    p2_WAIT_CNT,                //!< waiting for CNT to reach one of CT1, CT2, CT3
    p2_WAIT_CORDIC,             //!< waiting for CORDIC solver to finish
    p2_WAIT_PIN,                //!< waiting for PIN to change level
    p2_WAIT_HUB,                //!< waiting for HUB access
    p2_WAIT_CACHE,              //!< waiting on FIFO cache to be filled
    p2_WAIT_FLAG                //!< waiting for a specific FLAG bit
}   p2_wait_mode_e;

/**
 * @brief wait status
 */
typedef struct {
    p2_LONG flag;               //!< non-zero if waiting
    p2_wait_mode_e mode;        //!< current wait mode
}   p2_wait_t;

/**
 * @brief FIFO configuration and status
 */
typedef struct {
    p2_LONG buff[16];           //!< buffer of 16 longs read from / written to the HUB
    p2_LONG rindex;             //!< current read index
    p2_LONG windex;             //!< current write index
    p2_LONG head_addr;          //!< head address
    p2_LONG tail_addr;          //!< tail address
    p2_LONG addr0;              //!< 1st address
    p2_LONG addr1;              //!< 2nd address
    p2_LONG mode;               //!< FIFO mode
    p2_LONG word;               //!< FIFO word
    p2_LONG flag;               //!< FIFO flags
}   p2_fifo_t;

typedef struct {
    p2_opcode_u IR;
    p2_LONG R;
    p2_LONG D;
    p2_LONG S;
}   p2_queue_t;

typedef enum {
    f_bin,
    f_hex,
    f_oct
}   p2_opcode_format_e;
