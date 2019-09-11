/****************************************************************************
 *
 * Propeller2 enumeration of tokens
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
#include <QVector>

/**
 * @brief enumeration of tokens used in mnemonics for the P2 assembler and disassembler
 */
enum p2_TOKEN_e {
    t_invalid = -1,     //!< undefined value
    t_none,             //!< no string
    t_unknown,          //!< nothing found

    t_comment,          //!< token is a comment
    t_comment_eol,      //!< token is a comment until end of line (')
    t_comment_lcurly,   //!< token is a comment ({)
    t_comment_rcurly,   //!< token is a comment (})
    t_str_const,        //!< token is a string starting with doublequote (")
    t_bin_const,        //!< token is a binary value (%)
    t_byt_const,        //!< token is a byte index (i.e. base 4) value (%%)
    t_dec_const,        //!< token is a decimal value (starts with 1…9)
    t_hex_const,        //!< token is a hexadecimal value ($)
    t_real_const,       //!< token is a floating point value (x.y)
    t_locsym,           //!< token is a local symbol (starts with .)
    t_symbol,           //!< token is a symbol (alphanumeric)
    t_empty,            //!< token is empty

    t_D,                //!< D (register or 9 bit immediate)
    t_S,                //!< S (register or 9 bit immediate)

    // instructions
    t_ABS,
    t_ADD,
    t_ADDCT1,
    t_ADDCT2,
    t_ADDCT3,
    t_ADDPIX,
    t_ADDS,
    t_ADDSX,
    t_ADDX,
    t_AKPIN,
    t_ALLOWI,
    t_ALTB,
    t_ALTD,
    t_ALTGB,
    t_ALTGN,
    t_ALTGW,
    t_ALTI,
    t_ALTR,
    t_ALTS,
    t_ALTSB,
    t_ALTSN,
    t_ALTSW,
    t_AND,
    t_ANDN,
    t_AUGD,
    t_AUGS,
    t_BITC,
    t_BITH,
    t_BITL,
    t_BITNC,
    t_BITNOT,
    t_BITNZ,
    t_BITRND,
    t_BITZ,
    t_BLNPIX,
    t_BMASK,
    t_BRK,
    t_CALL,
    t_CALLA,
    t_CALLB,
    t_CALLD,
    t_CALLPA,
    t_CALLPB,
    t_CMP,
    t_CMPM,
    t_CMPR,
    t_CMPS,
    t_CMPSUB,
    t_CMPSX,
    t_CMPX,
    t_COGATN,
    t_COGBRK,
    t_COGID,
    t_COGINIT,
    t_COGSTOP,
    t_CRCBIT,
    t_CRCNIB,
    t_DECMOD,
    t_DECOD,
    t_DIRC,
    t_DIRH,
    t_DIRL,
    t_DIRNC,
    t_DIRNOT,
    t_DIRNZ,
    t_DIRRND,
    t_DIRZ,
    t_DJF,
    t_DJNF,
    t_DJNZ,
    t_DJZ,
    t_DRVC,
    t_DRVH,
    t_DRVL,
    t_DRVNC,
    t_DRVNOT,
    t_DRVNZ,
    t_DRVRND,
    t_DRVZ,
    t_ENCOD,
    t_EXECF,
    t_FBLOCK,
    t_FGE,
    t_FGES,
    t_FLE,
    t_FLES,
    t_FLTC,
    t_FLTH,
    t_FLTL,
    t_FLTNC,
    t_FLTNOT,
    t_FLTNZ,
    t_FLTRND,
    t_FLTZ,
    t_GETBRK,
    t_GETBYTE,
    t_GETCT,
    t_GETNIB,
    t_GETPTR,
    t_GETQX,
    t_GETQY,
    t_GETRND,
    t_GETSCP,
    t_GETWORD,
    t_GETXACC,
    t_HUBSET,
    t_IJNZ,
    t_IJZ,
    t_INCMOD,
    t_JATN,
    t_JCT1,
    t_JCT2,
    t_JCT3,
    t_JFBW,
    t_JINT,
    t_JMP,
    t_JMPREL,
    t_JNATN,
    t_JNCT1,
    t_JNCT2,
    t_JNCT3,
    t_JNFBW,
    t_JNINT,
    t_JNPAT,
    t_JNQMT,
    t_JNSE1,
    t_JNSE2,
    t_JNSE3,
    t_JNSE4,
    t_JNXFI,
    t_JNXMT,
    t_JNXRL,
    t_JNXRO,
    t_JPAT,
    t_JQMT,
    t_JSE1,
    t_JSE2,
    t_JSE3,
    t_JSE4,
    t_JXFI,
    t_JXMT,
    t_JXRL,
    t_JXRO,
    t_LOC,
    t_LOCKNEW,
    t_LOCKREL,
    t_LOCKRET,
    t_LOCKTRY,
    t_MERGEB,
    t_MERGEW,
    t_MIXPIX,
    t_MODCZ,
    t_MODC,
    t_MODZ,
    t_MOV,
    t_MOVBYTS,
    t_MUL,
    t_MULPIX,
    t_MULS,
    t_MUXC,
    t_MUXNC,
    t_MUXNIBS,
    t_MUXNITS,
    t_MUXNZ,
    t_MUXQ,
    t_MUXZ,
    t_NEG,
    t_NEGC,
    t_NEGNC,
    t_NEGNZ,
    t_NEGZ,
    t_NIXINT1,
    t_NIXINT2,
    t_NIXINT3,
    t_NOP,
    t_NOT,
    t_ONES,
    t_OR,
    t_OUTC,
    t_OUTH,
    t_OUTL,
    t_OUTNC,
    t_OUTNOT,
    t_OUTNZ,
    t_OUTRND,
    t_OUTZ,
    t_PA,
    t_PB,
    t_POLLATN,
    t_POLLCT1,
    t_POLLCT2,
    t_POLLCT3,
    t_POLLFBW,
    t_POLLINT,
    t_POLLPAT,
    t_POLLQMT,
    t_POLLSE1,
    t_POLLSE2,
    t_POLLSE3,
    t_POLLSE4,
    t_POLLXFI,
    t_POLLXMT,
    t_POLLXRL,
    t_POLLXRO,
    t_POP,
    t_POPA,
    t_POPB,
    t_PUSH,
    t_PUSHA,
    t_PUSHB,
    t_QDIV,
    t_QEXP,
    t_QFRAC,
    t_QLOG,
    t_QMUL,
    t_QROTATE,
    t_QSQRT,
    t_QVECTOR,
    t_RCL,
    t_RCR,
    t_RCZL,
    t_RCZR,
    t_RDBYTE,
    t_RDFAST,
    t_RDLONG,
    t_RDLUT,
    t_RDPIN,
    t_RDWORD,
    t_REP,
    t_RESI0,
    t_RESI1,
    t_RESI2,
    t_RESI3,
    t_RET,
    t_RETA,
    t_RETB,
    t_RETI0,
    t_RETI1,
    t_RETI2,
    t_RETI3,
    t_REV,
    t_RFBYTE,
    t_RFLONG,
    t_RFVAR,
    t_RFVARS,
    t_RFWORD,
    t_RGBEXP,
    t_RGBSQZ,
    t_ROL,
    t_ROLBYTE,
    t_ROLNIB,
    t_ROLWORD,
    t_ROR,
    t_RQPIN,
    t_SAL,
    t_SAR,
    t_SCA,
    t_SCAS,
    t_SETBYTE,
    t_SETCFRQ,
    t_SETCI,
    t_SETCMOD,
    t_SETCQ,
    t_SETCY,
    t_SETD,
    t_SETDACS,
    t_SETINT1,
    t_SETINT2,
    t_SETINT3,
    t_SETLUTS,
    t_SETNIB,
    t_SETPAT,
    t_SETPIV,
    t_SETPIX,
    t_SETQ,
    t_SETQ2,
    t_SETR,
    t_SETS,
    t_SETSCP,
    t_SETSE1,
    t_SETSE2,
    t_SETSE3,
    t_SETSE4,
    t_SETWORD,
    t_SETXFRQ,
    t_SEUSSF,
    t_SEUSSR,
    t_SHL,
    t_SHR,
    t_SIGNX,
    t_SKIP,
    t_SKIPF,
    t_SPLITB,
    t_SPLITW,
    t_STALLI,
    t_SUB,
    t_SUBR,
    t_SUBS,
    t_SUBSX,
    t_SUBX,
    t_SUMC,
    t_SUMNC,
    t_SUMNZ,
    t_SUMZ,
    t_TEST,
    t_TESTB,
    t_TESTBN,
    t_TESTN,
    t_TESTP,
    t_TESTPN,
    t_TJF,
    t_TJNF,
    t_TJNS,
    t_TJNZ,
    t_TJS,
    t_TJV,
    t_TJZ,
    t_TRGINT1,
    t_TRGINT2,
    t_TRGINT3,
    t_WAITATN,
    t_WAITCT1,
    t_WAITCT2,
    t_WAITCT3,
    t_WAITFBW,
    t_WAITINT,
    t_WAITPAT,
    t_WAITSE1,
    t_WAITSE2,
    t_WAITSE3,
    t_WAITSE4,
    t_WAITX,
    t_WAITXFI,
    t_WAITXMT,
    t_WAITXRL,
    t_WAITXRO,
    t_WFBYTE,
    t_WFLONG,
    t_WFWORD,
    t_WMLONG,
    t_WRBYTE,
    t_WRC,
    t_WRFAST,
    t_WRLONG,
    t_WRLUT,
    t_WRNC,
    t_WRNZ,
    t_WRPIN,
    t_WRWORD,
    t_WRZ,
    t_WXPIN,
    t_WYPIN,
    t_XCONT,
    t_XINIT,
    t_XOR,
    t_XORO32,
    t_XSTOP,
    t_XZERO,
    t_ZEROX,

    t_WC,
    t_WZ,
    t_WCZ,
    t_ANDC,
    t_ANDZ,
    t_ORC,
    t_ORZ,
    t_XORC,
    t_XORZ,

    // Section control
    t_DAT,
    t_CON,
    t_PUB,
    t_PRI,
    t_VAR,

    // Origin control
    t_ORG,
    t_ORGF,
    t_ORGH,
    t_ALIGNW,
    t_ALIGNL,
    t_FIT,

    // Data types and space reserving
    t_BYTE,
    t_WORD,
    t_LONG,
    t_RES,
    t_FILE,

    // conditials
    t__RET_,            //!< cond_never
    t_IF_NC_AND_NZ,     //!< cond_nc_and_nz
    t_IF_NZ_AND_NC,     //!< alias for cond_nc_and_nz
    t_IF_A,             //!< alias for cond_nc_and_nz
    t_IF_GT,            //!< alias for cond_nc_and_nz
    t_IF_00,            //!< alias for cond_nc_and_nz
    t_IF_NC_AND_Z,      //!< cond_nc_and_z
    t_IF_Z_AND_NC,      //!< alias for cond_nc_and_z
    t_IF_01,            //!< alias for cond_nc_and_z
    t_IF_NC,            //!< cond_nc
    t_IF_AE,            //!< alias for cond_nc
    t_IF_GE,            //!< alias for cond_nc
    t_IF_0X,            //!< alias for cond_nc
    t_IF_C_AND_NZ,      //!< cond_c_and_nz
    t_IF_NZ_AND_C,      //!< alias for cond_c_and_nz
    t_IF_10,            //!< alias for cond_c_and_nz
    t_IF_NZ,            //!< cond_nz
    t_IF_NE,            //!< alias for cond_nz
    t_IF_X0,            //!< alias for cond_nz
    t_IF_C_NE_Z,        //!< cond_c_ne_z
    t_IF_Z_NE_C,        //!< alias for cond_c_ne_z
    t_IF_DIFF,          //!< alias for cond_c_ne_z
    t_IF_NC_OR_NZ,      //!< cond_nc_or_nz
    t_IF_NZ_OR_NC,      //!< alias for cond_nc_or_nz
    t_IF_NOT_11,        //!< alias for cond_nc_or_nz
    t_IF_C_AND_Z,       //!< cond_c_and_z
    t_IF_Z_AND_C,       //!< alias for cond_c_and_z
    t_IF_11,            //!< alias for cond_c_and_z
    t_IF_C_EQ_Z,        //!< cond_c_eq_z
    t_IF_Z_EQ_C,        //!< alias for cond_c_eq_z
    t_IF_SAME,          //!< alias for cond_c_eq_z
    t_IF_Z,             //!< cond_z
    t_IF_E,             //!< alias for cond_z
    t_IF_X1,            //!< alias for cond_z
    t_IF_NC_OR_Z,       //!< cond_nc_or_z
    t_IF_Z_OR_NC,       //!< alias for cond_nc_or_z
    t_IF_BE,            //!< alias for cond_nc_or_z
    t_IF_NOT_10,        //!< alias for cond_nc_or_z
    t_IF_C,             //!< cond_c
    t_IF_B,             //!< alias for cond_c
    t_IF_LT,            //!< alias for cond_c
    t_IF_1X,            //!< alias for cond_c
    t_IF_C_OR_NZ,       //!< cond_c_or_nz
    t_IF_NZ_OR_C,       //!< alias for cond_c_or_nz
    t_IF_NOT_01,        //!< alias for cond_c_or_nz
    t_IF_C_OR_Z,        //!< cond_c_or_z
    t_IF_Z_OR_C,        //!< alias for cond_c_or_z
    t_IF_LE,            //!< alias for cond_c_or_z
    t_IF_NOT_00,        //!< alias for cond_c_or_z
    t_IF_ALWAYS,        //!< cond_always

    // MODCZ parameters
    t_MODCZ__CLR,       //!< cond_never
    t_MODCZ__NC_AND_NZ, //!< cond_nc_and_nz
    t_MODCZ__NZ_AND_NC, //!< alias for cond_nc_and_nz
    t_MODCZ__A,         //!< alias for cond_nc_and_nz
    t_MODCZ__GT,        //!< alias for cond_nc_and_nz
    t_MODCZ__00,        //!< alias for cond_nc_and_nz
    t_MODCZ__NC_AND_Z,  //!< cond_nc_and_z
    t_MODCZ__Z_AND_NC,  //!< alias for cond_nc_and_z
    t_MODCZ__01,        //!< alias for cond_nc_and_z
    t_MODCZ__NC,        //!< cond_nc
    t_MODCZ__AE,        //!< alias for cond_nc
    t_MODCZ__GE,        //!< alias for cond_nc
    t_MODCZ__0X,        //!< alias for cond_nc
    t_MODCZ__C_AND_NZ,  //!< cond_c_and_nz
    t_MODCZ__NZ_AND_C,  //!< alias for cond_c_and_nz
    t_MODCZ__10,        //!< alias for cond_c_and_nz
    t_MODCZ__NZ,        //!< cond_nz
    t_MODCZ__NE,        //!< alias for cond_nz
    t_MODCZ__X0,        //!< alias for cond_nz
    t_MODCZ__C_NE_Z,    //!< cond_c_ne_z
    t_MODCZ__Z_NE_C,    //!< alias for cond_c_ne_z
    t_MODCZ__DIFF,      //!< alias for cond_c_ne_z
    t_MODCZ__NC_OR_NZ,  //!< cond_nc_or_nz
    t_MODCZ__NZ_OR_NC,  //!< alias for cond_nc_or_nz
    t_MODCZ__NOT_11,    //!< alias for cond_nc_or_nz
    t_MODCZ__C_AND_Z,   //!< cond_c_and_z
    t_MODCZ__Z_AND_C,   //!< alias for cond_c_and_z
    t_MODCZ__11,        //!< alias for cond_c_and_z
    t_MODCZ__C_EQ_Z,    //!< cond_c_eq_z
    t_MODCZ__Z_EQ_C,    //!< alias for cond_c_eq_z
    t_MODCZ__SAME,      //!< alias for cond_c_eq_z
    t_MODCZ__Z,         //!< cond_z
    t_MODCZ__E,         //!< alias for cond_z
    t_MODCZ__X1,        //!< alias for cond_z
    t_MODCZ__NC_OR_Z,   //!< cond_nc_or_z
    t_MODCZ__Z_OR_NC,   //!< alias for cond_nc_or_z
    t_MODCZ__BE,        //!< alias for cond_nc_or_z
    t_MODCZ__NOT_10,    //!< alias for cond_nc_or_z
    t_MODCZ__C,         //!< cond_c
    t_MODCZ__B,         //!< alias for cond_c
    t_MODCZ__LT,        //!< alias for cond_c
    t_MODCZ__1X,        //!< alias for cond_c
    t_MODCZ__C_OR_NZ,   //!< cond_c_or_nz
    t_MODCZ__NZ_OR_C,   //!< alias for cond_c_or_nz
    t_MODCZ__NOT_01,    //!< alias for cond_c_or_nz
    t_MODCZ__C_OR_Z,    //!< cond_c_or_z
    t_MODCZ__Z_OR_C,    //!< alias for cond_c_or_z
    t_MODCZ__LE,        //!< alias for cond_c_or_z
    t_MODCZ__NOT_00,    //!< alias for cond_c_or_z
    t_MODCZ__SET,       //!< cond_always

    // assignment
    t_ASSIGN,          //!< "="

    // shadow registers
    t_DIRA,
    t_DIRB,
    t_INA,
    t_INB,
    t_OUTA,
    t_OUTB,
    t_PTRA,
    t_PTRA_postinc,
    t_PTRA_postdec,
    t_PTRA_preinc,
    t_PTRA_predec,
    t_PTRB,
    t_PTRB_postinc,
    t_PTRB_postdec,
    t_PTRB_preinc,
    t_PTRB_predec,

    // conversion functions
    t__FLOAT,           //!< float(integer)
    t__ROUND,           //!< round(real)
    t__TRUNC,           //!< trunc(real)

    // origin (PC)
    t_DOLLAR,           //!< "$"
    t_COMMA,            //!< ","

    // traits
    t_IMMEDIATE,        //!< "#"
    t_AUGMENTED,        //!< "##"
    t_RELATIVE,         //!< "@"
    t_ABSOLUTE,         //!< "\"
    t_HUBADDRESS,       //!< "#@"

    // relations
    t__LE,              //!< "<="
    t__LT,              //!< "<"
    t__GE,              //!< ">="
    t__GT,              //!< ">"
    t__EQ,              //!< "=="
    t__NE,              //!< "!="

    // parenthesis
    t__LPAREN,          //!< "("
    t__RPAREN,          //!< ")"
    t__LBRACKET,        //!< "["
    t__RBRACKET,        //!< "]"

    // primary ops
    t__INC,             //!< "++"
    t__DEC,             //!< "--"

    // unary ops
    t__NEG,             //!< "!"
    t__NOT,             //!< "~"

    // multiplication ops
    t__MUL,             //!< "*"
    t__DIV,             //!< "/"
    t__MOD,             //!< "%"

    // addition ops
    t__PLUS,            //!< "+"
    t__MINUS,           //!< "-"

    // shift ops
    t__SHL,             //!< "<<"
    t__SHR,             //!< ">>"

    // binary ops
    t__AND,             //!< "&"
    t__OR,              //!< "|"
    t__XOR,             //!< "^"
    t__REV,             //!< "><"

    // encode/decode
    t__ENCOD,           //!< "|<"
    t__DECOD,           //!< ">|"

    t__LOGAND,          //!< "&&"
    t__LOGOR,           //!< "||"
};

typedef QVector<p2_TOKEN_e> p2_token_v;
