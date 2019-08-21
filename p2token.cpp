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
    : m_token_string()
    , m_string_token()
    , m_token_types()
    , m_type_tokens()
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

    m_token_string.insert(t_invalid,            QStringLiteral("<invalid>"));
    m_token_string.insert(t_unknown,            QStringLiteral("«expr»"));
    m_token_string.insert(t_comma,              QStringLiteral(","));
    m_token_string.insert(t_string,             QStringLiteral("«string»"));
    m_token_string.insert(t_value_bin,          QStringLiteral("«bin»"));
    m_token_string.insert(t_value_oct,          QStringLiteral("«oct»"));
    m_token_string.insert(t_value_dec,          QStringLiteral("«dec»"));
    m_token_string.insert(t_value_hex,          QStringLiteral("«hex»"));
    m_token_string.insert(t_locsym,             QStringLiteral("«locsym»"));
    m_token_string.insert(t_symbol,             QStringLiteral("«symbol»"));
    m_token_string.insert(t_expression,         QStringLiteral("«expression»"));

    m_token_string.insert(t_empty,              QStringLiteral("<empty>"));
    m_token_string.insert(t__RET_,              QStringLiteral("_RET_"));

    m_token_string.insert(t_IF_NZ_AND_NC,       QStringLiteral("IF_NZ_AND_NC"));
    m_token_string.insert(t_IF_NC_AND_NZ,       QStringLiteral("IF_NC_AND_NZ"));
    m_token_string.insert(t_IF_A,               QStringLiteral("IF_A"));
    m_token_string.insert(t_IF_GT,              QStringLiteral("IF_GT"));
    m_token_string.insert(t_IF_Z_AND_NC,        QStringLiteral("IF_Z_AND_NC"));
    m_token_string.insert(t_IF_NC_AND_Z,        QStringLiteral("IF_NC_AND_Z"));
    m_token_string.insert(t_IF_NC,              QStringLiteral("IF_NC"));
    m_token_string.insert(t_IF_AE,              QStringLiteral("IF_AE"));
    m_token_string.insert(t_IF_GE,              QStringLiteral("IF_GE"));
    m_token_string.insert(t_IF_NZ_AND_C,        QStringLiteral("IF_NZ_AND_C"));
    m_token_string.insert(t_IF_C_AND_NZ,        QStringLiteral("IF_C_AND_NZ"));
    m_token_string.insert(t_IF_NZ,              QStringLiteral("IF_NZ"));
    m_token_string.insert(t_IF_NE,              QStringLiteral("IF_NE"));
    m_token_string.insert(t_IF_Z_NE_C,          QStringLiteral("IF_Z_NE_C"));
    m_token_string.insert(t_IF_C_NE_Z,          QStringLiteral("IF_C_NE_Z"));
    m_token_string.insert(t_IF_NZ_OR_NC,        QStringLiteral("IF_NZ_OR_NC"));
    m_token_string.insert(t_IF_NC_OR_NZ,        QStringLiteral("IF_NC_OR_NZ"));
    m_token_string.insert(t_IF_Z_AND_C,         QStringLiteral("IF_Z_AND_C"));
    m_token_string.insert(t_IF_C_AND_Z,         QStringLiteral("IF_C_AND_Z"));
    m_token_string.insert(t_IF_Z_EQ_C,          QStringLiteral("IF_Z_EQ_C"));
    m_token_string.insert(t_IF_C_EQ_Z,          QStringLiteral("IF_C_EQ_Z"));
    m_token_string.insert(t_IF_Z,               QStringLiteral("IF_Z"));
    m_token_string.insert(t_IF_E,               QStringLiteral("IF_E"));
    m_token_string.insert(t_IF_Z_OR_NC,         QStringLiteral("IF_Z_OR_NC"));
    m_token_string.insert(t_IF_NC_OR_Z,         QStringLiteral("IF_NC_OR_Z"));
    m_token_string.insert(t_IF_C,               QStringLiteral("IF_C"));
    m_token_string.insert(t_IF_B,               QStringLiteral("IF_B"));
    m_token_string.insert(t_IF_LT,              QStringLiteral("IF_LT"));
    m_token_string.insert(t_IF_NZ_OR_C,         QStringLiteral("IF_NZ_OR_C"));
    m_token_string.insert(t_IF_C_OR_NZ,         QStringLiteral("IF_C_OR_NZ"));
    m_token_string.insert(t_IF_Z_OR_C,          QStringLiteral("IF_Z_OR_C"));
    m_token_string.insert(t_IF_C_OR_Z,          QStringLiteral("IF_C_OR_Z"));
    m_token_string.insert(t_IF_BE,              QStringLiteral("IF_BE"));
    m_token_string.insert(t_IF_LE,              QStringLiteral("IF_LE"));
    m_token_string.insert(t_IF_ALWAYS,          QStringLiteral("IF_ALWAYS"));

    m_token_string.insert(t_MODCZ__CLR,         QStringLiteral("_CLR"));
    m_token_string.insert(t_MODCZ__NC_AND_NZ,   QStringLiteral("_NC_AND_NZ"));
    m_token_string.insert(t_MODCZ__NZ_AND_NC,   QStringLiteral("_NZ_AND_NC"));
    m_token_string.insert(t_MODCZ__GT,          QStringLiteral("_GT"));
    m_token_string.insert(t_MODCZ__NC_AND_Z,    QStringLiteral("_NC_AND_Z"));
    m_token_string.insert(t_MODCZ__Z_AND_NC,    QStringLiteral("_Z_AND_NC"));
    m_token_string.insert(t_MODCZ__NC,          QStringLiteral("_NC"));
    m_token_string.insert(t_MODCZ__GE,          QStringLiteral("_GE"));
    m_token_string.insert(t_MODCZ__C_AND_NZ,    QStringLiteral("_C_AND_NZ"));
    m_token_string.insert(t_MODCZ__NZ_AND_C,    QStringLiteral("_NZ_AND_C"));
    m_token_string.insert(t_MODCZ__NZ,          QStringLiteral("_NZ"));
    m_token_string.insert(t_MODCZ__NE,          QStringLiteral("_NE"));
    m_token_string.insert(t_MODCZ__C_NE_Z,      QStringLiteral("_C_NE_Z"));
    m_token_string.insert(t_MODCZ__Z_NE_C,      QStringLiteral("_Z_NE_C"));
    m_token_string.insert(t_MODCZ__NC_OR_NZ,    QStringLiteral("_NC_OR_NZ"));
    m_token_string.insert(t_MODCZ__NZ_OR_NC,    QStringLiteral("_NZ_OR_NC"));
    m_token_string.insert(t_MODCZ__C_AND_Z,     QStringLiteral("_C_AND_Z"));
    m_token_string.insert(t_MODCZ__Z_AND_C,     QStringLiteral("_Z_AND_C"));
    m_token_string.insert(t_MODCZ__C_EQ_Z,      QStringLiteral("_C_EQ_Z"));
    m_token_string.insert(t_MODCZ__Z_EQ_C,      QStringLiteral("_Z_EQ_C"));
    m_token_string.insert(t_MODCZ__Z,           QStringLiteral("_Z"));
    m_token_string.insert(t_MODCZ__E,           QStringLiteral("_E"));
    m_token_string.insert(t_MODCZ__NC_OR_Z,     QStringLiteral("_NC_OR_Z"));
    m_token_string.insert(t_MODCZ__Z_OR_NC,     QStringLiteral("_Z_OR_NC"));
    m_token_string.insert(t_MODCZ__C,           QStringLiteral("_C"));
    m_token_string.insert(t_MODCZ__LT,          QStringLiteral("_LT"));
    m_token_string.insert(t_MODCZ__C_OR_NZ,     QStringLiteral("_C_OR_NZ"));
    m_token_string.insert(t_MODCZ__NZ_OR_C,     QStringLiteral("_NZ_OR_C"));
    m_token_string.insert(t_MODCZ__C_OR_Z,      QStringLiteral("_C_OR_Z"));
    m_token_string.insert(t_MODCZ__Z_OR_C,      QStringLiteral("_Z_OR_C"));
    m_token_string.insert(t_MODCZ__LE,          QStringLiteral("_LE"));
    m_token_string.insert(t_MODCZ__SET,         QStringLiteral("_SET"));

    m_token_string.insert(t_ABS,                QStringLiteral("ABS"));
    m_token_string.insert(t_ADD,                QStringLiteral("ADD"));
    m_token_string.insert(t_ADDCT1,             QStringLiteral("ADDCT1"));
    m_token_string.insert(t_ADDCT2,             QStringLiteral("ADDCT2"));
    m_token_string.insert(t_ADDCT3,             QStringLiteral("ADDCT3"));
    m_token_string.insert(t_ADDPIX,             QStringLiteral("ADDPIX"));
    m_token_string.insert(t_ADDS,               QStringLiteral("ADDS"));
    m_token_string.insert(t_ADDSX,              QStringLiteral("ADDSX"));
    m_token_string.insert(t_ADDX,               QStringLiteral("ADDX"));
    m_token_string.insert(t_AKPIN,              QStringLiteral("AKPIN"));
    m_token_string.insert(t_ALLOWI,             QStringLiteral("ALLOWI"));
    m_token_string.insert(t_ALTB,               QStringLiteral("ALTB"));
    m_token_string.insert(t_ALTD,               QStringLiteral("ALTD"));
    m_token_string.insert(t_ALTGB,              QStringLiteral("ALTGB"));
    m_token_string.insert(t_ALTGN,              QStringLiteral("ALTGN"));
    m_token_string.insert(t_ALTGW,              QStringLiteral("ALTGW"));
    m_token_string.insert(t_ALTI,               QStringLiteral("ALTI"));
    m_token_string.insert(t_ALTR,               QStringLiteral("ALTR"));
    m_token_string.insert(t_ALTS,               QStringLiteral("ALTS"));
    m_token_string.insert(t_ALTSB,              QStringLiteral("ALTSB"));
    m_token_string.insert(t_ALTSN,              QStringLiteral("ALTSN"));
    m_token_string.insert(t_ALTSW,              QStringLiteral("ALTSW"));
    m_token_string.insert(t_AND,                QStringLiteral("AND"));
    m_token_string.insert(t_ANDN,               QStringLiteral("ANDN"));
    m_token_string.insert(t_AUGD,               QStringLiteral("AUGD"));
    m_token_string.insert(t_AUGS,               QStringLiteral("AUGS"));
    m_token_string.insert(t_BITC,               QStringLiteral("BITC"));
    m_token_string.insert(t_BITH,               QStringLiteral("BITH"));
    m_token_string.insert(t_BITL,               QStringLiteral("BITL"));
    m_token_string.insert(t_BITNC,              QStringLiteral("BITNC"));
    m_token_string.insert(t_BITNOT,             QStringLiteral("BITNOT"));
    m_token_string.insert(t_BITNZ,              QStringLiteral("BITNZ"));
    m_token_string.insert(t_BITRND,             QStringLiteral("BITRND"));
    m_token_string.insert(t_BITZ,               QStringLiteral("BITZ"));
    m_token_string.insert(t_BLNPIX,             QStringLiteral("BLNPIX"));
    m_token_string.insert(t_BMASK,              QStringLiteral("BMASK"));
    m_token_string.insert(t_BRK,                QStringLiteral("BRK"));
    m_token_string.insert(t_CALL,               QStringLiteral("CALL"));
    m_token_string.insert(t_CALLA,              QStringLiteral("CALLA"));
    m_token_string.insert(t_CALLB,              QStringLiteral("CALLB"));
    m_token_string.insert(t_CALLD,              QStringLiteral("CALLD"));
    m_token_string.insert(t_CALLPA,             QStringLiteral("CALLPA"));
    m_token_string.insert(t_CALLPB,             QStringLiteral("CALLPB"));
    m_token_string.insert(t_CMP,                QStringLiteral("CMP"));
    m_token_string.insert(t_CMPM,               QStringLiteral("CMPM"));
    m_token_string.insert(t_CMPR,               QStringLiteral("CMPR"));
    m_token_string.insert(t_CMPS,               QStringLiteral("CMPS"));
    m_token_string.insert(t_CMPSUB,             QStringLiteral("CMPSUB"));
    m_token_string.insert(t_CMPSX,              QStringLiteral("CMPSX"));
    m_token_string.insert(t_CMPX,               QStringLiteral("CMPX"));
    m_token_string.insert(t_COGATN,             QStringLiteral("COGATN"));
    m_token_string.insert(t_COGBRK,             QStringLiteral("COGBRK"));
    m_token_string.insert(t_COGID,              QStringLiteral("COGID"));
    m_token_string.insert(t_COGINIT,            QStringLiteral("COGINIT"));
    m_token_string.insert(t_COGSTOP,            QStringLiteral("COGSTOP"));
    m_token_string.insert(t_CRCBIT,             QStringLiteral("CRCBIT"));
    m_token_string.insert(t_CRCNIB,             QStringLiteral("CRCNIB"));
    m_token_string.insert(t_DECMOD,             QStringLiteral("DECMOD"));
    m_token_string.insert(t_DECOD,              QStringLiteral("DECOD"));
    m_token_string.insert(t_DIRC,               QStringLiteral("DIRC"));
    m_token_string.insert(t_DIRH,               QStringLiteral("DIRH"));
    m_token_string.insert(t_DIRL,               QStringLiteral("DIRL"));
    m_token_string.insert(t_DIRNC,              QStringLiteral("DIRNC"));
    m_token_string.insert(t_DIRNOT,             QStringLiteral("DIRNOT"));
    m_token_string.insert(t_DIRNZ,              QStringLiteral("DIRNZ"));
    m_token_string.insert(t_DIRRND,             QStringLiteral("DIRRND"));
    m_token_string.insert(t_DIRZ,               QStringLiteral("DIRZ"));
    m_token_string.insert(t_DJF,                QStringLiteral("DJF"));
    m_token_string.insert(t_DJNF,               QStringLiteral("DJNF"));
    m_token_string.insert(t_DJNZ,               QStringLiteral("DJNZ"));
    m_token_string.insert(t_DJZ,                QStringLiteral("DJZ"));
    m_token_string.insert(t_DRVC,               QStringLiteral("DRVC"));
    m_token_string.insert(t_DRVH,               QStringLiteral("DRVH"));
    m_token_string.insert(t_DRVL,               QStringLiteral("DRVL"));
    m_token_string.insert(t_DRVNC,              QStringLiteral("DRVNC"));
    m_token_string.insert(t_DRVNOT,             QStringLiteral("DRVNOT"));
    m_token_string.insert(t_DRVNZ,              QStringLiteral("DRVNZ"));
    m_token_string.insert(t_DRVRND,             QStringLiteral("DRVRND"));
    m_token_string.insert(t_DRVZ,               QStringLiteral("DRVZ"));
    m_token_string.insert(t_ENCOD,              QStringLiteral("ENCOD"));
    m_token_string.insert(t_EXECF,              QStringLiteral("EXECF"));
    m_token_string.insert(t_FBLOCK,             QStringLiteral("FBLOCK"));
    m_token_string.insert(t_FGE,                QStringLiteral("FGE"));
    m_token_string.insert(t_FGES,               QStringLiteral("FGES"));
    m_token_string.insert(t_FLE,                QStringLiteral("FLE"));
    m_token_string.insert(t_FLES,               QStringLiteral("FLES"));
    m_token_string.insert(t_FLTC,               QStringLiteral("FLTC"));
    m_token_string.insert(t_FLTH,               QStringLiteral("FLTH"));
    m_token_string.insert(t_FLTL,               QStringLiteral("FLTL"));
    m_token_string.insert(t_FLTNC,              QStringLiteral("FLTNC"));
    m_token_string.insert(t_FLTNOT,             QStringLiteral("FLTNOT"));
    m_token_string.insert(t_FLTNZ,              QStringLiteral("FLTNZ"));
    m_token_string.insert(t_FLTRND,             QStringLiteral("FLTRND"));
    m_token_string.insert(t_FLTZ,               QStringLiteral("FLTZ"));
    m_token_string.insert(t_GETBRK,             QStringLiteral("GETBRK"));
    m_token_string.insert(t_GETBYTE,            QStringLiteral("GETBYTE"));
    m_token_string.insert(t_GETCT,              QStringLiteral("GETCT"));
    m_token_string.insert(t_GETNIB,             QStringLiteral("GETNIB"));
    m_token_string.insert(t_GETPTR,             QStringLiteral("GETPTR"));
    m_token_string.insert(t_GETQX,              QStringLiteral("GETQX"));
    m_token_string.insert(t_GETQY,              QStringLiteral("GETQY"));
    m_token_string.insert(t_GETRND,             QStringLiteral("GETRND"));
    m_token_string.insert(t_GETSCP,             QStringLiteral("GETSCP"));
    m_token_string.insert(t_GETWORD,            QStringLiteral("GETWORD"));
    m_token_string.insert(t_GETXACC,            QStringLiteral("GETXACC"));
    m_token_string.insert(t_HUBSET,             QStringLiteral("HUBSET"));
    m_token_string.insert(t_IJNZ,               QStringLiteral("IJNZ"));
    m_token_string.insert(t_IJZ,                QStringLiteral("IJZ"));
    m_token_string.insert(t_INCMOD,             QStringLiteral("INCMOD"));
    m_token_string.insert(t_JATN,               QStringLiteral("JATN"));
    m_token_string.insert(t_JCT1,               QStringLiteral("JCT1"));
    m_token_string.insert(t_JCT2,               QStringLiteral("JCT2"));
    m_token_string.insert(t_JCT3,               QStringLiteral("JCT3"));
    m_token_string.insert(t_JFBW,               QStringLiteral("JFBW"));
    m_token_string.insert(t_JINT,               QStringLiteral("JINT"));
    m_token_string.insert(t_JMP,                QStringLiteral("JMP"));
    m_token_string.insert(t_JMPREL,             QStringLiteral("JMPREL"));
    m_token_string.insert(t_JNATN,              QStringLiteral("JNATN"));
    m_token_string.insert(t_JNCT1,              QStringLiteral("JNCT1"));
    m_token_string.insert(t_JNCT2,              QStringLiteral("JNCT2"));
    m_token_string.insert(t_JNCT3,              QStringLiteral("JNCT3"));
    m_token_string.insert(t_JNFBW,              QStringLiteral("JNFBW"));
    m_token_string.insert(t_JNINT,              QStringLiteral("JNINT"));
    m_token_string.insert(t_JNPAT,              QStringLiteral("JNPAT"));
    m_token_string.insert(t_JNQMT,              QStringLiteral("JNQMT"));
    m_token_string.insert(t_JNSE1,              QStringLiteral("JNSE1"));
    m_token_string.insert(t_JNSE2,              QStringLiteral("JNSE2"));
    m_token_string.insert(t_JNSE3,              QStringLiteral("JNSE3"));
    m_token_string.insert(t_JNSE4,              QStringLiteral("JNSE4"));
    m_token_string.insert(t_JNXFI,              QStringLiteral("JNXFI"));
    m_token_string.insert(t_JNXMT,              QStringLiteral("JNXMT"));
    m_token_string.insert(t_JNXRL,              QStringLiteral("JNXRL"));
    m_token_string.insert(t_JNXRO,              QStringLiteral("JNXRO"));
    m_token_string.insert(t_JPAT,               QStringLiteral("JPAT"));
    m_token_string.insert(t_JQMT,               QStringLiteral("JQMT"));
    m_token_string.insert(t_JSE1,               QStringLiteral("JSE1"));
    m_token_string.insert(t_JSE2,               QStringLiteral("JSE2"));
    m_token_string.insert(t_JSE3,               QStringLiteral("JSE3"));
    m_token_string.insert(t_JSE4,               QStringLiteral("JSE4"));
    m_token_string.insert(t_JXFI,               QStringLiteral("JXFI"));
    m_token_string.insert(t_JXMT,               QStringLiteral("JXMT"));
    m_token_string.insert(t_JXRL,               QStringLiteral("JXRL"));
    m_token_string.insert(t_JXRO,               QStringLiteral("JXRO"));
    m_token_string.insert(t_LOC,                QStringLiteral("LOC"));
    m_token_string.insert(t_LOCKNEW,            QStringLiteral("LOCKNEW"));
    m_token_string.insert(t_LOCKREL,            QStringLiteral("LOCKREL"));
    m_token_string.insert(t_LOCKRET,            QStringLiteral("LOCKRET"));
    m_token_string.insert(t_LOCKTRY,            QStringLiteral("LOCKTRY"));
    m_token_string.insert(t_MERGEB,             QStringLiteral("MERGEB"));
    m_token_string.insert(t_MERGEW,             QStringLiteral("MERGEW"));
    m_token_string.insert(t_MIXPIX,             QStringLiteral("MIXPIX"));
    m_token_string.insert(t_MODCZ,              QStringLiteral("MODCZ"));
    m_token_string.insert(t_MOV,                QStringLiteral("MOV"));
    m_token_string.insert(t_MOVBYTS,            QStringLiteral("MOVBYTS"));
    m_token_string.insert(t_MUL,                QStringLiteral("MUL"));
    m_token_string.insert(t_MULPIX,             QStringLiteral("MULPIX"));
    m_token_string.insert(t_MULS,               QStringLiteral("MULS"));
    m_token_string.insert(t_MUXC,               QStringLiteral("MUXC"));
    m_token_string.insert(t_MUXNC,              QStringLiteral("MUXNC"));
    m_token_string.insert(t_MUXNIBS,            QStringLiteral("MUXNIBS"));
    m_token_string.insert(t_MUXNITS,            QStringLiteral("MUXNITS"));
    m_token_string.insert(t_MUXNZ,              QStringLiteral("MUXNZ"));
    m_token_string.insert(t_MUXQ,               QStringLiteral("MUXQ"));
    m_token_string.insert(t_MUXZ,               QStringLiteral("MUXZ"));
    m_token_string.insert(t_NEG,                QStringLiteral("NEG"));
    m_token_string.insert(t_NEGC,               QStringLiteral("NEGC"));
    m_token_string.insert(t_NEGNC,              QStringLiteral("NEGNC"));
    m_token_string.insert(t_NEGNZ,              QStringLiteral("NEGNZ"));
    m_token_string.insert(t_NEGZ,               QStringLiteral("NEGZ"));
    m_token_string.insert(t_NIXINT1,            QStringLiteral("NIXINT1"));
    m_token_string.insert(t_NIXINT2,            QStringLiteral("NIXINT2"));
    m_token_string.insert(t_NIXINT3,            QStringLiteral("NIXINT3"));
    m_token_string.insert(t_NOP,                QStringLiteral("NOP"));
    m_token_string.insert(t_NOT,                QStringLiteral("NOT"));
    m_token_string.insert(t_ONES,               QStringLiteral("ONES"));
    m_token_string.insert(t_OR,                 QStringLiteral("OR"));
    m_token_string.insert(t_OUTC,               QStringLiteral("OUTC"));
    m_token_string.insert(t_OUTH,               QStringLiteral("OUTH"));
    m_token_string.insert(t_OUTL,               QStringLiteral("OUTL"));
    m_token_string.insert(t_OUTNC,              QStringLiteral("OUTNC"));
    m_token_string.insert(t_OUTNOT,             QStringLiteral("OUTNOT"));
    m_token_string.insert(t_OUTNZ,              QStringLiteral("OUTNZ"));
    m_token_string.insert(t_OUTRND,             QStringLiteral("OUTRND"));
    m_token_string.insert(t_OUTZ,               QStringLiteral("OUTZ"));
    m_token_string.insert(t_PA,                 QStringLiteral("PA"));
    m_token_string.insert(t_PB,                 QStringLiteral("PB"));
    m_token_string.insert(t_POLLATN,            QStringLiteral("POLLATN"));
    m_token_string.insert(t_POLLCT1,            QStringLiteral("POLLCT1"));
    m_token_string.insert(t_POLLCT2,            QStringLiteral("POLLCT2"));
    m_token_string.insert(t_POLLCT3,            QStringLiteral("POLLCT3"));
    m_token_string.insert(t_POLLFBW,            QStringLiteral("POLLFBW"));
    m_token_string.insert(t_POLLINT,            QStringLiteral("POLLINT"));
    m_token_string.insert(t_POLLPAT,            QStringLiteral("POLLPAT"));
    m_token_string.insert(t_POLLQMT,            QStringLiteral("POLLQMT"));
    m_token_string.insert(t_POLLSE1,            QStringLiteral("POLLSE1"));
    m_token_string.insert(t_POLLSE2,            QStringLiteral("POLLSE2"));
    m_token_string.insert(t_POLLSE3,            QStringLiteral("POLLSE3"));
    m_token_string.insert(t_POLLSE4,            QStringLiteral("POLLSE4"));
    m_token_string.insert(t_POLLXFI,            QStringLiteral("POLLXFI"));
    m_token_string.insert(t_POLLXMT,            QStringLiteral("POLLXMT"));
    m_token_string.insert(t_POLLXRL,            QStringLiteral("POLLXRL"));
    m_token_string.insert(t_POLLXRO,            QStringLiteral("POLLXRO"));
    m_token_string.insert(t_POP,                QStringLiteral("POP"));
    m_token_string.insert(t_POPA,               QStringLiteral("POPA"));
    m_token_string.insert(t_POPB,               QStringLiteral("POPB"));
    m_token_string.insert(t_PTRA,               QStringLiteral("PTRA"));
    m_token_string.insert(t_PTRB,               QStringLiteral("PTRB"));
    m_token_string.insert(t_PUSH,               QStringLiteral("PUSH"));
    m_token_string.insert(t_PUSHA,              QStringLiteral("PUSHA"));
    m_token_string.insert(t_PUSHB,              QStringLiteral("PUSHB"));
    m_token_string.insert(t_QDIV,               QStringLiteral("QDIV"));
    m_token_string.insert(t_QEXP,               QStringLiteral("QEXP"));
    m_token_string.insert(t_QFRAC,              QStringLiteral("QFRAC"));
    m_token_string.insert(t_QLOG,               QStringLiteral("QLOG"));
    m_token_string.insert(t_QMUL,               QStringLiteral("QMUL"));
    m_token_string.insert(t_QROTATE,            QStringLiteral("QROTATE"));
    m_token_string.insert(t_QSQRT,              QStringLiteral("QSQRT"));
    m_token_string.insert(t_QVECTOR,            QStringLiteral("QVECTOR"));
    m_token_string.insert(t_RCL,                QStringLiteral("RCL"));
    m_token_string.insert(t_RCR,                QStringLiteral("RCR"));
    m_token_string.insert(t_RCZL,               QStringLiteral("RCZL"));
    m_token_string.insert(t_RCZR,               QStringLiteral("RCZR"));
    m_token_string.insert(t_RDBYTE,             QStringLiteral("RDBYTE"));
    m_token_string.insert(t_RDFAST,             QStringLiteral("RDFAST"));
    m_token_string.insert(t_RDLONG,             QStringLiteral("RDLONG"));
    m_token_string.insert(t_RDLUT,              QStringLiteral("RDLUT"));
    m_token_string.insert(t_RDPIN,              QStringLiteral("RDPIN"));
    m_token_string.insert(t_RDWORD,             QStringLiteral("RDWORD"));
    m_token_string.insert(t_REP,                QStringLiteral("REP"));
    m_token_string.insert(t_RESI0,              QStringLiteral("RESI0"));
    m_token_string.insert(t_RESI1,              QStringLiteral("RESI1"));
    m_token_string.insert(t_RESI2,              QStringLiteral("RESI2"));
    m_token_string.insert(t_RESI3,              QStringLiteral("RESI3"));
    m_token_string.insert(t_RET,                QStringLiteral("RET"));
    m_token_string.insert(t_RETA,               QStringLiteral("RETA"));
    m_token_string.insert(t_RETB,               QStringLiteral("RETB"));
    m_token_string.insert(t_RETI0,              QStringLiteral("RETI0"));
    m_token_string.insert(t_RETI1,              QStringLiteral("RETI1"));
    m_token_string.insert(t_RETI2,              QStringLiteral("RETI2"));
    m_token_string.insert(t_RETI3,              QStringLiteral("RETI3"));
    m_token_string.insert(t_REV,                QStringLiteral("REV"));
    m_token_string.insert(t_RFBYTE,             QStringLiteral("RFBYTE"));
    m_token_string.insert(t_RFLONG,             QStringLiteral("RFLONG"));
    m_token_string.insert(t_RFVAR,              QStringLiteral("RFVAR"));
    m_token_string.insert(t_RFVARS,             QStringLiteral("RFVARS"));
    m_token_string.insert(t_RFWORD,             QStringLiteral("RFWORD"));
    m_token_string.insert(t_RGBEXP,             QStringLiteral("RGBEXP"));
    m_token_string.insert(t_RGBSQZ,             QStringLiteral("RGBSQZ"));
    m_token_string.insert(t_ROL,                QStringLiteral("ROL"));
    m_token_string.insert(t_ROLBYTE,            QStringLiteral("ROLBYTE"));
    m_token_string.insert(t_ROLNIB,             QStringLiteral("ROLNIB"));
    m_token_string.insert(t_ROLWORD,            QStringLiteral("ROLWORD"));
    m_token_string.insert(t_ROR,                QStringLiteral("ROR"));
    m_token_string.insert(t_RQPIN,              QStringLiteral("RQPIN"));
    m_token_string.insert(t_SAL,                QStringLiteral("SAL"));
    m_token_string.insert(t_SAR,                QStringLiteral("SAR"));
    m_token_string.insert(t_SCA,                QStringLiteral("SCA"));
    m_token_string.insert(t_SCAS,               QStringLiteral("SCAS"));
    m_token_string.insert(t_SETBYTE,            QStringLiteral("SETBYTE"));
    m_token_string.insert(t_SETCFRQ,            QStringLiteral("SETCFRQ"));
    m_token_string.insert(t_SETCI,              QStringLiteral("SETCI"));
    m_token_string.insert(t_SETCMOD,            QStringLiteral("SETCMOD"));
    m_token_string.insert(t_SETCQ,              QStringLiteral("SETCQ"));
    m_token_string.insert(t_SETCY,              QStringLiteral("SETCY"));
    m_token_string.insert(t_SETD,               QStringLiteral("SETD"));
    m_token_string.insert(t_SETDACS,            QStringLiteral("SETDACS"));
    m_token_string.insert(t_SETINT1,            QStringLiteral("SETINT1"));
    m_token_string.insert(t_SETINT2,            QStringLiteral("SETINT2"));
    m_token_string.insert(t_SETINT3,            QStringLiteral("SETINT3"));
    m_token_string.insert(t_SETLUTS,            QStringLiteral("SETLUTS"));
    m_token_string.insert(t_SETNIB,             QStringLiteral("SETNIB"));
    m_token_string.insert(t_SETPAT,             QStringLiteral("SETPAT"));
    m_token_string.insert(t_SETPIV,             QStringLiteral("SETPIV"));
    m_token_string.insert(t_SETPIX,             QStringLiteral("SETPIX"));
    m_token_string.insert(t_SETQ,               QStringLiteral("SETQ"));
    m_token_string.insert(t_SETQ2,              QStringLiteral("SETQ2"));
    m_token_string.insert(t_SETR,               QStringLiteral("SETR"));
    m_token_string.insert(t_SETS,               QStringLiteral("SETS"));
    m_token_string.insert(t_SETSCP,             QStringLiteral("SETSCP"));
    m_token_string.insert(t_SETSE1,             QStringLiteral("SETSE1"));
    m_token_string.insert(t_SETSE2,             QStringLiteral("SETSE2"));
    m_token_string.insert(t_SETSE3,             QStringLiteral("SETSE3"));
    m_token_string.insert(t_SETSE4,             QStringLiteral("SETSE4"));
    m_token_string.insert(t_SETWORD,            QStringLiteral("SETWORD"));
    m_token_string.insert(t_SETXFRQ,            QStringLiteral("SETXFRQ"));
    m_token_string.insert(t_SEUSSF,             QStringLiteral("SEUSSF"));
    m_token_string.insert(t_SEUSSR,             QStringLiteral("SEUSSR"));
    m_token_string.insert(t_SHL,                QStringLiteral("SHL"));
    m_token_string.insert(t_SHR,                QStringLiteral("SHR"));
    m_token_string.insert(t_SIGNX,              QStringLiteral("SIGNX"));
    m_token_string.insert(t_SKIP,               QStringLiteral("SKIP"));
    m_token_string.insert(t_SKIPF,              QStringLiteral("SKIPF"));
    m_token_string.insert(t_SPACE,              QStringLiteral("SPACE"));
    m_token_string.insert(t_SPLITB,             QStringLiteral("SPLITB"));
    m_token_string.insert(t_SPLITW,             QStringLiteral("SPLITW"));
    m_token_string.insert(t_STALLI,             QStringLiteral("STALLI"));
    m_token_string.insert(t_SUB,                QStringLiteral("SUB"));
    m_token_string.insert(t_SUBR,               QStringLiteral("SUBR"));
    m_token_string.insert(t_SUBS,               QStringLiteral("SUBS"));
    m_token_string.insert(t_SUBSX,              QStringLiteral("SUBSX"));
    m_token_string.insert(t_SUBX,               QStringLiteral("SUBX"));
    m_token_string.insert(t_SUMC,               QStringLiteral("SUMC"));
    m_token_string.insert(t_SUMNC,              QStringLiteral("SUMNC"));
    m_token_string.insert(t_SUMNZ,              QStringLiteral("SUMNZ"));
    m_token_string.insert(t_SUMZ,               QStringLiteral("SUMZ"));
    m_token_string.insert(t_TEST,               QStringLiteral("TEST"));
    m_token_string.insert(t_TESTB,              QStringLiteral("TESTB"));
    m_token_string.insert(t_TESTBN,             QStringLiteral("TESTBN"));
    m_token_string.insert(t_TESTN,              QStringLiteral("TESTN"));
    m_token_string.insert(t_TESTP,              QStringLiteral("TESTP"));
    m_token_string.insert(t_TESTPN,             QStringLiteral("TESTPN"));
    m_token_string.insert(t_TJF,                QStringLiteral("TJF"));
    m_token_string.insert(t_TJNF,               QStringLiteral("TJNF"));
    m_token_string.insert(t_TJNS,               QStringLiteral("TJNS"));
    m_token_string.insert(t_TJNZ,               QStringLiteral("TJNZ"));
    m_token_string.insert(t_TJS,                QStringLiteral("TJS"));
    m_token_string.insert(t_TJV,                QStringLiteral("TJV"));
    m_token_string.insert(t_TJZ,                QStringLiteral("TJZ"));
    m_token_string.insert(t_TRGINT1,            QStringLiteral("TRGINT1"));
    m_token_string.insert(t_TRGINT2,            QStringLiteral("TRGINT2"));
    m_token_string.insert(t_TRGINT3,            QStringLiteral("TRGINT3"));
    m_token_string.insert(t_WAITATN,            QStringLiteral("WAITATN"));
    m_token_string.insert(t_WAITCT1,            QStringLiteral("WAITCT1"));
    m_token_string.insert(t_WAITCT2,            QStringLiteral("WAITCT2"));
    m_token_string.insert(t_WAITCT3,            QStringLiteral("WAITCT3"));
    m_token_string.insert(t_WAITFBW,            QStringLiteral("WAITFBW"));
    m_token_string.insert(t_WAITINT,            QStringLiteral("WAITINT"));
    m_token_string.insert(t_WAITPAT,            QStringLiteral("WAITPAT"));
    m_token_string.insert(t_WAITSE1,            QStringLiteral("WAITSE1"));
    m_token_string.insert(t_WAITSE2,            QStringLiteral("WAITSE2"));
    m_token_string.insert(t_WAITSE3,            QStringLiteral("WAITSE3"));
    m_token_string.insert(t_WAITSE4,            QStringLiteral("WAITSE4"));
    m_token_string.insert(t_WAITX,              QStringLiteral("WAITX"));
    m_token_string.insert(t_WAITXFI,            QStringLiteral("WAITXFI"));
    m_token_string.insert(t_WAITXMT,            QStringLiteral("WAITXMT"));
    m_token_string.insert(t_WAITXRL,            QStringLiteral("WAITXRL"));
    m_token_string.insert(t_WAITXRO,            QStringLiteral("WAITXRO"));
    m_token_string.insert(t_WFBYTE,             QStringLiteral("WFBYTE"));
    m_token_string.insert(t_WFLONG,             QStringLiteral("WFLONG"));
    m_token_string.insert(t_WFWORD,             QStringLiteral("WFWORD"));
    m_token_string.insert(t_WMLONG,             QStringLiteral("WMLONG"));
    m_token_string.insert(t_WRBYTE,             QStringLiteral("WRBYTE"));
    m_token_string.insert(t_WRC,                QStringLiteral("WRC"));
    m_token_string.insert(t_WRFAST,             QStringLiteral("WRFAST"));
    m_token_string.insert(t_WRLONG,             QStringLiteral("WRLONG"));
    m_token_string.insert(t_WRLUT,              QStringLiteral("WRLUT"));
    m_token_string.insert(t_WRNC,               QStringLiteral("WRNC"));
    m_token_string.insert(t_WRNZ,               QStringLiteral("WRNZ"));
    m_token_string.insert(t_WRPIN,              QStringLiteral("WRPIN"));
    m_token_string.insert(t_WRWORD,             QStringLiteral("WRWORD"));
    m_token_string.insert(t_WRZ,                QStringLiteral("WRZ"));
    m_token_string.insert(t_WXPIN,              QStringLiteral("WXPIN"));
    m_token_string.insert(t_WYPIN,              QStringLiteral("WYPIN"));
    m_token_string.insert(t_XCONT,              QStringLiteral("XCONT"));
    m_token_string.insert(t_XINIT,              QStringLiteral("XINIT"));
    m_token_string.insert(t_XOR,                QStringLiteral("XOR"));
    m_token_string.insert(t_XORO32,             QStringLiteral("XORO32"));
    m_token_string.insert(t_XSTOP,              QStringLiteral("XSTOP"));
    m_token_string.insert(t_XZERO,              QStringLiteral("XZERO"));
    m_token_string.insert(t_ZEROX,              QStringLiteral("ZEROX"));

    m_token_string.insert(t_WC,                 QStringLiteral("WC"));
    m_token_string.insert(t_WZ,                 QStringLiteral("WZ"));
    m_token_string.insert(t_WCZ,                QStringLiteral("WCZ"));
    m_token_string.insert(t_ANDC,               QStringLiteral("ANDC"));
    m_token_string.insert(t_ANDZ,               QStringLiteral("ANDZ"));
    m_token_string.insert(t_ORC,                QStringLiteral("ORC"));
    m_token_string.insert(t_ORZ,                QStringLiteral("ORZ"));
    m_token_string.insert(t_XORC,               QStringLiteral("XORC"));
    m_token_string.insert(t_XORZ,               QStringLiteral("XORZ"));

    m_token_string.insert(t__BYTE,              QStringLiteral("BYTE"));
    m_token_string.insert(t__WORD,              QStringLiteral("WORD"));
    m_token_string.insert(t__LONG,              QStringLiteral("LONG"));
    m_token_string.insert(t__RES,               QStringLiteral("RES"));
    m_token_string.insert(t__FIT,               QStringLiteral("FIT"));

    m_token_string.insert(t__DAT,               QStringLiteral("DAT"));
    m_token_string.insert(t__CON,               QStringLiteral("CON"));
    m_token_string.insert(t__PUB,               QStringLiteral("PUB"));
    m_token_string.insert(t__PRI,               QStringLiteral("PRI"));
    m_token_string.insert(t__VAR,               QStringLiteral("VAR"));

    m_token_string.insert(t__ORG,               QStringLiteral("ORG"));
    m_token_string.insert(t__ORGH,              QStringLiteral("ORGH"));
    m_token_string.insert(t__ASSIGN,            QStringLiteral("="));
    m_token_string.insert(t__DOLLAR,            QStringLiteral("$"));

    m_token_string.insert(t__EQ,                QStringLiteral("=="));
    m_token_string.insert(t__NE,                QStringLiteral("!="));
    m_token_string.insert(t__GE,                QStringLiteral(">="));
    m_token_string.insert(t__GT,                QStringLiteral(">"));
    m_token_string.insert(t__LE,                QStringLiteral("<="));
    m_token_string.insert(t__LT,                QStringLiteral("<"));

    m_token_string.insert(t__LPAREN,            QStringLiteral("("));
    m_token_string.insert(t__RPAREN,            QStringLiteral(")"));

    m_token_string.insert(t__NEG,               QStringLiteral("!"));
    m_token_string.insert(t__NOT,               QStringLiteral("~"));
    m_token_string.insert(t__INC,               QStringLiteral("++"));
    m_token_string.insert(t__DEC,               QStringLiteral("--"));

    m_token_string.insert(t__ADD,               QStringLiteral("+"));
    m_token_string.insert(t__SUB,               QStringLiteral("-"));

    m_token_string.insert(t__MUL,               QStringLiteral("*"));
    m_token_string.insert(t__DIV,               QStringLiteral("/"));
    m_token_string.insert(t__MOD,               QStringLiteral("\\"));

    m_token_string.insert(t__AND,               QStringLiteral("&"));
    m_token_string.insert(t__OR,                QStringLiteral("|"));
    m_token_string.insert(t__XOR,               QStringLiteral("^"));
    m_token_string.insert(t__SHL,               QStringLiteral("<<"));
    m_token_string.insert(t__SHR,               QStringLiteral(">>"));
    m_token_string.insert(t__REV,               QStringLiteral("><"));

    // Set the unary operators
    m_token_types.insert(t__NEG,                tt_unary);
    m_token_types.insert(t__NOT,                tt_unary);
    m_token_types.insert(t__SUB,                tt_unary);
    m_token_types.insert(t__INC,                tt_unary);
    m_token_types.insert(t__DEC,                tt_unary);

    // Set the binary operators
    m_token_types.insert(t__AND,                tt_binop);
    m_token_types.insert(t__OR,                 tt_binop);
    m_token_types.insert(t__XOR,                tt_binop);
    m_token_types.insert(t__SHL,                tt_binop);
    m_token_types.insert(t__SHR,                tt_binop);

    // Set the addition operators
    m_token_types.insert(t__ADD,                tt_addop);
    m_token_types.insert(t__SUB,                tt_addop);

    // Set the multiplication operators
    m_token_types.insert(t__MUL,                tt_mulop);
    m_token_types.insert(t__DIV,                tt_mulop);
    m_token_types.insert(t__MOD,                tt_mulop);

    // Set the conditionals
    m_token_types.insert(t__RET_,               tt_conditional);
    m_token_types.insert(t_IF_NZ_AND_NC,        tt_conditional);
    m_token_types.insert(t_IF_NC_AND_NZ,        tt_conditional);
    m_token_types.insert(t_IF_A,                tt_conditional);
    m_token_types.insert(t_IF_GT,               tt_conditional);
    m_token_types.insert(t_IF_Z_AND_NC,         tt_conditional);
    m_token_types.insert(t_IF_NC_AND_Z,         tt_conditional);
    m_token_types.insert(t_IF_NC,               tt_conditional);
    m_token_types.insert(t_IF_AE,               tt_conditional);
    m_token_types.insert(t_IF_GE,               tt_conditional);
    m_token_types.insert(t_IF_NZ_AND_C,         tt_conditional);
    m_token_types.insert(t_IF_C_AND_NZ,         tt_conditional);
    m_token_types.insert(t_IF_NZ,               tt_conditional);
    m_token_types.insert(t_IF_NE,               tt_conditional);
    m_token_types.insert(t_IF_Z_NE_C,           tt_conditional);
    m_token_types.insert(t_IF_C_NE_Z,           tt_conditional);
    m_token_types.insert(t_IF_NZ_OR_NC,         tt_conditional);
    m_token_types.insert(t_IF_NC_OR_NZ,         tt_conditional);
    m_token_types.insert(t_IF_Z_AND_C,          tt_conditional);
    m_token_types.insert(t_IF_C_AND_Z,          tt_conditional);
    m_token_types.insert(t_IF_Z_EQ_C,           tt_conditional);
    m_token_types.insert(t_IF_C_EQ_Z,           tt_conditional);
    m_token_types.insert(t_IF_Z,                tt_conditional);
    m_token_types.insert(t_IF_E,                tt_conditional);
    m_token_types.insert(t_IF_Z_OR_NC,          tt_conditional);
    m_token_types.insert(t_IF_NC_OR_Z,          tt_conditional);
    m_token_types.insert(t_IF_C,                tt_conditional);
    m_token_types.insert(t_IF_B,                tt_conditional);
    m_token_types.insert(t_IF_LT,               tt_conditional);
    m_token_types.insert(t_IF_NZ_OR_C,          tt_conditional);
    m_token_types.insert(t_IF_C_OR_NZ,          tt_conditional);
    m_token_types.insert(t_IF_Z_OR_C,           tt_conditional);
    m_token_types.insert(t_IF_C_OR_Z,           tt_conditional);
    m_token_types.insert(t_IF_BE,               tt_conditional);
    m_token_types.insert(t_IF_LE,               tt_conditional);
    m_token_types.insert(t_IF_ALWAYS,           tt_conditional);

    // Set the MODCZ tokens
    m_token_types.insert(t_MODCZ__CLR,          tt_modcz_param);
    m_token_types.insert(t_MODCZ__NC_AND_NZ,    tt_modcz_param);
    m_token_types.insert(t_MODCZ__NZ_AND_NC,    tt_modcz_param);
    m_token_types.insert(t_MODCZ__GT,           tt_modcz_param);
    m_token_types.insert(t_MODCZ__NC_AND_Z,     tt_modcz_param);
    m_token_types.insert(t_MODCZ__Z_AND_NC,     tt_modcz_param);
    m_token_types.insert(t_MODCZ__NC,           tt_modcz_param);
    m_token_types.insert(t_MODCZ__GE,           tt_modcz_param);
    m_token_types.insert(t_MODCZ__C_AND_NZ,     tt_modcz_param);
    m_token_types.insert(t_MODCZ__NZ_AND_C,     tt_modcz_param);
    m_token_types.insert(t_MODCZ__NZ,           tt_modcz_param);
    m_token_types.insert(t_MODCZ__NE,           tt_modcz_param);
    m_token_types.insert(t_MODCZ__C_NE_Z,       tt_modcz_param);
    m_token_types.insert(t_MODCZ__Z_NE_C,       tt_modcz_param);
    m_token_types.insert(t_MODCZ__NC_OR_NZ,     tt_modcz_param);
    m_token_types.insert(t_MODCZ__NZ_OR_NC,     tt_modcz_param);
    m_token_types.insert(t_MODCZ__C_AND_Z,      tt_modcz_param);
    m_token_types.insert(t_MODCZ__Z_AND_C,      tt_modcz_param);
    m_token_types.insert(t_MODCZ__C_EQ_Z,       tt_modcz_param);
    m_token_types.insert(t_MODCZ__Z_EQ_C,       tt_modcz_param);
    m_token_types.insert(t_MODCZ__Z,            tt_modcz_param);
    m_token_types.insert(t_MODCZ__E,            tt_modcz_param);
    m_token_types.insert(t_MODCZ__NC_OR_Z,      tt_modcz_param);
    m_token_types.insert(t_MODCZ__Z_OR_NC,      tt_modcz_param);
    m_token_types.insert(t_MODCZ__C,            tt_modcz_param);
    m_token_types.insert(t_MODCZ__LT,           tt_modcz_param);
    m_token_types.insert(t_MODCZ__C_OR_NZ,      tt_modcz_param);
    m_token_types.insert(t_MODCZ__NZ_OR_C,      tt_modcz_param);
    m_token_types.insert(t_MODCZ__C_OR_Z,       tt_modcz_param);
    m_token_types.insert(t_MODCZ__Z_OR_C,       tt_modcz_param);
    m_token_types.insert(t_MODCZ__LE,           tt_modcz_param);
    m_token_types.insert(t_MODCZ__SET,          tt_modcz_param);


    // Section control
    m_token_types.insert(t__DAT,                tt_section);
    m_token_types.insert(t__CON,                tt_section);
    m_token_types.insert(t__PUB,                tt_section);
    m_token_types.insert(t__PRI,                tt_section);
    m_token_types.insert(t__VAR,                tt_section);

    // Origin control
    m_token_types.insert(t__ORG,                tt_origin);
    m_token_types.insert(t__ORGH,               tt_origin);
    m_token_types.insert(t__FIT,                tt_origin);

    // Data types and space reserving
    m_token_types.insert(t__BYTE,               tt_data);
    m_token_types.insert(t__WORD,               tt_data);
    m_token_types.insert(t__LONG,               tt_data);
    m_token_types.insert(t__RES,                tt_data);

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
    m_lookup_modcz.insert(t_MODCZ__CLR,        cc_clr);
    m_lookup_modcz.insert(t_MODCZ__NC_AND_NZ,  cc_nc_and_nz);
    m_lookup_modcz.insert(t_MODCZ__NZ_AND_NC,  cc_nc_and_nz);
    m_lookup_modcz.insert(t_MODCZ__GT,         cc_nc_and_nz);
    m_lookup_modcz.insert(t_MODCZ__NC_AND_Z,   cc_nc_and_z);
    m_lookup_modcz.insert(t_MODCZ__Z_AND_NC,   cc_nc_and_z);
    m_lookup_modcz.insert(t_MODCZ__NC,         cc_nc);
    m_lookup_modcz.insert(t_MODCZ__GE,         cc_nc);
    m_lookup_modcz.insert(t_MODCZ__C_AND_NZ,   cc_c_and_nz);
    m_lookup_modcz.insert(t_MODCZ__NZ_AND_C,   cc_c_and_nz);
    m_lookup_modcz.insert(t_MODCZ__NZ,         cc_nz);
    m_lookup_modcz.insert(t_MODCZ__NE,         cc_nz);
    m_lookup_modcz.insert(t_MODCZ__C_NE_Z,     cc_c_ne_z);
    m_lookup_modcz.insert(t_MODCZ__Z_NE_C,     cc_c_ne_z);
    m_lookup_modcz.insert(t_MODCZ__NC_OR_NZ,   cc_nc_or_nz);
    m_lookup_modcz.insert(t_MODCZ__NZ_OR_NC,   cc_nc_or_nz);
    m_lookup_modcz.insert(t_MODCZ__C_AND_Z,    cc_c_and_z);
    m_lookup_modcz.insert(t_MODCZ__Z_AND_C,    cc_c_and_z);
    m_lookup_modcz.insert(t_MODCZ__C_EQ_Z,     cc_c_eq_z);
    m_lookup_modcz.insert(t_MODCZ__Z_EQ_C,     cc_c_eq_z);
    m_lookup_modcz.insert(t_MODCZ__Z,          cc_z);
    m_lookup_modcz.insert(t_MODCZ__E,          cc_z);
    m_lookup_modcz.insert(t_MODCZ__NC_OR_Z,    cc_nc_or_z);
    m_lookup_modcz.insert(t_MODCZ__Z_OR_NC,    cc_nc_or_z);
    m_lookup_modcz.insert(t_MODCZ__C,          cc_c);
    m_lookup_modcz.insert(t_MODCZ__LT,         cc_c);
    m_lookup_modcz.insert(t_MODCZ__C_OR_NZ,    cc_c_or_nz);
    m_lookup_modcz.insert(t_MODCZ__NZ_OR_C,    cc_c_or_nz);
    m_lookup_modcz.insert(t_MODCZ__C_OR_Z,     cc_c_or_z);
    m_lookup_modcz.insert(t_MODCZ__Z_OR_C,     cc_c_or_z);
    m_lookup_modcz.insert(t_MODCZ__LE,         cc_c_or_z);
    m_lookup_modcz.insert(t_MODCZ__SET,        cc_always);

    // Build the reverse QMultiHash for string lookup
    foreach(p2_token_e tok, m_token_string.keys())
        foreach(const QString& str, m_token_string.values(tok))
            m_string_token.insert(str, tok);

    // Build the reverse QMultiHash for types lookup
    foreach(p2_token_e tok, m_token_types.keys())
        foreach(p2_tokentype_e typ, m_token_types.values(tok))
            m_type_tokens.insert(typ, tok);

    m_tokentype_name.insert(tt_none,            QStringLiteral("-"));
    m_tokentype_name.insert(tt_unary,           QStringLiteral("unary"));
    m_tokentype_name.insert(tt_binop,           QStringLiteral("binary"));
    m_tokentype_name.insert(tt_addop,           QStringLiteral("addops"));
    m_tokentype_name.insert(tt_mulop,           QStringLiteral("mulops"));
    m_tokentype_name.insert(tt_conditional,     QStringLiteral("conditional"));
    m_tokentype_name.insert(tt_modcz_param,     QStringLiteral("MODCZ"));
    m_tokentype_name.insert(tt_section,         QStringLiteral("section"));
    m_tokentype_name.insert(tt_origin,          QStringLiteral("origin"));
    m_tokentype_name.insert(tt_data,            QStringLiteral("data"));
}

/**
 * @brief Return a QString for the given %token
 * @param token one of p2_toke_e enume
 * @return QString in uppercase or lowercase
 */
QString P2Token::string(p2_token_e tok, bool lowercase) const
{
    QString str = m_token_string.value(tok);
    return lowercase ? str.toLower() : str;
}

/**
 * @brief Return a p2_token_e enumeration value for the QString %str
 * @param str QString to scan for
 * @param chop if true, chop off characters until a a token other than t_nothing is matched
 * @return p2_token_e enumeration value, or t_nothing if not a known string
 */
p2_token_e P2Token::token(const QString& str, bool chop) const
{
    QString ustr = str.toUpper().remove(QRegExp("\\s"));

    // Ignore leading '#' characters (for immediate modes)
    while (ustr.startsWith(QChar('#')))
        ustr.remove(0, 1);

    // Ignore leading '@' characters (for relative modes)
    while (ustr.startsWith(QChar('@')))
        ustr.remove(0, 1);

    p2_token_e tok = m_string_token.value(ustr, t_unknown);
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
            tok = t_value_bin;
            break;
        }

        if (0 == ustr.indexOf(rx_byt)) {
            tok = t_value_byt;
            break;
        }

        if (0 == ustr.indexOf(rx_oct)) {
            tok = t_value_oct;
            break;
        }

        if (0 == ustr.indexOf(rx_hex)) {
            tok = t_value_hex;
            break;
        }

        if (0 == ustr.indexOf(rx_dec)) {
            tok = t_value_dec;
            break;
        }

        if (0 == ustr.indexOf(rx_string)) {
            tok = t_string;
            break;
        }

        if (!chop)
            break;
        ustr.chop(1);
    }
    return tok;
}

p2_tokentype_e P2Token::type(p2_token_e tok)
{
    return m_token_types.value(tok, tt_none);
}

QString P2Token::typeName(p2_token_e tok)
{
    p2_tokentype_e type = m_token_types.value(tok, tt_none);
    return m_tokentype_name.value(type);
}

p2_tokentype_e P2Token::type(const QString& str)
{
    p2_token_e tok = m_string_token.value(str, t_invalid);
    return m_token_types.value(tok, tt_none);
}

p2_token_e P2Token::at_token(int& pos, const QString& str, QList<p2_token_e> tokens, p2_token_e dflt) const
{
    const int len = str.length();
    for (int n = 1; pos + n < len && n < 9; n++) {
        const QString& substr = str.mid(pos, n).toUpper();
        const p2_token_e tok = m_string_token.value(substr, dflt);
        if (tokens.contains(tok)) {
            pos += n;
            return tok;
        }
    }
    return t_invalid;
}

p2_token_e P2Token::at_token(const QString& str, QList<p2_token_e> tokens, p2_token_e dflt) const
{
    int pos = 0;
    return at_token(pos, str, tokens, dflt);
}

bool P2Token::is_type(p2_token_e tok, p2_tokentype_e type) const
{
    return m_token_types.contains(tok, type);
}

bool P2Token::is_unary(p2_token_e tok) const
{
    return is_type(tok, tt_unary);
}

bool P2Token::is_binop(p2_token_e tok) const
{
    return is_type(tok, tt_binop);
}

bool P2Token::is_addop(p2_token_e tok) const
{
    return is_type(tok, tt_addop);
}

bool P2Token::is_mulop(p2_token_e tok) const
{
    return is_type(tok, tt_mulop);
}

bool P2Token::is_operation(p2_token_e tok) const
{
    const QList<p2_tokentype_e>& list = m_token_types.values(tok);
    return list.contains(tt_unary) || list.contains(tt_binop) || list.contains(tt_addop) || list.contains(tt_mulop);
}

bool P2Token::is_conditional(p2_token_e tok) const
{
    return is_type(tok, tt_conditional);
}

bool P2Token::is_modcz_param(p2_token_e tok) const
{
    return is_type(tok, tt_modcz_param);
}

p2_token_e P2Token::at_type(int& pos, const QString& str, p2_tokentype_e type, p2_token_e dflt) const
{
    QList<p2_token_e> tokens = m_token_types.keys(type);
    return at_token(pos, str, tokens, dflt);
}

p2_token_e P2Token::at_type(const QString& str, p2_tokentype_e type, p2_token_e dflt) const
{
    int pos = 0;
    return at_type(pos, str, type, dflt);
}

p2_token_e P2Token::at_unary(int& pos, const QString& str, p2_token_e dflt) const
{
    return at_type(pos, str, tt_unary, dflt);
}

p2_token_e P2Token::at_unary(const QString& str, p2_token_e dflt) const
{
    int pos = 0;
    return at_type(pos, str, tt_unary, dflt);
}

p2_token_e P2Token::at_binop(int& pos, const QString& str, p2_token_e dflt) const
{
    return at_type(pos, str, tt_binop, dflt);
}

p2_token_e P2Token::at_binop(const QString& str, p2_token_e dflt) const
{
    int pos = 0;
    return at_type(pos, str, tt_binop, dflt);
}

p2_token_e P2Token::at_addop(int& pos, const QString& str, p2_token_e dflt) const
{
    return at_type(pos, str, tt_addop, dflt);
}

p2_token_e P2Token::at_addop(const QString& str, p2_token_e dflt) const
{
    int pos = 0;
    return at_type(pos, str, tt_addop, dflt);
}

p2_token_e P2Token::at_mulop(int &pos, const QString& str, p2_token_e dflt) const
{
    return at_type(pos, str, tt_mulop, dflt);
}

p2_token_e P2Token::at_mulop(const QString& str, p2_token_e dflt) const
{
    int pos = 0;
    return at_type(pos, str, tt_mulop, dflt);
}

p2_token_e P2Token::at_conditional(int& pos, const QString& str, p2_token_e dflt) const
{
    return at_type(pos, str, tt_conditional, dflt);
}

p2_token_e P2Token::at_conditional(const QString& str, p2_token_e dflt) const
{
    int pos = 0;
    return at_type(pos, str, tt_conditional, dflt);
}

p2_token_e P2Token::at_modcz_param(int& pos, const QString& str, p2_token_e dflt) const
{
    return at_type(pos, str, tt_modcz_param, dflt);
}

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
 * @param cond string with the condition
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
 * @param cond string with the parameter
 * @param dflt default parameter if string is not a MODCZ parameter
 * @return One of the 16 p2_cond_e values
 */
p2_cond_e P2Token::modcz_param(const QString& str, p2_cond_e dflt) const
{
    int pos = 0;
    p2_token_e cond = at_type(pos, str, tt_modcz_param);
    return m_lookup_modcz.value(cond, dflt);
}
