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
#include <QChar>
#include <QColor>
#include <QHash>
#include <QtEndian>

/**
 * @file This file defines the basic constants, enumerations, and structure
 * of opcodes (instructions) and the bitfields contained therein of the
 * Parallax Inc. Propeller2 microprocessor / microcontroller.
 */

#include "p2tokens.h"

/*
 * Definitions for the basic types based on Qt5 types
 */

//! Type of the Propeller2 BYTE
typedef quint8 p2_BYTE;
Q_DECLARE_METATYPE(p2_BYTE)
static constexpr int sz_BYTE = sizeof(p2_BYTE);

//! Type of the Propeller2 WORD
typedef quint16 p2_WORD;
Q_DECLARE_METATYPE(p2_WORD)
static constexpr int sz_WORD = sizeof(p2_WORD);

//! Type of the Propeller2 LONG
typedef quint32 p2_LONG;
Q_DECLARE_METATYPE(p2_LONG)
static constexpr int sz_LONG = sizeof(p2_LONG);

//! Intermediate type (unsigned long long); not a Propeller2 type, but used in expressions
typedef quint64 p2_QUAD;
Q_DECLARE_METATYPE(p2_QUAD)
static constexpr int sz_QUAD = sizeof(p2_QUAD);

//! Intermediate type (double); not a Propeller2 type, but used in expressions
typedef double p2_REAL;
Q_DECLARE_METATYPE(p2_REAL)
static constexpr int sz_REAL = sizeof(p2_REAL);

//! Type for an array (QVector) of BYTEs (interpret as ut_Byte or ut_Bool)
typedef QVector<p2_BYTE> p2_BYTES;
Q_DECLARE_METATYPE(p2_BYTES)

//! Type for an array (QVector) of WORDs (ut_Word)
typedef QVector<p2_WORD> p2_WORDS;
Q_DECLARE_METATYPE(p2_WORDS)

//! Type for an array (QVector) of LONGs (ut_Addr or ut_Long)
typedef QVector<p2_LONG> p2_LONGS;
Q_DECLARE_METATYPE(p2_LONGS)

//! Type for an array (QVector) of QUADs (ut_Quad)
typedef QVector<p2_QUAD> p2_QUADS;
Q_DECLARE_METATYPE(p2_QUADS)

//! Type for an array (QVector) of REALs (ut_Real)
typedef QVector<p2_REAL> p2_REALS;
Q_DECLARE_METATYPE(p2_REALS)

//! Type for an array (QVector) of chars (ut_String interpreted as ut_Byte or char)
typedef QVector<char> p2_CHARS;
Q_DECLARE_METATYPE(p2_CHARS)

//! Size of the HUB memory, i.e. address range, in bytes (this is 1MiB)
static constexpr p2_LONG MEM_SIZE = 1u << 20;

//! Lowest COG memory address
static constexpr p2_LONG COG_ADDR0 = 0u;

//! COG shift value
static constexpr p2_LONG COG_SHIFT = 9;

//! Size of the COG memory in LONGs
static constexpr p2_LONG COG_SIZE = 1u << COG_SHIFT;

//! Mask for the COG memory LONGs
static constexpr p2_LONG COG_MASK = COG_SIZE-1;

//! Lowest LUT memory address (in BYTEs)
static constexpr p2_LONG LUT_ADDR0 = COG_ADDR0+COG_SIZE*4u;

//! COG shift value
static constexpr p2_LONG LUT_SHIFT = 9;

//! Size of the LUT memory in LONGs
static constexpr p2_LONG LUT_SIZE = 1u << LUT_SHIFT;

//! Mask for the LUT memory LONGs
static constexpr p2_LONG LUT_MASK = LUT_SIZE-1;

//! Lowest HUB memory address (in BYTEs)
static constexpr p2_LONG HUB_ADDR0 = LUT_ADDR0+LUT_SIZE*4;

//! The most significant bit in a 32 bit word
static constexpr p2_LONG MSB = 1u << 31;

//! The least significant bit in a 32 bit word
static constexpr p2_LONG LSB = 1u;

//! The least significant nibble in a 32 bit word
static constexpr p2_LONG LNIBBLE = 0x0000000fu;

//! The least significant BYTE in a 32 bit LONG
static constexpr p2_LONG LBYTE = 0x000000ffu;

//! The least significant WORD in a 32 bit LONG
static constexpr p2_LONG LWORD = 0x0000ffffu;

//! The most significant WORD in a 32 bit LONG
static constexpr p2_LONG HWORD = 0xffff0000u;

//! All bits except the sign bit in a 32 bit LONG
static constexpr p2_LONG IMAX = 0x7fffffffu;

//! No bits 1 in a 32 bit LONG
static constexpr p2_LONG ZERO = 0x00000000u;

//! All bits 1 in a 32 bit LONG
static constexpr p2_LONG FULL = 0xffffffffu;

//! The least significant 20 bits for an address value
static constexpr p2_LONG A20MASK = (1u << 20) - 1;

//! The AUGS/AUGD value shift count
static constexpr p2_LONG AUG_SHIFT = 9;

//! The most significant 23 bits for an AUGS/AUGD (augmentation) value
static constexpr p2_LONG AUG_MASK = FULL << AUG_SHIFT;

//! The upper LONG max / mask in a 64 bit unsigned (QUAD)
static constexpr p2_QUAD HMAX = Q_UINT64_C(0xffffffff00000000);

//! The lower LONG max / mask in a 64 bit unsigned (QUAD)
static constexpr p2_QUAD LMAX = Q_UINT64_C(0x00000000ffffffff);

//! The mask for 5 bit instructions
static constexpr p2_LONG p2_mask5 = (1u << 5) - 1;

//! The mask for 7 bit instructions
static constexpr p2_LONG p2_mask7 = (1u << 7) - 1;

//! The mask for 8 bit instructions
static constexpr p2_LONG p2_mask8 = (1u << 8) - 1;

//! The mask for 9 bit instructions or DST/SRC values
static constexpr p2_LONG p2_mask9 = (1u << 9) - 1;

//! The shift count for the I bit in an instruction
static constexpr p2_LONG p2_shift_I = 9 + 9;

//! The shift count for the Z bit in an instruction
static constexpr p2_LONG p2_shift_Z = 1 + 9 + 9;

//! The shift count for the C bit in an instruction
static constexpr p2_LONG p2_shift_C = 1 + 1 + 9 + 9;

//! The shift count for the NNN bits (nibble index) in an instruction
static constexpr p2_LONG p2_shift_NNN = p2_shift_Z;

//! The shift count for the NN bits (BYTE index) in an instruction
static constexpr p2_LONG p2_shift_NN = p2_shift_Z;

//! The shift count for the N bit (WORD index) in an instruction
static constexpr p2_LONG p2_shift_N = p2_shift_Z;

//! The shift count for a 5 bit instruction in the opcode
static constexpr p2_LONG p2_shift_inst5 = 32 - 4 - 5;

//! The shift count for a 7 bit instruction in the opcode
static constexpr p2_LONG p2_shift_inst7 = 32 - 4 - 7;

//! The shift count for an 8 bit instruction in the opcode
static constexpr p2_LONG p2_shift_inst8 = 32 - 4 - 8;

//! The shift count for a 9 bit instruction in the opcode
static constexpr p2_LONG p2_shift_inst9 = 32 - 4 - 9;

//! The shift count for the D field in the opcode
static constexpr p2_LONG p2_shift_opdst = 9;

//! The shift count for the S field in the opcode
static constexpr p2_LONG p2_shift_opsrc = 0;

//! The bit mask for a 5 bit instruction
static constexpr p2_LONG p2_mask_inst5 = p2_mask5 << p2_shift_inst5;

//! The bit mask for a 7 bit instruction
static constexpr p2_LONG p2_mask_inst7 = p2_mask7 << p2_shift_inst7;

//! The bit mask for an 8 bit instruction
static constexpr p2_LONG p2_mask_inst8 = p2_mask8 << p2_shift_inst8;

//! The bit mask for a 9 bit instruction
static constexpr p2_LONG p2_mask_inst9 = p2_mask9 << p2_shift_inst9;

//! The bit mask for the C bit in an instruction
static constexpr p2_LONG p2_mask_C = 1u << p2_shift_C;

//! The bit mask for the Z bit in an instruction
static constexpr p2_LONG p2_mask_Z = 1u << p2_shift_Z;

//! The bit mask for the I bit in an instruction
static constexpr p2_LONG p2_mask_I = 1u << p2_shift_I;

//! The bit mask for both, the C and Z bits in an instruction
static constexpr p2_LONG p2_mask_CZ = p2_mask_C | p2_mask_Z;

//! The bit mask for all of the  C, Z, and I bits in an instruction
static constexpr p2_LONG p2_mask_CZI = p2_mask_C | p2_mask_Z | p2_mask_I;

//! The bit mask for a nibble index in an instruction
static constexpr p2_LONG p2_mask_NNN = 7u << p2_shift_NNN;

//! The bit mask for a BYTE index in an instruction
static constexpr p2_LONG p2_mask_NN = 3u << p2_shift_NN;

//! The bit mask for a WORD index in an instruction
static constexpr p2_LONG p2_mask_N = 1u << p2_shift_N;

//! The bit mask for a D value in an instruction
static constexpr p2_LONG p2_mask_D = p2_mask9 << p2_shift_opdst;

//! The bit mask for an S value in an instruction
static constexpr p2_LONG p2_mask_S = p2_mask9 << p2_shift_opsrc;

//! The bit mask for a p2_opdst_e instruction (9 bit instruction and D) in an opcode
static constexpr p2_LONG p2_mask_opdst = p2_mask_inst9 | p2_mask_D;

//! The bit mask for a p2_opsrc_e instruction (7 bit instruction and S) in an opcode
static constexpr p2_LONG p2_mask_opsrc = p2_mask_inst7 | p2_mask_S;

//! The bit mask for a p2_opx24_e instruction (7 bit instruction, D, and S) in an opcode
static constexpr p2_LONG p2_mask_opx24 = p2_mask_inst7 | p2_mask_D | p2_mask_S;

//! Just a name for a commonly used character: separates arguments of an instruction
static constexpr QChar chr_comma(',');

//! Just a name for a commonly used character: introduces a local symbol
static constexpr QChar chr_period('.');

//! Just a name for a commonly used character: the rest of the line is comment
static constexpr QChar chr_apostrophe('\'');

//! Just a name for a commonly used character: huh?
static constexpr QChar chr_semicolon(';');

//! Just a name for a commonly used character: double quotes enclose strings
static constexpr QChar chr_dquote('"');

//! Just a name for a commonly used character: underscore is a skipped character in numeric constants
static constexpr QChar chr_skip_digit('_');

//! Just a name for a commonly used character: string escape and also trait of an argument
static constexpr QChar chr_escape('\\');

//! Just a name for a commonly used character: start of higher precedence subexpression
static constexpr QChar chr_lparen('(');

//! Just a name for a commonly used character: end of higher precedence subexpression
static constexpr QChar chr_rparen(')');

//! Just a name for a commonly used character: starts a single (in line) or multi line comment
static constexpr QChar chr_lcurly('{');

//! Just a name for a commonly used character: ends a single (in line) or multi line comment
static constexpr QChar chr_rcurly('}');

//! Just a name for a commonly used character: generally means "immediate"
static constexpr QChar chr_number_sign('#');

//! Just a name for a commonly used character: generally means "relative"
static constexpr QChar chr_ampersand('@');

//! Just a name for a commonly used character: first character for a binary constant (double %% is a byte index, i.e. base 4)
static constexpr QChar chr_percent('%');

//! Just a name for a commonly used character: first character for a binary constant (double %% is a byte index, i.e. base 4)
static const QString str_byt_prefix = QLatin1String("%%");

//! Just a name for a commonly used character: delimiter between HUB and COG address
static constexpr QChar chr_colon(':');

//! Just a name for a commonly used character: represents the current origin (PC)
static constexpr QChar chr_dollar('$');

//! Just a name for a commonly used character: character used in assignment output
static constexpr QChar chr_larrow(L'←');

//! Just a name for a commonly used character: character used in assignment output
static constexpr QChar chr_rarrow(L'→');

//! Just a name for a commonly used character: character used in tokenizer debugging
static constexpr QChar chr_pilcrow(L'¶');

//! Just a name for a commonly used character: character used to enclose non-token names for type masks
static constexpr QChar chr_centerdot(L'·');

//! Just a name for a commonly used character: character used in tokenizer debugging
static constexpr QChar chr_ldangle(L'«');

//! Just a name for a commonly used character: character used in tokenizer debugging
static constexpr QChar chr_rdangle(L'»');

/**
 * @brief enumeration defining the basic data types used with the Propeller2
 * These are all unsigned types as signed operations are just a specific
 * subset in few Propeller2 instuctions.
 */
typedef enum {
    ut_Invalid = -1,                        //!< Type is not set
    ut_Bool,                                //!< Value is bool
    ut_Byte,                                //!< Value is BYTE sized
    ut_Word,                                //!< Value is WORD sized
    ut_Addr,                                //!< Value is an address (Program Counter)
    ut_Long,                                //!< Value is LONG sized
    ut_Quad,                                //!< Value is QUAD sized
    ut_Real,                                //!< Value is REAL sized (qreal can be float or double)
    ut_String                               //!< Value is an array of char
}   p2_Union_e;
Q_DECLARE_METATYPE(p2_Union_e)

/**
 * @brief Union of the integral Propeller2 types to store in a QVector
 */
typedef union {
    bool _bool;
    char _char;
    int _int;
    p2_BYTE _byte;
    p2_WORD _word;
    p2_LONG _long;
    p2_QUAD _quad;
    p2_REAL _real;
    p2_LONG _addr[2];
}   p2_Union_t;
Q_DECLARE_METATYPE(p2_Union_t)

//! A typed union member
typedef struct {
    p2_Union_e type;
    p2_Union_t value;
    bool hubmode;
} P2TypedValue;
Q_DECLARE_METATYPE(P2TypedValue)

static constexpr bool p2_cog = false;
static constexpr bool p2_hub = true;


/**
 * @brief The P2MatchMask class is used to handle opcode pattern
 * mask and match pairs of p2_LONG.
 */
class P2MatchMask : public QPair<p2_LONG,p2_LONG>
{
public:
    P2MatchMask(p2_LONG match = 0, p2_LONG mask = 0)
        : QPair<p2_LONG,p2_LONG>(match, ~mask)
    {}
    p2_LONG match() const { return first; }
    p2_LONG mask() const { return ~second; }
    void set_masked_match(const p2_LONG match) { first = match & ~second; }
};

typedef enum {
    tr_none         = 0,            //!< no special trait
    tr_IMMEDIATE    = (1u << 0),    //!< expression started with '#'
    tr_AUGMENTED    = (1u << 1),    //!< expression started with '##'
    tr_HUBADDRESS   = (1u << 2),    //!< expression contained '@'
    tr_ABSOLUTE     = (1u << 3),    //!< expression contained '\'
    tr_INDEX        = (1u << 4),    //!< expression contains an index: PTRA, PTRB, or '[' expr ']'
    tr_DEC          = (1u << 5),    //!< PTRA/PTRB with decrement (--)
    tr_INC          = (1u << 6),    //!< PTRA/PTRB with increment (++)
    tr_PRE          = (1u << 7),    //!< PTRA/PTRB with pre inc/dec (--PTRx or ++PTRx)
    tr_POST         = (1u << 8),    //!< PTRA/PTRB with post inc/dec post (PTRx-- or PTRx++)
}   p2_Traits_e;

//! P2Atom traits
class P2Traits
{
public:
    explicit P2Traits(p2_LONG l = tr_none);
    explicit P2Traits(p2_Traits_e t = tr_none);

    p2_Traits_e traits() const;
    p2_LONG value() const;
    void set(const p2_Traits_e set);
    bool has(const p2_Traits_e trait) const;
    bool has(const p2_LONG trait) const;
    void add(const p2_Traits_e set);
    void add(const p2_LONG set);
    void remove(const p2_Traits_e clear);
    void remove(const p2_LONG clear);
private:
    union {
        p2_LONG _long;
        p2_Traits_e traits;
    }   m;
};


/**
 * @brief Enumeration of the 16 conditional execution modes
 * In opcode patterns these are represented as EEEE (conditional execution)
 * or cccc, zzzz in the MODCZ,MODC,MODZ instructions.
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
}   p2_Cond_e;

/**
 * @brief Define an instruction with 5 bits in the p2_inst5_e enumeration
 */
#define INST5(b4,b3,b2,b1,b0) ((b4<<4)|(b3<<3)|(b2<<2)|(b1<<1)|(b0))

/**
 * @brief Define an instruction with 7 bits in the p2_inst7_e enumeration
 */
#define INST7(b6,b5,b4,b3,b2,b1,b0) ((b6<<6)|(b5<<5)|(b4<<4)|(b3<<3)|(b2<<2)|(b1<<1)|(b0))

/**
 * @brief Define an instruction with 8 bits in the p2_inst8_e enumeration (uses C bit for the extension)
 */
#define INST8(inst,b0)              ((inst<<1)|(b0))

/**
 * @brief Define an instruction with 9 bits in the p2_inst9_e enumeration (uses C and Z bits for the extension)
 */
#define INST9(inst,b1,b0)           ((inst<<2)|(b1<<1)|(b0))

/**
 * @brief Enumeration of the 4 instructions using only the more significant 5 of 7 bits
 */
typedef enum {
    p2_CALLD_ABS                = INST5(1,1,1,0,0),
    p2_LOC                      = INST5(1,1,1,0,1),
    p2_AUGS                     = INST5(1,1,1,1,0),
    p2_AUGD                     = INST5(1,1,1,1,1),
}   p2_INST5_e;

/**
 * @brief Enumeration of the 128 possible instruction types in 7 bit instructions
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
        p2_BITL_W               = p2_TESTB_W_BITL,
    p2_TESTBN_W_BITH            = INST7(0,1,0,0,0,0,1),
        p2_TESTBN_W             = p2_TESTBN_W_BITH,
        p2_BITH_W               = p2_TESTBN_W_BITH,
    p2_TESTB_AND_BITC           = INST7(0,1,0,0,0,1,0),
        p2_TESTB_AND            = p2_TESTB_AND_BITC,
        p2_BITC_W               = p2_TESTB_AND_BITC,
    p2_TESTBN_AND_BITNC         = INST7(0,1,0,0,0,1,1),
        p2_TESTBN_AND           = p2_TESTBN_AND_BITNC,
        p2_BITNC_W              = p2_TESTBN_AND_BITNC,
    p2_TESTB_OR_BITZ            = INST7(0,1,0,0,1,0,0),
        p2_TESTB_OR             = p2_TESTB_OR_BITZ,
        p2_BITZ_W               = p2_TESTB_OR_BITZ,
    p2_TESTBN_OR_BITNZ          = INST7(0,1,0,0,1,0,1),
        p2_TESTBN_OR            = p2_TESTBN_OR_BITNZ,
        p2_BITNZ_W              = p2_TESTBN_OR_BITNZ,
    p2_TESTB_XOR_BITRND         = INST7(0,1,0,0,1,1,0),
        p2_TESTB_XOR            = p2_TESTB_XOR_BITRND,
        p2_BITRND_W             = p2_TESTB_XOR_BITRND,
    p2_TESTBN_XOR_BITNOT        = INST7(0,1,0,0,1,1,1),
        p2_TESTBN_XOR           = p2_TESTBN_XOR_BITNOT,
        p2_BITNOT_W             = p2_TESTBN_XOR_BITNOT,

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

    p2_SETNIB_0_3               = INST7(1,0,0,0,0,0,0),
    p2_SETNIB_4_7               = INST7(1,0,0,0,0,0,1),
    p2_GETNIB_0_3               = INST7(1,0,0,0,0,1,0),
    p2_GETNIB_4_7               = INST7(1,0,0,0,0,1,1),
    p2_ROLNIB_0_3               = INST7(1,0,0,0,1,0,0),
    p2_ROLNIB_4_7               = INST7(1,0,0,0,1,0,1),
    p2_SETBYTE_0_3              = INST7(1,0,0,0,1,1,0),
    p2_GETBYTE_0_3              = INST7(1,0,0,0,1,1,1),

    p2_ROLBYTE_0_3              = INST7(1,0,0,1,0,0,0),
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

    p2_CALLD_ABS_PA             = INST7(1,1,1,0,0,0,0),
    p2_CALLD_ABS_PB             = INST7(1,1,1,0,0,0,1),
    p2_CALLD_ABS_PTRA           = INST7(1,1,1,0,0,1,0),
    p2_CALLD_ABS_PTRB           = INST7(1,1,1,0,0,1,1),

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

}   p2_INST7_e;

Q_STATIC_ASSERT(p2_AUGD_11 == 127u);

/**
 * @brief Enumeration of the p2_OPDST 7 bit instruction's D fields
 */
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
}   p2_OPDST_e;

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
    p2_OPSRC_INVALID            = 0x200
}   p2_OPSRC_e;

/**
 * @brief Enumeration of the p2_OPSRC_X24 instruction's D fields
 */
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

    p2_OPX24_INVALID            = 0x200,
}   p2_OPX24_e;

/**
 * @brief some specific opcodes tested for in the COG emulation
 */
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
}   p2_OPCODE_e;

/**
 * @brief some specific mask and opcodes tested for in the COG emulation
 */
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
}   p2_INSTX1_e;

/**
 * @brief some specific opcodes tested for in the COG emulation
 */
typedef enum {
    p2_INSTR_MASK2              = 0x0fe3e1ff,
    p2_INSTR_WAITXXX            = 0x0d602024,
}   p2_INSTX2_e;

/**
 * @brief Enumeration of the 8 bit instruction types
 * This is a 7 bit instruction with C bit appendix
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

}   p2_INST8_e;

/**
 * @brief Enumeration of the 9 bit instruction types
 * This is a 7 bit instruction with C and Z bits appendix
 */
typedef enum {
    p2_BITL                     = INST9(p2_TESTB_W_BITL,0,0),
    p2_TESTB_WZ                 = INST9(p2_TESTB_W_BITL,0,1),
    p2_TESTB_WC                 = INST9(p2_TESTB_W_BITL,1,0),
    p2_BITL_WCZ                 = INST9(p2_TESTB_W_BITL,1,1),

    p2_BITH                     = INST9(p2_TESTBN_W_BITH,0,0),
    p2_TESTBN_WZ                = INST9(p2_TESTBN_W_BITH,0,1),
    p2_TESTBN_WC                = INST9(p2_TESTBN_W_BITH,1,0),
    p2_BITH_WCZ                 = INST9(p2_TESTBN_W_BITH,1,1),

    p2_BITC                     = INST9(p2_TESTB_AND_BITC,0,0),
    p2_TESTB_ANDZ               = INST9(p2_TESTB_AND_BITC,0,1),
    p2_TESTB_ANDC               = INST9(p2_TESTB_AND_BITC,1,0),
    p2_BITC_WCZ                 = INST9(p2_TESTB_AND_BITC,1,1),

    p2_BITNC                    = INST9(p2_TESTBN_AND_BITNC,0,0),
    p2_TESTBN_ANDZ              = INST9(p2_TESTBN_AND_BITNC,0,1),
    p2_TESTBN_ANDC              = INST9(p2_TESTBN_AND_BITNC,1,0),
    p2_BITNC_WCZ                = INST9(p2_TESTBN_AND_BITNC,1,1),

    p2_BITZ                     = INST9(p2_TESTB_OR_BITZ,0,0),
    p2_TESTB_ORZ                = INST9(p2_TESTB_OR_BITZ,0,1),
    p2_TESTB_ORC                = INST9(p2_TESTB_OR_BITZ,1,0),
    p2_BITZ_WCZ                 = INST9(p2_TESTB_OR_BITZ,1,1),

    p2_BITNZ                    = INST9(p2_TESTBN_OR_BITNZ,0,0),
    p2_TESTBN_ORZ               = INST9(p2_TESTBN_OR_BITNZ,0,1),
    p2_TESTBN_ORC               = INST9(p2_TESTBN_OR_BITNZ,1,0),
    p2_BITNZ_WCZ                = INST9(p2_TESTBN_OR_BITNZ,1,1),

    p2_BITRND                   = INST9(p2_TESTB_XOR_BITRND,0,0),
    p2_TESTB_XORZ               = INST9(p2_TESTB_XOR_BITRND,0,1),
    p2_TESTB_XORC               = INST9(p2_TESTB_XOR_BITRND,1,0),
    p2_BITRND_WCZ               = INST9(p2_TESTB_XOR_BITRND,1,1),

    p2_BITNOT                   = INST9(p2_TESTBN_XOR_BITNOT,0,0),
    p2_TESTBN_XORZ              = INST9(p2_TESTBN_XOR_BITNOT,0,1),
    p2_TESTBN_XORC              = INST9(p2_TESTBN_XOR_BITNOT,1,0),
    p2_BITNOT_WCZ               = INST9(p2_TESTBN_XOR_BITNOT,1,1),

    p2_DIRL                     = INST9(p2_OPSRC,0,0),
    p2_TESTP_WZ                 = INST9(p2_OPSRC,0,1),
    p2_TESTP_WC                 = INST9(p2_OPSRC,1,0),
    p2_DIRL_WCZ                 = INST9(p2_OPSRC,1,1),

    p2_DIRH                     = INST9(p2_OPSRC,0,0),
    p2_TESTPN_WZ                = INST9(p2_OPSRC,0,1),
    p2_TESTPN_WC                = INST9(p2_OPSRC,1,0),
    p2_DIRH_WCZ                 = INST9(p2_OPSRC,1,1),

    p2_DIRC                     = INST9(p2_OPSRC,0,0),
    p2_TESTP_ANDZ               = INST9(p2_OPSRC,0,1),
    p2_TESTP_ANDC               = INST9(p2_OPSRC,1,0),
    p2_DIRC_WCZ                 = INST9(p2_OPSRC,1,1),

    p2_DIRNC                    = INST9(p2_OPSRC,0,0),
    p2_TESTPN_ANDZ              = INST9(p2_OPSRC,0,1),
    p2_TESTPN_ANDC              = INST9(p2_OPSRC,1,0),
    p2_DIRNC_WCZ                = INST9(p2_OPSRC,1,1),

    p2_DIRZ                     = INST9(p2_OPSRC,0,0),
    p2_TESTP_ORZ                = INST9(p2_OPSRC,0,1),
    p2_TESTP_ORC                = INST9(p2_OPSRC,1,0),
    p2_DIRZ_WCZ                 = INST9(p2_OPSRC,1,1),

    p2_DIRNZ                    = INST9(p2_OPSRC,0,0),
    p2_TESTPN_ORZ               = INST9(p2_OPSRC,0,1),
    p2_TESTPN_ORC               = INST9(p2_OPSRC,1,0),
    p2_DIRNZ_WCZ                = INST9(p2_OPSRC,1,1),

    p2_DIRRND                   = INST9(p2_OPSRC,0,0),
    p2_TESTP_XORZ               = INST9(p2_OPSRC,0,1),
    p2_TESTP_XORC               = INST9(p2_OPSRC,1,0),
    p2_DIRRND_WCZ               = INST9(p2_OPSRC,1,1),

    p2_DIRNOT                   = INST9(p2_OPSRC,0,0),
    p2_TESTPN_XORZ              = INST9(p2_OPSRC,0,1),
    p2_TESTPN_XORC              = INST9(p2_OPSRC,1,0),
    p2_DIRNOT_WCZ               = INST9(p2_OPSRC,1,1),

    p2_SETNIB_0                 = INST9(p2_SETNIB_0_3,0,0),
    p2_SETNIB_1                 = INST9(p2_SETNIB_0_3,0,1),
    p2_SETNIB_2                 = INST9(p2_SETNIB_0_3,1,0),
    p2_SETNIB_3                 = INST9(p2_SETNIB_0_3,1,1),
    p2_SETNIB_4                 = INST9(p2_SETNIB_4_7,0,0),
    p2_SETNIB_5                 = INST9(p2_SETNIB_4_7,0,1),
    p2_SETNIB_6                 = INST9(p2_SETNIB_4_7,1,0),
    p2_SETNIB_7                 = INST9(p2_SETNIB_4_7,1,1),

    p2_GETNIB_0                 = INST9(p2_GETNIB_0_3,0,0),
    p2_GETNIB_1                 = INST9(p2_GETNIB_0_3,0,1),
    p2_GETNIB_2                 = INST9(p2_GETNIB_0_3,1,0),
    p2_GETNIB_3                 = INST9(p2_GETNIB_0_3,1,1),
    p2_GETNIB_4                 = INST9(p2_GETNIB_4_7,0,0),
    p2_GETNIB_5                 = INST9(p2_GETNIB_4_7,0,1),
    p2_GETNIB_6                 = INST9(p2_GETNIB_4_7,1,0),
    p2_GETNIB_7                 = INST9(p2_GETNIB_4_7,1,1),

    p2_ROLNIB_0                 = INST9(p2_ROLNIB_0_3,0,0),
    p2_ROLNIB_1                 = INST9(p2_ROLNIB_0_3,0,1),
    p2_ROLNIB_2                 = INST9(p2_ROLNIB_0_3,1,0),
    p2_ROLNIB_3                 = INST9(p2_ROLNIB_0_3,1,1),
    p2_ROLNIB_4                 = INST9(p2_ROLNIB_4_7,0,0),
    p2_ROLNIB_5                 = INST9(p2_ROLNIB_4_7,0,1),
    p2_ROLNIB_6                 = INST9(p2_ROLNIB_4_7,1,0),
    p2_ROLNIB_7                 = INST9(p2_ROLNIB_4_7,1,1),

    p2_SETBYTE_0                = INST9(p2_SETBYTE_0_3,0,0),
    p2_SETBYTE_1                = INST9(p2_SETBYTE_0_3,0,1),
    p2_SETBYTE_2                = INST9(p2_SETBYTE_0_3,1,0),
    p2_SETBYTE_3                = INST9(p2_SETBYTE_0_3,1,1),

    p2_GETBYTE_0                = INST9(p2_GETBYTE_0_3,0,0),
    p2_GETBYTE_1                = INST9(p2_GETBYTE_0_3,0,1),
    p2_GETBYTE_2                = INST9(p2_GETBYTE_0_3,1,0),
    p2_GETBYTE_3                = INST9(p2_GETBYTE_0_3,1,1),

    p2_ROLBYTE_0                = INST9(p2_ROLBYTE_0_3,0,0),
    p2_ROLBYTE_1                = INST9(p2_ROLBYTE_0_3,0,1),
    p2_ROLBYTE_2                = INST9(p2_ROLBYTE_0_3,1,0),
    p2_ROLBYTE_3                = INST9(p2_ROLBYTE_0_3,1,1),

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

}   p2_INST9_e;

/**
 * @brief Structure of the Propeller2 opcode words with 5 bits instruction, WW, wc, wz, and im
 *
 * Note: To have the bit fields work for both, little
 * and big endian machines, the order has to match
 * the native order of bits in 32 bit words of the target.
 */
typedef struct {
#if (Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
    uint address:22;            //!< 22 bits of address
    bool rel:1;                 //!< R if true, relative to PC, otherwise absolute
    uint inst:5;                //!< instruction type
    uint cond:4;                //!< conditional execution
#elif (Q_BYTE_ORDER == Q_BIG_ENDIAN)
    uint cond:4;                //!< conditional execution
    uint inst:5;                //!< instruction type
    bool rel:1;                 //!< R if true, relative to PC, otherwise absolute
    uint address:22;            //!< 22 bits of address
#else
#error "Unknown byte order!"
#endif
}   p2_Opcode5_t;

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
}   p2_Opcode7_t;

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
}   p2_Opcode8_t;

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
}   p2_Opcode9_t;

/**
 * @brief Union of the opcode and the variants with 7, 8, and 9 bit instructions
 */
typedef union {
    p2_LONG opcode;             //!< opcode as 32 bit word
    p2_Opcode5_t op5;           //!< ocpode as bit fields (version with 5 bits instruction)
    p2_Opcode7_t op7;           //!< ocpode as bit fields (version with 7 bits instruction)
    p2_Opcode8_t op8;           //!< ocpode as bit fields (version including WC)
    p2_Opcode9_t op9;           //!< ocpode as bit fields (version including WC and WZ)
}   p2_opcode_u;


/**
 * @brief Structure of the 9 bit non-augmented index
 *<pre>
 * This structure defines the 9 bits of S/#/PTRx without AUGS
 * 0:   %0AAAAAAAA
 * 1:   %1SUPNNNNN
 * The topmost bit (%sup) determines whether the
 * lower 8 bits are to be interpreted as:
 * 0:   AAAAAAAA: 8 bits of address (0…255)
 * 1:   SUPNNNNN: structure definining the
 *      + S: PTRx false for PTRA, true for PTRB
 *      + U: false to keep PTRx same, true to update PTRx (PTRx += INDEX*SCALE)
 *      + P: false to use PTRx + INDEX*SCALE, true to use PTRx (post-modify)
 *      + NNNNN: index -16…15 for simple offsets, 0…15 for ++'s, 0…16 for --'s
 * SCALE is:
 *      = 1: for RDBYTE/WRBYTE
 *      = 2: for RDWORD/WRWORD
 *      = 4: for RDLONG/WRLONG/WMLONG
 *</pre>
 */
typedef union {
    uint        src:32;             //!< 9 lower bits of index as unsigned int
#if (Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
    struct {
        char    N:5;                //!< NNNNN: 5 bits of index
        bool    P:1;                //!< P: false to use PTRx + INDEX*SCALE, true to use PTRx (post-modify)
        bool    U:1;                //!< U: false to keep PTRx same, true to update PTRx (PTRx += INDEX*SCALE)
        bool    S:1;                //!< S: false for PTRA, true for PTRB
        bool    sup:1;              //!< 1: use 8 bits as follows
        uint    unused:23;          //!< 23 bits unused
    }   i;                          //!< index
    struct {
        uchar   A:8;                //!< AAAAAAAA: 8 bits of address
        bool    sup:1;              //!< 0: use 8 bits as follows
        uint    unused:23;          //!< 23 bits unused
    }   a;                          //!< address
#elif (Q_BYTE_ORDER == Q_BIG_ENDIAN)
    struct {
        uint    unused:23;          //!< 23 bits unused
        bool    sup:1;              //!< 1: use 8 bits as follows
        bool    S:1;                //!< S: false for PTRA, true for PTRB
        bool    U:1;                //!< U: false to keep PTRx same, true to update PTRx (PTRx += INDEX*SCALE)
        bool    P:1;                //!< P: false to use PTRx + INDEX*SCALE, true to use PTRx (post-modify)
        char    N:5;                //!< NNNNN: 5 bits of index
    }   i;                          //!< index
    struct {
        uint    unused:23;          //!< 23 bits unused
        bool    sup:1;              //!< 0: use 8 bits as follows
        uchar   A:8;                //!< AAAAAAAA: 8 bits of address
    }   a;                          //!< address
#else
#error "Unknown byte order!"
#endif
}   p2_Index9_t;

/**
 * @brief Structure of the 23 bit augmented (AUGS) index
 *<pre>
 * This structure defines the 23 bits of S/#/PTRx with AUGS
 * 0:   %000000000000AAAAAAAAAAA_AAAAAAAAA
 * 1:   %000000001SUPNNNNNNNNNNN_NNNNNNNNN
 * The topmost bit (sup) determines whether the
 * less significant 23 bits are to be interpreted as:
 * 0:   AAAAAAAAAAA_AAAAAAAAA: structure defining the
 *      + A: 20 bit of address ($000000…$fffff)
 * 1:   SUPNNNNNNNNNNN_NNNNNNNNN: structure definining the
 *      + S: PTRx false for PTRA, true for PTRB
 *      + U: false to keep PTRx same, true to update PTRx (PTRx += INDEX)
 *      + P: false to use PTRx + INDEX, true to use PTRx (post-modify)
 *      + NNNNNNNNNNN_NNNNNNNNN: 20 bit unscaled index
 *</pre>
 */
typedef union {
    uint        aug:32;             //!< 32 bits of index as unsigned int
#if (Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
    struct {
        int     N:20;               //!< NNNNNNNNNNN_NNNNNNNNN: 20 bits of index
        bool    P:1;                //!< P: false to use PTRx + INDEX*SCALE, true to use PTRx (post-modify)
        bool    U:1;                //!< U: false to keep PTRx same, true to update PTRx (PTRx += INDEX*SCALE)
        bool    S:1;                //!< S: false for PTRA, true for PTRB
        bool    sup:1;              //!< 1: use 23 bits as defined below
        uchar   unused_msb:8;       //!< 00000000: 8 unused most significant bits
    }   i;                          //!< index
    struct {
        uint    A:20;               //!< AAAAAAAAAAA_AAAAAAAAA: 20 bits of address
        bool    unused_nul:3;       //!< 000: more unused bits (000)
        bool    sup:1;              //!< 0: use 23 bits as defined below
        uchar   unused_msb:8;       //!< 00000000: 8 unused most significant bits
    }   a;                          //!< address
#elif (Q_BYTE_ORDER == Q_BIG_ENDIAN)
    struct {
        uchar   unused_msb:8;       //!< 00000000: 8 unused most significant bits
        bool    sup:1;              //!< 1: use 23 bits as defined above
        bool    S:1;                //!< S: false for PTRA, true for PTRB
        bool    U:1;                //!< U: false to keep PTRx same, true to update PTRx (PTRx += INDEX*SCALE)
        bool    P:1;                //!< P: false to use PTRx + INDEX*SCALE, true to use PTRx (post-modify)
        int     N:20;               //!< NNNNNNNNNNN_NNNNNNNNN: 20 bits of index
    }   i;                          //!< index
    struct {
        uchar   unused_msb:8;       //!< 00000000: 8 unused most significant bits
        bool    sup:1;              //!< 0: use 23 bits as defined above
        bool    unused_nul:3;       //!< 000: unused (000)
        uint    A:20;               //!< AAAAAAAAAAA_AAAAAAAAA: 20 bits of address
    }   a;                          //!< address
#else
#error "Unknown byte order!"
#endif
}   p2_Index23_t;

/**
 * @brief Structure of the COG and the shadow registers in the last 16 LONGs
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
}   p2_COGRegs_t;

/**
 * @brief Offsets of the COG shadow registers
 */
static constexpr p2_LONG offs_IJMP3 = 0x1f0;                    //!< offset of interrupt call address for INT3
static constexpr p2_LONG offs_IRET3 = 0x1f1;                    //!< offset of interrupt return address for INT3
static constexpr p2_LONG offs_IJMP2 = 0x1f2;                    //!< offset of interrupt call address for INT2
static constexpr p2_LONG offs_IRET2 = 0x1f3;                    //!< offset of interrupt return address for INT2
static constexpr p2_LONG offs_IJMP1 = 0x1f4;                    //!< offset of interrupt call address for INT1
static constexpr p2_LONG offs_IRET1 = 0x1f5;                    //!< offset of interrupt return address for INT1
static constexpr p2_LONG offs_PA    = 0x1f6;                    //!< offset of CALLD-imm return, CALLPA parameter, or LOC address
static constexpr p2_LONG offs_PB    = 0x1f7;                    //!< offset of CALLD-imm return, CALLPB parameter, or LOC address
static constexpr p2_LONG offs_PTRA  = 0x1f8;                    //!< offset of pointer A to hub RAM
static constexpr p2_LONG offs_PTRB  = 0x1f9;                    //!< offset of pointer B to hub RAM
static constexpr p2_LONG offs_DIRA  = 0x1fa;                    //!< offset of output enables for P31 ... P0
static constexpr p2_LONG offs_DIRB  = 0x1fb;                    //!< offset of output enables for P63 ... P32
static constexpr p2_LONG offs_OUTA  = 0x1fc;                    //!< offset of output states for P31 ... P0
static constexpr p2_LONG offs_OUTB  = 0x1fd;                    //!< offset of output states for P63 ... P32
static constexpr p2_LONG offs_INA   = 0x1fe;                    //!< offset of input states for P31 ... P0
static constexpr p2_LONG offs_INB   = 0x1ff;                    //!< offset of input states for P63 ... P32

/**
 * @brief Addresses of the COG shadow registers
 */
static constexpr p2_LONG addr_IJMP3 = sz_LONG * offs_IJMP3;     //!< address of interrupt call address for INT3
static constexpr p2_LONG addr_IRET3 = sz_LONG * offs_IRET3;     //!< address of interrupt return address for INT3
static constexpr p2_LONG addr_IJMP2 = sz_LONG * offs_IJMP2;     //!< address of interrupt call address for INT2
static constexpr p2_LONG addr_IRET2 = sz_LONG * offs_IRET2;     //!< address of interrupt return address for INT2
static constexpr p2_LONG addr_IJMP1 = sz_LONG * offs_IJMP1;     //!< address of interrupt call address for INT1
static constexpr p2_LONG addr_IRET1 = sz_LONG * offs_IRET1;     //!< address of interrupt return address for INT1
static constexpr p2_LONG addr_PA    = sz_LONG * offs_PA;        //!< address of CALLD-imm return, CALLPA parameter, or LOC address
static constexpr p2_LONG addr_PB    = sz_LONG * offs_PB;        //!< address of CALLD-imm return, CALLPB parameter, or LOC address
static constexpr p2_LONG addr_PTRA  = sz_LONG * offs_PTRA;      //!< address of pointer A to hub RAM
static constexpr p2_LONG addr_PTRB  = sz_LONG * offs_PTRB;      //!< address of pointer B to hub RAM
static constexpr p2_LONG addr_DIRA  = sz_LONG * offs_DIRA;      //!< address of output enables for P31 ... P0
static constexpr p2_LONG addr_DIRB  = sz_LONG * offs_DIRB;      //!< address of output enables for P63 ... P32
static constexpr p2_LONG addr_OUTA  = sz_LONG * offs_OUTA;      //!< address of output states for P31 ... P0
static constexpr p2_LONG addr_OUTB  = sz_LONG * offs_OUTB;      //!< address of output states for P63 ... P32
static constexpr p2_LONG addr_INA   = sz_LONG * offs_INA;       //!< address of input states for P31 ... P0
static constexpr p2_LONG addr_INB   = sz_LONG * offs_INB;       //!< address of input states for P63 ... P32

Q_STATIC_ASSERT(offs_INB == 0x1ff);

/**
 * @brief Union of the COG memory and shadow registers
 */
typedef union {
    p2_LONG RAM[512];
    p2_COGRegs_t REG;
}   p2_COG_t;

/**
 * @brief Structure of the LUT memory
 */
typedef struct {
    p2_LONG RAM[512];
}   p2_LUT_t;

/**
 * @brief The 16 flag bits (interrupt sources) per COG
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
}   p2_FLAGS_t;

/**
 * @brief PAT pattern matching mode enum
 */
typedef enum {
    p2_PAT_NONE,                //!< no pattern matching
    p2_PAT_PA_EQ,               //!< match if (PA & mask) == match
    p2_PAT_PA_NE,               //!< match if (PA & mask) != match
    p2_PAT_PB_EQ,               //!< match if (PB & mask) == match
    p2_PAT_PB_NE                //!< match if (PB & mask) != match
}   p2_PAT_mode_e;

/**
 * @brief PAT pattern matching data
 */
typedef struct {
    p2_PAT_mode_e mode;         //!< pattern matching mode
    p2_LONG mask;               //!< mask value
    p2_LONG match;              //!< match value
}   p2_PAT_t;

/**
 * @brief PIN edge mode enum
 */
typedef enum {
    p2_PIN_NONE,                //!< no pin edge
    p2_PIN_CHANGED_LO,          //!< match if pin changed to lo
    p2_PIN_CHANGED_HI,          //!< match if pin changed to hi
    p2_PIN_CHANGED              //!< match if pin changed
}   p2_PIN_mode_e;

/**
 * @brief PIN pin edge data
 */
typedef struct {
    p2_PIN_mode_e mode;         //!< pin edge mode
    p2_LONG edge;               //!< pin edge type
    p2_LONG num;                //!< pin number
}   p2_PIN_t;

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
}   p2_INT_flags_t;

/**
 * @brief union of INT flags and bits
 */
typedef union {
    p2_LONG bits;               //!< interrupt flags as LONG
    p2_INT_flags_t flags;       //!< interrupt flags as bit masks
}   p2_INT_bits_u;

/**
 * @brief LOCK edge mode enum
 */
typedef enum {
    p2_LOCK_NONE,               //!< no LOCK edge
    p2_LOCK_CHANGED_LO,         //!< match if LOCK changed to lo
    p2_LOCK_CHANGED_HI,         //!< match if LOCK changed to hi
    p2_LOCK_CHANGED             //!< match if LOCK changed
}   p2_LOCK_mode_e;

/**
 * @brief LOCK edge data
 */
typedef struct {
    p2_LONG num:4;              //!< LOCK COG id
    p2_LOCK_mode_e mode:2;      //!< LOCK edge mode
    p2_LONG edge:2;             //!< LOCK edge
    p2_LONG prev:1;             //!< LOCK previous state
}   p2_LOCK_t;

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
}   p2_WAIT_mode_e;

/**
 * @brief WAIT wait status
 */
typedef struct {
    p2_LONG flag;               //!< non-zero if waiting
    p2_WAIT_mode_e mode;        //!< current wait mode
}   p2_WAIT_t;

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
}   p2_FIFO_t;

/**
 * @brief Ummm.. not yet used. Meant to put in a structure what is in the instruction queue
 */
typedef struct {
    p2_opcode_u IR;
    p2_LONG R;
    p2_LONG D;
    p2_LONG S;
}   p2_QUEUE_t;

/**
 * @brief A globally used enumeration of display formats
 */
typedef enum {
    fmt_bin,                    //!< display as binary
    fmt_bit,                    //!< display as "byt" (that is hex fragments of about 8 bits, sometimes less, sometimes more)
    fmt_dec,                    //!< display as decimal (who needs this?)
    fmt_hex,                    //!< display as hexadecimal
    fmt_doc,                    //!< display as excerpt of the P2Doc information
}   p2_FORMAT_e;

extern const QString template_str_hubaddr;
extern const QString template_str_cogaddr;
extern const QString template_str_opcode_bin;
extern const QString template_str_opcode_byt;
extern const QString template_str_opcode_oct;
extern const QString template_str_opcode_dec;
extern const QString template_str_opcode_hex;
extern const QString template_str_opcode_doc;
extern const QString template_str_tokens;
extern const QString template_str_symbols;
extern const QString template_str_errors;
extern const QString template_str_instruction;
extern const QString template_str_description;
extern const QString key_tv_asm;

/**
 * @brief The enumeration of token types
 *
 * This is used to classify tokens into groups which can be tested for
 * easily using bit masks in a 64 bit mask value
 *
 */
typedef enum {
    tt_none,            //!< no specific type
    tt_comment,         //!< comment
    tt_parens,          //!< precedence  0: parenthesis "(", ")"
    tt_brackets,        //!< precedence  0: brackets "[", "]"
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
}   p2_TOKTYPE_e;

/**
 * @brief The type for token type bitmasks
 * Since there are more than 32 types we need to use a 64 bit quantity for the mask
 */
typedef quint64 p2_TOKMASK_t;

/**
 * @brief Constants for the bit masks of token types
 */
#define TTMASK(tt) static_cast<p2_TOKMASK_t>(Q_UINT64_C(1) << (tt))

static constexpr p2_TOKMASK_t tm_none        = 0;
static constexpr p2_TOKMASK_t tm_comment     = TTMASK(tt_comment);
static constexpr p2_TOKMASK_t tm_parens      = TTMASK(tt_parens);
static constexpr p2_TOKMASK_t tm_brackets    = TTMASK(tt_brackets);
static constexpr p2_TOKMASK_t tm_primary     = TTMASK(tt_primary);
static constexpr p2_TOKMASK_t tm_unary       = TTMASK(tt_unary);
static constexpr p2_TOKMASK_t tm_shiftop     = TTMASK(tt_shiftop);
static constexpr p2_TOKMASK_t tm_binop_rev   = TTMASK(tt_binop_rev);
static constexpr p2_TOKMASK_t tm_mulop       = TTMASK(tt_mulop);
static constexpr p2_TOKMASK_t tm_addop       = TTMASK(tt_addop);
static constexpr p2_TOKMASK_t tm_binop_and   = TTMASK(tt_binop_and);
static constexpr p2_TOKMASK_t tm_binop_xor   = TTMASK(tt_binop_xor);
static constexpr p2_TOKMASK_t tm_binop_or    = TTMASK(tt_binop_or);
static constexpr p2_TOKMASK_t tm_binop_encod = TTMASK(tt_binop_encod);
static constexpr p2_TOKMASK_t tm_binop_decod = TTMASK(tt_binop_decod);
static constexpr p2_TOKMASK_t tm_relation    = TTMASK(tt_relation);
static constexpr p2_TOKMASK_t tm_equality    = TTMASK(tt_equality);
static constexpr p2_TOKMASK_t tm_logop_and   = TTMASK(tt_logop_and);
static constexpr p2_TOKMASK_t tm_logop_or    = TTMASK(tt_logop_or);
static constexpr p2_TOKMASK_t tm_ternary     = TTMASK(tt_ternary);
static constexpr p2_TOKMASK_t tm_assignment  = TTMASK(tt_assignment);
static constexpr p2_TOKMASK_t tm_delimiter   = TTMASK(tt_delimiter);
static constexpr p2_TOKMASK_t tm_constant    = TTMASK(tt_constant);
static constexpr p2_TOKMASK_t tm_function    = TTMASK(tt_function);
static constexpr p2_TOKMASK_t tm_traits      = TTMASK(tt_traits);
static constexpr p2_TOKMASK_t tm_conditional = TTMASK(tt_conditional);
static constexpr p2_TOKMASK_t tm_modcz_param = TTMASK(tt_modcz_param);
static constexpr p2_TOKMASK_t tm_mnemonic    = TTMASK(tt_mnemonic);
static constexpr p2_TOKMASK_t tm_wcz_suffix  = TTMASK(tt_wcz_suffix);
static constexpr p2_TOKMASK_t tm_section     = TTMASK(tt_section);
static constexpr p2_TOKMASK_t tm_origin      = TTMASK(tt_origin);
static constexpr p2_TOKMASK_t tm_data        = TTMASK(tt_data);
static constexpr p2_TOKMASK_t tm_lexer       = TTMASK(tt_regexp);

static constexpr p2_TOKMASK_t tm_primary_unary =
        tm_primary |
        tm_unary;

static constexpr p2_TOKMASK_t tm_binop =
        tm_binop_and |
        tm_binop_xor |
        tm_binop_or |
        tm_binop_rev |
        tm_binop_encod |
        tm_binop_decod;

static constexpr p2_TOKMASK_t tm_operations =
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

static constexpr p2_TOKMASK_t tm_expression =
        tm_function |
        tm_operations |
        tm_constant;

inline constexpr p2_LONG hub2cog(const p2_LONG hub) {
    return static_cast<p2_LONG>(static_cast<int>(hub) / sz_LONG);
}
inline constexpr p2_LONG cog2hub(const p2_LONG cog) {
    return cog * sz_LONG;
}
