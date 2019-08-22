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
static const QString re_local = QStringLiteral("\\.[A-Z_][A-Z0-9_]*");

/**
 * @brief Regular expression for alphanumeric
 *
 * leading "A"…"Z", or "_"
 * then any number of "A"…"Z", "0"…"9", or "_"
 */
static const QString re_alnum = QStringLiteral("[A-Z_][A-Z0-9_]*");

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
    , rx_loc_symbol(QString("^%1$").arg(re_local))
    , rx_symbol(QString("^%1$").arg(re_alnum))
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

    m_token_name.insert(t_ABS,              QStringLiteral("ABS"));
    m_token_name.insert(t_ADD,              QStringLiteral("ADD"));
    m_token_name.insert(t_ADDCT1,           QStringLiteral("ADDCT1"));
    m_token_name.insert(t_ADDCT2,           QStringLiteral("ADDCT2"));
    m_token_name.insert(t_ADDCT3,           QStringLiteral("ADDCT3"));
    m_token_name.insert(t_ADDPIX,           QStringLiteral("ADDPIX"));
    m_token_name.insert(t_ADDS,             QStringLiteral("ADDS"));
    m_token_name.insert(t_ADDSX,            QStringLiteral("ADDSX"));
    m_token_name.insert(t_ADDX,             QStringLiteral("ADDX"));
    m_token_name.insert(t_AKPIN,            QStringLiteral("AKPIN"));
    m_token_name.insert(t_ALLOWI,           QStringLiteral("ALLOWI"));
    m_token_name.insert(t_ALTB,             QStringLiteral("ALTB"));
    m_token_name.insert(t_ALTD,             QStringLiteral("ALTD"));
    m_token_name.insert(t_ALTGB,            QStringLiteral("ALTGB"));
    m_token_name.insert(t_ALTGN,            QStringLiteral("ALTGN"));
    m_token_name.insert(t_ALTGW,            QStringLiteral("ALTGW"));
    m_token_name.insert(t_ALTI,             QStringLiteral("ALTI"));
    m_token_name.insert(t_ALTR,             QStringLiteral("ALTR"));
    m_token_name.insert(t_ALTS,             QStringLiteral("ALTS"));
    m_token_name.insert(t_ALTSB,            QStringLiteral("ALTSB"));
    m_token_name.insert(t_ALTSN,            QStringLiteral("ALTSN"));
    m_token_name.insert(t_ALTSW,            QStringLiteral("ALTSW"));
    m_token_name.insert(t_AND,              QStringLiteral("AND"));
    m_token_name.insert(t_ANDN,             QStringLiteral("ANDN"));
    m_token_name.insert(t_AUGD,             QStringLiteral("AUGD"));
    m_token_name.insert(t_AUGS,             QStringLiteral("AUGS"));
    m_token_name.insert(t_BITC,             QStringLiteral("BITC"));
    m_token_name.insert(t_BITH,             QStringLiteral("BITH"));
    m_token_name.insert(t_BITL,             QStringLiteral("BITL"));
    m_token_name.insert(t_BITNC,            QStringLiteral("BITNC"));
    m_token_name.insert(t_BITNOT,           QStringLiteral("BITNOT"));
    m_token_name.insert(t_BITNZ,            QStringLiteral("BITNZ"));
    m_token_name.insert(t_BITRND,           QStringLiteral("BITRND"));
    m_token_name.insert(t_BITZ,             QStringLiteral("BITZ"));
    m_token_name.insert(t_BLNPIX,           QStringLiteral("BLNPIX"));
    m_token_name.insert(t_BMASK,            QStringLiteral("BMASK"));
    m_token_name.insert(t_BRK,              QStringLiteral("BRK"));
    m_token_name.insert(t_CALL,             QStringLiteral("CALL"));
    m_token_name.insert(t_CALLA,            QStringLiteral("CALLA"));
    m_token_name.insert(t_CALLB,            QStringLiteral("CALLB"));
    m_token_name.insert(t_CALLD,            QStringLiteral("CALLD"));
    m_token_name.insert(t_CALLPA,           QStringLiteral("CALLPA"));
    m_token_name.insert(t_CALLPB,           QStringLiteral("CALLPB"));
    m_token_name.insert(t_CMP,              QStringLiteral("CMP"));
    m_token_name.insert(t_CMPM,             QStringLiteral("CMPM"));
    m_token_name.insert(t_CMPR,             QStringLiteral("CMPR"));
    m_token_name.insert(t_CMPS,             QStringLiteral("CMPS"));
    m_token_name.insert(t_CMPSUB,           QStringLiteral("CMPSUB"));
    m_token_name.insert(t_CMPSX,            QStringLiteral("CMPSX"));
    m_token_name.insert(t_CMPX,             QStringLiteral("CMPX"));
    m_token_name.insert(t_COGATN,           QStringLiteral("COGATN"));
    m_token_name.insert(t_COGBRK,           QStringLiteral("COGBRK"));
    m_token_name.insert(t_COGID,            QStringLiteral("COGID"));
    m_token_name.insert(t_COGINIT,          QStringLiteral("COGINIT"));
    m_token_name.insert(t_COGSTOP,          QStringLiteral("COGSTOP"));
    m_token_name.insert(t_CRCBIT,           QStringLiteral("CRCBIT"));
    m_token_name.insert(t_CRCNIB,           QStringLiteral("CRCNIB"));
    m_token_name.insert(t_DECMOD,           QStringLiteral("DECMOD"));
    m_token_name.insert(t_DECOD,            QStringLiteral("DECOD"));
    m_token_name.insert(t_DIRC,             QStringLiteral("DIRC"));
    m_token_name.insert(t_DIRH,             QStringLiteral("DIRH"));
    m_token_name.insert(t_DIRL,             QStringLiteral("DIRL"));
    m_token_name.insert(t_DIRNC,            QStringLiteral("DIRNC"));
    m_token_name.insert(t_DIRNOT,           QStringLiteral("DIRNOT"));
    m_token_name.insert(t_DIRNZ,            QStringLiteral("DIRNZ"));
    m_token_name.insert(t_DIRRND,           QStringLiteral("DIRRND"));
    m_token_name.insert(t_DIRZ,             QStringLiteral("DIRZ"));
    m_token_name.insert(t_DJF,              QStringLiteral("DJF"));
    m_token_name.insert(t_DJNF,             QStringLiteral("DJNF"));
    m_token_name.insert(t_DJNZ,             QStringLiteral("DJNZ"));
    m_token_name.insert(t_DJZ,              QStringLiteral("DJZ"));
    m_token_name.insert(t_DRVC,             QStringLiteral("DRVC"));
    m_token_name.insert(t_DRVH,             QStringLiteral("DRVH"));
    m_token_name.insert(t_DRVL,             QStringLiteral("DRVL"));
    m_token_name.insert(t_DRVNC,            QStringLiteral("DRVNC"));
    m_token_name.insert(t_DRVNOT,           QStringLiteral("DRVNOT"));
    m_token_name.insert(t_DRVNZ,            QStringLiteral("DRVNZ"));
    m_token_name.insert(t_DRVRND,           QStringLiteral("DRVRND"));
    m_token_name.insert(t_DRVZ,             QStringLiteral("DRVZ"));
    m_token_name.insert(t_ENCOD,            QStringLiteral("ENCOD"));
    m_token_name.insert(t_EXECF,            QStringLiteral("EXECF"));
    m_token_name.insert(t_FBLOCK,           QStringLiteral("FBLOCK"));
    m_token_name.insert(t_FGE,              QStringLiteral("FGE"));
    m_token_name.insert(t_FGES,             QStringLiteral("FGES"));
    m_token_name.insert(t_FLE,              QStringLiteral("FLE"));
    m_token_name.insert(t_FLES,             QStringLiteral("FLES"));
    m_token_name.insert(t_FLTC,             QStringLiteral("FLTC"));
    m_token_name.insert(t_FLTH,             QStringLiteral("FLTH"));
    m_token_name.insert(t_FLTL,             QStringLiteral("FLTL"));
    m_token_name.insert(t_FLTNC,            QStringLiteral("FLTNC"));
    m_token_name.insert(t_FLTNOT,           QStringLiteral("FLTNOT"));
    m_token_name.insert(t_FLTNZ,            QStringLiteral("FLTNZ"));
    m_token_name.insert(t_FLTRND,           QStringLiteral("FLTRND"));
    m_token_name.insert(t_FLTZ,             QStringLiteral("FLTZ"));
    m_token_name.insert(t_GETBRK,           QStringLiteral("GETBRK"));
    m_token_name.insert(t_GETBYTE,          QStringLiteral("GETBYTE"));
    m_token_name.insert(t_GETCT,            QStringLiteral("GETCT"));
    m_token_name.insert(t_GETNIB,           QStringLiteral("GETNIB"));
    m_token_name.insert(t_GETPTR,           QStringLiteral("GETPTR"));
    m_token_name.insert(t_GETQX,            QStringLiteral("GETQX"));
    m_token_name.insert(t_GETQY,            QStringLiteral("GETQY"));
    m_token_name.insert(t_GETRND,           QStringLiteral("GETRND"));
    m_token_name.insert(t_GETSCP,           QStringLiteral("GETSCP"));
    m_token_name.insert(t_GETWORD,          QStringLiteral("GETWORD"));
    m_token_name.insert(t_GETXACC,          QStringLiteral("GETXACC"));
    m_token_name.insert(t_HUBSET,           QStringLiteral("HUBSET"));
    m_token_name.insert(t_IJNZ,             QStringLiteral("IJNZ"));
    m_token_name.insert(t_IJZ,              QStringLiteral("IJZ"));
    m_token_name.insert(t_INCMOD,           QStringLiteral("INCMOD"));
    m_token_name.insert(t_JATN,             QStringLiteral("JATN"));
    m_token_name.insert(t_JCT1,             QStringLiteral("JCT1"));
    m_token_name.insert(t_JCT2,             QStringLiteral("JCT2"));
    m_token_name.insert(t_JCT3,             QStringLiteral("JCT3"));
    m_token_name.insert(t_JFBW,             QStringLiteral("JFBW"));
    m_token_name.insert(t_JINT,             QStringLiteral("JINT"));
    m_token_name.insert(t_JMP,              QStringLiteral("JMP"));
    m_token_name.insert(t_JMPREL,           QStringLiteral("JMPREL"));
    m_token_name.insert(t_JNATN,            QStringLiteral("JNATN"));
    m_token_name.insert(t_JNCT1,            QStringLiteral("JNCT1"));
    m_token_name.insert(t_JNCT2,            QStringLiteral("JNCT2"));
    m_token_name.insert(t_JNCT3,            QStringLiteral("JNCT3"));
    m_token_name.insert(t_JNFBW,            QStringLiteral("JNFBW"));
    m_token_name.insert(t_JNINT,            QStringLiteral("JNINT"));
    m_token_name.insert(t_JNPAT,            QStringLiteral("JNPAT"));
    m_token_name.insert(t_JNQMT,            QStringLiteral("JNQMT"));
    m_token_name.insert(t_JNSE1,            QStringLiteral("JNSE1"));
    m_token_name.insert(t_JNSE2,            QStringLiteral("JNSE2"));
    m_token_name.insert(t_JNSE3,            QStringLiteral("JNSE3"));
    m_token_name.insert(t_JNSE4,            QStringLiteral("JNSE4"));
    m_token_name.insert(t_JNXFI,            QStringLiteral("JNXFI"));
    m_token_name.insert(t_JNXMT,            QStringLiteral("JNXMT"));
    m_token_name.insert(t_JNXRL,            QStringLiteral("JNXRL"));
    m_token_name.insert(t_JNXRO,            QStringLiteral("JNXRO"));
    m_token_name.insert(t_JPAT,             QStringLiteral("JPAT"));
    m_token_name.insert(t_JQMT,             QStringLiteral("JQMT"));
    m_token_name.insert(t_JSE1,             QStringLiteral("JSE1"));
    m_token_name.insert(t_JSE2,             QStringLiteral("JSE2"));
    m_token_name.insert(t_JSE3,             QStringLiteral("JSE3"));
    m_token_name.insert(t_JSE4,             QStringLiteral("JSE4"));
    m_token_name.insert(t_JXFI,             QStringLiteral("JXFI"));
    m_token_name.insert(t_JXMT,             QStringLiteral("JXMT"));
    m_token_name.insert(t_JXRL,             QStringLiteral("JXRL"));
    m_token_name.insert(t_JXRO,             QStringLiteral("JXRO"));
    m_token_name.insert(t_LOC,              QStringLiteral("LOC"));
    m_token_name.insert(t_LOCKNEW,          QStringLiteral("LOCKNEW"));
    m_token_name.insert(t_LOCKREL,          QStringLiteral("LOCKREL"));
    m_token_name.insert(t_LOCKRET,          QStringLiteral("LOCKRET"));
    m_token_name.insert(t_LOCKTRY,          QStringLiteral("LOCKTRY"));
    m_token_name.insert(t_MERGEB,           QStringLiteral("MERGEB"));
    m_token_name.insert(t_MERGEW,           QStringLiteral("MERGEW"));
    m_token_name.insert(t_MIXPIX,           QStringLiteral("MIXPIX"));
    m_token_name.insert(t_MODCZ,            QStringLiteral("MODCZ"));
    m_token_name.insert(t_MOV,              QStringLiteral("MOV"));
    m_token_name.insert(t_MOVBYTS,          QStringLiteral("MOVBYTS"));
    m_token_name.insert(t_MUL,              QStringLiteral("MUL"));
    m_token_name.insert(t_MULPIX,           QStringLiteral("MULPIX"));
    m_token_name.insert(t_MULS,             QStringLiteral("MULS"));
    m_token_name.insert(t_MUXC,             QStringLiteral("MUXC"));
    m_token_name.insert(t_MUXNC,            QStringLiteral("MUXNC"));
    m_token_name.insert(t_MUXNIBS,          QStringLiteral("MUXNIBS"));
    m_token_name.insert(t_MUXNITS,          QStringLiteral("MUXNITS"));
    m_token_name.insert(t_MUXNZ,            QStringLiteral("MUXNZ"));
    m_token_name.insert(t_MUXQ,             QStringLiteral("MUXQ"));
    m_token_name.insert(t_MUXZ,             QStringLiteral("MUXZ"));
    m_token_name.insert(t_NEG,              QStringLiteral("NEG"));
    m_token_name.insert(t_NEGC,             QStringLiteral("NEGC"));
    m_token_name.insert(t_NEGNC,            QStringLiteral("NEGNC"));
    m_token_name.insert(t_NEGNZ,            QStringLiteral("NEGNZ"));
    m_token_name.insert(t_NEGZ,             QStringLiteral("NEGZ"));
    m_token_name.insert(t_NIXINT1,          QStringLiteral("NIXINT1"));
    m_token_name.insert(t_NIXINT2,          QStringLiteral("NIXINT2"));
    m_token_name.insert(t_NIXINT3,          QStringLiteral("NIXINT3"));
    m_token_name.insert(t_NOP,              QStringLiteral("NOP"));
    m_token_name.insert(t_NOT,              QStringLiteral("NOT"));
    m_token_name.insert(t_ONES,             QStringLiteral("ONES"));
    m_token_name.insert(t_OR,               QStringLiteral("OR"));
    m_token_name.insert(t_OUTC,             QStringLiteral("OUTC"));
    m_token_name.insert(t_OUTH,             QStringLiteral("OUTH"));
    m_token_name.insert(t_OUTL,             QStringLiteral("OUTL"));
    m_token_name.insert(t_OUTNC,            QStringLiteral("OUTNC"));
    m_token_name.insert(t_OUTNOT,           QStringLiteral("OUTNOT"));
    m_token_name.insert(t_OUTNZ,            QStringLiteral("OUTNZ"));
    m_token_name.insert(t_OUTRND,           QStringLiteral("OUTRND"));
    m_token_name.insert(t_OUTZ,             QStringLiteral("OUTZ"));
    m_token_name.insert(t_PA,               QStringLiteral("PA"));
    m_token_name.insert(t_PB,               QStringLiteral("PB"));
    m_token_name.insert(t_POLLATN,          QStringLiteral("POLLATN"));
    m_token_name.insert(t_POLLCT1,          QStringLiteral("POLLCT1"));
    m_token_name.insert(t_POLLCT2,          QStringLiteral("POLLCT2"));
    m_token_name.insert(t_POLLCT3,          QStringLiteral("POLLCT3"));
    m_token_name.insert(t_POLLFBW,          QStringLiteral("POLLFBW"));
    m_token_name.insert(t_POLLINT,          QStringLiteral("POLLINT"));
    m_token_name.insert(t_POLLPAT,          QStringLiteral("POLLPAT"));
    m_token_name.insert(t_POLLQMT,          QStringLiteral("POLLQMT"));
    m_token_name.insert(t_POLLSE1,          QStringLiteral("POLLSE1"));
    m_token_name.insert(t_POLLSE2,          QStringLiteral("POLLSE2"));
    m_token_name.insert(t_POLLSE3,          QStringLiteral("POLLSE3"));
    m_token_name.insert(t_POLLSE4,          QStringLiteral("POLLSE4"));
    m_token_name.insert(t_POLLXFI,          QStringLiteral("POLLXFI"));
    m_token_name.insert(t_POLLXMT,          QStringLiteral("POLLXMT"));
    m_token_name.insert(t_POLLXRL,          QStringLiteral("POLLXRL"));
    m_token_name.insert(t_POLLXRO,          QStringLiteral("POLLXRO"));
    m_token_name.insert(t_POP,              QStringLiteral("POP"));
    m_token_name.insert(t_POPA,             QStringLiteral("POPA"));
    m_token_name.insert(t_POPB,             QStringLiteral("POPB"));
    m_token_name.insert(t_PTRA,             QStringLiteral("PTRA"));
    m_token_name.insert(t_PTRA_postinc,     QStringLiteral("PTRA++"));
    m_token_name.insert(t_PTRA_postdec,     QStringLiteral("PTRA--"));
    m_token_name.insert(t_PTRA_preinc,      QStringLiteral("++PTRA"));
    m_token_name.insert(t_PTRA_predec,      QStringLiteral("--PTRA"));
    m_token_name.insert(t_PTRB,             QStringLiteral("PTRB"));
    m_token_name.insert(t_PTRB_postinc,     QStringLiteral("PTRB++"));
    m_token_name.insert(t_PTRB_postdec,     QStringLiteral("PTRB--"));
    m_token_name.insert(t_PTRB_preinc,      QStringLiteral("++PTRB"));
    m_token_name.insert(t_PTRB_predec,      QStringLiteral("--PTRB"));
    m_token_name.insert(t_PUSH,             QStringLiteral("PUSH"));
    m_token_name.insert(t_PUSHA,            QStringLiteral("PUSHA"));
    m_token_name.insert(t_PUSHB,            QStringLiteral("PUSHB"));
    m_token_name.insert(t_QDIV,             QStringLiteral("QDIV"));
    m_token_name.insert(t_QEXP,             QStringLiteral("QEXP"));
    m_token_name.insert(t_QFRAC,            QStringLiteral("QFRAC"));
    m_token_name.insert(t_QLOG,             QStringLiteral("QLOG"));
    m_token_name.insert(t_QMUL,             QStringLiteral("QMUL"));
    m_token_name.insert(t_QROTATE,          QStringLiteral("QROTATE"));
    m_token_name.insert(t_QSQRT,            QStringLiteral("QSQRT"));
    m_token_name.insert(t_QVECTOR,          QStringLiteral("QVECTOR"));
    m_token_name.insert(t_RCL,              QStringLiteral("RCL"));
    m_token_name.insert(t_RCR,              QStringLiteral("RCR"));
    m_token_name.insert(t_RCZL,             QStringLiteral("RCZL"));
    m_token_name.insert(t_RCZR,             QStringLiteral("RCZR"));
    m_token_name.insert(t_RDBYTE,           QStringLiteral("RDBYTE"));
    m_token_name.insert(t_RDFAST,           QStringLiteral("RDFAST"));
    m_token_name.insert(t_RDLONG,           QStringLiteral("RDLONG"));
    m_token_name.insert(t_RDLUT,            QStringLiteral("RDLUT"));
    m_token_name.insert(t_RDPIN,            QStringLiteral("RDPIN"));
    m_token_name.insert(t_RDWORD,           QStringLiteral("RDWORD"));
    m_token_name.insert(t_REP,              QStringLiteral("REP"));
    m_token_name.insert(t_RESI0,            QStringLiteral("RESI0"));
    m_token_name.insert(t_RESI1,            QStringLiteral("RESI1"));
    m_token_name.insert(t_RESI2,            QStringLiteral("RESI2"));
    m_token_name.insert(t_RESI3,            QStringLiteral("RESI3"));
    m_token_name.insert(t_RET,              QStringLiteral("RET"));
    m_token_name.insert(t_RETA,             QStringLiteral("RETA"));
    m_token_name.insert(t_RETB,             QStringLiteral("RETB"));
    m_token_name.insert(t_RETI0,            QStringLiteral("RETI0"));
    m_token_name.insert(t_RETI1,            QStringLiteral("RETI1"));
    m_token_name.insert(t_RETI2,            QStringLiteral("RETI2"));
    m_token_name.insert(t_RETI3,            QStringLiteral("RETI3"));
    m_token_name.insert(t_REV,              QStringLiteral("REV"));
    m_token_name.insert(t_RFBYTE,           QStringLiteral("RFBYTE"));
    m_token_name.insert(t_RFLONG,           QStringLiteral("RFLONG"));
    m_token_name.insert(t_RFVAR,            QStringLiteral("RFVAR"));
    m_token_name.insert(t_RFVARS,           QStringLiteral("RFVARS"));
    m_token_name.insert(t_RFWORD,           QStringLiteral("RFWORD"));
    m_token_name.insert(t_RGBEXP,           QStringLiteral("RGBEXP"));
    m_token_name.insert(t_RGBSQZ,           QStringLiteral("RGBSQZ"));
    m_token_name.insert(t_ROL,              QStringLiteral("ROL"));
    m_token_name.insert(t_ROLBYTE,          QStringLiteral("ROLBYTE"));
    m_token_name.insert(t_ROLNIB,           QStringLiteral("ROLNIB"));
    m_token_name.insert(t_ROLWORD,          QStringLiteral("ROLWORD"));
    m_token_name.insert(t_ROR,              QStringLiteral("ROR"));
    m_token_name.insert(t_RQPIN,            QStringLiteral("RQPIN"));
    m_token_name.insert(t_SAL,              QStringLiteral("SAL"));
    m_token_name.insert(t_SAR,              QStringLiteral("SAR"));
    m_token_name.insert(t_SCA,              QStringLiteral("SCA"));
    m_token_name.insert(t_SCAS,             QStringLiteral("SCAS"));
    m_token_name.insert(t_SETBYTE,          QStringLiteral("SETBYTE"));
    m_token_name.insert(t_SETCFRQ,          QStringLiteral("SETCFRQ"));
    m_token_name.insert(t_SETCI,            QStringLiteral("SETCI"));
    m_token_name.insert(t_SETCMOD,          QStringLiteral("SETCMOD"));
    m_token_name.insert(t_SETCQ,            QStringLiteral("SETCQ"));
    m_token_name.insert(t_SETCY,            QStringLiteral("SETCY"));
    m_token_name.insert(t_SETD,             QStringLiteral("SETD"));
    m_token_name.insert(t_SETDACS,          QStringLiteral("SETDACS"));
    m_token_name.insert(t_SETINT1,          QStringLiteral("SETINT1"));
    m_token_name.insert(t_SETINT2,          QStringLiteral("SETINT2"));
    m_token_name.insert(t_SETINT3,          QStringLiteral("SETINT3"));
    m_token_name.insert(t_SETLUTS,          QStringLiteral("SETLUTS"));
    m_token_name.insert(t_SETNIB,           QStringLiteral("SETNIB"));
    m_token_name.insert(t_SETPAT,           QStringLiteral("SETPAT"));
    m_token_name.insert(t_SETPIV,           QStringLiteral("SETPIV"));
    m_token_name.insert(t_SETPIX,           QStringLiteral("SETPIX"));
    m_token_name.insert(t_SETQ,             QStringLiteral("SETQ"));
    m_token_name.insert(t_SETQ2,            QStringLiteral("SETQ2"));
    m_token_name.insert(t_SETR,             QStringLiteral("SETR"));
    m_token_name.insert(t_SETS,             QStringLiteral("SETS"));
    m_token_name.insert(t_SETSCP,           QStringLiteral("SETSCP"));
    m_token_name.insert(t_SETSE1,           QStringLiteral("SETSE1"));
    m_token_name.insert(t_SETSE2,           QStringLiteral("SETSE2"));
    m_token_name.insert(t_SETSE3,           QStringLiteral("SETSE3"));
    m_token_name.insert(t_SETSE4,           QStringLiteral("SETSE4"));
    m_token_name.insert(t_SETWORD,          QStringLiteral("SETWORD"));
    m_token_name.insert(t_SETXFRQ,          QStringLiteral("SETXFRQ"));
    m_token_name.insert(t_SEUSSF,           QStringLiteral("SEUSSF"));
    m_token_name.insert(t_SEUSSR,           QStringLiteral("SEUSSR"));
    m_token_name.insert(t_SHL,              QStringLiteral("SHL"));
    m_token_name.insert(t_SHR,              QStringLiteral("SHR"));
    m_token_name.insert(t_SIGNX,            QStringLiteral("SIGNX"));
    m_token_name.insert(t_SKIP,             QStringLiteral("SKIP"));
    m_token_name.insert(t_SKIPF,            QStringLiteral("SKIPF"));
    m_token_name.insert(t_SPACE,            QStringLiteral("SPACE"));
    m_token_name.insert(t_SPLITB,           QStringLiteral("SPLITB"));
    m_token_name.insert(t_SPLITW,           QStringLiteral("SPLITW"));
    m_token_name.insert(t_STALLI,           QStringLiteral("STALLI"));
    m_token_name.insert(t_SUB,              QStringLiteral("SUB"));
    m_token_name.insert(t_SUBR,             QStringLiteral("SUBR"));
    m_token_name.insert(t_SUBS,             QStringLiteral("SUBS"));
    m_token_name.insert(t_SUBSX,            QStringLiteral("SUBSX"));
    m_token_name.insert(t_SUBX,             QStringLiteral("SUBX"));
    m_token_name.insert(t_SUMC,             QStringLiteral("SUMC"));
    m_token_name.insert(t_SUMNC,            QStringLiteral("SUMNC"));
    m_token_name.insert(t_SUMNZ,            QStringLiteral("SUMNZ"));
    m_token_name.insert(t_SUMZ,             QStringLiteral("SUMZ"));
    m_token_name.insert(t_TEST,             QStringLiteral("TEST"));
    m_token_name.insert(t_TESTB,            QStringLiteral("TESTB"));
    m_token_name.insert(t_TESTBN,           QStringLiteral("TESTBN"));
    m_token_name.insert(t_TESTN,            QStringLiteral("TESTN"));
    m_token_name.insert(t_TESTP,            QStringLiteral("TESTP"));
    m_token_name.insert(t_TESTPN,           QStringLiteral("TESTPN"));
    m_token_name.insert(t_TJF,              QStringLiteral("TJF"));
    m_token_name.insert(t_TJNF,             QStringLiteral("TJNF"));
    m_token_name.insert(t_TJNS,             QStringLiteral("TJNS"));
    m_token_name.insert(t_TJNZ,             QStringLiteral("TJNZ"));
    m_token_name.insert(t_TJS,              QStringLiteral("TJS"));
    m_token_name.insert(t_TJV,              QStringLiteral("TJV"));
    m_token_name.insert(t_TJZ,              QStringLiteral("TJZ"));
    m_token_name.insert(t_TRGINT1,          QStringLiteral("TRGINT1"));
    m_token_name.insert(t_TRGINT2,          QStringLiteral("TRGINT2"));
    m_token_name.insert(t_TRGINT3,          QStringLiteral("TRGINT3"));
    m_token_name.insert(t_WAITATN,          QStringLiteral("WAITATN"));
    m_token_name.insert(t_WAITCT1,          QStringLiteral("WAITCT1"));
    m_token_name.insert(t_WAITCT2,          QStringLiteral("WAITCT2"));
    m_token_name.insert(t_WAITCT3,          QStringLiteral("WAITCT3"));
    m_token_name.insert(t_WAITFBW,          QStringLiteral("WAITFBW"));
    m_token_name.insert(t_WAITINT,          QStringLiteral("WAITINT"));
    m_token_name.insert(t_WAITPAT,          QStringLiteral("WAITPAT"));
    m_token_name.insert(t_WAITSE1,          QStringLiteral("WAITSE1"));
    m_token_name.insert(t_WAITSE2,          QStringLiteral("WAITSE2"));
    m_token_name.insert(t_WAITSE3,          QStringLiteral("WAITSE3"));
    m_token_name.insert(t_WAITSE4,          QStringLiteral("WAITSE4"));
    m_token_name.insert(t_WAITX,            QStringLiteral("WAITX"));
    m_token_name.insert(t_WAITXFI,          QStringLiteral("WAITXFI"));
    m_token_name.insert(t_WAITXMT,          QStringLiteral("WAITXMT"));
    m_token_name.insert(t_WAITXRL,          QStringLiteral("WAITXRL"));
    m_token_name.insert(t_WAITXRO,          QStringLiteral("WAITXRO"));
    m_token_name.insert(t_WFBYTE,           QStringLiteral("WFBYTE"));
    m_token_name.insert(t_WFLONG,           QStringLiteral("WFLONG"));
    m_token_name.insert(t_WFWORD,           QStringLiteral("WFWORD"));
    m_token_name.insert(t_WMLONG,           QStringLiteral("WMLONG"));
    m_token_name.insert(t_WRBYTE,           QStringLiteral("WRBYTE"));
    m_token_name.insert(t_WRC,              QStringLiteral("WRC"));
    m_token_name.insert(t_WRFAST,           QStringLiteral("WRFAST"));
    m_token_name.insert(t_WRLONG,           QStringLiteral("WRLONG"));
    m_token_name.insert(t_WRLUT,            QStringLiteral("WRLUT"));
    m_token_name.insert(t_WRNC,             QStringLiteral("WRNC"));
    m_token_name.insert(t_WRNZ,             QStringLiteral("WRNZ"));
    m_token_name.insert(t_WRPIN,            QStringLiteral("WRPIN"));
    m_token_name.insert(t_WRWORD,           QStringLiteral("WRWORD"));
    m_token_name.insert(t_WRZ,              QStringLiteral("WRZ"));
    m_token_name.insert(t_WXPIN,            QStringLiteral("WXPIN"));
    m_token_name.insert(t_WYPIN,            QStringLiteral("WYPIN"));
    m_token_name.insert(t_XCONT,            QStringLiteral("XCONT"));
    m_token_name.insert(t_XINIT,            QStringLiteral("XINIT"));
    m_token_name.insert(t_XOR,              QStringLiteral("XOR"));
    m_token_name.insert(t_XORO32,           QStringLiteral("XORO32"));
    m_token_name.insert(t_XSTOP,            QStringLiteral("XSTOP"));
    m_token_name.insert(t_XZERO,            QStringLiteral("XZERO"));
    m_token_name.insert(t_ZEROX,            QStringLiteral("ZEROX"));

    // Set the type to tt_inst for whats in the hash now
    foreach(p2_token_e tok, m_token_name.keys())
            tt_set(tok, tt_inst);

    m_token_name.insert(t_invalid,          QStringLiteral("<invalid>"));
    m_token_name.insert(t_unknown,          QStringLiteral("«expr»"));
    m_token_name.insert(t_comma,            QStringLiteral(","));
    m_token_name.insert(t_string,           QStringLiteral("«string»"));
    m_token_name.insert(t_bin_const,        QStringLiteral("«bin»"));
    m_token_name.insert(t_oct_const,        QStringLiteral("«oct»"));
    m_token_name.insert(t_dec_const,        QStringLiteral("«dec»"));
    m_token_name.insert(t_hex_const,        QStringLiteral("«hex»"));
    m_token_name.insert(t_locsym,           QStringLiteral("«locsym»"));
    m_token_name.insert(t_symbol,           QStringLiteral("«symbol»"));
    m_token_name.insert(t_expression,       QStringLiteral("«expression»"));

    tt_set(t_invalid,       tt_lexer);
    tt_set(t_unknown,       tt_lexer);
    tt_set(t_comma,         tt_lexer);
    tt_set(t_string,        tt_lexer);
    tt_set(t_bin_const,     tt_lexer);
    tt_set(t_oct_const,     tt_lexer);
    tt_set(t_dec_const,     tt_lexer);
    tt_set(t_hex_const,     tt_lexer);
    tt_set(t_locsym,        tt_lexer);
    tt_set(t_symbol,        tt_lexer);
    tt_set(t_expression,    tt_lexer);

    m_token_name.insert(t_empty,            QStringLiteral("<empty>"));

    m_token_name.insert(t__RET_,            QStringLiteral("_RET_"));
    m_token_name.insert(t_IF_NZ_AND_NC,     QStringLiteral("IF_NZ_AND_NC"));
    m_token_name.insert(t_IF_NC_AND_NZ,     QStringLiteral("IF_NC_AND_NZ"));
    m_token_name.insert(t_IF_A,             QStringLiteral("IF_A"));
    m_token_name.insert(t_IF_GT,            QStringLiteral("IF_GT"));
    m_token_name.insert(t_IF_Z_AND_NC,      QStringLiteral("IF_Z_AND_NC"));
    m_token_name.insert(t_IF_NC_AND_Z,      QStringLiteral("IF_NC_AND_Z"));
    m_token_name.insert(t_IF_NC,            QStringLiteral("IF_NC"));
    m_token_name.insert(t_IF_AE,            QStringLiteral("IF_AE"));
    m_token_name.insert(t_IF_GE,            QStringLiteral("IF_GE"));
    m_token_name.insert(t_IF_NZ_AND_C,      QStringLiteral("IF_NZ_AND_C"));
    m_token_name.insert(t_IF_C_AND_NZ,      QStringLiteral("IF_C_AND_NZ"));
    m_token_name.insert(t_IF_NZ,            QStringLiteral("IF_NZ"));
    m_token_name.insert(t_IF_NE,            QStringLiteral("IF_NE"));
    m_token_name.insert(t_IF_Z_NE_C,        QStringLiteral("IF_Z_NE_C"));
    m_token_name.insert(t_IF_C_NE_Z,        QStringLiteral("IF_C_NE_Z"));
    m_token_name.insert(t_IF_NZ_OR_NC,      QStringLiteral("IF_NZ_OR_NC"));
    m_token_name.insert(t_IF_NC_OR_NZ,      QStringLiteral("IF_NC_OR_NZ"));
    m_token_name.insert(t_IF_Z_AND_C,       QStringLiteral("IF_Z_AND_C"));
    m_token_name.insert(t_IF_C_AND_Z,       QStringLiteral("IF_C_AND_Z"));
    m_token_name.insert(t_IF_Z_EQ_C,        QStringLiteral("IF_Z_EQ_C"));
    m_token_name.insert(t_IF_C_EQ_Z,        QStringLiteral("IF_C_EQ_Z"));
    m_token_name.insert(t_IF_Z,             QStringLiteral("IF_Z"));
    m_token_name.insert(t_IF_E,             QStringLiteral("IF_E"));
    m_token_name.insert(t_IF_Z_OR_NC,       QStringLiteral("IF_Z_OR_NC"));
    m_token_name.insert(t_IF_NC_OR_Z,       QStringLiteral("IF_NC_OR_Z"));
    m_token_name.insert(t_IF_C,             QStringLiteral("IF_C"));
    m_token_name.insert(t_IF_B,             QStringLiteral("IF_B"));
    m_token_name.insert(t_IF_LT,            QStringLiteral("IF_LT"));
    m_token_name.insert(t_IF_NZ_OR_C,       QStringLiteral("IF_NZ_OR_C"));
    m_token_name.insert(t_IF_C_OR_NZ,       QStringLiteral("IF_C_OR_NZ"));
    m_token_name.insert(t_IF_Z_OR_C,        QStringLiteral("IF_Z_OR_C"));
    m_token_name.insert(t_IF_C_OR_Z,        QStringLiteral("IF_C_OR_Z"));
    m_token_name.insert(t_IF_BE,            QStringLiteral("IF_BE"));
    m_token_name.insert(t_IF_LE,            QStringLiteral("IF_LE"));
    m_token_name.insert(t_IF_ALWAYS,        QStringLiteral("IF_ALWAYS"));

    m_token_name.insert(t_MODCZ__CLR,       QStringLiteral("_CLR"));
    m_token_name.insert(t_MODCZ__NC_AND_NZ, QStringLiteral("_NC_AND_NZ"));
    m_token_name.insert(t_MODCZ__NZ_AND_NC, QStringLiteral("_NZ_AND_NC"));
    m_token_name.insert(t_MODCZ__GT,        QStringLiteral("_GT"));
    m_token_name.insert(t_MODCZ__NC_AND_Z,  QStringLiteral("_NC_AND_Z"));
    m_token_name.insert(t_MODCZ__Z_AND_NC,  QStringLiteral("_Z_AND_NC"));
    m_token_name.insert(t_MODCZ__NC,        QStringLiteral("_NC"));
    m_token_name.insert(t_MODCZ__GE,        QStringLiteral("_GE"));
    m_token_name.insert(t_MODCZ__C_AND_NZ,  QStringLiteral("_C_AND_NZ"));
    m_token_name.insert(t_MODCZ__NZ_AND_C,  QStringLiteral("_NZ_AND_C"));
    m_token_name.insert(t_MODCZ__NZ,        QStringLiteral("_NZ"));
    m_token_name.insert(t_MODCZ__NE,        QStringLiteral("_NE"));
    m_token_name.insert(t_MODCZ__C_NE_Z,    QStringLiteral("_C_NE_Z"));
    m_token_name.insert(t_MODCZ__Z_NE_C,    QStringLiteral("_Z_NE_C"));
    m_token_name.insert(t_MODCZ__NC_OR_NZ,  QStringLiteral("_NC_OR_NZ"));
    m_token_name.insert(t_MODCZ__NZ_OR_NC,  QStringLiteral("_NZ_OR_NC"));
    m_token_name.insert(t_MODCZ__C_AND_Z,   QStringLiteral("_C_AND_Z"));
    m_token_name.insert(t_MODCZ__Z_AND_C,   QStringLiteral("_Z_AND_C"));
    m_token_name.insert(t_MODCZ__C_EQ_Z,    QStringLiteral("_C_EQ_Z"));
    m_token_name.insert(t_MODCZ__Z_EQ_C,    QStringLiteral("_Z_EQ_C"));
    m_token_name.insert(t_MODCZ__Z,         QStringLiteral("_Z"));
    m_token_name.insert(t_MODCZ__E,         QStringLiteral("_E"));
    m_token_name.insert(t_MODCZ__NC_OR_Z,   QStringLiteral("_NC_OR_Z"));
    m_token_name.insert(t_MODCZ__Z_OR_NC,   QStringLiteral("_Z_OR_NC"));
    m_token_name.insert(t_MODCZ__C,         QStringLiteral("_C"));
    m_token_name.insert(t_MODCZ__LT,        QStringLiteral("_LT"));
    m_token_name.insert(t_MODCZ__C_OR_NZ,   QStringLiteral("_C_OR_NZ"));
    m_token_name.insert(t_MODCZ__NZ_OR_C,   QStringLiteral("_NZ_OR_C"));
    m_token_name.insert(t_MODCZ__C_OR_Z,    QStringLiteral("_C_OR_Z"));
    m_token_name.insert(t_MODCZ__Z_OR_C,    QStringLiteral("_Z_OR_C"));
    m_token_name.insert(t_MODCZ__LE,        QStringLiteral("_LE"));
    m_token_name.insert(t_MODCZ__SET,       QStringLiteral("_SET"));

    m_token_name.insert(t_WC,               QStringLiteral("WC"));
    m_token_name.insert(t_WZ,               QStringLiteral("WZ"));
    m_token_name.insert(t_WCZ,              QStringLiteral("WCZ"));
    m_token_name.insert(t_ANDC,             QStringLiteral("ANDC"));
    m_token_name.insert(t_ANDZ,             QStringLiteral("ANDZ"));
    m_token_name.insert(t_ORC,              QStringLiteral("ORC"));
    m_token_name.insert(t_ORZ,              QStringLiteral("ORZ"));
    m_token_name.insert(t_XORC,             QStringLiteral("XORC"));
    m_token_name.insert(t_XORZ,             QStringLiteral("XORZ"));

    m_token_name.insert(t__BYTE,            QStringLiteral("BYTE"));
    m_token_name.insert(t__WORD,            QStringLiteral("WORD"));
    m_token_name.insert(t__LONG,            QStringLiteral("LONG"));
    m_token_name.insert(t__RES,             QStringLiteral("RES"));
    m_token_name.insert(t__FIT,             QStringLiteral("FIT"));

    m_token_name.insert(t__DAT,             QStringLiteral("DAT"));
    m_token_name.insert(t__CON,             QStringLiteral("CON"));
    m_token_name.insert(t__PUB,             QStringLiteral("PUB"));
    m_token_name.insert(t__PRI,             QStringLiteral("PRI"));
    m_token_name.insert(t__VAR,             QStringLiteral("VAR"));

    m_token_name.insert(t__ORG,             QStringLiteral("ORG"));
    m_token_name.insert(t__ORGH,            QStringLiteral("ORGH"));
    m_token_name.insert(t__ASSIGN,          QStringLiteral("="));
    m_token_name.insert(t__DOLLAR,          QStringLiteral("$"));

    m_token_name.insert(t__LPAREN,          QStringLiteral("("));
    m_token_name.insert(t__RPAREN,          QStringLiteral(")"));

    // Set the primary operators
    m_token_name.insert(t__INC,             QStringLiteral("++"));
    m_token_name.insert(t__DEC,             QStringLiteral("--"));
    tt_set(t__INC, tt_primary);
    tt_set(t__DEC, tt_primary);

    // Set the unary operators
    m_token_name.insert(t__NEG,             QStringLiteral("!"));
    m_token_name.insert(t__NOT,             QStringLiteral("~"));
    tt_set(t__NEG, tt_unary);
    tt_set(t__NOT, tt_unary);
    tt_set(t__ADD, tt_unary);
    tt_set(t__SUB, tt_unary);

    // Set the multiplication operators
    m_token_name.insert(t__MUL,             QStringLiteral("*"));
    m_token_name.insert(t__DIV,             QStringLiteral("/"));
    m_token_name.insert(t__MOD,             QStringLiteral("\\"));
    tt_set(t__MUL, tt_mulop);
    tt_set(t__DIV, tt_mulop);
    tt_set(t__MOD, tt_mulop);

    // Set the addition operators
    m_token_name.insert(t__ADD,             QStringLiteral("+"));
    m_token_name.insert(t__SUB,             QStringLiteral("-"));
    tt_set(t__ADD, tt_addop);
    tt_set(t__SUB, tt_addop);


    // Set the shift operators
    m_token_name.insert(t__SHL,             QStringLiteral("<<"));
    m_token_name.insert(t__SHR,             QStringLiteral(">>"));
    tt_set(t__SHL, tt_shiftop);
    tt_set(t__SHR, tt_shiftop);

    // Set the less/greater comparison operators
    m_token_name.insert(t__GE,              QStringLiteral(">="));
    m_token_name.insert(t__GT,              QStringLiteral(">"));
    m_token_name.insert(t__LE,              QStringLiteral("<="));
    m_token_name.insert(t__LT,              QStringLiteral("<"));
    tt_set(t__GE, tt_relation);
    tt_set(t__GT, tt_relation);
    tt_set(t__LE, tt_relation);
    tt_set(t__LT, tt_relation);

    // Set the equal/unequal comparison operators
    m_token_name.insert(t__EQ,              QStringLiteral("=="));
    m_token_name.insert(t__NE,              QStringLiteral("!="));
    tt_set(t__EQ, tt_equality);
    tt_set(t__NE, tt_equality);

    // Set the binary and operators
    m_token_name.insert(t__AND,             QStringLiteral("&"));
    tt_set(t__AND, tt_binop_and);

    // Set the binary xor operators
    m_token_name.insert(t__XOR,             QStringLiteral("^"));
    tt_set(t__XOR, tt_binop_xor);

    // Set the binary xor operators
    m_token_name.insert(t__OR,              QStringLiteral("|"));
    tt_set(t__OR, tt_binop_or);

    m_token_name.insert(t__REV,             QStringLiteral("><"));
    tt_set(t__REV, tt_binop_rev);

    // Set the conditionals
    tt_set(t__RET_,               tt_conditional);
    tt_set(t_IF_NZ_AND_NC,        tt_conditional);
    tt_set(t_IF_NC_AND_NZ,        tt_conditional);
    tt_set(t_IF_A,                tt_conditional);
    tt_set(t_IF_GT,               tt_conditional);
    tt_set(t_IF_Z_AND_NC,         tt_conditional);
    tt_set(t_IF_NC_AND_Z,         tt_conditional);
    tt_set(t_IF_NC,               tt_conditional);
    tt_set(t_IF_AE,               tt_conditional);
    tt_set(t_IF_GE,               tt_conditional);
    tt_set(t_IF_NZ_AND_C,         tt_conditional);
    tt_set(t_IF_C_AND_NZ,         tt_conditional);
    tt_set(t_IF_NZ,               tt_conditional);
    tt_set(t_IF_NE,               tt_conditional);
    tt_set(t_IF_Z_NE_C,           tt_conditional);
    tt_set(t_IF_C_NE_Z,           tt_conditional);
    tt_set(t_IF_NZ_OR_NC,         tt_conditional);
    tt_set(t_IF_NC_OR_NZ,         tt_conditional);
    tt_set(t_IF_Z_AND_C,          tt_conditional);
    tt_set(t_IF_C_AND_Z,          tt_conditional);
    tt_set(t_IF_Z_EQ_C,           tt_conditional);
    tt_set(t_IF_C_EQ_Z,           tt_conditional);
    tt_set(t_IF_Z,                tt_conditional);
    tt_set(t_IF_E,                tt_conditional);
    tt_set(t_IF_Z_OR_NC,          tt_conditional);
    tt_set(t_IF_NC_OR_Z,          tt_conditional);
    tt_set(t_IF_C,                tt_conditional);
    tt_set(t_IF_B,                tt_conditional);
    tt_set(t_IF_LT,               tt_conditional);
    tt_set(t_IF_NZ_OR_C,          tt_conditional);
    tt_set(t_IF_C_OR_NZ,          tt_conditional);
    tt_set(t_IF_Z_OR_C,           tt_conditional);
    tt_set(t_IF_C_OR_Z,           tt_conditional);
    tt_set(t_IF_BE,               tt_conditional);
    tt_set(t_IF_LE,               tt_conditional);
    tt_set(t_IF_ALWAYS,           tt_conditional);

    // Set the MODCZ tokens
    tt_set(t_MODCZ__CLR,          tt_modcz_param);
    tt_set(t_MODCZ__NC_AND_NZ,    tt_modcz_param);
    tt_set(t_MODCZ__NZ_AND_NC,    tt_modcz_param);
    tt_set(t_MODCZ__GT,           tt_modcz_param);
    tt_set(t_MODCZ__NC_AND_Z,     tt_modcz_param);
    tt_set(t_MODCZ__Z_AND_NC,     tt_modcz_param);
    tt_set(t_MODCZ__NC,           tt_modcz_param);
    tt_set(t_MODCZ__GE,           tt_modcz_param);
    tt_set(t_MODCZ__C_AND_NZ,     tt_modcz_param);
    tt_set(t_MODCZ__NZ_AND_C,     tt_modcz_param);
    tt_set(t_MODCZ__NZ,           tt_modcz_param);
    tt_set(t_MODCZ__NE,           tt_modcz_param);
    tt_set(t_MODCZ__C_NE_Z,       tt_modcz_param);
    tt_set(t_MODCZ__Z_NE_C,       tt_modcz_param);
    tt_set(t_MODCZ__NC_OR_NZ,     tt_modcz_param);
    tt_set(t_MODCZ__NZ_OR_NC,     tt_modcz_param);
    tt_set(t_MODCZ__C_AND_Z,      tt_modcz_param);
    tt_set(t_MODCZ__Z_AND_C,      tt_modcz_param);
    tt_set(t_MODCZ__C_EQ_Z,       tt_modcz_param);
    tt_set(t_MODCZ__Z_EQ_C,       tt_modcz_param);
    tt_set(t_MODCZ__Z,            tt_modcz_param);
    tt_set(t_MODCZ__E,            tt_modcz_param);
    tt_set(t_MODCZ__NC_OR_Z,      tt_modcz_param);
    tt_set(t_MODCZ__Z_OR_NC,      tt_modcz_param);
    tt_set(t_MODCZ__C,            tt_modcz_param);
    tt_set(t_MODCZ__LT,           tt_modcz_param);
    tt_set(t_MODCZ__C_OR_NZ,      tt_modcz_param);
    tt_set(t_MODCZ__NZ_OR_C,      tt_modcz_param);
    tt_set(t_MODCZ__C_OR_Z,       tt_modcz_param);
    tt_set(t_MODCZ__Z_OR_C,       tt_modcz_param);
    tt_set(t_MODCZ__LE,           tt_modcz_param);
    tt_set(t_MODCZ__SET,          tt_modcz_param);

    // Section control
    tt_set(t__DAT, tt_section);
    tt_set(t__CON, tt_section);
    tt_set(t__PUB, tt_section);
    tt_set(t__PRI, tt_section);
    tt_set(t__VAR, tt_section);

    // Origin control
    tt_set(t__ORG,  tt_origin);
    tt_set(t__ORGH, tt_origin);
    tt_set(t__FIT,  tt_origin);

    // Data types and space reserving
    tt_set(t__BYTE, tt_data);
    tt_set(t__WORD, tt_data);
    tt_set(t__LONG, tt_data);
    tt_set(t__RES,  tt_data);

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
            tok = t_locsym;
            break;
        }

        if (0 == ustr.indexOf(rx_symbol)) {
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
    const quint64 ops =
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
            tm_logop_and |
            tm_logop_or;
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
p2_token_e P2Token::at_types(int& pos, const QString& str, const QList<p2_tokentype_e>& types, p2_token_e dflt) const
{
    int len = 0;
    p2_token_e tok = token(str.mid(pos), true, &len);

    if (t_unknown == tok)
        return dflt;

    // Build bit mask for types
    quint64 mask = tm_none;
    foreach(const p2_tokentype_e type, types)
        mask |= TTMASK(type);

    if (m_token_type.value(tok, tm_none) & mask) {
        pos += len;
    } else {
        tok = dflt;
    }

    return tok;
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

void P2Token::tt_set(p2_token_e tok, p2_tokentype_e type)
{
    quint64 mask = m_token_type.value(tok, 0);
    mask |= TTMASK(type);
    m_token_type.insert(tok, mask);
}

void P2Token::tt_clr(p2_token_e tok, p2_tokentype_e type)
{
    quint64 mask = m_token_type.value(tok, 0);
    mask &= ~TTMASK(type);
    m_token_type.insert(tok, mask);
}

bool P2Token::tt_chk(p2_token_e tok, p2_tokentype_e type) const
{
    quint64 mask = m_token_type.value(tok, 0);
    return mask & TTMASK(type) ? true : false;
}
