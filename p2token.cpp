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
#include "p2util.h"

#define DBG_REGEX 0
#define DBG_TOKEN 0

#if DBG_REGEX
#define DEBUG_REGEX(x,...) qDebug(x,__VA_ARGS__)
#else
#define DEBUG_REGEX(x,...)
#endif

#if DBG_TOKEN
#define DEBUG_TOKEN(x,...) qDebug(x,__VA_ARGS__)
#else
#define DEBUG_TOKEN(x,...)
#endif

/**
 * @brief Regular expression for comments
 */
static const QString re_comment_curly = QLatin1String("\\{[^\\}]*\\}");

/**
 * @brief Regular expression for comments
 */
//static const QString re_comment_eol = QLatin1String("'[^\\{\\}]*");
static const QString re_comment_eol = QLatin1String("^'.*");

/**
 * @brief Regular expression for local symbol
 *
 * leading "."
 * then any number of "A"…"Z", "0"…"9", or "_"
 */
static const QString re_locsym = QLatin1String("^[.][A-Z_]+[A-Z0-9_]*");

/**
 * @brief Regular expression for alphanumeric
 *
 * leading "A"…"Z", or "_"
 * then any number of "A"…"Z", "0"…"9", or "_"
 */
static const QString re_symbol = QLatin1String("^[A-Z_]+[A-Z0-9_]*");

/**
 * @brief Regular expression for binary number
 *
 * leading "%"
 * then one or more of "0", "1", or "_"
 */
static const QString re_bin_const = QLatin1String("^%[_]*[01_]+");

/**
 * @brief Regular expression for byte number
 *
 * leading "%%"
 * then one or more of "0"…"3", or "_"
 */
static const QString re_byt_const = QLatin1String("^%%[_]*[0-3_]+");

/**
 * @brief Regular expression for a decimal number
 *
 * leading "0"…"9"
 * then any number of "0"…"9", or "_"
 */
static const QString re_dec_const = QLatin1String("^[0-9_]+(?!\\.)");

/**
 * @brief Regular expression an octal number
 *
 * leading "$"
 * then any number of "0"…"9", "A"…"F", or "_"
 */
static const QString re_hex_const = QLatin1String("^\\$[_]*[0-9A-F_]+");

/**
 * @brief Regular expression for a string enclosed in doublequotes, possibly containing escaped doublequotes
 * initial '"'
 * then any number of escaped doublequotes (\") or other characters (.)
 * trailing '"'
 */
static const QString re_str_const = QLatin1String("^\"([^\\\"]|\\\\.)*\"");

/**
 * @brief Regular expression for a qreal number
 *
 * leading any number of "0"…"9"
 * then one period "."
 * then any number of "0"…"9"
 */
static const QString re_real_const = QLatin1String("^[0-9_]*\\.[0-9_]*");

//! Global static instance of the P2Token class
P2Token Token;


/**
 * @file Chip Gracey's comment from https://forums.parallax.com/discussion/170176/spin2-syntax
 *
 *                 Instruction/
 * Operator	Term Assign	Priority	Term		Priority	Type		Description
 * -------------------------------------------------------------------------------------------------------------------------------------------------
 * ++ (pre)	++var		1		++var		1		var prefix	Pre-increment
 * -- (pre)	--var		1		--var		1		var prefix	Pre-decrement
 * ?? (pre)	??var		1		??var		1		var prefix	XORO32, iterate x and return pseudo-random
 *
 * ++ (post)	var++		1		var++		1		var postfix	Post-increment
 * -- (post)	var--		1		var--		1		var postfix	Post-decrement
 * !! (post)	var!!		1		var!!		1		var postfix	Post-logical NOT
 * !  (post)	var!		1		var!		1		var postfix	Post-bitwise NOT
 * \  (post)	var\x		1		var\x		1		var postfix	Post-set to y
 *
 * !		!= var		1		!x		2		unary		Bitwise NOT, 1's complement
 * -		-= var		1		-x		2		unary		Negation, 2's complement
 * ABS		ABS= var	1		ABS x		2		unary		Absolute value
 * ENCOD        ENCOD= var	1		ENCOD x		2		unary		Encode MSB, 31..0
 * DECOD        DECOD= var	1		DECOD x		2		unary		Decode, 1 << (x & $1F)
 * ONES		ONES= var	1		ONES x		2		unary		Count ones
 * SQRT		SQRT= var	1		SQRT x		2		unary		Square root of unsigned x
 * QLOG		QLOG= var	1		QLOG x		2		unary		Unsigned to logarithm
 * QEXP		QEXP= var	1		QEXP x		2		unary		Logarithm to unsigned
 *
 * >>		var >>= x	16		x >> y		3		binary		Shift right, insert 0's
 * <<		var <<= x	16		x << y		3		binary		Shift left, insert 0's
 * SAR		var SAR= x	16		x SAR y		3		binary		Shift right, insert MSB's
 * ROR		var ROR= x	16		x ROR y		3		binary		Rotate right
 * ROL		var ROL= x	16		x ROL y		3		binary		Rotate left
 * REV		var REV= x	16		x REV y		3		binary		Reverse y LSBs of x and zero-extend
 * ZEROX        var ZEROX= x	16		x ZEROX y	3		binary		Zero-extend above bit y
 * SIGNX        var SIGNX= x	16		x SIGNX y	3		binary		Sign-extend from bit y
 *
 * &		var &= x	16		x & y		4		binary		Bitwise AND
 * ^		var ^= x	16		x ^ y		5		binary		Bitwise XOR
 * |		var |= x	16		x | y		6		binary		Bitwise OR
 *
 * *		var *= x	16		x * y		7		binary		Signed multiply
 * /		var /= x	16		x / y		7		binary		Signed divide, return quotient
 * +/		var +/= x	16		x +/ y		7		binary		Unsigned divide, return quotient
 * //		var //= x	16		x // y		7		binary		Signed divide, return remainder
 * +//		var +//= x	16		x +// y		7		binary		Unsigned divide, return remainder
 * SCA		var SCA= x	16		x SCA y		7		binary		Unsigned scale (x * y) >> 32
 * SCAS		var SCAS= x	16		x SCAS y	7		binary		Signed scale (x * y) >> 30
 * FRAC		var FRAC= x	16		x FRAC y	7		binary		Unsigned fraction {x, 32'b0} / y
 *
 * +		var += x	16		x + y		8		binary		Add
 * -		var -= x	16		x - y		8		binary		Subtract
 *
 * #>		var #>= x	16		x #> y		9		binary		Ensure x => y, signed
 * <#		var <#= x	16		x <# y		9		binary		Ensure x <= y, signed
 *
 * <				-		x < y		10		binary		Signed less than		(returns 0 or -1)
 * +<				-		x +< y		10		binary		Unsigned less than		(returns 0 or -1)
 * <=				-		x <= y		10		binary		Signed less than or equal	(returns 0 or -1)
 * +<=				-		x +<= y		10		binary		Unsigned less than or equal	(returns 0 or -1)
 * ==				-		x == y		10		binary		Equal				(returns 0 or -1)
 * <>				-		x <> y		10		binary		Not equal			(returns 0 or -1)
 * >=				-		x >= y		10		binary		Signed greater than or equal	(returns 0 or -1)
 * +>=				-		x +>= y		10		binary		Unsigned greater than or equal	(returns 0 or -1)
 * >				-		x > y		10		binary		Signed greater than		(returns 0 or -1)
 * +>				-		x +> y		10		binary		Unsigned greater than		(returns 0 or -1)
 * <=>				-		x <=> y		10		binary		Signed comparison	   (<,=,> returns -1,0,1)
 *
 * !!, NOT      !!= var		1		!!x		11		unary		Logical NOT  (x == 0,            returns 0 or -1)
 * &&, AND      var &&= x	16		x && y		12		binary		Logical AND  (x <> 0 AND y <> 0, returns 0 or -1)
 * ^^, XOR      var ^^= x	16		x ^^ y		13		binary		Logical XOR  (x <> 0 XOR y <> 0, returns 0 or -1)
 * ||, OR       var ||= x	16		x || y		14		binary		Logical OR   (x <> 0 OR  y <> 0, returns 0 or -1)
 *
 * ? :						x ? y : z	15		ternary		Choose between y and z
 *
 * :=		var := x	16						assign		Set var to x
 *
 * (,,,) :=	(x,y) := (a,b)							assign		set (x,y) to (a,b), 2..16 variables
 *
 *
 * Math functions
 * ---------------------------------------------------------------------------------------------
 * (x,y) := ROTXY(x,y,t)		Rotate cartesian (x,y) by t and assign resultant (x,y)
 * (r,t) := XYPOL(x,y)		Convert cartesian (x,y) to polar and assign resultant (r,t)
 * (x,y) := POLXY(r,t)		Convert polar (r,t) to cartesian and assign resultant (x,y)
 *
 * Miscellaneous
 * ---------------------------------------------------------------------------------------------
 * SWAP(x,y)			Swap variables x and y (uses '\' bytecode for efficiency)
 *
 */

#define TN_ADD(token,mask,string) tn_ADD(token,#token,mask,string)

P2Token::P2Token()
    : m_token_enum_name()
    , m_token_string()
    , m_string_token()
    , m_token_type()
    , m_type_token()
    , m_lookup_cond()
    , m_lookup_modcz()
    , m_t_type_name()
    , rx_comment_eol(re_comment_eol, Qt::CaseInsensitive)
    , rx_comment_curly(re_comment_curly, Qt::CaseInsensitive)
    , rx_symbol(re_symbol, Qt::CaseInsensitive)
    , rx_locsym(re_locsym, Qt::CaseInsensitive)
    , rx_bin_const(re_bin_const, Qt::CaseInsensitive)
    , rx_byt_const(re_byt_const, Qt::CaseInsensitive)
    , rx_hex_const(re_hex_const, Qt::CaseInsensitive)
    , rx_dec_const(re_dec_const, Qt::CaseInsensitive)
    , rx_str_const(re_str_const, Qt::CaseInsensitive)
    , rx_real_const(re_real_const, Qt::CaseInsensitive)
{
    TN_ADD(t_invalid,           tm_lexer, QStringLiteral("·INVALID·"));
    TN_ADD(t_none,              tm_none, QString());
    TN_ADD(t_unknown,           tm_lexer, QStringLiteral("·unknown·"));
    TN_ADD(t_comment,           tm_comment, QStringLiteral("·comment·"));
    TN_ADD(t_comment_eol,       tm_comment, QStringLiteral("'"));
    TN_ADD(t_comment_lcurly,    tm_comment, QStringLiteral("{"));
    TN_ADD(t_comment_rcurly,    tm_comment, QStringLiteral("}"));
    TN_ADD(t_str_const,         tm_lexer, QStringLiteral("·str_const·"));
    TN_ADD(t_bin_const,         tm_lexer, QStringLiteral("·bin_const·"));
    TN_ADD(t_byt_const,         tm_lexer, QStringLiteral("·byt_const·"));
    TN_ADD(t_dec_const,         tm_lexer, QStringLiteral("·dec_const·"));
    TN_ADD(t_real_const,        tm_lexer, QStringLiteral("·real_const·"));
    TN_ADD(t_hex_const,         tm_lexer, QStringLiteral("·hex_const·"));
    TN_ADD(t_locsym,            tm_lexer, QStringLiteral("·locsym·"));
    TN_ADD(t_symbol,            tm_lexer, QStringLiteral("·symbol·"));

    TN_ADD(t_D,                 tm_none, QStringLiteral("»D«"));
    TN_ADD(t_S,                 tm_none, QStringLiteral("»S«"));

    TN_ADD(t_ABS,               tm_mnemonic, QLatin1String("ABS"));
    TN_ADD(t_ADD,               tm_mnemonic, QLatin1String("ADD"));
    TN_ADD(t_ADDCT1,            tm_mnemonic, QLatin1String("ADDCT1"));
    TN_ADD(t_ADDCT2,            tm_mnemonic, QLatin1String("ADDCT2"));
    TN_ADD(t_ADDCT3,            tm_mnemonic, QLatin1String("ADDCT3"));
    TN_ADD(t_ADDPIX,            tm_mnemonic, QLatin1String("ADDPIX"));
    TN_ADD(t_ADDS,              tm_mnemonic, QLatin1String("ADDS"));
    TN_ADD(t_ADDSX,             tm_mnemonic, QLatin1String("ADDSX"));
    TN_ADD(t_ADDX,              tm_mnemonic, QLatin1String("ADDX"));
    TN_ADD(t_AKPIN,             tm_mnemonic, QLatin1String("AKPIN"));
    TN_ADD(t_ALLOWI,            tm_mnemonic, QLatin1String("ALLOWI"));
    TN_ADD(t_ALTB,              tm_mnemonic, QLatin1String("ALTB"));
    TN_ADD(t_ALTD,              tm_mnemonic, QLatin1String("ALTD"));
    TN_ADD(t_ALTGB,             tm_mnemonic, QLatin1String("ALTGB"));
    TN_ADD(t_ALTGN,             tm_mnemonic, QLatin1String("ALTGN"));
    TN_ADD(t_ALTGW,             tm_mnemonic, QLatin1String("ALTGW"));
    TN_ADD(t_ALTI,              tm_mnemonic, QLatin1String("ALTI"));
    TN_ADD(t_ALTR,              tm_mnemonic, QLatin1String("ALTR"));
    TN_ADD(t_ALTS,              tm_mnemonic, QLatin1String("ALTS"));
    TN_ADD(t_ALTSB,             tm_mnemonic, QLatin1String("ALTSB"));
    TN_ADD(t_ALTSN,             tm_mnemonic, QLatin1String("ALTSN"));
    TN_ADD(t_ALTSW,             tm_mnemonic, QLatin1String("ALTSW"));
    TN_ADD(t_AND,               tm_mnemonic, QLatin1String("AND"));
    TN_ADD(t_ANDN,              tm_mnemonic, QLatin1String("ANDN"));
    TN_ADD(t_AUGD,              tm_mnemonic, QLatin1String("AUGD"));
    TN_ADD(t_AUGS,              tm_mnemonic, QLatin1String("AUGS"));
    TN_ADD(t_BITC,              tm_mnemonic, QLatin1String("BITC"));
    TN_ADD(t_BITH,              tm_mnemonic, QLatin1String("BITH"));
    TN_ADD(t_BITL,              tm_mnemonic, QLatin1String("BITL"));
    TN_ADD(t_BITNC,             tm_mnemonic, QLatin1String("BITNC"));
    TN_ADD(t_BITNOT,            tm_mnemonic, QLatin1String("BITNOT"));
    TN_ADD(t_BITNZ,             tm_mnemonic, QLatin1String("BITNZ"));
    TN_ADD(t_BITRND,            tm_mnemonic, QLatin1String("BITRND"));
    TN_ADD(t_BITZ,              tm_mnemonic, QLatin1String("BITZ"));
    TN_ADD(t_BLNPIX,            tm_mnemonic, QLatin1String("BLNPIX"));
    TN_ADD(t_BMASK,             tm_mnemonic, QLatin1String("BMASK"));
    TN_ADD(t_BRK,               tm_mnemonic, QLatin1String("BRK"));
    TN_ADD(t_CALL,              tm_mnemonic, QLatin1String("CALL"));
    TN_ADD(t_CALLA,             tm_mnemonic, QLatin1String("CALLA"));
    TN_ADD(t_CALLB,             tm_mnemonic, QLatin1String("CALLB"));
    TN_ADD(t_CALLD,             tm_mnemonic, QLatin1String("CALLD"));
    TN_ADD(t_CALLPA,            tm_mnemonic, QLatin1String("CALLPA"));
    TN_ADD(t_CALLPB,            tm_mnemonic, QLatin1String("CALLPB"));
    TN_ADD(t_CMP,               tm_mnemonic, QLatin1String("CMP"));
    TN_ADD(t_CMPM,              tm_mnemonic, QLatin1String("CMPM"));
    TN_ADD(t_CMPR,              tm_mnemonic, QLatin1String("CMPR"));
    TN_ADD(t_CMPS,              tm_mnemonic, QLatin1String("CMPS"));
    TN_ADD(t_CMPSUB,            tm_mnemonic, QLatin1String("CMPSUB"));
    TN_ADD(t_CMPSX,             tm_mnemonic, QLatin1String("CMPSX"));
    TN_ADD(t_CMPX,              tm_mnemonic, QLatin1String("CMPX"));
    TN_ADD(t_COGATN,            tm_mnemonic, QLatin1String("COGATN"));
    TN_ADD(t_COGBRK,            tm_mnemonic, QLatin1String("COGBRK"));
    TN_ADD(t_COGID,             tm_mnemonic, QLatin1String("COGID"));
    TN_ADD(t_COGINIT,           tm_mnemonic, QLatin1String("COGINIT"));
    TN_ADD(t_COGSTOP,           tm_mnemonic, QLatin1String("COGSTOP"));
    TN_ADD(t_CRCBIT,            tm_mnemonic, QLatin1String("CRCBIT"));
    TN_ADD(t_CRCNIB,            tm_mnemonic, QLatin1String("CRCNIB"));
    TN_ADD(t_DECMOD,            tm_mnemonic, QLatin1String("DECMOD"));
    TN_ADD(t_DECOD,             tm_mnemonic, QLatin1String("DECOD"));
    TN_ADD(t_DIRC,              tm_mnemonic, QLatin1String("DIRC"));
    TN_ADD(t_DIRH,              tm_mnemonic, QLatin1String("DIRH"));
    TN_ADD(t_DIRL,              tm_mnemonic, QLatin1String("DIRL"));
    TN_ADD(t_DIRNC,             tm_mnemonic, QLatin1String("DIRNC"));
    TN_ADD(t_DIRNOT,            tm_mnemonic, QLatin1String("DIRNOT"));
    TN_ADD(t_DIRNZ,             tm_mnemonic, QLatin1String("DIRNZ"));
    TN_ADD(t_DIRRND,            tm_mnemonic, QLatin1String("DIRRND"));
    TN_ADD(t_DIRZ,              tm_mnemonic, QLatin1String("DIRZ"));
    TN_ADD(t_DJF,               tm_mnemonic, QLatin1String("DJF"));
    TN_ADD(t_DJNF,              tm_mnemonic, QLatin1String("DJNF"));
    TN_ADD(t_DJNZ,              tm_mnemonic, QLatin1String("DJNZ"));
    TN_ADD(t_DJZ,               tm_mnemonic, QLatin1String("DJZ"));
    TN_ADD(t_DRVC,              tm_mnemonic, QLatin1String("DRVC"));
    TN_ADD(t_DRVH,              tm_mnemonic, QLatin1String("DRVH"));
    TN_ADD(t_DRVL,              tm_mnemonic, QLatin1String("DRVL"));
    TN_ADD(t_DRVNC,             tm_mnemonic, QLatin1String("DRVNC"));
    TN_ADD(t_DRVNOT,            tm_mnemonic, QLatin1String("DRVNOT"));
    TN_ADD(t_DRVNZ,             tm_mnemonic, QLatin1String("DRVNZ"));
    TN_ADD(t_DRVRND,            tm_mnemonic, QLatin1String("DRVRND"));
    TN_ADD(t_DRVZ,              tm_mnemonic, QLatin1String("DRVZ"));
    TN_ADD(t_ENCOD,             tm_mnemonic, QLatin1String("ENCOD"));
    TN_ADD(t_EXECF,             tm_mnemonic, QLatin1String("EXECF"));
    TN_ADD(t_FBLOCK,            tm_mnemonic, QLatin1String("FBLOCK"));
    TN_ADD(t_FGE,               tm_mnemonic, QLatin1String("FGE"));
    TN_ADD(t_FGES,              tm_mnemonic, QLatin1String("FGES"));
    TN_ADD(t_FLE,               tm_mnemonic, QLatin1String("FLE"));
    TN_ADD(t_FLES,              tm_mnemonic, QLatin1String("FLES"));
    TN_ADD(t_FLTC,              tm_mnemonic, QLatin1String("FLTC"));
    TN_ADD(t_FLTH,              tm_mnemonic, QLatin1String("FLTH"));
    TN_ADD(t_FLTL,              tm_mnemonic, QLatin1String("FLTL"));
    TN_ADD(t_FLTNC,             tm_mnemonic, QLatin1String("FLTNC"));
    TN_ADD(t_FLTNOT,            tm_mnemonic, QLatin1String("FLTNOT"));
    TN_ADD(t_FLTNZ,             tm_mnemonic, QLatin1String("FLTNZ"));
    TN_ADD(t_FLTRND,            tm_mnemonic, QLatin1String("FLTRND"));
    TN_ADD(t_FLTZ,              tm_mnemonic, QLatin1String("FLTZ"));
    TN_ADD(t_GETBRK,            tm_mnemonic, QLatin1String("GETBRK"));
    TN_ADD(t_GETBYTE,           tm_mnemonic, QLatin1String("GETBYTE"));
    TN_ADD(t_GETCT,             tm_mnemonic, QLatin1String("GETCT"));
    TN_ADD(t_GETNIB,            tm_mnemonic, QLatin1String("GETNIB"));
    TN_ADD(t_GETPTR,            tm_mnemonic, QLatin1String("GETPTR"));
    TN_ADD(t_GETQX,             tm_mnemonic, QLatin1String("GETQX"));
    TN_ADD(t_GETQY,             tm_mnemonic, QLatin1String("GETQY"));
    TN_ADD(t_GETRND,            tm_mnemonic, QLatin1String("GETRND"));
    TN_ADD(t_GETSCP,            tm_mnemonic, QLatin1String("GETSCP"));
    TN_ADD(t_GETWORD,           tm_mnemonic, QLatin1String("GETWORD"));
    TN_ADD(t_GETXACC,           tm_mnemonic, QLatin1String("GETXACC"));
    TN_ADD(t_HUBSET,            tm_mnemonic, QLatin1String("HUBSET"));
    TN_ADD(t_IJNZ,              tm_mnemonic, QLatin1String("IJNZ"));
    TN_ADD(t_IJZ,               tm_mnemonic, QLatin1String("IJZ"));
    TN_ADD(t_INCMOD,            tm_mnemonic, QLatin1String("INCMOD"));
    TN_ADD(t_JATN,              tm_mnemonic, QLatin1String("JATN"));
    TN_ADD(t_JCT1,              tm_mnemonic, QLatin1String("JCT1"));
    TN_ADD(t_JCT2,              tm_mnemonic, QLatin1String("JCT2"));
    TN_ADD(t_JCT3,              tm_mnemonic, QLatin1String("JCT3"));
    TN_ADD(t_JFBW,              tm_mnemonic, QLatin1String("JFBW"));
    TN_ADD(t_JINT,              tm_mnemonic, QLatin1String("JINT"));
    TN_ADD(t_JMP,               tm_mnemonic, QLatin1String("JMP"));
    TN_ADD(t_JMPREL,            tm_mnemonic, QLatin1String("JMPREL"));
    TN_ADD(t_JNATN,             tm_mnemonic, QLatin1String("JNATN"));
    TN_ADD(t_JNCT1,             tm_mnemonic, QLatin1String("JNCT1"));
    TN_ADD(t_JNCT2,             tm_mnemonic, QLatin1String("JNCT2"));
    TN_ADD(t_JNCT3,             tm_mnemonic, QLatin1String("JNCT3"));
    TN_ADD(t_JNFBW,             tm_mnemonic, QLatin1String("JNFBW"));
    TN_ADD(t_JNINT,             tm_mnemonic, QLatin1String("JNINT"));
    TN_ADD(t_JNPAT,             tm_mnemonic, QLatin1String("JNPAT"));
    TN_ADD(t_JNQMT,             tm_mnemonic, QLatin1String("JNQMT"));
    TN_ADD(t_JNSE1,             tm_mnemonic, QLatin1String("JNSE1"));
    TN_ADD(t_JNSE2,             tm_mnemonic, QLatin1String("JNSE2"));
    TN_ADD(t_JNSE3,             tm_mnemonic, QLatin1String("JNSE3"));
    TN_ADD(t_JNSE4,             tm_mnemonic, QLatin1String("JNSE4"));
    TN_ADD(t_JNXFI,             tm_mnemonic, QLatin1String("JNXFI"));
    TN_ADD(t_JNXMT,             tm_mnemonic, QLatin1String("JNXMT"));
    TN_ADD(t_JNXRL,             tm_mnemonic, QLatin1String("JNXRL"));
    TN_ADD(t_JNXRO,             tm_mnemonic, QLatin1String("JNXRO"));
    TN_ADD(t_JPAT,              tm_mnemonic, QLatin1String("JPAT"));
    TN_ADD(t_JQMT,              tm_mnemonic, QLatin1String("JQMT"));
    TN_ADD(t_JSE1,              tm_mnemonic, QLatin1String("JSE1"));
    TN_ADD(t_JSE2,              tm_mnemonic, QLatin1String("JSE2"));
    TN_ADD(t_JSE3,              tm_mnemonic, QLatin1String("JSE3"));
    TN_ADD(t_JSE4,              tm_mnemonic, QLatin1String("JSE4"));
    TN_ADD(t_JXFI,              tm_mnemonic, QLatin1String("JXFI"));
    TN_ADD(t_JXMT,              tm_mnemonic, QLatin1String("JXMT"));
    TN_ADD(t_JXRL,              tm_mnemonic, QLatin1String("JXRL"));
    TN_ADD(t_JXRO,              tm_mnemonic, QLatin1String("JXRO"));
    TN_ADD(t_LOC,               tm_mnemonic, QLatin1String("LOC"));
    TN_ADD(t_LOCKNEW,           tm_mnemonic, QLatin1String("LOCKNEW"));
    TN_ADD(t_LOCKREL,           tm_mnemonic, QLatin1String("LOCKREL"));
    TN_ADD(t_LOCKRET,           tm_mnemonic, QLatin1String("LOCKRET"));
    TN_ADD(t_LOCKTRY,           tm_mnemonic, QLatin1String("LOCKTRY"));
    TN_ADD(t_MERGEB,            tm_mnemonic, QLatin1String("MERGEB"));
    TN_ADD(t_MERGEW,            tm_mnemonic, QLatin1String("MERGEW"));
    TN_ADD(t_MIXPIX,            tm_mnemonic, QLatin1String("MIXPIX"));
    TN_ADD(t_MODCZ,             tm_mnemonic, QLatin1String("MODCZ"));
    TN_ADD(t_MODC,              tm_mnemonic, QLatin1String("MODC"));
    TN_ADD(t_MODZ,              tm_mnemonic, QLatin1String("MODZ"));
    TN_ADD(t_MOV,               tm_mnemonic, QLatin1String("MOV"));
    TN_ADD(t_MOVBYTS,           tm_mnemonic, QLatin1String("MOVBYTS"));
    TN_ADD(t_MUL,               tm_mnemonic, QLatin1String("MUL"));
    TN_ADD(t_MULPIX,            tm_mnemonic, QLatin1String("MULPIX"));
    TN_ADD(t_MULS,              tm_mnemonic, QLatin1String("MULS"));
    TN_ADD(t_MUXC,              tm_mnemonic, QLatin1String("MUXC"));
    TN_ADD(t_MUXNC,             tm_mnemonic, QLatin1String("MUXNC"));
    TN_ADD(t_MUXNIBS,           tm_mnemonic, QLatin1String("MUXNIBS"));
    TN_ADD(t_MUXNITS,           tm_mnemonic, QLatin1String("MUXNITS"));
    TN_ADD(t_MUXNZ,             tm_mnemonic, QLatin1String("MUXNZ"));
    TN_ADD(t_MUXQ,              tm_mnemonic, QLatin1String("MUXQ"));
    TN_ADD(t_MUXZ,              tm_mnemonic, QLatin1String("MUXZ"));
    TN_ADD(t_NEG,               tm_mnemonic, QLatin1String("NEG"));
    TN_ADD(t_NEGC,              tm_mnemonic, QLatin1String("NEGC"));
    TN_ADD(t_NEGNC,             tm_mnemonic, QLatin1String("NEGNC"));
    TN_ADD(t_NEGNZ,             tm_mnemonic, QLatin1String("NEGNZ"));
    TN_ADD(t_NEGZ,              tm_mnemonic, QLatin1String("NEGZ"));
    TN_ADD(t_NIXINT1,           tm_mnemonic, QLatin1String("NIXINT1"));
    TN_ADD(t_NIXINT2,           tm_mnemonic, QLatin1String("NIXINT2"));
    TN_ADD(t_NIXINT3,           tm_mnemonic, QLatin1String("NIXINT3"));
    TN_ADD(t_NOP,               tm_mnemonic, QLatin1String("NOP"));
    TN_ADD(t_NOT,               tm_mnemonic, QLatin1String("NOT"));
    TN_ADD(t_ONES,              tm_mnemonic, QLatin1String("ONES"));
    TN_ADD(t_OR,                tm_mnemonic, QLatin1String("OR"));
    TN_ADD(t_OUTC,              tm_mnemonic, QLatin1String("OUTC"));
    TN_ADD(t_OUTH,              tm_mnemonic, QLatin1String("OUTH"));
    TN_ADD(t_OUTL,              tm_mnemonic, QLatin1String("OUTL"));
    TN_ADD(t_OUTNC,             tm_mnemonic, QLatin1String("OUTNC"));
    TN_ADD(t_OUTNOT,            tm_mnemonic, QLatin1String("OUTNOT"));
    TN_ADD(t_OUTNZ,             tm_mnemonic, QLatin1String("OUTNZ"));
    TN_ADD(t_OUTRND,            tm_mnemonic, QLatin1String("OUTRND"));
    TN_ADD(t_OUTZ,              tm_mnemonic, QLatin1String("OUTZ"));
    TN_ADD(t_POLLATN,           tm_mnemonic, QLatin1String("POLLATN"));
    TN_ADD(t_POLLCT1,           tm_mnemonic, QLatin1String("POLLCT1"));
    TN_ADD(t_POLLCT2,           tm_mnemonic, QLatin1String("POLLCT2"));
    TN_ADD(t_POLLCT3,           tm_mnemonic, QLatin1String("POLLCT3"));
    TN_ADD(t_POLLFBW,           tm_mnemonic, QLatin1String("POLLFBW"));
    TN_ADD(t_POLLINT,           tm_mnemonic, QLatin1String("POLLINT"));
    TN_ADD(t_POLLPAT,           tm_mnemonic, QLatin1String("POLLPAT"));
    TN_ADD(t_POLLQMT,           tm_mnemonic, QLatin1String("POLLQMT"));
    TN_ADD(t_POLLSE1,           tm_mnemonic, QLatin1String("POLLSE1"));
    TN_ADD(t_POLLSE2,           tm_mnemonic, QLatin1String("POLLSE2"));
    TN_ADD(t_POLLSE3,           tm_mnemonic, QLatin1String("POLLSE3"));
    TN_ADD(t_POLLSE4,           tm_mnemonic, QLatin1String("POLLSE4"));
    TN_ADD(t_POLLXFI,           tm_mnemonic, QLatin1String("POLLXFI"));
    TN_ADD(t_POLLXMT,           tm_mnemonic, QLatin1String("POLLXMT"));
    TN_ADD(t_POLLXRL,           tm_mnemonic, QLatin1String("POLLXRL"));
    TN_ADD(t_POLLXRO,           tm_mnemonic, QLatin1String("POLLXRO"));
    TN_ADD(t_POP,               tm_mnemonic, QLatin1String("POP"));
    TN_ADD(t_POPA,              tm_mnemonic, QLatin1String("POPA"));
    TN_ADD(t_POPB,              tm_mnemonic, QLatin1String("POPB"));
    TN_ADD(t_PUSH,              tm_mnemonic, QLatin1String("PUSH"));
    TN_ADD(t_PUSHA,             tm_mnemonic, QLatin1String("PUSHA"));
    TN_ADD(t_PUSHB,             tm_mnemonic, QLatin1String("PUSHB"));
    TN_ADD(t_QDIV,              tm_mnemonic, QLatin1String("QDIV"));
    TN_ADD(t_QEXP,              tm_mnemonic, QLatin1String("QEXP"));
    TN_ADD(t_QFRAC,             tm_mnemonic, QLatin1String("QFRAC"));
    TN_ADD(t_QLOG,              tm_mnemonic, QLatin1String("QLOG"));
    TN_ADD(t_QMUL,              tm_mnemonic, QLatin1String("QMUL"));
    TN_ADD(t_QROTATE,           tm_mnemonic, QLatin1String("QROTATE"));
    TN_ADD(t_QSQRT,             tm_mnemonic, QLatin1String("QSQRT"));
    TN_ADD(t_QVECTOR,           tm_mnemonic, QLatin1String("QVECTOR"));
    TN_ADD(t_RCL,               tm_mnemonic, QLatin1String("RCL"));
    TN_ADD(t_RCR,               tm_mnemonic, QLatin1String("RCR"));
    TN_ADD(t_RCZL,              tm_mnemonic, QLatin1String("RCZL"));
    TN_ADD(t_RCZR,              tm_mnemonic, QLatin1String("RCZR"));
    TN_ADD(t_RDBYTE,            tm_mnemonic, QLatin1String("RDBYTE"));
    TN_ADD(t_RDFAST,            tm_mnemonic, QLatin1String("RDFAST"));
    TN_ADD(t_RDLONG,            tm_mnemonic, QLatin1String("RDLONG"));
    TN_ADD(t_RDLUT,             tm_mnemonic, QLatin1String("RDLUT"));
    TN_ADD(t_RDPIN,             tm_mnemonic, QLatin1String("RDPIN"));
    TN_ADD(t_RDWORD,            tm_mnemonic, QLatin1String("RDWORD"));
    TN_ADD(t_REP,               tm_mnemonic, QLatin1String("REP"));
    TN_ADD(t_RESI0,             tm_mnemonic, QLatin1String("RESI0"));
    TN_ADD(t_RESI1,             tm_mnemonic, QLatin1String("RESI1"));
    TN_ADD(t_RESI2,             tm_mnemonic, QLatin1String("RESI2"));
    TN_ADD(t_RESI3,             tm_mnemonic, QLatin1String("RESI3"));
    TN_ADD(t_RET,               tm_mnemonic, QLatin1String("RET"));
    TN_ADD(t_RETA,              tm_mnemonic, QLatin1String("RETA"));
    TN_ADD(t_RETB,              tm_mnemonic, QLatin1String("RETB"));
    TN_ADD(t_RETI0,             tm_mnemonic, QLatin1String("RETI0"));
    TN_ADD(t_RETI1,             tm_mnemonic, QLatin1String("RETI1"));
    TN_ADD(t_RETI2,             tm_mnemonic, QLatin1String("RETI2"));
    TN_ADD(t_RETI3,             tm_mnemonic, QLatin1String("RETI3"));
    TN_ADD(t_REV,               tm_mnemonic, QLatin1String("REV"));
    TN_ADD(t_RFBYTE,            tm_mnemonic, QLatin1String("RFBYTE"));
    TN_ADD(t_RFLONG,            tm_mnemonic, QLatin1String("RFLONG"));
    TN_ADD(t_RFVAR,             tm_mnemonic, QLatin1String("RFVAR"));
    TN_ADD(t_RFVARS,            tm_mnemonic, QLatin1String("RFVARS"));
    TN_ADD(t_RFWORD,            tm_mnemonic, QLatin1String("RFWORD"));
    TN_ADD(t_RGBEXP,            tm_mnemonic, QLatin1String("RGBEXP"));
    TN_ADD(t_RGBSQZ,            tm_mnemonic, QLatin1String("RGBSQZ"));
    TN_ADD(t_ROL,               tm_mnemonic, QLatin1String("ROL"));
    TN_ADD(t_ROLBYTE,           tm_mnemonic, QLatin1String("ROLBYTE"));
    TN_ADD(t_ROLNIB,            tm_mnemonic, QLatin1String("ROLNIB"));
    TN_ADD(t_ROLWORD,           tm_mnemonic, QLatin1String("ROLWORD"));
    TN_ADD(t_ROR,               tm_mnemonic, QLatin1String("ROR"));
    TN_ADD(t_RQPIN,             tm_mnemonic, QLatin1String("RQPIN"));
    TN_ADD(t_SAL,               tm_mnemonic, QLatin1String("SAL"));
    TN_ADD(t_SAR,               tm_mnemonic, QLatin1String("SAR"));
    TN_ADD(t_SCA,               tm_mnemonic, QLatin1String("SCA"));
    TN_ADD(t_SCAS,              tm_mnemonic, QLatin1String("SCAS"));
    TN_ADD(t_SETBYTE,           tm_mnemonic, QLatin1String("SETBYTE"));
    TN_ADD(t_SETCFRQ,           tm_mnemonic, QLatin1String("SETCFRQ"));
    TN_ADD(t_SETCI,             tm_mnemonic, QLatin1String("SETCI"));
    TN_ADD(t_SETCMOD,           tm_mnemonic, QLatin1String("SETCMOD"));
    TN_ADD(t_SETCQ,             tm_mnemonic, QLatin1String("SETCQ"));
    TN_ADD(t_SETCY,             tm_mnemonic, QLatin1String("SETCY"));
    TN_ADD(t_SETD,              tm_mnemonic, QLatin1String("SETD"));
    TN_ADD(t_SETDACS,           tm_mnemonic, QLatin1String("SETDACS"));
    TN_ADD(t_SETINT1,           tm_mnemonic, QLatin1String("SETINT1"));
    TN_ADD(t_SETINT2,           tm_mnemonic, QLatin1String("SETINT2"));
    TN_ADD(t_SETINT3,           tm_mnemonic, QLatin1String("SETINT3"));
    TN_ADD(t_SETLUTS,           tm_mnemonic, QLatin1String("SETLUTS"));
    TN_ADD(t_SETNIB,            tm_mnemonic, QLatin1String("SETNIB"));
    TN_ADD(t_SETPAT,            tm_mnemonic, QLatin1String("SETPAT"));
    TN_ADD(t_SETPIV,            tm_mnemonic, QLatin1String("SETPIV"));
    TN_ADD(t_SETPIX,            tm_mnemonic, QLatin1String("SETPIX"));
    TN_ADD(t_SETQ,              tm_mnemonic, QLatin1String("SETQ"));
    TN_ADD(t_SETQ2,             tm_mnemonic, QLatin1String("SETQ2"));
    TN_ADD(t_SETR,              tm_mnemonic, QLatin1String("SETR"));
    TN_ADD(t_SETS,              tm_mnemonic, QLatin1String("SETS"));
    TN_ADD(t_SETSCP,            tm_mnemonic, QLatin1String("SETSCP"));
    TN_ADD(t_SETSE1,            tm_mnemonic, QLatin1String("SETSE1"));
    TN_ADD(t_SETSE2,            tm_mnemonic, QLatin1String("SETSE2"));
    TN_ADD(t_SETSE3,            tm_mnemonic, QLatin1String("SETSE3"));
    TN_ADD(t_SETSE4,            tm_mnemonic, QLatin1String("SETSE4"));
    TN_ADD(t_SETWORD,           tm_mnemonic, QLatin1String("SETWORD"));
    TN_ADD(t_SETXFRQ,           tm_mnemonic, QLatin1String("SETXFRQ"));
    TN_ADD(t_SEUSSF,            tm_mnemonic, QLatin1String("SEUSSF"));
    TN_ADD(t_SEUSSR,            tm_mnemonic, QLatin1String("SEUSSR"));
    TN_ADD(t_SHL,               tm_mnemonic, QLatin1String("SHL"));
    TN_ADD(t_SHR,               tm_mnemonic, QLatin1String("SHR"));
    TN_ADD(t_SIGNX,             tm_mnemonic, QLatin1String("SIGNX"));
    TN_ADD(t_SKIP,              tm_mnemonic, QLatin1String("SKIP"));
    TN_ADD(t_SKIPF,             tm_mnemonic, QLatin1String("SKIPF"));
    TN_ADD(t_SPLITB,            tm_mnemonic, QLatin1String("SPLITB"));
    TN_ADD(t_SPLITW,            tm_mnemonic, QLatin1String("SPLITW"));
    TN_ADD(t_STALLI,            tm_mnemonic, QLatin1String("STALLI"));
    TN_ADD(t_SUB,               tm_mnemonic, QLatin1String("SUB"));
    TN_ADD(t_SUBR,              tm_mnemonic, QLatin1String("SUBR"));
    TN_ADD(t_SUBS,              tm_mnemonic, QLatin1String("SUBS"));
    TN_ADD(t_SUBSX,             tm_mnemonic, QLatin1String("SUBSX"));
    TN_ADD(t_SUBX,              tm_mnemonic, QLatin1String("SUBX"));
    TN_ADD(t_SUMC,              tm_mnemonic, QLatin1String("SUMC"));
    TN_ADD(t_SUMNC,             tm_mnemonic, QLatin1String("SUMNC"));
    TN_ADD(t_SUMNZ,             tm_mnemonic, QLatin1String("SUMNZ"));
    TN_ADD(t_SUMZ,              tm_mnemonic, QLatin1String("SUMZ"));
    TN_ADD(t_TEST,              tm_mnemonic, QLatin1String("TEST"));
    TN_ADD(t_TESTB,             tm_mnemonic, QLatin1String("TESTB"));
    TN_ADD(t_TESTBN,            tm_mnemonic, QLatin1String("TESTBN"));
    TN_ADD(t_TESTN,             tm_mnemonic, QLatin1String("TESTN"));
    TN_ADD(t_TESTP,             tm_mnemonic, QLatin1String("TESTP"));
    TN_ADD(t_TESTPN,            tm_mnemonic, QLatin1String("TESTPN"));
    TN_ADD(t_TJF,               tm_mnemonic, QLatin1String("TJF"));
    TN_ADD(t_TJNF,              tm_mnemonic, QLatin1String("TJNF"));
    TN_ADD(t_TJNS,              tm_mnemonic, QLatin1String("TJNS"));
    TN_ADD(t_TJNZ,              tm_mnemonic, QLatin1String("TJNZ"));
    TN_ADD(t_TJS,               tm_mnemonic, QLatin1String("TJS"));
    TN_ADD(t_TJV,               tm_mnemonic, QLatin1String("TJV"));
    TN_ADD(t_TJZ,               tm_mnemonic, QLatin1String("TJZ"));
    TN_ADD(t_TRGINT1,           tm_mnemonic, QLatin1String("TRGINT1"));
    TN_ADD(t_TRGINT2,           tm_mnemonic, QLatin1String("TRGINT2"));
    TN_ADD(t_TRGINT3,           tm_mnemonic, QLatin1String("TRGINT3"));
    TN_ADD(t_WAITATN,           tm_mnemonic, QLatin1String("WAITATN"));
    TN_ADD(t_WAITCT1,           tm_mnemonic, QLatin1String("WAITCT1"));
    TN_ADD(t_WAITCT2,           tm_mnemonic, QLatin1String("WAITCT2"));
    TN_ADD(t_WAITCT3,           tm_mnemonic, QLatin1String("WAITCT3"));
    TN_ADD(t_WAITFBW,           tm_mnemonic, QLatin1String("WAITFBW"));
    TN_ADD(t_WAITINT,           tm_mnemonic, QLatin1String("WAITINT"));
    TN_ADD(t_WAITPAT,           tm_mnemonic, QLatin1String("WAITPAT"));
    TN_ADD(t_WAITSE1,           tm_mnemonic, QLatin1String("WAITSE1"));
    TN_ADD(t_WAITSE2,           tm_mnemonic, QLatin1String("WAITSE2"));
    TN_ADD(t_WAITSE3,           tm_mnemonic, QLatin1String("WAITSE3"));
    TN_ADD(t_WAITSE4,           tm_mnemonic, QLatin1String("WAITSE4"));
    TN_ADD(t_WAITX,             tm_mnemonic, QLatin1String("WAITX"));
    TN_ADD(t_WAITXFI,           tm_mnemonic, QLatin1String("WAITXFI"));
    TN_ADD(t_WAITXMT,           tm_mnemonic, QLatin1String("WAITXMT"));
    TN_ADD(t_WAITXRL,           tm_mnemonic, QLatin1String("WAITXRL"));
    TN_ADD(t_WAITXRO,           tm_mnemonic, QLatin1String("WAITXRO"));
    TN_ADD(t_WFBYTE,            tm_mnemonic, QLatin1String("WFBYTE"));
    TN_ADD(t_WFLONG,            tm_mnemonic, QLatin1String("WFLONG"));
    TN_ADD(t_WFWORD,            tm_mnemonic, QLatin1String("WFWORD"));
    TN_ADD(t_WMLONG,            tm_mnemonic, QLatin1String("WMLONG"));
    TN_ADD(t_WRBYTE,            tm_mnemonic, QLatin1String("WRBYTE"));
    TN_ADD(t_WRC,               tm_mnemonic, QLatin1String("WRC"));
    TN_ADD(t_WRFAST,            tm_mnemonic, QLatin1String("WRFAST"));
    TN_ADD(t_WRLONG,            tm_mnemonic, QLatin1String("WRLONG"));
    TN_ADD(t_WRLUT,             tm_mnemonic, QLatin1String("WRLUT"));
    TN_ADD(t_WRNC,              tm_mnemonic, QLatin1String("WRNC"));
    TN_ADD(t_WRNZ,              tm_mnemonic, QLatin1String("WRNZ"));
    TN_ADD(t_WRPIN,             tm_mnemonic, QLatin1String("WRPIN"));
    TN_ADD(t_WRWORD,            tm_mnemonic, QLatin1String("WRWORD"));
    TN_ADD(t_WRZ,               tm_mnemonic, QLatin1String("WRZ"));
    TN_ADD(t_WXPIN,             tm_mnemonic, QLatin1String("WXPIN"));
    TN_ADD(t_WYPIN,             tm_mnemonic, QLatin1String("WYPIN"));
    TN_ADD(t_XCONT,             tm_mnemonic, QLatin1String("XCONT"));
    TN_ADD(t_XINIT,             tm_mnemonic, QLatin1String("XINIT"));
    TN_ADD(t_XOR,               tm_mnemonic, QLatin1String("XOR"));
    TN_ADD(t_XORO32,            tm_mnemonic, QLatin1String("XORO32"));
    TN_ADD(t_XSTOP,             tm_mnemonic, QLatin1String("XSTOP"));
    TN_ADD(t_XZERO,             tm_mnemonic, QLatin1String("XZERO"));
    TN_ADD(t_ZEROX,             tm_mnemonic, QLatin1String("ZEROX"));
    TN_ADD(t_empty,             tm_mnemonic, QLatin1String("<empty>"));

    TN_ADD(t_WC,                tm_wcz_suffix, QLatin1String("WC"));
    TN_ADD(t_WZ,                tm_wcz_suffix, QLatin1String("WZ"));
    TN_ADD(t_WCZ,               tm_wcz_suffix, QLatin1String("WCZ"));
    TN_ADD(t_ANDC,              tm_wcz_suffix, QLatin1String("ANDC"));
    TN_ADD(t_ANDZ,              tm_wcz_suffix, QLatin1String("ANDZ"));
    TN_ADD(t_ORC,               tm_wcz_suffix, QLatin1String("ORC"));
    TN_ADD(t_ORZ,               tm_wcz_suffix, QLatin1String("ORZ"));
    TN_ADD(t_XORC,              tm_wcz_suffix, QLatin1String("XORC"));
    TN_ADD(t_XORZ,              tm_wcz_suffix, QLatin1String("XORZ"));

    // Data
    TN_ADD(t_BYTE,              tm_mnemonic | tm_data, QLatin1String("BYTE"));
    TN_ADD(t_WORD,              tm_mnemonic | tm_data, QLatin1String("WORD"));
    TN_ADD(t_LONG,              tm_mnemonic | tm_data, QLatin1String("LONG"));
    TN_ADD(t_RES,               tm_mnemonic | tm_data, QLatin1String("RES"));
    TN_ADD(t_FILE,              tm_mnemonic | tm_data, QLatin1String("FILE"));

    // Section control
    TN_ADD(t_DAT,               tm_mnemonic | tm_section, QLatin1String("DAT"));
    TN_ADD(t_CON,               tm_mnemonic | tm_section, QLatin1String("CON"));
    TN_ADD(t_PUB,               tm_mnemonic | tm_section, QLatin1String("PUB"));
    TN_ADD(t_PRI,               tm_mnemonic | tm_section, QLatin1String("PRI"));
    TN_ADD(t_VAR,               tm_mnemonic | tm_section, QLatin1String("VAR"));

    // Origin control
    TN_ADD(t_ALIGNW,            tm_mnemonic | tm_origin, QLatin1String("ALIGNW"));
    TN_ADD(t_ALIGNL,            tm_mnemonic | tm_origin, QLatin1String("ALIGNL"));
    TN_ADD(t_ORG,               tm_mnemonic | tm_origin, QLatin1String("ORG"));
    TN_ADD(t_ORGF,              tm_mnemonic | tm_origin, QLatin1String("ORGF"));
    TN_ADD(t_ORGH,              tm_mnemonic | tm_origin, QLatin1String("ORGH"));
    TN_ADD(t_FIT,               tm_mnemonic | tm_origin, QLatin1String("FIT"));

    // Conditionals
    TN_ADD(t__RET_,             tm_conditional, QLatin1String("_RET_"));

    TN_ADD(t_IF_NZ_AND_NC,      tm_conditional, QLatin1String("IF_NZ_AND_NC"));
    TN_ADD(t_IF_NC_AND_NZ,      tm_conditional, QLatin1String("IF_NC_AND_NZ"));
    TN_ADD(t_IF_A,              tm_conditional, QLatin1String("IF_A"));
    TN_ADD(t_IF_GT,             tm_conditional, QLatin1String("IF_GT"));
    TN_ADD(t_IF_00,             tm_conditional, QLatin1String("IF_00"));

    TN_ADD(t_IF_Z_AND_NC,       tm_conditional, QLatin1String("IF_Z_AND_NC"));
    TN_ADD(t_IF_NC_AND_Z,       tm_conditional, QLatin1String("IF_NC_AND_Z"));
    TN_ADD(t_IF_01,             tm_conditional, QLatin1String("IF_01"));

    TN_ADD(t_IF_NC,             tm_conditional, QLatin1String("IF_NC"));
    TN_ADD(t_IF_AE,             tm_conditional, QLatin1String("IF_AE"));
    TN_ADD(t_IF_GE,             tm_conditional, QLatin1String("IF_GE"));
    TN_ADD(t_IF_0X,             tm_conditional, QLatin1String("IF_0X"));

    TN_ADD(t_IF_NZ_AND_C,       tm_conditional, QLatin1String("IF_NZ_AND_C"));
    TN_ADD(t_IF_C_AND_NZ,       tm_conditional, QLatin1String("IF_C_AND_NZ"));
    TN_ADD(t_IF_10,             tm_conditional, QLatin1String("IF_10"));

    TN_ADD(t_IF_NZ,             tm_conditional, QLatin1String("IF_NZ"));
    TN_ADD(t_IF_NE,             tm_conditional, QLatin1String("IF_NE"));
    TN_ADD(t_IF_X0,             tm_conditional, QLatin1String("IF_X0"));

    TN_ADD(t_IF_Z_NE_C,         tm_conditional, QLatin1String("IF_Z_NE_C"));
    TN_ADD(t_IF_C_NE_Z,         tm_conditional, QLatin1String("IF_C_NE_Z"));
    TN_ADD(t_IF_DIFF,           tm_conditional, QLatin1String("IF_DIFF"));

    TN_ADD(t_IF_NZ_OR_NC,       tm_conditional, QLatin1String("IF_NZ_OR_NC"));
    TN_ADD(t_IF_NC_OR_NZ,       tm_conditional, QLatin1String("IF_NC_OR_NZ"));
    TN_ADD(t_IF_NOT_11,         tm_conditional, QLatin1String("IF_NOT_11"));

    TN_ADD(t_IF_Z_AND_C,        tm_conditional, QLatin1String("IF_Z_AND_C"));
    TN_ADD(t_IF_C_AND_Z,        tm_conditional, QLatin1String("IF_C_AND_Z"));
    TN_ADD(t_IF_11,             tm_conditional, QLatin1String("IF_11"));

    TN_ADD(t_IF_Z_EQ_C,         tm_conditional, QLatin1String("IF_Z_EQ_C"));
    TN_ADD(t_IF_C_EQ_Z,         tm_conditional, QLatin1String("IF_C_EQ_Z"));
    TN_ADD(t_IF_SAME,           tm_conditional, QLatin1String("IF_SAME"));

    TN_ADD(t_IF_Z,              tm_conditional, QLatin1String("IF_Z"));
    TN_ADD(t_IF_E,              tm_conditional, QLatin1String("IF_E"));
    TN_ADD(t_IF_X1,             tm_conditional, QLatin1String("IF_X1"));

    TN_ADD(t_IF_Z_OR_NC,        tm_conditional, QLatin1String("IF_Z_OR_NC"));
    TN_ADD(t_IF_NC_OR_Z,        tm_conditional, QLatin1String("IF_NC_OR_Z"));
    TN_ADD(t_IF_NOT_10,         tm_conditional, QLatin1String("IF_NOT_10"));

    TN_ADD(t_IF_C,              tm_conditional, QLatin1String("IF_C"));
    TN_ADD(t_IF_B,              tm_conditional, QLatin1String("IF_B"));
    TN_ADD(t_IF_LT,             tm_conditional, QLatin1String("IF_LT"));
    TN_ADD(t_IF_1X,             tm_conditional, QLatin1String("IF_1X"));

    TN_ADD(t_IF_NZ_OR_C,        tm_conditional, QLatin1String("IF_NZ_OR_C"));
    TN_ADD(t_IF_C_OR_NZ,        tm_conditional, QLatin1String("IF_C_OR_NZ"));
    TN_ADD(t_IF_NOT_01,         tm_conditional, QLatin1String("IF_NOT_01"));

    TN_ADD(t_IF_Z_OR_C,         tm_conditional, QLatin1String("IF_Z_OR_C"));
    TN_ADD(t_IF_C_OR_Z,         tm_conditional, QLatin1String("IF_C_OR_Z"));
    TN_ADD(t_IF_BE,             tm_conditional, QLatin1String("IF_BE"));
    TN_ADD(t_IF_LE,             tm_conditional, QLatin1String("IF_LE"));
    TN_ADD(t_IF_NOT_00,         tm_conditional, QLatin1String("IF_NOT_00"));

    TN_ADD(t_IF_ALWAYS,         tm_conditional, QLatin1String("IF_ALWAYS"));

    // MODCZ parameters
    TN_ADD(t_MODCZ__CLR,        tm_modcz_param, QLatin1String("_CLR"));

    TN_ADD(t_MODCZ__NC_AND_NZ,  tm_modcz_param, QLatin1String("_NC_AND_NZ"));
    TN_ADD(t_MODCZ__NZ_AND_NC,  tm_modcz_param, QLatin1String("_NZ_AND_NC"));
    TN_ADD(t_MODCZ__A,          tm_modcz_param, QLatin1String("_A"));
    TN_ADD(t_MODCZ__GT,         tm_modcz_param, QLatin1String("_GT"));
    TN_ADD(t_MODCZ__00,         tm_modcz_param, QLatin1String("_00"));

    TN_ADD(t_MODCZ__NC_AND_Z,   tm_modcz_param, QLatin1String("_NC_AND_Z"));
    TN_ADD(t_MODCZ__Z_AND_NC,   tm_modcz_param, QLatin1String("_Z_AND_NC"));
    TN_ADD(t_MODCZ__10,         tm_modcz_param, QLatin1String("_10"));

    TN_ADD(t_MODCZ__NC,         tm_modcz_param, QLatin1String("_NC"));
    TN_ADD(t_MODCZ__AE,         tm_modcz_param, QLatin1String("_AE"));
    TN_ADD(t_MODCZ__GE,         tm_modcz_param, QLatin1String("_GE"));
    TN_ADD(t_MODCZ__0X,         tm_modcz_param, QLatin1String("_0X"));

    TN_ADD(t_MODCZ__C_AND_NZ,   tm_modcz_param, QLatin1String("_C_AND_NZ"));
    TN_ADD(t_MODCZ__NZ_AND_C,   tm_modcz_param, QLatin1String("_NZ_AND_C"));
    TN_ADD(t_MODCZ__10,         tm_modcz_param, QLatin1String("_10"));

    TN_ADD(t_MODCZ__NZ,         tm_modcz_param, QLatin1String("_NZ"));
    TN_ADD(t_MODCZ__NE,         tm_modcz_param, QLatin1String("_NE"));
    TN_ADD(t_MODCZ__X0,         tm_modcz_param, QLatin1String("_X0"));

    TN_ADD(t_MODCZ__C_NE_Z,     tm_modcz_param, QLatin1String("_C_NE_Z"));
    TN_ADD(t_MODCZ__Z_NE_C,     tm_modcz_param, QLatin1String("_Z_NE_C"));
    TN_ADD(t_MODCZ__DIFF,       tm_modcz_param, QLatin1String("_DIFF"));

    TN_ADD(t_MODCZ__NC_OR_NZ,   tm_modcz_param, QLatin1String("_NC_OR_NZ"));
    TN_ADD(t_MODCZ__NZ_OR_NC,   tm_modcz_param, QLatin1String("_NZ_OR_NC"));
    TN_ADD(t_MODCZ__NOT_11,     tm_modcz_param, QLatin1String("_NZ_OR_NC"));

    TN_ADD(t_MODCZ__C_AND_Z,    tm_modcz_param, QLatin1String("_C_AND_Z"));
    TN_ADD(t_MODCZ__Z_AND_C,    tm_modcz_param, QLatin1String("_Z_AND_C"));
    TN_ADD(t_MODCZ__11,         tm_modcz_param, QLatin1String("_11"));

    TN_ADD(t_MODCZ__C_EQ_Z,     tm_modcz_param, QLatin1String("_C_EQ_Z"));
    TN_ADD(t_MODCZ__Z_EQ_C,     tm_modcz_param, QLatin1String("_Z_EQ_C"));
    TN_ADD(t_MODCZ__SAME,       tm_modcz_param, QLatin1String("_SAME"));

    TN_ADD(t_MODCZ__Z,          tm_modcz_param, QLatin1String("_Z"));
    TN_ADD(t_MODCZ__E,          tm_modcz_param, QLatin1String("_E"));
    TN_ADD(t_MODCZ__X1,         tm_modcz_param, QLatin1String("_X1"));

    TN_ADD(t_MODCZ__NC_OR_Z,    tm_modcz_param, QLatin1String("_NC_OR_Z"));
    TN_ADD(t_MODCZ__Z_OR_NC,    tm_modcz_param, QLatin1String("_Z_OR_NC"));
    TN_ADD(t_MODCZ__NOT_10,     tm_modcz_param, QLatin1String("_NOT_10"));

    TN_ADD(t_MODCZ__C,          tm_modcz_param, QLatin1String("_C"));
    TN_ADD(t_MODCZ__B,          tm_modcz_param, QLatin1String("_B"));
    TN_ADD(t_MODCZ__LT,         tm_modcz_param, QLatin1String("_LT"));
    TN_ADD(t_MODCZ__1X,         tm_modcz_param, QLatin1String("_1X"));

    TN_ADD(t_MODCZ__C_OR_NZ,    tm_modcz_param, QLatin1String("_C_OR_NZ"));
    TN_ADD(t_MODCZ__NZ_OR_C,    tm_modcz_param, QLatin1String("_NZ_OR_C"));
    TN_ADD(t_MODCZ__NOT_10,     tm_modcz_param, QLatin1String("_NOT_10"));

    TN_ADD(t_MODCZ__C_OR_Z,     tm_modcz_param, QLatin1String("_C_OR_Z"));
    TN_ADD(t_MODCZ__Z_OR_C,     tm_modcz_param, QLatin1String("_Z_OR_C"));
    TN_ADD(t_MODCZ__LE,         tm_modcz_param, QLatin1String("_LE"));
    TN_ADD(t_MODCZ__NOT_00,     tm_modcz_param, QLatin1String("_NOT_00"));

    TN_ADD(t_MODCZ__SET,        tm_modcz_param, QLatin1String("_SET"));

    // Assignment
    TN_ADD(t_ASSIGN,            tm_mnemonic | tm_assignment, QLatin1String("="));
    TN_ADD(t_COMMA,             tm_delimiter, QLatin1String(","));

    // LUT shadow register constants
    TN_ADD(t_PA,                tm_constant, QLatin1String("PA"));
    TN_ADD(t_PB,                tm_constant, QLatin1String("PB"));
    TN_ADD(t_PTRA,              tm_constant, QLatin1String("PTRA"));
    TN_ADD(t_PTRA_postinc,      tm_constant, QLatin1String("PTRA++"));
    TN_ADD(t_PTRA_postdec,      tm_constant, QLatin1String("PTRA--"));
    TN_ADD(t_PTRA_preinc,       tm_constant, QLatin1String("++PTRA"));
    TN_ADD(t_PTRA_predec,       tm_constant, QLatin1String("--PTRA"));
    TN_ADD(t_PTRB,              tm_constant, QLatin1String("PTRB"));
    TN_ADD(t_PTRB_postinc,      tm_constant, QLatin1String("PTRB++"));
    TN_ADD(t_PTRB_postdec,      tm_constant, QLatin1String("PTRB--"));
    TN_ADD(t_PTRB_preinc,       tm_constant, QLatin1String("++PTRB"));
    TN_ADD(t_PTRB_predec,       tm_constant, QLatin1String("--PTRB"));
    TN_ADD(t_DIRA,              tm_constant, QLatin1String("DIRA"));
    TN_ADD(t_DIRB,              tm_constant, QLatin1String("DIRB"));
    TN_ADD(t_OUTA,              tm_constant, QLatin1String("OUTA"));
    TN_ADD(t_OUTB,              tm_constant, QLatin1String("OUTB"));
    TN_ADD(t_INA,               tm_constant, QLatin1String("INA"));
    TN_ADD(t_INB,               tm_constant, QLatin1String("INB"));

    // Conversion to different type
    TN_ADD(t__FLOAT,            tm_function, QLatin1String("FLOAT"));
    TN_ADD(t__ROUND,            tm_function, QLatin1String("ROUND"));
    TN_ADD(t__TRUNC,            tm_function, QLatin1String("TRUNC"));

    // Current PC reference
    TN_ADD(t_DOLLAR,            tm_constant, QLatin1String("$"));

    // Traits
    TN_ADD(t_IMMEDIATE,         tm_traits, QLatin1String("#"));
    TN_ADD(t_AUGMENTED,         tm_traits, QLatin1String("##"));
    TN_ADD(t_RELATIVE,          tm_traits, QLatin1String("@"));
    TN_ADD(t_ABSOLUTE,          tm_traits, QLatin1String("\\"));
    TN_ADD(t_ADDRESS_HUB,       tm_traits, QLatin1String("#@"));
    TN_ADD(t_RELATIVE_HUB,      tm_traits, QLatin1String("##@"));

    // Sub expression in parens
    TN_ADD(t__LPAREN,           tm_parens, QLatin1String("("));
    TN_ADD(t__RPAREN,           tm_parens, QLatin1String(")"));

    // Index expression in brackets
    TN_ADD(t__LBRACKET,         tm_brackets, QLatin1String("["));
    TN_ADD(t__RBRACKET,         tm_brackets, QLatin1String("]"));

    // Set the primary operators
    TN_ADD(t__INC,              tm_primary, QLatin1String("++"));
    TN_ADD(t__DEC,              tm_primary, QLatin1String("--"));

    // Set the unary operators
    TN_ADD(t__NEG,              tm_unary, QLatin1String("!"));
    TN_ADD(t__NOT,              tm_unary, QLatin1String("~"));

    // Set the multiplication operators
    TN_ADD(t__MUL,              tm_mulop, QLatin1String("*"));
    TN_ADD(t__DIV,              tm_mulop, QLatin1String("/"));
    TN_ADD(t__MOD,              tm_mulop, QLatin1String("%"));

    // Set the addition operators
    TN_ADD(t__PLUS,             tm_addop | tm_unary, QLatin1String("+"));
    TN_ADD(t__MINUS,            tm_addop | tm_unary, QLatin1String("-"));

    // Set the shift operators
    TN_ADD(t__SHL,              tm_shiftop, QLatin1String("<<"));
    TN_ADD(t__SHR,              tm_shiftop, QLatin1String(">>"));

    // Set the less/greater comparison operators
    TN_ADD(t__GE,               tm_relation, QLatin1String(">="));
    TN_ADD(t__GT,               tm_relation, QLatin1String(">"));
    TN_ADD(t__LE,               tm_relation, QLatin1String("<="));
    TN_ADD(t__LT,               tm_relation, QLatin1String("<"));

    // Set the equal/unequal comparison operators
    TN_ADD(t__EQ,               tm_equality, QLatin1String("=="));
    TN_ADD(t__NE,               tm_equality, QLatin1String("!="));

    // Set the binary operators
    TN_ADD(t__AND,              tm_binop_and, QLatin1String("&"));
    TN_ADD(t__XOR,              tm_binop_xor, QLatin1String("^"));
    TN_ADD(t__OR,               tm_binop_or,  QLatin1String("|"));
    TN_ADD(t__REV,              tm_binop_rev, QLatin1String("><"));

    // Encode / Decode
    TN_ADD(t__ENCOD,            tm_binop_encod, QLatin1String("|<"));
    TN_ADD(t__DECOD,            tm_binop_decod, QLatin1String(">|"));

    // Set the logical operators
    TN_ADD(t__LOGAND,           tm_logop_and, QLatin1String("&&"));
    TN_ADD(t__LOGOR,            tm_logop_or, QLatin1String("||"));

    // Set the conditionals lookup table
    m_lookup_cond.insert(t__RET_,               cc__ret_);
    m_lookup_cond.insert(t_IF_NC_AND_NZ,        cc_nc_and_nz);
    m_lookup_cond.insert(t_IF_NZ_AND_NC,        cc_nc_and_nz);
    m_lookup_cond.insert(t_IF_A,                cc_nc_and_nz);
    m_lookup_cond.insert(t_IF_GT,               cc_nc_and_nz);
    m_lookup_cond.insert(t_IF_00,               cc_nc_and_nz);

    m_lookup_cond.insert(t_IF_NC_AND_Z,         cc_nc_and_z);
    m_lookup_cond.insert(t_IF_Z_AND_NC,         cc_nc_and_z);
    m_lookup_cond.insert(t_IF_01,               cc_nc_and_z);

    m_lookup_cond.insert(t_IF_NC,               cc_nc);
    m_lookup_cond.insert(t_IF_AE,               cc_nc);
    m_lookup_cond.insert(t_IF_GE,               cc_nc);
    m_lookup_cond.insert(t_IF_0X,               cc_nc);

    m_lookup_cond.insert(t_IF_C_AND_NZ,         cc_c_and_nz);
    m_lookup_cond.insert(t_IF_NZ_AND_C,         cc_c_and_nz);
    m_lookup_cond.insert(t_IF_10,               cc_c_and_nz);

    m_lookup_cond.insert(t_IF_NZ,               cc_nz);
    m_lookup_cond.insert(t_IF_NE,               cc_nz);
    m_lookup_cond.insert(t_IF_X0,               cc_nz);

    m_lookup_cond.insert(t_IF_C_NE_Z,           cc_c_ne_z);
    m_lookup_cond.insert(t_IF_Z_NE_C,           cc_c_ne_z);
    m_lookup_cond.insert(t_IF_DIFF,             cc_c_ne_z);

    m_lookup_cond.insert(t_IF_NC_OR_NZ,         cc_nc_or_nz);
    m_lookup_cond.insert(t_IF_NZ_OR_NC,         cc_nc_or_nz);
    m_lookup_cond.insert(t_IF_NOT_11,           cc_nc_or_nz);

    m_lookup_cond.insert(t_IF_C_AND_Z,          cc_c_and_z);
    m_lookup_cond.insert(t_IF_Z_AND_C,          cc_c_and_z);
    m_lookup_cond.insert(t_IF_11,               cc_c_and_z);

    m_lookup_cond.insert(t_IF_C_EQ_Z,           cc_c_eq_z);
    m_lookup_cond.insert(t_IF_Z_EQ_C,           cc_c_eq_z);
    m_lookup_cond.insert(t_IF_SAME,             cc_c_eq_z);

    m_lookup_cond.insert(t_IF_Z,                cc_z);
    m_lookup_cond.insert(t_IF_E,                cc_z);
    m_lookup_cond.insert(t_IF_X1,               cc_z);

    m_lookup_cond.insert(t_IF_NC_OR_Z,          cc_nc_or_z);
    m_lookup_cond.insert(t_IF_Z_OR_NC,          cc_nc_or_z);
    m_lookup_cond.insert(t_IF_BE,               cc_nc_or_z);
    m_lookup_cond.insert(t_IF_X1,               cc_nc_or_z);

    m_lookup_cond.insert(t_IF_C,                cc_c);
    m_lookup_cond.insert(t_IF_B,                cc_c);
    m_lookup_cond.insert(t_IF_LT,               cc_c);
    m_lookup_cond.insert(t_IF_1X,               cc_c);

    m_lookup_cond.insert(t_IF_C_OR_NZ,          cc_c_or_nz);
    m_lookup_cond.insert(t_IF_NZ_OR_C,          cc_c_or_nz);
    m_lookup_cond.insert(t_IF_NOT_01,           cc_c_or_nz);

    m_lookup_cond.insert(t_IF_C_OR_Z,           cc_c_or_z);
    m_lookup_cond.insert(t_IF_Z_OR_C,           cc_c_or_z);
    m_lookup_cond.insert(t_IF_LE,               cc_c_or_z);
    m_lookup_cond.insert(t_IF_NOT_00,           cc_c_or_z);

    m_lookup_cond.insert(t_IF_ALWAYS,           cc_always);

    // Set the MODCZ lookup table
    m_lookup_modcz.insert(t_MODCZ__CLR,         cc_clr);

    m_lookup_modcz.insert(t_MODCZ__NC_AND_NZ,   cc_nc_and_nz);
    m_lookup_modcz.insert(t_MODCZ__NZ_AND_NC,   cc_nc_and_nz);
    m_lookup_modcz.insert(t_MODCZ__A,           cc_nc_and_nz);
    m_lookup_modcz.insert(t_MODCZ__GT,          cc_nc_and_nz);
    m_lookup_modcz.insert(t_MODCZ__00,          cc_nc_and_nz);

    m_lookup_modcz.insert(t_MODCZ__NC_AND_Z,    cc_nc_and_z);
    m_lookup_modcz.insert(t_MODCZ__Z_AND_NC,    cc_nc_and_z);
    m_lookup_modcz.insert(t_MODCZ__01,          cc_nc_and_z);

    m_lookup_modcz.insert(t_MODCZ__NC,          cc_nc);
    m_lookup_modcz.insert(t_MODCZ__AE,          cc_nc);
    m_lookup_modcz.insert(t_MODCZ__GE,          cc_nc);
    m_lookup_modcz.insert(t_MODCZ__0X,          cc_nc);

    m_lookup_modcz.insert(t_MODCZ__C_AND_NZ,    cc_c_and_nz);
    m_lookup_modcz.insert(t_MODCZ__NZ_AND_C,    cc_c_and_nz);
    m_lookup_modcz.insert(t_MODCZ__10,          cc_c_and_nz);

    m_lookup_modcz.insert(t_MODCZ__NZ,          cc_nz);
    m_lookup_modcz.insert(t_MODCZ__NE,          cc_nz);
    m_lookup_modcz.insert(t_MODCZ__X0,          cc_nz);

    m_lookup_modcz.insert(t_MODCZ__C_NE_Z,      cc_c_ne_z);
    m_lookup_modcz.insert(t_MODCZ__Z_NE_C,      cc_c_ne_z);
    m_lookup_modcz.insert(t_MODCZ__DIFF,        cc_c_ne_z);

    m_lookup_modcz.insert(t_MODCZ__NC_OR_NZ,    cc_nc_or_nz);
    m_lookup_modcz.insert(t_MODCZ__NZ_OR_NC,    cc_nc_or_nz);
    m_lookup_modcz.insert(t_MODCZ__NOT_11,      cc_nc_or_nz);

    m_lookup_modcz.insert(t_MODCZ__C_AND_Z,     cc_c_and_z);
    m_lookup_modcz.insert(t_MODCZ__Z_AND_C,     cc_c_and_z);
    m_lookup_modcz.insert(t_MODCZ__11,          cc_c_and_z);

    m_lookup_modcz.insert(t_MODCZ__C_EQ_Z,      cc_c_eq_z);
    m_lookup_modcz.insert(t_MODCZ__Z_EQ_C,      cc_c_eq_z);
    m_lookup_modcz.insert(t_MODCZ__SAME,        cc_c_eq_z);

    m_lookup_modcz.insert(t_MODCZ__Z,           cc_z);
    m_lookup_modcz.insert(t_MODCZ__E,           cc_z);
    m_lookup_modcz.insert(t_MODCZ__X1,          cc_z);

    m_lookup_modcz.insert(t_MODCZ__NC_OR_Z,     cc_nc_or_z);
    m_lookup_modcz.insert(t_MODCZ__Z_OR_NC,     cc_nc_or_z);
    m_lookup_modcz.insert(t_MODCZ__BE,          cc_nc_or_z);
    m_lookup_modcz.insert(t_MODCZ__X1,          cc_nc_or_z);

    m_lookup_modcz.insert(t_MODCZ__C,           cc_c);
    m_lookup_modcz.insert(t_MODCZ__B,           cc_c);
    m_lookup_modcz.insert(t_MODCZ__LT,          cc_c);
    m_lookup_modcz.insert(t_MODCZ__1X,          cc_c);

    m_lookup_modcz.insert(t_MODCZ__NZ_OR_C,     cc_c_or_nz);
    m_lookup_modcz.insert(t_MODCZ__C_OR_NZ,     cc_c_or_nz);
    m_lookup_modcz.insert(t_MODCZ__NOT_01,      cc_c_or_nz);

    m_lookup_modcz.insert(t_MODCZ__Z_OR_C,      cc_c_or_z);
    m_lookup_modcz.insert(t_MODCZ__C_OR_Z,      cc_c_or_z);
    m_lookup_modcz.insert(t_MODCZ__LE,          cc_c_or_z);
    m_lookup_modcz.insert(t_MODCZ__NOT_00,      cc_c_or_z);

    m_lookup_modcz.insert(t_MODCZ__SET,         cc_always);

    // Build the reverse QHash for string lookup
    foreach(p2_TOKEN_e tok, m_token_string.keys())
        foreach(const QString& str, m_token_string.values(tok))
            m_string_token.insert(str, tok);

    // Build the reverse QMultiHash for types lookup
    foreach(p2_TOKEN_e tok, m_token_type.keys())
        foreach(p2_TOKENMASK_t mask, m_token_type.values(tok))
            m_type_token.insertMulti(mask, tok);

    m_t_type_name.insert(tt_none,           QStringLiteral("-"));
    m_t_type_name.insert(tt_parens,         QStringLiteral("Parens"));
    m_t_type_name.insert(tt_brackets,       QStringLiteral("Brackets"));
    m_t_type_name.insert(tt_primary,        QStringLiteral("Primary"));
    m_t_type_name.insert(tt_unary,          QStringLiteral("Unary"));
    m_t_type_name.insert(tt_mulop,          QStringLiteral("MulOp"));
    m_t_type_name.insert(tt_addop,          QStringLiteral("AddOp"));
    m_t_type_name.insert(tt_shiftop,        QStringLiteral("ShiftOp"));
    m_t_type_name.insert(tt_relation,       QStringLiteral("Relation"));
    m_t_type_name.insert(tt_equality,       QStringLiteral("Equality"));
    m_t_type_name.insert(tt_binop_and,      QStringLiteral("Binary AND"));
    m_t_type_name.insert(tt_binop_xor,      QStringLiteral("Binary XOR"));
    m_t_type_name.insert(tt_binop_or,       QStringLiteral("Binary OR"));
    m_t_type_name.insert(tt_binop_rev,      QStringLiteral("Binary REV"));
    m_t_type_name.insert(tt_logop_and,      QStringLiteral("Logic AND"));
    m_t_type_name.insert(tt_logop_or,       QStringLiteral("Logic OR"));
    m_t_type_name.insert(tt_ternary,        QStringLiteral("Ternary"));
    m_t_type_name.insert(tt_assignment,     QStringLiteral("Assignment"));
    m_t_type_name.insert(tt_delimiter,      QStringLiteral("Delimiter"));
    m_t_type_name.insert(tt_constant,       QStringLiteral("Constant"));
    m_t_type_name.insert(tt_function,       QStringLiteral("Function"));
    m_t_type_name.insert(tt_traits,         QStringLiteral("Traits"));
    m_t_type_name.insert(tt_conditional,    QStringLiteral("Conditional"));
    m_t_type_name.insert(tt_modcz_param,    QStringLiteral("MODCZ param"));
    m_t_type_name.insert(tt_mnemonic,       QStringLiteral("Instruction"));
    m_t_type_name.insert(tt_wcz_suffix,     QStringLiteral("WC/WZ suffix"));
    m_t_type_name.insert(tt_section,        QStringLiteral("Section"));
    m_t_type_name.insert(tt_origin,         QStringLiteral("Origin"));
    m_t_type_name.insert(tt_data,           QStringLiteral("Data"));
    m_t_type_name.insert(tt_regexp,         QStringLiteral("RegExp"));
}

/**
 * @brief Return a QString for the given token %tok
 * @param tok one of p2_toke_e enum
 * @return QString in uppercase or lowercase
 */
QString P2Token::string(p2_TOKEN_e tok, bool lowercase) const
{
    QString str = m_token_string.value(tok);
    return lowercase ? str.toLower() : str;
}

/**
 * @brief Return a QString for the given token %tok's enumeration name
 * @param tok one of p2_toke_e enum
 */
QString P2Token::enum_name(p2_TOKEN_e tok) const
{
    return m_token_enum_name.value(tok);
}

/**
 * @brief Count number of curly braces, left and right, return the sum
 * @param ref QStringRef to a portion of a string to scan
 * @return number of lcurly - number of rcruly braces
 */
static int count_curly(const QStringRef& ref)
{
    int curly = 0;
    curly += ref.count(chr_lcurly);
    curly -= ref.count(chr_rcurly);
    return curly;
}

/**
 * @brief Tokenize a string and append to P2AsmWords
 * @param line string to tokenize
 * @param lineno line number
 * @param in_curly reference to an integer with the current curly braces level
 * @return A QVector of P2Words for the line
 */
P2Words P2Token::tokenize(const QString* line, const int lineno, int& in_curly) const
{
    static QRegExp rx;
    static QString regex;
    P2Words words;
    p2_TOKEN_e tok;
    const int len = line->length();
    int pos = 0;
    int lastpos = 0;

#if (DBG_REGEX || DBG_TOKEN)
    qDebug("%s: ····························· tokenize line ·································", __func__);
    qDebug("%-4d: %s%s%s", lineno, qPrintable(chr_ldangle), qPrintable(line), qPrintable(chr_rdangle));
#endif

    if (rx.isEmpty()) {
        // first time initialization of the QRegExp
        QStringList list;

        list += QStringLiteral("^\\s+");                                    // spaces
        list += re_comment_curly;                                           // comments in {curly braces}
        list += re_comment_eol;                                             // Comments 'something...
        list += re_locsym;                                                  // local symbols
        list += re_symbol;                                                  // symbols
        list += re_bin_const;                                               // bin constants
        list += re_byt_const;                                               // byt constants
        list += re_real_const;                                              // real constants
        list += re_dec_const;                                               // dec constants
        list += re_hex_const;                                               // hex constants
        list += re_str_const;                                               // str constants
        list += QRegExp::escape(m_token_string.value(t_PTRA));              // PTRA
        list += QRegExp::escape(m_token_string.value(t_PTRA_postinc));      // PTRA++
        list += QRegExp::escape(m_token_string.value(t_PTRA_postdec));      // PTRA--
        list += QRegExp::escape(m_token_string.value(t_PTRA_preinc));       // ++PTRA
        list += QRegExp::escape(m_token_string.value(t_PTRA_predec));       // --PTRA
        list += QRegExp::escape(m_token_string.value(t_PTRB));              // PTRB
        list += QRegExp::escape(m_token_string.value(t_PTRB_postinc));      // PTRB++
        list += QRegExp::escape(m_token_string.value(t_PTRB_postdec));      // PTRB--
        list += QRegExp::escape(m_token_string.value(t_PTRB_preinc));       // ++PTRB
        list += QRegExp::escape(m_token_string.value(t_PTRB_predec));       // --PTRB
        list += QRegExp::escape(m_token_string.value(t_IMMEDIATE));         // immediate (#)
        list += QRegExp::escape(m_token_string.value(t_AUGMENTED));         // augmented (##)
        list += QRegExp::escape(m_token_string.value(t_RELATIVE));          // relative (@)
        list += QRegExp::escape(m_token_string.value(t_ABSOLUTE));          // absolute (\)
        list += QRegExp::escape(m_token_string.value(t_ADDRESS_HUB));       // address HUB (#@)
        list += QRegExp::escape(m_token_string.value(t_RELATIVE_HUB));      // relative HUB (@@@)
        list += QRegExp::escape(m_token_string.value(t_DOLLAR));            // dollar ($)
        list += QRegExp::escape(m_token_string.value(t_COMMA));             // comma (,)
        list += QRegExp::escape(m_token_string.value(t__LBRACKET));         // left bracket ([) (index expression start)
        list += QRegExp::escape(m_token_string.value(t__RBRACKET));         // right bracket (]) (index expression end)
        list += QRegExp::escape(m_token_string.value(t__INC));              // increment (++)
        list += QRegExp::escape(m_token_string.value(t__PLUS));             // add / unary plus (+)
        list += QRegExp::escape(m_token_string.value(t__DEC));              // decrement (--)
        list += QRegExp::escape(m_token_string.value(t__MINUS));            // subtract / unary minus (-)
        list += QRegExp::escape(m_token_string.value(t__SHL));              // shift left (<<)
        list += QRegExp::escape(m_token_string.value(t__LT));               // less than (<)
        list += QRegExp::escape(m_token_string.value(t__SHR));              // shift right (>>)
        list += QRegExp::escape(m_token_string.value(t__REV));              // reverse (><)
        list += QRegExp::escape(m_token_string.value(t__DECOD));            // binary decode (>|)
        list += QRegExp::escape(m_token_string.value(t__ENCOD));            // binary encode (|<)
        list += QRegExp::escape(m_token_string.value(t__GT));               // greater than (>)
        list += QRegExp::escape(m_token_string.value(t__MUL));              // multiply (*)
        list += QRegExp::escape(m_token_string.value(t__DIV));              // divide (/)
        list += QRegExp::escape(m_token_string.value(t__MOD));              // modulo (%)
        list += QRegExp::escape(m_token_string.value(t__AND));              // binary AND (&)
        list += QRegExp::escape(m_token_string.value(t__OR));               // binary OR (|)
        list += QRegExp::escape(m_token_string.value(t__LPAREN));           // left parenthesis "(" (sub expression start)
        list += QRegExp::escape(m_token_string.value(t__RPAREN));           // right parenthesis ")" (sub expression end)
        list += QRegExp::escape(m_token_string.value(t__EQ));               // equals (==)
        list += QRegExp::escape(m_token_string.value(t_ASSIGN));            // assignment (=)
        list += QRegExp::escape(m_token_string.value(t_comment_lcurly));    // left curly brace ({) (comment start)
        list += QRegExp::escape(m_token_string.value(t_comment_rcurly));    // right curly brace (}) (comment end)

        rx.setPatternSyntax(QRegExp::RegExp2);  // greedy syntax
        regex = QString("(%1)").arg(list.join(QChar('|')));
        rx.setPattern(regex);
        rx.setCaseSensitivity(Qt::CaseInsensitive);
        Q_ASSERT(rx.isValid());
    }

    while (pos < line->length() && line->at(pos).isSpace())
        ++pos;

    while (-1 != (pos = rx.indexIn(*line, lastpos, QRegExp::CaretAtOffset))) {
        int tlen = rx.matchedLength();
        QStringRef ref(line, pos, tlen);

        if (tlen > 0 && ref.trimmed().isEmpty()) {
            DEBUG_REGEX("  match %d @%-3d #%-3d SPACE(S)", in_curly, pos, tlen);
            while (pos < line->length() && line->at(pos).isSpace())
                ++pos;
            lastpos = pos;
            if (pos >= len) {
                DEBUG_REGEX("  match %d @%-3d #%-3d end-of-line", in_curly, pos, tlen);
                break;
            }
            continue;
        }

        if (pos >= len) {
            DEBUG_REGEX("  match %d @%-3d #%-3d end-of-line", in_curly, pos, tlen);
            break;
        }

        if (ref[0] != chr_dquote && ref[0] != chr_apostrophe) {
            int curly = count_curly(ref);
            if (curly) {
                in_curly = qMax(in_curly + curly, 0);
                const int tlen = len - pos;
                tok = curly >= 0 ? t_comment_lcurly : t_comment_rcurly;
                QStringRef ref(line, pos, tlen);
                words.append(P2Word(tok, ref, lineno));
                pos += tlen;
                DEBUG_REGEX("  match %d @%-3d #%-3d %s%s%s",
                            in_curly, pos, tlen,
                            qPrintable(chr_ldangle), qPrintable(ref.toString()), qPrintable(chr_rdangle));
                goto leave;
            }
        }

        if (ref[0] == chr_apostrophe) {
            const int tlen = len - pos;
            tok = t_comment_eol;
            QStringRef ref(line, pos, tlen);
            words.append(P2Word(tok, ref, lineno));
            DEBUG_REGEX("  comnt %d @%-3d #%-3d %s%s%s",
                        in_curly, pos, tlen,
                        qPrintable(chr_ldangle), qPrintable(ref.toString()), qPrintable(chr_rdangle));
            goto leave;
        }

        if (in_curly > 0) {
            const int tlen = len - pos;
            QStringRef ref(line, pos, tlen);
            int curly = count_curly(ref);
            in_curly = qMax(in_curly + curly, 0);

            tok = in_curly > 0 ? t_comment_lcurly : t_comment_rcurly;
            words.append(P2Word(tok, ref, lineno));
            DEBUG_REGEX("  curly %d @%-3d #%-3d %s%s%s",
                        in_curly, pos, tlen,
                        qPrintable(chr_ldangle), qPrintable(ref.toString()), qPrintable(chr_rdangle));
            goto leave;
        }

        DEBUG_REGEX("  match %d @%-3d #%-3d %s%s%s",
                    in_curly, pos, tlen,
                    qPrintable(chr_ldangle), qPrintable(line.mid(pos, tlen)), qPrintable(chr_rdangle));

        tok = token(*line, pos, tlen);
        ref = QStringRef(line, pos, tlen);
        words.append(P2Word(tok, ref, lineno));
        if (tok != t_str_const) {
            int curly = count_curly(ref);
            in_curly = qMax(in_curly + curly, 0);
        }
        pos += tlen;
        lastpos = pos;
    }

leave:

#if DBG_TOKEN
    int i = 0;
    if (words.isEmpty()) {

        DEBUG_REGEX("%s: empty %d @%-3d #%-3d %s%s%s", __func__,
                    in_curly, 0, line.length(),
                    qPrintable(chr_ldangle), qPrintable(line), qPrintable(chr_rdangle));
        return words;

    } else foreach(const P2Word& word, words) {
        const p2_token_e tok = word.tok();
        const QString& name = enum_name(tok);
        const QString& str = word.str();
        const int pos = word.pos();
        const int len = word.len();

        DEBUG_TOKEN("  word[%-3d] %s%s%s%*s @%-3d #%-3d %s%s%s",
                    i,
                    qPrintable(chr_ldangle), qPrintable(name), qPrintable(chr_rdangle),
                    24 - name.length(), "",
                    pos, len,
                    qPrintable(chr_ldangle), qPrintable(str), qPrintable(chr_rdangle));
        i++;
    }
#endif


#if (DBG_REGEX || DBG_TOKEN)
    qDebug("%s: ·············································································", __func__);
#endif
    return words;
}

/**
 * @brief Return a p2_token_e enumeration value for the QString %str
 * @param str QString to scan for
 * @param chop if true, chop off characters until a a token other than t_nothing is matched
 * @param plen optional pointer to an int to receive the length of the token
 * @return p2_token_e enumeration value, or t_nothing if not a known string
 */
p2_TOKEN_e P2Token::token(const QString& line, int pos, int& len, bool chop) const
{
    p2_TOKEN_e tok = t_unknown;
    QStringRef ref(&line, pos, len);

    while (len > 0) {

        tok = m_string_token.value(ref.toString().toUpper());
        if (t_none != tok && t_unknown != tok) {
            len = m_token_string.value(tok).length();
            break;
        }

        if (pos == rx_comment_eol.indexIn(line, pos, QRegExp::CaretAtOffset)) {
            len = rx_comment_eol.matchedLength();
            tok = t_comment_eol;
            break;
        }

        if (pos == rx_comment_curly.indexIn(line, pos, QRegExp::CaretAtOffset)) {
            len = rx_comment_curly.matchedLength();
            tok = t_comment;
            break;
        }

        if (pos == rx_symbol.indexIn(line, pos, QRegExp::CaretAtOffset)) {
            len = rx_symbol.matchedLength();
            tok = t_symbol;
            break;
        }

        if (pos == rx_locsym.indexIn(line, pos, QRegExp::CaretAtOffset)) {
            len = rx_locsym.matchedLength();
            tok = t_locsym;
            break;
        }

        if (pos == rx_bin_const.indexIn(line, pos, QRegExp::CaretAtOffset)) {
            len = rx_bin_const.matchedLength();
            tok = t_bin_const;
            break;
        }

        if (pos == rx_byt_const.indexIn(line, pos, QRegExp::CaretAtOffset)) {
            len = rx_byt_const.matchedLength();
            tok = t_byt_const;
            break;
        }

        if (pos == rx_hex_const.indexIn(line, pos, QRegExp::CaretAtOffset)) {
            len = rx_hex_const.matchedLength();
            tok = t_hex_const;
            break;
        }

        if (pos == rx_real_const.indexIn(line, pos, QRegExp::CaretAtOffset)) {
            len = rx_real_const.matchedLength();
            tok = t_real_const;
            break;
        }

        if (pos == rx_dec_const.indexIn(line, pos, QRegExp::CaretAtOffset)) {
            len = rx_dec_const.matchedLength();
            tok = t_dec_const;
            break;
        }

        if (pos == rx_str_const.indexIn(line, pos, QRegExp::CaretAtOffset)) {
            len = rx_str_const.matchedLength();
            tok = t_str_const;
            break;
        }

        if (!chop)
            break;

        // chop off one character
        len--;
        ref.chop(1);
    }

    return tok;
}

/**
 * @brief Check if a token is one of a type mask
 * @param tok token value
 * @param typemask token type bit mask
 * @return true if token type is set, or false otherwise
 */
bool P2Token::is_type(p2_TOKEN_e tok, p2_TOKENMASK_t typemask) const
{
    const p2_TOKENMASK_t bits = m_token_type.value(tok, 0);
    return (bits & typemask) ? true : false;
}

/**
 * @brief Check if a token is of a specific type
 * @param tok token value
 * @param type token type
 * @return true if token type is set, or false otherwise
 */
bool P2Token::is_type(p2_TOKEN_e tok, p2_TOKENTYPE_e type) const
{
    const p2_TOKENMASK_t bits = m_token_type.value(tok, 0);
    const p2_TOKENMASK_t mask = TTMASK(type);
    return (bits & mask) ? true : false;
}

/**
 * @brief Check if a string is a token of a specific type
 * @param str string to tokenize
 * @param type token type
 * @return true if token type is set, or false otherwise
 */
bool P2Token::is_type(const QString& str, p2_TOKENTYPE_e type) const
{
    const p2_TOKEN_e tok = m_string_token.value(str);
    return is_type(tok, type);
}


/**
 * @brief Return the list of type names which are set in the type mask
 * @param typemask bit mask of token types
 * @return QStringList with the type names
 */
QStringList P2Token::type_names(p2_TOKENMASK_t typemask) const
{
    QStringList list;
    for (int i = 0; i <= 64 && typemask != 0; i++, typemask >>= 1)
        if (typemask & 1)
                list += m_t_type_name.value(static_cast<p2_TOKENTYPE_e>(i));
    if (list.isEmpty())
        list += m_t_type_name.value(static_cast<p2_TOKENTYPE_e>(tt_none));
    return list;
}

/**
 * @brief Return the list of type names which are set for a token
 * @param tok token value
 * @return QStringList with the type names
 */
QStringList P2Token::type_names(p2_TOKEN_e tok) const
{
    const p2_TOKENMASK_t typemask = m_token_type.value(tok, 0);
    return type_names(typemask);
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
p2_TOKEN_e P2Token::at_token(int& pos, const QString& str, QList<p2_TOKEN_e> tokens, p2_TOKEN_e dflt) const
{
    int len = str.length() - pos;
    p2_TOKEN_e tok = token(str.mid(pos), true, pos, len);
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
p2_TOKEN_e P2Token::at_token(const QString& str, QList<p2_TOKEN_e> tokens, p2_TOKEN_e dflt) const
{
    int pos = 0;
    return at_token(pos, str, tokens, dflt);
}

/**
 * @brief Return true, if a token value is an operation
 * @param tok token value
 * @return true if operation, or flags otherwise
 */
bool P2Token::is_operation(p2_TOKEN_e tok) const
{
    // Bit mask for operations
    const p2_TOKENMASK_t mask = m_token_type.value(tok, 0);
    return (mask & tm_operations) ? true : false;
}

/**
 * @brief Check if token is a conditional
 * @param tok token value
 * @return true if conditional, or false otherwise
 */
bool P2Token::is_conditional(p2_TOKEN_e tok) const
{
    return is_type(tok, tt_conditional);
}

/**
 * @brief Check if token is a MODCZ parameter
 * @param tok token value
 * @return true if MODCZ parameter, or false otherwise
 */
bool P2Token::is_modcz_param(p2_TOKEN_e tok) const
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
p2_TOKEN_e P2Token::at_type(int& pos, const QString& str, p2_TOKENMASK_t typemask, p2_TOKEN_e dflt) const
{
    int len = str.length() - pos;
    p2_TOKEN_e tok = token(str, true, pos, len);

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
p2_TOKEN_e P2Token::at_type(int& pos, const QString& str, p2_TOKENTYPE_e type, p2_TOKEN_e dflt) const
{
    int len = str.length() - pos;
    p2_TOKEN_e tok = token(str, true, pos, len);

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
p2_TOKEN_e P2Token::at_type(const QString& str, p2_TOKENTYPE_e type, p2_TOKEN_e dflt) const
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

p2_TOKEN_e P2Token::at_types(int& pos, const QString& str, p2_TOKENMASK_t typemask, p2_TOKEN_e dflt) const
{
    int len = str.length() - pos;
    p2_TOKEN_e tok = token(str, true, pos, len);

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
p2_TOKEN_e P2Token::at_types(int& pos, const QString& str, const QList<p2_TOKENTYPE_e>& types, p2_TOKEN_e dflt) const
{
    // Build bit mask for types
    p2_TOKENMASK_t typemask = tm_none;
    foreach(const p2_TOKENTYPE_e type, types)
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
p2_TOKEN_e P2Token::at_types(const QString& str, const QList<p2_TOKENTYPE_e>& types, p2_TOKEN_e dflt) const
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
p2_TOKEN_e P2Token::at_conditional(int& pos, const QString& str, p2_TOKEN_e dflt) const
{
    return at_type(pos, str, tt_conditional, dflt);
}

/**
 * @brief Check if a string starts with a token of type tt_conditional
 * @param str string to inspect
 * @param dflt default token value to return if not found
 * @return token value if found, or dflt otherwise
 */
p2_TOKEN_e P2Token::at_conditional(const QString& str, p2_TOKEN_e dflt) const
{
    int pos = 0;
    return at_conditional(pos, str, dflt);
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
p2_TOKEN_e P2Token::at_modcz_param(int& pos, const QString& str, p2_TOKEN_e dflt) const
{
    return at_type(pos, str, tt_modcz_param, dflt);
}

/**
 * @brief Check if a string starts with a token of type tt_modcz_param
 * @param str string to inspect
 * @param dflt default token value to return if not found
 * @return token value if found, or dflt otherwise
 */
p2_TOKEN_e P2Token::at_modcz_param(const QString& str, p2_TOKEN_e dflt) const
{
    int pos = 0;
    return at_modcz_param(pos, str, dflt);

}

/**
 * @brief Return the value for a conditional execution token
 * @param cond token with the condition
 * @param dflt default condition if token is not a conditional
 * @return One of the 16 p2_cond_e values
 */
p2_Cond_e P2Token::conditional(p2_TOKEN_e cond, p2_Cond_e dflt) const
{
    return m_lookup_cond.value(cond, dflt);
}

/**
 * @brief Return the value for a conditional execution string
 * @param str string with the condition
 * @param dflt default condition if string is not a conditional
 * @return One of the 16 p2_cond_e values
 */
p2_Cond_e P2Token::conditional(const QString& str, p2_Cond_e dflt) const
{
    int pos = 0;
    p2_TOKEN_e cond = at_type(pos, str, tt_conditional);
    return m_lookup_cond.value(cond, dflt);
}

/**
 * @brief Return the value for a MODCZ parameter token
 * @param cond token with the parameter
 * @param dflt default condition if token is not a MODCZ parameter
 * @return One of the 16 p2_cond_e values
 */
p2_Cond_e P2Token::modcz_param(p2_TOKEN_e cond, p2_Cond_e dflt) const
{
    return m_lookup_modcz.value(cond, dflt);
}

/**
 * @brief Return the value for a MODCZ parameter token
 * @param str string with the parameter
 * @param dflt default parameter if string is not a MODCZ parameter
 * @return One of the 16 p2_cond_e values
 */
p2_Cond_e P2Token::modcz_param(const QString& str, p2_Cond_e dflt) const
{
    int pos = 0;
    p2_TOKEN_e cond = at_type(pos, str, tt_modcz_param);
    return m_lookup_modcz.value(cond, dflt);
}

/**
 * @brief Set bits of typemask in the token %tok hash entry
 * @param tok token value
 * @param typemask bit mask with type(s) to set
 */
void P2Token::tt_set(p2_TOKEN_e tok, p2_TOKENMASK_t typemask)
{
    p2_TOKENMASK_t mask = m_token_type.value(tok, 0) | typemask;
    m_token_type.insert(tok, mask);
}

/**
 * @brief Clear bits of typemask in the token %tok hash entry
 * @param tok token value
 * @param typemask bit mask with type(s) to clear
 */
void P2Token::tt_clr(p2_TOKEN_e tok, p2_TOKENMASK_t typemask)
{
    p2_TOKENMASK_t mask = m_token_type.value(tok, 0) & ~typemask;
    m_token_type.insert(tok, mask);
}

/**
 * @brief Check bits of typemask in the token %tok hash entry
 * @param tok token value
 * @param typemask bit mask with type(s) to check for
 * @return true if any bit of typemask is set, or false otherwise
 */
bool P2Token::tt_chk(p2_TOKEN_e tok, p2_TOKENMASK_t typemask) const
{
    return m_token_type.value(tok, 0) & typemask ? true : false;
}

/**
 * @brief Add a token to the m_token_nam hash and its types to the typemask
 * @param tok token value
 * @param typemask token type bitmask
 * @param str string
 */
void P2Token::tn_ADD(p2_TOKEN_e tok, const QString& enum_name, p2_TOKENMASK_t typemask, const QString& string)
{
    m_token_enum_name.insert(tok, enum_name);
    m_token_string.insert(tok, string);
    tt_set(tok, typemask);
}
