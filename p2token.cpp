/****************************************************************************
 *
 * Propeller2 token and instruction mnemonics implementation
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
#include <QString>
#include <QRegExp>
#include "p2token.h"

#if 0
static const char *Delimiters[] = {
    "@@@", "##", "#", ",", "[", "]", "++", "+", "--", "-", "<<",
    "<", ">>", "><", ">|", ">", "*", "/", "\\", "&", "|<", "|", "(", ")",
     "@", "==", "=",
    nullptr
};
#endif

/**
 * @brief Regular expression for local symbol
 *
 * leading "."
 * then any number of "A"…"Z", "0"…"9", or "_"
 */
static const QString re_loc_symbol = QStringLiteral("\\.[A-Z_][A-Z0-9_]*");

/**
 * @brief Regular expression for alphanumeric
 *
 * leading "A"…"Z", or "_"
 * then any number of "A"…"Z", "0"…"9", or "_"
 */
static const QString re_symbol = QStringLiteral("[A-Z_][A-Z0-9_]*");

/**
 * @brief Regular expression for a string enclosed in doublequotes, possibly containing escaped doublequotes
 * initial '"'
 * then any number of escaped doublequotes (\") or other characters (.)
 * trailing '"'
 */
static const QString re_string = QStringLiteral("\"(?:[^\\\"]|\\.)*\"");

/**
 * @brief Regular expression for binary number
 *
 * leading "%"
 * then one or more of "0", "1", or "_"
 */
static const QString re_bin = QStringLiteral("%+[01_]+");

/**
 * @brief Regular expression for byte number
 *
 * leading "%%"
 * then one or more of "0"…"3", or "_"
 */
static const QString re_byt = QStringLiteral("%%+[0-3_]+");

/**
 * @brief Regular expression for octal number
 *
 * leading "0"
 * then one or more of "0"…"7", or "_"
 */
static const QString re_oct = QStringLiteral("0[0-7_]+");

/**
 * @brief Regular expression for a decimal number
 *
 * leading "0"…"9"
 * then any number of "0"…"9", or "_"
 */
static const QString re_dec = QStringLiteral("[0-9][0-9_]*");

/**
 * @brief Regular expression an octal number
 *
 * leading "$"
 * then any number of "0"…"9", "A"…"F", or "_"
 */
static const QString re_hex = QStringLiteral("\\$[0-9A-F_]+");

/**
 * @brief Regular expression unary operators
 *
 * any of "-", "!", "~", "--", or "++"
 */
static const QString re_unary = QStringLiteral("([-]{1,2}|[+]{1,2}|[!]|[~])?");

/**
 * @brief Regular expression for multiplication operators
 * any of "*", "/", "\"
 */
static const QString re_mulops = QStringLiteral("([*]|[/]|[\\\\])");

/**
 * @brief Regular expression for addition operators
 *
 * any of "-", "+"
 */
static const QString re_addops = QStringLiteral("([-]|[+])");

/**
 * @brief Regular expression for binary operators
 * any of "&", "|", "^", "<<", ">>", "><"
 */
static const QString re_binops = QStringLiteral("([&]|[|]|[^]|[<][<]|[>][>]|[>][<])");

//! Global static instance of the P2Token class
P2Token Token;

P2Token::P2Token()
    : m_token_name()
    , m_name_token()
    , m_token_type()
    , m_type_token()
    , m_lookup_cond()
    , m_lookup_modcz()
    , m_tokentype_name()
    , rx_loc_symbol(QString("^%1").arg(re_loc_symbol))
    , rx_symbol(QString("^%1").arg(re_symbol))
    , rx_bin(QString("^%1$").arg(re_bin))
    , rx_byt(QString("^%1$").arg(re_byt))
    , rx_oct(QString("^%1$").arg(re_oct))
    , rx_dec(QString("^%1$").arg(re_dec))
    , rx_hex(QString("^%1$").arg(re_hex))
    , rx_string(QString("^%1$").arg(re_string))
    , rx_unary(QString("^%1$").arg(re_unary))
    , rx_mulops(QString("^%1$").arg(re_mulops))
    , rx_addops(QString("^%1$").arg(re_addops))
    , rx_binops(QString("^%1$").arg(re_binops))
    , rx_expression()
{
    Q_ASSERT(rx_loc_symbol.isValid());
    Q_ASSERT(rx_symbol.isValid());
    Q_ASSERT(rx_bin.isValid());
    Q_ASSERT(rx_byt.isValid());
    Q_ASSERT(rx_oct.isValid());
    Q_ASSERT(rx_dec.isValid());
    Q_ASSERT(rx_hex.isValid());
    Q_ASSERT(rx_string.isValid());
    Q_ASSERT(rx_unary.isValid());
    Q_ASSERT(rx_mulops.isValid());
    Q_ASSERT(rx_addops.isValid());
    Q_ASSERT(rx_binops.isValid());

    tn_add(t_ABS,              tt_inst, QStringLiteral("ABS"));
    tn_add(t_ADD,              tt_inst, QStringLiteral("ADD"));
    tn_add(t_ADDCT1,           tt_inst, QStringLiteral("ADDCT1"));
    tn_add(t_ADDCT2,           tt_inst, QStringLiteral("ADDCT2"));
    tn_add(t_ADDCT3,           tt_inst, QStringLiteral("ADDCT3"));
    tn_add(t_ADDPIX,           tt_inst, QStringLiteral("ADDPIX"));
    tn_add(t_ADDS,             tt_inst, QStringLiteral("ADDS"));
    tn_add(t_ADDSX,            tt_inst, QStringLiteral("ADDSX"));
    tn_add(t_ADDX,             tt_inst, QStringLiteral("ADDX"));
    tn_add(t_AKPIN,            tt_inst, QStringLiteral("AKPIN"));
    tn_add(t_ALLOWI,           tt_inst, QStringLiteral("ALLOWI"));
    tn_add(t_ALTB,             tt_inst, QStringLiteral("ALTB"));
    tn_add(t_ALTD,             tt_inst, QStringLiteral("ALTD"));
    tn_add(t_ALTGB,            tt_inst, QStringLiteral("ALTGB"));
    tn_add(t_ALTGN,            tt_inst, QStringLiteral("ALTGN"));
    tn_add(t_ALTGW,            tt_inst, QStringLiteral("ALTGW"));
    tn_add(t_ALTI,             tt_inst, QStringLiteral("ALTI"));
    tn_add(t_ALTR,             tt_inst, QStringLiteral("ALTR"));
    tn_add(t_ALTS,             tt_inst, QStringLiteral("ALTS"));
    tn_add(t_ALTSB,            tt_inst, QStringLiteral("ALTSB"));
    tn_add(t_ALTSN,            tt_inst, QStringLiteral("ALTSN"));
    tn_add(t_ALTSW,            tt_inst, QStringLiteral("ALTSW"));
    tn_add(t_AND,              tt_inst, QStringLiteral("AND"));
    tn_add(t_ANDN,             tt_inst, QStringLiteral("ANDN"));
    tn_add(t_AUGD,             tt_inst, QStringLiteral("AUGD"));
    tn_add(t_AUGS,             tt_inst, QStringLiteral("AUGS"));
    tn_add(t_BITC,             tt_inst, QStringLiteral("BITC"));
    tn_add(t_BITH,             tt_inst, QStringLiteral("BITH"));
    tn_add(t_BITL,             tt_inst, QStringLiteral("BITL"));
    tn_add(t_BITNC,            tt_inst, QStringLiteral("BITNC"));
    tn_add(t_BITNOT,           tt_inst, QStringLiteral("BITNOT"));
    tn_add(t_BITNZ,            tt_inst, QStringLiteral("BITNZ"));
    tn_add(t_BITRND,           tt_inst, QStringLiteral("BITRND"));
    tn_add(t_BITZ,             tt_inst, QStringLiteral("BITZ"));
    tn_add(t_BLNPIX,           tt_inst, QStringLiteral("BLNPIX"));
    tn_add(t_BMASK,            tt_inst, QStringLiteral("BMASK"));
    tn_add(t_BRK,              tt_inst, QStringLiteral("BRK"));
    tn_add(t_CALL,             tt_inst, QStringLiteral("CALL"));
    tn_add(t_CALLA,            tt_inst, QStringLiteral("CALLA"));
    tn_add(t_CALLB,            tt_inst, QStringLiteral("CALLB"));
    tn_add(t_CALLD,            tt_inst, QStringLiteral("CALLD"));
    tn_add(t_CALLPA,           tt_inst, QStringLiteral("CALLPA"));
    tn_add(t_CALLPB,           tt_inst, QStringLiteral("CALLPB"));
    tn_add(t_CMP,              tt_inst, QStringLiteral("CMP"));
    tn_add(t_CMPM,             tt_inst, QStringLiteral("CMPM"));
    tn_add(t_CMPR,             tt_inst, QStringLiteral("CMPR"));
    tn_add(t_CMPS,             tt_inst, QStringLiteral("CMPS"));
    tn_add(t_CMPSUB,           tt_inst, QStringLiteral("CMPSUB"));
    tn_add(t_CMPSX,            tt_inst, QStringLiteral("CMPSX"));
    tn_add(t_CMPX,             tt_inst, QStringLiteral("CMPX"));
    tn_add(t_COGATN,           tt_inst, QStringLiteral("COGATN"));
    tn_add(t_COGBRK,           tt_inst, QStringLiteral("COGBRK"));
    tn_add(t_COGID,            tt_inst, QStringLiteral("COGID"));
    tn_add(t_COGINIT,          tt_inst, QStringLiteral("COGINIT"));
    tn_add(t_COGSTOP,          tt_inst, QStringLiteral("COGSTOP"));
    tn_add(t_CRCBIT,           tt_inst, QStringLiteral("CRCBIT"));
    tn_add(t_CRCNIB,           tt_inst, QStringLiteral("CRCNIB"));
    tn_add(t_DECMOD,           tt_inst, QStringLiteral("DECMOD"));
    tn_add(t_DECOD,            tt_inst, QStringLiteral("DECOD"));
    tn_add(t_DIRC,             tt_inst, QStringLiteral("DIRC"));
    tn_add(t_DIRH,             tt_inst, QStringLiteral("DIRH"));
    tn_add(t_DIRL,             tt_inst, QStringLiteral("DIRL"));
    tn_add(t_DIRNC,            tt_inst, QStringLiteral("DIRNC"));
    tn_add(t_DIRNOT,           tt_inst, QStringLiteral("DIRNOT"));
    tn_add(t_DIRNZ,            tt_inst, QStringLiteral("DIRNZ"));
    tn_add(t_DIRRND,           tt_inst, QStringLiteral("DIRRND"));
    tn_add(t_DIRZ,             tt_inst, QStringLiteral("DIRZ"));
    tn_add(t_DJF,              tt_inst, QStringLiteral("DJF"));
    tn_add(t_DJNF,             tt_inst, QStringLiteral("DJNF"));
    tn_add(t_DJNZ,             tt_inst, QStringLiteral("DJNZ"));
    tn_add(t_DJZ,              tt_inst, QStringLiteral("DJZ"));
    tn_add(t_DRVC,             tt_inst, QStringLiteral("DRVC"));
    tn_add(t_DRVH,             tt_inst, QStringLiteral("DRVH"));
    tn_add(t_DRVL,             tt_inst, QStringLiteral("DRVL"));
    tn_add(t_DRVNC,            tt_inst, QStringLiteral("DRVNC"));
    tn_add(t_DRVNOT,           tt_inst, QStringLiteral("DRVNOT"));
    tn_add(t_DRVNZ,            tt_inst, QStringLiteral("DRVNZ"));
    tn_add(t_DRVRND,           tt_inst, QStringLiteral("DRVRND"));
    tn_add(t_DRVZ,             tt_inst, QStringLiteral("DRVZ"));
    tn_add(t_ENCOD,            tt_inst, QStringLiteral("ENCOD"));
    tn_add(t_EXECF,            tt_inst, QStringLiteral("EXECF"));
    tn_add(t_FBLOCK,           tt_inst, QStringLiteral("FBLOCK"));
    tn_add(t_FGE,              tt_inst, QStringLiteral("FGE"));
    tn_add(t_FGES,             tt_inst, QStringLiteral("FGES"));
    tn_add(t_FLE,              tt_inst, QStringLiteral("FLE"));
    tn_add(t_FLES,             tt_inst, QStringLiteral("FLES"));
    tn_add(t_FLTC,             tt_inst, QStringLiteral("FLTC"));
    tn_add(t_FLTH,             tt_inst, QStringLiteral("FLTH"));
    tn_add(t_FLTL,             tt_inst, QStringLiteral("FLTL"));
    tn_add(t_FLTNC,            tt_inst, QStringLiteral("FLTNC"));
    tn_add(t_FLTNOT,           tt_inst, QStringLiteral("FLTNOT"));
    tn_add(t_FLTNZ,            tt_inst, QStringLiteral("FLTNZ"));
    tn_add(t_FLTRND,           tt_inst, QStringLiteral("FLTRND"));
    tn_add(t_FLTZ,             tt_inst, QStringLiteral("FLTZ"));
    tn_add(t_GETBRK,           tt_inst, QStringLiteral("GETBRK"));
    tn_add(t_GETBYTE,          tt_inst, QStringLiteral("GETBYTE"));
    tn_add(t_GETCT,            tt_inst, QStringLiteral("GETCT"));
    tn_add(t_GETNIB,           tt_inst, QStringLiteral("GETNIB"));
    tn_add(t_GETPTR,           tt_inst, QStringLiteral("GETPTR"));
    tn_add(t_GETQX,            tt_inst, QStringLiteral("GETQX"));
    tn_add(t_GETQY,            tt_inst, QStringLiteral("GETQY"));
    tn_add(t_GETRND,           tt_inst, QStringLiteral("GETRND"));
    tn_add(t_GETSCP,           tt_inst, QStringLiteral("GETSCP"));
    tn_add(t_GETWORD,          tt_inst, QStringLiteral("GETWORD"));
    tn_add(t_GETXACC,          tt_inst, QStringLiteral("GETXACC"));
    tn_add(t_HUBSET,           tt_inst, QStringLiteral("HUBSET"));
    tn_add(t_IJNZ,             tt_inst, QStringLiteral("IJNZ"));
    tn_add(t_IJZ,              tt_inst, QStringLiteral("IJZ"));
    tn_add(t_INCMOD,           tt_inst, QStringLiteral("INCMOD"));
    tn_add(t_JATN,             tt_inst, QStringLiteral("JATN"));
    tn_add(t_JCT1,             tt_inst, QStringLiteral("JCT1"));
    tn_add(t_JCT2,             tt_inst, QStringLiteral("JCT2"));
    tn_add(t_JCT3,             tt_inst, QStringLiteral("JCT3"));
    tn_add(t_JFBW,             tt_inst, QStringLiteral("JFBW"));
    tn_add(t_JINT,             tt_inst, QStringLiteral("JINT"));
    tn_add(t_JMP,              tt_inst, QStringLiteral("JMP"));
    tn_add(t_JMPREL,           tt_inst, QStringLiteral("JMPREL"));
    tn_add(t_JNATN,            tt_inst, QStringLiteral("JNATN"));
    tn_add(t_JNCT1,            tt_inst, QStringLiteral("JNCT1"));
    tn_add(t_JNCT2,            tt_inst, QStringLiteral("JNCT2"));
    tn_add(t_JNCT3,            tt_inst, QStringLiteral("JNCT3"));
    tn_add(t_JNFBW,            tt_inst, QStringLiteral("JNFBW"));
    tn_add(t_JNINT,            tt_inst, QStringLiteral("JNINT"));
    tn_add(t_JNPAT,            tt_inst, QStringLiteral("JNPAT"));
    tn_add(t_JNQMT,            tt_inst, QStringLiteral("JNQMT"));
    tn_add(t_JNSE1,            tt_inst, QStringLiteral("JNSE1"));
    tn_add(t_JNSE2,            tt_inst, QStringLiteral("JNSE2"));
    tn_add(t_JNSE3,            tt_inst, QStringLiteral("JNSE3"));
    tn_add(t_JNSE4,            tt_inst, QStringLiteral("JNSE4"));
    tn_add(t_JNXFI,            tt_inst, QStringLiteral("JNXFI"));
    tn_add(t_JNXMT,            tt_inst, QStringLiteral("JNXMT"));
    tn_add(t_JNXRL,            tt_inst, QStringLiteral("JNXRL"));
    tn_add(t_JNXRO,            tt_inst, QStringLiteral("JNXRO"));
    tn_add(t_JPAT,             tt_inst, QStringLiteral("JPAT"));
    tn_add(t_JQMT,             tt_inst, QStringLiteral("JQMT"));
    tn_add(t_JSE1,             tt_inst, QStringLiteral("JSE1"));
    tn_add(t_JSE2,             tt_inst, QStringLiteral("JSE2"));
    tn_add(t_JSE3,             tt_inst, QStringLiteral("JSE3"));
    tn_add(t_JSE4,             tt_inst, QStringLiteral("JSE4"));
    tn_add(t_JXFI,             tt_inst, QStringLiteral("JXFI"));
    tn_add(t_JXMT,             tt_inst, QStringLiteral("JXMT"));
    tn_add(t_JXRL,             tt_inst, QStringLiteral("JXRL"));
    tn_add(t_JXRO,             tt_inst, QStringLiteral("JXRO"));
    tn_add(t_LOC,              tt_inst, QStringLiteral("LOC"));
    tn_add(t_LOCKNEW,          tt_inst, QStringLiteral("LOCKNEW"));
    tn_add(t_LOCKREL,          tt_inst, QStringLiteral("LOCKREL"));
    tn_add(t_LOCKRET,          tt_inst, QStringLiteral("LOCKRET"));
    tn_add(t_LOCKTRY,          tt_inst, QStringLiteral("LOCKTRY"));
    tn_add(t_MERGEB,           tt_inst, QStringLiteral("MERGEB"));
    tn_add(t_MERGEW,           tt_inst, QStringLiteral("MERGEW"));
    tn_add(t_MIXPIX,           tt_inst, QStringLiteral("MIXPIX"));
    tn_add(t_MODCZ,            tt_inst, QStringLiteral("MODCZ"));
    tn_add(t_MOV,              tt_inst, QStringLiteral("MOV"));
    tn_add(t_MOVBYTS,          tt_inst, QStringLiteral("MOVBYTS"));
    tn_add(t_MUL,              tt_inst, QStringLiteral("MUL"));
    tn_add(t_MULPIX,           tt_inst, QStringLiteral("MULPIX"));
    tn_add(t_MULS,             tt_inst, QStringLiteral("MULS"));
    tn_add(t_MUXC,             tt_inst, QStringLiteral("MUXC"));
    tn_add(t_MUXNC,            tt_inst, QStringLiteral("MUXNC"));
    tn_add(t_MUXNIBS,          tt_inst, QStringLiteral("MUXNIBS"));
    tn_add(t_MUXNITS,          tt_inst, QStringLiteral("MUXNITS"));
    tn_add(t_MUXNZ,            tt_inst, QStringLiteral("MUXNZ"));
    tn_add(t_MUXQ,             tt_inst, QStringLiteral("MUXQ"));
    tn_add(t_MUXZ,             tt_inst, QStringLiteral("MUXZ"));
    tn_add(t_NEG,              tt_inst, QStringLiteral("NEG"));
    tn_add(t_NEGC,             tt_inst, QStringLiteral("NEGC"));
    tn_add(t_NEGNC,            tt_inst, QStringLiteral("NEGNC"));
    tn_add(t_NEGNZ,            tt_inst, QStringLiteral("NEGNZ"));
    tn_add(t_NEGZ,             tt_inst, QStringLiteral("NEGZ"));
    tn_add(t_NIXINT1,          tt_inst, QStringLiteral("NIXINT1"));
    tn_add(t_NIXINT2,          tt_inst, QStringLiteral("NIXINT2"));
    tn_add(t_NIXINT3,          tt_inst, QStringLiteral("NIXINT3"));
    tn_add(t_NOP,              tt_inst, QStringLiteral("NOP"));
    tn_add(t_NOT,              tt_inst, QStringLiteral("NOT"));
    tn_add(t_ONES,             tt_inst, QStringLiteral("ONES"));
    tn_add(t_OR,               tt_inst, QStringLiteral("OR"));
    tn_add(t_OUTC,             tt_inst, QStringLiteral("OUTC"));
    tn_add(t_OUTH,             tt_inst, QStringLiteral("OUTH"));
    tn_add(t_OUTL,             tt_inst, QStringLiteral("OUTL"));
    tn_add(t_OUTNC,            tt_inst, QStringLiteral("OUTNC"));
    tn_add(t_OUTNOT,           tt_inst, QStringLiteral("OUTNOT"));
    tn_add(t_OUTNZ,            tt_inst, QStringLiteral("OUTNZ"));
    tn_add(t_OUTRND,           tt_inst, QStringLiteral("OUTRND"));
    tn_add(t_OUTZ,             tt_inst, QStringLiteral("OUTZ"));
    tn_add(t_PA,               tt_inst, QStringLiteral("PA"));
    tn_add(t_PB,               tt_inst, QStringLiteral("PB"));
    tn_add(t_POLLATN,          tt_inst, QStringLiteral("POLLATN"));
    tn_add(t_POLLCT1,          tt_inst, QStringLiteral("POLLCT1"));
    tn_add(t_POLLCT2,          tt_inst, QStringLiteral("POLLCT2"));
    tn_add(t_POLLCT3,          tt_inst, QStringLiteral("POLLCT3"));
    tn_add(t_POLLFBW,          tt_inst, QStringLiteral("POLLFBW"));
    tn_add(t_POLLINT,          tt_inst, QStringLiteral("POLLINT"));
    tn_add(t_POLLPAT,          tt_inst, QStringLiteral("POLLPAT"));
    tn_add(t_POLLQMT,          tt_inst, QStringLiteral("POLLQMT"));
    tn_add(t_POLLSE1,          tt_inst, QStringLiteral("POLLSE1"));
    tn_add(t_POLLSE2,          tt_inst, QStringLiteral("POLLSE2"));
    tn_add(t_POLLSE3,          tt_inst, QStringLiteral("POLLSE3"));
    tn_add(t_POLLSE4,          tt_inst, QStringLiteral("POLLSE4"));
    tn_add(t_POLLXFI,          tt_inst, QStringLiteral("POLLXFI"));
    tn_add(t_POLLXMT,          tt_inst, QStringLiteral("POLLXMT"));
    tn_add(t_POLLXRL,          tt_inst, QStringLiteral("POLLXRL"));
    tn_add(t_POLLXRO,          tt_inst, QStringLiteral("POLLXRO"));
    tn_add(t_POP,              tt_inst, QStringLiteral("POP"));
    tn_add(t_POPA,             tt_inst, QStringLiteral("POPA"));
    tn_add(t_POPB,             tt_inst, QStringLiteral("POPB"));
    tn_add(t_PTRA,             tt_inst, QStringLiteral("PTRA"));
    tn_add(t_PTRA_postinc,     tt_inst, QStringLiteral("PTRA++"));
    tn_add(t_PTRA_postdec,     tt_inst, QStringLiteral("PTRA--"));
    tn_add(t_PTRA_preinc,      tt_inst, QStringLiteral("++PTRA"));
    tn_add(t_PTRA_predec,      tt_inst, QStringLiteral("--PTRA"));
    tn_add(t_PTRB,             tt_inst, QStringLiteral("PTRB"));
    tn_add(t_PTRB_postinc,     tt_inst, QStringLiteral("PTRB++"));
    tn_add(t_PTRB_postdec,     tt_inst, QStringLiteral("PTRB--"));
    tn_add(t_PTRB_preinc,      tt_inst, QStringLiteral("++PTRB"));
    tn_add(t_PTRB_predec,      tt_inst, QStringLiteral("--PTRB"));
    tn_add(t_PUSH,             tt_inst, QStringLiteral("PUSH"));
    tn_add(t_PUSHA,            tt_inst, QStringLiteral("PUSHA"));
    tn_add(t_PUSHB,            tt_inst, QStringLiteral("PUSHB"));
    tn_add(t_QDIV,             tt_inst, QStringLiteral("QDIV"));
    tn_add(t_QEXP,             tt_inst, QStringLiteral("QEXP"));
    tn_add(t_QFRAC,            tt_inst, QStringLiteral("QFRAC"));
    tn_add(t_QLOG,             tt_inst, QStringLiteral("QLOG"));
    tn_add(t_QMUL,             tt_inst, QStringLiteral("QMUL"));
    tn_add(t_QROTATE,          tt_inst, QStringLiteral("QROTATE"));
    tn_add(t_QSQRT,            tt_inst, QStringLiteral("QSQRT"));
    tn_add(t_QVECTOR,          tt_inst, QStringLiteral("QVECTOR"));
    tn_add(t_RCL,              tt_inst, QStringLiteral("RCL"));
    tn_add(t_RCR,              tt_inst, QStringLiteral("RCR"));
    tn_add(t_RCZL,             tt_inst, QStringLiteral("RCZL"));
    tn_add(t_RCZR,             tt_inst, QStringLiteral("RCZR"));
    tn_add(t_RDBYTE,           tt_inst, QStringLiteral("RDBYTE"));
    tn_add(t_RDFAST,           tt_inst, QStringLiteral("RDFAST"));
    tn_add(t_RDLONG,           tt_inst, QStringLiteral("RDLONG"));
    tn_add(t_RDLUT,            tt_inst, QStringLiteral("RDLUT"));
    tn_add(t_RDPIN,            tt_inst, QStringLiteral("RDPIN"));
    tn_add(t_RDWORD,           tt_inst, QStringLiteral("RDWORD"));
    tn_add(t_REP,              tt_inst, QStringLiteral("REP"));
    tn_add(t_RESI0,            tt_inst, QStringLiteral("RESI0"));
    tn_add(t_RESI1,            tt_inst, QStringLiteral("RESI1"));
    tn_add(t_RESI2,            tt_inst, QStringLiteral("RESI2"));
    tn_add(t_RESI3,            tt_inst, QStringLiteral("RESI3"));
    tn_add(t_RET,              tt_inst, QStringLiteral("RET"));
    tn_add(t_RETA,             tt_inst, QStringLiteral("RETA"));
    tn_add(t_RETB,             tt_inst, QStringLiteral("RETB"));
    tn_add(t_RETI0,            tt_inst, QStringLiteral("RETI0"));
    tn_add(t_RETI1,            tt_inst, QStringLiteral("RETI1"));
    tn_add(t_RETI2,            tt_inst, QStringLiteral("RETI2"));
    tn_add(t_RETI3,            tt_inst, QStringLiteral("RETI3"));
    tn_add(t_REV,              tt_inst, QStringLiteral("REV"));
    tn_add(t_RFBYTE,           tt_inst, QStringLiteral("RFBYTE"));
    tn_add(t_RFLONG,           tt_inst, QStringLiteral("RFLONG"));
    tn_add(t_RFVAR,            tt_inst, QStringLiteral("RFVAR"));
    tn_add(t_RFVARS,           tt_inst, QStringLiteral("RFVARS"));
    tn_add(t_RFWORD,           tt_inst, QStringLiteral("RFWORD"));
    tn_add(t_RGBEXP,           tt_inst, QStringLiteral("RGBEXP"));
    tn_add(t_RGBSQZ,           tt_inst, QStringLiteral("RGBSQZ"));
    tn_add(t_ROL,              tt_inst, QStringLiteral("ROL"));
    tn_add(t_ROLBYTE,          tt_inst, QStringLiteral("ROLBYTE"));
    tn_add(t_ROLNIB,           tt_inst, QStringLiteral("ROLNIB"));
    tn_add(t_ROLWORD,          tt_inst, QStringLiteral("ROLWORD"));
    tn_add(t_ROR,              tt_inst, QStringLiteral("ROR"));
    tn_add(t_RQPIN,            tt_inst, QStringLiteral("RQPIN"));
    tn_add(t_SAL,              tt_inst, QStringLiteral("SAL"));
    tn_add(t_SAR,              tt_inst, QStringLiteral("SAR"));
    tn_add(t_SCA,              tt_inst, QStringLiteral("SCA"));
    tn_add(t_SCAS,             tt_inst, QStringLiteral("SCAS"));
    tn_add(t_SETBYTE,          tt_inst, QStringLiteral("SETBYTE"));
    tn_add(t_SETCFRQ,          tt_inst, QStringLiteral("SETCFRQ"));
    tn_add(t_SETCI,            tt_inst, QStringLiteral("SETCI"));
    tn_add(t_SETCMOD,          tt_inst, QStringLiteral("SETCMOD"));
    tn_add(t_SETCQ,            tt_inst, QStringLiteral("SETCQ"));
    tn_add(t_SETCY,            tt_inst, QStringLiteral("SETCY"));
    tn_add(t_SETD,             tt_inst, QStringLiteral("SETD"));
    tn_add(t_SETDACS,          tt_inst, QStringLiteral("SETDACS"));
    tn_add(t_SETINT1,          tt_inst, QStringLiteral("SETINT1"));
    tn_add(t_SETINT2,          tt_inst, QStringLiteral("SETINT2"));
    tn_add(t_SETINT3,          tt_inst, QStringLiteral("SETINT3"));
    tn_add(t_SETLUTS,          tt_inst, QStringLiteral("SETLUTS"));
    tn_add(t_SETNIB,           tt_inst, QStringLiteral("SETNIB"));
    tn_add(t_SETPAT,           tt_inst, QStringLiteral("SETPAT"));
    tn_add(t_SETPIV,           tt_inst, QStringLiteral("SETPIV"));
    tn_add(t_SETPIX,           tt_inst, QStringLiteral("SETPIX"));
    tn_add(t_SETQ,             tt_inst, QStringLiteral("SETQ"));
    tn_add(t_SETQ2,            tt_inst, QStringLiteral("SETQ2"));
    tn_add(t_SETR,             tt_inst, QStringLiteral("SETR"));
    tn_add(t_SETS,             tt_inst, QStringLiteral("SETS"));
    tn_add(t_SETSCP,           tt_inst, QStringLiteral("SETSCP"));
    tn_add(t_SETSE1,           tt_inst, QStringLiteral("SETSE1"));
    tn_add(t_SETSE2,           tt_inst, QStringLiteral("SETSE2"));
    tn_add(t_SETSE3,           tt_inst, QStringLiteral("SETSE3"));
    tn_add(t_SETSE4,           tt_inst, QStringLiteral("SETSE4"));
    tn_add(t_SETWORD,          tt_inst, QStringLiteral("SETWORD"));
    tn_add(t_SETXFRQ,          tt_inst, QStringLiteral("SETXFRQ"));
    tn_add(t_SEUSSF,           tt_inst, QStringLiteral("SEUSSF"));
    tn_add(t_SEUSSR,           tt_inst, QStringLiteral("SEUSSR"));
    tn_add(t_SHL,              tt_inst, QStringLiteral("SHL"));
    tn_add(t_SHR,              tt_inst, QStringLiteral("SHR"));
    tn_add(t_SIGNX,            tt_inst, QStringLiteral("SIGNX"));
    tn_add(t_SKIP,             tt_inst, QStringLiteral("SKIP"));
    tn_add(t_SKIPF,            tt_inst, QStringLiteral("SKIPF"));
    tn_add(t_SPACE,            tt_inst, QStringLiteral("SPACE"));
    tn_add(t_SPLITB,           tt_inst, QStringLiteral("SPLITB"));
    tn_add(t_SPLITW,           tt_inst, QStringLiteral("SPLITW"));
    tn_add(t_STALLI,           tt_inst, QStringLiteral("STALLI"));
    tn_add(t_SUB,              tt_inst, QStringLiteral("SUB"));
    tn_add(t_SUBR,             tt_inst, QStringLiteral("SUBR"));
    tn_add(t_SUBS,             tt_inst, QStringLiteral("SUBS"));
    tn_add(t_SUBSX,            tt_inst, QStringLiteral("SUBSX"));
    tn_add(t_SUBX,             tt_inst, QStringLiteral("SUBX"));
    tn_add(t_SUMC,             tt_inst, QStringLiteral("SUMC"));
    tn_add(t_SUMNC,            tt_inst, QStringLiteral("SUMNC"));
    tn_add(t_SUMNZ,            tt_inst, QStringLiteral("SUMNZ"));
    tn_add(t_SUMZ,             tt_inst, QStringLiteral("SUMZ"));
    tn_add(t_TEST,             tt_inst, QStringLiteral("TEST"));
    tn_add(t_TESTB,            tt_inst, QStringLiteral("TESTB"));
    tn_add(t_TESTBN,           tt_inst, QStringLiteral("TESTBN"));
    tn_add(t_TESTN,            tt_inst, QStringLiteral("TESTN"));
    tn_add(t_TESTP,            tt_inst, QStringLiteral("TESTP"));
    tn_add(t_TESTPN,           tt_inst, QStringLiteral("TESTPN"));
    tn_add(t_TJF,              tt_inst, QStringLiteral("TJF"));
    tn_add(t_TJNF,             tt_inst, QStringLiteral("TJNF"));
    tn_add(t_TJNS,             tt_inst, QStringLiteral("TJNS"));
    tn_add(t_TJNZ,             tt_inst, QStringLiteral("TJNZ"));
    tn_add(t_TJS,              tt_inst, QStringLiteral("TJS"));
    tn_add(t_TJV,              tt_inst, QStringLiteral("TJV"));
    tn_add(t_TJZ,              tt_inst, QStringLiteral("TJZ"));
    tn_add(t_TRGINT1,          tt_inst, QStringLiteral("TRGINT1"));
    tn_add(t_TRGINT2,          tt_inst, QStringLiteral("TRGINT2"));
    tn_add(t_TRGINT3,          tt_inst, QStringLiteral("TRGINT3"));
    tn_add(t_WAITATN,          tt_inst, QStringLiteral("WAITATN"));
    tn_add(t_WAITCT1,          tt_inst, QStringLiteral("WAITCT1"));
    tn_add(t_WAITCT2,          tt_inst, QStringLiteral("WAITCT2"));
    tn_add(t_WAITCT3,          tt_inst, QStringLiteral("WAITCT3"));
    tn_add(t_WAITFBW,          tt_inst, QStringLiteral("WAITFBW"));
    tn_add(t_WAITINT,          tt_inst, QStringLiteral("WAITINT"));
    tn_add(t_WAITPAT,          tt_inst, QStringLiteral("WAITPAT"));
    tn_add(t_WAITSE1,          tt_inst, QStringLiteral("WAITSE1"));
    tn_add(t_WAITSE2,          tt_inst, QStringLiteral("WAITSE2"));
    tn_add(t_WAITSE3,          tt_inst, QStringLiteral("WAITSE3"));
    tn_add(t_WAITSE4,          tt_inst, QStringLiteral("WAITSE4"));
    tn_add(t_WAITX,            tt_inst, QStringLiteral("WAITX"));
    tn_add(t_WAITXFI,          tt_inst, QStringLiteral("WAITXFI"));
    tn_add(t_WAITXMT,          tt_inst, QStringLiteral("WAITXMT"));
    tn_add(t_WAITXRL,          tt_inst, QStringLiteral("WAITXRL"));
    tn_add(t_WAITXRO,          tt_inst, QStringLiteral("WAITXRO"));
    tn_add(t_WFBYTE,           tt_inst, QStringLiteral("WFBYTE"));
    tn_add(t_WFLONG,           tt_inst, QStringLiteral("WFLONG"));
    tn_add(t_WFWORD,           tt_inst, QStringLiteral("WFWORD"));
    tn_add(t_WMLONG,           tt_inst, QStringLiteral("WMLONG"));
    tn_add(t_WRBYTE,           tt_inst, QStringLiteral("WRBYTE"));
    tn_add(t_WRC,              tt_inst, QStringLiteral("WRC"));
    tn_add(t_WRFAST,           tt_inst, QStringLiteral("WRFAST"));
    tn_add(t_WRLONG,           tt_inst, QStringLiteral("WRLONG"));
    tn_add(t_WRLUT,            tt_inst, QStringLiteral("WRLUT"));
    tn_add(t_WRNC,             tt_inst, QStringLiteral("WRNC"));
    tn_add(t_WRNZ,             tt_inst, QStringLiteral("WRNZ"));
    tn_add(t_WRPIN,            tt_inst, QStringLiteral("WRPIN"));
    tn_add(t_WRWORD,           tt_inst, QStringLiteral("WRWORD"));
    tn_add(t_WRZ,              tt_inst, QStringLiteral("WRZ"));
    tn_add(t_WXPIN,            tt_inst, QStringLiteral("WXPIN"));
    tn_add(t_WYPIN,            tt_inst, QStringLiteral("WYPIN"));
    tn_add(t_XCONT,            tt_inst, QStringLiteral("XCONT"));
    tn_add(t_XINIT,            tt_inst, QStringLiteral("XINIT"));
    tn_add(t_XOR,              tt_inst, QStringLiteral("XOR"));
    tn_add(t_XORO32,           tt_inst, QStringLiteral("XORO32"));
    tn_add(t_XSTOP,            tt_inst, QStringLiteral("XSTOP"));
    tn_add(t_XZERO,            tt_inst, QStringLiteral("XZERO"));
    tn_add(t_ZEROX,            tt_inst, QStringLiteral("ZEROX"));
    tn_add(t_empty,            tt_inst, QStringLiteral("<empty>"));

    tn_add(t_invalid,          tt_lexer, QStringLiteral("<invalid>"));
    tn_add(t_unknown,          tt_lexer, QStringLiteral("«expr»"));
    tn_add(t_comma,            tt_lexer, QStringLiteral(","));
    tn_add(t_string,           tt_lexer, QStringLiteral("«string»"));
    tn_add(t_bin_const,        tt_lexer, QStringLiteral("«bin»"));
    tn_add(t_oct_const,        tt_lexer, QStringLiteral("«oct»"));
    tn_add(t_dec_const,        tt_lexer, QStringLiteral("«dec»"));
    tn_add(t_hex_const,        tt_lexer, QStringLiteral("«hex»"));
    tn_add(t_locsym,           tt_lexer, QStringLiteral("«locsym»"));
    tn_add(t_symbol,           tt_lexer, QStringLiteral("«symbol»"));
    tn_add(t_expression,       tt_lexer, QStringLiteral("«expression»"));


    tn_add(t__RET_,            tt_conditional, QStringLiteral("_RET_"));
    tn_add(t_IF_NZ_AND_NC,     tt_conditional, QStringLiteral("IF_NZ_AND_NC"));
    tn_add(t_IF_NC_AND_NZ,     tt_conditional, QStringLiteral("IF_NC_AND_NZ"));
    tn_add(t_IF_A,             tt_conditional, QStringLiteral("IF_A"));
    tn_add(t_IF_GT,            tt_conditional, QStringLiteral("IF_GT"));
    tn_add(t_IF_Z_AND_NC,      tt_conditional, QStringLiteral("IF_Z_AND_NC"));
    tn_add(t_IF_NC_AND_Z,      tt_conditional, QStringLiteral("IF_NC_AND_Z"));
    tn_add(t_IF_NC,            tt_conditional, QStringLiteral("IF_NC"));
    tn_add(t_IF_AE,            tt_conditional, QStringLiteral("IF_AE"));
    tn_add(t_IF_GE,            tt_conditional, QStringLiteral("IF_GE"));
    tn_add(t_IF_NZ_AND_C,      tt_conditional, QStringLiteral("IF_NZ_AND_C"));
    tn_add(t_IF_C_AND_NZ,      tt_conditional, QStringLiteral("IF_C_AND_NZ"));
    tn_add(t_IF_NZ,            tt_conditional, QStringLiteral("IF_NZ"));
    tn_add(t_IF_NE,            tt_conditional, QStringLiteral("IF_NE"));
    tn_add(t_IF_Z_NE_C,        tt_conditional, QStringLiteral("IF_Z_NE_C"));
    tn_add(t_IF_C_NE_Z,        tt_conditional, QStringLiteral("IF_C_NE_Z"));
    tn_add(t_IF_NZ_OR_NC,      tt_conditional, QStringLiteral("IF_NZ_OR_NC"));
    tn_add(t_IF_NC_OR_NZ,      tt_conditional, QStringLiteral("IF_NC_OR_NZ"));
    tn_add(t_IF_Z_AND_C,       tt_conditional, QStringLiteral("IF_Z_AND_C"));
    tn_add(t_IF_C_AND_Z,       tt_conditional, QStringLiteral("IF_C_AND_Z"));
    tn_add(t_IF_Z_EQ_C,        tt_conditional, QStringLiteral("IF_Z_EQ_C"));
    tn_add(t_IF_C_EQ_Z,        tt_conditional, QStringLiteral("IF_C_EQ_Z"));
    tn_add(t_IF_Z,             tt_conditional, QStringLiteral("IF_Z"));
    tn_add(t_IF_E,             tt_conditional, QStringLiteral("IF_E"));
    tn_add(t_IF_Z_OR_NC,       tt_conditional, QStringLiteral("IF_Z_OR_NC"));
    tn_add(t_IF_NC_OR_Z,       tt_conditional, QStringLiteral("IF_NC_OR_Z"));
    tn_add(t_IF_C,             tt_conditional, QStringLiteral("IF_C"));
    tn_add(t_IF_B,             tt_conditional, QStringLiteral("IF_B"));
    tn_add(t_IF_LT,            tt_conditional, QStringLiteral("IF_LT"));
    tn_add(t_IF_NZ_OR_C,       tt_conditional, QStringLiteral("IF_NZ_OR_C"));
    tn_add(t_IF_C_OR_NZ,       tt_conditional, QStringLiteral("IF_C_OR_NZ"));
    tn_add(t_IF_Z_OR_C,        tt_conditional, QStringLiteral("IF_Z_OR_C"));
    tn_add(t_IF_C_OR_Z,        tt_conditional, QStringLiteral("IF_C_OR_Z"));
    tn_add(t_IF_BE,            tt_conditional, QStringLiteral("IF_BE"));
    tn_add(t_IF_LE,            tt_conditional, QStringLiteral("IF_LE"));
    tn_add(t_IF_ALWAYS,        tt_conditional, QStringLiteral("IF_ALWAYS"));

    tn_add(t_MODCZ__CLR,       tt_modcz_param, QStringLiteral("_CLR"));
    tn_add(t_MODCZ__NC_AND_NZ, tt_modcz_param, QStringLiteral("_NC_AND_NZ"));
    tn_add(t_MODCZ__NZ_AND_NC, tt_modcz_param, QStringLiteral("_NZ_AND_NC"));
    tn_add(t_MODCZ__GT,        tt_modcz_param, QStringLiteral("_GT"));
    tn_add(t_MODCZ__NC_AND_Z,  tt_modcz_param, QStringLiteral("_NC_AND_Z"));
    tn_add(t_MODCZ__Z_AND_NC,  tt_modcz_param, QStringLiteral("_Z_AND_NC"));
    tn_add(t_MODCZ__NC,        tt_modcz_param, QStringLiteral("_NC"));
    tn_add(t_MODCZ__GE,        tt_modcz_param, QStringLiteral("_GE"));
    tn_add(t_MODCZ__C_AND_NZ,  tt_modcz_param, QStringLiteral("_C_AND_NZ"));
    tn_add(t_MODCZ__NZ_AND_C,  tt_modcz_param, QStringLiteral("_NZ_AND_C"));
    tn_add(t_MODCZ__NZ,        tt_modcz_param, QStringLiteral("_NZ"));
    tn_add(t_MODCZ__NE,        tt_modcz_param, QStringLiteral("_NE"));
    tn_add(t_MODCZ__C_NE_Z,    tt_modcz_param, QStringLiteral("_C_NE_Z"));
    tn_add(t_MODCZ__Z_NE_C,    tt_modcz_param, QStringLiteral("_Z_NE_C"));
    tn_add(t_MODCZ__NC_OR_NZ,  tt_modcz_param, QStringLiteral("_NC_OR_NZ"));
    tn_add(t_MODCZ__NZ_OR_NC,  tt_modcz_param, QStringLiteral("_NZ_OR_NC"));
    tn_add(t_MODCZ__C_AND_Z,   tt_modcz_param, QStringLiteral("_C_AND_Z"));
    tn_add(t_MODCZ__Z_AND_C,   tt_modcz_param, QStringLiteral("_Z_AND_C"));
    tn_add(t_MODCZ__C_EQ_Z,    tt_modcz_param, QStringLiteral("_C_EQ_Z"));
    tn_add(t_MODCZ__Z_EQ_C,    tt_modcz_param, QStringLiteral("_Z_EQ_C"));
    tn_add(t_MODCZ__Z,         tt_modcz_param, QStringLiteral("_Z"));
    tn_add(t_MODCZ__E,         tt_modcz_param, QStringLiteral("_E"));
    tn_add(t_MODCZ__NC_OR_Z,   tt_modcz_param, QStringLiteral("_NC_OR_Z"));
    tn_add(t_MODCZ__Z_OR_NC,   tt_modcz_param, QStringLiteral("_Z_OR_NC"));
    tn_add(t_MODCZ__C,         tt_modcz_param, QStringLiteral("_C"));
    tn_add(t_MODCZ__LT,        tt_modcz_param, QStringLiteral("_LT"));
    tn_add(t_MODCZ__C_OR_NZ,   tt_modcz_param, QStringLiteral("_C_OR_NZ"));
    tn_add(t_MODCZ__NZ_OR_C,   tt_modcz_param, QStringLiteral("_NZ_OR_C"));
    tn_add(t_MODCZ__C_OR_Z,    tt_modcz_param, QStringLiteral("_C_OR_Z"));
    tn_add(t_MODCZ__Z_OR_C,    tt_modcz_param, QStringLiteral("_Z_OR_C"));
    tn_add(t_MODCZ__LE,        tt_modcz_param, QStringLiteral("_LE"));
    tn_add(t_MODCZ__SET,       tt_modcz_param, QStringLiteral("_SET"));

    tn_add(t_WC,               tt_wcz_suffix, QStringLiteral("WC"));
    tn_add(t_WZ,               tt_wcz_suffix, QStringLiteral("WZ"));
    tn_add(t_WCZ,              tt_wcz_suffix, QStringLiteral("WCZ"));
    tn_add(t_ANDC,             tt_wcz_suffix, QStringLiteral("ANDC"));
    tn_add(t_ANDZ,             tt_wcz_suffix, QStringLiteral("ANDZ"));
    tn_add(t_ORC,              tt_wcz_suffix, QStringLiteral("ORC"));
    tn_add(t_ORZ,              tt_wcz_suffix, QStringLiteral("ORZ"));
    tn_add(t_XORC,             tt_wcz_suffix, QStringLiteral("XORC"));
    tn_add(t_XORZ,             tt_wcz_suffix, QStringLiteral("XORZ"));

    // Data
    tn_add(t__BYTE,            tm_inst | tm_data, QStringLiteral("BYTE"));
    tn_add(t__WORD,            tm_inst | tm_data, QStringLiteral("WORD"));
    tn_add(t__LONG,            tm_inst | tm_data, QStringLiteral("LONG"));
    tn_add(t__RES,             tm_inst | tm_data, QStringLiteral("RES"));

    // Section control
    tn_add(t__DAT,             tt_section, QStringLiteral("DAT"));
    tn_add(t__CON,             tt_section, QStringLiteral("CON"));
    tn_add(t__PUB,             tt_section, QStringLiteral("PUB"));
    tn_add(t__PRI,             tt_section, QStringLiteral("PRI"));
    tn_add(t__VAR,             tt_section, QStringLiteral("VAR"));

    // Origin control
    tn_add(t__ORG,             tt_origin, QStringLiteral("ORG"));
    tn_add(t__ORGH,            tt_origin, QStringLiteral("ORGH"));
    tn_add(t__FIT,             tt_origin, QStringLiteral("FIT"));

    // Assignment
    tn_add(t__ASSIGN,          tt_assignment, QStringLiteral("="));

    // Current PC reference
    m_token_name.insert(t__DOLLAR,          QStringLiteral("$"));

    // Sub expression in parens
    tn_add(t__LPAREN,          tt_parens, QStringLiteral("("));
    tn_add(t__RPAREN,          tt_parens, QStringLiteral(")"));

    // Set the primary operators
    tn_add(t__INC,             tt_primary, QStringLiteral("++"));
    tn_add(t__DEC,             tt_primary, QStringLiteral("--"));

    // Set the unary operators
    tn_add(t__NEG,             tt_unary, QStringLiteral("!"));
    tn_add(t__NOT,             tt_unary, QStringLiteral("~"));
    tt_set(t__ADD, tt_unary);
    tt_set(t__SUB, tt_unary);

    // Set the multiplication operators
    tn_add(t__MUL,             tt_mulop, QStringLiteral("*"));
    tn_add(t__DIV,             tt_mulop, QStringLiteral("/"));
    tn_add(t__MOD,             tt_mulop, QStringLiteral("\\"));

    // Set the addition operators
    tn_add(t__ADD,             tt_addop, QStringLiteral("+"));
    tn_add(t__SUB,             tt_addop, QStringLiteral("-"));

    // Set the shift operators
    tn_add(t__SHL,             tt_shiftop, QStringLiteral("<<"));
    tn_add(t__SHR,             tt_shiftop, QStringLiteral(">>"));

    // Set the less/greater comparison operators
    tn_add(t__GE,              tt_relation, QStringLiteral(">="));
    tn_add(t__GT,              tt_relation, QStringLiteral(">"));
    tn_add(t__LE,              tt_relation, QStringLiteral("<="));
    tn_add(t__LT,              tt_relation, QStringLiteral("<"));

    // Set the equal/unequal comparison operators
    tn_add(t__EQ,              tt_equality, QStringLiteral("=="));
    tn_add(t__NE,              tt_equality, QStringLiteral("!="));

    // Set the binary and operator
    tn_add(t__AND,             tt_binop_and, QStringLiteral("&"));

    // Set the binary xor operator
    tn_add(t__XOR,             tt_binop_xor, QStringLiteral("^"));

    // Set the binary or operator
    tn_add(t__OR,              tt_binop_or,  QStringLiteral("|"));

    // Set the unary reverse operator
    tn_add(t__REV,             tt_binop_rev, QStringLiteral("><"));

    // Set the conditionals lookup table
    m_lookup_cond.insert(t__RET_,               cc__ret_);
    m_lookup_cond.insert(t_IF_NZ_AND_NC,        cc_nc_and_nz);
    m_lookup_cond.insert(t_IF_NC_AND_NZ,        cc_nc_and_nz);
    m_lookup_cond.insert(t_IF_A,                cc_nc_and_nz);
    m_lookup_cond.insert(t_IF_GT,               cc_nc_and_nz);
    m_lookup_cond.insert(t_IF_Z_AND_NC,         cc_nc_and_z);
    m_lookup_cond.insert(t_IF_NC_AND_Z,         cc_nc_and_z);
    m_lookup_cond.insert(t_IF_NC,               cc_nc);
    m_lookup_cond.insert(t_IF_AE,               cc_nc);
    m_lookup_cond.insert(t_IF_GE,               cc_nc);
    m_lookup_cond.insert(t_IF_NZ_AND_C,         cc_c_and_nz);
    m_lookup_cond.insert(t_IF_C_AND_NZ,         cc_c_and_nz);
    m_lookup_cond.insert(t_IF_NZ,               cc_nz);
    m_lookup_cond.insert(t_IF_NE,               cc_nz);
    m_lookup_cond.insert(t_IF_Z_NE_C,           cc_c_ne_z);
    m_lookup_cond.insert(t_IF_C_NE_Z,           cc_c_ne_z);
    m_lookup_cond.insert(t_IF_NZ_OR_NC,         cc_nc_or_nz);
    m_lookup_cond.insert(t_IF_NC_OR_NZ,         cc_nc_or_nz);
    m_lookup_cond.insert(t_IF_Z_AND_C,          cc_c_and_z);
    m_lookup_cond.insert(t_IF_C_AND_Z,          cc_c_and_z);
    m_lookup_cond.insert(t_IF_Z_EQ_C,           cc_c_eq_z);
    m_lookup_cond.insert(t_IF_C_EQ_Z,           cc_c_eq_z);
    m_lookup_cond.insert(t_IF_Z,                cc_z);
    m_lookup_cond.insert(t_IF_E,                cc_z);
    m_lookup_cond.insert(t_IF_Z_OR_NC,          cc_nc_or_z);
    m_lookup_cond.insert(t_IF_NC_OR_Z,          cc_nc_or_z);
    m_lookup_cond.insert(t_IF_C,                cc_c);
    m_lookup_cond.insert(t_IF_B,                cc_c);
    m_lookup_cond.insert(t_IF_BE,               cc_c);
    m_lookup_cond.insert(t_IF_LT,               cc_c);
    m_lookup_cond.insert(t_IF_NZ_OR_C,          cc_c_or_nz);
    m_lookup_cond.insert(t_IF_C_OR_NZ,          cc_c_or_nz);
    m_lookup_cond.insert(t_IF_Z_OR_C,           cc_c_or_z);
    m_lookup_cond.insert(t_IF_C_OR_Z,           cc_c_or_z);
    m_lookup_cond.insert(t_IF_LE,               cc_c_or_z);
    m_lookup_cond.insert(t_IF_ALWAYS,           cc_always);

    // Set the MODCZ lookup table
    m_lookup_modcz.insert(t_MODCZ__CLR,         cc_clr);
    m_lookup_modcz.insert(t_MODCZ__NC_AND_NZ,   cc_nc_and_nz);
    m_lookup_modcz.insert(t_MODCZ__NZ_AND_NC,   cc_nc_and_nz);
    m_lookup_modcz.insert(t_MODCZ__GT,          cc_nc_and_nz);
    m_lookup_modcz.insert(t_MODCZ__NC_AND_Z,    cc_nc_and_z);
    m_lookup_modcz.insert(t_MODCZ__Z_AND_NC,    cc_nc_and_z);
    m_lookup_modcz.insert(t_MODCZ__NC,          cc_nc);
    m_lookup_modcz.insert(t_MODCZ__GE,          cc_nc);
    m_lookup_modcz.insert(t_MODCZ__C_AND_NZ,    cc_c_and_nz);
    m_lookup_modcz.insert(t_MODCZ__NZ_AND_C,    cc_c_and_nz);
    m_lookup_modcz.insert(t_MODCZ__NZ,          cc_nz);
    m_lookup_modcz.insert(t_MODCZ__NE,          cc_nz);
    m_lookup_modcz.insert(t_MODCZ__C_NE_Z,      cc_c_ne_z);
    m_lookup_modcz.insert(t_MODCZ__Z_NE_C,      cc_c_ne_z);
    m_lookup_modcz.insert(t_MODCZ__NC_OR_NZ,    cc_nc_or_nz);
    m_lookup_modcz.insert(t_MODCZ__NZ_OR_NC,    cc_nc_or_nz);
    m_lookup_modcz.insert(t_MODCZ__C_AND_Z,     cc_c_and_z);
    m_lookup_modcz.insert(t_MODCZ__Z_AND_C,     cc_c_and_z);
    m_lookup_modcz.insert(t_MODCZ__C_EQ_Z,      cc_c_eq_z);
    m_lookup_modcz.insert(t_MODCZ__Z_EQ_C,      cc_c_eq_z);
    m_lookup_modcz.insert(t_MODCZ__Z,           cc_z);
    m_lookup_modcz.insert(t_MODCZ__E,           cc_z);
    m_lookup_modcz.insert(t_MODCZ__NC_OR_Z,     cc_nc_or_z);
    m_lookup_modcz.insert(t_MODCZ__Z_OR_NC,     cc_nc_or_z);
    m_lookup_modcz.insert(t_MODCZ__C,           cc_c);
    m_lookup_modcz.insert(t_MODCZ__LT,          cc_c);
    m_lookup_modcz.insert(t_MODCZ__C_OR_NZ,     cc_c_or_nz);
    m_lookup_modcz.insert(t_MODCZ__NZ_OR_C,     cc_c_or_nz);
    m_lookup_modcz.insert(t_MODCZ__C_OR_Z,      cc_c_or_z);
    m_lookup_modcz.insert(t_MODCZ__Z_OR_C,      cc_c_or_z);
    m_lookup_modcz.insert(t_MODCZ__LE,          cc_c_or_z);
    m_lookup_modcz.insert(t_MODCZ__SET,         cc_always);

    // Build the reverse QMultiHash for string lookup
    foreach(p2_token_e tok, m_token_name.keys())
        foreach(const QString& str, m_token_name.values(tok))
            m_name_token.insert(str, tok);

    // Build the reverse QHash for types lookup
    foreach(p2_token_e tok, m_token_type.keys())
        foreach(quint64 mask, m_token_type.values(tok))
            m_type_token.insert(mask, tok);

    m_tokentype_name.insert(tt_none,            QStringLiteral("-"));
    m_tokentype_name.insert(tt_parens,          QStringLiteral("parenthesis"));
    m_tokentype_name.insert(tt_primary,         QStringLiteral("primary"));
    m_tokentype_name.insert(tt_unary,           QStringLiteral("unary"));
    m_tokentype_name.insert(tt_mulop,           QStringLiteral("mulop"));
    m_tokentype_name.insert(tt_addop,           QStringLiteral("addop"));
    m_tokentype_name.insert(tt_shiftop,         QStringLiteral("shiftop"));
    m_tokentype_name.insert(tt_relation,        QStringLiteral("relation"));
    m_tokentype_name.insert(tt_equality,        QStringLiteral("equality"));
    m_tokentype_name.insert(tt_binop_and,       QStringLiteral("binary and"));
    m_tokentype_name.insert(tt_binop_xor,       QStringLiteral("binary xor"));
    m_tokentype_name.insert(tt_binop_or,        QStringLiteral("binary or"));
    m_tokentype_name.insert(tt_binop_rev,       QStringLiteral("binary rev"));
    m_tokentype_name.insert(tt_logop_and,       QStringLiteral("logic and"));
    m_tokentype_name.insert(tt_logop_or,        QStringLiteral("logic or"));
    m_tokentype_name.insert(tt_ternary,         QStringLiteral("ternary"));
    m_tokentype_name.insert(tt_assignment,      QStringLiteral("assignment"));
    m_tokentype_name.insert(tt_conditional,     QStringLiteral("conditional"));
    m_tokentype_name.insert(tt_modcz_param,     QStringLiteral("modcz param"));
    m_tokentype_name.insert(tt_inst,            QStringLiteral("inst"));
    m_tokentype_name.insert(tt_inst,            QStringLiteral("wc/wz suffix"));
    m_tokentype_name.insert(tt_section,         QStringLiteral("section"));
    m_tokentype_name.insert(tt_origin,          QStringLiteral("origin"));
    m_tokentype_name.insert(tt_data,            QStringLiteral("data"));
    m_tokentype_name.insert(tt_lexer,           QStringLiteral("lexer"));
}

/**
 * @brief Return a QString for the given %token
 * @param token one of p2_toke_e enume
 * @return QString in uppercase or lowercase
 */
QString P2Token::string(p2_token_e tok, bool lowercase) const
{
    QString str = m_token_name.value(tok);
    return lowercase ? str.toLower() : str;
}

/**
 * @brief Return a p2_token_e enumeration value for the QString %str
 * @param str QString to scan for
 * @param chop if true, chop off characters until a a token other than t_nothing is matched
 * @param plen optional pointer to an int to receive the length of the token
 * @return p2_token_e enumeration value, or t_nothing if not a known string
 */
p2_token_e P2Token::token(const QString& str, bool chop, int* plen) const
{
    QString ustr = str.toUpper().simplified().remove(QChar::Space);

    // Ignore leading '#' characters (for immediate modes)
    while (ustr.startsWith(QChar('#')))
        ustr.remove(0, 1);

    // Ignore leading '@' characters (for relative modes)
    while (ustr.startsWith(QChar('@')))
        ustr.remove(0, 1);

    p2_token_e tok = m_name_token.value(ustr, t_unknown);
    for (;;) {
        if (t_unknown != tok)
            break;

        if (ustr.isEmpty())
            break;

        if (0 == ustr.indexOf(rx_loc_symbol)) {
            ustr.truncate(rx_loc_symbol.cap(0).length());
            tok = t_locsym;
            break;
        }

        if (0 == rx_symbol.indexIn(ustr)) {
            ustr.truncate(rx_symbol.cap(0).length());
            tok = t_symbol;
            break;
        }

        if (0 == ustr.indexOf(rx_bin)) {
            tok = t_bin_const;
            break;
        }

        if (0 == ustr.indexOf(rx_byt)) {
            tok = t_byt_const;
            break;
        }

        if (0 == ustr.indexOf(rx_oct)) {
            tok = t_oct_const;
            break;
        }

        if (0 == ustr.indexOf(rx_hex)) {
            tok = t_hex_const;
            break;
        }

        if (0 == ustr.indexOf(rx_dec)) {
            tok = t_dec_const;
            break;
        }

        if (0 == ustr.indexOf(rx_string)) {
            tok = t_string;
            break;
        }

        if (!chop)
            break;

        ustr.chop(1);
        tok = m_name_token.value(ustr, t_unknown);
    }
    if (plen) {
        *plen = ustr.length();
    }
    return tok;
}

/**
 * @brief Check if a token is of a specific type
 * @param tok token value
 * @param type token type
 * @return true if token type is set, or false otherwise
 */
bool P2Token::is_type(p2_token_e tok, p2_tokentype_e type) const
{
    const quint64 bits = m_token_type.value(tok, 0);
    const quint64 mask = TTMASK(type);
    return (bits & mask) ? true : false;
}

/**
 * @brief Check if a string is a token of a specific type
 * @param str string to tokenize
 * @param type token type
 * @return true if token type is set, or false otherwise
 */
bool P2Token::is_type(const QString& str, p2_tokentype_e type) const
{
    const p2_token_e tok = m_name_token.value(str);
    return is_type(tok, type);
}

/**
 * @brief Return the list of type names which are set for a token
 * @param tok token value
 * @return QStringList with the type names
 */
QStringList P2Token::typeNames(p2_token_e tok) const
{
    const quint64 mask = m_token_type.value(tok, 0);
    QStringList list;
    if (0 == mask) {
        list += m_tokentype_name.value(static_cast<p2_tokentype_e>(tt_none));
    } else {
        for (int i = 0; i <= 64; i++)
            if (mask & TTMASK(i))
                    list += m_tokentype_name.value(static_cast<p2_tokentype_e>(i));
    }
    return list;
}

/**
 * @brief Check if a string starting at pos contains a token from the list of tokens
 *
 * Side effect: if a matching token is found, %pos is incremented by its length.
 *
 * @param pos position where to start scanning
 * @param str string to inspect
 * @param tokens list of tokens to expect
 * @param dflt default value, if no token from the list is found
 * @return token value, or dflt if none found
 */
p2_token_e P2Token::at_token(int& pos, const QString& str, QList<p2_token_e> tokens, p2_token_e dflt) const
{
    int len = 0;
    p2_token_e tok = token(str.mid(pos), true, &len);
    if (t_unknown == tok)
        return dflt;
    if (tokens.contains(tok)) {
        pos += len;
    }
    return tok;
}

/**
 * @brief Check if a string contains a token from the list of tokens
 * @param str string to inspect
 * @param tokens list of tokens to expect
 * @param dflt default value, if no token from the list is found
 * @return token value, or dflt if none found
 */
p2_token_e P2Token::at_token(const QString& str, QList<p2_token_e> tokens, p2_token_e dflt) const
{
    int pos = 0;
    return at_token(pos, str, tokens, dflt);
}

/**
 * @brief Return true, if a token value is an operation
 * @param tok token value
 * @return true if operation, or flags otherwise
 */
bool P2Token::is_operation(p2_token_e tok) const
{
    // Bit mask for operations
    const quint64 ops = tm_operations;
    const quint64 mask = m_token_type.value(tok, 0);
    return (mask & ops) ? true : false;
}

/**
 * @brief Check if token is a conditional
 * @param tok token value
 * @return true if conditional, or false otherwise
 */
bool P2Token::is_conditional(p2_token_e tok) const
{
    return is_type(tok, tt_conditional);
}

/**
 * @brief Check if token is a MODCZ parameter
 * @param tok token value
 * @return true if MODCZ parameter, or false otherwise
 */
bool P2Token::is_modcz_param(p2_token_e tok) const
{
    return is_type(tok, tt_modcz_param);
}

/**
 * @brief Check if a string starting at pos contains a token of a token type set in the mask
 *
 * Side effect: if a matching token is found, %pos is incremented by its length.
 *
 * @param pos position where to start scanning
 * @param str string to inspect
 * @param typemask types to expect
 * @param dflt default token value to return if not found
 * @return token value if found, or dflt otherwise
 */
p2_token_e P2Token::at_type(int& pos, const QString& str, quint64 typemask, p2_token_e dflt) const
{
    int len = 0;
    p2_token_e tok = token(str.mid(pos), true, &len);

    if (t_unknown == tok)
        return dflt;

    if (tt_chk(tok, typemask)) {
        pos += len;
    } else {
        tok = dflt;
    }

    return tok;
}

/**
 * @brief Check if a string starting at pos contains a token of a specific token type
 *
 * Side effect: if a matching token is found, %pos is incremented by its length.
 *
 * @param pos position where to start scanning
 * @param str string to inspect
 * @param type type to expect
 * @param dflt default token value to return if not found
 * @return token value if found, or dflt otherwise
 */
p2_token_e P2Token::at_type(int& pos, const QString& str, p2_tokentype_e type, p2_token_e dflt) const
{
    int len = 0;
    p2_token_e tok = token(str.mid(pos), true, &len);

    if (t_unknown == tok)
        return dflt;

    if (tt_chk(tok, type)) {
        pos += len;
    } else {
        tok = dflt;
    }

    return tok;
}


/**
 * @brief Check if a string starts with a token of a specific token type
 * @param str string to inspect
 * @param type type to expect
 * @param dflt default token value to return if not found
 * @return token value if found, or dflt otherwise
 */
p2_token_e P2Token::at_type(const QString& str, p2_tokentype_e type, p2_token_e dflt) const
{
    int pos = 0;
    return at_type(pos, str, type, dflt);
}

/**
 * @brief Check if a string starting at pos contains a token of a type found in a list of token types
 *
 * Side effect: if a matching token is found, pos is incremented by its length.
 *
 * @param pos position where to start scanning
 * @param str string to inspect
 * @param types token types to expect
 * @param dflt default token value to return if not found
 * @return token value if found, or dflt otherwise
 */

p2_token_e P2Token::at_types(int& pos, const QString& str, quint64 typemask, p2_token_e dflt) const
{
    int len = 0;
    p2_token_e tok = token(str.mid(pos), true, &len);

    if (t_unknown == tok)
        return dflt;

    // Build bit mask for types

    if (tt_chk(tok, typemask)) {
        pos += len;
    } else {
        tok = dflt;
    }

    return tok;
}

/**
 * @brief Check if a string starting at pos contains a token of a type found in a list of token types
 *
 * Side effect: if a matching token is found, pos is incremented by its length.
 *
 * @param pos position where to start scanning
 * @param str string to inspect
 * @param types token types to expect
 * @param dflt default token value to return if not found
 * @return token value if found, or dflt otherwise
 */
p2_token_e P2Token::at_types(int& pos, const QString& str, const QList<p2_tokentype_e>& types, p2_token_e dflt) const
{
    // Build bit mask for types
    quint64 typemask = tm_none;
    foreach(const p2_tokentype_e type, types)
        typemask |= TTMASK(type);
    return at_types(pos, str, typemask, dflt);
}

/**
 * @brief Check if a string starts with a token of a type found in a list of token types
 * @param str string to inspect
 * @param types token types to expect
 * @param dflt default token value to return if not found
 * @return token value if found, or dflt otherwise
 */
p2_token_e P2Token::at_types(const QString& str, const QList<p2_tokentype_e>& types, p2_token_e dflt) const
{
    int pos = 0;
    return at_types(pos, str, types, dflt);
}

/**
 * @brief Check if a string starting at pos contains a token of type tt_conditional
 *
 * Side effect: if a matching token is found, pos is incremented by its length.
 *
 * @param pos position where to start scanning
 * @param str string to inspect
 * @param dflt default token value to return if not found
 * @return token value if found, or dflt otherwise
 */
p2_token_e P2Token::at_conditional(int& pos, const QString& str, p2_token_e dflt) const
{
    return at_type(pos, str, tt_conditional, dflt);
}

/**
 * @brief Check if a string starts with a token of type tt_conditional
 * @param str string to inspect
 * @param dflt default token value to return if not found
 * @return token value if found, or dflt otherwise
 */
p2_token_e P2Token::at_conditional(const QString& str, p2_token_e dflt) const
{
    int pos = 0;
    return at_type(pos, str, tt_conditional, dflt);
}

/**
 * @brief Check if a string starting at pos contains a token of type tt_modcz_param
 *
 * Side effect: if a matching token is found, pos is incremented by its length.
 *
 * @param pos position where to start scanning
 * @param str string to inspect
 * @param dflt default token value to return if not found
 * @return token value if found, or dflt otherwise
 */
p2_token_e P2Token::at_modcz_param(int& pos, const QString& str, p2_token_e dflt) const
{
    return at_type(pos, str, tt_modcz_param, dflt);
}

/**
 * @brief Check if a string starts with a token of type tt_modcz_param
 * @param str string to inspect
 * @param dflt default token value to return if not found
 * @return token value if found, or dflt otherwise
 */
p2_token_e P2Token::at_modcz_param(const QString& str, p2_token_e dflt) const
{
    int pos = 0;
    return at_type(pos, str, tt_modcz_param, dflt);

}

/**
 * @brief Return the value for a conditional execution token
 * @param cond token with the condition
 * @param dflt default condition if token is not a conditional
 * @return One of the 16 p2_cond_e values
 */
p2_cond_e P2Token::conditional(p2_token_e cond, p2_cond_e dflt) const
{
    return m_lookup_cond.value(cond, dflt);
}

/**
 * @brief Return the value for a conditional execution string
 * @param str string with the condition
 * @param dflt default condition if string is not a conditional
 * @return One of the 16 p2_cond_e values
 */
p2_cond_e P2Token::conditional(const QString& str, p2_cond_e dflt) const
{
    int pos = 0;
    p2_token_e cond = at_type(pos, str, tt_conditional);
    return m_lookup_cond.value(cond, dflt);
}

/**
 * @brief Return the value for a MODCZ parameter token
 * @param cond token with the parameter
 * @param dflt default condition if token is not a MODCZ parameter
 * @return One of the 16 p2_cond_e values
 */
p2_cond_e P2Token::modcz_param(p2_token_e cond, p2_cond_e dflt) const
{
    return m_lookup_modcz.value(cond, dflt);
}

/**
 * @brief Return the value for a MODCZ parameter token
 * @param str string with the parameter
 * @param dflt default parameter if string is not a MODCZ parameter
 * @return One of the 16 p2_cond_e values
 */
p2_cond_e P2Token::modcz_param(const QString& str, p2_cond_e dflt) const
{
    int pos = 0;
    p2_token_e cond = at_type(pos, str, tt_modcz_param);
    return m_lookup_modcz.value(cond, dflt);
}

/**
 * @brief Add a token to the m_token_nam hash and its type to the m_token_type bitmask
 * @param tok token value
 * @param type token type value
 * @param str string
 */
void P2Token::tn_add(p2_token_e tok, p2_tokentype_e type, const QString& str)
{
    m_token_name.insert(tok, str);
    tt_set(tok, type);
}

/**
 * @brief Add a token to the m_token_nam hash and its types to the typemask
 * @param tok token value
 * @param typemask token type bitmask
 * @param str string
 */
void P2Token::tn_add(p2_token_e tok, quint64 typemask, const QString& str)
{
    m_token_name.insert(tok, str);
    tt_set(tok, typemask);
}

void P2Token::tt_set(p2_token_e tok, quint64 typemask)
{
    quint64 mask = m_token_type.value(tok, 0) | typemask;
    m_token_type.insert(tok, mask);
}

void P2Token::tt_clr(p2_token_e tok, quint64 typemask)
{
    quint64 mask = m_token_type.value(tok, 0) & ~typemask;
    m_token_type.insert(tok, mask);
}

bool P2Token::tt_chk(p2_token_e tok, quint64 typemask) const
{
    return m_token_type.value(tok, 0) & typemask ? true : false;
}

void P2Token::tt_set(p2_token_e tok, p2_tokentype_e type)
{
    tt_set(tok, TTMASK(type));
}

void P2Token::tt_clr(p2_token_e tok, p2_tokentype_e type)
{
    tt_clr(tok, TTMASK(type));
}

bool P2Token::tt_chk(p2_token_e tok, p2_tokentype_e type) const
{
    return tt_chk(tok, TTMASK(type));
}
