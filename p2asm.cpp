/****************************************************************************
 *
 * Propeller2 assembler
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
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QRegExp>
#include "p2asm.h"

static const QStringList factors_operators =
        QStringList()
        << QStringLiteral("*")
        << QStringLiteral("/")
        << QStringLiteral("\\");

static const QStringList summands_operators =
        QStringList()
        << QStringLiteral("+")
        << QStringLiteral("-");

static const QStringList logical_operators =
        QStringList()
        << QStringLiteral("!")
        << QStringLiteral("&")
        << QStringLiteral("|")
        << QStringLiteral("^")
        << QStringLiteral("<<")
        << QStringLiteral(">>");

/**
 * @brief P2Asm constructor
 * @param parent pointer to parent QObject
 */
P2Asm::P2Asm(QObject *parent)
    : QObject(parent)
{
}

/**
 * @brief P2Asm destructor
 */
P2Asm::~P2Asm()
{
}

p2_cond_e P2Asm::conditional(P2Params& params, p2_token_e cond)
{
    p2_cond_e eeee = cond__ret_;
    switch (cond) {
    case t__RET_:
        eeee = cond__ret_;
        params.idx++;
        break;
    case t_IF_C:
        eeee = cond_c;
        params.idx++;
        break;
    case t_IF_C_AND_NZ:
        eeee = cond_c_and_nz;
        params.idx++;
        break;
    case t_IF_C_AND_Z:
        eeee = cond_c_and_z;
        params.idx++;
        break;
    case t_IF_C_EQ_Z:
        eeee = cond_c_eq_z;
        params.idx++;
        break;
    case t_IF_C_NE_Z:
        eeee = cond_c_ne_z;
        params.idx++;
        break;
    case t_IF_C_OR_NZ:
        eeee = cond_c_or_nz;
        params.idx++;
        break;
    case t_IF_C_OR_Z:
        eeee = cond_c_or_z;
        params.idx++;
        break;
    case t_IF_NC:
        eeee = cond_nc;
        params.idx++;
        break;
    case t_IF_NC_AND_NZ:
        eeee = cond_nc_and_nz;
        params.idx++;
        break;
    case t_IF_NC_AND_Z:
        eeee = cond_nc_and_z;
        params.idx++;
        break;
    case t_IF_NC_OR_NZ:
        eeee = cond_nc_or_nz;
        params.idx++;
        break;
    case t_IF_NC_OR_Z:
        eeee = cond_nc_or_z;
        params.idx++;
        break;
    case t_IF_NZ:
        eeee = cond_nz;
        params.idx++;
        break;
    case t_IF_Z:
        eeee = cond_z;
        params.idx++;
        break;
    default:
        eeee = cond_always;
        break;
    }

    return eeee;
}

/**
 * @brief Split a line into words and tokenize
 *
 * Keep strings inside double quotes as a single word.
 * Split at one or multiple blank (spaces, tabstop) characters.
 * Also split at comma.
 *
 * @param line assembler source line
 * @return QStringList with words
 */
bool P2Asm::split_and_tokenize(P2Params& params, const QString& line)
{
    QString word;
    QChar string = QChar::Null;
    bool escaped = false;
    bool comment = false;

    params.words.clear();
    foreach(QChar ch, line) {

        if (comment) {
            word += ch;
            continue;
        }

        if (escaped) {
            escaped = false;
            word += ch;
            continue;
        }

        if (string != QChar::Null) {
            if (ch == string) {
                string = QChar::Null;
            }
            word += ch;
            continue;
        }

        if (ch.isSpace()) {
            if (!word.isEmpty()) {
                params.words += word;
                word.clear();
            }
            continue;
        }

        if (ch == QChar('\'')) {
            comment = true;
            word += ch;
            continue;
        }

        if (ch == QChar(',')) {
            params.words += word;
            word.clear();
            continue;
        }

        if (ch == QChar('"')) {
            word += ch;
            string = ch;
            continue;
        }
        word += ch;
    }

    // Append last word, if it isn't the trailing comment
    if (!comment && !word.isEmpty())
        params.words += word;

    // Now tokenize the words
    params.tokens.clear();
    foreach(const QString& word, params.words) {
        if (word.startsWith(QChar('\'')))
            break;
        params.tokens += Token.token(word);
    }
    params.cnt = params.tokens.count();
    params.idx = 0;

    return true;
}

/**
 * @brief Assemble a QStringList of lines of SPIN2 source code
 * @param source code
 * @return P2Params structure
 */
bool P2Asm::assemble(P2Params& params, const QStringList& source)
{
    bool multi_comment = false;

    foreach(QString line, source) {
        line.remove(QRegExp("\\r$"));
        // Parse the next line
        params.line = line;
        params.lineno += 1;
        params.error.clear();

        if (line.startsWith(QStringLiteral("{")))
            multi_comment = true;

        if (multi_comment) {
            if (line.endsWith("}"))
                multi_comment = false;
            // skip over multi line comment
            continue;
        }

        // Skip over empty lines
        if (line.isEmpty())
            continue;

        // Split line into words and tokenize it
        split_and_tokenize(params, line);

        // Set current program counter from next
        params.curr_pc = params.next_pc;

        // Whenever the original line starts with a non-blank char,
        // the first word is defined as a symbol for the current origin
        params.symbol.clear();
        if (params.idx < params.cnt && params.tokens.at(params.idx) == t_nothing) {
            params.symbol = params.words.at(params.idx);
            // skip over first word
            params.idx++;
            P2AsmSymbol sym = params.symbols.value(params.symbol);
            if (sym.isEmpty()) {
                params.symbols.insert(params.symbol, params.curr_pc);
            } else {
                emit Error(params.lineno, tr("Symbol already defined: %1").arg(params.symbol));
            }
        }

        // Skip if no more tokens were found
        if (params.idx >= params.tokens.count())
            continue;

        // Reset all instruction bits
        params.IR.opcode = 0;

        // Conditional execution prefix
        const p2_token_e cond = params.tokens.at(params.idx);
        const p2_cond_e eeee = conditional(params, cond);
        params.IR.op.cond = eeee;

        // Expect a token for an instruction
        const p2_token_e inst = params.tokens.at(params.idx);
        bool success = false;

        switch (inst) {
        case t_ABS:
            success = asm_abs(params);
            break;

        case t_ADD:
            success = asm_add(params);
            break;

        case t_ADDCT1:
            success = asm_addct1(params);
            break;

        case t_ADDCT2:
            success = asm_addct2(params);
            break;

        case t_ADDCT3:
            success = asm_addct3(params);
            break;

        case t_ADDPIX:
            success = asm_addpix(params);
            break;

        case t_ADDS:
            success = asm_adds(params);
            break;

        case t_ADDSX:
            success = asm_addsx(params);
            break;

        case t_ADDX:
            success = asm_addx(params);
            break;

        case t_AKPIN:
            success = asm_akpin(params);
            break;

        case t_ALLOWI:
            success = asm_allowi(params);
            break;

        case t_ALTB:
            success = asm_altb(params);
            break;

        case t_ALTD:
            success = asm_altd(params);
            break;

        case t_ALTGB:
            success = asm_altgb(params);
            break;

        case t_ALTGN:
            success = asm_altgn(params);
            break;

        case t_ALTGW:
            success = asm_altgw(params);
            break;

        case t_ALTI:
            success = asm_alti(params);
            break;

        case t_ALTR:
            success = asm_altr(params);
            break;

        case t_ALTS:
            success = asm_alts(params);
            break;

        case t_ALTSB:
            success = asm_altsb(params);
            break;

        case t_ALTSN:
            success = asm_altsn(params);
            break;

        case t_ALTSW:
            success = asm_altsw(params);
            break;

        case t_AND:
            success = asm_and(params);
            break;

        case t_ANDN:
            success = asm_andn(params);
            break;

        case t_AUGD:
            success = asm_augd(params);
            break;

        case t_AUGS:
            success = asm_augs(params);
            break;

        case t_BITC:
            success = asm_bitc(params);
            break;

        case t_BITH:
            success = asm_bith(params);
            break;

        case t_BITL:
            success = asm_bitl(params);
            break;

        case t_BITNC:
            success = asm_bitnc(params);
            break;

        case t_BITNOT:
            success = asm_bitnot(params);
            break;

        case t_BITNZ:
            success = asm_bitnz(params);
            break;

        case t_BITRND:
            success = asm_bitrnd(params);
            break;

        case t_BITZ:
            success = asm_bitz(params);
            break;

        case t_BLNPIX:
            success = asm_blnpix(params);
            break;

        case t_BMASK:
            success = asm_bmask(params);
            break;

        case t_BRK:
            success = asm_brk(params);
            break;

        case t_CALL:
            success = asm_call(params);
            break;

        case t_CALLA:
            success = asm_calla(params);
            break;

        case t_CALLB:
            success = asm_callb(params);
            break;

        case t_CALLD:
            success = asm_calld(params);
            break;

        case t_CALLPA:
            success = asm_callpa(params);
            break;

        case t_CALLPB:
            success = asm_callpb(params);
            break;

        case t_CMP:
            success = asm_cmp(params);
            break;

        case t_CMPM:
            success = asm_cmpm(params);
            break;

        case t_CMPR:
            success = asm_cmpr(params);
            break;

        case t_CMPS:
            success = asm_cmps(params);
            break;

        case t_CMPSUB:
            success = asm_cmpsub(params);
            break;

        case t_CMPSX:
            success = asm_cmpsx(params);
            break;

        case t_CMPX:
            success = asm_cmpx(params);
            break;

        case t_COGATN:
            success = asm_cogatn(params);
            break;

        case t_COGBRK:
            success = asm_cogbrk(params);
            break;

        case t_COGID:
            success = asm_cogid(params);
            break;

        case t_COGINIT:
            success = asm_coginit(params);
            break;

        case t_COGSTOP:
            success = asm_cogstop(params);
            break;

        case t_CRCBIT:
            success = asm_crcbit(params);
            break;

        case t_CRCNIB:
            success = asm_crcnib(params);
            break;

        case t_DECMOD:
            success = asm_decmod(params);
            break;

        case t_DECOD:
            success = asm_decod(params);
            break;

        case t_DIRC:
            success = asm_dirc(params);
            break;

        case t_DIRH:
            success = asm_dirh(params);
            break;

        case t_DIRL:
            success = asm_dirl(params);
            break;

        case t_DIRNC:
            success = asm_dirnc(params);
            break;

        case t_DIRNOT:
            success = asm_dirnot(params);
            break;

        case t_DIRNZ:
            success = asm_dirnz(params);
            break;

        case t_DIRRND:
            success = asm_dirrnd(params);
            break;

        case t_DIRZ:
            success = asm_dirz(params);
            break;

        case t_DJF:
            success = asm_djf(params);
            break;

        case t_DJNF:
            success = asm_djnf(params);
            break;

        case t_DJNZ:
            success = asm_djnz(params);
            break;

        case t_DJZ:
            success = asm_djz(params);
            break;

        case t_DRVC:
            success = asm_drvc(params);
            break;

        case t_DRVH:
            success = asm_drvh(params);
            break;

        case t_DRVL:
            success = asm_drvl(params);
            break;

        case t_DRVNC:
            success = asm_drvnc(params);
            break;

        case t_DRVNOT:
            success = asm_drvnot(params);
            break;

        case t_DRVNZ:
            success = asm_drvnz(params);
            break;

        case t_DRVRND:
            success = asm_drvrnd(params);
            break;

        case t_DRVZ:
            success = asm_drvz(params);
            break;

        case t_ENCOD:
            success = asm_encod(params);
            break;

        case t_EXECF:
            success = asm_execf(params);
            break;

        case t_FBLOCK:
            success = asm_fblock(params);
            break;

        case t_FGE:
            success = asm_fge(params);
            break;

        case t_FGES:
            success = asm_fges(params);
            break;

        case t_FLE:
            success = asm_fle(params);
            break;

        case t_FLES:
            success = asm_fles(params);
            break;

        case t_FLTC:
            success = asm_fltc(params);
            break;

        case t_FLTH:
            success = asm_flth(params);
            break;

        case t_FLTL:
            success = asm_fltl(params);
            break;

        case t_FLTNC:
            success = asm_fltnc(params);
            break;

        case t_FLTNOT:
            success = asm_fltnot(params);
            break;

        case t_FLTNZ:
            success = asm_fltnz(params);
            break;

        case t_FLTRND:
            success = asm_fltrnd(params);
            break;

        case t_FLTZ:
            success = asm_fltz(params);
            break;

        case t_GETBRK:
            success = asm_getbrk(params);
            break;

        case t_GETBYTE:
            success = asm_getbyte(params);
            break;

        case t_GETCT:
            success = asm_getct(params);
            break;

        case t_GETNIB:
            success = asm_getnib(params);
            break;

        case t_GETPTR:
            success = asm_getptr(params);
            break;

        case t_GETQX:
            success = asm_getqx(params);
            break;

        case t_GETQY:
            success = asm_getqy(params);
            break;

        case t_GETRND:
            success = asm_getrnd(params);
            break;

        case t_GETSCP:
            success = asm_getscp(params);
            break;

        case t_GETWORD:
            success = asm_getword(params);
            break;

        case t_GETXACC:
            success = asm_getxacc(params);
            break;

        case t_HUBSET:
            success = asm_hubset(params);
            break;

        case t_IJNZ:
            success = asm_ijnz(params);
            break;

        case t_IJZ:
            success = asm_ijz(params);
            break;

        case t_INCMOD:
            success = asm_incmod(params);
            break;

        case t_JATN:
            success = asm_jatn(params);
            break;

        case t_JCT1:
            success = asm_jct1(params);
            break;

        case t_JCT2:
            success = asm_jct2(params);
            break;

        case t_JCT3:
            success = asm_jct3(params);
            break;

        case t_JFBW:
            success = asm_jfbw(params);
            break;

        case t_JINT:
            success = asm_jint(params);
            break;

        case t_JMP:
            success = asm_jmp(params);
            break;

        case t_JMPREL:
            success = asm_jmprel(params);
            break;

        case t_JNATN:
            success = asm_jnatn(params);
            break;

        case t_JNCT1:
            success = asm_jnct1(params);
            break;

        case t_JNCT2:
            success = asm_jnct2(params);
            break;

        case t_JNCT3:
            success = asm_jnct3(params);
            break;

        case t_JNFBW:
            success = asm_jnfbw(params);
            break;

        case t_JNINT:
            success = asm_jnint(params);
            break;

        case t_JNPAT:
            success = asm_jnpat(params);
            break;

        case t_JNQMT:
            success = asm_jnqmt(params);
            break;

        case t_JNSE1:
            success = asm_jnse1(params);
            break;

        case t_JNSE2:
            success = asm_jnse2(params);
            break;

        case t_JNSE3:
            success = asm_jnse3(params);
            break;

        case t_JNSE4:
            success = asm_jnse4(params);
            break;

        case t_JNXFI:
            success = asm_jnxfi(params);
            break;

        case t_JNXMT:
            success = asm_jnxmt(params);
            break;

        case t_JNXRL:
            success = asm_jnxrl(params);
            break;

        case t_JNXRO:
            success = asm_jnxro(params);
            break;

        case t_JPAT:
            success = asm_jpat(params);
            break;

        case t_JQMT:
            success = asm_jqmt(params);
            break;

        case t_JSE1:
            success = asm_jse1(params);
            break;

        case t_JSE2:
            success = asm_jse2(params);
            break;

        case t_JSE3:
            success = asm_jse3(params);
            break;

        case t_JSE4:
            success = asm_jse4(params);
            break;

        case t_JXFI:
            success = asm_jxfi(params);
            break;

        case t_JXMT:
            success = asm_jxmt(params);
            break;

        case t_JXRL:
            success = asm_jxrl(params);
            break;

        case t_JXRO:
            success = asm_jxro(params);
            break;

        case t_LOC:
            success = asm_loc_pa(params);
            break;

        case t_LOCKNEW:
            success = asm_locknew(params);
            break;

        case t_LOCKREL:
            success = asm_lockrel(params);
            break;

        case t_LOCKRET:
            success = asm_lockret(params);
            break;

        case t_LOCKTRY:
            success = asm_locktry(params);
            break;

        case t_MERGEB:
            success = asm_mergeb(params);
            break;

        case t_MERGEW:
            success = asm_mergew(params);
            break;

        case t_MIXPIX:
            success = asm_mixpix(params);
            break;

        case t_MODCZ:
            success = asm_modcz(params);
            break;

        case t_MOV:
            success = asm_mov(params);
            break;

        case t_MOVBYTS:
            success = asm_movbyts(params);
            break;

        case t_MUL:
            success = asm_mul(params);
            break;

        case t_MULPIX:
            success = asm_mulpix(params);
            break;

        case t_MULS:
            success = asm_muls(params);
            break;

        case t_MUXC:
            success = asm_muxc(params);
            break;

        case t_MUXNC:
            success = asm_muxnc(params);
            break;

        case t_MUXNIBS:
            success = asm_muxnibs(params);
            break;

        case t_MUXNITS:
            success = asm_muxnits(params);
            break;

        case t_MUXNZ:
            success = asm_muxnz(params);
            break;

        case t_MUXQ:
            success = asm_muxq(params);
            break;

        case t_MUXZ:
            success = asm_muxz(params);
            break;

        case t_NEG:
            success = asm_neg(params);
            break;

        case t_NEGC:
            success = asm_negc(params);
            break;

        case t_NEGNC:
            success = asm_negnc(params);
            break;

        case t_NEGNZ:
            success = asm_negnz(params);
            break;

        case t_NEGZ:
            success = asm_negz(params);
            break;

        case t_NIXINT1:
            success = asm_nixint1(params);
            break;

        case t_NIXINT2:
            success = asm_nixint2(params);
            break;

        case t_NIXINT3:
            success = asm_nixint3(params);
            break;

        case t_NOP:
            success = asm_nop(params);
            break;

        case t_NOT:
            success = asm_not(params);
            break;

        case t_ONES:
            success = asm_ones(params);
            break;

        case t_OR:
            success = asm_or(params);
            break;

        case t_OUTC:
            success = asm_outc(params);
            break;

        case t_OUTH:
            success = asm_outh(params);
            break;

        case t_OUTL:
            success = asm_outl(params);
            break;

        case t_OUTNC:
            success = asm_outnc(params);
            break;

        case t_OUTNOT:
            success = asm_outnot(params);
            break;

        case t_OUTNZ:
            success = asm_outnz(params);
            break;

        case t_OUTRND:
            success = asm_outrnd(params);
            break;

        case t_OUTZ:
            success = asm_outz(params);
            break;

        case t_POLLATN:
            success = asm_pollatn(params);
            break;

        case t_POLLCT1:
            success = asm_pollct1(params);
            break;

        case t_POLLCT2:
            success = asm_pollct2(params);
            break;

        case t_POLLCT3:
            success = asm_pollct3(params);
            break;

        case t_POLLFBW:
            success = asm_pollfbw(params);
            break;

        case t_POLLINT:
            success = asm_pollint(params);
            break;

        case t_POLLPAT:
            success = asm_pollpat(params);
            break;

        case t_POLLQMT:
            success = asm_pollqmt(params);
            break;

        case t_POLLSE1:
            success = asm_pollse1(params);
            break;

        case t_POLLSE2:
            success = asm_pollse2(params);
            break;

        case t_POLLSE3:
            success = asm_pollse3(params);
            break;

        case t_POLLSE4:
            success = asm_pollse4(params);
            break;

        case t_POLLXFI:
            success = asm_pollxfi(params);
            break;

        case t_POLLXMT:
            success = asm_pollxmt(params);
            break;

        case t_POLLXRL:
            success = asm_pollxrl(params);
            break;

        case t_POLLXRO:
            success = asm_pollxro(params);
            break;

        case t_POP:
            success = asm_pop(params);
            break;

        case t_POPA:
            success = asm_popa(params);
            break;

        case t_POPB:
            success = asm_popb(params);
            break;

        case t_PUSH:
            success = asm_push(params);
            break;

        case t_PUSHA:
            success = asm_pusha(params);
            break;

        case t_PUSHB:
            success = asm_pushb(params);
            break;

        case t_QDIV:
            success = asm_qdiv(params);
            break;

        case t_QEXP:
            success = asm_qexp(params);
            break;

        case t_QFRAC:
            success = asm_qfrac(params);
            break;

        case t_QLOG:
            success = asm_qlog(params);
            break;

        case t_QMUL:
            success = asm_qmul(params);
            break;

        case t_QROTATE:
            success = asm_qrotate(params);
            break;

        case t_QSQRT:
            success = asm_qsqrt(params);
            break;

        case t_QVECTOR:
            success = asm_qvector(params);
            break;

        case t_RCL:
            success = asm_rcl(params);
            break;

        case t_RCR:
            success = asm_rcr(params);
            break;

        case t_RCZL:
            success = asm_rczl(params);
            break;

        case t_RCZR:
            success = asm_rczr(params);
            break;

        case t_RDBYTE:
            success = asm_rdbyte(params);
            break;

        case t_RDFAST:
            success = asm_rdfast(params);
            break;

        case t_RDLONG:
            success = asm_rdlong(params);
            break;

        case t_RDLUT:
            success = asm_rdlut(params);
            break;

        case t_RDPIN:
            success = asm_rdpin(params);
            break;

        case t_RDWORD:
            success = asm_rdword(params);
            break;

        case t_REP:
            success = asm_rep(params);
            break;

        case t_RESI0:
            success = asm_resi0(params);
            break;

        case t_RESI1:
            success = asm_resi1(params);
            break;

        case t_RESI2:
            success = asm_resi2(params);
            break;

        case t_RESI3:
            success = asm_resi3(params);
            break;

        case t_RET:
            success = asm_ret(params);
            break;

        case t_RETA:
            success = asm_reta(params);
            break;

        case t_RETB:
            success = asm_retb(params);
            break;

        case t_RETI0:
            success = asm_reti0(params);
            break;

        case t_RETI1:
            success = asm_reti1(params);
            break;

        case t_RETI2:
            success = asm_reti2(params);
            break;

        case t_RETI3:
            success = asm_reti3(params);
            break;

        case t_REV:
            success = asm_rev(params);
            break;

        case t_RFBYTE:
            success = asm_rfbyte(params);
            break;

        case t_RFLONG:
            success = asm_rflong(params);
            break;

        case t_RFVAR:
            success = asm_rfvar(params);
            break;

        case t_RFVARS:
            success = asm_rfvars(params);
            break;

        case t_RFWORD:
            success = asm_rfword(params);
            break;

        case t_RGBEXP:
            success = asm_rgbexp(params);
            break;

        case t_RGBSQZ:
            success = asm_rgbsqz(params);
            break;

        case t_ROL:
            success = asm_rol(params);
            break;

        case t_ROLBYTE:
            success = asm_rolbyte(params);
            break;

        case t_ROLNIB:
            success = asm_rolnib(params);
            break;

        case t_ROLWORD:
            success = asm_rolword(params);
            break;

        case t_ROR:
            success = asm_ror(params);
            break;

        case t_RQPIN:
            success = asm_rqpin(params);
            break;

        case t_SAL:
            success = asm_sal(params);
            break;

        case t_SAR:
            success = asm_sar(params);
            break;

        case t_SCA:
            success = asm_sca(params);
            break;

        case t_SCAS:
            success = asm_scas(params);
            break;

        case t_SETBYTE:
            success = asm_setbyte(params);
            break;

        case t_SETCFRQ:
            success = asm_setcfrq(params);
            break;

        case t_SETCI:
            success = asm_setci(params);
            break;

        case t_SETCMOD:
            success = asm_setcmod(params);
            break;

        case t_SETCQ:
            success = asm_setcq(params);
            break;

        case t_SETCY:
            success = asm_setcy(params);
            break;

        case t_SETD:
            success = asm_setd(params);
            break;

        case t_SETDACS:
            success = asm_setdacs(params);
            break;

        case t_SETINT1:
            success = asm_setint1(params);
            break;

        case t_SETINT2:
            success = asm_setint2(params);
            break;

        case t_SETINT3:
            success = asm_setint3(params);
            break;

        case t_SETLUTS:
            success = asm_setluts(params);
            break;

        case t_SETNIB:
            success = asm_setnib(params);
            break;

        case t_SETPAT:
            success = asm_setpat(params);
            break;

        case t_SETPIV:
            success = asm_setpiv(params);
            break;

        case t_SETPIX:
            success = asm_setpix(params);
            break;

        case t_SETQ:
            success = asm_setq(params);
            break;

        case t_SETQ2:
            success = asm_setq2(params);
            break;

        case t_SETR:
            success = asm_setr(params);
            break;

        case t_SETS:
            success = asm_sets(params);
            break;

        case t_SETSCP:
            success = asm_setscp(params);
            break;

        case t_SETSE1:
            success = asm_setse1(params);
            break;

        case t_SETSE2:
            success = asm_setse2(params);
            break;

        case t_SETSE3:
            success = asm_setse3(params);
            break;

        case t_SETSE4:
            success = asm_setse4(params);
            break;

        case t_SETWORD:
            success = asm_setword(params);
            break;

        case t_SETXFRQ:
            success = asm_setxfrq(params);
            break;

        case t_SEUSSF:
            success = asm_seussf(params);
            break;

        case t_SEUSSR:
            success = asm_seussr(params);
            break;

        case t_SHL:
            success = asm_shl(params);
            break;

        case t_SHR:
            success = asm_shr(params);
            break;

        case t_SIGNX:
            success = asm_signx(params);
            break;

        case t_SKIP:
            success = asm_skip(params);
            break;

        case t_SKIPF:
            success = asm_skipf(params);
            break;

        case t_SPLITB:
            success = asm_splitb(params);
            break;

        case t_SPLITW:
            success = asm_splitw(params);
            break;

        case t_STALLI:
            success = asm_stalli(params);
            break;

        case t_SUB:
            success = asm_sub(params);
            break;

        case t_SUBR:
            success = asm_subr(params);
            break;

        case t_SUBS:
            success = asm_subs(params);
            break;

        case t_SUBSX:
            success = asm_subsx(params);
            break;

        case t_SUBX:
            success = asm_subx(params);
            break;

        case t_SUMC:
            success = asm_sumc(params);
            break;

        case t_SUMNC:
            success = asm_sumnc(params);
            break;

        case t_SUMNZ:
            success = asm_sumnz(params);
            break;

        case t_SUMZ:
            success = asm_sumz(params);
            break;

        case t_TEST:
            success = asm_test(params);
            break;

        case t_TESTB:
            success = asm_testb_w(params);
            break;

        case t_TESTBN:
            success = asm_testbn_w(params);
            break;

        case t_TESTN:
            success = asm_testn(params);
            break;

        case t_TESTNB:
            success = asm_testn(params);
            break;

        case t_TESTP:
            success = asm_testp_w(params);
            break;

        case t_TESTPN:
            success = asm_testpn_w(params);
            break;

        case t_TJF:
            success = asm_tjf(params);
            break;

        case t_TJNF:
            success = asm_tjnf(params);
            break;

        case t_TJNS:
            success = asm_tjns(params);
            break;

        case t_TJNZ:
            success = asm_tjnz(params);
            break;

        case t_TJS:
            success = asm_tjs(params);
            break;

        case t_TJV:
            success = asm_tjv(params);
            break;

        case t_TJZ:
            success = asm_tjz(params);
            break;

        case t_TRGINT1:
            success = asm_trgint1(params);
            break;

        case t_TRGINT2:
            success = asm_trgint2(params);
            break;

        case t_TRGINT3:
            success = asm_trgint3(params);
            break;

        case t_WAITATN:
            success = asm_waitatn(params);
            break;

        case t_WAITCT1:
            success = asm_waitct1(params);
            break;

        case t_WAITCT2:
            success = asm_waitct2(params);
            break;

        case t_WAITCT3:
            success = asm_waitct3(params);
            break;

        case t_WAITFBW:
            success = asm_waitfbw(params);
            break;

        case t_WAITINT:
            success = asm_waitint(params);
            break;

        case t_WAITPAT:
            success = asm_waitpat(params);
            break;

        case t_WAITSE1:
            success = asm_waitse1(params);
            break;

        case t_WAITSE2:
            success = asm_waitse2(params);
            break;

        case t_WAITSE3:
            success = asm_waitse3(params);
            break;

        case t_WAITSE4:
            success = asm_waitse4(params);
            break;

        case t_WAITX:
            success = asm_waitx(params);
            break;

        case t_WAITXFI:
            success = asm_waitxfi(params);
            break;

        case t_WAITXMT:
            success = asm_waitxmt(params);
            break;

        case t_WAITXRL:
            success = asm_waitxrl(params);
            break;

        case t_WAITXRO:
            success = asm_waitxro(params);
            break;

        case t_WFBYTE:
            success = asm_wfbyte(params);
            break;

        case t_WFLONG:
            success = asm_wflong(params);
            break;

        case t_WFWORD:
            success = asm_wfword(params);
            break;

        case t_WMLONG:
            success = asm_wmlong(params);
            break;

        case t_WRBYTE:
            success = asm_wrbyte(params);
            break;

        case t_WRC:
            success = asm_wrc(params);
            break;

        case t_WRFAST:
            success = asm_wrfast(params);
            break;

        case t_WRLONG:
            success = asm_wrlong(params);
            break;

        case t_WRLUT:
            success = asm_wrlut(params);
            break;

        case t_WRNC:
            success = asm_wrnc(params);
            break;

        case t_WRNZ:
            success = asm_wrnz(params);
            break;

        case t_WRPIN:
            success = asm_wrpin(params);
            break;

        case t_WRWORD:
            success = asm_wrword(params);
            break;

        case t_WRZ:
            success = asm_wrz(params);
            break;

        case t_WXPIN:
            success = asm_wxpin(params);
            break;

        case t_WYPIN:
            success = asm_wypin(params);
            break;

        case t_XCONT:
            success = asm_xcont(params);
            break;

        case t_XINIT:
            success = asm_xinit(params);
            break;

        case t_XOR:
            success = asm_xor(params);
            break;

        case t_XORO32:
            success = asm_xoro32(params);
            break;

        case t_XSTOP:
            success = asm_xstop(params);
            break;

        case t_XZERO:
            success = asm_xzero(params);
            break;

        case t_ZEROX:
            success = asm_zerox(params);
            break;


        case t__BYTE:
            success = asm_byte(params);
            break;

        case t__WORD:
            success = asm_word(params);
            break;

        case t__LONG:
            success = asm_long(params);
            break;

        case t__RES:
            success = asm_res(params);
            break;

        case t__FIT:
            success = asm_fit(params);
            break;

        case t__ORG:
            success = asm_org(params);
            break;

        case t__ORGH:
            success = asm_orgh(params);
            break;

        case t__ASSIGN:
            success = asm_assign(params);
            break;

        case t__DOLLAR:
            break;

        case t_PA:
        case t_PB:
        case t_PTRA:
        case t_PTRA_predec:
        case t_PTRA_postinc:
        case t_PTRB:
        case t_PTRB_predec:
        case t_PTRB_postinc:
            params.error = tr("Not an instruction: %1").arg(line);
            emit Error(params.lineno, params.error);
            break;

        case t__RET_:
        case t_IF_C:
        case t_IF_C_AND_NZ:
        case t_IF_C_AND_Z:
        case t_IF_C_EQ_Z:
        case t_IF_C_NE_Z:
        case t_IF_C_OR_NZ:
        case t_IF_C_OR_Z:
        case t_IF_NC:
        case t_IF_NC_AND_NZ:
        case t_IF_NC_AND_Z:
        case t_IF_NC_OR_NZ:
        case t_IF_NC_OR_Z:
        case t_IF_NZ:
        case t_IF_Z:
            params.error = tr("Multiple conditionals in line: %1").arg(line);
            emit Error(params.lineno, params.error);
            break;

        default:
            // Handle non-instruction tokens
            break;
        }

        if (success) {
            qDebug("%s: line=%-5d cnt=%-3d pc=$%06x [%08x] : %s", __func__,
                   params.lineno, params.cnt, params.curr_pc, params.IR.opcode, qPrintable(line));
            // Calculate next PC for regular instructions
            if (params.curr_pc < 0x400) {
                params.next_pc = params.curr_pc + 1;
            } else {
                params.next_pc = params.curr_pc + 4;
            }
            if (params.next_pc > params.last_pc)
                params.last_pc = params.next_pc;
        } else {
            qDebug("%s: line=%-5d cnt=%-3d pc=$%06x [%-8s] : %s", __func__,
                   params.lineno, params.cnt, params.curr_pc, "", qPrintable(line));
            if (!params.error.isEmpty())
                qDebug("%s: %s", __func__, qPrintable(params.error));
        }
    }

    return true;
}

/**
 * @brief Assemble a source file
 * @param filename name of the SPIN2 source
 * @return QByteArray containing the binary
 */
bool P2Asm::assemble(P2Params& params, const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        emit Error(0, tr("Can not open '%1' for reading.").arg(filename));
        return false;
    }
    QTextStream stream(&file);
    QStringList source;
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        source += line;
    }
    return assemble(params, source);
}

/**
 * @brief Convert a string of binary digits into an unsigned value
 * @param str binary digits
 * @return value of binary digits in str
 */
quint64 P2Asm::from_bin(int& pos, const QString& str, const QString& stop)
{
    static const QString digits = QLatin1String("01");
    quint64 value = 0;
    while (pos < str.length()) {
        QChar ch = str.at(pos);
        if (stop.contains(ch))
            return value;
        value = value * 2 + static_cast<uint>(digits.indexOf(ch, Qt::CaseInsensitive));
        pos++;
    }
    return value;
}

/**
 * @brief Convert a string of octal digits into an unsigned value
 * @param str octal digits
 * @return value of octal digits in str
 */
quint64 P2Asm::from_oct(int& pos, const QString& str, const QString& stop)
{
    static const QString digits = QLatin1String("01234567");
    quint64 value = 0;
    while (pos < str.length()) {
        QChar ch = str.at(pos);
        if (stop.contains(ch))
            return value;
        value = value * 8 + static_cast<uint>(digits.indexOf(ch, Qt::CaseInsensitive));
        pos++;
    }
    return value;
}

/**
 * @brief Convert a string of decimal digits into an unsigned value
 * @param str decimal digits
 * @return value of decimal digits in str
 */
quint64 P2Asm::from_dec(int& pos, const QString& str, const QString& stop)
{
    static const QString digits = QLatin1String("0123456789");
    quint64 value = 0;
    while (pos < str.length()) {
        QChar ch = str.at(pos);
        if (stop.contains(ch))
            return value;
        value = value * 10 + static_cast<uint>(digits.indexOf(ch, Qt::CaseInsensitive));
        pos++;
    }
    return value;
}

/**
 * @brief Convert a string of hexadecimal digits into an unsigned value
 * @param str hexadecimal digits
 * @return value of hexadecimal digits in str
 */
quint64 P2Asm::from_hex(int& pos, const QString& str, const QString& stop)
{
    static const QString digits = QLatin1String("0123456789ABCDEF");
    quint64 value = 0;
    while (pos < str.length()) {
        QChar ch = str.at(pos);
        if (stop.contains(ch))
            return value;
        value = value * 16 + static_cast<uint>(digits.indexOf(ch, Qt::CaseInsensitive));
        pos++;
    }
    return value;
}

/**
 * @brief Convert a string of ASCII characters into an unsigned value
 * @param str ASCII characters
 * @param stop characters to stop at
 * @return value as 64 bit unsigned
 */
quint64 P2Asm::from_str(int& pos, const QString& str, const QString& stop)
{
    quint64 value = 0;
    while (pos < str.length()) {
        QChar ch = str.at(pos);
        if (stop.contains(ch))
            return value;
        value = value * 256 + static_cast<uint>(ch.toLatin1());
        pos++;
    }
    return value;
}

void P2Asm::skip_spc(int &pos, const QString& str)
{
    while (pos < str.length() && str.at(pos).isSpace())
        pos++;
}

/**
 * @brief Parse an atomic part of an expression
 * @param params reference to P2Params
 * @param pos position in word where to start
 * @param str string to parse
 * @param tok token found when trying to tokenize the string
 * @return value of the atom
 */
QVariant P2Asm::parse_atom(P2Params& params, int& pos, const QString& str, p2_token_e tok)
{
    static const QString stop = QStringLiteral("*/\\+-&|^<>()\"");
    static const QString stop_str = QChar('"');

    // Unary minus
    bool minus = false;
    for (/* */; pos < str.length(); pos++) {
        skip_spc(pos, str);
        if (str.at(pos) == QChar('-')) {
            minus = !minus;
            continue;
        }
        break;
    }
    QString rem = str.mid(pos);
    P2AsmSymbol sym = params.symbols.value(rem);
    QVariant value;

    switch (tok) {

    case t_nothing:
        if (sym.isEmpty()) {
            skip_spc(pos, str);
            // Not a symbolic name
            if (pos >= str.length()) {
                value = 0;
            } else {
                switch (str.at(pos).toLatin1()) {
                case '$':   // Hexadecimal constant
                    pos++;
                    value = from_hex(pos, str, stop);
                    break;
                case '%':   // Binary constant
                    pos++;
                    value = from_bin(pos, str, stop);
                    break;
                case '0':   // Octal constant
                    pos++;
                    value = from_oct(pos, str, stop);
                    break;
                case '1': case '2': case '3':
                case '4': case '5': case '6':
                case '7': case '8': case '9':   // decimal constant
                    value = from_dec(pos, str, stop);
                    break;
                case '"':   // string constant
                    pos++;
                    value = from_str(pos, str, stop_str);
                    break;
                default:
                    // undefined symbol
                    value = 0;
                    break;
                }
            }
        } else {
            // Symbolic name
            value = sym.value<quint64>();
            if (value.isValid()) {
                sym.addReference(params.lineno);
            }
        }

        // Negate result?
        if (minus)
            value = - value.toULongLong();
        break;

    default:
        params.error = tr("Reserved word used as parameter: %1").arg(str);
        return value;
    }
    return value;
}

/**
 * @brief Parse an an expression of factors (multiplicators or divisors)
 * @param params reference to P2Params
 * @param pos position in word where to start
 * @param str string to parse
 * @param tok token found when trying to tokenize the string
 * @return value of the factors
 */
QVariant P2Asm::parse_factors(P2Params& params, int& pos, const QString& str, p2_token_e tok)
{
    QVariant value = parse_atom(params, pos, str, tok);
    for (;;) {
        skip_spc(pos, str);
        if (pos >= str.length())
            return value;

        QString s = str.mid(pos, 1);
        int op = factors_operators.indexOf(s);

        if (op < 0)
            return value;

        pos += 1;
        QVariant value2 = parse_atom(params, pos, str, tok);
        if (!value2.isValid()) {
            params.error = tr("Invalid character in expression (factors): %1").arg(str.mid(pos));
            return value;
        }

        switch (op) {
        case 0: // Multiplication
            value = value.toULongLong() * value2.toULongLong();
            break;
        case 1: // Division
            value = value.toULongLong() / value2.toULongLong();
            break;
        case 2: // Modulus
            value = value.toULongLong() % value2.toULongLong();
            break;
        }
    }
}


/**
 * @brief Parse an an expression of summands (addends and minuends)
 * @param params reference to P2Params
 * @param pos position in word where to start
 * @param str string to parse
 * @param tok token found when trying to tokenize the string
 * @return value of the factors
 */
QVariant P2Asm::parse_summands(P2Params& params, int& pos, const QString& str, p2_token_e tok)
{
    QVariant value = parse_factors(params, pos, str, tok);
    for (;;) {
        skip_spc(pos, str);
        if (pos >= str.length())
            return value;

        QString s = str.mid(pos, 1);
        int op = summands_operators.indexOf(s);

        if (op < 0)
            return value;

        pos += 1;
        QVariant value2 = parse_factors(params, pos, str, tok);
        if (!value2.isValid()) {
            params.error = tr("Invalid character in expression (summands): %1").arg(str.mid(pos));
            return value;
        }

        switch (op) {
        case 0: // Addition
            value = value.toULongLong() + value2.toULongLong();
            break;
        case 1: // Subtraction
            value = value.toULongLong() - value2.toULongLong();
            break;
        }
    }
}

/**
 * @brief Parse an an expression of binary operations (and, or, xor, left shift, right shift)
 * @param params reference to P2Params
 * @param pos position in word where to start
 * @param str string to parse
 * @param tok token found when trying to tokenize the string
 * @return value of the factors
 */
QVariant P2Asm::parse_binops(P2Params& params, int& pos, const QString& str, p2_token_e tok)
{
    QVariant value = parse_summands(params, pos, str, tok);
    for (;;) {
        skip_spc(pos, str);
        if (pos >= str.length())
            return value;

        QString s = str.mid(pos, 1);
        int op = logical_operators.indexOf(s);
        if (op < 0) {
            s = str.mid(pos, 2);
            op = logical_operators.indexOf(s);
            if (op < 0)
                return value;
            pos += 2;
        } else {
            pos += 1;
        }

        QVariant value2 = parse_summands(params, pos, str, tok);
        if (!value2.isValid()) {
            params.error = tr("Invalid character in expression (summands): %1").arg(str.mid(pos));
            return value;
        }

        switch (op) {
        case 0: // Logical NOT
            value = ~value.toULongLong();
            break;
        case 1: // Logical AND
            value = value.toULongLong() & value2.toULongLong();
            break;
        case 2: // Logical OR
            value = value.toULongLong() | value2.toULongLong();
            break;
        case 3: // Logical XOR
            value = value.toULongLong() ^ value2.toULongLong();
            break;
        case 4: // Logical SHL
            value = value.toULongLong() << value2.toULongLong();
            break;
        case 5: // Logical SHR
            value = value.toULongLong() >> value2.toULongLong();
            break;
        }
    }
}

/**
 * @brief Evaluate an expression
 * @param params reference to P2Params
 * @param imm_to put immediate flag into: -1=nowhere (default), 0=imm, 1=wz, 2=wc
 * @return QVariant with the value of the expression
 */
QVariant P2Asm::expression(P2Params& params, imm_to_e imm_to)
{
    QString str = params.words.value(params.idx);
    p2_token_e tok = params.tokens.value(params.idx);
    if (params.idx >= params.tokens.count())
        return QVariant();
    params.idx++;

    int pos = 0;
    skip_spc(pos, str);
    bool imm = str.startsWith(QChar('#'));
    if (imm)
        pos++;

    QVariant value;

    switch (tok) {
    case t_PA:
        value = offs_PA;
        break;
    case t_PB:
        value = offs_PB;
        break;
    case t_PTRA:
        value = offs_PTRA;
        break;
    case t_PTRB:
        value = offs_PTRB;
        break;
    default:
        value = parse_binops(params, pos, str, tok);
    }

    // Set immediate flag, if specified
    switch (imm_to) {
    case immediate_imm:
        params.IR.op.imm = imm;
        break;
    case immediate_wz:
        params.IR.op.wz = imm;
        break;
    case immediate_wc:
        params.IR.op.wc = imm;
        break;
    default:
        // silence compiler warnings
        break;
    }

    return value;
}

/**
 * @brief Expect end of line, i.e. no more parameters
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::end_of_line(P2Params& params, bool binary)
{
    if (params.idx < params.cnt) {
        // ignore extra parameters?
        params.error = tr("Found extra parameters: %1")
                       .arg(params.words.mid(params.idx).join(QChar::Space));
        return false;
    }

    if (binary) {
        if (params.curr_pc < 0x400) {
            params.binary.l[params.curr_pc] = params.IR.opcode;
        } else {
            params.binary.l[params.curr_pc / 4] = params.IR.opcode;
        }
    }
    return true;
}

/**
 * @brief Optional WC, WZ, or WCZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::optional_wcz(P2Params& params)
{
    if (params.idx < params.cnt) {
        p2_token_e tok = params.tokens.value(params.idx);
        switch (tok) {
        case t_WC:
            params.IR.op.wc = true;
            params.idx++;
            break;
        case t_WZ:
            params.IR.op.wz = true;
            params.idx++;
            break;
        case t_WCZ:
            params.IR.op.wc = true;
            params.IR.op.wz = true;
            params.idx++;
            break;
        default:
            params.error = tr("Unexpected flag update '%1' in: %2")
                           .arg(params.words.value(params.idx))
                           .arg(params.line);
            return false;
        }
    }
    return true;
}

/**
 * @brief Optional WC
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::optional_wc(P2Params& params)
{
    if (params.idx < params.cnt) {
        p2_token_e tok = params.tokens.value(params.idx);
        switch (tok) {
        case t_WC:
            params.IR.op.wc = true;
            params.idx++;
            break;
        default:
            params.error = tr("Unexpected flag update '%1' in: %2")
                           .arg(params.words.value(params.idx))
                           .arg(params.line);
            return false;
        }
    }
    return true;
}

/**
 * @brief Optional WZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::optional_wz(P2Params& params)
{
    if (params.idx < params.cnt) {
        p2_token_e tok = params.tokens.value(params.idx);
        switch (tok) {
        case t_WZ:
            params.IR.op.wz = true;
            params.idx++;
            break;
        default:
            params.error = tr("Unexpected flag update '%1' in: %2")
                           .arg(params.words.value(params.idx))
                           .arg(params.line);
            return false;
        }
    }
    return true;
}

/**
 * @brief Assignment operation
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::asm_assign(P2Params& params)
{
    params.idx++;
    QVariant value = expression(params);
    params.symbols.setValue(params.symbol, value);
    return end_of_line(params, false);
}

/**
 * @brief Origin operation
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::asm_org(P2Params& params)
{
    params.idx++;
    QVariant value = expression(params);
    if (value.isNull())
        value = params.last_pc;
    params.next_pc = value.toUInt();
    params.symbols.setValue(params.symbol, value);
    return end_of_line(params, false);
}

/**
 * @brief Origin high operation
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::asm_orgh(P2Params& params)
{
    params.idx++;
    QVariant value = expression(params);
    if (value.isNull())
        value = 0x00400;
    params.next_pc = value.toUInt();
    params.symbols.setValue(params.symbol, value);
    return end_of_line(params, false);
}

/**
 * @brief Expect instruction with optional WC, WZ, or WCZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_with_cz(P2Params& params)
{
    params.idx++;
    optional_wcz(params);
    return end_of_line(params);
}

/**
 * @brief Expect instruction with optional WC
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_with_c(P2Params& params)
{
    params.idx++;
    optional_wc(params);
    return end_of_line(params);
}


/**
 * @brief Expect instruction with optional WZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_with_z(P2Params& params)
{
    params.idx++;
    optional_wz(params);
    return end_of_line(params);
}

/**
 * @brief Expect no more parameters after instruction
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_inst(P2Params& params)
{
    return end_of_line(params);
}


/**
 * @brief Expect parameters for D and {#}S
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_d_imm_s(P2Params& params)
{
    params.idx++;
    QVariant dst = expression(params);
    QVariant src = expression(params, immediate_imm);
    params.IR.op.dst = dst.toUInt();
    params.IR.op.src = dst.toUInt();
    return end_of_line(params);
}

/**
 * @brief Expect parameters for D and optional WC, WZ, or WCZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_d_cz(P2Params& params)
{
    params.idx++;
    QVariant dst = expression(params);
    params.IR.op.dst = dst.toUInt();
    optional_wcz(params);
    return end_of_line(params);
}

/**
 * @brief Expect optional WC, WZ, or WCZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_cz(P2Params& params)
{
    params.idx++;
    optional_wcz(params);
    return end_of_line(params);
}

/**
 * @brief Expect conditional for C, conditional for Z, and optional WC, WZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_cccc_zzzz_cz(P2Params& params)
{
    params.idx++;
    p2_cond_e cccc = conditional(params, params.tokens.value(params.idx));
    params.idx++;
    p2_cond_e zzzz = conditional(params, params.tokens.value(params.idx));
    params.idx++;
    params.IR.op.dst = static_cast<P2LONG>((cccc << 4) | zzzz);
    optional_wcz(params);
    return end_of_line(params);
}

/**
 * @brief Expect parameters for D
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_d(P2Params& params)
{
    params.idx++;
    QVariant dst = expression(params);
    params.IR.op.dst = dst.toUInt();
    return end_of_line(params);
}

/**
 * @brief Expect parameters for {#}D setting WZ for immediate
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_wz_d(P2Params& params)
{
    params.idx++;
    QVariant dst = expression(params, immediate_wz);
    params.IR.op.dst = dst.toUInt();
    return end_of_line(params);
}

/**
 * @brief Expect parameters for {#}D
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_imm_d(P2Params& params)
{
    params.idx++;
    QVariant dst = expression(params, immediate_imm);
    params.IR.op.dst = dst.toUInt();
    return end_of_line(params);
}

/**
 * @brief Expect parameters for {#}D, and optional WC, WZ, or WCZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_imm_d_cz(P2Params& params)
{
    params.idx++;
    QVariant dst = expression(params, immediate_imm);
    params.IR.op.dst = dst.toUInt();
    optional_wcz(params);
    return end_of_line(params);
}

/**
 * @brief Expect parameters for {#}D, and optional WC
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_imm_d_c(P2Params& params)
{
    params.idx++;
    QVariant dst = expression(params, immediate_imm);
    params.IR.op.dst = dst.toUInt();
    optional_wc(params);
    return end_of_line(params);
}

/**
 * @brief Expect parameters for D, and {#}S, and optional WC, WZ, or WCZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_d_imm_s_cz(P2Params& params)
{
    params.idx++;
    QVariant dst = expression(params);
    QVariant src = expression(params, immediate_imm);
    params.IR.op.dst = dst.toUInt();
    params.IR.op.src = dst.toUInt();
    optional_wcz(params);
    return end_of_line(params);
}

/**
 * @brief Expect parameters for D, and {#}S, and optional WC
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_d_imm_s_c(P2Params& params)
{
    params.idx++;
    QVariant dst = expression(params);
    QVariant src = expression(params, immediate_imm);
    params.IR.op.dst = dst.toUInt();
    params.IR.op.src = dst.toUInt();
    optional_wc(params);
    return end_of_line(params);
}

/**
 * @brief Expect parameters for D, and {#}S, and optional WZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_d_imm_s_z(P2Params& params)
{
    params.idx++;
    QVariant dst = expression(params);
    QVariant src = expression(params, immediate_imm);
    params.IR.op.dst = dst.toUInt();
    params.IR.op.src = src.toUInt();
    optional_wz(params);
    return end_of_line(params);
}

/**
 * @brief Expect parameters for {#}D, and {#}S
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_wz_d_imm_s(P2Params& params)
{
    params.idx++;
    QVariant dst = expression(params, immediate_wz);
    QVariant src = expression(params, immediate_imm);
    params.IR.op.dst = dst.toUInt();
    params.IR.op.src = src.toUInt();
    return end_of_line(params);
}

/**
 * @brief Expect parameters for D, and {#}S, and #N (0 … 7)
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_d_imm_s_nnn(P2Params& params)
{
    params.idx++;
    QVariant dst = expression(params);
    QVariant src = expression(params, immediate_imm);
    params.IR.op.dst = dst.toUInt();
    params.IR.op.src = src.toUInt();
    if (params.idx < params.cnt) {
        QVariant n = expression(params);
        if (n.isNull()) {
            params.error = tr("Expected immediate #n in: %1")
                           .arg(params.line);
            return false;
        }
    } else {
        params.error = tr("Missing immediate #n in: %1")
                       .arg(params.line);
        return false;
    }
    return end_of_line(params);
}

/**
 * @brief Expect parameters for D, and {#}S, and #N (0 … 1)
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_d_imm_s_n(P2Params& params)
{
    params.idx++;
    QVariant dst = expression(params);
    QVariant src = expression(params, immediate_imm);
    params.IR.op.dst = dst.toUInt();
    params.IR.op.src = dst.toUInt();
    if (params.idx < params.cnt) {
        QVariant n = expression(params);
        if (n.isNull()) {
            params.error = tr("Expected immediate #n in: %1")
                           .arg(params.line);
        }
    } else {
        params.error = tr("Missing immediate #n in: %1")
                       .arg(params.line);
    }
    return end_of_line(params);
}

/**
 * @brief Expect parameters for {#}S
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_imm_s(P2Params& params)
{
    params.idx++;
    QVariant src = expression(params, immediate_imm);
    params.IR.op.src = src.toUInt();
    return end_of_line(params);
}

/**
 * @brief Expect parameters for {#}S, and optional WC
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_imm_s_cz(P2Params& params)
{
    params.idx++;
    QVariant src = expression(params, immediate_imm);
    params.IR.op.src = src.toUInt();
    return end_of_line(params);
}


/**
 * @brief Expect parameters for #AAAAAAAAAAAA (19 bit address for CALL/CALLA/CALLB/LOC)
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_ptr_pc_abs(P2Params& params)
{
    params.idx++;
    p2_token_e dst = params.tokens.value(params.idx);
    switch (dst) {
    case t_PA:
        break;
    case t_PB:
        params.IR.op.wz = true;
        break;
    case t_PTRA:
        params.IR.op.wc = true;
        break;
    case t_PTRB:
        params.IR.op.wz = true;
        params.IR.op.wc = true;
        break;
    default:
        params.error = tr("Invalid pointer parameter: %1")
                       .arg(params.words.mid(params.idx).join(QChar::Space));
        return false;
    }
    params.idx++;

    quint64 addr = expression(params).toULongLong();
    params.IR.opcode |= addr & A20MASK;

    return end_of_line(params);
}

/**
 * @brief Expect parameters for #AAAAAAAAAAAA (19 bit address for CALL/CALLA/CALLB)
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_pc_abs(P2Params& params)
{
    params.idx++;
    quint64 addr = expression(params).toULongLong();
    params.IR.opcode |= addr & A20MASK;

    return end_of_line(params);
}

/**
 * @brief Expect parameters for #AAAAAAAAAAAA (23 bit address for AUGD/AUGS)
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_imm23(P2Params& params, QVector<p2_inst_e> aug)
{
    params.idx++;
    quint64 addr = expression(params).toULongLong();
    params.IR.op.inst = static_cast<p2_inst_e>(params.IR.op.inst | aug[(addr >> 21) & 3]);

    return end_of_line(params);
}

/**
 * @brief Expect one or more bytes of data
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::asm_byte(P2Params& params)
{
    params.idx++;
    while (params.idx < params.cnt) {
        QVariant data = expression(params);
        qDebug("%s: byte %#x", __func__, data.toUInt());
        params.IR.opcode = data.toUInt();
    }
    return end_of_line(params);
}

/**
 * @brief Expect one or more bytes of data
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::asm_word(P2Params& params)
{
    params.idx++;
    while (params.idx < params.cnt) {
        QVariant data = expression(params);
        qDebug("%s: word %#x", __func__, data.toUInt());
        params.IR.opcode = data.toUInt();
    }
    return end_of_line(params);
}

/**
 * @brief Expect one or more bytes of data
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::asm_long(P2Params& params)
{
    params.idx++;
    while (params.idx < params.cnt) {
        QVariant data = expression(params);
        params.IR.opcode = data.toUInt();
    }
    return end_of_line(params);
}

/**
 * @brief Expect one or more bytes of data
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::asm_res(P2Params& params)
{
    params.idx++;
    while (params.idx < params.cnt) {
        QVariant data = expression(params);
        params.IR.opcode = 0;
    }
    return end_of_line(params);
}

/**
 * @brief Expect an address which is checked for being >= params.curr_pc
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::asm_fit(P2Params& params)
{
    params.idx++;
    while (params.idx < params.cnt) {
        QVariant data = expression(params);
    }
    return end_of_line(params);
}

/**
 * @brief No operation.
 *
 * 0000 0000000 000 000000000 000000000
 *
 * NOP
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_nop(P2Params& params)
{
    params.idx++;
    params.IR.opcode = p2_ROR;
    return parse_inst(params);
}

/**
 * @brief Rotate right.
 *
 * EEEE 0000000 CZI DDDDDDDDD SSSSSSSSS
 *
 * ROR     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [31:0]  of ({D[31:0], D[31:0]}     >> S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[0].
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_ror(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_ROR;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Rotate left.
 *
 * EEEE 0000001 CZI DDDDDDDDD SSSSSSSSS
 *
 * ROL     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [63:32] of ({D[31:0], D[31:0]}     << S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[31].
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rol(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_ROR;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Shift right.
 *
 * EEEE 0000010 CZI DDDDDDDDD SSSSSSSSS
 *
 * SHR     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [31:0]  of ({32'b0, D[31:0]}       >> S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[0].
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_shr(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_SHR;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Shift left.
 *
 * EEEE 0000011 CZI DDDDDDDDD SSSSSSSSS
 *
 * SHL     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [63:32] of ({D[31:0], 32'b0}       << S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[31].
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_shl(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_SHL;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Rotate carry right.
 *
 * EEEE 0000100 CZI DDDDDDDDD SSSSSSSSS
 *
 * RCR     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [31:0]  of ({{32{C}}, D[31:0]}     >> S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[0].
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rcr(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_RCR;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Rotate carry left.
 *
 * EEEE 0000101 CZI DDDDDDDDD SSSSSSSSS
 *
 * RCL     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [63:32] of ({D[31:0], {32{C}}}     << S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[31].
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rcl(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_RCL;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Shift arithmetic right.
 *
 * EEEE 0000110 CZI DDDDDDDDD SSSSSSSSS
 *
 * SAR     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [31:0]  of ({{32{D[31]}}, D[31:0]} >> S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[0].
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_sar(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_SAR;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Shift arithmetic left.
 *
 * EEEE 0000111 CZI DDDDDDDDD SSSSSSSSS
 *
 * SAL     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [63:32] of ({D[31:0], {32{D[0]}}}  << S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[31].
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_sal(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_SAL;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Add S into D.
 *
 * EEEE 0001000 CZI DDDDDDDDD SSSSSSSSS
 *
 * ADD     D,{#}S   {WC/WZ/WCZ}
 *
 * D = D + S.
 * C = carry of (D + S).
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_add(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_ADD;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Add (S + C) into D, extended.
 *
 * EEEE 0001001 CZI DDDDDDDDD SSSSSSSSS
 *
 * ADDX    D,{#}S   {WC/WZ/WCZ}
 *
 * D = D + S + C.
 * C = carry of (D + S + C).
 * Z = Z AND (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_addx(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_ADDX;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Add S into D, signed.
 *
 * EEEE 0001010 CZI DDDDDDDDD SSSSSSSSS
 *
 * ADDS    D,{#}S   {WC/WZ/WCZ}
 *
 * D = D + S.
 * C = correct sign of (D + S).
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_adds(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_ADDS;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Add (S + C) into D, signed and extended.
 *
 * EEEE 0001011 CZI DDDDDDDDD SSSSSSSSS
 *
 * ADDSX   D,{#}S   {WC/WZ/WCZ}
 *
 * D = D + S + C.
 * C = correct sign of (D + S + C).
 * Z = Z AND (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_addsx(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_ADDSX;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Subtract S from D.
 *
 * EEEE 0001100 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUB     D,{#}S   {WC/WZ/WCZ}
 *
 * D = D - S.
 * C = borrow of (D - S).
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_sub(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_SUB;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Subtract (S + C) from D, extended.
 *
 * EEEE 0001101 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUBX    D,{#}S   {WC/WZ/WCZ}
 *
 * D = D - (S + C).
 * C = borrow of (D - (S + C)).
 * Z = Z AND (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_subx(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_SUBX;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Subtract S from D, signed.
 *
 * EEEE 0001110 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUBS    D,{#}S   {WC/WZ/WCZ}
 *
 * D = D - S.
 * C = correct sign of (D - S).
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_subs(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_SUBS;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Subtract (S + C) from D, signed and extended.
 *
 * EEEE 0001111 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUBSX   D,{#}S   {WC/WZ/WCZ}
 *
 * D = D - (S + C).
 * C = correct sign of (D - (S + C)).
 * Z = Z AND (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_subsx(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_SUBSX;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Compare D to S.
 *
 * EEEE 0010000 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMP     D,{#}S   {WC/WZ/WCZ}
 *
 * C = borrow of (D - S).
 * Z = (D == S).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_cmp(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_CMP;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Compare D to (S + C), extended.
 *
 * EEEE 0010001 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMPX    D,{#}S   {WC/WZ/WCZ}
 *
 * C = borrow of (D - (S + C)).
 * Z = Z AND (D == S + C).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_cmpx(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_CMPX;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Compare D to S, signed.
 *
 * EEEE 0010010 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMPS    D,{#}S   {WC/WZ/WCZ}
 *
 * C = correct sign of (D - S).
 * Z = (D == S).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_cmps(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_CMPS;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Compare D to (S + C), signed and extended.
 *
 * EEEE 0010011 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMPSX   D,{#}S   {WC/WZ/WCZ}
 *
 * C = correct sign of (D - (S + C)).
 * Z = Z AND (D == S + C).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_cmpsx(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_CMPSX;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Compare S to D (reverse).
 *
 * EEEE 0010100 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMPR    D,{#}S   {WC/WZ/WCZ}
 *
 * C = borrow of (S - D).
 * Z = (D == S).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_cmpr(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_CMPR;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Compare D to S, get MSB of difference into C.
 *
 * EEEE 0010101 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMPM    D,{#}S   {WC/WZ/WCZ}
 *
 * C = MSB of (D - S).
 * Z = (D == S).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_cmpm(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_CMPM;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Subtract D from S (reverse).
 *
 * EEEE 0010110 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUBR    D,{#}S   {WC/WZ/WCZ}
 *
 * D = S - D.
 * C = borrow of (S - D).
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_subr(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_SUBR;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Compare and subtract S from D if D >= S.
 *
 * EEEE 0010111 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMPSUB  D,{#}S   {WC/WZ/WCZ}
 *
 * If D => S then D = D - S and C = 1, else D same and C = 0.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_cmpsub(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_CMPSUB;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Force D >= S.
 *
 * EEEE 0011000 CZI DDDDDDDDD SSSSSSSSS
 *
 * FGE     D,{#}S   {WC/WZ/WCZ}
 *
 * If D < S then D = S and C = 1, else D same and C = 0.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_fge(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_FGE;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Force D <= S.
 *
 * EEEE 0011001 CZI DDDDDDDDD SSSSSSSSS
 *
 * FLE     D,{#}S   {WC/WZ/WCZ}
 *
 * If D > S then D = S and C = 1, else D same and C = 0.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_fle(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_FLE;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Force D >= S, signed.
 *
 * EEEE 0011010 CZI DDDDDDDDD SSSSSSSSS
 *
 * FGES    D,{#}S   {WC/WZ/WCZ}
 *
 * If D < S then D = S and C = 1, else D same and C = 0.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_fges(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_FGES;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Force D <= S, signed.
 *
 * EEEE 0011011 CZI DDDDDDDDD SSSSSSSSS
 *
 * FLES    D,{#}S   {WC/WZ/WCZ}
 *
 * If D > S then D = S and C = 1, else D same and C = 0.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_fles(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_FLES;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Sum +/-S into D by  C.
 *
 * EEEE 0011100 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUMC    D,{#}S   {WC/WZ/WCZ}
 *
 * If C = 1 then D = D - S, else D = D + S.
 * C = correct sign of (D +/- S).
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_sumc(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_SUMC;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Sum +/-S into D by !C.
 *
 * EEEE 0011101 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUMNC   D,{#}S   {WC/WZ/WCZ}
 *
 * If C = 0 then D = D - S, else D = D + S.
 * C = correct sign of (D +/- S).
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_sumnc(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_SUMNC;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Sum +/-S into D by  Z.
 *
 * EEEE 0011110 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUMZ    D,{#}S   {WC/WZ/WCZ}
 *
 * If Z = 1 then D = D - S, else D = D + S.
 * C = correct sign of (D +/- S).
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_sumz(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_SUMZ;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Sum +/-S into D by !Z.
 *
 * EEEE 0011111 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUMNZ   D,{#}S   {WC/WZ/WCZ}
 *
 * If Z = 0 then D = D - S, else D = D + S.
 * C = correct sign of (D +/- S).
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_sumnz(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_SUMNZ;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Test bit S[4:0] of  D, write to C/Z.
 *
 * EEEE 0100000 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTB   D,{#}S         WC/WZ
 *
 * C/Z =          D[S[4:0]].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_testb_w(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_TESTB_W;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Test bit S[4:0] of !D, write to C/Z.
 *
 * EEEE 0100001 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTBN  D,{#}S         WC/WZ
 *
 * C/Z =         !D[S[4:0]].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_testbn_w(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_TESTBN_W;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Test bit S[4:0] of  D, AND into C/Z.
 *
 * EEEE 0100010 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTB   D,{#}S     ANDC/ANDZ
 *
 * C/Z = C/Z AND  D[S[4:0]].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_testb_and(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_TESTB_AND;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Test bit S[4:0] of !D, AND into C/Z.
 *
 * EEEE 0100011 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTBN  D,{#}S     ANDC/ANDZ
 *
 * C/Z = C/Z AND !D[S[4:0]].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_testbn_and(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_TESTBN_AND;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Test bit S[4:0] of  D, OR  into C/Z.
 *
 * EEEE 0100100 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTB   D,{#}S       ORC/ORZ
 *
 * C/Z = C/Z OR   D[S[4:0]].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_testb_or(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_TESTB_OR;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Test bit S[4:0] of !D, OR  into C/Z.
 *
 * EEEE 0100101 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTBN  D,{#}S       ORC/ORZ
 *
 * C/Z = C/Z OR  !D[S[4:0]].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_testbn_or(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_TESTBN_OR;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Test bit S[4:0] of  D, XOR into C/Z.
 *
 * EEEE 0100110 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTB   D,{#}S     XORC/XORZ
 *
 * C/Z = C/Z XOR  D[S[4:0]].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_testb_xor(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_TESTB_XOR;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Test bit S[4:0] of !D, XOR into C/Z.
 *
 * EEEE 0100111 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTBN  D,{#}S     XORC/XORZ
 *
 * C/Z = C/Z XOR !D[S[4:0]].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_testbn_xor(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_TESTBN_XOR;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Bit S[4:0] of D = 0,    C,Z = D[S[4:0]].
 *
 * EEEE 0100000 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITL    D,{#}S         {WCZ}
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_bitl(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_BITL;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Bit S[4:0] of D = 1,    C,Z = D[S[4:0]].
 *
 * EEEE 0100001 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITH    D,{#}S         {WCZ}
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_bith(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_BITH;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Bit S[4:0] of D = C,    C,Z = D[S[4:0]].
 *
 * EEEE 0100010 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITC    D,{#}S         {WCZ}
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_bitc(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_BITC;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Bit S[4:0] of D = !C,   C,Z = D[S[4:0]].
 *
 * EEEE 0100011 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNC   D,{#}S         {WCZ}
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_bitnc(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_BITNC;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Bit S[4:0] of D = Z,    C,Z = D[S[4:0]].
 *
 * EEEE 0100100 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITZ    D,{#}S         {WCZ}
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_bitz(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_BITZ;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Bit S[4:0] of D = !Z,   C,Z = D[S[4:0]].
 *
 * EEEE 0100101 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNZ   D,{#}S         {WCZ}
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_bitnz(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_BITNZ;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Bit S[4:0] of D = RND,  C,Z = D[S[4:0]].
 *
 * EEEE 0100110 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITRND  D,{#}S         {WCZ}
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_bitrnd(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_BITRND;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Bit S[4:0] of D = !bit, C,Z = D[S[4:0]].
 *
 * EEEE 0100111 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNOT  D,{#}S         {WCZ}
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_bitnot(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_BITNOT;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief AND S into D.
 *
 * EEEE 0101000 CZI DDDDDDDDD SSSSSSSSS
 *
 * AND     D,{#}S   {WC/WZ/WCZ}
 *
 * D = D & S.
 * C = parity of result.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_and(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_AND;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief AND !S into D.
 *
 * EEEE 0101001 CZI DDDDDDDDD SSSSSSSSS
 *
 * ANDN    D,{#}S   {WC/WZ/WCZ}
 *
 * D = D & !S.
 * C = parity of result.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_andn(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_ANDN;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief OR S into D.
 *
 * EEEE 0101010 CZI DDDDDDDDD SSSSSSSSS
 *
 * OR      D,{#}S   {WC/WZ/WCZ}
 *
 * D = D | S.
 * C = parity of result.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_or(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OR;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief XOR S into D.
 *
 * EEEE 0101011 CZI DDDDDDDDD SSSSSSSSS
 *
 * XOR     D,{#}S   {WC/WZ/WCZ}
 *
 * D = D ^ S.
 * C = parity of result.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_xor(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_XOR;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Mux C into each D bit that is '1' in S.
 *
 * EEEE 0101100 CZI DDDDDDDDD SSSSSSSSS
 *
 * MUXC    D,{#}S   {WC/WZ/WCZ}
 *
 * D = (!S & D ) | (S & {32{ C}}).
 * C = parity of result.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_muxc(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_MUXC;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Mux !C into each D bit that is '1' in S.
 *
 * EEEE 0101101 CZI DDDDDDDDD SSSSSSSSS
 *
 * MUXNC   D,{#}S   {WC/WZ/WCZ}
 *
 * D = (!S & D ) | (S & {32{!C}}).
 * C = parity of result.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_muxnc(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_MUXNC;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Mux Z into each D bit that is '1' in S.
 *
 * EEEE 0101110 CZI DDDDDDDDD SSSSSSSSS
 *
 * MUXZ    D,{#}S   {WC/WZ/WCZ}
 *
 * D = (!S & D ) | (S & {32{ Z}}).
 * C = parity of result.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_muxz(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_MUXZ;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Mux !Z into each D bit that is '1' in S.
 *
 * EEEE 0101111 CZI DDDDDDDDD SSSSSSSSS
 *
 * MUXNZ   D,{#}S   {WC/WZ/WCZ}
 *
 * D = (!S & D ) | (S & {32{!Z}}).
 * C = parity of result.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_muxnz(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_MUXNZ;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Move S into D.
 *
 * EEEE 0110000 CZI DDDDDDDDD SSSSSSSSS
 *
 * MOV     D,{#}S   {WC/WZ/WCZ}
 *
 * D = S.
 * C = S[31].
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_mov(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_MOV;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Get !S into D.
 *
 * EEEE 0110001 CZI DDDDDDDDD SSSSSSSSS
 *
 * NOT     D,{#}S   {WC/WZ/WCZ}
 *
 * D = !S.
 * C = !S[31].
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_not(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_NOT;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Get absolute value of S into D.
 *
 * EEEE 0110010 CZI DDDDDDDDD SSSSSSSSS
 *
 * ABS     D,{#}S   {WC/WZ/WCZ}
 *
 * D = ABS(S).
 * C = S[31].
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_abs(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_ABS;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Negate S into D.
 *
 * EEEE 0110011 CZI DDDDDDDDD SSSSSSSSS
 *
 * NEG     D,{#}S   {WC/WZ/WCZ}
 *
 * D = -S.
 * C = MSB of result.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_neg(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_NEG;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Negate S by  C into D.
 *
 * EEEE 0110100 CZI DDDDDDDDD SSSSSSSSS
 *
 * NEGC    D,{#}S   {WC/WZ/WCZ}
 *
 * If C = 1 then D = -S, else D = S.
 * C = MSB of result.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_negc(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_NEGC;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Negate S by !C into D.
 *
 * EEEE 0110101 CZI DDDDDDDDD SSSSSSSSS
 *
 * NEGNC   D,{#}S   {WC/WZ/WCZ}
 *
 * If C = 0 then D = -S, else D = S.
 * C = MSB of result.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_negnc(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_NEGNC;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Negate S by  Z into D.
 *
 * EEEE 0110110 CZI DDDDDDDDD SSSSSSSSS
 *
 * NEGZ    D,{#}S   {WC/WZ/WCZ}
 *
 * If Z = 1 then D = -S, else D = S.
 * C = MSB of result.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_negz(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_NEGZ;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Negate S by !Z into D.
 *
 * EEEE 0110111 CZI DDDDDDDDD SSSSSSSSS
 *
 * NEGNZ   D,{#}S   {WC/WZ/WCZ}
 *
 * If Z = 0 then D = -S, else D = S.
 * C = MSB of result.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_negnz(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_NEGNZ;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Increment with modulus.
 *
 * EEEE 0111000 CZI DDDDDDDDD SSSSSSSSS
 *
 * INCMOD  D,{#}S   {WC/WZ/WCZ}
 *
 * If D = S then D = 0 and C = 1, else D = D + 1 and C = 0.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_incmod(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_INCMOD;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Decrement with modulus.
 *
 * EEEE 0111001 CZI DDDDDDDDD SSSSSSSSS
 *
 * DECMOD  D,{#}S   {WC/WZ/WCZ}
 *
 * If D = 0 then D = S and C = 1, else D = D - 1 and C = 0.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_decmod(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_DECMOD;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Zero-extend D above bit S[4:0].
 *
 * EEEE 0111010 CZI DDDDDDDDD SSSSSSSSS
 *
 * ZEROX   D,{#}S   {WC/WZ/WCZ}
 *
 * C = MSB of result.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_zerox(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_ZEROX;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Sign-extend D from bit S[4:0].
 *
 * EEEE 0111011 CZI DDDDDDDDD SSSSSSSSS
 *
 * SIGNX   D,{#}S   {WC/WZ/WCZ}
 *
 * C = MSB of result.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_signx(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_SIGNX;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Get bit position of top-most '1' in S into D.
 *
 * EEEE 0111100 CZI DDDDDDDDD SSSSSSSSS
 *
 * ENCOD   D,{#}S   {WC/WZ/WCZ}
 *
 * D = position of top '1' in S (0..31).
 * C = (S != 0).
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_encod(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_ENCOD;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Get number of '1's in S into D.
 *
 * EEEE 0111101 CZI DDDDDDDDD SSSSSSSSS
 *
 * ONES    D,{#}S   {WC/WZ/WCZ}
 *
 * D = number of '1's in S (0..32).
 * C = LSB of result.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_ones(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_ONES;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Test D with S.
 *
 * EEEE 0111110 CZI DDDDDDDDD SSSSSSSSS
 *
 * TEST    D,{#}S   {WC/WZ/WCZ}
 *
 * C = parity of (D & S).
 * Z = ((D & S) == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_test(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_TEST;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Test D with !S.
 *
 * EEEE 0111111 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTN   D,{#}S   {WC/WZ/WCZ}
 *
 * C = parity of (D & !S).
 * Z = ((D & !S) == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_testn(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_TESTN;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Set S[3:0] into nibble N in D, keeping rest of D same.
 *
 * EEEE 100000N NNI DDDDDDDDD SSSSSSSSS
 *
 * SETNIB  D,{#}S,#N
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setnib(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_SETNIB_0;
    return parse_d_imm_s_nnn(params);
}

/**
 * @brief Set S[3:0] into nibble established by prior ALTSN instruction.
 *
 * EEEE 1000000 00I 000000000 SSSSSSSSS
 *
 * SETNIB  {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setnib_altsn(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_SETNIB_0;
    return parse_imm_s(params);
}

/**
 * @brief Get nibble N of S into D.
 *
 * EEEE 100001N NNI DDDDDDDDD SSSSSSSSS
 *
 * GETNIB  D,{#}S,#N
 *
 * D = {28'b0, S.NIBBLE[N]).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_getnib(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_GETNIB_0;
    return parse_d_imm_s_nnn(params);
}

/**
 * @brief Get nibble established by prior ALTGN instruction into D.
 *
 * EEEE 1000010 000 DDDDDDDDD 000000000
 *
 * GETNIB  D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_getnib_altgn(P2Params& params)
{
    params.IR.op.inst = p2_GETNIB_0;
    return parse_imm_s(params);
}

/**
 * @brief Rotate-left nibble N of S into D.
 *
 * EEEE 100010N NNI DDDDDDDDD SSSSSSSSS
 *
 * ROLNIB  D,{#}S,#N
 *
 * D = {D[27:0], S.NIBBLE[N]).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rolnib(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_ROLNIB_0;
    return parse_d_imm_s_nnn(params);
}

/**
 * @brief Rotate-left nibble established by prior ALTGN instruction into D.
 *
 * EEEE 1000100 000 DDDDDDDDD 000000000
 *
 * ROLNIB  D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rolnib_altgn(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_ROLNIB_0;
    return parse_d(params);
}

/**
 * @brief Set S[7:0] into byte N in D, keeping rest of D same.
 *
 * EEEE 1000110 NNI DDDDDDDDD SSSSSSSSS
 *
 * SETBYTE D,{#}S,#N
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setbyte(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_SETBYTE;
    return parse_d_imm_s_nnn(params);
}

/**
 * @brief Set S[7:0] into byte established by prior ALTSB instruction.
 *
 * EEEE 1000110 00I 000000000 SSSSSSSSS
 *
 * SETBYTE {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setbyte_altsb(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_SETBYTE;
    return parse_imm_s(params);
}

/**
 * @brief Get byte N of S into D.
 *
 * EEEE 1000111 NNI DDDDDDDDD SSSSSSSSS
 *
 * GETBYTE D,{#}S,#N
 *
 * D = {24'b0, S.BYTE[N]).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_getbyte(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_GETBYTE;
    return parse_d_imm_s_nnn(params);
}

/**
 * @brief Get byte established by prior ALTGB instruction into D.
 *
 * EEEE 1000111 000 DDDDDDDDD 000000000
 *
 * GETBYTE D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_getbyte_altgb(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_GETBYTE;
    return parse_d(params);
}

/**
 * @brief Rotate-left byte N of S into D.
 *
 * EEEE 1001000 NNI DDDDDDDDD SSSSSSSSS
 *
 * ROLBYTE D,{#}S,#N
 *
 * D = {D[23:0], S.BYTE[N]).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rolbyte(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_ROLBYTE;
    return parse_d_imm_s_nnn(params);
}

/**
 * @brief Rotate-left byte established by prior ALTGB instruction into D.
 *
 * EEEE 1001000 000 DDDDDDDDD 000000000
 *
 * ROLBYTE D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rolbyte_altgb(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_ROLBYTE;
    return parse_d(params);
}

/**
 * @brief Set S[15:0] into word N in D, keeping rest of D same.
 *
 * EEEE 1001001 0NI DDDDDDDDD SSSSSSSSS
 *
 * SETWORD D,{#}S,#N
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setword(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_SETWORD;
    return parse_d_imm_s_n(params);
}

/**
 * @brief Set S[15:0] into word established by prior ALTSW instruction.
 *
 * EEEE 1001001 00I 000000000 SSSSSSSSS
 *
 * SETWORD {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setword_altsw(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_SETWORD_ALTSW;
    return parse_imm_s(params);
}

/**
 * @brief Get word N of S into D.
 *
 * EEEE 1001001 1NI DDDDDDDDD SSSSSSSSS
 *
 * GETWORD D,{#}S,#N
 *
 * D = {16'b0, S.WORD[N]).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_getword(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_GETWORD;
    return parse_d_imm_s_n(params);
}

/**
 * @brief Get word established by prior ALTGW instruction into D.
 *
 * EEEE 1001001 100 DDDDDDDDD 000000000
 *
 * GETWORD D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_getword_altgw(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_GETWORD_ALTGW;
    return parse_d(params);
}

/**
 * @brief Rotate-left word N of S into D.
 *
 * EEEE 1001010 0NI DDDDDDDDD SSSSSSSSS
 *
 * ROLWORD D,{#}S,#N
 *
 * D = {D[15:0], S.WORD[N]).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rolword(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ROLWORD;
    return parse_d_imm_s_n(params);
}

/**
 * @brief Rotate-left word established by prior ALTGW instruction into D.
 *
 * EEEE 1001010 000 DDDDDDDDD 000000000
 *
 * ROLWORD D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rolword_altgw(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ROLWORD_ALTGW;
    return parse_d(params);
}

/**
 * @brief Alter subsequent SETNIB instruction.
 *
 * EEEE 1001010 10I DDDDDDDDD SSSSSSSSS
 *
 * ALTSN   D,{#}S
 *
 * Next D field = (D[11:3] + S) & $1FF, N field = D[2:0].
 * D += sign-extended S[17:9].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_altsn(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ALTSN;
    return parse_d_imm_s(params);
}

/**
 * @brief Alter subsequent SETNIB instruction.
 *
 * EEEE 1001010 101 DDDDDDDDD 000000000
 *
 * ALTSN   D
 *
 * Next D field = D[11:3], N field = D[2:0].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_altsn_d(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ALTSN;
    return parse_d(params);
}

/**
 * @brief Alter subsequent GETNIB/ROLNIB instruction.
 *
 * EEEE 1001010 11I DDDDDDDDD SSSSSSSSS
 *
 * ALTGN   D,{#}S
 *
 * Next S field = (D[11:3] + S) & $1FF, N field = D[2:0].
 * D += sign-extended S[17:9].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_altgn(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ALTGN;
    return parse_d_imm_s(params);
}

/**
 * @brief Alter subsequent GETNIB/ROLNIB instruction.
 *
 * EEEE 1001010 111 DDDDDDDDD 000000000
 *
 * ALTGN   D
 *
 * Next S field = D[11:3], N field = D[2:0].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_altgn_d(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ALTGN;
    return parse_d(params);
}

/**
 * @brief Alter subsequent SETBYTE instruction.
 *
 * EEEE 1001011 00I DDDDDDDDD SSSSSSSSS
 *
 * ALTSB   D,{#}S
 *
 * Next D field = (D[10:2] + S) & $1FF, N field = D[1:0].
 * D += sign-extended S[17:9].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_altsb(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ALTSB;
    return parse_d_imm_s(params);
}

/**
 * @brief Alter subsequent SETBYTE instruction.
 *
 * EEEE 1001011 001 DDDDDDDDD 000000000
 *
 * ALTSB   D
 *
 * Next D field = D[10:2], N field = D[1:0].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_altsb_d(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ALTSB;
    return parse_d(params);
}

/**
 * @brief Alter subsequent GETBYTE/ROLBYTE instruction.
 *
 * EEEE 1001011 01I DDDDDDDDD SSSSSSSSS
 *
 * ALTGB   D,{#}S
 *
 * Next S field = (D[10:2] + S) & $1FF, N field = D[1:0].
 * D += sign-extended S[17:9].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_altgb(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ALTGB;
    return parse_d_imm_s(params);
}

/**
 * @brief Alter subsequent GETBYTE/ROLBYTE instruction.
 *
 * EEEE 1001011 011 DDDDDDDDD 000000000
 *
 * ALTGB   D
 *
 * Next S field = D[10:2], N field = D[1:0].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_altgb_d(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ALTGB;
    return parse_d(params);
}

/**
 * @brief Alter subsequent SETWORD instruction.
 *
 * EEEE 1001011 10I DDDDDDDDD SSSSSSSSS
 *
 * ALTSW   D,{#}S
 *
 * Next D field = (D[9:1] + S) & $1FF, N field = D[0].
 * D += sign-extended S[17:9].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_altsw(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ALTSW;
    return parse_d_imm_s(params);
}

/**
 * @brief Alter subsequent SETWORD instruction.
 *
 * EEEE 1001011 101 DDDDDDDDD 000000000
 *
 * ALTSW   D
 *
 * Next D field = D[9:1], N field = D[0].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_altsw_d(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ALTSW;
    return parse_d(params);
}

/**
 * @brief Alter subsequent GETWORD/ROLWORD instruction.
 *
 * EEEE 1001011 11I DDDDDDDDD SSSSSSSSS
 *
 * ALTGW   D,{#}S
 *
 * Next S field = ((D[9:1] + S) & $1FF), N field = D[0].
 * D += sign-extended S[17:9].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_altgw(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ALTGW;
    return parse_d_imm_s(params);
}

/**
 * @brief Alter subsequent GETWORD/ROLWORD instruction.
 *
 * EEEE 1001011 111 DDDDDDDDD 000000000
 *
 * ALTGW   D
 *
 * Next S field = D[9:1], N field = D[0].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_altgw_d(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ALTGW;
    return parse_d(params);
}

/**
 * @brief Alter result register address (normally D field) of next instruction to (D + S) & $1FF.
 *
 * EEEE 1001100 00I DDDDDDDDD SSSSSSSSS
 *
 * ALTR    D,{#}S
 *
 * D += sign-extended S[17:9].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_altr(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ALTR;
    return parse_d_imm_s(params);
}

/**
 * @brief Alter result register address (normally D field) of next instruction to D[8:0].
 *
 * EEEE 1001100 001 DDDDDDDDD 000000000
 *
 * ALTR    D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_altr_d(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ALTD;
    return parse_d(params);
}

/**
 * @brief Alter D field of next instruction to (D + S) & $1FF.
 *
 * EEEE 1001100 01I DDDDDDDDD SSSSSSSSS
 *
 * ALTD    D,{#}S
 *
 * D += sign-extended S[17:9].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_altd(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ALTD;
    return parse_d_imm_s(params);
}

/**
 * @brief Alter D field of next instruction to D[8:0].
 *
 * EEEE 1001100 011 DDDDDDDDD 000000000
 *
 * ALTD    D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_altd_d(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ALTD;
    return parse_d(params);
}

/**
 * @brief Alter S field of next instruction to (D + S) & $1FF.
 *
 * EEEE 1001100 10I DDDDDDDDD SSSSSSSSS
 *
 * ALTS    D,{#}S
 *
 * D += sign-extended S[17:9].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_alts(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ALTS;
    return parse_d_imm_s(params);
}

/**
 * @brief Alter S field of next instruction to D[8:0].
 *
 * EEEE 1001100 101 DDDDDDDDD 000000000
 *
 * ALTS    D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_alts_d(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ALTS;
    return parse_d(params);
}

/**
 * @brief Alter D field of next instruction to (D[13:5] + S) & $1FF.
 *
 * EEEE 1001100 11I DDDDDDDDD SSSSSSSSS
 *
 * ALTB    D,{#}S
 *
 * D += sign-extended S[17:9].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_altb(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ALTB;
    return parse_d_imm_s(params);
}

/**
 * @brief Alter D field of next instruction to D[13:5].
 *
 * EEEE 1001100 111 DDDDDDDDD 000000000
 *
 * ALTB    D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_altb_d(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ALTB;
    return parse_d(params);
}

/**
 * @brief Substitute next instruction's I/R/D/S fields with fields from D, per S.
 *
 * EEEE 1001101 00I DDDDDDDDD SSSSSSSSS
 *
 * ALTI    D,{#}S
 *
 * Modify D per S.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_alti(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ALTI;
    return parse_d_imm_s(params);
}

/**
 * @brief Execute D in place of next instruction.
 *
 * EEEE 1001101 001 DDDDDDDDD 101100100
 *
 * ALTI    D
 *
 * D stays same.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_alti_d(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ALTI;
    return parse_d(params);
}

/**
 * @brief Set R field of D to S[8:0].
 *
 * EEEE 1001101 01I DDDDDDDDD SSSSSSSSS
 *
 * SETR    D,{#}S
 *
 * D = {D[31:28], S[8:0], D[18:0]}.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setr(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_SETR;
    return parse_d_imm_s(params);
}

/**
 * @brief Set D field of D to S[8:0].
 *
 * EEEE 1001101 10I DDDDDDDDD SSSSSSSSS
 *
 * SETD    D,{#}S
 *
 * D = {D[31:18], S[8:0], D[8:0]}.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setd(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_SETD;
    return parse_d_imm_s(params);
}

/**
 * @brief Set S field of D to S[8:0].
 *
 * EEEE 1001101 11I DDDDDDDDD SSSSSSSSS
 *
 * SETS    D,{#}S
 *
 * D = {D[31:9], S[8:0]}.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_sets(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_SETS;
    return parse_d_imm_s(params);
}

/**
 * @brief Decode S[4:0] into D.
 *
 * EEEE 1001110 00I DDDDDDDDD SSSSSSSSS
 *
 * DECOD   D,{#}S
 *
 * D = 1 << S[4:0].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_decod(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_DECOD;
    return parse_d_imm_s(params);
}

/**
 * @brief Decode D[4:0] into D.
 *
 * EEEE 1001110 000 DDDDDDDDD DDDDDDDDD
 *
 * DECOD   D
 *
 * D = 1 << D[4:0].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_decod_d(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_DECOD;
    return parse_d(params);
}

/**
 * @brief Get LSB-justified bit mask of size (S[4:0] + 1) into D.
 *
 * EEEE 1001110 01I DDDDDDDDD SSSSSSSSS
 *
 * BMASK   D,{#}S
 *
 * D = ($0000_0002 << S[4:0]) - 1.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_bmask(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_BMASK;
    return parse_d_imm_s(params);
}

/**
 * @brief Get LSB-justified bit mask of size (D[4:0] + 1) into D.
 *
 * EEEE 1001110 010 DDDDDDDDD DDDDDDDDD
 *
 * BMASK   D
 *
 * D = ($0000_0002 << D[4:0]) - 1.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_bmask_d(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_BMASK;
    return parse_d(params);
}

/**
 * @brief Iterate CRC value in D using C and polynomial in S.
 *
 * EEEE 1001110 10I DDDDDDDDD SSSSSSSSS
 *
 * CRCBIT  D,{#}S
 *
 * If (C XOR D[0]) then D = (D >> 1) XOR S, else D = (D >> 1).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_crcbit(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_CRCBIT;
    return parse_d_imm_s(params);
}

/**
 * @brief Iterate CRC value in D using Q[31:28] and polynomial in S.
 *
 * EEEE 1001110 11I DDDDDDDDD SSSSSSSSS
 *
 * CRCNIB  D,{#}S
 *
 * Like CRCBIT, but 4x.
 * Q = Q << 4.
 * Use SETQ+CRCNIB+CRCNIB+CRCNIB.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_crcnib(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_CRCNIB;
    return parse_d_imm_s(params);
}

/**
 * @brief For each non-zero bit pair in S, copy that bit pair into the corresponding D bits, else leave that D bit pair the same.
 *
 * EEEE 1001111 00I DDDDDDDDD SSSSSSSSS
 *
 * MUXNITS D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_muxnits(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_MUXNITS;
    return parse_d_imm_s(params);
}

/**
 * @brief For each non-zero nibble in S, copy that nibble into the corresponding D nibble, else leave that D nibble the same.
 *
 * EEEE 1001111 01I DDDDDDDDD SSSSSSSSS
 *
 * MUXNIBS D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_muxnibs(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_MUXNIBS;
    return parse_d_imm_s(params);
}

/**
 * @brief Used after SETQ.
 *
 * EEEE 1001111 10I DDDDDDDDD SSSSSSSSS
 *
 * MUXQ    D,{#}S
 *
 * For each '1' bit in Q, copy the corresponding bit in S into D.
 * D = (D & !Q) | (S & Q).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_muxq(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_MUXQ;
    return parse_d_imm_s(params);
}

/**
 * @brief Move bytes within D, per S.
 *
 * EEEE 1001111 11I DDDDDDDDD SSSSSSSSS
 *
 * MOVBYTS D,{#}S
 *
 * D = {D.BYTE[S[7:6]], D.BYTE[S[5:4]], D.BYTE[S[3:2]], D.BYTE[S[1:0]]}.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_movbyts(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_MOVBYTS;
    return parse_d_imm_s(params);
}

/**
 * @brief D = unsigned (D[15:0] * S[15:0]).
 *
 * EEEE 1010000 0ZI DDDDDDDDD SSSSSSSSS
 *
 * MUL     D,{#}S          {WZ}
 *
 * Z = (S == 0) | (D == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_mul(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_MUL;
    return parse_d_imm_s_z(params);
}

/**
 * @brief D = signed (D[15:0] * S[15:0]).
 *
 * EEEE 1010000 1ZI DDDDDDDDD SSSSSSSSS
 *
 * MULS    D,{#}S          {WZ}
 *
 * Z = (S == 0) | (D == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_muls(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_MULS;
    return parse_d_imm_s_z(params);
}

/**
 * @brief Next instruction's S value = unsigned (D[15:0] * S[15:0]) >> 16.
 *
 * EEEE 1010001 0ZI DDDDDDDDD SSSSSSSSS
 *
 * SCA     D,{#}S          {WZ}
 *
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_sca(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_SCA;
    return parse_d_imm_s_z(params);
}

/**
 * @brief Next instruction's S value = signed (D[15:0] * S[15:0]) >> 14.
 *
 * EEEE 1010001 1ZI DDDDDDDDD SSSSSSSSS
 *
 * SCAS    D,{#}S          {WZ}
 *
 * In this scheme, $4000 = 1.0 and $C000 = -1.0.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_scas(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_SCAS;
    return parse_d_imm_s_z(params);
}

/**
 * @brief Add bytes of S into bytes of D, with $FF saturation.
 *
 * EEEE 1010010 00I DDDDDDDDD SSSSSSSSS
 *
 * ADDPIX  D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_addpix(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ADDPIX;
    return parse_d_imm_s(params);
}

/**
 * @brief Multiply bytes of S into bytes of D, where $FF = 1.
 *
 * EEEE 1010010 01I DDDDDDDDD SSSSSSSSS
 *
 * MULPIX  D,{#}S
 *
 * 0.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_mulpix(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_MULPIX;
    return parse_d_imm_s(params);
}

/**
 * @brief Alpha-blend bytes of S into bytes of D, using SETPIV value.
 *
 * EEEE 1010010 10I DDDDDDDDD SSSSSSSSS
 *
 * BLNPIX  D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_blnpix(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_BLNPIX;
    return parse_d_imm_s(params);
}

/**
 * @brief Mix bytes of S into bytes of D, using SETPIX and SETPIV values.
 *
 * EEEE 1010010 11I DDDDDDDDD SSSSSSSSS
 *
 * MIXPIX  D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_mixpix(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_MIXPIX;
    return parse_d_imm_s(params);
}

/**
 * @brief Set CT1 event to trigger on CT = D + S.
 *
 * EEEE 1010011 00I DDDDDDDDD SSSSSSSSS
 *
 * ADDCT1  D,{#}S
 *
 * Adds S into D.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_addct1(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ADDCT1;
    return parse_d_imm_s(params);
}

/**
 * @brief Set CT2 event to trigger on CT = D + S.
 *
 * EEEE 1010011 01I DDDDDDDDD SSSSSSSSS
 *
 * ADDCT2  D,{#}S
 *
 * Adds S into D.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_addct2(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ADDCT2;
    return parse_d_imm_s(params);
}

/**
 * @brief Set CT3 event to trigger on CT = D + S.
 *
 * EEEE 1010011 10I DDDDDDDDD SSSSSSSSS
 *
 * ADDCT3  D,{#}S
 *
 * Adds S into D.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_addct3(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_ADDCT3;
    return parse_d_imm_s(params);
}

/**
 * @brief Write only non-$00 bytes in D[31:0] to hub address {#}S/PTRx.
 *
 * EEEE 1010011 11I DDDDDDDDD SSSSSSSSS
 *
 * WMLONG  D,{#}S/P
 *
 * Prior SETQ/SETQ2 invokes cog/LUT block transfer.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_wmlong(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_WMLONG;
    return parse_d_imm_s(params);
}

/**
 * @brief Read smart pin S[5:0] result "Z" into D, don't acknowledge smart pin ("Q" in RQPIN means "quiet").
 *
 * EEEE 1010100 C0I DDDDDDDDD SSSSSSSSS
 *
 * RQPIN   D,{#}S          {WC}
 *
 * C = modal result.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rqpin(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_RQPIN;
    return parse_d_imm_s_c(params);
}

/**
 * @brief Read smart pin S[5:0] result "Z" into D, acknowledge smart pin.
 *
 * EEEE 1010100 C1I DDDDDDDDD SSSSSSSSS
 *
 * RDPIN   D,{#}S          {WC}
 *
 * C = modal result.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rdpin(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_RDPIN;
    return parse_d_imm_s_c(params);
}

/**
 * @brief Read LUT data from address S[8:0] into D.
 *
 * EEEE 1010101 CZI DDDDDDDDD SSSSSSSSS
 *
 * RDLUT   D,{#}S   {WC/WZ/WCZ}
 *
 * C = MSB of data.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rdlut(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_RDLUT;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Read zero-extended byte from hub address {#}S/PTRx into D.
 *
 * EEEE 1010110 CZI DDDDDDDDD SSSSSSSSS
 *
 * RDBYTE  D,{#}S/P {WC/WZ/WCZ}
 *
 * C = MSB of byte.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rdbyte(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_RDBYTE;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Read zero-extended word from hub address {#}S/PTRx into D.
 *
 * EEEE 1010111 CZI DDDDDDDDD SSSSSSSSS
 *
 * RDWORD  D,{#}S/P {WC/WZ/WCZ}
 *
 * C = MSB of word.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rdword(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_RDWORD;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Read long from hub address {#}S/PTRx into D.
 *
 * EEEE 1011000 CZI DDDDDDDDD SSSSSSSSS
 *
 * RDLONG  D,{#}S/P {WC/WZ/WCZ}
 *
 * C = MSB of long.
 * *   Prior SETQ/SETQ2 invokes cog/LUT block transfer.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rdlong(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_RDLONG;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Read long from hub address --PTRA into D.
 *
 * EEEE 1011000 CZ1 DDDDDDDDD 101011111
 *
 * POPA    D        {WC/WZ/WCZ}
 *
 * C = MSB of long.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_popa(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_RDLONG;
    params.IR.op.imm = true;
    params.IR.op.src = 0x15f;
    return parse_d_cz(params);
}

/**
 * @brief Read long from hub address --PTRB into D.
 *
 * EEEE 1011000 CZ1 DDDDDDDDD 111011111
 *
 * POPB    D        {WC/WZ/WCZ}
 *
 * C = MSB of long.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_popb(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_RDLONG;
    params.IR.op.imm = true;
    params.IR.op.src = 0x1df;
    return parse_d_cz(params);
}

/**
 * @brief Call to S** by writing {C, Z, 10'b0, PC[19:0]} to D.
 *
 * EEEE 1011001 CZI DDDDDDDDD SSSSSSSSS
 *
 * CALLD   D,{#}S   {WC/WZ/WCZ}
 *
 * C = S[31], Z = S[30].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_calld(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_CALLD;
    return parse_d_imm_s_cz(params);
}

/**
 * @brief Resume from INT3.
 *
 * EEEE 1011001 110 111110000 111110001
 *
 * RESI3
 *
 * (CALLD $1F0,$1F1 WC,WZ).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_resi3(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_CALLD;
    params.IR.op.wc = true;
    params.IR.op.wz = true;
    params.IR.op.dst = offs_IJMP3;
    params.IR.op.src = offs_IRET3;
    return parse_inst(params);
}

/**
 * @brief Resume from INT2.
 *
 * EEEE 1011001 110 111110010 111110011
 *
 * RESI2
 *
 * (CALLD $1F2,$1F3 WC,WZ).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_resi2(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_CALLD;
    params.IR.op.wc = true;
    params.IR.op.wz = true;
    params.IR.op.dst = offs_IJMP2;
    params.IR.op.src = offs_IRET2;
    return parse_inst(params);
}

/**
 * @brief Resume from INT1.
 *
 * EEEE 1011001 110 111110100 111110101
 *
 * RESI1
 *
 * (CALLD $1F4,$1F5 WC,WZ).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_resi1(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_CALLD;
    params.IR.op.wc = true;
    params.IR.op.wz = true;
    params.IR.op.dst = offs_IJMP1;
    params.IR.op.src = offs_IRET1;
    return parse_inst(params);
}

/**
 * @brief Resume from INT0.
 *
 * EEEE 1011001 110 111111110 111111111
 *
 * RESI0
 *
 * (CALLD $1FE,$1FF WC,WZ).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_resi0(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_CALLD;
    params.IR.op.wc = true;
    params.IR.op.wz = true;
    params.IR.op.dst = offs_INA;
    params.IR.op.src = offs_INB;
    return parse_inst(params);
}

/**
 * @brief Return from INT3.
 *
 * EEEE 1011001 110 111111111 111110001
 *
 * RETI3
 *
 * (CALLD $1FF,$1F1 WC,WZ).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_reti3(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_CALLD;
    params.IR.op.wc = true;
    params.IR.op.wz = true;
    params.IR.op.dst = offs_INB;
    params.IR.op.src = offs_IRET3;
    return parse_inst(params);
}

/**
 * @brief Return from INT2.
 *
 * EEEE 1011001 110 111111111 111110011
 *
 * RETI2
 *
 * (CALLD $1FF,$1F3 WC,WZ).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_reti2(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_CALLD;
    params.IR.op.wc = true;
    params.IR.op.wz = true;
    params.IR.op.dst = offs_INB;
    params.IR.op.src = offs_IRET2;
    return parse_inst(params);
}

/**
 * @brief Return from INT1.
 *
 * EEEE 1011001 110 111111111 111110101
 *
 * RETI1
 *
 * (CALLD $1FF,$1F5 WC,WZ).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_reti1(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_CALLD;
    params.IR.op.wc = true;
    params.IR.op.wz = true;
    params.IR.op.dst = offs_INB;
    params.IR.op.src = offs_IRET1;
    return parse_inst(params);
}

/**
 * @brief Return from INT0.
 *
 * EEEE 1011001 110 111111111 111111111
 *
 * RETI0
 *
 * (CALLD $1FF,$1FF WC,WZ).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_reti0(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_CALLD;
    params.IR.op.wc = true;
    params.IR.op.wz = true;
    params.IR.op.dst = offs_INB;
    params.IR.op.src = offs_INB;
    return parse_inst(params);
}

/**
 * @brief Call to S** by pushing {C, Z, 10'b0, PC[19:0]} onto stack, copy D to PA.
 *
 * EEEE 1011010 0LI DDDDDDDDD SSSSSSSSS
 *
 * CALLPA  {#}D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_callpa(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_CALLPA;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Call to S** by pushing {C, Z, 10'b0, PC[19:0]} onto stack, copy D to PB.
 *
 * EEEE 1011010 1LI DDDDDDDDD SSSSSSSSS
 *
 * CALLPB  {#}D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_callpb(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_CALLPB;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Decrement D and jump to S** if result is zero.
 *
 * EEEE 1011011 00I DDDDDDDDD SSSSSSSSS
 *
 * DJZ     D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_djz(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_DJZ;
    return parse_d_imm_s(params);
}

/**
 * @brief Decrement D and jump to S** if result is not zero.
 *
 * EEEE 1011011 01I DDDDDDDDD SSSSSSSSS
 *
 * DJNZ    D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_djnz(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_DJNZ;
    return parse_d_imm_s(params);
}

/**
 * @brief Decrement D and jump to S** if result is $FFFF_FFFF.
 *
 * EEEE 1011011 10I DDDDDDDDD SSSSSSSSS
 *
 * DJF     D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_djf(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_DJF;
    return parse_d_imm_s(params);
}

/**
 * @brief Decrement D and jump to S** if result is not $FFFF_FFFF.
 *
 * EEEE 1011011 11I DDDDDDDDD SSSSSSSSS
 *
 * DJNF    D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_djnf(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_DJNF;
    return parse_d_imm_s(params);
}

/**
 * @brief Increment D and jump to S** if result is zero.
 *
 * EEEE 1011100 00I DDDDDDDDD SSSSSSSSS
 *
 * IJZ     D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_ijz(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_IJZ;
    return parse_d_imm_s(params);
}

/**
 * @brief Increment D and jump to S** if result is not zero.
 *
 * EEEE 1011100 01I DDDDDDDDD SSSSSSSSS
 *
 * IJNZ    D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_ijnz(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_IJNZ;
    return parse_d_imm_s(params);
}

/**
 * @brief Test D and jump to S** if D is zero.
 *
 * EEEE 1011100 10I DDDDDDDDD SSSSSSSSS
 *
 * TJZ     D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_tjz(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_TJZ;
    return parse_d_imm_s(params);
}

/**
 * @brief Test D and jump to S** if D is not zero.
 *
 * EEEE 1011100 11I DDDDDDDDD SSSSSSSSS
 *
 * TJNZ    D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_tjnz(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_TJNZ;
    return parse_d_imm_s(params);
}

/**
 * @brief Test D and jump to S** if D is full (D = $FFFF_FFFF).
 *
 * EEEE 1011101 00I DDDDDDDDD SSSSSSSSS
 *
 * TJF     D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_tjf(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_TJF;
    return parse_d_imm_s(params);
}

/**
 * @brief Test D and jump to S** if D is not full (D != $FFFF_FFFF).
 *
 * EEEE 1011101 01I DDDDDDDDD SSSSSSSSS
 *
 * TJNF    D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_tjnf(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_TJNF;
    return parse_d_imm_s(params);
}

/**
 * @brief Test D and jump to S** if D is signed (D[31] = 1).
 *
 * EEEE 1011101 10I DDDDDDDDD SSSSSSSSS
 *
 * TJS     D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_tjs(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_TJS;
    return parse_d_imm_s(params);
}

/**
 * @brief Test D and jump to S** if D is not signed (D[31] = 0).
 *
 * EEEE 1011101 11I DDDDDDDDD SSSSSSSSS
 *
 * TJNS    D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_tjns(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_TJNS;
    return parse_d_imm_s(params);
}

/**
 * @brief Test D and jump to S** if D overflowed (D[31] != C, C = 'correct sign' from last addition/subtraction).
 *
 * EEEE 1011110 00I DDDDDDDDD SSSSSSSSS
 *
 * TJV     D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_tjv(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_TJV;
    return parse_d_imm_s(params);
}

/**
 * @brief Jump to S** if INT event flag is set.
 *
 * EEEE 1011110 01I 000000000 SSSSSSSSS
 *
 * JINT    {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jint(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JINT;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if CT1 event flag is set.
 *
 * EEEE 1011110 01I 000000001 SSSSSSSSS
 *
 * JCT1    {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jct1(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JCT1;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if CT2 event flag is set.
 *
 * EEEE 1011110 01I 000000010 SSSSSSSSS
 *
 * JCT2    {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jct2(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JCT2;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if CT3 event flag is set.
 *
 * EEEE 1011110 01I 000000011 SSSSSSSSS
 *
 * JCT3    {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jct3(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JCT3;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if SE1 event flag is set.
 *
 * EEEE 1011110 01I 000000100 SSSSSSSSS
 *
 * JSE1    {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jse1(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JSE1;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if SE2 event flag is set.
 *
 * EEEE 1011110 01I 000000101 SSSSSSSSS
 *
 * JSE2    {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jse2(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JSE2;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if SE3 event flag is set.
 *
 * EEEE 1011110 01I 000000110 SSSSSSSSS
 *
 * JSE3    {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jse3(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JSE3;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if SE4 event flag is set.
 *
 * EEEE 1011110 01I 000000111 SSSSSSSSS
 *
 * JSE4    {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jse4(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JSE4;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if PAT event flag is set.
 *
 * EEEE 1011110 01I 000001000 SSSSSSSSS
 *
 * JPAT    {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jpat(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JPAT;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if FBW event flag is set.
 *
 * EEEE 1011110 01I 000001001 SSSSSSSSS
 *
 * JFBW    {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jfbw(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JFBW;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if XMT event flag is set.
 *
 * EEEE 1011110 01I 000001010 SSSSSSSSS
 *
 * JXMT    {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jxmt(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JXMT;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if XFI event flag is set.
 *
 * EEEE 1011110 01I 000001011 SSSSSSSSS
 *
 * JXFI    {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jxfi(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JXFI;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if XRO event flag is set.
 *
 * EEEE 1011110 01I 000001100 SSSSSSSSS
 *
 * JXRO    {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jxro(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JXRO;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if XRL event flag is set.
 *
 * EEEE 1011110 01I 000001101 SSSSSSSSS
 *
 * JXRL    {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jxrl(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JXRL;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if ATN event flag is set.
 *
 * EEEE 1011110 01I 000001110 SSSSSSSSS
 *
 * JATN    {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jatn(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JATN;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if QMT event flag is set.
 *
 * EEEE 1011110 01I 000001111 SSSSSSSSS
 *
 * JQMT    {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jqmt(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JQMT;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if INT event flag is clear.
 *
 * EEEE 1011110 01I 000010000 SSSSSSSSS
 *
 * JNINT   {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jnint(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JNINT;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if CT1 event flag is clear.
 *
 * EEEE 1011110 01I 000010001 SSSSSSSSS
 *
 * JNCT1   {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jnct1(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JNCT1;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if CT2 event flag is clear.
 *
 * EEEE 1011110 01I 000010010 SSSSSSSSS
 *
 * JNCT2   {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jnct2(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JNCT2;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if CT3 event flag is clear.
 *
 * EEEE 1011110 01I 000010011 SSSSSSSSS
 *
 * JNCT3   {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jnct3(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JNCT3;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if SE1 event flag is clear.
 *
 * EEEE 1011110 01I 000010100 SSSSSSSSS
 *
 * JNSE1   {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jnse1(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JNSE1;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if SE2 event flag is clear.
 *
 * EEEE 1011110 01I 000010101 SSSSSSSSS
 *
 * JNSE2   {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jnse2(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JNSE2;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if SE3 event flag is clear.
 *
 * EEEE 1011110 01I 000010110 SSSSSSSSS
 *
 * JNSE3   {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jnse3(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JNSE3;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if SE4 event flag is clear.
 *
 * EEEE 1011110 01I 000010111 SSSSSSSSS
 *
 * JNSE4   {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jnse4(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JNSE4;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if PAT event flag is clear.
 *
 * EEEE 1011110 01I 000011000 SSSSSSSSS
 *
 * JNPAT   {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jnpat(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JNPAT;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if FBW event flag is clear.
 *
 * EEEE 1011110 01I 000011001 SSSSSSSSS
 *
 * JNFBW   {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jnfbw(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JNFBW;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if XMT event flag is clear.
 *
 * EEEE 1011110 01I 000011010 SSSSSSSSS
 *
 * JNXMT   {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jnxmt(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JNXMT;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if XFI event flag is clear.
 *
 * EEEE 1011110 01I 000011011 SSSSSSSSS
 *
 * JNXFI   {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jnxfi(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JNXFI;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if XRO event flag is clear.
 *
 * EEEE 1011110 01I 000011100 SSSSSSSSS
 *
 * JNXRO   {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jnxro(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JNXRO;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if XRL event flag is clear.
 *
 * EEEE 1011110 01I 000011101 SSSSSSSSS
 *
 * JNXRL   {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jnxrl(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JNXRL;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if ATN event flag is clear.
 *
 * EEEE 1011110 01I 000011110 SSSSSSSSS
 *
 * JNATN   {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jnatn(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JNATN;
    return parse_imm_s(params);
}

/**
 * @brief Jump to S** if QMT event flag is clear.
 *
 * EEEE 1011110 01I 000011111 SSSSSSSSS
 *
 * JNQMT   {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jnqmt(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_OPDST;
    params.IR.op.dst = p2_OPDST_JNQMT;
    return parse_imm_s(params);
}

/**
 * @brief <empty>.
 *
 * EEEE 1011110 1LI DDDDDDDDD SSSSSSSSS
 *
 * <empty> {#}D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_1011110_1(P2Params& params)
{
    params.idx++;
    params.IR.op9.inst = p2_1011110_10;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief <empty>.
 *
 * EEEE 1011111 0LI DDDDDDDDD SSSSSSSSS
 *
 * <empty> {#}D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_1011111_0(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_1011111_0;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Set pin pattern for PAT event.
 *
 * EEEE 1011111 1LI DDDDDDDDD SSSSSSSSS
 *
 * SETPAT  {#}D,{#}S
 *
 * C selects INA/INB, Z selects =/!=, D provides mask value, S provides match value.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setpat(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_SETPAT;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Write D to mode register of smart pin S[5:0], acknowledge smart pin.
 *
 * EEEE 1100000 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRPIN   {#}D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_wrpin(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_WRPIN;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Acknowledge smart pin S[5:0].
 *
 * EEEE 1100000 01I 000000001 SSSSSSSSS
 *
 * AKPIN   {#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_akpin(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_WRPIN;
    params.IR.op8.wz = true;
    params.IR.op.dst = 1;
    return parse_imm_s(params);
}

/**
 * @brief Write D to parameter "X" of smart pin S[5:0], acknowledge smart pin.
 *
 * EEEE 1100000 1LI DDDDDDDDD SSSSSSSSS
 *
 * WXPIN   {#}D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_wxpin(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_WXPIN;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Write D to parameter "Y" of smart pin S[5:0], acknowledge smart pin.
 *
 * EEEE 1100001 0LI DDDDDDDDD SSSSSSSSS
 *
 * WYPIN   {#}D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_wypin(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_WYPIN;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Write D to LUT address S[8:0].
 *
 * EEEE 1100001 1LI DDDDDDDDD SSSSSSSSS
 *
 * WRLUT   {#}D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_wrlut(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_WRLUT;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Write byte in D[7:0] to hub address {#}S/PTRx.
 *
 * EEEE 1100010 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRBYTE  {#}D,{#}S/P
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_wrbyte(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_WRBYTE;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Write word in D[15:0] to hub address {#}S/PTRx.
 *
 * EEEE 1100010 1LI DDDDDDDDD SSSSSSSSS
 *
 * WRWORD  {#}D,{#}S/P
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_wrword(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_WRWORD;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Write long in D[31:0] to hub address {#}S/PTRx.
 *
 * EEEE 1100011 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRLONG  {#}D,{#}S/P
 *
 * Prior SETQ/SETQ2 invokes cog/LUT block transfer.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_wrlong(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_WRLONG;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Write long in D[31:0] to hub address PTRA++.
 *
 * EEEE 1100011 0L1 DDDDDDDDD 101100001
 *
 * PUSHA   {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_pusha(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_WRLONG;
    params.IR.op8.imm = true;
    params.IR.op8.src = 0x161;
    return parse_wz_d(params);
}

/**
 * @brief Write long in D[31:0] to hub address PTRB++.
 *
 * EEEE 1100011 0L1 DDDDDDDDD 111100001
 *
 * PUSHB   {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_pushb(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_WRLONG;
    params.IR.op8.imm = true;
    params.IR.op8.src = 0x1e1;
    return parse_wz_d(params);
}

/**
 * @brief Begin new fast hub read via FIFO.
 *
 * EEEE 1100011 1LI DDDDDDDDD SSSSSSSSS
 *
 * RDFAST  {#}D,{#}S
 *
 * D[31] = no wait, D[13:0] = block size in 64-byte units (0 = max), S[19:0] = block start address.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rdfast(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_RDFAST;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Begin new fast hub write via FIFO.
 *
 * EEEE 1100100 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRFAST  {#}D,{#}S
 *
 * D[31] = no wait, D[13:0] = block size in 64-byte units (0 = max), S[19:0] = block start address.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_wrfast(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_WRFAST;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Set next block for when block wraps.
 *
 * EEEE 1100100 1LI DDDDDDDDD SSSSSSSSS
 *
 * FBLOCK  {#}D,{#}S
 *
 * D[13:0] = block size in 64-byte units (0 = max), S[19:0] = block start address.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_fblock(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_FBLOCK;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Issue streamer command immediately, zeroing phase.
 *
 * EEEE 1100101 0LI DDDDDDDDD SSSSSSSSS
 *
 * XINIT   {#}D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_xinit(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_XINIT;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Stop streamer immediately.
 *
 * EEEE 1100101 011 000000000 000000000
 *
 * XSTOP
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_xstop(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_XINIT;
    params.IR.op8.wz = true;
    params.IR.op8.imm = true;
    params.IR.op8.dst = 0x000;
    params.IR.op8.src = 0x000;
    return parse_inst(params);
}

/**
 * @brief Buffer new streamer command to be issued on final NCO rollover of current command, zeroing phase.
 *
 * EEEE 1100101 1LI DDDDDDDDD SSSSSSSSS
 *
 * XZERO   {#}D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_xzero(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_XZERO;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Buffer new streamer command to be issued on final NCO rollover of current command, continuing phase.
 *
 * EEEE 1100110 0LI DDDDDDDDD SSSSSSSSS
 *
 * XCONT   {#}D,{#}S
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_xcont(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_XCONT;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Execute next D[8:0] instructions S times.
 *
 * EEEE 1100110 1LI DDDDDDDDD SSSSSSSSS
 *
 * REP     {#}D,{#}S
 *
 * If S = 0, repeat infinitely.
 * If D[8:0] = 0, nothing repeats.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rep(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_REP;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Start cog selected by D.
 *
 * EEEE 1100111 CLI DDDDDDDDD SSSSSSSSS
 *
 * COGINIT {#}D,{#}S       {WC}
 *
 * S[19:0] sets hub startup address and PTRB of cog.
 * Prior SETQ sets PTRA of cog.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_coginit(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_COGINIT;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Begin CORDIC unsigned multiplication of D * S.
 *
 * EEEE 1101000 0LI DDDDDDDDD SSSSSSSSS
 *
 * QMUL    {#}D,{#}S
 *
 * GETQX/GETQY retrieves lower/upper product.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_qmul(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_QMUL;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Begin CORDIC unsigned division of {SETQ value or 32'b0, D} / S.
 *
 * EEEE 1101000 1LI DDDDDDDDD SSSSSSSSS
 *
 * QDIV    {#}D,{#}S
 *
 * GETQX/GETQY retrieves quotient/remainder.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_qdiv(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_QDIV;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Begin CORDIC unsigned division of {D, SETQ value or 32'b0} / S.
 *
 * EEEE 1101001 0LI DDDDDDDDD SSSSSSSSS
 *
 * QFRAC   {#}D,{#}S
 *
 * GETQX/GETQY retrieves quotient/remainder.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_qfrac(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_QFRAC;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Begin CORDIC square root of {S, D}.
 *
 * EEEE 1101001 1LI DDDDDDDDD SSSSSSSSS
 *
 * QSQRT   {#}D,{#}S
 *
 * GETQX retrieves root.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_qsqrt(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_QSQRT;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Begin CORDIC rotation of point (D, SETQ value or 32'b0) by angle S.
 *
 * EEEE 1101010 0LI DDDDDDDDD SSSSSSSSS
 *
 * QROTATE {#}D,{#}S
 *
 * GETQX/GETQY retrieves X/Y.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_qrotate(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_QROTATE;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Begin CORDIC vectoring of point (D, S).
 *
 * EEEE 1101010 1LI DDDDDDDDD SSSSSSSSS
 *
 * QVECTOR {#}D,{#}S
 *
 * GETQX/GETQY retrieves length/angle.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_qvector(P2Params& params)
{
    params.idx++;
    params.IR.op8.inst = p2_QVECTOR;
    return parse_wz_d_imm_s(params);
}

/**
 * @brief Set hub configuration to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000000000
 *
 * HUBSET  {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_hubset(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_HUBSET;
    return parse_imm_d(params);
}

/**
 * @brief If D is register and no WC, get cog ID (0 to 15) into D.
 *
 * EEEE 1101011 C0L DDDDDDDDD 000000001
 *
 * COGID   {#}D            {WC}
 *
 * If WC, check status of cog D[3:0], C = 1 if on.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_cogid(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_COGID;
    return parse_imm_d_c(params);
}

/**
 * @brief Stop cog D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000000011
 *
 * COGSTOP {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_cogstop(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_COGSTOP;
    return parse_imm_d(params);
}

/**
 * @brief Request a LOCK.
 *
 * EEEE 1101011 C00 DDDDDDDDD 000000100
 *
 * LOCKNEW D               {WC}
 *
 * D will be written with the LOCK number (0 to 15).
 * C = 1 if no LOCK available.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_locknew(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_LOCKNEW;
    return parse_imm_d_c(params);
}

/**
 * @brief Return LOCK D[3:0] for reallocation.
 *
 * EEEE 1101011 00L DDDDDDDDD 000000101
 *
 * LOCKRET {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_lockret(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_LOCKRET;
    return parse_imm_d(params);
}

/**
 * @brief Try to get LOCK D[3:0].
 *
 * EEEE 1101011 C0L DDDDDDDDD 000000110
 *
 * LOCKTRY {#}D            {WC}
 *
 * C = 1 if got LOCK.
 * LOCKREL releases LOCK.
 * LOCK is also released if owner cog stops or restarts.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_locktry(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_LOCKTRY;
    return parse_imm_d_c(params);
}

/**
 * @brief Release LOCK D[3:0].
 *
 * EEEE 1101011 C0L DDDDDDDDD 000000111
 *
 * LOCKREL {#}D            {WC}
 *
 * If D is a register and WC, get current/last cog id of LOCK owner into D and LOCK status into C.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_lockrel(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_LOCKREL;
    return parse_imm_d_c(params);
}

/**
 * @brief Begin CORDIC number-to-logarithm conversion of D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000001110
 *
 * QLOG    {#}D
 *
 * GETQX retrieves log {5'whole_exponent, 27'fractional_exponent}.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_qlog(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_QLOG;
    return parse_imm_d(params);
}

/**
 * @brief Begin CORDIC logarithm-to-number conversion of D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000001111
 *
 * QEXP    {#}D
 *
 * GETQX retrieves number.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_qexp(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_QEXP;
    return parse_imm_d(params);
}

/**
 * @brief Read zero-extended byte from FIFO into D. Used after RDFAST.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000010000
 *
 * RFBYTE  D        {WC/WZ/WCZ}
 *
 * C = MSB of byte.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rfbyte(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_RFBYTE;
    return parse_d_cz(params);
}

/**
 * @brief Read zero-extended word from FIFO into D. Used after RDFAST.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000010001
 *
 * RFWORD  D        {WC/WZ/WCZ}
 *
 * C = MSB of word.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rfword(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_RFWORD;
    return parse_d_cz(params);
}

/**
 * @brief Read long from FIFO into D. Used after RDFAST.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000010010
 *
 * RFLONG  D        {WC/WZ/WCZ}
 *
 * C = MSB of long.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rflong(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_RFLONG;
    return parse_d_cz(params);
}

/**
 * @brief Read zero-extended 1..4-byte value from FIFO into D. Used after RDFAST.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000010011
 *
 * RFVAR   D        {WC/WZ/WCZ}
 *
 * C = 0.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rfvar(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_RFVAR;
    return parse_d_cz(params);
}

/**
 * @brief Read sign-extended 1..4-byte value from FIFO into D. Used after RDFAST.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000010100
 *
 * RFVARS  D        {WC/WZ/WCZ}
 *
 * C = MSB of value.
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rfvars(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_RFVARS;
    return parse_d_cz(params);
}

/**
 * @brief Write byte in D[7:0] into FIFO. Used after WRFAST.
 *
 * EEEE 1101011 00L DDDDDDDDD 000010101
 *
 * WFBYTE  {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_wfbyte(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_WFBYTE;
    return parse_imm_d(params);
}

/**
 * @brief Write word in D[15:0] into FIFO. Used after WRFAST.
 *
 * EEEE 1101011 00L DDDDDDDDD 000010110
 *
 * WFWORD  {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_wfword(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_WFWORD;
    return parse_imm_d(params);
}

/**
 * @brief Write long in D[31:0] into FIFO. Used after WRFAST.
 *
 * EEEE 1101011 00L DDDDDDDDD 000010111
 *
 * WFLONG  {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_wflong(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_WFLONG;
    return parse_imm_d(params);
}

/**
 * @brief Retrieve CORDIC result X into D.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000011000
 *
 * GETQX   D        {WC/WZ/WCZ}
 *
 * Waits, in case result not ready.
 * C = X[31].
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_getqx(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_GETQX;
    return parse_d_cz(params);
}

/**
 * @brief Retrieve CORDIC result Y into D.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000011001
 *
 * GETQY   D        {WC/WZ/WCZ}
 *
 * Waits, in case result no ready.
 * C = Y[31].
 * Z = (result == 0).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_getqy(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_GETQY;
    return parse_d_cz(params);
}

/**
 * @brief Get CT into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 000011010
 *
 * GETCT   D
 *
 * CT is the free-running 32-bit system counter that increments on every clock.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_getct(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_GETCT;
    return parse_d(params);
}

/**
 * @brief Get RND into D/C/Z.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000011011
 *
 * GETRND  D        {WC/WZ/WCZ}
 *
 * RND is the PRNG that updates on every clock.
 * D = RND[31:0], C = RND[31], Z = RND[30], unique per cog.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_getrnd(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_GETRND;
    return parse_d_cz(params);
}

/**
 * @brief Get RND into C/Z.
 *
 * EEEE 1101011 CZ1 000000000 000011011
 *
 * GETRND            WC/WZ/WCZ
 *
 * C = RND[31], Z = RND[30], unique per cog.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_getrnd_cz(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_GETRND;
    return parse_cz(params);
}

/**
 * @brief DAC3 = D[31:24], DAC2 = D[23:16], DAC1 = D[15:8], DAC0 = D[7:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000011100
 *
 * SETDACS {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setdacs(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SETDACS;
    return parse_imm_d(params);
}

/**
 * @brief Set streamer NCO frequency to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000011101
 *
 * SETXFRQ {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setxfrq(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SETXFRQ;
    return parse_imm_d(params);
}

/**
 * @brief Get the streamer's Goertzel X accumulator into D and the Y accumulator into the next instruction's S, clear accumulators.
 *
 * EEEE 1101011 000 DDDDDDDDD 000011110
 *
 * GETXACC D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_getxacc(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_GETXACC;
    return parse_d(params);
}

/**
 * @brief Wait 2 + D clocks if no WC/WZ/WCZ.
 *
 * EEEE 1101011 00L DDDDDDDDD 000011111
 *
 * WAITX   {#}D     {WC/WZ/WCZ}
 *
 * If WC/WZ/WCZ, wait 2 + (D & RND) clocks.
 * C/Z = 0.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_waitx(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_WAITX;
    return parse_imm_d_cz(params);
}

/**
 * @brief Set SE1 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100000
 *
 * SETSE1  {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setse1(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SETSE1;
    return parse_imm_d(params);
}

/**
 * @brief Set SE2 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100001
 *
 * SETSE2  {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setse2(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SETSE2;
    return parse_imm_d(params);
}

/**
 * @brief Set SE3 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100010
 *
 * SETSE3  {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setse3(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SETSE3;
    return parse_imm_d(params);
}

/**
 * @brief Set SE4 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100011
 *
 * SETSE4  {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setse4(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SETSE4;
    return parse_imm_d(params);
}

/**
 * @brief Get INT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000000 000100100
 *
 * POLLINT          {WC/WZ/WCZ}
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_pollint(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_POLLINT;
    return parse_cz(params);
}

/**
 * @brief Get CT1 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000001 000100100
 *
 * POLLCT1          {WC/WZ/WCZ}
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_pollct1(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_POLLCT1;
    return parse_cz(params);
}

/**
 * @brief Get CT2 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000010 000100100
 *
 * POLLCT2          {WC/WZ/WCZ}
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_pollct2(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_POLLCT2;
    return parse_cz(params);
}

/**
 * @brief Get CT3 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000011 000100100
 *
 * POLLCT3          {WC/WZ/WCZ}
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_pollct3(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_POLLCT3;
    return parse_cz(params);
}

/**
 * @brief Get SE1 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000100 000100100
 *
 * POLLSE1          {WC/WZ/WCZ}
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_pollse1(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_POLLSE1;
    return parse_cz(params);
}

/**
 * @brief Get SE2 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000101 000100100
 *
 * POLLSE2          {WC/WZ/WCZ}
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_pollse2(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_POLLSE2;
    return parse_cz(params);
}

/**
 * @brief Get SE3 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000110 000100100
 *
 * POLLSE3          {WC/WZ/WCZ}
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_pollse3(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_POLLSE3;
    return parse_cz(params);
}

/**
 * @brief Get SE4 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000111 000100100
 *
 * POLLSE4          {WC/WZ/WCZ}
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_pollse4(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_POLLSE4;
    return parse_cz(params);
}

/**
 * @brief Get PAT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001000 000100100
 *
 * POLLPAT          {WC/WZ/WCZ}
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_pollpat(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_POLLPAT;
    return parse_cz(params);
}

/**
 * @brief Get FBW event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001001 000100100
 *
 * POLLFBW          {WC/WZ/WCZ}
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_pollfbw(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_POLLFBW;
    return parse_cz(params);
}

/**
 * @brief Get XMT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001010 000100100
 *
 * POLLXMT          {WC/WZ/WCZ}
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_pollxmt(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_POLLXMT;
    return parse_cz(params);
}

/**
 * @brief Get XFI event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001011 000100100
 *
 * POLLXFI          {WC/WZ/WCZ}
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_pollxfi(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_POLLXFI;
    return parse_cz(params);
}

/**
 * @brief Get XRO event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001100 000100100
 *
 * POLLXRO          {WC/WZ/WCZ}
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_pollxro(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_POLLXRO;
    return parse_cz(params);
}

/**
 * @brief Get XRL event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001101 000100100
 *
 * POLLXRL          {WC/WZ/WCZ}
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_pollxrl(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_POLLXRL;
    return parse_cz(params);
}

/**
 * @brief Get ATN event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001110 000100100
 *
 * POLLATN          {WC/WZ/WCZ}
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_pollatn(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_POLLATN;
    return parse_cz(params);
}

/**
 * @brief Get QMT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001111 000100100
 *
 * POLLQMT          {WC/WZ/WCZ}
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_pollqmt(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_POLLQMT;
    return parse_cz(params);
}

/**
 * @brief Wait for INT event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000010000 000100100
 *
 * WAITINT          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_waitint(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_WAITINT;
    return parse_cz(params);
}

/**
 * @brief Wait for CT1 event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000010001 000100100
 *
 * WAITCT1          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_waitct1(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_WAITCT1;
    return parse_cz(params);
}

/**
 * @brief Wait for CT2 event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000010010 000100100
 *
 * WAITCT2          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_waitct2(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_WAITCT2;
    return parse_cz(params);
}

/**
 * @brief Wait for CT3 event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000010011 000100100
 *
 * WAITCT3          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_waitct3(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_WAITCT3;
    return parse_cz(params);
}

/**
 * @brief Wait for SE1 event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000010100 000100100
 *
 * WAITSE1          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_waitse1(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_WAITSE1;
    return parse_cz(params);
}

/**
 * @brief Wait for SE2 event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000010101 000100100
 *
 * WAITSE2          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_waitse2(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_WAITSE2;
    return parse_cz(params);
}

/**
 * @brief Wait for SE3 event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000010110 000100100
 *
 * WAITSE3          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_waitse3(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_WAITSE3;
    return parse_cz(params);
}

/**
 * @brief Wait for SE4 event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000010111 000100100
 *
 * WAITSE4          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_waitse4(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_WAITSE4;
    return parse_cz(params);
}

/**
 * @brief Wait for PAT event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000011000 000100100
 *
 * WAITPAT          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_waitpat(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_WAITPAT;
    return parse_cz(params);
}

/**
 * @brief Wait for FBW event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000011001 000100100
 *
 * WAITFBW          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_waitfbw(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_WAITFBW;
    return parse_cz(params);
}

/**
 * @brief Wait for XMT event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000011010 000100100
 *
 * WAITXMT          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_waitxmt(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_WAITXMT;
    return parse_cz(params);
}

/**
 * @brief Wait for XFI event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000011011 000100100
 *
 * WAITXFI          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_waitxfi(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_WAITXFI;
    return parse_cz(params);
}

/**
 * @brief Wait for XRO event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000011100 000100100
 *
 * WAITXRO          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_waitxro(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_WAITXRO;
    return parse_cz(params);
}

/**
 * @brief Wait for XRL event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000011101 000100100
 *
 * WAITXRL          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_waitxrl(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_WAITXRL;
    return parse_cz(params);
}

/**
 * @brief Wait for ATN event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000011110 000100100
 *
 * WAITATN          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_waitatn(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_WAITATN;
    return parse_cz(params);
}

/**
 * @brief Allow interrupts (default).
 *
 * EEEE 1101011 000 000100000 000100100
 *
 * ALLOWI
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_allowi(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_ALLOWI;
    return parse_inst(params);
}

/**
 * @brief Stall Interrupts.
 *
 * EEEE 1101011 000 000100001 000100100
 *
 * STALLI
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_stalli(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_STALLI;
    return parse_inst(params);
}

/**
 * @brief Trigger INT1, regardless of STALLI mode.
 *
 * EEEE 1101011 000 000100010 000100100
 *
 * TRGINT1
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_trgint1(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_TRGINT1;
    return parse_inst(params);
}

/**
 * @brief Trigger INT2, regardless of STALLI mode.
 *
 * EEEE 1101011 000 000100011 000100100
 *
 * TRGINT2
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_trgint2(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_TRGINT2;
    return parse_inst(params);
}

/**
 * @brief Trigger INT3, regardless of STALLI mode.
 *
 * EEEE 1101011 000 000100100 000100100
 *
 * TRGINT3
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_trgint3(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_TRGINT3;
    return parse_inst(params);
}

/**
 * @brief Cancel INT1.
 *
 * EEEE 1101011 000 000100101 000100100
 *
 * NIXINT1
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_nixint1(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_NIXINT1;
    return parse_inst(params);
}

/**
 * @brief Cancel INT2.
 *
 * EEEE 1101011 000 000100110 000100100
 *
 * NIXINT2
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_nixint2(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_NIXINT2;
    return parse_inst(params);
}

/**
 * @brief Cancel INT3.
 *
 * EEEE 1101011 000 000100111 000100100
 *
 * NIXINT3
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_nixint3(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_X24;
    params.IR.op.dst = p2_OPX24_NIXINT3;
    return parse_inst(params);
}

/**
 * @brief Set INT1 source to D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100101
 *
 * SETINT1 {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setint1(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SETINT1;
    return parse_imm_d(params);
}

/**
 * @brief Set INT2 source to D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100110
 *
 * SETINT2 {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setint2(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SETINT2;
    return parse_imm_d(params);
}

/**
 * @brief Set INT3 source to D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100111
 *
 * SETINT3 {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setint3(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SETINT3;
    return parse_imm_d(params);
}

/**
 * @brief Set Q to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000101000
 *
 * SETQ    {#}D
 *
 * Use before RDLONG/WRLONG/WMLONG to set block transfer.
 * Also used before MUXQ/COGINIT/QDIV/QFRAC/QROTATE/WAITxxx.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setq(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SETQ;
    return parse_imm_d(params);
}

/**
 * @brief Set Q to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000101001
 *
 * SETQ2   {#}D
 *
 * Use before RDLONG/WRLONG/WMLONG to set LUT block transfer.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setq2(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SETQ2;
    return parse_imm_d(params);
}

/**
 * @brief Push D onto stack.
 *
 * EEEE 1101011 00L DDDDDDDDD 000101010
 *
 * PUSH    {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_push(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_PUSH;
    return parse_imm_d(params);
}

/**
 * @brief Pop stack (K).
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101011
 *
 * POP     D        {WC/WZ/WCZ}
 *
 * C = K[31], Z = K[30], D = K.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_pop(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_POP;
    return parse_d_cz(params);
}

/**
 * @brief Jump to D.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101100
 *
 * JMP     D        {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jmp(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_JMP;
    return parse_d_cz(params);
}

/**
 * @brief Call to D by pushing {C, Z, 10'b0, PC[19:0]} onto stack.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101101
 *
 * CALL    D        {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_call(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_CALL_RET;
    return parse_d_cz(params);
}

/**
 * @brief Return by popping stack (K).
 *
 * EEEE 1101011 CZ1 000000000 000101101
 *
 * RET              {WC/WZ/WCZ}
 *
 * C = K[31], Z = K[30], PC = K[19:0].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_ret(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.imm = true;
    params.IR.op.dst = 0x000;
    params.IR.op.src = p2_OPSRC_CALL_RET;
    return parse_cz(params);
}

/**
 * @brief Call to D by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRA++.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101110
 *
 * CALLA   D        {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_calla(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_CALLA_RETA;
    return parse_d_cz(params);
}

/**
 * @brief Return by reading hub long (L) at --PTRA.
 *
 * EEEE 1101011 CZ1 000000000 000101110
 *
 * RETA             {WC/WZ/WCZ}
 *
 * C = L[31], Z = L[30], PC = L[19:0].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_reta(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.imm = true;
    params.IR.op.dst = 0x000;
    params.IR.op.src = p2_OPSRC_CALLA_RETA;
    return parse_cz(params);
}

/**
 * @brief Call to D by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRB++.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101111
 *
 * CALLB   D        {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_callb(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_CALLB_RETB;
    return parse_d_cz(params);
}

/**
 * @brief Return by reading hub long (L) at --PTRB.
 *
 * EEEE 1101011 CZ1 000000000 000101111
 *
 * RETB             {WC/WZ/WCZ}
 *
 * C = L[31], Z = L[30], PC = L[19:0].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_retb(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.imm = true;
    params.IR.op.dst = 0x000;
    params.IR.op.src = p2_OPSRC_CALLB_RETB;
    return parse_cz(params);
}

/**
 * @brief Jump ahead/back by D instructions.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110000
 *
 * JMPREL  {#}D
 *
 * For cogex, PC += D[19:0].
 * For hubex, PC += D[17:0] << 2.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jmprel(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_JMPREL;
    return parse_imm_d(params);
}

/**
 * @brief Skip instructions per D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110001
 *
 * SKIP    {#}D
 *
 * Subsequent instructions 0.
 * 31 get cancelled for each '1' bit in D[0].
 * D[31].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_skip(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SKIP;
    return parse_imm_d(params);
}

/**
 * @brief Skip cog/LUT instructions fast per D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110010
 *
 * SKIPF   {#}D
 *
 * Like SKIP, but instead of cancelling instructions, the PC leaps over them.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_skipf(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SKIPF;
    return parse_imm_d(params);
}

/**
 * @brief Jump to D[9:0] in cog/LUT and set SKIPF pattern to D[31:10].
 *
 * EEEE 1101011 00L DDDDDDDDD 000110011
 *
 * EXECF   {#}D
 *
 * PC = {10'b0, D[9:0]}.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_execf(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_EXECF;
    return parse_imm_d(params);
}

/**
 * @brief Get current FIFO hub pointer into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 000110100
 *
 * GETPTR  D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_getptr(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_GETPTR;
    return parse_d(params);
}

/**
 * @brief Get breakpoint status into D according to WC/WZ/WCZ.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000110101
 *
 * GETBRK  D          WC/WZ/WCZ
 *
 * C = 0.
 * Z = 0.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_getbrk(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.imm = false;
    params.IR.op.src = p2_OPSRC_COGBRK;
    return parse_d_cz(params);
}

/**
 * @brief If in debug ISR, trigger asynchronous breakpoint in cog D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000110101
 *
 * COGBRK  {#}D
 *
 * Cog D[3:0] must have asynchronous breakpoint enabled.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_cogbrk(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_COGBRK;
    return parse_imm_d(params);
}

/**
 * @brief If in debug ISR, set next break condition to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110110
 *
 * BRK     {#}D
 *
 * Else, trigger break if enabled, conditionally write break code to D[7:0].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_brk(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_BRK;
    return parse_imm_d(params);
}

/**
 * @brief If D[0] = 1 then enable LUT sharing, where LUT writes within the adjacent odd/even companion cog are copied to this LUT.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110111
 *
 * SETLUTS {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setluts(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SETLUTS;
    return parse_imm_d(params);
}

/**
 * @brief Set the colorspace converter "CY" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111000
 *
 * SETCY   {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setcy(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SETCY;
    return parse_imm_d(params);
}

/**
 * @brief Set the colorspace converter "CI" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111001
 *
 * SETCI   {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setci(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SETCI;
    return parse_imm_d(params);
}

/**
 * @brief Set the colorspace converter "CQ" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111010
 *
 * SETCQ   {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setcq(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SETCQ;
    return parse_imm_d(params);
}

/**
 * @brief Set the colorspace converter "CFRQ" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111011
 *
 * SETCFRQ {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setcfrq(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SETCFRQ;
    return parse_imm_d(params);
}

/**
 * @brief Set the colorspace converter "CMOD" parameter to D[6:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111100
 *
 * SETCMOD {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setcmod(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SETCMOD;
    return parse_imm_d(params);
}

/**
 * @brief Set BLNPIX/MIXPIX blend factor to D[7:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111101
 *
 * SETPIV  {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setpiv(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SETPIV;
    return parse_imm_d(params);
}

/**
 * @brief Set MIXPIX mode to D[5:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111110
 *
 * SETPIX  {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setpix(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SETPIX;
    return parse_imm_d(params);
}

/**
 * @brief Strobe "attention" of all cogs whose corresponging bits are high in D[15:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111111
 *
 * COGATN  {#}D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_cogatn(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_COGATN;
    return parse_imm_d(params);
}

/**
 * @brief Test  IN bit of pin D[5:0], write to C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000000
 *
 * TESTP   {#}D           WC/WZ
 *
 * C/Z =          IN[D[5:0]].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_testp_w(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_TESTP_W_DIRL;
    return parse_imm_d_cz(params);
}

/**
 * @brief Test !IN bit of pin D[5:0], write to C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000001
 *
 * TESTPN  {#}D           WC/WZ
 *
 * C/Z =         !IN[D[5:0]].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_testpn_w(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_TESTPN_W;
    return parse_imm_d_cz(params);
}

/**
 * @brief Test  IN bit of pin D[5:0], AND into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000010
 *
 * TESTP   {#}D       ANDC/ANDZ
 *
 * C/Z = C/Z AND  IN[D[5:0]].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_testp_and(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_TESTP_AND;
    return parse_imm_d_cz(params);
}

/**
 * @brief Test !IN bit of pin D[5:0], AND into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000011
 *
 * TESTPN  {#}D       ANDC/ANDZ
 *
 * C/Z = C/Z AND !IN[D[5:0]].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_testpn_and(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_TESTPN_AND;
    return parse_imm_d_cz(params);
}

/**
 * @brief Test  IN bit of pin D[5:0], OR  into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000100
 *
 * TESTP   {#}D         ORC/ORZ
 *
 * C/Z = C/Z OR   IN[D[5:0]].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_testp_or(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_TESTP_OR;
    return parse_imm_d_cz(params);
}

/**
 * @brief Test !IN bit of pin D[5:0], OR  into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000101
 *
 * TESTPN  {#}D         ORC/ORZ
 *
 * C/Z = C/Z OR  !IN[D[5:0]].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_testpn_or(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_TESTPN_OR;
    return parse_imm_d_cz(params);
}

/**
 * @brief Test  IN bit of pin D[5:0], XOR into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000110
 *
 * TESTP   {#}D       XORC/XORZ
 *
 * C/Z = C/Z XOR  IN[D[5:0]].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_testp_xor(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_TESTP_XOR;
    return parse_imm_d_cz(params);
}

/**
 * @brief Test !IN bit of pin D[5:0], XOR into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000111
 *
 * TESTPN  {#}D       XORC/XORZ
 *
 * C/Z = C/Z XOR !IN[D[5:0]].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_testpn_xor(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_TESTPN_XOR;
    return parse_imm_d_cz(params);
}

/**
 * @brief DIR bit of pin D[5:0] = 0.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000000
 *
 * DIRL    {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_dirl(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_DIRL;
    return parse_imm_d_cz(params);
}

/**
 * @brief DIR bit of pin D[5:0] = 1.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000001
 *
 * DIRH    {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_dirh(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_DIRH;
    return parse_imm_d_cz(params);
}

/**
 * @brief DIR bit of pin D[5:0] = C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000010
 *
 * DIRC    {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_dirc(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_DIRC;
    return parse_imm_d_cz(params);
}

/**
 * @brief DIR bit of pin D[5:0] = !C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000011
 *
 * DIRNC   {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_dirnc(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_DIRNC;
    return parse_imm_d_cz(params);
}

/**
 * @brief DIR bit of pin D[5:0] = Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000100
 *
 * DIRZ    {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_dirz(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_DIRZ;
    return parse_imm_d_cz(params);
}

/**
 * @brief DIR bit of pin D[5:0] = !Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000101
 *
 * DIRNZ   {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_dirnz(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_DIRNZ;
    return parse_imm_d_cz(params);
}

/**
 * @brief DIR bit of pin D[5:0] = RND.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000110
 *
 * DIRRND  {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_dirrnd(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_DIRRND;
    return parse_imm_d_cz(params);
}

/**
 * @brief DIR bit of pin D[5:0] = !bit.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000111
 *
 * DIRNOT  {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_dirnot(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_DIRNOT;
    return parse_imm_d_cz(params);
}

/**
 * @brief OUT bit of pin D[5:0] = 0.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001000
 *
 * OUTL    {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_outl(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_OUTL;
    return parse_imm_d_cz(params);
}

/**
 * @brief OUT bit of pin D[5:0] = 1.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001001
 *
 * OUTH    {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_outh(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_OUTH;
    return parse_imm_d_cz(params);
}

/**
 * @brief OUT bit of pin D[5:0] = C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001010
 *
 * OUTC    {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_outc(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_OUTC;
    return parse_imm_d_cz(params);
}

/**
 * @brief OUT bit of pin D[5:0] = !C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001011
 *
 * OUTNC   {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_outnc(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_OUTNC;
    return parse_imm_d_cz(params);
}

/**
 * @brief OUT bit of pin D[5:0] = Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001100
 *
 * OUTZ    {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_outz(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_OUTZ;
    return parse_imm_d_cz(params);
}

/**
 * @brief OUT bit of pin D[5:0] = !Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001101
 *
 * OUTNZ   {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_outnz(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_OUTNZ;
    return parse_imm_d_cz(params);
}

/**
 * @brief OUT bit of pin D[5:0] = RND.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001110
 *
 * OUTRND  {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_outrnd(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_OUTRND;
    return parse_imm_d_cz(params);
}

/**
 * @brief OUT bit of pin D[5:0] = !bit.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001111
 *
 * OUTNOT  {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_outnot(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_OUTNOT;
    return parse_imm_d_cz(params);
}

/**
 * @brief OUT bit of pin D[5:0] = 0.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001010000
 *
 * FLTL    {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_fltl(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_FLTL;
    return parse_imm_d_cz(params);
}

/**
 * @brief OUT bit of pin D[5:0] = 1.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001010001
 *
 * FLTH    {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_flth(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_FLTH;
    return parse_imm_d_cz(params);
}

/**
 * @brief OUT bit of pin D[5:0] = C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001010010
 *
 * FLTC    {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_fltc(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_FLTC;
    return parse_imm_d_cz(params);
}

/**
 * @brief OUT bit of pin D[5:0] = !C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001010011
 *
 * FLTNC   {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_fltnc(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_FLTNC;
    return parse_imm_d_cz(params);
}

/**
 * @brief OUT bit of pin D[5:0] = Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001010100
 *
 * FLTZ    {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_fltz(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_FLTZ;
    return parse_imm_d_cz(params);
}

/**
 * @brief OUT bit of pin D[5:0] = !Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001010101
 *
 * FLTNZ   {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_fltnz(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_FLTNZ;
    return parse_imm_d_cz(params);
}

/**
 * @brief OUT bit of pin D[5:0] = RND.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001010110
 *
 * FLTRND  {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_fltrnd(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_FLTRND;
    return parse_imm_d_cz(params);
}

/**
 * @brief OUT bit of pin D[5:0] = !bit.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001010111
 *
 * FLTNOT  {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_fltnot(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_FLTNOT;
    return parse_imm_d_cz(params);
}

/**
 * @brief OUT bit of pin D[5:0] = 0.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001011000
 *
 * DRVL    {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_drvl(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_DRVL;
    return parse_imm_d_cz(params);
}

/**
 * @brief OUT bit of pin D[5:0] = 1.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001011001
 *
 * DRVH    {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_drvh(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_DRVH;
    return parse_imm_d_cz(params);
}

/**
 * @brief OUT bit of pin D[5:0] = C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001011010
 *
 * DRVC    {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_drvc(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_DRVC;
    return parse_imm_d_cz(params);
}

/**
 * @brief OUT bit of pin D[5:0] = !C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001011011
 *
 * DRVNC   {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_drvnc(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_DRVNC;
    return parse_imm_d_cz(params);
}

/**
 * @brief OUT bit of pin D[5:0] = Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001011100
 *
 * DRVZ    {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_drvz(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_DRVZ;
    return parse_imm_d_cz(params);
}

/**
 * @brief OUT bit of pin D[5:0] = !Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001011101
 *
 * DRVNZ   {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_drvnz(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_DRVNZ;
    return parse_imm_d_cz(params);
}

/**
 * @brief OUT bit of pin D[5:0] = RND.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001011110
 *
 * DRVRND  {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_drvrnd(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_DRVRND;
    return parse_imm_d_cz(params);
}

/**
 * @brief OUT bit of pin D[5:0] = !bit.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001011111
 *
 * DRVNOT  {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_drvnot(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_DRVNOT;
    return parse_imm_d_cz(params);
}

/**
 * @brief Split every 4th bit of S into bytes of D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100000
 *
 * SPLITB  D
 *
 * D = {S[31], S[27], S[23], S[19], ... S[12], S[8], S[4], S[0]}.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_splitb(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SPLITB;
    return parse_d(params);
}

/**
 * @brief Merge bits of bytes in S into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100001
 *
 * MERGEB  D
 *
 * D = {S[31], S[23], S[15], S[7], ... S[24], S[16], S[8], S[0]}.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_mergeb(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_MERGEB;
    return parse_d(params);
}

/**
 * @brief Split bits of S into words of D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100010
 *
 * SPLITW  D
 *
 * D = {S[31], S[29], S[27], S[25], ... S[6], S[4], S[2], S[0]}.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_splitw(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SPLITW;
    return parse_d(params);
}

/**
 * @brief Merge bits of words in S into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100011
 *
 * MERGEW  D
 *
 * D = {S[31], S[15], S[30], S[14], ... S[17], S[1], S[16], S[0]}.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_mergew(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_MERGEW;
    return parse_d(params);
}

/**
 * @brief Relocate and periodically invert bits from S into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100100
 *
 * SEUSSF  D
 *
 * Returns to original value on 32nd iteration.
 * Forward pattern.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_seussf(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SEUSSF;
    return parse_d(params);
}

/**
 * @brief Relocate and periodically invert bits from S into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100101
 *
 * SEUSSR  D
 *
 * Returns to original value on 32nd iteration.
 * Reverse pattern.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_seussr(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SEUSSR;
    return parse_d(params);
}

/**
 * @brief Squeeze 8:8:8 RGB value in S[31:8] into 5:6:5 value in D[15:0].
 *
 * EEEE 1101011 000 DDDDDDDDD 001100110
 *
 * RGBSQZ  D
 *
 * D = {15'b0, S[31:27], S[23:18], S[15:11]}.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rgbsqz(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_RGBSQZ;
    return parse_d(params);
}

/**
 * @brief Expand 5:6:5 RGB value in S[15:0] into 8:8:8 value in D[31:8].
 *
 * EEEE 1101011 000 DDDDDDDDD 001100111
 *
 * RGBEXP  D
 *
 * D = {S[15:11,15:13], S[10:5,10:9], S[4:0,4:2], 8'b0}.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rgbexp(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_RGBEXP;
    return parse_d(params);
}

/**
 * @brief Iterate D with xoroshiro32+ PRNG algorithm and put PRNG result into next instruction's S.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101000
 *
 * XORO32  D
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_xoro32(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_XORO32;
    return parse_d(params);
}

/**
 * @brief Reverse D bits.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101001
 *
 * REV     D
 *
 * D = D[0:31].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rev(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_REV;
    return parse_d(params);
}

/**
 * @brief Rotate C,Z right through D.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 001101010
 *
 * RCZR    D        {WC/WZ/WCZ}
 *
 * D = {C, Z, D[31:2]}.
 * C = D[1],  Z = D[0].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rczr(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_RCZR;
    return parse_d_cz(params);
}

/**
 * @brief Rotate C,Z left through D.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 001101011
 *
 * RCZL    D        {WC/WZ/WCZ}
 *
 * D = {D[29:0], C, Z}.
 * C = D[31], Z = D[30].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_rczl(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_RCZL;
    return parse_d_cz(params);
}

/**
 * @brief Write 0 or 1 to D, according to  C.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101100
 *
 * WRC     D
 *
 * D = {31'b0,  C).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_wrc(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_WRC;
    return parse_d(params);
}

/**
 * @brief Write 0 or 1 to D, according to !C.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101101
 *
 * WRNC    D
 *
 * D = {31'b0, !C).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_wrnc(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_WRNC;
    return parse_d(params);
}

/**
 * @brief Write 0 or 1 to D, according to  Z.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101110
 *
 * WRZ     D
 *
 * D = {31'b0,  Z).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_wrz(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_WRZ;
    return parse_d(params);
}

/**
 * @brief Write 0 or 1 to D, according to !Z.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101111
 *
 * WRNZ    D
 *
 * D = {31'b0, !Z).
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_wrnz(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_WRNZ_MODCZ;
    return parse_d(params);
}

/**
 * @brief Modify C and Z according to cccc and zzzz.
 *
 * EEEE 1101011 CZ1 0cccczzzz 001101111
 *
 * MODCZ   c,z      {WC/WZ/WCZ}
 *
 * C = cccc[{C,Z}], Z = zzzz[{C,Z}].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_modcz(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_WRNZ_MODCZ;
    return parse_cccc_zzzz_cz(params);
}

/**
 * @brief Set scope mode.
 * SETSCP points the scope mux to a set of four pins starting
 * at (D[5:0] AND $3C), with D[6]=1 to enable scope operation.
 *
 * EEEE 1101011 00L DDDDDDDDD 001110000
 *
 * SETSCP  {#}D
 *
 * Pins D[5:2], enable D[6].
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_setscp(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_SETSCP;
    return parse_imm_d(params);
}

/**
 * @brief Get scope values.
 *
 * EEEE 1101011 000 DDDDDDDDD 001110001
 *
 * Any time GETSCP is executed, the lower bytes of those four pins' RDPIN values are returned in D.
 * This feature will mainly be useful on the next silicon, as the FPGAs don't have ADC-capable pins.
 *
 * GETSCP  D
 *
 * C = cccc[{C,Z}], Z = zzzz[{C,Z}].
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_getscp(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_GETSCP;
    return parse_d(params);
}

/**
 * @brief Jump to A.
 *
 * EEEE 1101100 RAA AAAAAAAAA AAAAAAAAA
 *
 * JMP     #A
 *
 * If R = 1, PC += A, else PC = A.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_jmp_abs(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_JMP;
    return parse_pc_abs(params);
}

/**
 * @brief Call to A by pushing {C, Z, 10'b0, PC[19:0]} onto stack.
 *
 * EEEE 1101101 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALL    #A
 *
 * If R = 1, PC += A, else PC = A.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_call_abs(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_CALL_RET;
    return parse_pc_abs(params);
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRA++.
 *
 * EEEE 1101110 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLA   #A
 *
 * If R = 1, PC += A, else PC = A.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_calla_abs(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_CALLA_RETA;
    return parse_pc_abs(params);
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRB++.
 *
 * EEEE 1101111 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLB   #A
 *
 * If R = 1, PC += A, else PC = A.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_callb_abs(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_OPSRC;
    params.IR.op.src = p2_OPSRC_CALLB_RETB;
    return parse_pc_abs(params);
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 1110000 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PA,#A
 *
 * If R = 1, PC += A, else PC = A.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_calld_pa_abs(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_CALLD_PA_ABS;
    return parse_pc_abs(params);
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 1110001 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PB,#A
 *
 * If R = 1, PC += A, else PC = A.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_calld_pb_abs(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_CALLD_PB_ABS;
    return parse_pc_abs(params);
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PTRA (per W).
 *
 * EEEE 1110010 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PTRA,#A
 *
 * If R = 1, PC += A, else PC = A.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_calld_ptra_abs(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_CALLD_PTRA_ABS;
    return parse_pc_abs(params);
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PTRB (per W).
 *
 * EEEE 1110011 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PTRB,#A
 *
 * If R = 1, PC += A, else PC = A.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_calld_ptrb_abs(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_CALLD_PTRB_ABS;
    return parse_pc_abs(params);
}

/**
 * @brief Get {12'b0, address[19:0]} into PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 11101WW RAA AAAAAAAAA AAAAAAAAA
 *
 * LOC     PA/PB/PTRA/PTRB,#A
 *
 * If R = 1, address = PC + A, else address = A.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_loc(P2Params& params)
{
    params.idx++;
    p2_token_e tok = params.tokens.value(params.idx);
    bool success = false;
    switch (tok) {
    case t_PA:
        success = asm_loc_pa(params);
        break;
    case t_PB:
        success = asm_loc_pb(params);
        break;
    case t_PTRA:
        success = asm_loc_ptra(params);
        break;
    case t_PTRB:
        success = asm_loc_ptrb(params);
        break;
    default:
        params.error = tr("Invalid pointer type '%1' (expected one of PA,PB,PTRA,PTRB).")
                       .arg(params.words.value(params.idx));
    }
    return success;
}

/**
 * @brief Get {12'b0, address[19:0]} into PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 11101WW RAA AAAAAAAAA AAAAAAAAA
 *
 * LOC     PA/PB/PTRA/PTRB,#A
 *
 * If R = 1, address = PC + A, else address = A.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_loc_pa(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_LOC_PA;
    return parse_pc_abs(params);
}

/**
 * @brief Get {12'b0, address[19:0]} into PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 11101WW RAA AAAAAAAAA AAAAAAAAA
 *
 * LOC     PA/PB/PTRA/PTRB,#A
 *
 * If R = 1, address = PC + A, else address = A.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_loc_pb(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_LOC_PB;
    return parse_pc_abs(params);
}

/**
 * @brief Get {12'b0, address[19:0]} into PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 11101WW RAA AAAAAAAAA AAAAAAAAA
 *
 * LOC     PA/PB/PTRA/PTRB,#A
 *
 * If R = 1, address = PC + A, else address = A.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_loc_ptra(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_LOC_PTRA;
    return parse_pc_abs(params);
}

/**
 * @brief Get {12'b0, address[19:0]} into PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 11101WW RAA AAAAAAAAA AAAAAAAAA
 *
 * LOC     PA/PB/PTRA/PTRB,#A
 *
 * If R = 1, address = PC + A, else address = A.
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_loc_ptrb(P2Params& params)
{
    params.idx++;
    params.IR.op.inst = p2_LOC_PTRB;
    return parse_pc_abs(params);
}

/**
 * @brief Queue #N[31:9] to be used as upper 23 bits for next #S occurrence, so that the next 9-bit #S will be augmented to 32 bits.
 *
 * EEEE 11110NN NNN NNNNNNNNN NNNNNNNNN
 *
 * AUGS    #N
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_augs(P2Params& params)
{
    static const QVector<p2_inst_e> augs = QVector<p2_inst_e>()
                                           << p2_AUGS_00
                                           << p2_AUGS_01
                                           << p2_AUGS_10
                                           << p2_AUGS_11;
    params.idx++;
    params.IR.op.inst = p2_AUGS_00;
    return parse_imm23(params, augs);
}

/**
 * @brief Queue #N[31:9] to be used as upper 23 bits for next #D occurrence, so that the next 9-bit #D will be augmented to 32 bits.
 *
 * EEEE 11111NN NNN NNNNNNNNN NNNNNNNNN
 *
 * AUGD    #N
 *
 *
 *
 * @param params reference to the assembler parameters
 */
bool P2Asm::asm_augd(P2Params& params)
{
    static const QVector<p2_inst_e> augd = QVector<p2_inst_e>()
                                           << p2_AUGD_00
                                           << p2_AUGD_01
                                           << p2_AUGD_10
                                           << p2_AUGD_11;
    params.idx++;
    params.IR.op.inst = p2_AUGD_00;
    return parse_imm23(params, augd);
}
