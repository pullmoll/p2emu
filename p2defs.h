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

typedef quint8 P2BYTE;
typedef quint16 P2WORD;
typedef quint32 P2LONG;

typedef union {
#if (Q_BYTE_ORDER == Q_LITTLE_ENDIAN)
    P2BYTE b0,b1,b2,b3;
    P2BYTE b[4];
    P2WORD w0, w1;
    P2WORD w[2];
    P2LONG l;
#elif (Q_BYTE_ORDER == Q_BIG_ENDIAN)
    P2BYTE b3,b2,b1,b0;
    P2BYTE b[4];
    P2WORD w1, w0;
    P2WORD w[2];
    P2LONG l;
#else
#error "Unknown byte order!"
#endif
}   p2_BWL;

/**
 * @brief Enumeration of the 16 conditional execution modes
 */
typedef enum {
    cond__ret_,                 //!< execute always; if no branch is taken, return
    cond_nc_and_nz,             //!< execute if C == 0 and Z == 0
    cond_nc_and_z,              //!< execute if C == 0 and Z == 1
    cond_nc,                    //!< execute if C == 0
    cond_c_and_nz,              //!< execute if C == 1 and Z == 0
    cond_nz,                    //!< execute if Z == 0
    cond_c_ne_z,                //!< execute if C != Z
    cond_nc_or_nz,              //!< execute if C == 0 or Z == 0
    cond_c_and_z,               //!< execute if C == 1 and Z == 1
    cond_c_eq_z,                //!< execute if C == Z
    cond_z,                     //!< execute if Z == 1
    cond_nc_or_z,               //!< execute if C == 0 or Z == 1
    cond_c,                     //!< execute if C == 1
    cond_c_or_nz,               //!< execute if C == 1 or Z == 0
    cond_c_or_z,                //!< execute if C == 1 or Z == 1
    cond_always                 //!< execute always (default)
}   p2_cond_e;

/**
 * @brief Enumeration of the 128 possible instruction types
 *
 * NB: Some instructions use more bits from the opcode word,
 * so there are extra bit masks and patterns defined besides
 * the 7 bit opcodes.
 *
 */
typedef enum {
    p2_ROR,                     //!< 0000000
    p2_ROL,                     //!< 0000001
    p2_SHR,                     //!< 0000010
    p2_SHL,                     //!< 0000011
    p2_RCR,                     //!< 0000100
    p2_RCL,                     //!< 0000101
    p2_SAR,                     //!< 0000110
    p2_SAL,                     //!< 0000111

    p2_ADD,                     //!< 0001000
    p2_ADDX,                    //!< 0001001
    p2_ADDS,                    //!< 0001010
    p2_ADDSX,                   //!< 0001011
    p2_SUB,                     //!< 0001100
    p2_SUBX,                    //!< 0001101
    p2_SUBS,                    //!< 0001110
    p2_SUBSX,                   //!< 0001111

    p2_CMP,                     //!< 0010000
    p2_CMPX,                    //!< 0010001
    p2_CMPS,                    //!< 0010010
    p2_CMPSX,                   //!< 0010011
    p2_CMPR,                    //!< 0010100
    p2_CMPM,                    //!< 0010101
    p2_SUBR,                    //!< 0010110
    p2_CMPSUB,                  //!< 0010111

    p2_FGE,                     //!< 0011000
    p2_FLE,                     //!< 0011001
    p2_FGES,                    //!< 0011010
    p2_FLES,                    //!< 0011011
    p2_SUMC,                    //!< 0011100
    p2_SUMNC,                   //!< 0011101
    p2_SUMZ,                    //!< 0011110
    p2_SUMNZ,                   //!< 0011111

    p2_TESTB_W_BITL,            //!< 0100000
    p2_TESTBN_W_BITH,           //!< 0100001
    p2_TESTB_AND_BITC,          //!< 0100010
    p2_TESTBN_AND_BITNC,        //!< 0100011
    p2_TESTB_OR_BITZ,           //!< 0100100
    p2_TESTBN_OR_BITNZ,         //!< 0100101
    p2_TESTB_XOR_BITRND,        //!< 0100110
    p2_TESTBN_XOR_BITNOT,       //!< 0100111

    p2_AND,                     //!< 0101000
    p2_ANDN,                    //!< 0101001
    p2_OR,                      //!< 0101010
    p2_XOR,                     //!< 0101011
    p2_MUXC,                    //!< 0101100
    p2_MUXNC,                   //!< 0101101
    p2_MUXZ,                    //!< 0101110
    p2_MUXNZ,                   //!< 0101111

    p2_MOV,                     //!< 0110000
    p2_NOT,                     //!< 0110001
    p2_ABS,                     //!< 0110010
    p2_NEG,                     //!< 0110011
    p2_NEGC,                    //!< 0110100
    p2_NEGNC,                   //!< 0110101
    p2_NEGZ,                    //!< 0110110
    p2_NEGNZ,                   //!< 0110111

    p2_INCMOD,                  //!< 0111000
    p2_DECMOD,                  //!< 0111001
    p2_ZEROX,                   //!< 0111010
    p2_SIGNX,                   //!< 0111011
    p2_ENCOD,                   //!< 0111100
    p2_ONES,                    //!< 0111101
    p2_TEST,                    //!< 0111110
    p2_TESTN,                   //!< 0111111

    p2_SETNIB_0,                //!< 1000000
    p2_SETNIB_1,                //!< 1000001
    p2_GETNIB_0,                //!< 1000010
    p2_GETNIB_1,                //!< 1000011
    p2_ROLNIB_0,                //!< 1000100
    p2_ROLNIB_1,                //!< 1000101
    p2_SETBYTE,                 //!< 1000110
    p2_GETBYTE,                 //!< 1000111

    p2_ROLBYTE,                 //!< 1001000
    p2_SETWORD_GETWORD,         //!< 1001001
    p2_1001010,                 //!< 1001010
    p2_1001011,                 //!< 1001011
    p2_1001100,                 //!< 1001100
    p2_1001101,                 //!< 1001101
    p2_1001110,                 //!< 1001110
    p2_MUXXXX,                  //!< 1001111

    p2_MUL_MULS,                //!< 1010000
    p2_SCA_SCAS,                //!< 1010001
    p2_1010010,                 //!< 1010010
    p2_WMLONG_ADDCTx,           //!< 1010011
    p2_RQPIND_RDPIN,            //!< 1010100
    p2_RDLUT,                   //!< 1010101
    p2_RDBYTE,                  //!< 1010110
    p2_RDWORD,                  //!< 1010111

    p2_RDLONG,                  //!< 1011000
    p2_CALLD,                   //!< 1011001
    p2_CALLP,                   //!< 1011010
    p2_1011011,                 //!< 1011011
    p2_1011100,                 //!< 1011100
    p2_1011101,                 //!< 1011101
    p2_1011110,                 //!< 1011110
    p2_1011111,                 //!< 1011111

    p2_1100000,                 //!< 1100000
    p2_1100001,                 //!< 1100001
    p2_1100010,                 //!< 1100010
    p2_1100011,                 //!< 1100011
    p2_1100100,                 //!< 1100100
    p2_1100101,                 //!< 1100101
    p2_1100110,                 //!< 1100110
    p2_COGINIT,                 //!< 1100111

    p2_1101000,                 //!< 1101000
    p2_1101001,                 //!< 1101001
    p2_1101010,                 //!< 1101010
    p2_1101011,                 //!< 1101011

    p2_JMP_ABS,                 //!< 1101100
    p2_CALL_ABS,                //!< 1101101
    p2_CALLA_ABS,               //!< 1101110
    p2_CALLB_ABS,               //!< 1101111

    p2_CALLD_PA_ABS,            //!< 1110000
    p2_CALLD_PB_ABS,            //!< 1110001
    p2_CALLD_PTRA_ABS,          //!< 1110010
    p2_CALLD_PTRB_ABS,          //!< 1110011

    p2_LOC_PA,                  //!< 1110100
    p2_LOC_PB,                  //!< 1110101
    p2_LOC_PTRA,                //!< 1110110
    p2_LOC_PTRB,                //!< 1110111

    p2_AUGS_00,                 //!< 1111000
    p2_AUGS_01,                 //!< 1111001
    p2_AUGS_10,                 //!< 1111010
    p2_AUGS_11,                 //!< 1111011

    p2_AUGD_00,                 //!< 1111100
    p2_AUGD_01,                 //!< 1111101
    p2_AUGD_10,                 //!< 1111110
    p2_AUDG_11,                 //!< 1111111

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
    p2_OPSRC_TESTPN_W_DIRH      = 0x41,
    p2_OPSRC_TESTP_AND_DIRC     = 0x42,
    p2_OPSRC_TESTPN_AND_DIRNC   = 0x43,
    p2_OPSRC_TESTP_OR_DIRZ      = 0x44,
    p2_OPSRC_TESTPN_OR_DIRNZ    = 0x45,
    p2_OPSRC_TESTP_XOR_DIRRND   = 0x46,
    p2_OPSRC_TESTPN_XOR_DIRNOT  = 0x47,
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
    p2_INSTR_MASK2              = 0x0fe3e1ff,
    p2_INSTR_WAITXXX            = 0x0d602024,

}   p2_inst_e;

Q_STATIC_ASSERT(p2_AUDG_11 == 127);

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
    unsigned inst:9;            //!< instruction type including WC and WZ
    unsigned cond:4;            //!< conditional execution
#elif (Q_BYTE_ORDER == Q_BIG_ENDIAN)
    unsigned cond:4;            //!< conditional execution
    unsigned inst:9;            //!< instruction type including WC and WZ
    bool imm:1;                 //!< immediate flag
    unsigned dst:9;             //!< destination (D or #D)
    unsigned src:9;             //!< source (S or #S)
#else
#error "Unknown byte order!"
#endif
}   p2_opcode2_t;

/**
 * @brief union of a 32 bit word and the opcode bit fields
 */
typedef union {
    unsigned int word;          //!< opcode as 32 bit word
    p2_opcode_t op;             //!< ocpode as bit fields
    p2_opcode2_t op2;           //!< ocpode as bit fields (version including WC and WZ)
}   p2_opword_t;

/**
 * @brief structure of the shadow registers at the end of LUT
 */
typedef struct {
    P2LONG RAM[512-16];        //!< general-use code/data registers
    P2LONG IJMP3;              //!< interrupt call address for INT3
    P2LONG IRET3;              //!< interrupt return address for INT3
    P2LONG IJMP2;              //!< interrupt call address for INT2
    P2LONG IRET2;              //!< interrupt return address for INT2
    P2LONG IJMP1;              //!< interrupt call address for INT1
    P2LONG IRET1;              //!< interrupt return address for INT1
    P2LONG PA;                 //!< CALLD-imm return, CALLPA parameter, or LOC address
    P2LONG PB;                 //!< CALLD-imm return, CALLPB parameter, or LOC address
    P2LONG PTRA;               //!< pointer A to hub RAM
    P2LONG PTRB;               //!< pointer B to hub RAM
    P2LONG DIRA;               //!< output enables for P31 ... P0
    P2LONG DIRB;               //!< output enables for P63 ... P32
    P2LONG OUTA;               //!< output states for P31 ... P0
    P2LONG OUTB;               //!< output states for P63 ... P32
    P2LONG INA;                //!< input states for P31 ... P0
    P2LONG INB;                //!< input states for P63 ... P32
}   p2_lutregs_t;

typedef enum {
    offs_IJMP3 = offsetof(p2_lutregs_t, IJMP3) / sizeof(P2LONG),
    offs_IRET3 = offsetof(p2_lutregs_t, IRET3) / sizeof(P2LONG),
    offs_IJMP2 = offsetof(p2_lutregs_t, IJMP2) / sizeof(P2LONG),
    offs_IRET2 = offsetof(p2_lutregs_t, IRET2) / sizeof(P2LONG),
    offs_IJMP1 = offsetof(p2_lutregs_t, IJMP1) / sizeof(P2LONG),
    offs_IRET1 = offsetof(p2_lutregs_t, IRET1) / sizeof(P2LONG),
    offs_PA    = offsetof(p2_lutregs_t,    PA) / sizeof(P2LONG),
    offs_PB    = offsetof(p2_lutregs_t,    PB) / sizeof(P2LONG),
    offs_PTRA  = offsetof(p2_lutregs_t,  PTRA) / sizeof(P2LONG),
    offs_PTRB  = offsetof(p2_lutregs_t,  PTRB) / sizeof(P2LONG),
    offs_DIRA  = offsetof(p2_lutregs_t,  DIRA) / sizeof(P2LONG),
    offs_DIRB  = offsetof(p2_lutregs_t,  DIRB) / sizeof(P2LONG),
    offs_OUTA  = offsetof(p2_lutregs_t,  OUTA) / sizeof(P2LONG),
    offs_OUTB  = offsetof(p2_lutregs_t,  OUTB) / sizeof(P2LONG),
    offs_INA   = offsetof(p2_lutregs_t,   INA) / sizeof(P2LONG),
    offs_INB   = offsetof(p2_lutregs_t,   INB) / sizeof(P2LONG)
}   p2_lutregs_e;

Q_STATIC_ASSERT(offs_INB == 0x1ff);

/**
 * @brief union of the COG memory (shadow registers ?)
 */
typedef union {
    P2LONG RAM[512];
}   p2_cog_t;

/**
 * @brief union of the LUT memory and shadow registers
 */
typedef union {
    P2LONG RAM[512];
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
    P2LONG mask;               //!< mask value
    P2LONG match;              //!< match value
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
    P2LONG edge;               //!< pin edge type
    P2LONG num;                //!< pin number
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
    P2LONG bits;               //!< interrupt flags as LONG
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
    P2LONG num:4;              //!< LOCK COG id
    p2_lock_mode_e mode:2;      //!< LOCK edge mode
    P2LONG edge:2;             //!< LOCK edge
    P2LONG prev:1;             //!< LOCK previous state
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
    P2LONG flag;               //!< non-zero if waiting
    p2_wait_mode_e mode;        //!< current wait mode
}   p2_wait_t;

/**
 * @brief FIFO configuration and status
 */
typedef struct {
    P2LONG buff[16];           //!< buffer of 16 longs read from / written to the HUB
    P2LONG rindex;             //!< current read index
    P2LONG windex;             //!< current write index
    P2LONG head_addr;          //!< head address
    P2LONG tail_addr;          //!< tail address
    P2LONG addr0;              //!< 1st address
    P2LONG addr1;              //!< 2nd address
    P2LONG mode;               //!< FIFO mode
    P2LONG word;               //!< FIFO word
    P2LONG flag;               //!< FIFO flags
}   p2_fifo_t;

typedef struct {
    p2_opword_t IR;
    P2LONG R;
    P2LONG D;
    P2LONG S;
}   p2_queue_t;
