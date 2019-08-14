#include <QString>
#include "p2token.h"

P2Token::P2Token(bool lowercase)
    : m_lowercase(lowercase)
{
}

/**
 * @brief Return the current lowercase enable state
 * @return
 */
bool P2Token::lowercase() const
{
    return m_lowercase;
}

/**
 * @brief Set the lowercase enable true or false
 * @param flag lowercase enable (true), disable (false)
 */
void P2Token::setLowercase(bool flag)
{
    m_lowercase = flag;
}

/**
 * @brief Return a QString for the given %token
 * @param token one of p2_toke_e enume
 * @return QString in uppercase or lowercase
 */
QString P2Token::str(p2_token_e token)
{
    QString str = QStringLiteral("<invalid>");

    switch (token) {
    case t_invalid:
        break;
    case t_nothing:
        str.clear();
        break;
    case t_empty:
        str = QStringLiteral("<empty>");
        break;
    case t_ABS:
        str = QStringLiteral("ABS");
        break;
    case t_ADD:
        str = QStringLiteral("ADD");
        break;
    case t_ADDCT1:
        str = QStringLiteral("ADDCT1");
        break;
    case t_ADDCT2:
        str = QStringLiteral("ADDCT2");
        break;
    case t_ADDCT3:
        str = QStringLiteral("ADDCT3");
        break;
    case t_ADDPIX:
        str = QStringLiteral("ADDPIX");
        break;
    case t_ADDS:
        str = QStringLiteral("ADDS");
        break;
    case t_ADDSX:
        str = QStringLiteral("ADDSX");
        break;
    case t_ADDX:
        str = QStringLiteral("ADDX");
        break;
    case t_AKPIN:
        str = QStringLiteral("AKPIN");
        break;
    case t_ALLOWI:
        str = QStringLiteral("ALLOWI");
        break;
    case t_ALTB:
        str = QStringLiteral("ALTB");
        break;
    case t_ALTD:
        str = QStringLiteral("ALTD");
        break;
    case t_ALTGB:
        str = QStringLiteral("ALTGB");
        break;
    case t_ALTGN:
        str = QStringLiteral("ALTGN");
        break;
    case t_ALTGW:
        str = QStringLiteral("ALTGW");
        break;
    case t_ALTI:
        str = QStringLiteral("ALTI");
        break;
    case t_ALTR:
        str = QStringLiteral("ALTR");
        break;
    case t_ALTS:
        str = QStringLiteral("ALTS");
        break;
    case t_ALTSB:
        str = QStringLiteral("ALTSB");
        break;
    case t_ALTSN:
        str = QStringLiteral("ALTSN");
        break;
    case t_ALTSW:
        str = QStringLiteral("ALTSW");
        break;
    case t_AND:
        str = QStringLiteral("AND");
        break;
    case t_ANDN:
        str = QStringLiteral("ANDN");
        break;
    case t_AUGD:
        str = QStringLiteral("AUGD");
        break;
    case t_AUGS:
        str = QStringLiteral("AUGS");
        break;
    case t_BITC:
        str = QStringLiteral("BITC");
        break;
    case t_BITH:
        str = QStringLiteral("BITH");
        break;
    case t_BITL:
        str = QStringLiteral("BITL");
        break;
    case t_BITNC:
        str = QStringLiteral("BITNC");
        break;
    case t_BITNOT:
        str = QStringLiteral("BITNOT");
        break;
    case t_BITNZ:
        str = QStringLiteral("BITNZ");
        break;
    case t_BITRND:
        str = QStringLiteral("BITRND");
        break;
    case t_BITZ:
        str = QStringLiteral("BITZ");
        break;
    case t_BLNPIX:
        str = QStringLiteral("BLNPIX");
        break;
    case t_BMASK:
        str = QStringLiteral("BMASK");
        break;
    case t_BRK:
        str = QStringLiteral("BRK");
        break;
    case t_C:
        str = QStringLiteral("C");
        break;
    case t_CALL:
        str = QStringLiteral("CALL");
        break;
    case t_CALLA:
        str = QStringLiteral("CALLA");
        break;
    case t_CALLB:
        str = QStringLiteral("CALLB");
        break;
    case t_CALLD:
        str = QStringLiteral("CALLD");
        break;
    case t_CALLPA:
        str = QStringLiteral("CALLPA");
        break;
    case t_CALLPB:
        str = QStringLiteral("CALLPB");
        break;
    case t_CMP:
        str = QStringLiteral("CMP");
        break;
    case t_CMPM:
        str = QStringLiteral("CMPM");
        break;
    case t_CMPR:
        str = QStringLiteral("CMPR");
        break;
    case t_CMPS:
        str = QStringLiteral("CMPS");
        break;
    case t_CMPSUB:
        str = QStringLiteral("CMPSUB");
        break;
    case t_CMPSX:
        str = QStringLiteral("CMPSX");
        break;
    case t_CMPX:
        str = QStringLiteral("CMPX");
        break;
    case t_COGATN:
        str = QStringLiteral("COGATN");
        break;
    case t_COGBRK:
        str = QStringLiteral("COGBRK");
        break;
    case t_COGID:
        str = QStringLiteral("COGID");
        break;
    case t_COGINIT:
        str = QStringLiteral("COGINIT");
        break;
    case t_COGSTOP:
        str = QStringLiteral("COGSTOP");
        break;
    case t_CRCBIT:
        str = QStringLiteral("CRCBIT");
        break;
    case t_CRCNIB:
        str = QStringLiteral("CRCNIB");
        break;
    case t_CZ:
        str = QStringLiteral("CZ");
        break;
    case t_DECMOD:
        str = QStringLiteral("DECMOD");
        break;
    case t_DECOD:
        str = QStringLiteral("DECOD");
        break;
    case t_DIRC:
        str = QStringLiteral("DIRC");
        break;
    case t_DIRH:
        str = QStringLiteral("DIRH");
        break;
    case t_DIRL:
        str = QStringLiteral("DIRL");
        break;
    case t_DIRNC:
        str = QStringLiteral("DIRNC");
        break;
    case t_DIRNOT:
        str = QStringLiteral("DIRNOT");
        break;
    case t_DIRNZ:
        str = QStringLiteral("DIRNZ");
        break;
    case t_DIRRND:
        str = QStringLiteral("DIRRND");
        break;
    case t_DIRZ:
        str = QStringLiteral("DIRZ");
        break;
    case t_DJF:
        str = QStringLiteral("DJF");
        break;
    case t_DJNF:
        str = QStringLiteral("DJNF");
        break;
    case t_DJNZ:
        str = QStringLiteral("DJNZ");
        break;
    case t_DJZ:
        str = QStringLiteral("DJZ");
        break;
    case t_DRVC:
        str = QStringLiteral("DRVC");
        break;
    case t_DRVH:
        str = QStringLiteral("DRVH");
        break;
    case t_DRVL:
        str = QStringLiteral("DRVL");
        break;
    case t_DRVNC:
        str = QStringLiteral("DRVNC");
        break;
    case t_DRVNOT:
        str = QStringLiteral("DRVNOT");
        break;
    case t_DRVNZ:
        str = QStringLiteral("DRVNZ");
        break;
    case t_DRVRND:
        str = QStringLiteral("DRVRND");
        break;
    case t_DRVZ:
        str = QStringLiteral("DRVZ");
        break;
    case t_ENCOD:
        str = QStringLiteral("ENCOD");
        break;
    case t_EXECF:
        str = QStringLiteral("EXECF");
        break;
    case t_FBLOCK:
        str = QStringLiteral("FBLOCK");
        break;
    case t_FGE:
        str = QStringLiteral("FGE");
        break;
    case t_FGES:
        str = QStringLiteral("FGES");
        break;
    case t_FLE:
        str = QStringLiteral("FLE");
        break;
    case t_FLES:
        str = QStringLiteral("FLES");
        break;
    case t_FLTC:
        str = QStringLiteral("FLTC");
        break;
    case t_FLTH:
        str = QStringLiteral("FLTH");
        break;
    case t_FLTL:
        str = QStringLiteral("FLTL");
        break;
    case t_FLTNC:
        str = QStringLiteral("FLTNC");
        break;
    case t_FLTNOT:
        str = QStringLiteral("FLTNOT");
        break;
    case t_FLTNZ:
        str = QStringLiteral("FLTNZ");
        break;
    case t_FLTRND:
        str = QStringLiteral("FLTRND");
        break;
    case t_FLTZ:
        str = QStringLiteral("FLTZ");
        break;
    case t_GETBRK:
        str = QStringLiteral("GETBRK");
        break;
    case t_GETBYTE:
        str = QStringLiteral("GETBYTE");
        break;
    case t_GETCT:
        str = QStringLiteral("GETCT");
        break;
    case t_GETNIB:
        str = QStringLiteral("GETNIB");
        break;
    case t_GETPTR:
        str = QStringLiteral("GETPTR");
        break;
    case t_GETQX:
        str = QStringLiteral("GETQX");
        break;
    case t_GETQY:
        str = QStringLiteral("GETQY");
        break;
    case t_GETRND:
        str = QStringLiteral("GETRND");
        break;
    case t_GETWORD:
        str = QStringLiteral("GETWORD");
        break;
    case t_GETXACC:
        str = QStringLiteral("GETXACC");
        break;
    case t_HUBSET:
        str = QStringLiteral("HUBSET");
        break;
    case t_IF_C:
        str = QStringLiteral("IF_C");
        break;
    case t_IF_C_AND_NZ:
        str = QStringLiteral("IF_C_AND_NZ");
        break;
    case t_IF_C_AND_Z:
        str = QStringLiteral("IF_C_AND_Z");
        break;
    case t_IF_C_EQ_Z:
        str = QStringLiteral("IF_C_EQ_Z");
        break;
    case t_IF_C_NE_Z:
        str = QStringLiteral("IF_C_NE_Z");
        break;
    case t_IF_C_OR_NZ:
        str = QStringLiteral("IF_C_OR_NZ");
        break;
    case t_IF_C_OR_Z:
        str = QStringLiteral("IF_C_OR_Z");
        break;
    case t_IF_NC:
        str = QStringLiteral("IF_NC");
        break;
    case t_IF_NC_AND_NZ:
        str = QStringLiteral("IF_NC_AND_NZ");
        break;
    case t_IF_NC_AND_Z:
        str = QStringLiteral("IF_NC_AND_Z");
        break;
    case t_IF_NC_OR_NZ:
        str = QStringLiteral("IF_NC_OR_NZ");
        break;
    case t_IF_NC_OR_Z:
        str = QStringLiteral("IF_NC_OR_Z");
        break;
    case t_IF_NZ:
        str = QStringLiteral("IF_NZ");
        break;
    case t_IF_Z:
        str = QStringLiteral("IF_Z");
        break;
    case t_IJNZ:
        str = QStringLiteral("IJNZ");
        break;
    case t_IJZ:
        str = QStringLiteral("IJZ");
        break;
    case t_INCMOD:
        str = QStringLiteral("INCMOD");
        break;
    case t_JATN:
        str = QStringLiteral("JATN");
        break;
    case t_JCT1:
        str = QStringLiteral("JCT1");
        break;
    case t_JCT2:
        str = QStringLiteral("JCT2");
        break;
    case t_JCT3:
        str = QStringLiteral("JCT3");
        break;
    case t_JFBW:
        str = QStringLiteral("JFBW");
        break;
    case t_JINT:
        str = QStringLiteral("JINT");
        break;
    case t_JMP:
        str = QStringLiteral("JMP");
        break;
    case t_JMPREL:
        str = QStringLiteral("JMPREL");
        break;
    case t_JNATN:
        str = QStringLiteral("JNATN");
        break;
    case t_JNCT1:
        str = QStringLiteral("JNCT1");
        break;
    case t_JNCT2:
        str = QStringLiteral("JNCT2");
        break;
    case t_JNCT3:
        str = QStringLiteral("JNCT3");
        break;
    case t_JNFBW:
        str = QStringLiteral("JNFBW");
        break;
    case t_JNINT:
        str = QStringLiteral("JNINT");
        break;
    case t_JNPAT:
        str = QStringLiteral("JNPAT");
        break;
    case t_JNQMT:
        str = QStringLiteral("JNQMT");
        break;
    case t_JNSE1:
        str = QStringLiteral("JNSE1");
        break;
    case t_JNSE2:
        str = QStringLiteral("JNSE2");
        break;
    case t_JNSE3:
        str = QStringLiteral("JNSE3");
        break;
    case t_JNSE4:
        str = QStringLiteral("JNSE4");
        break;
    case t_JNXFI:
        str = QStringLiteral("JNXFI");
        break;
    case t_JNXMT:
        str = QStringLiteral("JNXMT");
        break;
    case t_JNXRL:
        str = QStringLiteral("JNXRL");
        break;
    case t_JNXRO:
        str = QStringLiteral("JNXRO");
        break;
    case t_JPAT:
        str = QStringLiteral("JPAT");
        break;
    case t_JQMT:
        str = QStringLiteral("JQMT");
        break;
    case t_JSE1:
        str = QStringLiteral("JSE1");
        break;
    case t_JSE2:
        str = QStringLiteral("JSE2");
        break;
    case t_JSE3:
        str = QStringLiteral("JSE3");
        break;
    case t_JSE4:
        str = QStringLiteral("JSE4");
        break;
    case t_JXFI:
        str = QStringLiteral("JXFI");
        break;
    case t_JXMT:
        str = QStringLiteral("JXMT");
        break;
    case t_JXRL:
        str = QStringLiteral("JXRL");
        break;
    case t_JXRO:
        str = QStringLiteral("JXRO");
        break;
    case t_LOC:
        str = QStringLiteral("LOC");
        break;
    case t_LOCKNEW:
        str = QStringLiteral("LOCKNEW");
        break;
    case t_LOCKREL:
        str = QStringLiteral("LOCKREL");
        break;
    case t_LOCKRET:
        str = QStringLiteral("LOCKRET");
        break;
    case t_LOCKTRY:
        str = QStringLiteral("LOCKTRY");
        break;
    case t_MERGEB:
        str = QStringLiteral("MERGEB");
        break;
    case t_MERGEW:
        str = QStringLiteral("MERGEW");
        break;
    case t_MIXPIX:
        str = QStringLiteral("MIXPIX");
        break;
    case t_MODCZ:
        str = QStringLiteral("MODCZ");
        break;
    case t_MOV:
        str = QStringLiteral("MOV");
        break;
    case t_MOVBYTS:
        str = QStringLiteral("MOVBYTS");
        break;
    case t_MUL:
        str = QStringLiteral("MUL");
        break;
    case t_MULPIX:
        str = QStringLiteral("MULPIX");
        break;
    case t_MULS:
        str = QStringLiteral("MULS");
        break;
    case t_MUXC:
        str = QStringLiteral("MUXC");
        break;
    case t_MUXNC:
        str = QStringLiteral("MUXNC");
        break;
    case t_MUXNIBS:
        str = QStringLiteral("MUXNIBS");
        break;
    case t_MUXNITS:
        str = QStringLiteral("MUXNITS");
        break;
    case t_MUXNZ:
        str = QStringLiteral("MUXNZ");
        break;
    case t_MUXQ:
        str = QStringLiteral("MUXQ");
        break;
    case t_MUXZ:
        str = QStringLiteral("MUXZ");
        break;
    case t_NEG:
        str = QStringLiteral("NEG");
        break;
    case t_NEGC:
        str = QStringLiteral("NEGC");
        break;
    case t_NEGNC:
        str = QStringLiteral("NEGNC");
        break;
    case t_NEGNZ:
        str = QStringLiteral("NEGNZ");
        break;
    case t_NEGZ:
        str = QStringLiteral("NEGZ");
        break;
    case t_NEVER:
        str = QStringLiteral("NEVER");
        break;
    case t_NIXINT1:
        str = QStringLiteral("NIXINT1");
        break;
    case t_NIXINT2:
        str = QStringLiteral("NIXINT2");
        break;
    case t_NIXINT3:
        str = QStringLiteral("NIXINT3");
        break;
    case t_NOP:
        str = QStringLiteral("NOP");
        break;
    case t_NOT:
        str = QStringLiteral("NOT");
        break;
    case t_ONES:
        str = QStringLiteral("ONES");
        break;
    case t_OR:
        str = QStringLiteral("OR");
        break;
    case t_OUTC:
        str = QStringLiteral("OUTC");
        break;
    case t_OUTH:
        str = QStringLiteral("OUTH");
        break;
    case t_OUTL:
        str = QStringLiteral("OUTL");
        break;
    case t_OUTNC:
        str = QStringLiteral("OUTNC");
        break;
    case t_OUTNOT:
        str = QStringLiteral("OUTNOT");
        break;
    case t_OUTNZ:
        str = QStringLiteral("OUTNZ");
        break;
    case t_OUTRND:
        str = QStringLiteral("OUTRND");
        break;
    case t_OUTZ:
        str = QStringLiteral("OUTZ");
        break;
    case t_PA:
        str = QStringLiteral("PA");
        break;
    case t_PB:
        str = QStringLiteral("PB");
        break;
    case t_POLLATN:
        str = QStringLiteral("POLLATN");
        break;
    case t_POLLCT1:
        str = QStringLiteral("POLLCT1");
        break;
    case t_POLLCT2:
        str = QStringLiteral("POLLCT2");
        break;
    case t_POLLCT3:
        str = QStringLiteral("POLLCT3");
        break;
    case t_POLLFBW:
        str = QStringLiteral("POLLFBW");
        break;
    case t_POLLINT:
        str = QStringLiteral("POLLINT");
        break;
    case t_POLLPAT:
        str = QStringLiteral("POLLPAT");
        break;
    case t_POLLQMT:
        str = QStringLiteral("POLLQMT");
        break;
    case t_POLLSE1:
        str = QStringLiteral("POLLSE1");
        break;
    case t_POLLSE2:
        str = QStringLiteral("POLLSE2");
        break;
    case t_POLLSE3:
        str = QStringLiteral("POLLSE3");
        break;
    case t_POLLSE4:
        str = QStringLiteral("POLLSE4");
        break;
    case t_POLLXFI:
        str = QStringLiteral("POLLXFI");
        break;
    case t_POLLXMT:
        str = QStringLiteral("POLLXMT");
        break;
    case t_POLLXRL:
        str = QStringLiteral("POLLXRL");
        break;
    case t_POLLXRO:
        str = QStringLiteral("POLLXRO");
        break;
    case t_POP:
        str = QStringLiteral("POP");
        break;
    case t_POPA:
        str = QStringLiteral("POPA");
        break;
    case t_POPB:
        str = QStringLiteral("POPB");
        break;
    case t_PTRA:
        str = QStringLiteral("PTRA");
        break;
    case t_PTRB:
        str = QStringLiteral("PTRB");
        break;
    case t_PUSH:
        str = QStringLiteral("PUSH");
        break;
    case t_PUSHA:
        str = QStringLiteral("PUSHA");
        break;
    case t_PUSHB:
        str = QStringLiteral("PUSHB");
        break;
    case t_QDIV:
        str = QStringLiteral("QDIV");
        break;
    case t_QEXP:
        str = QStringLiteral("QEXP");
        break;
    case t_QFRAC:
        str = QStringLiteral("QFRAC");
        break;
    case t_QLOG:
        str = QStringLiteral("QLOG");
        break;
    case t_QMUL:
        str = QStringLiteral("QMUL");
        break;
    case t_QROTATE:
        str = QStringLiteral("QROTATE");
        break;
    case t_QSQRT:
        str = QStringLiteral("QSQRT");
        break;
    case t_QVECTOR:
        str = QStringLiteral("QVECTOR");
        break;
    case t_RCL:
        str = QStringLiteral("RCL");
        break;
    case t_RCR:
        str = QStringLiteral("RCR");
        break;
    case t_RCZL:
        str = QStringLiteral("RCZL");
        break;
    case t_RCZR:
        str = QStringLiteral("RCZR");
        break;
    case t_RDBYTE:
        str = QStringLiteral("RDBYTE");
        break;
    case t_RDFAST:
        str = QStringLiteral("RDFAST");
        break;
    case t_RDLONG:
        str = QStringLiteral("RDLONG");
        break;
    case t_RDLUT:
        str = QStringLiteral("RDLUT");
        break;
    case t_RDPIN:
        str = QStringLiteral("RDPIN");
        break;
    case t_RDWORD:
        str = QStringLiteral("RDWORD");
        break;
    case t_REP:
        str = QStringLiteral("REP");
        break;
    case t_RESI0:
        str = QStringLiteral("RESI0");
        break;
    case t_RESI1:
        str = QStringLiteral("RESI1");
        break;
    case t_RESI2:
        str = QStringLiteral("RESI2");
        break;
    case t_RESI3:
        str = QStringLiteral("RESI3");
        break;
    case t_RET:
        str = QStringLiteral("RET");
        break;
    case t_RETA:
        str = QStringLiteral("RETA");
        break;
    case t_RETB:
        str = QStringLiteral("RETB");
        break;
    case t_RETI0:
        str = QStringLiteral("RETI0");
        break;
    case t_RETI1:
        str = QStringLiteral("RETI1");
        break;
    case t_RETI2:
        str = QStringLiteral("RETI2");
        break;
    case t_RETI3:
        str = QStringLiteral("RETI3");
        break;
    case t_REV:
        str = QStringLiteral("REV");
        break;
    case t_RFBYTE:
        str = QStringLiteral("RFBYTE");
        break;
    case t_RFLONG:
        str = QStringLiteral("RFLONG");
        break;
    case t_RFVAR:
        str = QStringLiteral("RFVAR");
        break;
    case t_RFVARS:
        str = QStringLiteral("RFVARS");
        break;
    case t_RFWORD:
        str = QStringLiteral("RFWORD");
        break;
    case t_RGBEXP:
        str = QStringLiteral("RGBEXP");
        break;
    case t_RGBSQZ:
        str = QStringLiteral("RGBSQZ");
        break;
    case t_ROL:
        str = QStringLiteral("ROL");
        break;
    case t_ROLBYTE:
        str = QStringLiteral("ROLBYTE");
        break;
    case t_ROLNIB:
        str = QStringLiteral("ROLNIB");
        break;
    case t_ROLWORD:
        str = QStringLiteral("ROLWORD");
        break;
    case t_ROR:
        str = QStringLiteral("ROR");
        break;
    case t_RQPIN:
        str = QStringLiteral("RQPIN");
        break;
    case t_SAL:
        str = QStringLiteral("SAL");
        break;
    case t_SAR:
        str = QStringLiteral("SAR");
        break;
    case t_SCA:
        str = QStringLiteral("SCA");
        break;
    case t_SCAS:
        str = QStringLiteral("SCAS");
        break;
    case t_SETBYTE:
        str = QStringLiteral("SETBYTE");
        break;
    case t_SETCFRQ:
        str = QStringLiteral("SETCFRQ");
        break;
    case t_SETCI:
        str = QStringLiteral("SETCI");
        break;
    case t_SETCMOD:
        str = QStringLiteral("SETCMOD");
        break;
    case t_SETCQ:
        str = QStringLiteral("SETCQ");
        break;
    case t_SETCY:
        str = QStringLiteral("SETCY");
        break;
    case t_SETD:
        str = QStringLiteral("SETD");
        break;
    case t_SETDACS:
        str = QStringLiteral("SETDACS");
        break;
    case t_SETINT1:
        str = QStringLiteral("SETINT1");
        break;
    case t_SETINT2:
        str = QStringLiteral("SETINT2");
        break;
    case t_SETINT3:
        str = QStringLiteral("SETINT3");
        break;
    case t_SETLUTS:
        str = QStringLiteral("SETLUTS");
        break;
    case t_SETNIB:
        str = QStringLiteral("SETNIB");
        break;
    case t_SETPAT:
        str = QStringLiteral("SETPAT");
        break;
    case t_SETPIV:
        str = QStringLiteral("SETPIV");
        break;
    case t_SETPIX:
        str = QStringLiteral("SETPIX");
        break;
    case t_SETQ:
        str = QStringLiteral("SETQ");
        break;
    case t_SETQ2:
        str = QStringLiteral("SETQ2");
        break;
    case t_SETR:
        str = QStringLiteral("SETR");
        break;
    case t_SETS:
        str = QStringLiteral("SETS");
        break;
    case t_SETSE1:
        str = QStringLiteral("SETSE1");
        break;
    case t_SETSE2:
        str = QStringLiteral("SETSE2");
        break;
    case t_SETSE3:
        str = QStringLiteral("SETSE3");
        break;
    case t_SETSE4:
        str = QStringLiteral("SETSE4");
        break;
    case t_SETWORD:
        str = QStringLiteral("SETWORD");
        break;
    case t_SETXFRQ:
        str = QStringLiteral("SETXFRQ");
        break;
    case t_SEUSSF:
        str = QStringLiteral("SEUSSF");
        break;
    case t_SEUSSR:
        str = QStringLiteral("SEUSSR");
        break;
    case t_SHL:
        str = QStringLiteral("SHL");
        break;
    case t_SHR:
        str = QStringLiteral("SHR");
        break;
    case t_SIGNX:
        str = QStringLiteral("SIGNX");
        break;
    case t_SKIP:
        str = QStringLiteral("SKIP");
        break;
    case t_SKIPF:
        str = QStringLiteral("SKIPF");
        break;
    case t_SPLITB:
        str = QStringLiteral("SPLITB");
        break;
    case t_SPLITW:
        str = QStringLiteral("SPLITW");
        break;
    case t_STALLI:
        str = QStringLiteral("STALLI");
        break;
    case t_SUB:
        str = QStringLiteral("SUB");
        break;
    case t_SUBR:
        str = QStringLiteral("SUBR");
        break;
    case t_SUBS:
        str = QStringLiteral("SUBS");
        break;
    case t_SUBSX:
        str = QStringLiteral("SUBSX");
        break;
    case t_SUBX:
        str = QStringLiteral("SUBX");
        break;
    case t_SUMC:
        str = QStringLiteral("SUMC");
        break;
    case t_SUMNC:
        str = QStringLiteral("SUMNC");
        break;
    case t_SUMNZ:
        str = QStringLiteral("SUMNZ");
        break;
    case t_SUMZ:
        str = QStringLiteral("SUMZ");
        break;
    case t_TEST:
        str = QStringLiteral("TEST");
        break;
    case t_TESTB:
        str = QStringLiteral("TESTB");
        break;
    case t_TESTBN:
        str = QStringLiteral("TESTBN");
        break;
    case t_TESTN:
        str = QStringLiteral("TESTN");
        break;
    case t_TESTNB:
        str = QStringLiteral("TESTNB");
        break;
    case t_TESTP:
        str = QStringLiteral("TESTP");
        break;
    case t_TESTPN:
        str = QStringLiteral("TESTPN");
        break;
    case t_TJF:
        str = QStringLiteral("TJF");
        break;
    case t_TJNF:
        str = QStringLiteral("TJNF");
        break;
    case t_TJNS:
        str = QStringLiteral("TJNS");
        break;
    case t_TJNZ:
        str = QStringLiteral("TJNZ");
        break;
    case t_TJS:
        str = QStringLiteral("TJS");
        break;
    case t_TJV:
        str = QStringLiteral("TJV");
        break;
    case t_TJZ:
        str = QStringLiteral("TJZ");
        break;
    case t_TRGINT1:
        str = QStringLiteral("TRGINT1");
        break;
    case t_TRGINT2:
        str = QStringLiteral("TRGINT2");
        break;
    case t_TRGINT3:
        str = QStringLiteral("TRGINT3");
        break;
    case t_W:
        str = QStringLiteral("W");
        break;
    case t_WAITATN:
        str = QStringLiteral("WAITATN");
        break;
    case t_WAITCT1:
        str = QStringLiteral("WAITCT1");
        break;
    case t_WAITCT2:
        str = QStringLiteral("WAITCT2");
        break;
    case t_WAITCT3:
        str = QStringLiteral("WAITCT3");
        break;
    case t_WAITFBW:
        str = QStringLiteral("WAITFBW");
        break;
    case t_WAITINT:
        str = QStringLiteral("WAITINT");
        break;
    case t_WAITPAT:
        str = QStringLiteral("WAITPAT");
        break;
    case t_WAITSE1:
        str = QStringLiteral("WAITSE1");
        break;
    case t_WAITSE2:
        str = QStringLiteral("WAITSE2");
        break;
    case t_WAITSE3:
        str = QStringLiteral("WAITSE3");
        break;
    case t_WAITSE4:
        str = QStringLiteral("WAITSE4");
        break;
    case t_WAITX:
        str = QStringLiteral("WAITX");
        break;
    case t_WAITXFI:
        str = QStringLiteral("WAITXFI");
        break;
    case t_WAITXMT:
        str = QStringLiteral("WAITXMT");
        break;
    case t_WAITXRL:
        str = QStringLiteral("WAITXRL");
        break;
    case t_WAITXRO:
        str = QStringLiteral("WAITXRO");
        break;
    case t_WFBYTE:
        str = QStringLiteral("WFBYTE");
        break;
    case t_WFLONG:
        str = QStringLiteral("WFLONG");
        break;
    case t_WFWORD:
        str = QStringLiteral("WFWORD");
        break;
    case t_WMLONG:
        str = QStringLiteral("WMLONG");
        break;
    case t_WRBYTE:
        str = QStringLiteral("WRBYTE");
        break;
    case t_WRC:
        str = QStringLiteral("WRC");
        break;
    case t_WRFAST:
        str = QStringLiteral("WRFAST");
        break;
    case t_WRLONG:
        str = QStringLiteral("WRLONG");
        break;
    case t_WRLUT:
        str = QStringLiteral("WRLUT");
        break;
    case t_WRNC:
        str = QStringLiteral("WRNC");
        break;
    case t_WRNZ:
        str = QStringLiteral("WRNZ");
        break;
    case t_WRPIN:
        str = QStringLiteral("WRPIN");
        break;
    case t_WRWORD:
        str = QStringLiteral("WRWORD");
        break;
    case t_WRZ:
        str = QStringLiteral("WRZ");
        break;
    case t_WXPIN:
        str = QStringLiteral("WXPIN");
        break;
    case t_WYPIN:
        str = QStringLiteral("WYPIN");
        break;
    case t_XCONT:
        str = QStringLiteral("XCONT");
        break;
    case t_XINIT:
        str = QStringLiteral("XINIT");
        break;
    case t_XOR:
        str = QStringLiteral("XOR");
        break;
    case t_XORO32:
        str = QStringLiteral("XORO32");
        break;
    case t_XSTOP:
        str = QStringLiteral("XSTOP");
        break;
    case t_Z:
        str = QStringLiteral("Z");
        break;
    case t_XZERO:
        str = QStringLiteral("XZERO");
        break;
    case t_ZEROX:
        str = QStringLiteral("ZEROX");
        break;
    case t__RET_:
        str = QStringLiteral("_RET_");
        break;
    }

    return m_lowercase ? str.toLower() : str;
}
