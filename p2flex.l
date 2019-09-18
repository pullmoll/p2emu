%{
#include "p2token.h"
#include "p2word.h"

static int yycolumn = 0;
static int curly = 0;

#define isatty(x) (0)

%}

%option 8bit
%option case-insensitive
%option batch
/* %option noyywrap */
%option nounistd
%option nodefault
%option yylineno

%s CURLY

STRCONST      \"[^\"]*\"
BINCONST    "%"[_]?[01_]+
BYTCONST    "%%"[_]*[0-3_]+
DECCONST    [0-9]+[0-9_]*
REALCONST   [0-9]+[0-9_]*"."[0-9_]*
HEXCONST    "$"[0-9A-F_]+
SYMBOL      [A-Z_]?[A-Z0-9_]+
LOCSYM      "."[A-Z_]?[A-Z0-9_]+

%%
<CURLY>[\n]             return t_EOL;
<CURLY>[ \t]+           return t_none;
<CURLY>"'"[^\n]+        return t_comment_eol;
<CURLY>[ \t]*{STRCONST} return t_comment_eol;
<CURLY>[^\n{}]+         return t_comment_curly;
<CURLY>"}"+             {
                            for (int i = 0; i < yyleng; i++)
                                if ('}' == yytext[i])
                                    --curly;
                            if (0 == curly)
                                BEGIN(INITIAL);
                            return t_comment_eol;
                        }
[ \t]+                  return t_none;
[\n]			return t_EOL;
"{"[^\n}]*		{
                            if (0 == curly)
                                BEGIN(CURLY);
                            for (int i = 0; i < yyleng; i++)
                                if ('{' == yytext[i])
                                    curly++;
                            return t_comment_lcurly;
                        }
"ABS"                   return t_ABS;
"ADD"                   return t_ADD;
"ADDCT1"		return t_ADDCT1;
"ADDCT2"		return t_ADDCT2;
"ADDCT3"		return t_ADDCT3;
"ADDPIX"		return t_ADDPIX;
"ADDS"                  return t_ADDS;
"ADDSX"                 return t_ADDSX;
"ADDX"                  return t_ADDX;
"AKPIN"                 return t_AKPIN;
"ALLOWI"		return t_ALLOWI;
"ALTB"                  return t_ALTB;
"ALTD"                  return t_ALTD;
"ALTGB"                 return t_ALTGB;
"ALTGN"                 return t_ALTGN;
"ALTGW"                 return t_ALTGW;
"ALTI"                  return t_ALTI;
"ALTR"                  return t_ALTR;
"ALTS"                  return t_ALTS;
"ALTSB"                 return t_ALTSB;
"ALTSN"                 return t_ALTSN;
"ALTSW"                 return t_ALTSW;
"AND"                   return t_AND;
"ANDN"                  return t_ANDN;
"AUGD"                  return t_AUGD;
"AUGS"                  return t_AUGS;
"BITC"                  return t_BITC;
"BITH"                  return t_BITH;
"BITL"                  return t_BITL;
"BITNC"                 return t_BITNC;
"BITNOT"		return t_BITNOT;
"BITNZ"                 return t_BITNZ;
"BITRND"		return t_BITRND;
"BITZ"                  return t_BITZ;
"BLNPIX"		return t_BLNPIX;
"BMASK"                 return t_BMASK;
"BRK"                   return t_BRK;
"CALL"                  return t_CALL;
"CALLA"                 return t_CALLA;
"CALLB"                 return t_CALLB;
"CALLD"                 return t_CALLD;
"CALLPA"		return t_CALLPA;
"CALLPB"		return t_CALLPB;
"CMP"                   return t_CMP;
"CMPM"                  return t_CMPM;
"CMPR"                  return t_CMPR;
"CMPS"                  return t_CMPS;
"CMPSUB"		return t_CMPSUB;
"CMPSX"                 return t_CMPSX;
"CMPX"                  return t_CMPX;
"COGATN"		return t_COGATN;
"COGBRK"		return t_COGBRK;
"COGID"                 return t_COGID;
"COGINIT"		return t_COGINIT;
"COGSTOP"		return t_COGSTOP;
"CRCBIT"		return t_CRCBIT;
"CRCNIB"		return t_CRCNIB;
"DECMOD"		return t_DECMOD;
"DECOD"                 return t_DECOD;
"DIRC"                  return t_DIRC;
"DIRH"                  return t_DIRH;
"DIRL"                  return t_DIRL;
"DIRNC"                 return t_DIRNC;
"DIRNOT"		return t_DIRNOT;
"DIRNZ"                 return t_DIRNZ;
"DIRRND"		return t_DIRRND;
"DIRZ"                  return t_DIRZ;
"DJF"                   return t_DJF;
"DJNF"                  return t_DJNF;
"DJNZ"                  return t_DJNZ;
"DJZ"                   return t_DJZ;
"DRVC"                  return t_DRVC;
"DRVH"                  return t_DRVH;
"DRVL"                  return t_DRVL;
"DRVNC"                 return t_DRVNC;
"DRVNOT"		return t_DRVNOT;
"DRVNZ"                 return t_DRVNZ;
"DRVRND"		return t_DRVRND;
"DRVZ"                  return t_DRVZ;
"ENCOD"                 return t_ENCOD;
"EXECF"                 return t_EXECF;
"FBLOCK"		return t_FBLOCK;
"FGE"                   return t_FGE;
"FGES"                  return t_FGES;
"FLE"                   return t_FLE;
"FLES"                  return t_FLES;
"FLTC"                  return t_FLTC;
"FLTH"                  return t_FLTH;
"FLTL"                  return t_FLTL;
"FLTNC"                 return t_FLTNC;
"FLTNOT"		return t_FLTNOT;
"FLTNZ"                 return t_FLTNZ;
"FLTRND"		return t_FLTRND;
"FLTZ"                  return t_FLTZ;
"GETBRK"		return t_GETBRK;
"GETBYTE"           	return t_GETBYTE;
"GETCT"                 return t_GETCT;
"GETNIB"		return t_GETNIB;
"GETPTR"		return t_GETPTR;
"GETQX"                 return t_GETQX;
"GETQY"                 return t_GETQY;
"GETRND"		return t_GETRND;
"GETSCP"		return t_GETSCP;
"GETWORD"		return t_GETWORD;
"GETXACC"		return t_GETXACC;
"HUBSET"		return t_HUBSET;
"IJNZ"                  return t_IJNZ;
"IJZ"                   return t_IJZ;
"INCMOD"		return t_INCMOD;
"JATN"                  return t_JATN;
"JCT1"                  return t_JCT1;
"JCT2"                  return t_JCT2;
"JCT3"                  return t_JCT3;
"JFBW"                  return t_JFBW;
"JINT"                  return t_JINT;
"JMP"                   return t_JMP;
"JMPREL"            	return t_JMPREL;
"JNATN"                 return t_JNATN;
"JNCT1"                 return t_JNCT1;
"JNCT2"                 return t_JNCT2;
"JNCT3"                 return t_JNCT3;
"JNFBW"                 return t_JNFBW;
"JNINT"                 return t_JNINT;
"JNPAT"                 return t_JNPAT;
"JNQMT"                 return t_JNQMT;
"JNSE1"                 return t_JNSE1;
"JNSE2"                 return t_JNSE2;
"JNSE3"                 return t_JNSE3;
"JNSE4"                 return t_JNSE4;
"JNXFI"                 return t_JNXFI;
"JNXMT"                 return t_JNXMT;
"JNXRL"                 return t_JNXRL;
"JNXRO"                 return t_JNXRO;
"JPAT"                  return t_JPAT;
"JQMT"                  return t_JQMT;
"JSE1"                  return t_JSE1;
"JSE2"                  return t_JSE2;
"JSE3"                  return t_JSE3;
"JSE4"                  return t_JSE4;
"JXFI"                  return t_JXFI;
"JXMT"                  return t_JXMT;
"JXRL"                  return t_JXRL;
"JXRO"                  return t_JXRO;
"LOC"                   return t_LOC;
"LOCKNEW"		return t_LOCKNEW;
"LOCKREL"		return t_LOCKREL;
"LOCKRET"		return t_LOCKRET;
"LOCKTRY"		return t_LOCKTRY;
"MERGEB"		return t_MERGEB;
"MERGEW"		return t_MERGEW;
"MIXPIX"		return t_MIXPIX;
"MODCZ"                 return t_MODCZ;
"MODC"                  return t_MODC;
"MODZ"                  return t_MODZ;
"MOV"                   return t_MOV;
"MOVBYTS"		return t_MOVBYTS;
"MUL"                   return t_MUL;
"MULPIX"		return t_MULPIX;
"MULS"                  return t_MULS;
"MUXC"                  return t_MUXC;
"MUXNC"                 return t_MUXNC;
"MUXNIBS"           	return t_MUXNIBS;
"MUXNITS"		return t_MUXNITS;
"MUXNZ"                 return t_MUXNZ;
"MUXQ"                  return t_MUXQ;
"MUXZ"                  return t_MUXZ;
"NEG"                   return t_NEG;
"NEGC"                  return t_NEGC;
"NEGNC"                 return t_NEGNC;
"NEGNZ"                 return t_NEGNZ;
"NEGZ"                  return t_NEGZ;
"NIXINT1"		return t_NIXINT1;
"NIXINT2"		return t_NIXINT2;
"NIXINT3"		return t_NIXINT3;
"NOP"                   return t_NOP;
"NOT"                   return t_NOT;
"ONES"                  return t_ONES;
"OR"                    return t_OR;
"OUTC"                  return t_OUTC;
"OUTH"                  return t_OUTH;
"OUTL"                  return t_OUTL;
"OUTNC"                 return t_OUTNC;
"OUTNOT"		return t_OUTNOT;
"OUTNZ"                 return t_OUTNZ;
"OUTRND"		return t_OUTRND;
"OUTZ"                  return t_OUTZ;
"POLLATN"		return t_POLLATN;
"POLLCT1"		return t_POLLCT1;
"POLLCT2"		return t_POLLCT2;
"POLLCT3"		return t_POLLCT3;
"POLLFBW"		return t_POLLFBW;
"POLLINT"		return t_POLLINT;
"POLLPAT"		return t_POLLPAT;
"POLLQMT"		return t_POLLQMT;
"POLLSE1"		return t_POLLSE1;
"POLLSE2"		return t_POLLSE2;
"POLLSE3"		return t_POLLSE3;
"POLLSE4"		return t_POLLSE4;
"POLLXFI"		return t_POLLXFI;
"POLLXMT"		return t_POLLXMT;
"POLLXRL"		return t_POLLXRL;
"POLLXRO"		return t_POLLXRO;
"POP"                   return t_POP;
"POPA"                  return t_POPA;
"POPB"                  return t_POPB;
"PUSH"                  return t_PUSH;
"PUSHA"                 return t_PUSHA;
"PUSHB"                 return t_PUSHB;
"QDIV"                  return t_QDIV;
"QEXP"                  return t_QEXP;
"QFRAC"                 return t_QFRAC;
"QLOG"                  return t_QLOG;
"QMUL"                  return t_QMUL;
"QROTATE"		return t_QROTATE;
"QSQRT"                 return t_QSQRT;
"QVECTOR"		return t_QVECTOR;
"RCL"                   return t_RCL;
"RCR"                   return t_RCR;
"RCZL"                  return t_RCZL;
"RCZR"                  return t_RCZR;
"RDBYTE"		return t_RDBYTE;
"RDFAST"		return t_RDFAST;
"RDLONG"		return t_RDLONG;
"RDLUT"                 return t_RDLUT;
"RDPIN"                 return t_RDPIN;
"RDWORD"		return t_RDWORD;
"REP"                   return t_REP;
"RESI0"                 return t_RESI0;
"RESI1"                 return t_RESI1;
"RESI2"                 return t_RESI2;
"RESI3"                 return t_RESI3;
"RET"                   return t_RET;
"RETA"                  return t_RETA;
"RETB"                  return t_RETB;
"RETI0"                 return t_RETI0;
"RETI1"                 return t_RETI1;
"RETI2"                 return t_RETI2;
"RETI3"                 return t_RETI3;
"REV"                   return t_REV;
"RFBYTE"		return t_RFBYTE;
"RFLONG"		return t_RFLONG;
"RFVAR"                 return t_RFVAR;
"RFVARS"		return t_RFVARS;
"RFWORD"		return t_RFWORD;
"RGBEXP"		return t_RGBEXP;
"RGBSQZ"		return t_RGBSQZ;
"ROL"                   return t_ROL;
"ROLBYTE"		return t_ROLBYTE;
"ROLNIB"		return t_ROLNIB;
"ROLWORD"		return t_ROLWORD;
"ROR"                   return t_ROR;
"RQPIN"                 return t_RQPIN;
"SAL"                   return t_SAL;
"SAR"                   return t_SAR;
"SCA"                   return t_SCA;
"SCAS"                  return t_SCAS;
"SETBYTE"		return t_SETBYTE;
"SETCFRQ"		return t_SETCFRQ;
"SETCI"                 return t_SETCI;
"SETCMOD"		return t_SETCMOD;
"SETCQ"                 return t_SETCQ;
"SETCY"                 return t_SETCY;
"SETD"                  return t_SETD;
"SETDACS"		return t_SETDACS;
"SETINT1"		return t_SETINT1;
"SETINT2"		return t_SETINT2;
"SETINT3"		return t_SETINT3;
"SETLUTS"		return t_SETLUTS;
"SETNIB"		return t_SETNIB;
"SETPAT"		return t_SETPAT;
"SETPIV"		return t_SETPIV;
"SETPIX"		return t_SETPIX;
"SETQ"                  return t_SETQ;
"SETQ2"                 return t_SETQ2;
"SETR"                  return t_SETR;
"SETS"                  return t_SETS;
"SETSCP"		return t_SETSCP;
"SETSE1"		return t_SETSE1;
"SETSE2"		return t_SETSE2;
"SETSE3"		return t_SETSE3;
"SETSE4"		return t_SETSE4;
"SETWORD"		return t_SETWORD;
"SETXFRQ"		return t_SETXFRQ;
"SEUSSF"		return t_SEUSSF;
"SEUSSR"		return t_SEUSSR;
"SHL"                   return t_SHL;
"SHR"                   return t_SHR;
"SIGNX"                 return t_SIGNX;
"SKIP"                  return t_SKIP;
"SKIPF"                 return t_SKIPF;
"SPLITB"		return t_SPLITB;
"SPLITW"		return t_SPLITW;
"STALLI"		return t_STALLI;
"SUB"                   return t_SUB;
"SUBR"                  return t_SUBR;
"SUBS"                  return t_SUBS;
"SUBSX"                 return t_SUBSX;
"SUBX"                  return t_SUBX;
"SUMC"                  return t_SUMC;
"SUMNC"                 return t_SUMNC;
"SUMNZ"                 return t_SUMNZ;
"SUMZ"                  return t_SUMZ;
"TEST"                  return t_TEST;
"TESTB"                 return t_TESTB;
"TESTBN"		return t_TESTBN;
"TESTN"                 return t_TESTN;
"TESTP"                 return t_TESTP;
"TESTPN"		return t_TESTPN;
"TJF"                   return t_TJF;
"TJNF"                  return t_TJNF;
"TJNS"                  return t_TJNS;
"TJNZ"                  return t_TJNZ;
"TJS"                   return t_TJS;
"TJV"                   return t_TJV;
"TJZ"                   return t_TJZ;
"TRGINT1"		return t_TRGINT1;
"TRGINT2"		return t_TRGINT2;
"TRGINT3"		return t_TRGINT3;
"WAITATN"		return t_WAITATN;
"WAITCT1"		return t_WAITCT1;
"WAITCT2"		return t_WAITCT2;
"WAITCT3"		return t_WAITCT3;
"WAITFBW"		return t_WAITFBW;
"WAITINT"		return t_WAITINT;
"WAITPAT"		return t_WAITPAT;
"WAITSE1"		return t_WAITSE1;
"WAITSE2"		return t_WAITSE2;
"WAITSE3"		return t_WAITSE3;
"WAITSE4"		return t_WAITSE4;
"WAITX"                 return t_WAITX;
"WAITXFI"		return t_WAITXFI;
"WAITXMT"		return t_WAITXMT;
"WAITXRL"		return t_WAITXRL;
"WAITXRO"		return t_WAITXRO;
"WFBYTE"		return t_WFBYTE;
"WFLONG"		return t_WFLONG;
"WFWORD"		return t_WFWORD;
"WMLONG"		return t_WMLONG;
"WRBYTE"		return t_WRBYTE;
"WRC"                   return t_WRC;
"WRFAST"		return t_WRFAST;
"WRLONG"		return t_WRLONG;
"WRLUT"                 return t_WRLUT;
"WRNC"                  return t_WRNC;
"WRNZ"                  return t_WRNZ;
"WRPIN"                 return t_WRPIN;
"WRWORD"		return t_WRWORD;
"WRZ"                   return t_WRZ;
"WXPIN"                 return t_WXPIN;
"WYPIN"                 return t_WYPIN;
"XCONT"                 return t_XCONT;
"XINIT"                 return t_XINIT;
"XOR"                   return t_XOR;
"XORO32"		return t_XORO32;
"XSTOP"                 return t_XSTOP;
"XZERO"                 return t_XZERO;
"ZEROX"                 return t_ZEROX;

"WC"                    return t_WC;
"WZ"                    return t_WZ;
"WCZ"                   return t_WCZ;
"ANDC"                  return t_ANDC;
"ANDZ"                  return t_ANDZ;
"ORC"                   return t_ORC;
"ORZ"                   return t_ORZ;
"XORC"                  return t_XORC;
"XORZ"                  return t_XORZ;

"DAT"                   return t_DAT;
"CON"                   return t_CON;
"PUB"                   return t_PUB;
"PRI"                   return t_PRI;
"VAR"                   return t_VAR;

"ORG"                   return t_ORG;
"ORGF"                  return t_ORGF;
"ORGH"                  return t_ORGH;
"ALIGNW"		return t_ALIGNW;
"ALIGNL"		return t_ALIGNL;
"FIT"                   return t_FIT;

"BYTE"                  return t_BYTE;
"WORD"                  return t_WORD;
"LONG"                  return t_LONG;
"RES"                   return t_RES;
"FILE"                  return t_FILE;

"_RET_"                 return t__RET_;
"IF_NC_AND_NZ"		return t_IF_NC_AND_NZ;
"IF_NZ_AND_NC"		return t_IF_NZ_AND_NC;
"IF_A"                  return t_IF_A;
"IF_GT"                 return t_IF_GT;
"IF_00"                 return t_IF_00;
"IF_NC_AND_Z"		return t_IF_NC_AND_Z;
"IF_Z_AND_NC"		return t_IF_Z_AND_NC;
"IF_01"                 return t_IF_01;
"IF_NC"                 return t_IF_NC;
"IF_AE"                 return t_IF_AE;
"IF_GE"                 return t_IF_GE;
"IF_0X"                 return t_IF_0X;
"IF_C_AND_NZ"		return t_IF_C_AND_NZ;
"IF_NZ_AND_C"		return t_IF_NZ_AND_C;
"IF_10"                 return t_IF_10;
"IF_NZ"                 return t_IF_NZ;
"IF_NE"                 return t_IF_NE;
"IF_X0"                 return t_IF_X0;
"IF_C_NE_Z"		return t_IF_C_NE_Z;
"IF_Z_NE_C"		return t_IF_Z_NE_C;
"IF_DIFF"		return t_IF_DIFF;
"IF_NC_OR_NZ"		return t_IF_NC_OR_NZ;
"IF_NZ_OR_NC"		return t_IF_NZ_OR_NC;
"IF_NOT_11"		return t_IF_NOT_11;
"IF_C_AND_Z"		return t_IF_C_AND_Z;
"IF_Z_AND_C"		return t_IF_Z_AND_C;
"IF_11"                 return t_IF_11;
"IF_C_EQ_Z"		return t_IF_C_EQ_Z;
"IF_Z_EQ_C"		return t_IF_Z_EQ_C;
"IF_SAME"		return t_IF_SAME;
"IF_Z"                  return t_IF_Z;
"IF_E"                  return t_IF_E;
"IF_X1"                 return t_IF_X1;
"IF_NC_OR_Z"		return t_IF_NC_OR_Z;
"IF_Z_OR_NC"		return t_IF_Z_OR_NC;
"IF_BE"                 return t_IF_BE;
"IF_NOT_10"		return t_IF_NOT_10;
"IF_C"                  return t_IF_C;
"IF_B"                  return t_IF_B;
"IF_LT"                 return t_IF_LT;
"IF_1X"                 return t_IF_1X;
"IF_C_OR_NZ"		return t_IF_C_OR_NZ;
"IF_NZ_OR_C"		return t_IF_NZ_OR_C;
"IF_NOT_01"		return t_IF_NOT_01;
"IF_C_OR_Z"		return t_IF_C_OR_Z;
"IF_Z_OR_C"		return t_IF_Z_OR_C;
"IF_LE"                 return t_IF_LE;
"IF_NOT_00"		return t_IF_NOT_00;
"IF_ALWAYS"		return t_IF_ALWAYS;

"_CLR"                  return t_MODCZ__CLR;
"_NC_AND_NZ"		return t_MODCZ__NC_AND_NZ;
"_NZ_AND_NC"		return t_MODCZ__NZ_AND_NC;
"_A"                    return t_MODCZ__A;
"_GT"                   return t_MODCZ__GT;
"_00"                   return t_MODCZ__00;
"_NC_AND_Z"		return t_MODCZ__NC_AND_Z;
"_Z_AND_NC"		return t_MODCZ__Z_AND_NC;
"_01"                   return t_MODCZ__01;
"_NC"                   return t_MODCZ__NC;
"_AE"                   return t_MODCZ__AE;
"_GE"                   return t_MODCZ__GE;
"_0X"                   return t_MODCZ__0X;
"_C_AND_NZ"		return t_MODCZ__C_AND_NZ;
"_NZ_AND_C"		return t_MODCZ__NZ_AND_C;
"_10"                   return t_MODCZ__10;
"_NZ"                   return t_MODCZ__NZ;
"_NE"                   return t_MODCZ__NE;
"_X0"                   return t_MODCZ__X0;
"_C_NE_Z"           	return t_MODCZ__C_NE_Z;
"_Z_NE_C"		return t_MODCZ__Z_NE_C;
"_DIFF"                 return t_MODCZ__DIFF;
"_NC_OR_NZ"		return t_MODCZ__NC_OR_NZ;
"_NZ_OR_NC"		return t_MODCZ__NZ_OR_NC;
"_NOT_11"		return t_MODCZ__NOT_11;
"_C_AND_Z"		return t_MODCZ__C_AND_Z;
"_Z_AND_C"		return t_MODCZ__Z_AND_C;
"_11"                   return t_MODCZ__11;
"_C_EQ_Z"		return t_MODCZ__C_EQ_Z;
"_Z_EQ_C"		return t_MODCZ__Z_EQ_C;
"_SAME"                 return t_MODCZ__SAME;
"_Z"                    return t_MODCZ__Z;
"_E"                    return t_MODCZ__E;
"_X1"                   return t_MODCZ__X1;
"_NC_OR_Z"		return t_MODCZ__NC_OR_Z;
"_Z_OR_NC"		return t_MODCZ__Z_OR_NC;
"_BE"                   return t_MODCZ__BE;
"_NOT_10"		return t_MODCZ__NOT_10;
"_C"                    return t_MODCZ__C;
"_B"                    return t_MODCZ__B;
"_LT"                   return t_MODCZ__LT;
"_1X"                   return t_MODCZ__1X;
"_C_OR_NZ"		return t_MODCZ__C_OR_NZ;
"_NZ_OR_C"		return t_MODCZ__NZ_OR_C;
"_NOT_01"		return t_MODCZ__NOT_01;
"_C_OR_Z"		return t_MODCZ__C_OR_Z;
"_Z_OR_C"		return t_MODCZ__Z_OR_C;
"_LE"                   return t_MODCZ__LE;
"_NOT_00"		return t_MODCZ__NOT_00;
"_SET"                  return t_MODCZ__SET;

"="			return t_ASSIGN;
"IJMP3"                 return t_IJMP3;
"IRET3"                 return t_IRET3;
"IJMP2"                 return t_IJMP2;
"IRET2"                 return t_IRET2;
"IJMP1"                 return t_IJMP1;
"IRET1"                 return t_IRET1;
"PA"                    return t_PA;
"PB"                    return t_PB;
"PTRA"                  return t_PTRA;
"--PTRA"		return t_PTRA_predec;
"--PTRB"		return t_PTRB_predec;
"PTRA--"		return t_PTRA_postdec;
"PTRB--"		return t_PTRB_postdec;
"PTRB"                  return t_PTRB;
"++PTRA"		return t_PTRA_preinc;
"++PTRB"		return t_PTRB_preinc;
"PTRA++"		return t_PTRA_postinc;
"PTRB++"		return t_PTRB_postinc;
"DIRA"                  return t_DIRA;
"DIRB"                  return t_DIRB;
"OUTA"                  return t_OUTA;
"OUTB"                  return t_OUTB;
"INA"                   return t_INA;
"INB"                   return t_INB;
"FLOAT"                 return t_FUNC_FLOAT;
"ROUND"                 return t_FUNC_ROUND;
"TRUNC"                 return t_FUNC_TRUNC;
"$"			return t_DOLLAR;
","			return t_COMMA;
"#"                     return t_IMMEDIATE;
"##"			return t_AUGMENTED;
"@"			return t_RELATIVE;
\\			return t_ABSOLUTE;
"#@"			return t_HUBADDRESS;
"<="			return t_EXPR_LE;
"<"			return t_EXPR_LT;
">="			return t_EXPR_GE;
">"                     return t_EXPR_GT;
"=="			return t_EXPR_EQ;
"!="			return t_EXPR_NE;
"("			return t_EXPR_LPAREN;
")"			return t_EXPR_RPAREN;
"["			return t_EXPR_LBRACKET;
"]"			return t_EXPR_RBRACKET;
"++"                    return t_EXPR_INC;
"--"			return t_EXPR_DEC;
"!"			return t_EXPR_NEG;
"~"			return t_EXPR_NOT;
"*"			return t_EXPR_MUL;
"/"			return t_EXPR_DIV;
"%"			return t_EXPR_MOD;
"+"                     return t_EXPR_PLUS;
"-"			return t_EXPR_MINUS;
"<<"			return t_EXPR_SHL;
">>"			return t_EXPR_SHR;
"&"			return t_EXPR_AND;
"|"			return t_EXPR_OR;
"^"			return t_EXPR_XOR;
"><"			return t_EXPR_REV;
"|<"			return t_EXPR_ENCOD;
">|"			return t_EXPR_DECOD;
"&&"			return t_EXPR_LOGAND;
"||"			return t_EXPR_LOGOR;
"'"[^\n]*               return t_comment_eol;
"{"[^}\n]*"}"           return t_comment_curly;
{BINCONST}		return t_bin_const;
{BYTCONST}		return t_byt_const;
{DECCONST}              return t_dec_const;
{HEXCONST}		return t_hex_const;
{REALCONST}             return t_real_const;
{STRCONST}		return t_str_const;
{SYMBOL}                return t_symbol;
{LOCSYM}                return t_locsym;
<*>.			return t_unknown;
%%

int yywrap(void)
{
    Q_UNUSED(yyunput)
    return true;
}

static bool p2flex_compare_pos(const P2Word& w1, const P2Word& w2)
{
    return w1.pos() < w2.pos();
}

p2_words_hash_t p2flex_source(QVector<const QString*> source)
{
    p2_words_hash_t words_hash;
    p2_word_hash_t word_hash;
    word_hash.clear();

    // Put each source line, terminated with QChar::LineFeed,
    // into a single byte buffer
    QByteArray buffer;
    for (int i = 0; i < source.count(); i++) {
        buffer.append(source[i]->toUtf8());
        buffer.append(QChar::LineFeed);
    }
    // Append the end-of-buffer char twice
    buffer.append(char(YY_END_OF_BUFFER_CHAR));
    buffer.append(char(YY_END_OF_BUFFER_CHAR));

    // reset local state
    yylineno = 1;
    yycolumn = 0;
    curly = 0;

    // Begin lexing the buffer
    yy_scan_buffer(buffer.data(), buffer.length());
    int res = yylex();
    while (res > 0) {
        p2_TOKEN_e tok = static_cast<p2_TOKEN_e>(res);
        const int lineno = yylineno;
        const int pos = yycolumn;
        const int len = yyleng;
        const QString* line = source.value(lineno-1, nullptr);
        QStringRef ref(line, pos, len);

        if (nullptr == line) {
            res = yylex();
            continue;
        }

        switch (tok) {
        case t_none:
            // ignore but skip whitespace
            yycolumn += yyleng;
            break;
        case t_EOL:
            // reset column at EOL
            yycolumn = 0;
            break;
        case t_unknown:
            fprintf(stderr, "********** not handled: row=%-4d col=%-3d len=%-3d {%s}\n",
                lineno, pos, len, qPrintable(ref.toString()));
            break;
        default:
            // insert word for token into hash
            word_hash.insert(lineno, P2Word(lineno, tok, ref));
            yycolumn += yyleng;
        }
        res = yylex();
    }

    // Now extract the tokens per line and sort them left-to-right
    for (int i = 0; i < source.count(); i++) {
        const int lineno = i + 1;
        QList<P2Word> words = word_hash.values(lineno);
        if (words.count() > 1)
            std::sort(words.begin(), words.end(), p2flex_compare_pos);
        words_hash.insert(lineno, words);
    }
    return words_hash;
}
