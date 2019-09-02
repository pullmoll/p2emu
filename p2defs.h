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
#include <QChar>
#include <QColor>
#include <QHash>

#include "p2tokens.h"

//! Type of the Propeller2 BYTE
typedef quint8 p2_BYTE;

//! Type of the Propeller2 WORD
typedef quint16 p2_WORD;

//! Type of the Propeller2 LONG
typedef quint32 p2_LONG;

//! Intermediate type (unsigned long long); not a Propeller2 type
typedef quint64 p2_QUAD;

//! Intermediate type (unsigned long long); not a Propeller2 type
typedef qreal p2_REAL;

//! Type for an array (vector) of BYTEs
typedef QVector<p2_BYTE> p2_BYTES;

//! Type for an array (vector) of WORDs
typedef QVector<p2_WORD> p2_WORDS;

//! Type for an array (vector) of LONGs
typedef QVector<p2_LONG> p2_LONGS;

//! Type for a mask/match pair for P2DocOpcode and P2Doc
typedef QPair<p2_LONG,p2_LONG> P2MatchMask;

/// Union of bytes, words, and a long in endianess aware ordering
typedef union {
#if (Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
    p2_LONG l;                              //!< long
    p2_WORD w0, w1;                         //!< words 0, 1 as least significant word first
    p2_WORD w[2];                           //!< 2 words as an array
    p2_BYTE b0, b1, b2, b3;                 //!< bytes 0, 1, 2, 3 as least significant byte first
    p2_BYTE b[4];                           //!< 4 bytes as an array
#elif (Q_BYTE_ORDER == Q_BIG_ENDIAN)
    p2_LONG l;                              //!< long
    p2_WORD w1, w0;                         //!< words 1, 0 as most significant word first
    p2_WORD w[2];                           //!< 2 words as an array
    p2_BYTE b3, b2, b1, b0;                 //!< bytes 3, 2, 1, 0 as most significant byte first
    p2_BYTE b[4];                           //!< 4 bytes as an array
#else
#error "Unknown byte order!"
#endif
}   p2_BWL;


/// Union of bytes, words, and a long in endianess aware ordering
typedef union {
#if (Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
    p2_QUAD q;                              //!< quad
    p2_LONG l0, l1;                         //!< long
    p2_WORD w0, w1, w2, w3;                 //!< words 0, 1 as least significant word first
    p2_WORD w[4];                           //!< 4 words as an array
    p2_BYTE b0, b1, b2, b3, b4, b5, b6, b7; //!< bytes 0, 1, 2, 3, 4, 5, 6, 7 as least significant byte first
    p2_BYTE b[8];                           //!< 8 bytes as an array
    char c[8];                              //!< 8 chars as an array
#elif (Q_BYTE_ORDER == Q_BIG_ENDIAN)
    p2_QUAD q;                              //!< quad
    p2_LONG l1, l0;                         //!< long
    p2_WORD w3, w2, w1, w0;                 //!< words 3, 2, 1, 0 as least significant word last
    p2_WORD w[4];                           //!< 4 words as an array
    p2_BYTE b0, b1, b2, b3, b4, b5, b6, b7; //!< bytes 7, 6, 5, 4, 3, 2, 1, 0 as least significant byte last
    p2_BYTE b[8];                           //!< 8 bytes as an array
    char c[8];                              //!< 8 chars as an array
#else
#error "Unknown byte order!"
#endif
}   p2_BWLQ;


//! Size of the HUB memory in bytes (1MiB)
static constexpr p2_LONG MEM_SIZE = 1u << 20;

//! Lowest COG memory address
static constexpr p2_LONG COG_ADDR0 = 0u;

//! COG shift value
static constexpr p2_LONG COG_SHIFT = 9;

//! Size of the COG memory in longs
static constexpr p2_LONG COG_SIZE = 1u << COG_SHIFT;

//! Mask for the COG memory longs
static constexpr p2_LONG COG_MASK = (COG_SIZE-1);

//! Lowest LUT memory address
static constexpr p2_LONG LUT_ADDR0 = (COG_ADDR0+COG_SIZE*4u);

//! COG shift value
static constexpr p2_LONG LUT_SHIFT = 9;

//! Size of the LUT memory in longs
static constexpr p2_LONG LUT_SIZE = 1u << LUT_SHIFT;

//! Mask for the LUT memory longs
static constexpr p2_LONG LUT_MASK = (LUT_SIZE-1);

//! Lowest HUB memory address
static constexpr p2_LONG HUB_ADDR0 = (LUT_ADDR0+LUT_SIZE*4);

//! most significant bit in a 32 bit word
static constexpr p2_LONG MSB = 1u << 31;

//! least significant bit in a 32 bit word
static constexpr p2_LONG LSB = 1u;

//! least significant nibble in a 32 bit word
static constexpr p2_LONG LNIBBLE = 0x0000000fu;

//! least significant byte in a 32 bit word
static constexpr p2_LONG LBYTE = 0x000000ffu;

//! least significant word in a 32 bit word
static constexpr p2_LONG LWORD = 0x0000ffffu;

//! most significant word in a 32 bit word
static constexpr p2_LONG HWORD = 0xffff0000u;

//! bits without sign bit in a 32 bit word
static constexpr p2_LONG IMAX = 0x7fffffffu;

//! no bits in a 32 bit word
static constexpr p2_LONG ZERO = 0x00000000u;

//! all bits in a 32 bit word
static constexpr p2_LONG FULL = 0xffffffffu;

//! least significant 20 bits for an address value
static constexpr p2_LONG A20MASK = (1u << 20) - 1;

//! AUGS/AUGD value shift
static constexpr p2_LONG AUG_SHIFT = 9;

//! most significant 23 bits for an augmentation value
static constexpr p2_LONG AUG_MASK = FULL << AUG_SHIFT;

//! upper word max / mask in a 64 bit unsigned
static constexpr p2_QUAD HMAX = Q_UINT64_C(0xffffffff00000000);

//! lower word max / mask in a 64 bit unsigned
static constexpr p2_QUAD LMAX = Q_UINT64_C(0x00000000ffffffff);

static constexpr p2_LONG p2_mask7 = (1u << 7) - 1;
static constexpr p2_LONG p2_mask8 = (1u << 8) - 1;
static constexpr p2_LONG p2_mask9 = (1u << 9) - 1;
static constexpr p2_LONG p2_shift_CZI = 9 + 9;
static constexpr p2_LONG p2_shift_NNN = 1 + p2_shift_CZI;
static constexpr p2_LONG p2_shift_NN = p2_shift_NNN;
static constexpr p2_LONG p2_shift_N = p2_shift_NNN;
static constexpr p2_LONG p2_shift_inst7 = 3 + 9 + 9;
static constexpr p2_LONG p2_shift_inst8 = 2 + 9 + 9;
static constexpr p2_LONG p2_shift_inst9 = 1 + 9 + 9;
static constexpr p2_LONG p2_shift_opdst = 9;
static constexpr p2_LONG p2_shift_opsrc = 0;

static constexpr p2_LONG p2_mask_inst7 = p2_mask7 << p2_shift_inst7;
static constexpr p2_LONG p2_mask_inst8 = p2_mask8 << p2_shift_inst8;
static constexpr p2_LONG p2_mask_inst9 = p2_mask9 << p2_shift_inst9;
static constexpr p2_LONG p2_mask_C = 4u << p2_shift_CZI;
static constexpr p2_LONG p2_mask_Z = 2u << p2_shift_CZI;
static constexpr p2_LONG p2_mask_I = 1u << p2_shift_CZI;
static constexpr p2_LONG p2_mask_CZ = p2_mask_C | p2_mask_Z;
static constexpr p2_LONG p2_mask_CZI = p2_mask_CZ | p2_mask_I;
static constexpr p2_LONG p2_mask_NNN = 7u << p2_shift_NNN;
static constexpr p2_LONG p2_mask_NN = 3u << p2_shift_NN;
static constexpr p2_LONG p2_mask_N = 1u << p2_shift_N;
static constexpr p2_LONG p2_mask_D = p2_mask9 << p2_shift_opdst;
static constexpr p2_LONG p2_mask_S = p2_mask9 << p2_shift_opsrc;
static constexpr p2_LONG p2_mask_opdst = p2_mask_inst9 | p2_mask_D;
static constexpr p2_LONG p2_mask_opsrc = p2_mask_inst7 | p2_mask_S;
static constexpr p2_LONG p2_mask_opx24 = p2_mask_inst7 | p2_mask_D | p2_mask_S;

static constexpr QChar chr_comma(',');
static constexpr QChar chr_apostrophe('\'');
static constexpr QChar chr_semicolon(';');
static constexpr QChar chr_dquote('"');
static constexpr QChar chr_skip_digit('_');
static constexpr QChar str_escape('\\');
static constexpr QChar chr_lparen('(');
static constexpr QChar chr_rparen(')');
static constexpr QChar chr_lcurly('{');
static constexpr QChar chr_rcurly('}');
static constexpr QChar chr_number_sign('#');
static constexpr QChar chr_ampersand('@');
static constexpr QChar chr_percent('%');
static constexpr QChar chr_dollar('$');
static constexpr QChar chr_pilcrow(L'¶');
static constexpr QChar chr_centerdot(L'·');
static constexpr QChar chr_ldangle(L'«');
static constexpr QChar chr_rdangle(L'»');

extern const QString bin_digits;
extern const QString byt_digits;
extern const QString dec_digits;
extern const QString hex_digits;
extern const QString real_digits;

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
    p2_TJV_OPDST_empty          = INST7(1,0,1,1,1,1,0),
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

    p2_AUGS                     = INST7(1,1,1,1,0,0,0),
    p2_AUGS_01                  = INST7(1,1,1,1,0,0,1),
    p2_AUGS_10                  = INST7(1,1,1,1,0,1,0),
    p2_AUGS_11                  = INST7(1,1,1,1,0,1,1),

    p2_AUGD                     = INST7(1,1,1,1,1,0,0),
    p2_AUGD_01                  = INST7(1,1,1,1,1,0,1),
    p2_AUGD_10                  = INST7(1,1,1,1,1,1,0),
    p2_AUGD_11                  = INST7(1,1,1,1,1,1,1),

}   p2_inst7_e;

Q_STATIC_ASSERT(p2_AUGD_11 == 127u);

typedef enum {

    p2_OPDST_JINT               = 0x000,
    p2_OPDST_JCT1               = 0x001,
    p2_OPDST_JCT2               = 0x002,
    p2_OPDST_JCT3               = 0x003,
    p2_OPDST_JSE1               = 0x004,
    p2_OPDST_JSE2               = 0x005,
    p2_OPDST_JSE3               = 0x006,
    p2_OPDST_JSE4               = 0x007,
    p2_OPDST_JPAT               = 0x008,
    p2_OPDST_JFBW               = 0x009,
    p2_OPDST_JXMT               = 0x00a,
    p2_OPDST_JXFI               = 0x00b,
    p2_OPDST_JXRO               = 0x00c,
    p2_OPDST_JXRL               = 0x00d,
    p2_OPDST_JATN               = 0x00e,
    p2_OPDST_JQMT               = 0x00f,

    p2_OPDST_JNINT              = 0x010,
    p2_OPDST_JNCT1              = 0x011,
    p2_OPDST_JNCT2              = 0x012,
    p2_OPDST_JNCT3              = 0x013,
    p2_OPDST_JNSE1              = 0x014,
    p2_OPDST_JNSE2              = 0x015,
    p2_OPDST_JNSE3              = 0x016,
    p2_OPDST_JNSE4              = 0x017,
    p2_OPDST_JNPAT              = 0x018,
    p2_OPDST_JNFBW              = 0x019,
    p2_OPDST_JNXMT              = 0x01a,
    p2_OPDST_JNXFI              = 0x01b,
    p2_OPDST_JNXRO              = 0x01c,
    p2_OPDST_JNXRL              = 0x01d,
    p2_OPDST_JNATN              = 0x01e,
    p2_OPDST_JNQMT              = 0x01f,
}   p2_opdst_e;

typedef enum {
    p2_OPSRC_HUBSET             = 0x000,
    p2_OPSRC_COGID              = 0x001,
    p2_OPSRC_COGSTOP            = 0x003,
    p2_OPSRC_LOCKNEW            = 0x004,
    p2_OPSRC_LOCKRET            = 0x005,
    p2_OPSRC_LOCKTRY            = 0x006,
    p2_OPSRC_LOCKREL            = 0x007,
    p2_OPSRC_QLOG               = 0x00e,
    p2_OPSRC_QEXP               = 0x00f,

    p2_OPSRC_RFBYTE             = 0x010,
    p2_OPSRC_RFWORD             = 0x011,
    p2_OPSRC_RFLONG             = 0x012,
    p2_OPSRC_RFVAR              = 0x013,
    p2_OPSRC_RFVARS             = 0x014,
    p2_OPSRC_WFBYTE             = 0x015,
    p2_OPSRC_WFWORD             = 0x016,
    p2_OPSRC_WFLONG             = 0x017,

    p2_OPSRC_GETQX              = 0x018,
    p2_OPSRC_GETQY              = 0x019,
    p2_OPSRC_GETCT              = 0x01a,
    p2_OPSRC_GETRND             = 0x01b,
    p2_OPSRC_SETDACS            = 0x01c,
    p2_OPSRC_SETXFRQ            = 0x01d,
    p2_OPSRC_GETXACC            = 0x01e,
    p2_OPSRC_WAITX              = 0x01f,

    p2_OPSRC_SETSE1             = 0x020,
    p2_OPSRC_SETSE2             = 0x021,
    p2_OPSRC_SETSE3             = 0x022,
    p2_OPSRC_SETSE4             = 0x023,
    p2_OPSRC_X24                = 0x024,
    p2_OPSRC_SETINT1            = 0x025,
    p2_OPSRC_SETINT2            = 0x026,
    p2_OPSRC_SETINT3            = 0x027,

    p2_OPSRC_SETQ               = 0x028,
    p2_OPSRC_SETQ2              = 0x029,
    p2_OPSRC_PUSH               = 0x02a,
    p2_OPSRC_POP                = 0x02b,
    p2_OPSRC_JMP                = 0x02c,
    p2_OPSRC_CALL_RET           = 0x02d,
    p2_OPSRC_CALLA_RETA         = 0x02e,
    p2_OPSRC_CALLB_RETB         = 0x02f,

    p2_OPSRC_JMPREL             = 0x030,
    p2_OPSRC_SKIP               = 0x031,
    p2_OPSRC_SKIPF              = 0x032,
    p2_OPSRC_EXECF              = 0x033,
    p2_OPSRC_GETPTR             = 0x034,
    p2_OPSRC_COGBRK             = 0x035,
    p2_OPSRC_BRK                = 0x036,
    p2_OPSRC_SETLUTS            = 0x037,

    p2_OPSRC_SETCY              = 0x038,
    p2_OPSRC_SETCI              = 0x039,
    p2_OPSRC_SETCQ              = 0x03a,
    p2_OPSRC_SETCFRQ            = 0x03b,
    p2_OPSRC_SETCMOD            = 0x03c,
    p2_OPSRC_SETPIV             = 0x03d,
    p2_OPSRC_SETPIX             = 0x03e,
    p2_OPSRC_COGATN             = 0x03f,

    p2_OPSRC_TESTP_W_DIRL       = 0x040,
        p2_OPSRC_TESTP_W        = p2_OPSRC_TESTP_W_DIRL,        //!< with C != Z
        p2_OPSRC_DIRL           = p2_OPSRC_TESTP_W_DIRL,        //!< with C == Z
    p2_OPSRC_TESTPN_W_DIRH      = 0x041,
        p2_OPSRC_TESTPN_W       = p2_OPSRC_TESTPN_W_DIRH,       //!< with C != Z
        p2_OPSRC_DIRH           = p2_OPSRC_TESTPN_W_DIRH,       //!< with C == Z
    p2_OPSRC_TESTP_AND_DIRC     = 0x042,
        p2_OPSRC_TESTP_AND      = p2_OPSRC_TESTP_AND_DIRC,      //!< with C != Z
        p2_OPSRC_DIRC           = p2_OPSRC_TESTP_AND_DIRC,      //!< with C == Z
    p2_OPSRC_TESTPN_AND_DIRNC   = 0x043,
        p2_OPSRC_TESTPN_AND     = p2_OPSRC_TESTPN_AND_DIRNC,    //!< with C != Z
        p2_OPSRC_DIRNC          = p2_OPSRC_TESTPN_AND_DIRNC,    //!< with C == Z
    p2_OPSRC_TESTP_OR_DIRZ      = 0x044,
        p2_OPSRC_TESTP_OR       = p2_OPSRC_TESTP_OR_DIRZ,       //!< with C != Z
        p2_OPSRC_DIRZ           = p2_OPSRC_TESTP_OR_DIRZ,       //!< with C == Z
    p2_OPSRC_TESTPN_OR_DIRNZ    = 0x045,
        p2_OPSRC_TESTPN_OR      = p2_OPSRC_TESTPN_OR_DIRNZ,     //!< with C != Z
        p2_OPSRC_DIRNZ          = p2_OPSRC_TESTPN_OR_DIRNZ,     //!< with C == Z
    p2_OPSRC_TESTP_XOR_DIRRND   = 0x046,
        p2_OPSRC_TESTP_XOR      = p2_OPSRC_TESTP_XOR_DIRRND,    //!< with C != Z
        p2_OPSRC_DIRRND         = p2_OPSRC_TESTP_XOR_DIRRND,    //!< with C == Z
    p2_OPSRC_TESTPN_XOR_DIRNOT  = 0x047,
        p2_OPSRC_TESTPN_XOR     = p2_OPSRC_TESTPN_XOR_DIRNOT,   //!< with C != Z
        p2_OPSRC_DIRNOT         = p2_OPSRC_TESTPN_XOR_DIRNOT,   //!< with C == Z

    p2_OPSRC_OUTL               = 0x048,
    p2_OPSRC_OUTH               = 0x049,
    p2_OPSRC_OUTC               = 0x04a,
    p2_OPSRC_OUTNC              = 0x04b,
    p2_OPSRC_OUTZ               = 0x04c,
    p2_OPSRC_OUTNZ              = 0x04d,
    p2_OPSRC_OUTRND             = 0x04e,
    p2_OPSRC_OUTNOT             = 0x04f,

    p2_OPSRC_FLTL               = 0x050,
    p2_OPSRC_FLTH               = 0x051,
    p2_OPSRC_FLTC               = 0x052,
    p2_OPSRC_FLTNC              = 0x053,
    p2_OPSRC_FLTZ               = 0x054,
    p2_OPSRC_FLTNZ              = 0x055,
    p2_OPSRC_FLTRND             = 0x056,
    p2_OPSRC_FLTNOT             = 0x057,

    p2_OPSRC_DRVL               = 0x058,
    p2_OPSRC_DRVH               = 0x059,
    p2_OPSRC_DRVC               = 0x05a,
    p2_OPSRC_DRVNC              = 0x05b,
    p2_OPSRC_DRVZ               = 0x05c,
    p2_OPSRC_DRVNZ              = 0x05d,
    p2_OPSRC_DRVRND             = 0x05e,
    p2_OPSRC_DRVNOT             = 0x05f,

    p2_OPSRC_SPLITB             = 0x060,
    p2_OPSRC_MERGEB             = 0x061,
    p2_OPSRC_SPLITW             = 0x062,
    p2_OPSRC_MERGEW             = 0x063,
    p2_OPSRC_SEUSSF             = 0x064,
    p2_OPSRC_SEUSSR             = 0x065,
    p2_OPSRC_RGBSQZ             = 0x066,
    p2_OPSRC_RGBEXP             = 0x067,

    p2_OPSRC_XORO32             = 0x068,
    p2_OPSRC_REV                = 0x069,
    p2_OPSRC_RCZR               = 0x06a,
    p2_OPSRC_RCZL               = 0x06b,
    p2_OPSRC_WRC                = 0x06c,
    p2_OPSRC_WRNC               = 0x06d,
    p2_OPSRC_WRZ                = 0x06e,
    p2_OPSRC_WRNZ_MODCZ         = 0x06f,

    p2_OPSRC_SETSCP             = 0x070,
    p2_OPSRC_GETSCP             = 0x071,
}   p2_opsrc_e;

typedef enum {
    p2_OPX24_POLLINT            = 0x000,
    p2_OPX24_POLLCT1            = 0x001,
    p2_OPX24_POLLCT2            = 0x002,
    p2_OPX24_POLLCT3            = 0x003,
    p2_OPX24_POLLSE1            = 0x004,
    p2_OPX24_POLLSE2            = 0x005,
    p2_OPX24_POLLSE3            = 0x006,
    p2_OPX24_POLLSE4            = 0x007,
    p2_OPX24_POLLPAT            = 0x008,
    p2_OPX24_POLLFBW            = 0x009,
    p2_OPX24_POLLXMT            = 0x00a,
    p2_OPX24_POLLXFI            = 0x00b,
    p2_OPX24_POLLXRO            = 0x00c,
    p2_OPX24_POLLXRL            = 0x00d,
    p2_OPX24_POLLATN            = 0x00e,
    p2_OPX24_POLLQMT            = 0x00f,

    p2_OPX24_WAITINT            = 0x010,
    p2_OPX24_WAITCT1            = 0x011,
    p2_OPX24_WAITCT2            = 0x012,
    p2_OPX24_WAITCT3            = 0x013,
    p2_OPX24_WAITSE1            = 0x014,
    p2_OPX24_WAITSE2            = 0x015,
    p2_OPX24_WAITSE3            = 0x016,
    p2_OPX24_WAITSE4            = 0x017,
    p2_OPX24_WAITPAT            = 0x018,
    p2_OPX24_WAITFBW            = 0x019,
    p2_OPX24_WAITXMT            = 0x01a,
    p2_OPX24_WAITXFI            = 0x01b,
    p2_OPX24_WAITXRO            = 0x01c,
    p2_OPX24_WAITXRL            = 0x01d,
    p2_OPX24_WAITATN            = 0x01e,

    p2_OPX24_ALLOWI             = 0x020,
    p2_OPX24_STALLI             = 0x021,
    p2_OPX24_TRGINT1            = 0x022,
    p2_OPX24_TRGINT2            = 0x023,
    p2_OPX24_TRGINT3            = 0x024,
    p2_OPX24_NIXINT1            = 0x025,
    p2_OPX24_NIXINT2            = 0x026,
    p2_OPX24_NIXINT3            = 0x027,
}   p2_opx24_e;

typedef enum {
    p2_OPCODE_WMLONG            = 0x053,
    p2_OPCODE_RDBYTE            = 0x056,
    p2_OPCODE_RDWORD            = 0x057,
    p2_OPCODE_RDLONG            = 0x058,
    p2_OPCODE_CALLD             = 0x059,
    p2_OPCODE_CALLPB            = 0x05e,
    p2_OPCODE_JINT              = 0x05f,
    p2_OPCODE_WRBYTE            = 0x062,
    p2_OPCODE_WRWORD            = 0x062,
    p2_OPCODE_WRLONG            = 0x063,
    p2_OPCODE_QMUL              = 0x068,
    p2_OPCODE_QVECTOR           = 0x06a,
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
 * @brief Enumeration of the 8 bit instruction types
 * This is the 7 bit instruction with appended WC
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

    p2_TJV_OPDST                = INST8(p2_TJV_OPDST_empty,0),
    p2_1011110_1                = INST8(p2_TJV_OPDST_empty,1),
    p2_1011111_0                = INST8(p2_empty_SETPAT,0),
    p2_SETPAT                   = INST8(p2_empty_SETPAT,1),

    p2_RQPIN                    = INST8(p2_RQPIN_RDPIN,0),
    p2_RDPIN                    = INST8(p2_RQPIN_RDPIN,1),

    p2_WRPIN                    = INST8(p2_WRPIN_AKPIN_WXPIN, 0),
    p2_WXPIN                    = INST8(p2_WRPIN_AKPIN_WXPIN, 1),

    p2_WYPIN                    = INST8(p2_WYPIN_WRLUT,0),
    p2_WRLUT                    = INST8(p2_WYPIN_WRLUT,1),

    p2_WRBYTE                   = INST8(p2_WRBYTE_WRWORD,0),
    p2_WRWORD                   = INST8(p2_WRBYTE_WRWORD,1),

    p2_WRLONG                   = INST8(p2_WRLONG_RDFAST,0),
    p2_RDFAST                   = INST8(p2_WRLONG_RDFAST,1),

    p2_WRFAST                   = INST8(p2_WRFAST_FBLOCK,0),
    p2_FBLOCK                   = INST8(p2_WRFAST_FBLOCK,1),

    p2_XINIT                    = INST8(p2_XINIT_XSTOP_XZERO,0),
    p2_XZERO                    = INST8(p2_XINIT_XSTOP_XZERO,1),

    p2_XCONT                    = INST8(p2_XCONT_REP,0),
    p2_REP                      = INST8(p2_XCONT_REP,1),

}   p2_inst8_e;

/**
 * @brief Enumeration of the 9 bit instruction types
 * This is the 7 bit instruction with appended WC and WZ
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

    p2_DIRL_eol                 = INST9(p2_OPSRC,0,0),
    p2_TESTP_WZ                 = INST9(p2_OPSRC,0,1),
    p2_TESTP_WC                 = INST9(p2_OPSRC,1,0),
    p2_DIRL_WCZ                 = INST9(p2_OPSRC,1,1),

    p2_DIRH_eol                 = INST9(p2_OPSRC,0,0),
    p2_TESTPN_WZ                = INST9(p2_OPSRC,0,1),
    p2_TESTPN_WC                = INST9(p2_OPSRC,1,0),
    p2_DIRH_WCZ                 = INST9(p2_OPSRC,1,1),

    p2_DIRC_eol                 = INST9(p2_OPSRC,0,0),
    p2_TESTP_ANDZ               = INST9(p2_OPSRC,0,1),
    p2_TESTP_ANDC               = INST9(p2_OPSRC,1,0),
    p2_DIRC_WCZ                 = INST9(p2_OPSRC,1,1),

    p2_DIRNC_eol                = INST9(p2_OPSRC,0,0),
    p2_TESTPN_ANDZ              = INST9(p2_OPSRC,0,1),
    p2_TESTPN_ANDC              = INST9(p2_OPSRC,1,0),
    p2_DIRNC_WCZ                = INST9(p2_OPSRC,1,1),

    p2_DIRZ_eol                 = INST9(p2_OPSRC,0,0),
    p2_TESTP_ORZ                = INST9(p2_OPSRC,0,1),
    p2_TESTP_ORC                = INST9(p2_OPSRC,1,0),
    p2_DIRZ_WCZ                 = INST9(p2_OPSRC,1,1),

    p2_DIRNZ_eol                = INST9(p2_OPSRC,0,0),
    p2_TESTPN_ORZ               = INST9(p2_OPSRC,0,1),
    p2_TESTPN_ORC               = INST9(p2_OPSRC,1,0),
    p2_DIRNZ_WCZ                = INST9(p2_OPSRC,1,1),

    p2_DIRRND_eol               = INST9(p2_OPSRC,0,0),
    p2_TESTP_XORZ               = INST9(p2_OPSRC,0,1),
    p2_TESTP_XORC               = INST9(p2_OPSRC,1,0),
    p2_DIRRND_WCZ               = INST9(p2_OPSRC,1,1),

    p2_DIRNOT_eol               = INST9(p2_OPSRC,0,0),
    p2_TESTPN_XORZ              = INST9(p2_OPSRC,0,1),
    p2_TESTPN_XORC              = INST9(p2_OPSRC,1,0),
    p2_DIRNOT_WCZ               = INST9(p2_OPSRC,1,1),

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

    p2_RESI0                    = INST9(p2_CALLD,0,0),
    p2_RESI1                    = INST9(p2_CALLD,0,1),
    p2_RESI2                    = INST9(p2_CALLD,1,0),
    p2_RESI3                    = INST9(p2_CALLD,1,1),

    p2_RETI0                    = INST9(p2_CALLD,0,0),
    p2_RETI1                    = INST9(p2_CALLD,0,1),
    p2_RETI2                    = INST9(p2_CALLD,1,0),
    p2_RETI3                    = INST9(p2_CALLD,1,1),

    p2_TJV                      = INST9(p2_TJV_OPDST_empty,0,0),
    p2_OPDST                    = INST9(p2_TJV_OPDST_empty,0,1),
    p2_1011110_10               = INST9(p2_TJV_OPDST_empty,1,0),
    p2_1011110_11               = INST9(p2_TJV_OPDST_empty,1,1),

    p2_COGBRK                   = INST9(p2_OPSRC_COGBRK,0,0),
    p2_GETBRK_WZ                = INST9(p2_OPSRC_COGBRK,0,1),
    p2_GETBRK_WC                = INST9(p2_OPSRC_COGBRK,1,0),
    p2_GETBRK_WCZ               = INST9(p2_OPSRC_COGBRK,1,1),

}   p2_inst9_e;


/**
 * @brief Structure of the Propeller2 opcode words with 7 bits instruction, wc, wz, and im
 *
 * Note: To have the bit fields work for both, little
 * and big endian machines, the order has to match
 * the native order of bits in 32 bit words of the target.
 */
typedef struct {
#if (Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
    uint src:9;                 //!< source (S or #S)
    uint dst:9;                 //!< destination (D or #D)
    bool im:1;                  //!< immediate flag
    bool wz:1;                  //!< update Z flag
    bool wc:1;                  //!< update C flag
    uint inst:7;                //!< instruction type
    uint cond:4;                //!< conditional execution
#elif (Q_BYTE_ORDER == Q_BIG_ENDIAN)
    uint cond:4;                //!< conditional execution
    uint inst:7;                //!< instruction type
    bool uc:1;                  //!< update C flag
    bool uz:1;                  //!< update Z flag
    bool im:1;                  //!< immediate flag
    uint dst:9;                 //!< destination (D or #D)
    uint src:9;                 //!< source (S or #S)
#else
#error "Unknown byte order!"
#endif
}   p2_opcode7_t;

/**
 * @brief Structure of the Propeller2 opcode words with 8 bits of instruction, wz, and im
 */
typedef struct {
#if (Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
    uint src:9;                 //!< source (S or #S)
    uint dst:9;                 //!< destination (D or #D)
    bool im:1;                  //!< immediate flag
    bool wz:1;                  //!< update Z flag
    uint inst:8;                //!< instruction type including WC
    uint cond:4;                //!< conditional execution
#elif (Q_BYTE_ORDER == Q_BIG_ENDIAN)
    uint cond:4;                //!< conditional execution
    uint inst:8;                //!< instruction type including WC
    bool wz:1;                  //!< update Z flag
    bool im:1;                  //!< immediate flag
    uint dst:9;                 //!< destination (D or #D)
    uint src:9;                 //!< source (S or #S)
#else
#error "Unknown byte order!"
#endif
}   p2_opcode8_t;

/**
 * @brief Structure of the Propeller2 opcode words with 9 bits of instruction and im
 */
typedef struct {
#if (Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
    uint src:9;                 //!< source (S or #S)
    uint dst:9;                 //!< destination (D or #D)
    bool im:1;                  //!< immediate flag
    uint inst:9;                //!< instruction type including WC and WZ
    uint cond:4;                //!< conditional execution
#elif (Q_BYTE_ORDER == Q_BIG_ENDIAN)
    uint cond:4;                //!< conditional execution
    uint inst:9;                //!< instruction type including WC and WZ
    bool im:1;                  //!< immediate flag
    uint dst:9;                 //!< destination (D or #D)
    uint src:9;                 //!< source (S or #S)
#else
#error "Unknown byte order!"
#endif
}   p2_opcode9_t;

typedef union {
    p2_LONG opcode;             //!< opcode as 32 bit word
    p2_opcode7_t op;            //!< ocpode as bit fields (version with 7 bits instruction)
    p2_opcode8_t op8;           //!< ocpode as bit fields (version including WC)
    p2_opcode9_t op9;           //!< ocpode as bit fields (version including WC and WZ)
}   p2_opcode_u;

/**
 * @brief Structure of the LUT and the shadow registers in the last 16 LONGs
 */
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

/**
 * @brief Offsets of the LUT shadow registers
 */
typedef enum {
    offs_IJMP3 = 0x1f0,         //!< offset of interrupt call address for INT3
    offs_IRET3,                 //!< offset of interrupt return address for INT3
    offs_IJMP2,                 //!< offset of interrupt call address for INT2
    offs_IRET2,                 //!< offset of interrupt return address for INT2
    offs_IJMP1,                 //!< offset of interrupt call address for INT1
    offs_IRET1,                 //!< offset of interrupt return address for INT1
    offs_PA,                    //!< offset of CALLD-imm return, CALLPA parameter, or LOC address
    offs_PB,                    //!< offset of CALLD-imm return, CALLPB parameter, or LOC address
    offs_PTRA,                  //!< offset of pointer A to hub RAM
    offs_PTRB,                  //!< offset of pointer B to hub RAM
    offs_DIRA,                  //!< offset of output enables for P31 ... P0
    offs_DIRB,                  //!< offset of output enables for P63 ... P32
    offs_OUTA,                  //!< offset of output states for P31 ... P0
    offs_OUTB,                  //!< offset of output states for P63 ... P32
    offs_INA,                   //!< offset of input states for P31 ... P0
    offs_INB,                   //!< offset of input states for P63 ... P32
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
 * @brief 16 flag bits per COG
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
 * @brief PAT pattern matching mode enum
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
 * @brief PIN edge mode enum
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
    p2_int_flags_t flags;       //!< interrupt flags as bit masks
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
 * @brief WAIT wait reason enum
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
 * @brief WAIT wait status
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
    fmt_bin,
    fmt_byt,
    fmt_dec,
    fmt_hex,
}   p2_opcode_format_e;

extern const QString template_str_origin;
extern const QString template_str_address;
extern const QString template_str_opcode_bin;
extern const QString template_str_opcode_byt;
extern const QString template_str_opcode_oct;
extern const QString template_str_opcode_dec;
extern const QString template_str_opcode_hex;
extern const QString template_str_tokens;
extern const QString template_str_symbols;
extern const QString template_str_errors;
extern const QString template_str_instruction;
extern const QString template_str_description;
extern const QString key_tv_asm;

typedef enum {
    color_source,
    color_comment,
    color_comma,
    color_str_const,
    color_bin_const,
    color_byt_const,
    color_dec_const,
    color_hex_const,
    color_real_const,
    color_locsym,
    color_symbol,
    color_expression,
    color_section,
    color_conditional,
    color_modzc_param,
    color_instruction,
    color_wcz_suffix,
}   p2_palette_e;

extern QColor p2_palette(p2_palette_e pal, bool highlight = false);
extern QColor p2_palette(p2_token_e tok, bool highlight = false);

extern QString format_opcode_bin(const p2_opcode_u& IR);
extern QString format_opcode_byt(const p2_opcode_u& IR);
extern QString format_opcode_oct(const p2_opcode_u& IR);
extern QString format_opcode_dec(const p2_opcode_u& IR);
extern QString format_opcode_hex(const p2_opcode_u& IR);

extern QString format_data_bin(const p2_opcode_u& IR, bool prefix = false);
extern QString format_data_byt(const p2_opcode_u& IR, bool prefix = false);
extern QString format_data_oct(const p2_opcode_u& IR, bool prefix = false);
extern QString format_data_dec(const p2_opcode_u& IR);
extern QString format_data_hex(const p2_opcode_u& IR, bool prefix = false);

extern QString format_data_bin(const p2_LONG data, bool prefix = false);
extern QString format_data_byt(const p2_LONG data, bool prefix = false);
extern QString format_data_oct(const p2_LONG data, bool prefix = false);
extern QString format_data_dec(const p2_LONG data);
extern QString format_data_hex(const p2_LONG data, bool prefix = false);

extern QString format_opcode(const p2_opcode_u& IR, const p2_opcode_format_e fmt = fmt_bin);
extern QString format_data(const p2_opcode_u& IR, const p2_opcode_format_e fmt = fmt_bin, bool prefix = false);
extern QString format_data(const p2_LONG data, const p2_opcode_format_e fmt = fmt_bin, bool prefix = false);

/**
 * @brief enumeration of token types
 */
typedef enum {
    tt_none,            //!< no specific type
    tt_comment,         //!< comment
    tt_parens,          //!< precedence  0: parenthesis "(", ")", "[", "]"
    tt_primary,         //!< precedence  1: primary operators (++, --)
    tt_unary,           //!< precedence  2: unary operators (+, -, !, ~, more?)
    tt_shiftop,         //!< precedence  3: shift operators (<<, >>)
    tt_binop_rev,       //!< precedence  4: binary reverse (><)
    tt_mulop,           //!< precedence  5: multiplication operators (*, /, \)
    tt_addop,           //!< precedence  6: addition operators (+, -)
    tt_binop_and,       //!< precedence  7: binary and (&)
    tt_binop_xor,       //!< precedence  8: binary xor (^)
    tt_binop_or,        //!< precedence  9: binary or (|)
    tt_binop_encod,     //!< precedence 10: binary encode (>|)
    tt_binop_decod,     //!< precedence 11: binary decode (|<)
    tt_relation,        //!< precedence 12: comparisons (<, <=, >, >=)
    tt_equality,        //!< precedence 13: comparisons (==, !=)
    tt_logop_and,       //!< precedence 14: logical and (&&)
    tt_logop_or,        //!< precedence 15: logical or (||)
    tt_ternary,         //!< precedence 16: ternary operator (?:)
    tt_assignment,      //!< precedence 17: assignment (=)
    tt_delimiter,       //!< delimiter (,)
    tt_constant,        //!< constant value ($, PA, PB, PTRA, PTRB)
    tt_function,        //!< function on value: FLOAT()
    tt_traits,          //!< immediate (#), relative (@), HUB address (#@), force augmented (##), force absolute (\)
    tt_relative,        //!< relative value (@, @@@)
    tt_conditional,     //!< conditional execution
    tt_modcz_param,     //!< MODCZ parameters
    tt_mnemonic,        //!< instruction mnemonic
    tt_wcz_suffix,      //!< suffixes WC, WZ, WCZ, ANDC, ANDZ, ORC, ORZ, XORC, XORZ
    tt_section,         //!< section control
    tt_origin,          //!< origin control
    tt_data,            //!< data generating
    tt_regexp,          //!< pseudo token from lexing a string
}   p2_t_type_e;

typedef quint64 p2_t_mask_t;

/**
 * @brief bit masks for token types
 */
#define TTMASK(tt) static_cast<p2_t_mask_t>(Q_UINT64_C(1) << (tt))

static constexpr p2_t_mask_t tm_none        = 0;
static constexpr p2_t_mask_t tm_comment     = TTMASK(tt_comment);
static constexpr p2_t_mask_t tm_parens      = TTMASK(tt_parens);
static constexpr p2_t_mask_t tm_primary     = TTMASK(tt_primary);
static constexpr p2_t_mask_t tm_unary       = TTMASK(tt_unary);
static constexpr p2_t_mask_t tm_shiftop     = TTMASK(tt_shiftop);
static constexpr p2_t_mask_t tm_binop_rev   = TTMASK(tt_binop_rev);
static constexpr p2_t_mask_t tm_mulop       = TTMASK(tt_mulop);
static constexpr p2_t_mask_t tm_addop       = TTMASK(tt_addop);
static constexpr p2_t_mask_t tm_binop_and   = TTMASK(tt_binop_and);
static constexpr p2_t_mask_t tm_binop_xor   = TTMASK(tt_binop_xor);
static constexpr p2_t_mask_t tm_binop_or    = TTMASK(tt_binop_or);
static constexpr p2_t_mask_t tm_binop_encod = TTMASK(tt_binop_encod);
static constexpr p2_t_mask_t tm_binop_decod = TTMASK(tt_binop_decod);
static constexpr p2_t_mask_t tm_relation    = TTMASK(tt_relation);
static constexpr p2_t_mask_t tm_equality    = TTMASK(tt_equality);
static constexpr p2_t_mask_t tm_logop_and   = TTMASK(tt_logop_and);
static constexpr p2_t_mask_t tm_logop_or    = TTMASK(tt_logop_or);
static constexpr p2_t_mask_t tm_ternary     = TTMASK(tt_ternary);
static constexpr p2_t_mask_t tm_assignment  = TTMASK(tt_assignment);
static constexpr p2_t_mask_t tm_delimiter   = TTMASK(tt_delimiter);
static constexpr p2_t_mask_t tm_constant    = TTMASK(tt_constant);
static constexpr p2_t_mask_t tm_function    = TTMASK(tt_function);
static constexpr p2_t_mask_t tm_traits      = TTMASK(tt_traits);
static constexpr p2_t_mask_t tm_conditional = TTMASK(tt_conditional);
static constexpr p2_t_mask_t tm_modcz_param = TTMASK(tt_modcz_param);
static constexpr p2_t_mask_t tm_mnemonic    = TTMASK(tt_mnemonic);
static constexpr p2_t_mask_t tm_wcz_suffix  = TTMASK(tt_wcz_suffix);
static constexpr p2_t_mask_t tm_section     = TTMASK(tt_section);
static constexpr p2_t_mask_t tm_origin      = TTMASK(tt_origin);
static constexpr p2_t_mask_t tm_data        = TTMASK(tt_data);
static constexpr p2_t_mask_t tm_lexer       = TTMASK(tt_regexp);

static constexpr p2_t_mask_t tm_primary_unary =
        tm_primary |
        tm_unary;

static constexpr p2_t_mask_t tm_binop =
        tm_binop_and |
        tm_binop_xor |
        tm_binop_or |
        tm_binop_rev |
        tm_binop_encod |
        tm_binop_decod;

static constexpr p2_t_mask_t tm_operations =
        tm_parens |
        tm_primary |
        tm_unary |
        tm_mulop |
        tm_addop |
        tm_shiftop |
        tm_relation |
        tm_equality |
        tm_binop_and |
        tm_binop_xor |
        tm_binop_or |
        tm_binop_rev |
        tm_binop_encod |
        tm_binop_decod |
        tm_logop_and |
        tm_logop_or;

static constexpr p2_t_mask_t tm_expression =
        tm_function |
        tm_operations |
        tm_constant;
