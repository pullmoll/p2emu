/****************************************************************************
 *
 * Propeller2 token and instruction mnemonics class
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
#include <QFlag>
#include <QString>
#include <QMultiHash>
#include "p2defs.h"

/**
 * @brief enumeration of token types
 */
typedef enum {
    tt_none,            //!< no specific type
    tt_parens,          //!< precedence  0: parenthesis "(" and ")"
    tt_primary,         //!< precedence  1: primary operators (++, --)
    tt_unary,           //!< precedence  2: unary operators (+, -, !, ~, more?)
    tt_mulop,           //!< precedence  3: multiplication operators (*, /, \)
    tt_addop,           //!< precedence  4: addition operators (+, -)
    tt_shiftop,         //!< precedence  5: shift operators (<<, >>)
    tt_relation,        //!< precedence  6: comparisons (<, <=, >, >=)
    tt_equality,        //!< precedence  7: comparisons (==, !=)
    tt_binop_and,       //!< precedence  8: binary and (&)
    tt_binop_xor,       //!< precedence  9: binary xor (^)
    tt_binop_or,        //!< precedence 10: binary or (|)
    tt_binop_rev,       //!< precedence 11: binary or (><)
    tt_logop_and,       //!< precedence 12: logical and (&&)
    tt_logop_or,        //!< precedence 13: logical or (||)
    tt_ternary,         //!< precedence 14: ternary operator (?:)
    tt_assignment,      //!< precedence 15: assignment (=)
    tt_conditional,     //!< conditional execution
    tt_modcz_param,     //!< MODCZ parameters
    tt_inst,            //!< instruction
    tt_wcz_suffix,      //!< suffixes WC, WZ, WCZ, ANDC, ANDZ, ORC, ORZ, XORC, XORZ
    tt_section,         //!< section control
    tt_origin,          //!< origin control
    tt_data,            //!< data generating
    tt_lexer,           //!< pseudo token from lexing a string
}   p2_tokentype_e;

/**
 * @brief bit masks for token types
 */
#define TTMASK(tt) (Q_UINT64_C(1) << (tt))

static constexpr quint64 tm_none        = 0;
static constexpr quint64 tm_parens      = TTMASK(tt_parens);
static constexpr quint64 tm_primary     = TTMASK(tt_primary);
static constexpr quint64 tm_unary       = TTMASK(tt_unary);
static constexpr quint64 tm_mulop       = TTMASK(tt_mulop);
static constexpr quint64 tm_addop       = TTMASK(tt_addop);
static constexpr quint64 tm_shiftop     = TTMASK(tt_shiftop);
static constexpr quint64 tm_relation    = TTMASK(tt_relation);
static constexpr quint64 tm_equality    = TTMASK(tt_equality);
static constexpr quint64 tm_binop_and   = TTMASK(tt_binop_and);
static constexpr quint64 tm_binop_xor   = TTMASK(tt_binop_xor);
static constexpr quint64 tm_binop_or    = TTMASK(tt_binop_or);
static constexpr quint64 tm_binop_rev   = TTMASK(tt_binop_rev);
static constexpr quint64 tm_logop_and   = TTMASK(tt_logop_and);
static constexpr quint64 tm_logop_or    = TTMASK(tt_logop_or);
static constexpr quint64 tm_ternary     = TTMASK(tt_ternary);
static constexpr quint64 tm_assignment  = TTMASK(tt_assignment);
static constexpr quint64 tm_conditional = TTMASK(tt_conditional);
static constexpr quint64 tm_modcz_param = TTMASK(tt_modcz_param);
static constexpr quint64 tm_inst        = TTMASK(tt_inst);
static constexpr quint64 tm_wcz_suffix  = TTMASK(tt_wcz_suffix);
static constexpr quint64 tm_section     = TTMASK(tt_section);
static constexpr quint64 tm_origin      = TTMASK(tt_origin);
static constexpr quint64 tm_data        = TTMASK(tt_data);
static constexpr quint64 tm_lexer       = TTMASK(tt_lexer);

static constexpr quint64 tm_primary_unary = tm_primary | tm_unary;

static constexpr quint64 tm_binops      = tm_binop_and |
                                          tm_binop_xor |
                                          tm_binop_or |
                                          tm_binop_rev;

static constexpr quint64 tm_operations  = tm_primary |
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
                                          tm_logop_and |
                                          tm_logop_or;
/**
 * @brief enumeration of tokens used in mnemonics for the P2 assembler and disassembler
 */
typedef enum {
    t_invalid = -1,     //!< undefined value
    t_unknown,          //!< nothing found

    t_comma,            //!< token is a comma (,)
    t_string,           //!< token is a string starting with doublequote (")
    t_bin_const,        //!< token is a binary value (%)
    t_byt_const,        //!< token is a byte index (i.e. base 4) value (%%)
    t_oct_const,        //!< token is an octal value (0…)
    t_dec_const,        //!< token is a decimal value (starts with 1…9)
    t_hex_const,        //!< token is a hexadecimal value ($)
    t_locsym,           //!< token is a local symbol (starts with .)
    t_symbol,           //!< token is a symbol (alphanumeric)
    t_expression,       //!< token is an expression (contains operators)
    t_empty,            //!< token is empty

    // conditials
    t__RET_,            //!< cond_never
    t_IF_NC_AND_NZ,     //!< cond_nc_and_nz
    t_IF_NZ_AND_NC,     //!< alias for cond_nc_and_nz
    t_IF_A,             //!< alias for cond_nc_and_nz
    t_IF_GT,            //!< alias for cond_nc_and_nz
    t_IF_NC_AND_Z,      //!< cond_nc_and_z
    t_IF_Z_AND_NC,      //!< alias for cond_nc_and_z
    t_IF_NC,            //!< cond_nc
    t_IF_AE,            //!< alias for cond_nc
    t_IF_GE,            //!< alias for cond_nc
    t_IF_C_AND_NZ,      //!< cond_c_and_nz
    t_IF_NZ_AND_C,      //!< alias for cond_c_and_nz
    t_IF_NZ,            //!< cond_nz
    t_IF_NE,            //!< alias for cond_nz
    t_IF_C_NE_Z,        //!< cond_c_ne_z
    t_IF_Z_NE_C,        //!< alias for cond_c_ne_z
    t_IF_NC_OR_NZ,      //!< cond_nc_or_nz
    t_IF_NZ_OR_NC,      //!< alias for cond_nc_or_nz
    t_IF_C_AND_Z,       //!< cond_c_and_z
    t_IF_Z_AND_C,       //!< alias for cond_c_and_z
    t_IF_C_EQ_Z,        //!< cond_c_eq_z
    t_IF_Z_EQ_C,        //!< alias for cond_c_eq_z
    t_IF_Z,             //!< cond_z
    t_IF_E,             //!< alias for cond_z
    t_IF_NC_OR_Z,       //!< cond_nc_or_z
    t_IF_Z_OR_NC,       //!< alias for cond_nc_or_z
    t_IF_BE,            //!< alias for cond_nc_or_z
    t_IF_C,             //!< cond_c
    t_IF_B,             //!< alias for cond_c
    t_IF_LT,            //!< alias for cond_c
    t_IF_C_OR_NZ,       //!< cond_c_or_nz
    t_IF_NZ_OR_C,       //!< alias for cond_c_or_nz
    t_IF_C_OR_Z,        //!< cond_c_or_z
    t_IF_Z_OR_C,        //!< alias for cond_c_or_z
    t_IF_LE,            //!< alias for cond_c_or_z
    t_IF_ALWAYS,        //!< cond_always

    // MODCZ parameters
    t_MODCZ__CLR,       //!< cond_never
    t_MODCZ__NC_AND_NZ, //!< cond_nc_and_nz
    t_MODCZ__NZ_AND_NC, //!< alias for cond_nc_and_nz
    t_MODCZ__GT,        //!< alias for cond_nc_and_nz
    t_MODCZ__NC_AND_Z,  //!< cond_nc_and_z
    t_MODCZ__Z_AND_NC,  //!< alias for cond_nc_and_z
    t_MODCZ__NC,        //!< cond_nc
    t_MODCZ__GE,        //!< alias for cond_nc
    t_MODCZ__C_AND_NZ,  //!< cond_c_and_nz
    t_MODCZ__NZ_AND_C,  //!< alias for cond_c_and_nz
    t_MODCZ__NZ,        //!< cond_nz
    t_MODCZ__NE,        //!< alias for cond_nz
    t_MODCZ__C_NE_Z,    //!< cond_c_ne_z
    t_MODCZ__Z_NE_C,    //!< alias for cond_c_ne_z
    t_MODCZ__NC_OR_NZ,  //!< cond_nc_or_nz
    t_MODCZ__NZ_OR_NC,  //!< alias for cond_nc_or_nz
    t_MODCZ__C_AND_Z,   //!< cond_c_and_z
    t_MODCZ__Z_AND_C,   //!< alias for cond_c_and_z
    t_MODCZ__C_EQ_Z,    //!< cond_c_eq_z
    t_MODCZ__Z_EQ_C,    //!< alias for cond_c_eq_z
    t_MODCZ__Z,         //!< cond_z
    t_MODCZ__E,         //!< alias for cond_z
    t_MODCZ__NC_OR_Z,   //!< cond_nc_or_z
    t_MODCZ__Z_OR_NC,   //!< alias for cond_nc_or_z
    t_MODCZ__C,         //!< cond_c
    t_MODCZ__LT,        //!< alias for cond_c
    t_MODCZ__C_OR_NZ,   //!< cond_c_or_nz
    t_MODCZ__NZ_OR_C,   //!< alias for cond_c_or_nz
    t_MODCZ__C_OR_Z,    //!< cond_c_or_z
    t_MODCZ__Z_OR_C,    //!< alias for cond_c_or_z
    t_MODCZ__LE,        //!< alias for cond_c_or_z
    t_MODCZ__SET,       //!< cond_always

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
    t_SPACE,
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
    t__DAT,
    t__CON,
    t__PUB,
    t__PRI,
    t__VAR,

    // Origin control
    t__ORG,
    t__ORGH,
    t__FIT,

    // Data types and space reserving
    t__BYTE,
    t__WORD,
    t__LONG,
    t__RES,

    // assignment
    t__ASSIGN,          //!< "="

    // origin (PC)
    t__DOLLAR,          //!< "$"

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

    // primary ops
    t__INC,             //!< "++"
    t__DEC,             //!< "--"

    // unary ops
    t__NEG,             //!< "!"
    t__NOT,             //!< "~"

    // multiplication ops
    t__MUL,             //!< "*"
    t__DIV,             //!< "/"
    t__MOD,             //!< "\"

    // addition ops
    t__ADD,             //!< "+"
    t__SUB,             //!< "-"

    // shift ops
    t__SHL,             //!< "<<"
    t__SHR,             //!< ">>"

    // binary ops
    t__AND,             //!< "&"
    t__OR,              //!< "|"
    t__XOR,             //!< "^"
    t__REV,             //!< "><"

    t__ANDAND,          //!< "&&"
    t__OROR,            //!< "||"
}   p2_token_e;

typedef QVector<p2_token_e> p2_token_v;

class P2Token
{
public:
    P2Token();
    QString string(p2_token_e tok, bool lowercase = false) const;
    p2_token_e token(const QString& str, bool chop = false, int* plen = nullptr) const;

    bool is_type(p2_token_e tok, p2_tokentype_e type) const;
    bool is_type(const QString& str, p2_tokentype_e type) const;
    QStringList typeNames(p2_token_e tok) const;

    bool is_operation(p2_token_e tok) const;

    p2_token_e at_token(int& pos, const QString& str, QList<p2_token_e> tokens, p2_token_e dflt = t_invalid) const;
    p2_token_e at_token(const QString& str, QList<p2_token_e> tokens, p2_token_e dflt = t_invalid) const;

    bool is_conditional(p2_token_e tok) const;
    bool is_modcz_param(p2_token_e tok) const;

    p2_token_e at_type(int& pos, const QString& str, quint64 typemask, p2_token_e dflt = t_invalid) const;
    p2_token_e at_type(int& pos, const QString& str, p2_tokentype_e type, p2_token_e dflt = t_invalid) const;
    p2_token_e at_type(const QString& str, p2_tokentype_e type, p2_token_e dflt = t_invalid) const;

    p2_token_e at_types(int& pos, const QString& str, quint64 typemask, p2_token_e dflt = t_invalid) const;
    p2_token_e at_types(int& pos, const QString& str, const QList<p2_tokentype_e>& types, p2_token_e dflt = t_invalid) const;
    p2_token_e at_types(const QString& str, const QList<p2_tokentype_e>& types, p2_token_e dflt = t_invalid) const;

    p2_token_e at_conditional(int& pos, const QString& str, p2_token_e dflt = t_invalid) const;
    p2_token_e at_conditional(const QString& str, p2_token_e dflt = t_invalid) const;

    p2_token_e at_modcz_param(int& pos, const QString& str, p2_token_e dflt = t_invalid) const;
    p2_token_e at_modcz_param(const QString& str, p2_token_e dflt = t_invalid) const;

    p2_cond_e conditional(p2_token_e cond, p2_cond_e dflt = cc_always) const;
    p2_cond_e conditional(const QString& str, p2_cond_e dflt = cc_always) const;

    p2_cond_e modcz_param(p2_token_e cond, p2_cond_e dflt = cc_clr) const;
    p2_cond_e modcz_param(const QString& str, p2_cond_e dflt = cc_clr) const;

private:
    QMultiHash<p2_token_e, QString> m_token_name;
    QMultiHash<QString, p2_token_e> m_name_token;
    QHash<p2_token_e, quint64> m_token_type;
    QMultiHash<quint64, p2_token_e> m_type_token;
    QHash<p2_token_e, p2_cond_e> m_lookup_cond;
    QHash<p2_token_e, p2_cond_e> m_lookup_modcz;
    QHash<p2_tokentype_e, QString> m_tokentype_name;

    QRegExp rx_loc_symbol;
    QRegExp rx_symbol;
    QRegExp rx_bin;
    QRegExp rx_byt;
    QRegExp rx_oct;
    QRegExp rx_dec;
    QRegExp rx_hex;
    QRegExp rx_string;
    QRegExp rx_unary;
    QRegExp rx_mulops;
    QRegExp rx_addops;
    QRegExp rx_binops;
    QRegExp rx_expression;

    void tn_add(p2_token_e tok, p2_tokentype_e type, const QString& str);
    void tn_add(p2_token_e tok, quint64 typemask, const QString& str);

    void tt_set(p2_token_e tok, quint64 typemask);
    void tt_clr(p2_token_e tok, quint64 types);
    bool tt_chk(p2_token_e tok, quint64 typemask) const;

    void tt_set(p2_token_e tok, p2_tokentype_e type);
    void tt_clr(p2_token_e tok, p2_tokentype_e type);
    bool tt_chk(p2_token_e tok, p2_tokentype_e type) const;
};

extern P2Token Token;
