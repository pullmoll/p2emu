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

#define DBG_REGEX   0
#define DBG_TOKEN   1

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
static const QString re_comment = QStringLiteral("((\\{[^\\}]*\\})|('.*)){1}");

/**
 * @brief Regular expression for local symbol
 *
 * leading "."
 * then any number of "A"…"Z", "0"…"9", or "_"
 */
static const QString re_locsym = QStringLiteral("[.][A-Z_]+[A-Z0-9_]*");

/**
 * @brief Regular expression for alphanumeric
 *
 * leading "A"…"Z", or "_"
 * then any number of "A"…"Z", "0"…"9", or "_"
 */
static const QString re_symbol = QStringLiteral("[A-Z_]+[A-Z0-9_]*");

/**
 * @brief Regular expression for binary number
 *
 * leading "%"
 * then one or more of "0", "1", or "_"
 */
static const QString re_bin_const = QStringLiteral("%[_]*[01_]+");

/**
 * @brief Regular expression for byte number
 *
 * leading "%%"
 * then one or more of "0"…"3", or "_"
 */
static const QString re_byt_const = QStringLiteral("%%[_]*[0-3_]+");

/**
 * @brief Regular expression for octal number
 *
 * leading "0"
 * then one or more of "0"…"7", or "_"
 */
static const QString re_oct_const = QStringLiteral("[0][_]*[0-7_]*");

/**
 * @brief Regular expression for a decimal number
 *
 * leading "0"…"9"
 * then any number of "0"…"9", or "_"
 */
static const QString re_dec_const = QStringLiteral("[1-9][_]*[0-9_]*");

/**
 * @brief Regular expression an octal number
 *
 * leading "$"
 * then any number of "0"…"9", "A"…"F", or "_"
 */
static const QString re_hex_const = QStringLiteral("\\$[_]*[0-9A-F_]+");

/**
 * @brief Regular expression for a string enclosed in doublequotes, possibly containing escaped doublequotes
 * initial '"'
 * then any number of escaped doublequotes (\") or other characters (.)
 * trailing '"'
 */
static const QString re_str_const = QStringLiteral("\"([^\\\"]|\\\\.)*\"");

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

P2Token::P2Token()
    : m_token_name()
    , m_name_token()
    , m_token_type()
    , m_type_token()
    , m_lookup_cond()
    , m_lookup_modcz()
    , m_ttype_name()
{
    tn_add(t_invalid,          tm_lexer, QStringLiteral("«INVALID»"));
    tn_add(t_unknown,          tm_lexer, QStringLiteral("«unknown»"));
    tn_add(t_comment,          tm_lexer | tm_comment, QStringLiteral("«comment»"));
    tn_add(t_comment_apo,      tm_lexer | tm_comment, QStringLiteral("'"));
    tn_add(t_comment_lcurly,   tm_lexer | tm_comment, QStringLiteral("{"));
    tn_add(t_comment_rcurly,   tm_lexer | tm_comment, QStringLiteral("}"));
    tn_add(t_comma,            tm_delimiter, QStringLiteral(","));
    tn_add(t_string,           tm_lexer, QStringLiteral("«str_const»"));
    tn_add(t_bin_const,        tm_lexer, QStringLiteral("«bin_const»"));
    tn_add(t_byt_const,        tm_lexer, QStringLiteral("«byt_const»"));
    tn_add(t_oct_const,        tm_lexer, QStringLiteral("«oct_const»"));
    tn_add(t_dec_const,        tm_lexer, QStringLiteral("«dec_const»"));
    tn_add(t_hex_const,        tm_lexer, QStringLiteral("«hex_const»"));
    tn_add(t_locsym,           tm_lexer, QStringLiteral("«locsym»"));
    tn_add(t_symbol,           tm_lexer, QStringLiteral("«symbol»"));

    tn_add(t_ABS,              tm_inst, QStringLiteral("ABS"));
    tn_add(t_ADD,              tm_inst, QStringLiteral("ADD"));
    tn_add(t_ADDCT1,           tm_inst, QStringLiteral("ADDCT1"));
    tn_add(t_ADDCT2,           tm_inst, QStringLiteral("ADDCT2"));
    tn_add(t_ADDCT3,           tm_inst, QStringLiteral("ADDCT3"));
    tn_add(t_ADDPIX,           tm_inst, QStringLiteral("ADDPIX"));
    tn_add(t_ADDS,             tm_inst, QStringLiteral("ADDS"));
    tn_add(t_ADDSX,            tm_inst, QStringLiteral("ADDSX"));
    tn_add(t_ADDX,             tm_inst, QStringLiteral("ADDX"));
    tn_add(t_AKPIN,            tm_inst, QStringLiteral("AKPIN"));
    tn_add(t_ALLOWI,           tm_inst, QStringLiteral("ALLOWI"));
    tn_add(t_ALTB,             tm_inst, QStringLiteral("ALTB"));
    tn_add(t_ALTD,             tm_inst, QStringLiteral("ALTD"));
    tn_add(t_ALTGB,            tm_inst, QStringLiteral("ALTGB"));
    tn_add(t_ALTGN,            tm_inst, QStringLiteral("ALTGN"));
    tn_add(t_ALTGW,            tm_inst, QStringLiteral("ALTGW"));
    tn_add(t_ALTI,             tm_inst, QStringLiteral("ALTI"));
    tn_add(t_ALTR,             tm_inst, QStringLiteral("ALTR"));
    tn_add(t_ALTS,             tm_inst, QStringLiteral("ALTS"));
    tn_add(t_ALTSB,            tm_inst, QStringLiteral("ALTSB"));
    tn_add(t_ALTSN,            tm_inst, QStringLiteral("ALTSN"));
    tn_add(t_ALTSW,            tm_inst, QStringLiteral("ALTSW"));
    tn_add(t_AND,              tm_inst, QStringLiteral("AND"));
    tn_add(t_ANDN,             tm_inst, QStringLiteral("ANDN"));
    tn_add(t_AUGD,             tm_inst, QStringLiteral("AUGD"));
    tn_add(t_AUGS,             tm_inst, QStringLiteral("AUGS"));
    tn_add(t_BITC,             tm_inst, QStringLiteral("BITC"));
    tn_add(t_BITH,             tm_inst, QStringLiteral("BITH"));
    tn_add(t_BITL,             tm_inst, QStringLiteral("BITL"));
    tn_add(t_BITNC,            tm_inst, QStringLiteral("BITNC"));
    tn_add(t_BITNOT,           tm_inst, QStringLiteral("BITNOT"));
    tn_add(t_BITNZ,            tm_inst, QStringLiteral("BITNZ"));
    tn_add(t_BITRND,           tm_inst, QStringLiteral("BITRND"));
    tn_add(t_BITZ,             tm_inst, QStringLiteral("BITZ"));
    tn_add(t_BLNPIX,           tm_inst, QStringLiteral("BLNPIX"));
    tn_add(t_BMASK,            tm_inst, QStringLiteral("BMASK"));
    tn_add(t_BRK,              tm_inst, QStringLiteral("BRK"));
    tn_add(t_CALL,             tm_inst, QStringLiteral("CALL"));
    tn_add(t_CALLA,            tm_inst, QStringLiteral("CALLA"));
    tn_add(t_CALLB,            tm_inst, QStringLiteral("CALLB"));
    tn_add(t_CALLD,            tm_inst, QStringLiteral("CALLD"));
    tn_add(t_CALLPA,           tm_inst, QStringLiteral("CALLPA"));
    tn_add(t_CALLPB,           tm_inst, QStringLiteral("CALLPB"));
    tn_add(t_CMP,              tm_inst, QStringLiteral("CMP"));
    tn_add(t_CMPM,             tm_inst, QStringLiteral("CMPM"));
    tn_add(t_CMPR,             tm_inst, QStringLiteral("CMPR"));
    tn_add(t_CMPS,             tm_inst, QStringLiteral("CMPS"));
    tn_add(t_CMPSUB,           tm_inst, QStringLiteral("CMPSUB"));
    tn_add(t_CMPSX,            tm_inst, QStringLiteral("CMPSX"));
    tn_add(t_CMPX,             tm_inst, QStringLiteral("CMPX"));
    tn_add(t_COGATN,           tm_inst, QStringLiteral("COGATN"));
    tn_add(t_COGBRK,           tm_inst, QStringLiteral("COGBRK"));
    tn_add(t_COGID,            tm_inst, QStringLiteral("COGID"));
    tn_add(t_COGINIT,          tm_inst, QStringLiteral("COGINIT"));
    tn_add(t_COGSTOP,          tm_inst, QStringLiteral("COGSTOP"));
    tn_add(t_CRCBIT,           tm_inst, QStringLiteral("CRCBIT"));
    tn_add(t_CRCNIB,           tm_inst, QStringLiteral("CRCNIB"));
    tn_add(t_DECMOD,           tm_inst, QStringLiteral("DECMOD"));
    tn_add(t_DECOD,            tm_inst, QStringLiteral("DECOD"));
    tn_add(t_DIRA,             tm_constant, QStringLiteral("DIRA"));
    tn_add(t_DIRB,             tm_constant, QStringLiteral("DIRB"));
    tn_add(t_DIRC,             tm_inst, QStringLiteral("DIRC"));
    tn_add(t_DIRH,             tm_inst, QStringLiteral("DIRH"));
    tn_add(t_DIRL,             tm_inst, QStringLiteral("DIRL"));
    tn_add(t_DIRNC,            tm_inst, QStringLiteral("DIRNC"));
    tn_add(t_DIRNOT,           tm_inst, QStringLiteral("DIRNOT"));
    tn_add(t_DIRNZ,            tm_inst, QStringLiteral("DIRNZ"));
    tn_add(t_DIRRND,           tm_inst, QStringLiteral("DIRRND"));
    tn_add(t_DIRZ,             tm_inst, QStringLiteral("DIRZ"));
    tn_add(t_DJF,              tm_inst, QStringLiteral("DJF"));
    tn_add(t_DJNF,             tm_inst, QStringLiteral("DJNF"));
    tn_add(t_DJNZ,             tm_inst, QStringLiteral("DJNZ"));
    tn_add(t_DJZ,              tm_inst, QStringLiteral("DJZ"));
    tn_add(t_DRVC,             tm_inst, QStringLiteral("DRVC"));
    tn_add(t_DRVH,             tm_inst, QStringLiteral("DRVH"));
    tn_add(t_DRVL,             tm_inst, QStringLiteral("DRVL"));
    tn_add(t_DRVNC,            tm_inst, QStringLiteral("DRVNC"));
    tn_add(t_DRVNOT,           tm_inst, QStringLiteral("DRVNOT"));
    tn_add(t_DRVNZ,            tm_inst, QStringLiteral("DRVNZ"));
    tn_add(t_DRVRND,           tm_inst, QStringLiteral("DRVRND"));
    tn_add(t_DRVZ,             tm_inst, QStringLiteral("DRVZ"));
    tn_add(t_ENCOD,            tm_inst, QStringLiteral("ENCOD"));
    tn_add(t_EXECF,            tm_inst, QStringLiteral("EXECF"));
    tn_add(t_FBLOCK,           tm_inst, QStringLiteral("FBLOCK"));
    tn_add(t_FGE,              tm_inst, QStringLiteral("FGE"));
    tn_add(t_FGES,             tm_inst, QStringLiteral("FGES"));
    tn_add(t_FLE,              tm_inst, QStringLiteral("FLE"));
    tn_add(t_FLES,             tm_inst, QStringLiteral("FLES"));
    tn_add(t_FLTC,             tm_inst, QStringLiteral("FLTC"));
    tn_add(t_FLTH,             tm_inst, QStringLiteral("FLTH"));
    tn_add(t_FLTL,             tm_inst, QStringLiteral("FLTL"));
    tn_add(t_FLTNC,            tm_inst, QStringLiteral("FLTNC"));
    tn_add(t_FLTNOT,           tm_inst, QStringLiteral("FLTNOT"));
    tn_add(t_FLTNZ,            tm_inst, QStringLiteral("FLTNZ"));
    tn_add(t_FLTRND,           tm_inst, QStringLiteral("FLTRND"));
    tn_add(t_FLTZ,             tm_inst, QStringLiteral("FLTZ"));
    tn_add(t_GETBRK,           tm_inst, QStringLiteral("GETBRK"));
    tn_add(t_GETBYTE,          tm_inst, QStringLiteral("GETBYTE"));
    tn_add(t_GETCT,            tm_inst, QStringLiteral("GETCT"));
    tn_add(t_GETNIB,           tm_inst, QStringLiteral("GETNIB"));
    tn_add(t_GETPTR,           tm_inst, QStringLiteral("GETPTR"));
    tn_add(t_GETQX,            tm_inst, QStringLiteral("GETQX"));
    tn_add(t_GETQY,            tm_inst, QStringLiteral("GETQY"));
    tn_add(t_GETRND,           tm_inst, QStringLiteral("GETRND"));
    tn_add(t_GETSCP,           tm_inst, QStringLiteral("GETSCP"));
    tn_add(t_GETWORD,          tm_inst, QStringLiteral("GETWORD"));
    tn_add(t_GETXACC,          tm_inst, QStringLiteral("GETXACC"));
    tn_add(t_HUBSET,           tm_inst, QStringLiteral("HUBSET"));
    tn_add(t_IJNZ,             tm_inst, QStringLiteral("IJNZ"));
    tn_add(t_IJZ,              tm_inst, QStringLiteral("IJZ"));
    tn_add(t_INA,              tm_constant, QStringLiteral("INA"));
    tn_add(t_INB,              tm_constant, QStringLiteral("INB"));
    tn_add(t_INCMOD,           tm_inst, QStringLiteral("INCMOD"));
    tn_add(t_JATN,             tm_inst, QStringLiteral("JATN"));
    tn_add(t_JCT1,             tm_inst, QStringLiteral("JCT1"));
    tn_add(t_JCT2,             tm_inst, QStringLiteral("JCT2"));
    tn_add(t_JCT3,             tm_inst, QStringLiteral("JCT3"));
    tn_add(t_JFBW,             tm_inst, QStringLiteral("JFBW"));
    tn_add(t_JINT,             tm_inst, QStringLiteral("JINT"));
    tn_add(t_JMP,              tm_inst, QStringLiteral("JMP"));
    tn_add(t_JMPREL,           tm_inst, QStringLiteral("JMPREL"));
    tn_add(t_JNATN,            tm_inst, QStringLiteral("JNATN"));
    tn_add(t_JNCT1,            tm_inst, QStringLiteral("JNCT1"));
    tn_add(t_JNCT2,            tm_inst, QStringLiteral("JNCT2"));
    tn_add(t_JNCT3,            tm_inst, QStringLiteral("JNCT3"));
    tn_add(t_JNFBW,            tm_inst, QStringLiteral("JNFBW"));
    tn_add(t_JNINT,            tm_inst, QStringLiteral("JNINT"));
    tn_add(t_JNPAT,            tm_inst, QStringLiteral("JNPAT"));
    tn_add(t_JNQMT,            tm_inst, QStringLiteral("JNQMT"));
    tn_add(t_JNSE1,            tm_inst, QStringLiteral("JNSE1"));
    tn_add(t_JNSE2,            tm_inst, QStringLiteral("JNSE2"));
    tn_add(t_JNSE3,            tm_inst, QStringLiteral("JNSE3"));
    tn_add(t_JNSE4,            tm_inst, QStringLiteral("JNSE4"));
    tn_add(t_JNXFI,            tm_inst, QStringLiteral("JNXFI"));
    tn_add(t_JNXMT,            tm_inst, QStringLiteral("JNXMT"));
    tn_add(t_JNXRL,            tm_inst, QStringLiteral("JNXRL"));
    tn_add(t_JNXRO,            tm_inst, QStringLiteral("JNXRO"));
    tn_add(t_JPAT,             tm_inst, QStringLiteral("JPAT"));
    tn_add(t_JQMT,             tm_inst, QStringLiteral("JQMT"));
    tn_add(t_JSE1,             tm_inst, QStringLiteral("JSE1"));
    tn_add(t_JSE2,             tm_inst, QStringLiteral("JSE2"));
    tn_add(t_JSE3,             tm_inst, QStringLiteral("JSE3"));
    tn_add(t_JSE4,             tm_inst, QStringLiteral("JSE4"));
    tn_add(t_JXFI,             tm_inst, QStringLiteral("JXFI"));
    tn_add(t_JXMT,             tm_inst, QStringLiteral("JXMT"));
    tn_add(t_JXRL,             tm_inst, QStringLiteral("JXRL"));
    tn_add(t_JXRO,             tm_inst, QStringLiteral("JXRO"));
    tn_add(t_LOC,              tm_inst, QStringLiteral("LOC"));
    tn_add(t_LOCKNEW,          tm_inst, QStringLiteral("LOCKNEW"));
    tn_add(t_LOCKREL,          tm_inst, QStringLiteral("LOCKREL"));
    tn_add(t_LOCKRET,          tm_inst, QStringLiteral("LOCKRET"));
    tn_add(t_LOCKTRY,          tm_inst, QStringLiteral("LOCKTRY"));
    tn_add(t_MERGEB,           tm_inst, QStringLiteral("MERGEB"));
    tn_add(t_MERGEW,           tm_inst, QStringLiteral("MERGEW"));
    tn_add(t_MIXPIX,           tm_inst, QStringLiteral("MIXPIX"));
    tn_add(t_MODCZ,            tm_inst, QStringLiteral("MODCZ"));
    tn_add(t_MOV,              tm_inst, QStringLiteral("MOV"));
    tn_add(t_MOVBYTS,          tm_inst, QStringLiteral("MOVBYTS"));
    tn_add(t_MUL,              tm_inst, QStringLiteral("MUL"));
    tn_add(t_MULPIX,           tm_inst, QStringLiteral("MULPIX"));
    tn_add(t_MULS,             tm_inst, QStringLiteral("MULS"));
    tn_add(t_MUXC,             tm_inst, QStringLiteral("MUXC"));
    tn_add(t_MUXNC,            tm_inst, QStringLiteral("MUXNC"));
    tn_add(t_MUXNIBS,          tm_inst, QStringLiteral("MUXNIBS"));
    tn_add(t_MUXNITS,          tm_inst, QStringLiteral("MUXNITS"));
    tn_add(t_MUXNZ,            tm_inst, QStringLiteral("MUXNZ"));
    tn_add(t_MUXQ,             tm_inst, QStringLiteral("MUXQ"));
    tn_add(t_MUXZ,             tm_inst, QStringLiteral("MUXZ"));
    tn_add(t_NEG,              tm_inst, QStringLiteral("NEG"));
    tn_add(t_NEGC,             tm_inst, QStringLiteral("NEGC"));
    tn_add(t_NEGNC,            tm_inst, QStringLiteral("NEGNC"));
    tn_add(t_NEGNZ,            tm_inst, QStringLiteral("NEGNZ"));
    tn_add(t_NEGZ,             tm_inst, QStringLiteral("NEGZ"));
    tn_add(t_NIXINT1,          tm_inst, QStringLiteral("NIXINT1"));
    tn_add(t_NIXINT2,          tm_inst, QStringLiteral("NIXINT2"));
    tn_add(t_NIXINT3,          tm_inst, QStringLiteral("NIXINT3"));
    tn_add(t_NOP,              tm_inst, QStringLiteral("NOP"));
    tn_add(t_NOT,              tm_inst, QStringLiteral("NOT"));
    tn_add(t_ONES,             tm_inst, QStringLiteral("ONES"));
    tn_add(t_OR,               tm_inst, QStringLiteral("OR"));
    tn_add(t_OUTA,             tm_constant, QStringLiteral("OUTA"));
    tn_add(t_OUTB,             tm_constant, QStringLiteral("OUTB"));
    tn_add(t_OUTC,             tm_inst, QStringLiteral("OUTC"));
    tn_add(t_OUTH,             tm_inst, QStringLiteral("OUTH"));
    tn_add(t_OUTL,             tm_inst, QStringLiteral("OUTL"));
    tn_add(t_OUTNC,            tm_inst, QStringLiteral("OUTNC"));
    tn_add(t_OUTNOT,           tm_inst, QStringLiteral("OUTNOT"));
    tn_add(t_OUTNZ,            tm_inst, QStringLiteral("OUTNZ"));
    tn_add(t_OUTRND,           tm_inst, QStringLiteral("OUTRND"));
    tn_add(t_OUTZ,             tm_inst, QStringLiteral("OUTZ"));
    tn_add(t_PA,               tm_inst, QStringLiteral("PA"));
    tn_add(t_PB,               tm_inst, QStringLiteral("PB"));
    tn_add(t_POLLATN,          tm_inst, QStringLiteral("POLLATN"));
    tn_add(t_POLLCT1,          tm_inst, QStringLiteral("POLLCT1"));
    tn_add(t_POLLCT2,          tm_inst, QStringLiteral("POLLCT2"));
    tn_add(t_POLLCT3,          tm_inst, QStringLiteral("POLLCT3"));
    tn_add(t_POLLFBW,          tm_inst, QStringLiteral("POLLFBW"));
    tn_add(t_POLLINT,          tm_inst, QStringLiteral("POLLINT"));
    tn_add(t_POLLPAT,          tm_inst, QStringLiteral("POLLPAT"));
    tn_add(t_POLLQMT,          tm_inst, QStringLiteral("POLLQMT"));
    tn_add(t_POLLSE1,          tm_inst, QStringLiteral("POLLSE1"));
    tn_add(t_POLLSE2,          tm_inst, QStringLiteral("POLLSE2"));
    tn_add(t_POLLSE3,          tm_inst, QStringLiteral("POLLSE3"));
    tn_add(t_POLLSE4,          tm_inst, QStringLiteral("POLLSE4"));
    tn_add(t_POLLXFI,          tm_inst, QStringLiteral("POLLXFI"));
    tn_add(t_POLLXMT,          tm_inst, QStringLiteral("POLLXMT"));
    tn_add(t_POLLXRL,          tm_inst, QStringLiteral("POLLXRL"));
    tn_add(t_POLLXRO,          tm_inst, QStringLiteral("POLLXRO"));
    tn_add(t_POP,              tm_inst, QStringLiteral("POP"));
    tn_add(t_POPA,             tm_inst, QStringLiteral("POPA"));
    tn_add(t_POPB,             tm_inst, QStringLiteral("POPB"));
    tn_add(t_PTRA,             tm_constant, QStringLiteral("PTRA"));
    tn_add(t_PTRA_postinc,     tm_constant, QStringLiteral("PTRA++"));
    tn_add(t_PTRA_postdec,     tm_constant, QStringLiteral("PTRA--"));
    tn_add(t_PTRA_preinc,      tm_constant, QStringLiteral("++PTRA"));
    tn_add(t_PTRA_predec,      tm_constant, QStringLiteral("--PTRA"));
    tn_add(t_PTRB,             tm_constant, QStringLiteral("PTRB"));
    tn_add(t_PTRB_postinc,     tm_constant, QStringLiteral("PTRB++"));
    tn_add(t_PTRB_postdec,     tm_constant, QStringLiteral("PTRB--"));
    tn_add(t_PTRB_preinc,      tm_constant, QStringLiteral("++PTRB"));
    tn_add(t_PTRB_predec,      tm_constant, QStringLiteral("--PTRB"));
    tn_add(t_PUSH,             tm_inst, QStringLiteral("PUSH"));
    tn_add(t_PUSHA,            tm_inst, QStringLiteral("PUSHA"));
    tn_add(t_PUSHB,            tm_inst, QStringLiteral("PUSHB"));
    tn_add(t_QDIV,             tm_inst, QStringLiteral("QDIV"));
    tn_add(t_QEXP,             tm_inst, QStringLiteral("QEXP"));
    tn_add(t_QFRAC,            tm_inst, QStringLiteral("QFRAC"));
    tn_add(t_QLOG,             tm_inst, QStringLiteral("QLOG"));
    tn_add(t_QMUL,             tm_inst, QStringLiteral("QMUL"));
    tn_add(t_QROTATE,          tm_inst, QStringLiteral("QROTATE"));
    tn_add(t_QSQRT,            tm_inst, QStringLiteral("QSQRT"));
    tn_add(t_QVECTOR,          tm_inst, QStringLiteral("QVECTOR"));
    tn_add(t_RCL,              tm_inst, QStringLiteral("RCL"));
    tn_add(t_RCR,              tm_inst, QStringLiteral("RCR"));
    tn_add(t_RCZL,             tm_inst, QStringLiteral("RCZL"));
    tn_add(t_RCZR,             tm_inst, QStringLiteral("RCZR"));
    tn_add(t_RDBYTE,           tm_inst, QStringLiteral("RDBYTE"));
    tn_add(t_RDFAST,           tm_inst, QStringLiteral("RDFAST"));
    tn_add(t_RDLONG,           tm_inst, QStringLiteral("RDLONG"));
    tn_add(t_RDLUT,            tm_inst, QStringLiteral("RDLUT"));
    tn_add(t_RDPIN,            tm_inst, QStringLiteral("RDPIN"));
    tn_add(t_RDWORD,           tm_inst, QStringLiteral("RDWORD"));
    tn_add(t_REP,              tm_inst, QStringLiteral("REP"));
    tn_add(t_RESI0,            tm_inst, QStringLiteral("RESI0"));
    tn_add(t_RESI1,            tm_inst, QStringLiteral("RESI1"));
    tn_add(t_RESI2,            tm_inst, QStringLiteral("RESI2"));
    tn_add(t_RESI3,            tm_inst, QStringLiteral("RESI3"));
    tn_add(t_RET,              tm_inst, QStringLiteral("RET"));
    tn_add(t_RETA,             tm_inst, QStringLiteral("RETA"));
    tn_add(t_RETB,             tm_inst, QStringLiteral("RETB"));
    tn_add(t_RETI0,            tm_inst, QStringLiteral("RETI0"));
    tn_add(t_RETI1,            tm_inst, QStringLiteral("RETI1"));
    tn_add(t_RETI2,            tm_inst, QStringLiteral("RETI2"));
    tn_add(t_RETI3,            tm_inst, QStringLiteral("RETI3"));
    tn_add(t_REV,              tm_inst, QStringLiteral("REV"));
    tn_add(t_RFBYTE,           tm_inst, QStringLiteral("RFBYTE"));
    tn_add(t_RFLONG,           tm_inst, QStringLiteral("RFLONG"));
    tn_add(t_RFVAR,            tm_inst, QStringLiteral("RFVAR"));
    tn_add(t_RFVARS,           tm_inst, QStringLiteral("RFVARS"));
    tn_add(t_RFWORD,           tm_inst, QStringLiteral("RFWORD"));
    tn_add(t_RGBEXP,           tm_inst, QStringLiteral("RGBEXP"));
    tn_add(t_RGBSQZ,           tm_inst, QStringLiteral("RGBSQZ"));
    tn_add(t_ROL,              tm_inst, QStringLiteral("ROL"));
    tn_add(t_ROLBYTE,          tm_inst, QStringLiteral("ROLBYTE"));
    tn_add(t_ROLNIB,           tm_inst, QStringLiteral("ROLNIB"));
    tn_add(t_ROLWORD,          tm_inst, QStringLiteral("ROLWORD"));
    tn_add(t_ROR,              tm_inst, QStringLiteral("ROR"));
    tn_add(t_RQPIN,            tm_inst, QStringLiteral("RQPIN"));
    tn_add(t_SAL,              tm_inst, QStringLiteral("SAL"));
    tn_add(t_SAR,              tm_inst, QStringLiteral("SAR"));
    tn_add(t_SCA,              tm_inst, QStringLiteral("SCA"));
    tn_add(t_SCAS,             tm_inst, QStringLiteral("SCAS"));
    tn_add(t_SETBYTE,          tm_inst, QStringLiteral("SETBYTE"));
    tn_add(t_SETCFRQ,          tm_inst, QStringLiteral("SETCFRQ"));
    tn_add(t_SETCI,            tm_inst, QStringLiteral("SETCI"));
    tn_add(t_SETCMOD,          tm_inst, QStringLiteral("SETCMOD"));
    tn_add(t_SETCQ,            tm_inst, QStringLiteral("SETCQ"));
    tn_add(t_SETCY,            tm_inst, QStringLiteral("SETCY"));
    tn_add(t_SETD,             tm_inst, QStringLiteral("SETD"));
    tn_add(t_SETDACS,          tm_inst, QStringLiteral("SETDACS"));
    tn_add(t_SETINT1,          tm_inst, QStringLiteral("SETINT1"));
    tn_add(t_SETINT2,          tm_inst, QStringLiteral("SETINT2"));
    tn_add(t_SETINT3,          tm_inst, QStringLiteral("SETINT3"));
    tn_add(t_SETLUTS,          tm_inst, QStringLiteral("SETLUTS"));
    tn_add(t_SETNIB,           tm_inst, QStringLiteral("SETNIB"));
    tn_add(t_SETPAT,           tm_inst, QStringLiteral("SETPAT"));
    tn_add(t_SETPIV,           tm_inst, QStringLiteral("SETPIV"));
    tn_add(t_SETPIX,           tm_inst, QStringLiteral("SETPIX"));
    tn_add(t_SETQ,             tm_inst, QStringLiteral("SETQ"));
    tn_add(t_SETQ2,            tm_inst, QStringLiteral("SETQ2"));
    tn_add(t_SETR,             tm_inst, QStringLiteral("SETR"));
    tn_add(t_SETS,             tm_inst, QStringLiteral("SETS"));
    tn_add(t_SETSCP,           tm_inst, QStringLiteral("SETSCP"));
    tn_add(t_SETSE1,           tm_inst, QStringLiteral("SETSE1"));
    tn_add(t_SETSE2,           tm_inst, QStringLiteral("SETSE2"));
    tn_add(t_SETSE3,           tm_inst, QStringLiteral("SETSE3"));
    tn_add(t_SETSE4,           tm_inst, QStringLiteral("SETSE4"));
    tn_add(t_SETWORD,          tm_inst, QStringLiteral("SETWORD"));
    tn_add(t_SETXFRQ,          tm_inst, QStringLiteral("SETXFRQ"));
    tn_add(t_SEUSSF,           tm_inst, QStringLiteral("SEUSSF"));
    tn_add(t_SEUSSR,           tm_inst, QStringLiteral("SEUSSR"));
    tn_add(t_SHL,              tm_inst, QStringLiteral("SHL"));
    tn_add(t_SHR,              tm_inst, QStringLiteral("SHR"));
    tn_add(t_SIGNX,            tm_inst, QStringLiteral("SIGNX"));
    tn_add(t_SKIP,             tm_inst, QStringLiteral("SKIP"));
    tn_add(t_SKIPF,            tm_inst, QStringLiteral("SKIPF"));
    tn_add(t_SPLITB,           tm_inst, QStringLiteral("SPLITB"));
    tn_add(t_SPLITW,           tm_inst, QStringLiteral("SPLITW"));
    tn_add(t_STALLI,           tm_inst, QStringLiteral("STALLI"));
    tn_add(t_SUB,              tm_inst, QStringLiteral("SUB"));
    tn_add(t_SUBR,             tm_inst, QStringLiteral("SUBR"));
    tn_add(t_SUBS,             tm_inst, QStringLiteral("SUBS"));
    tn_add(t_SUBSX,            tm_inst, QStringLiteral("SUBSX"));
    tn_add(t_SUBX,             tm_inst, QStringLiteral("SUBX"));
    tn_add(t_SUMC,             tm_inst, QStringLiteral("SUMC"));
    tn_add(t_SUMNC,            tm_inst, QStringLiteral("SUMNC"));
    tn_add(t_SUMNZ,            tm_inst, QStringLiteral("SUMNZ"));
    tn_add(t_SUMZ,             tm_inst, QStringLiteral("SUMZ"));
    tn_add(t_TEST,             tm_inst, QStringLiteral("TEST"));
    tn_add(t_TESTB,            tm_inst, QStringLiteral("TESTB"));
    tn_add(t_TESTBN,           tm_inst, QStringLiteral("TESTBN"));
    tn_add(t_TESTN,            tm_inst, QStringLiteral("TESTN"));
    tn_add(t_TESTP,            tm_inst, QStringLiteral("TESTP"));
    tn_add(t_TESTPN,           tm_inst, QStringLiteral("TESTPN"));
    tn_add(t_TJF,              tm_inst, QStringLiteral("TJF"));
    tn_add(t_TJNF,             tm_inst, QStringLiteral("TJNF"));
    tn_add(t_TJNS,             tm_inst, QStringLiteral("TJNS"));
    tn_add(t_TJNZ,             tm_inst, QStringLiteral("TJNZ"));
    tn_add(t_TJS,              tm_inst, QStringLiteral("TJS"));
    tn_add(t_TJV,              tm_inst, QStringLiteral("TJV"));
    tn_add(t_TJZ,              tm_inst, QStringLiteral("TJZ"));
    tn_add(t_TRGINT1,          tm_inst, QStringLiteral("TRGINT1"));
    tn_add(t_TRGINT2,          tm_inst, QStringLiteral("TRGINT2"));
    tn_add(t_TRGINT3,          tm_inst, QStringLiteral("TRGINT3"));
    tn_add(t_WAITATN,          tm_inst, QStringLiteral("WAITATN"));
    tn_add(t_WAITCT1,          tm_inst, QStringLiteral("WAITCT1"));
    tn_add(t_WAITCT2,          tm_inst, QStringLiteral("WAITCT2"));
    tn_add(t_WAITCT3,          tm_inst, QStringLiteral("WAITCT3"));
    tn_add(t_WAITFBW,          tm_inst, QStringLiteral("WAITFBW"));
    tn_add(t_WAITINT,          tm_inst, QStringLiteral("WAITINT"));
    tn_add(t_WAITPAT,          tm_inst, QStringLiteral("WAITPAT"));
    tn_add(t_WAITSE1,          tm_inst, QStringLiteral("WAITSE1"));
    tn_add(t_WAITSE2,          tm_inst, QStringLiteral("WAITSE2"));
    tn_add(t_WAITSE3,          tm_inst, QStringLiteral("WAITSE3"));
    tn_add(t_WAITSE4,          tm_inst, QStringLiteral("WAITSE4"));
    tn_add(t_WAITX,            tm_inst, QStringLiteral("WAITX"));
    tn_add(t_WAITXFI,          tm_inst, QStringLiteral("WAITXFI"));
    tn_add(t_WAITXMT,          tm_inst, QStringLiteral("WAITXMT"));
    tn_add(t_WAITXRL,          tm_inst, QStringLiteral("WAITXRL"));
    tn_add(t_WAITXRO,          tm_inst, QStringLiteral("WAITXRO"));
    tn_add(t_WFBYTE,           tm_inst, QStringLiteral("WFBYTE"));
    tn_add(t_WFLONG,           tm_inst, QStringLiteral("WFLONG"));
    tn_add(t_WFWORD,           tm_inst, QStringLiteral("WFWORD"));
    tn_add(t_WMLONG,           tm_inst, QStringLiteral("WMLONG"));
    tn_add(t_WRBYTE,           tm_inst, QStringLiteral("WRBYTE"));
    tn_add(t_WRC,              tm_inst, QStringLiteral("WRC"));
    tn_add(t_WRFAST,           tm_inst, QStringLiteral("WRFAST"));
    tn_add(t_WRLONG,           tm_inst, QStringLiteral("WRLONG"));
    tn_add(t_WRLUT,            tm_inst, QStringLiteral("WRLUT"));
    tn_add(t_WRNC,             tm_inst, QStringLiteral("WRNC"));
    tn_add(t_WRNZ,             tm_inst, QStringLiteral("WRNZ"));
    tn_add(t_WRPIN,            tm_inst, QStringLiteral("WRPIN"));
    tn_add(t_WRWORD,           tm_inst, QStringLiteral("WRWORD"));
    tn_add(t_WRZ,              tm_inst, QStringLiteral("WRZ"));
    tn_add(t_WXPIN,            tm_inst, QStringLiteral("WXPIN"));
    tn_add(t_WYPIN,            tm_inst, QStringLiteral("WYPIN"));
    tn_add(t_XCONT,            tm_inst, QStringLiteral("XCONT"));
    tn_add(t_XINIT,            tm_inst, QStringLiteral("XINIT"));
    tn_add(t_XOR,              tm_inst, QStringLiteral("XOR"));
    tn_add(t_XORO32,           tm_inst, QStringLiteral("XORO32"));
    tn_add(t_XSTOP,            tm_inst, QStringLiteral("XSTOP"));
    tn_add(t_XZERO,            tm_inst, QStringLiteral("XZERO"));
    tn_add(t_ZEROX,            tm_inst, QStringLiteral("ZEROX"));
    tn_add(t_empty,            tm_inst, QStringLiteral("<empty>"));

    tn_add(t_WC,               tm_wcz_suffix, QStringLiteral("WC"));
    tn_add(t_WZ,               tm_wcz_suffix, QStringLiteral("WZ"));
    tn_add(t_WCZ,              tm_wcz_suffix, QStringLiteral("WCZ"));
    tn_add(t_ANDC,             tm_wcz_suffix, QStringLiteral("ANDC"));
    tn_add(t_ANDZ,             tm_wcz_suffix, QStringLiteral("ANDZ"));
    tn_add(t_ORC,              tm_wcz_suffix, QStringLiteral("ORC"));
    tn_add(t_ORZ,              tm_wcz_suffix, QStringLiteral("ORZ"));
    tn_add(t_XORC,             tm_wcz_suffix, QStringLiteral("XORC"));
    tn_add(t_XORZ,             tm_wcz_suffix, QStringLiteral("XORZ"));

    // Data
    tn_add(t__BYTE,            tm_inst | tm_data, QStringLiteral("BYTE"));
    tn_add(t__WORD,            tm_inst | tm_data, QStringLiteral("WORD"));
    tn_add(t__LONG,            tm_inst | tm_data, QStringLiteral("LONG"));
    tn_add(t__RES,             tm_inst | tm_data, QStringLiteral("RES"));
    tn_add(t__FILE,            tm_inst | tm_data, QStringLiteral("FILE"));

    // Section control
    tn_add(t__DAT,             tm_section, QStringLiteral("DAT"));
    tn_add(t__CON,             tm_section, QStringLiteral("CON"));
    tn_add(t__PUB,             tm_section, QStringLiteral("PUB"));
    tn_add(t__PRI,             tm_section, QStringLiteral("PRI"));
    tn_add(t__VAR,             tm_section, QStringLiteral("VAR"));

    // Origin control
    tn_add(t__ALIGNW,          tm_inst | tm_origin, QStringLiteral("ALIGNW"));
    tn_add(t__ALIGNL,          tm_inst | tm_origin, QStringLiteral("ALIGNL"));
    tn_add(t__ORG,             tm_inst | tm_origin, QStringLiteral("ORG"));
    tn_add(t__ORGH,            tm_inst | tm_origin, QStringLiteral("ORGH"));
    tn_add(t__FIT,             tm_inst | tm_origin, QStringLiteral("FIT"));

    // Conditionals
    tn_add(t__RET_,            tm_conditional, QStringLiteral("_RET_"));
    tn_add(t_IF_NZ_AND_NC,     tm_conditional, QStringLiteral("IF_NZ_AND_NC"));
    tn_add(t_IF_NC_AND_NZ,     tm_conditional, QStringLiteral("IF_NC_AND_NZ"));
    tn_add(t_IF_A,             tm_conditional, QStringLiteral("IF_A"));
    tn_add(t_IF_GT,            tm_conditional, QStringLiteral("IF_GT"));
    tn_add(t_IF_Z_AND_NC,      tm_conditional, QStringLiteral("IF_Z_AND_NC"));
    tn_add(t_IF_NC_AND_Z,      tm_conditional, QStringLiteral("IF_NC_AND_Z"));
    tn_add(t_IF_NC,            tm_conditional, QStringLiteral("IF_NC"));
    tn_add(t_IF_AE,            tm_conditional, QStringLiteral("IF_AE"));
    tn_add(t_IF_GE,            tm_conditional, QStringLiteral("IF_GE"));
    tn_add(t_IF_NZ_AND_C,      tm_conditional, QStringLiteral("IF_NZ_AND_C"));
    tn_add(t_IF_C_AND_NZ,      tm_conditional, QStringLiteral("IF_C_AND_NZ"));
    tn_add(t_IF_NZ,            tm_conditional, QStringLiteral("IF_NZ"));
    tn_add(t_IF_NE,            tm_conditional, QStringLiteral("IF_NE"));
    tn_add(t_IF_Z_NE_C,        tm_conditional, QStringLiteral("IF_Z_NE_C"));
    tn_add(t_IF_C_NE_Z,        tm_conditional, QStringLiteral("IF_C_NE_Z"));
    tn_add(t_IF_NZ_OR_NC,      tm_conditional, QStringLiteral("IF_NZ_OR_NC"));
    tn_add(t_IF_NC_OR_NZ,      tm_conditional, QStringLiteral("IF_NC_OR_NZ"));
    tn_add(t_IF_Z_AND_C,       tm_conditional, QStringLiteral("IF_Z_AND_C"));
    tn_add(t_IF_C_AND_Z,       tm_conditional, QStringLiteral("IF_C_AND_Z"));
    tn_add(t_IF_Z_EQ_C,        tm_conditional, QStringLiteral("IF_Z_EQ_C"));
    tn_add(t_IF_C_EQ_Z,        tm_conditional, QStringLiteral("IF_C_EQ_Z"));
    tn_add(t_IF_Z,             tm_conditional, QStringLiteral("IF_Z"));
    tn_add(t_IF_E,             tm_conditional, QStringLiteral("IF_E"));
    tn_add(t_IF_Z_OR_NC,       tm_conditional, QStringLiteral("IF_Z_OR_NC"));
    tn_add(t_IF_NC_OR_Z,       tm_conditional, QStringLiteral("IF_NC_OR_Z"));
    tn_add(t_IF_C,             tm_conditional, QStringLiteral("IF_C"));
    tn_add(t_IF_B,             tm_conditional, QStringLiteral("IF_B"));
    tn_add(t_IF_LT,            tm_conditional, QStringLiteral("IF_LT"));
    tn_add(t_IF_NZ_OR_C,       tm_conditional, QStringLiteral("IF_NZ_OR_C"));
    tn_add(t_IF_C_OR_NZ,       tm_conditional, QStringLiteral("IF_C_OR_NZ"));
    tn_add(t_IF_Z_OR_C,        tm_conditional, QStringLiteral("IF_Z_OR_C"));
    tn_add(t_IF_C_OR_Z,        tm_conditional, QStringLiteral("IF_C_OR_Z"));
    tn_add(t_IF_BE,            tm_conditional, QStringLiteral("IF_BE"));
    tn_add(t_IF_LE,            tm_conditional, QStringLiteral("IF_LE"));
    tn_add(t_IF_ALWAYS,        tm_conditional, QStringLiteral("IF_ALWAYS"));

    // MODCZ parameters
    tn_add(t_MODCZ__CLR,       tm_modcz_param, QStringLiteral("_CLR"));
    tn_add(t_MODCZ__NC_AND_NZ, tm_modcz_param, QStringLiteral("_NC_AND_NZ"));
    tn_add(t_MODCZ__NZ_AND_NC, tm_modcz_param, QStringLiteral("_NZ_AND_NC"));
    tn_add(t_MODCZ__GT,        tm_modcz_param, QStringLiteral("_GT"));
    tn_add(t_MODCZ__NC_AND_Z,  tm_modcz_param, QStringLiteral("_NC_AND_Z"));
    tn_add(t_MODCZ__Z_AND_NC,  tm_modcz_param, QStringLiteral("_Z_AND_NC"));
    tn_add(t_MODCZ__NC,        tm_modcz_param, QStringLiteral("_NC"));
    tn_add(t_MODCZ__GE,        tm_modcz_param, QStringLiteral("_GE"));
    tn_add(t_MODCZ__C_AND_NZ,  tm_modcz_param, QStringLiteral("_C_AND_NZ"));
    tn_add(t_MODCZ__NZ_AND_C,  tm_modcz_param, QStringLiteral("_NZ_AND_C"));
    tn_add(t_MODCZ__NZ,        tm_modcz_param, QStringLiteral("_NZ"));
    tn_add(t_MODCZ__NE,        tm_modcz_param, QStringLiteral("_NE"));
    tn_add(t_MODCZ__C_NE_Z,    tm_modcz_param, QStringLiteral("_C_NE_Z"));
    tn_add(t_MODCZ__Z_NE_C,    tm_modcz_param, QStringLiteral("_Z_NE_C"));
    tn_add(t_MODCZ__NC_OR_NZ,  tm_modcz_param, QStringLiteral("_NC_OR_NZ"));
    tn_add(t_MODCZ__NZ_OR_NC,  tm_modcz_param, QStringLiteral("_NZ_OR_NC"));
    tn_add(t_MODCZ__C_AND_Z,   tm_modcz_param, QStringLiteral("_C_AND_Z"));
    tn_add(t_MODCZ__Z_AND_C,   tm_modcz_param, QStringLiteral("_Z_AND_C"));
    tn_add(t_MODCZ__C_EQ_Z,    tm_modcz_param, QStringLiteral("_C_EQ_Z"));
    tn_add(t_MODCZ__Z_EQ_C,    tm_modcz_param, QStringLiteral("_Z_EQ_C"));
    tn_add(t_MODCZ__Z,         tm_modcz_param, QStringLiteral("_Z"));
    tn_add(t_MODCZ__E,         tm_modcz_param, QStringLiteral("_E"));
    tn_add(t_MODCZ__NC_OR_Z,   tm_modcz_param, QStringLiteral("_NC_OR_Z"));
    tn_add(t_MODCZ__Z_OR_NC,   tm_modcz_param, QStringLiteral("_Z_OR_NC"));
    tn_add(t_MODCZ__C,         tm_modcz_param, QStringLiteral("_C"));
    tn_add(t_MODCZ__LT,        tm_modcz_param, QStringLiteral("_LT"));
    tn_add(t_MODCZ__C_OR_NZ,   tm_modcz_param, QStringLiteral("_C_OR_NZ"));
    tn_add(t_MODCZ__NZ_OR_C,   tm_modcz_param, QStringLiteral("_NZ_OR_C"));
    tn_add(t_MODCZ__C_OR_Z,    tm_modcz_param, QStringLiteral("_C_OR_Z"));
    tn_add(t_MODCZ__Z_OR_C,    tm_modcz_param, QStringLiteral("_Z_OR_C"));
    tn_add(t_MODCZ__LE,        tm_modcz_param, QStringLiteral("_LE"));
    tn_add(t_MODCZ__SET,       tm_modcz_param, QStringLiteral("_SET"));

    // Assignment
    tn_add(t__ASSIGN,          tm_inst | tm_assignment,  QStringLiteral("="));

    // Current PC reference
    tn_add(t__DOLLAR,          tm_constant, QStringLiteral("$"));

    // Immedia value
    tn_add(t__IMMEDIATE2,      tm_immediate, QStringLiteral("##"));
    tn_add(t__IMMEDIATE,       tm_immediate, QStringLiteral("#"));
    tn_add(t__RELATIVE_HUB,    tm_relative, QStringLiteral("@@@"));
    tn_add(t__RELATIVE,        tm_relative, QStringLiteral("@"));

    // Sub expression in parens
    tn_add(t__LPAREN,          tm_parens, QStringLiteral("("));
    tn_add(t__RPAREN,          tm_parens, QStringLiteral(")"));
    tn_add(t__LBRACKET,        tm_parens, QStringLiteral("["));
    tn_add(t__RBRACKET,        tm_parens, QStringLiteral("]"));

    // Set the primary operators
    tn_add(t__INC,             tm_primary, QStringLiteral("++"));
    tn_add(t__DEC,             tm_primary, QStringLiteral("--"));

    // Set the unary operators
    tn_add(t__NEG,             tm_unary, QStringLiteral("!"));
    tn_add(t__NOT,             tm_unary, QStringLiteral("~"));

    // Set the multiplication operators
    tn_add(t__MUL,             tm_mulop, QStringLiteral("*"));
    tn_add(t__DIV,             tm_mulop, QStringLiteral("/"));
    tn_add(t__MOD,             tm_mulop, QStringLiteral("\\"));

    // Set the addition operators
    tn_add(t__PLUS,            tm_addop | tm_unary, QStringLiteral("+"));
    tn_add(t__MINUS,           tm_addop | tm_unary, QStringLiteral("-"));

    // Set the shift operators
    tn_add(t__SHL,             tm_shiftop, QStringLiteral("<<"));
    tn_add(t__SHR,             tm_shiftop, QStringLiteral(">>"));

    // Set the less/greater comparison operators
    tn_add(t__GE,              tm_relation, QStringLiteral(">="));
    tn_add(t__GT,              tm_relation, QStringLiteral(">"));
    tn_add(t__LE,              tm_relation, QStringLiteral("<="));
    tn_add(t__LT,              tm_relation, QStringLiteral("<"));

    // Set the equal/unequal comparison operators
    tn_add(t__EQ,              tm_equality, QStringLiteral("=="));
    tn_add(t__NE,              tm_equality, QStringLiteral("!="));

    // Set the binary operators
    tn_add(t__AND,             tm_binop_and, QStringLiteral("&"));
    tn_add(t__XOR,             tm_binop_xor, QStringLiteral("^"));
    tn_add(t__OR,              tm_binop_or,  QStringLiteral("|"));
    tn_add(t__REV,             tm_binop_rev, QStringLiteral("><"));

    // Set the logical operators
    tn_add(t__LOGAND,          tm_logop_and, QStringLiteral("&&"));
    tn_add(t__LOGOR,           tm_logop_or, QStringLiteral("||"));

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
            m_name_token.insertMulti(str, tok);

    // Build the reverse QHash for types lookup
    foreach(p2_token_e tok, m_token_type.keys())
        foreach(p2_t_mask_t mask, m_token_type.values(tok))
            m_type_token.insertMulti(mask, tok);

    m_ttype_name.insert(tt_none,            QStringLiteral("-"));
    m_ttype_name.insert(tt_parens,          QStringLiteral("Parenthesis"));
    m_ttype_name.insert(tt_primary,         QStringLiteral("Primary"));
    m_ttype_name.insert(tt_unary,           QStringLiteral("Unary"));
    m_ttype_name.insert(tt_mulop,           QStringLiteral("MulOp"));
    m_ttype_name.insert(tt_addop,           QStringLiteral("AddOp"));
    m_ttype_name.insert(tt_shiftop,         QStringLiteral("ShiftOp"));
    m_ttype_name.insert(tt_relation,        QStringLiteral("Relation"));
    m_ttype_name.insert(tt_equality,        QStringLiteral("Equality"));
    m_ttype_name.insert(tt_binop_and,       QStringLiteral("Binary AND"));
    m_ttype_name.insert(tt_binop_xor,       QStringLiteral("Binary XOR"));
    m_ttype_name.insert(tt_binop_or,        QStringLiteral("Binary OR"));
    m_ttype_name.insert(tt_binop_rev,       QStringLiteral("Binary REV"));
    m_ttype_name.insert(tt_logop_and,       QStringLiteral("Logic AND"));
    m_ttype_name.insert(tt_logop_or,        QStringLiteral("Logic OR"));
    m_ttype_name.insert(tt_ternary,         QStringLiteral("Ternary"));
    m_ttype_name.insert(tt_assignment,      QStringLiteral("Assignment"));
    m_ttype_name.insert(tt_delimiter,       QStringLiteral("Delimiter"));
    m_ttype_name.insert(tt_constant,        QStringLiteral("Constant"));
    m_ttype_name.insert(tt_immediate,       QStringLiteral("Immediate"));
    m_ttype_name.insert(tt_relative,        QStringLiteral("Relative"));
    m_ttype_name.insert(tt_conditional,     QStringLiteral("Conditional"));
    m_ttype_name.insert(tt_modcz_param,     QStringLiteral("MODCZ param"));
    m_ttype_name.insert(tt_inst,            QStringLiteral("Instruction"));
    m_ttype_name.insert(tt_wcz_suffix,      QStringLiteral("WC/WZ suffix"));
    m_ttype_name.insert(tt_section,         QStringLiteral("Section"));
    m_ttype_name.insert(tt_origin,          QStringLiteral("Origin"));
    m_ttype_name.insert(tt_data,            QStringLiteral("Data"));
    m_ttype_name.insert(tt_lexer,           QStringLiteral("Lexer"));
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
 * @brief Tokenize a string and append to P2AsmWords
 * @param pos starting position in string
 * @param str string to tokenize
 * @param words vector of P2AsmWord to append to
 * @return true on success, or false on error
 */
P2Words P2Token::tokenize(const QString& str, int& in_curly) const
{
    static QRegExp rx;
    static QString re;
    P2Words words;
    p2_token_e tok;
    int curly = 0;
    int pos = 0, lastpos = -1;

    DEBUG_REGEX("%s: ***** tokenize *****", __func__);


    if (rx.isEmpty()) {
        // first time initialization of the QRegExp
        QStringList list;

        list += QStringLiteral("'.*");                  // Comments '... something
        list += QStringLiteral("\\s+");                 // Spaces
        list += re_str_const;                           // Strings with possibly escaped double quotes
        list += re_comment;                             // Comments in curly braces
        list += QStringLiteral("[^,'\"\\s]+");          // Any word without , ' " Space
        static const char *Delimiters[] = {
            "@@@", "##", "#", ",", "[", "]", "++", "+", "--", "-", "<<",
            "<", ">>", "><", ">|", ">", "*", "/", "\\", "&", "|<", "|", "(", ")",
             "@", "==", "=", ";", "{", "}",
            nullptr
        };
        for (int i = 0; Delimiters[i]; i++)
            list += QRegExp::escape(QString::fromLatin1(Delimiters[i])) + QStringLiteral("{1}");
        re = QString("(%1)").arg(list.join(QChar('|')));

        // rx.setPatternSyntax(QRegExp::RegExp2);  // greedy syntax
        rx.setPattern(re);
        Q_ASSERT(rx.isValid());
    }

    if (in_curly > 0) {

        curly += str.count(chr_lcurly);
        curly -= str.count(chr_rcurly);

        if (in_curly > 0 || curly > 0) {
            int tlen = str.length() - pos;
            tok = t_comment;
            words.append(P2Word(tok, str, pos, pos + tlen - 1));
            pos += tlen;
            DEBUG_REGEX("%s: match {%d} @%-3d #%-3d %s", __func__, curly, pos, tlen, qPrintable(str));
        }

        in_curly += curly;
        return words;
    }

    while ((pos = rx.indexIn(str, pos)) != -1) {
        QString word = rx.cap(1);
        int len = rx.matchedLength();

        word = word.trimmed();
        if (word.isEmpty()) {
            if (len > 0) {
                pos += len;
                continue;
            }
            DEBUG_REGEX("%s: match {%d} @%-3d #%-3d %s", __func__, curly, pos, len, qPrintable(word));
            break;
        }

        if (!word.startsWith(chr_dquote)) {
            curly += word.count(chr_lcurly);
            curly -= word.count(chr_rcurly);
        }

        if (in_curly > 0 || curly > 0) {
            len = str.length() - pos;
            tok = t_comment;
            words.append(P2Word(tok, word, pos, pos + len - 1));
            pos += len;
            DEBUG_REGEX("%s: match {%d} @%-3d #%-3d %s", __func__, curly, pos, len, qPrintable(word));
            in_curly += curly;
            return words;
        }

        DEBUG_REGEX("%s: match {%d} @%-3d #%-3d '%s'", __func__, curly, pos, len, qPrintable(rx.cap(1)));
        int tlen = 0;
        tok = token(word, true, &tlen);
        words.append(P2Word(tok, word, pos, pos + tlen - 1));
        pos += tlen;
        if (pos == lastpos) {
            DEBUG_REGEX("%s: break {%d} @%-3d #%-3d %s", __func__, curly, pos, len, qPrintable(word));
            break;
        } else {
            lastpos = pos;
        }
    }

    for (int i = 0; i < words.count(); i++) {
        const P2Word& word = words[i];
        tok = word.tok();
        if (t_unknown == tok)
            words[i].set_token(token(word.str()));
        DEBUG_TOKEN("%s: word[%-3d] {%-20s} @%-3d #%-3d %s", __func__,
                        i, qPrintable(string(tok)),
                        word.pos(), word.len(), qPrintable(word.str()));
        pos = word.end() + 1;
    }

    in_curly += curly;
    return words;
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
    QString ustr = str.toUpper();
    p2_token_e tok = t_unknown;

    for (;;) {
        if (ustr.isEmpty())
            break;

        const QList<p2_token_e>& tokens = m_name_token.values(ustr);
        tok = tokens.value(0, t_unknown);
        if (t_unknown != tok) {
            QStringList names = m_token_name.values(tok);
            ustr.truncate(names[0].length());
            break;
        }

        QRegExp rx_locsym(re_locsym, Qt::CaseInsensitive);
        if (rx_locsym.exactMatch(ustr)) {
            tok = t_locsym;
            break;
        }

        QRegExp rx_comment(re_comment, Qt::CaseInsensitive);
        if (rx_comment.exactMatch(ustr)) {
            tok = t_comment;
            break;
        }

        QRegExp rx_symbol(re_symbol, Qt::CaseInsensitive);
        if (rx_symbol.exactMatch(ustr)) {
            tok = t_symbol;
            break;
        }

        QRegExp rx_bin_const(re_bin_const, Qt::CaseInsensitive);
        if (rx_bin_const.exactMatch(ustr)) {
            tok = t_bin_const;
            break;
        }

        QRegExp rx_byt_const(re_byt_const, Qt::CaseInsensitive);
        if (rx_byt_const.exactMatch(ustr)) {
            tok = t_byt_const;
            break;
        }

        QRegExp rx_oct_const(re_oct_const, Qt::CaseInsensitive);
        if (rx_oct_const.exactMatch(ustr)) {
            tok = t_oct_const;
            break;
        }

        QRegExp rx_hex_const(re_hex_const, Qt::CaseInsensitive);
        if (rx_hex_const.exactMatch(ustr)) {
            tok = t_hex_const;
            break;
        }

        QRegExp rx_dec_const(re_dec_const, Qt::CaseInsensitive);
        if (rx_dec_const.exactMatch(ustr)) {
            tok = t_dec_const;
            break;
        }

        QRegExp rx_str_const(re_str_const, Qt::CaseInsensitive);
        if (rx_str_const.exactMatch(ustr)) {
            tok = t_string;
            break;
        }

        if (!chop)
            break;

        // chop off one character
        ustr.chop(1);
    }

    if (plen)
        *plen = ustr.length();

    return tok;
}

/**
 * @brief Check if a token is one of a type mask
 * @param tok token value
 * @param typemask token type bit mask
 * @return true if token type is set, or false otherwise
 */
bool P2Token::is_type(p2_token_e tok, p2_t_mask_t typemask) const
{
    const p2_t_mask_t bits = m_token_type.value(tok, 0);
    return (bits & typemask) ? true : false;
}

/**
 * @brief Check if a token is of a specific type
 * @param tok token value
 * @param type token type
 * @return true if token type is set, or false otherwise
 */
bool P2Token::is_type(p2_token_e tok, p2_t_type_e type) const
{
    const p2_t_mask_t bits = m_token_type.value(tok, 0);
    const p2_t_mask_t mask = TTMASK(type);
    return (bits & mask) ? true : false;
}

/**
 * @brief Check if a string is a token of a specific type
 * @param str string to tokenize
 * @param type token type
 * @return true if token type is set, or false otherwise
 */
bool P2Token::is_type(const QString& str, p2_t_type_e type) const
{
    const p2_token_e tok = m_name_token.value(str);
    return is_type(tok, type);
}


/**
 * @brief Return the list of type names which are set in the type mask
 * @param typemask bit mask of token types
 * @return QStringList with the type names
 */
QStringList P2Token::type_names(p2_t_mask_t typemask) const
{
    QStringList list;
    for (int i = 0; i <= 64 && typemask != 0; i++, typemask >>= 1)
        if (typemask & 1)
                list += m_ttype_name.value(static_cast<p2_t_type_e>(i));
    if (list.isEmpty())
        list += m_ttype_name.value(static_cast<p2_t_type_e>(tt_none));
    return list;
}

/**
 * @brief Return the list of type names which are set for a token
 * @param tok token value
 * @return QStringList with the type names
 */
QStringList P2Token::type_names(p2_token_e tok) const
{
    const p2_t_mask_t typemask = m_token_type.value(tok, 0);
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
    const p2_t_mask_t mask = m_token_type.value(tok, 0);
    return (mask & tm_operations) ? true : false;
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
p2_token_e P2Token::at_type(int& pos, const QString& str, p2_t_mask_t typemask, p2_token_e dflt) const
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
p2_token_e P2Token::at_type(int& pos, const QString& str, p2_t_type_e type, p2_token_e dflt) const
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
p2_token_e P2Token::at_type(const QString& str, p2_t_type_e type, p2_token_e dflt) const
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

p2_token_e P2Token::at_types(int& pos, const QString& str, p2_t_mask_t typemask, p2_token_e dflt) const
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
p2_token_e P2Token::at_types(int& pos, const QString& str, const QList<p2_t_type_e>& types, p2_token_e dflt) const
{
    // Build bit mask for types
    p2_t_mask_t typemask = tm_none;
    foreach(const p2_t_type_e type, types)
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
p2_token_e P2Token::at_types(const QString& str, const QList<p2_t_type_e>& types, p2_token_e dflt) const
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
    return at_modcz_param(pos, str, dflt);

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
 * @brief Set bits of typemask in the token %tok hash entry
 * @param tok token value
 * @param typemask bit mask with type(s) to set
 */
void P2Token::tt_set(p2_token_e tok, p2_t_mask_t typemask)
{
    p2_t_mask_t mask = m_token_type.value(tok, 0) | typemask;
    m_token_type.insert(tok, mask);
}

/**
 * @brief Clear bits of typemask in the token %tok hash entry
 * @param tok token value
 * @param typemask bit mask with type(s) to clear
 */
void P2Token::tt_clr(p2_token_e tok, p2_t_mask_t typemask)
{
    p2_t_mask_t mask = m_token_type.value(tok, 0) & ~typemask;
    m_token_type.insert(tok, mask);
}

/**
 * @brief Check bits of typemask in the token %tok hash entry
 * @param tok token value
 * @param typemask bit mask with type(s) to check for
 * @return true if any bit of typemask is set, or false otherwise
 */
bool P2Token::tt_chk(p2_token_e tok, p2_t_mask_t typemask) const
{
    return m_token_type.value(tok, 0) & typemask ? true : false;
}

/**
 * @brief Add a token to the m_token_nam hash and its types to the typemask
 * @param tok token value
 * @param typemask token type bitmask
 * @param str string
 */
void P2Token::tn_add(p2_token_e tok, p2_t_mask_t typemask, const QString& str)
{
    m_token_name.insert(tok, str);
    tt_set(tok, typemask);
}
