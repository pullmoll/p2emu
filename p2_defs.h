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
    cond_never              //!< actually execute always (?)
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
    bool uz:1;                  //!< update Z flag
    bool uc:1;                  //!< update C flag
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
