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

static const QString stop_expr = QStringLiteral("*/\\+-&|^<=>()\"'");
static const QString bin_digits = QStringLiteral("01");
static const QString oct_digits = QStringLiteral("01234567");
static const QString dec_digits = QStringLiteral("0123456789");
static const QString hex_digits = QStringLiteral("0123456789ABCDEF");

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

p2_cond_e P2Asm::conditional(P2Params& p, p2_token_e cond)
{
    if (Token.type(cond) != tt_conditional)
        return cc_always;
    p2_cond_e result = Token.conditional(cond, cc_always);
    p.idx += 1;
    return result;
}

p2_cond_e P2Asm::parse_modcz(P2Params& p, p2_token_e cond)
{
    p2_cond_e result = Token.modcz_param(cond, cc_clr);
    p.idx += 1;
    return result;
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
bool P2Asm::split_and_tokenize(P2Params& p, const QString& line)
{
    QStringList words;
    QString word;
    QChar instring = QChar::Null;
    bool escaped = false;
    bool comment = false;

    p.words.clear();
    foreach(QChar ch, line) {

        // previous character was an escape (\)
        if (escaped) {
            escaped = false;
            word += ch;
            continue;
        }

        // inside string?
        if (instring != QChar::Null) {
            if (ch == instring) {
                // end of string
                instring = QChar::Null;
            }
            word += ch;
            continue;
        }

        // white space?
        if (ch.isSpace()) {
            if (word.isEmpty())
                continue;
            // non empty white space separated word
            words += word;
            word.clear();
            continue;
        }

        // start of comment until end of line
        if (ch == QChar('\'')) {
            comment = true;
            break;
        }

        // a comma?
        if (ch == QChar(',')) {
            if (!word.isEmpty())
                words += word;
            // make comma (,) a separate token
            word = ch;
            words += word;
            word.clear();
            continue;
        }

        // start of a string?
        if (ch == QChar('"')) {
            word += ch;
            instring = ch;
            continue;
        }

        // append to word
        word += ch;
    }

    // Append last word, if it isn't the trailing comment
    if (!comment && !word.isEmpty())
        words += word;

    // Now tokenize the words
    p2_token_v tokens;
    foreach(const QString& word, words)
        tokens += Token.token(word);

    p.tokens = tokens;
    p.words = words;
    p.cnt = p.tokens.count();
    p.idx = 0;

    return true;
}

/**
 * @brief Assemble a QStringList of lines of SPIN2 source code
 * @param p reference to the P2Params assembler state
 * @param source code
 * @return true on success
 */
bool P2Asm::assemble_pass(P2Params& p)
{
    p.pass_clear();
    bool multi_comment = false;

    foreach(QString line, p.source) {
        // Parse the next line
        p.line = line;
        p.lineno += 1;
        p.error.clear();
        p.words.clear();
        p.tokens.clear();

        if (line.startsWith(QStringLiteral("{")))
            multi_comment = true;

        if (multi_comment) {
            if (line.endsWith("}"))
                multi_comment = false;
            // skip over multi line comment
            results(p);
            continue;
        }

        // Skip over empty lines
        if (line.isEmpty()) {
            results(p);
            continue;
        }

        // Split line into words and tokenize it
        split_and_tokenize(p, line);

        // Set current program counter from next
        p.curr_pc = p.next_pc;
        // Assume the instruction advances by 1 long
        p.advance = 1;

        // Whenever the line starts with a token t_nothing, i.e. not an reserved name,
        // the first word is defined as a symbol for the current program counter value
        p.symbol.clear();
        if (p.idx < p.cnt) {
            QString symbol;

            switch (p.tokens.at(p.idx)) {
            case t_local:
                // append local name to section::function / section
                symbol = QString("%1::%2%3")
                         .arg(p.section)
                         .arg(p.function)
                         .arg(p.words.at(p.idx));
                break;

            case t_name:
                // append global name to section::symbol
                p.function = p.words.at(p.idx);
                symbol = QString("%1::%2%3")
                         .arg(p.section)
                         .arg(p.words.at(p.idx));
                break;

            case t__DAT:
                p.section = QStringLiteral("DAT");
                break;

            case t__CON:
                p.section = QStringLiteral("CON");
                break;

            case t__PUB:
                p.section = QStringLiteral("PUB");
                break;

            case t__PRI:
                p.section = QStringLiteral("PRI");
                break;

            case t__VAR:
                p.section = QStringLiteral("VAR");
                break;

            default:
                symbol.clear();
            }
            // defining a symbol?
            if (!symbol.isEmpty()) {
                p.idx++;   // skip over first word
                P2AsmSymbol sym = p.symbols.value(symbol);
                if (sym.isEmpty()) {
                    // Not defined yet
                    p.symbols.insert(symbol, p.curr_pc);
                } else if (p.pass > 1) {
                    p.symbols.setValue(symbol, p.curr_pc);
                } else {
                    // Already defined
                    p.error = tr("Symbol '%1' already defined in line %2: $%3")
                            .arg(symbol)
                            .arg(sym.reference(0))
                            .arg(sym.value<P2LONG>(), 6, 16, QChar('0'));
                    emit Error(p.lineno, p.error);
                }
                p.symbol = symbol;
            }
        }

        // Skip if no more tokens were found
        if (p.idx >= p.tokens.count()) {
            results(p);
            continue;
        }

        // Reset all instruction bits
        p.IR.opcode = 0;

        // Conditional execution prefix
        p.IR.op.cond = conditional(p, p.tokens.at(p.idx));

        // Expect a token for an instruction
        const p2_token_e inst = p.tokens.at(p.idx);
        bool success = false;

        switch (inst) {
        case t_ABS:
            success = asm_abs(p);
            break;

        case t_ADD:
            success = asm_add(p);
            break;

        case t_ADDCT1:
            success = asm_addct1(p);
            break;

        case t_ADDCT2:
            success = asm_addct2(p);
            break;

        case t_ADDCT3:
            success = asm_addct3(p);
            break;

        case t_ADDPIX:
            success = asm_addpix(p);
            break;

        case t_ADDS:
            success = asm_adds(p);
            break;

        case t_ADDSX:
            success = asm_addsx(p);
            break;

        case t_ADDX:
            success = asm_addx(p);
            break;

        case t_AKPIN:
            success = asm_akpin(p);
            break;

        case t_ALLOWI:
            success = asm_allowi(p);
            break;

        case t_ALTB:
            success = asm_altb(p);
            break;

        case t_ALTD:
            success = asm_altd(p);
            break;

        case t_ALTGB:
            success = asm_altgb(p);
            break;

        case t_ALTGN:
            success = asm_altgn(p);
            break;

        case t_ALTGW:
            success = asm_altgw(p);
            break;

        case t_ALTI:
            success = asm_alti(p);
            break;

        case t_ALTR:
            success = asm_altr(p);
            break;

        case t_ALTS:
            success = asm_alts(p);
            break;

        case t_ALTSB:
            success = asm_altsb(p);
            break;

        case t_ALTSN:
            success = asm_altsn(p);
            break;

        case t_ALTSW:
            success = asm_altsw(p);
            break;

        case t_AND:
            success = asm_and(p);
            break;

        case t_ANDN:
            success = asm_andn(p);
            break;

        case t_AUGD:
            success = asm_augd(p);
            break;

        case t_AUGS:
            success = asm_augs(p);
            break;

        case t_BITC:
            success = asm_bitc(p);
            break;

        case t_BITH:
            success = asm_bith(p);
            break;

        case t_BITL:
            success = asm_bitl(p);
            break;

        case t_BITNC:
            success = asm_bitnc(p);
            break;

        case t_BITNOT:
            success = asm_bitnot(p);
            break;

        case t_BITNZ:
            success = asm_bitnz(p);
            break;

        case t_BITRND:
            success = asm_bitrnd(p);
            break;

        case t_BITZ:
            success = asm_bitz(p);
            break;

        case t_BLNPIX:
            success = asm_blnpix(p);
            break;

        case t_BMASK:
            success = asm_bmask(p);
            break;

        case t_BRK:
            success = asm_brk(p);
            break;

        case t_CALL:
            success = asm_call(p);
            break;

        case t_CALLA:
            success = asm_calla(p);
            break;

        case t_CALLB:
            success = asm_callb(p);
            break;

        case t_CALLD:
            success = asm_calld(p);
            break;

        case t_CALLPA:
            success = asm_callpa(p);
            break;

        case t_CALLPB:
            success = asm_callpb(p);
            break;

        case t_CMP:
            success = asm_cmp(p);
            break;

        case t_CMPM:
            success = asm_cmpm(p);
            break;

        case t_CMPR:
            success = asm_cmpr(p);
            break;

        case t_CMPS:
            success = asm_cmps(p);
            break;

        case t_CMPSUB:
            success = asm_cmpsub(p);
            break;

        case t_CMPSX:
            success = asm_cmpsx(p);
            break;

        case t_CMPX:
            success = asm_cmpx(p);
            break;

        case t_COGATN:
            success = asm_cogatn(p);
            break;

        case t_COGBRK:
            success = asm_cogbrk(p);
            break;

        case t_COGID:
            success = asm_cogid(p);
            break;

        case t_COGINIT:
            success = asm_coginit(p);
            break;

        case t_COGSTOP:
            success = asm_cogstop(p);
            break;

        case t_CRCBIT:
            success = asm_crcbit(p);
            break;

        case t_CRCNIB:
            success = asm_crcnib(p);
            break;

        case t_DECMOD:
            success = asm_decmod(p);
            break;

        case t_DECOD:
            success = asm_decod(p);
            break;

        case t_DIRC:
            success = asm_dirc(p);
            break;

        case t_DIRH:
            success = asm_dirh(p);
            break;

        case t_DIRL:
            success = asm_dirl(p);
            break;

        case t_DIRNC:
            success = asm_dirnc(p);
            break;

        case t_DIRNOT:
            success = asm_dirnot(p);
            break;

        case t_DIRNZ:
            success = asm_dirnz(p);
            break;

        case t_DIRRND:
            success = asm_dirrnd(p);
            break;

        case t_DIRZ:
            success = asm_dirz(p);
            break;

        case t_DJF:
            success = asm_djf(p);
            break;

        case t_DJNF:
            success = asm_djnf(p);
            break;

        case t_DJNZ:
            success = asm_djnz(p);
            break;

        case t_DJZ:
            success = asm_djz(p);
            break;

        case t_DRVC:
            success = asm_drvc(p);
            break;

        case t_DRVH:
            success = asm_drvh(p);
            break;

        case t_DRVL:
            success = asm_drvl(p);
            break;

        case t_DRVNC:
            success = asm_drvnc(p);
            break;

        case t_DRVNOT:
            success = asm_drvnot(p);
            break;

        case t_DRVNZ:
            success = asm_drvnz(p);
            break;

        case t_DRVRND:
            success = asm_drvrnd(p);
            break;

        case t_DRVZ:
            success = asm_drvz(p);
            break;

        case t_ENCOD:
            success = asm_encod(p);
            break;

        case t_EXECF:
            success = asm_execf(p);
            break;

        case t_FBLOCK:
            success = asm_fblock(p);
            break;

        case t_FGE:
            success = asm_fge(p);
            break;

        case t_FGES:
            success = asm_fges(p);
            break;

        case t_FLE:
            success = asm_fle(p);
            break;

        case t_FLES:
            success = asm_fles(p);
            break;

        case t_FLTC:
            success = asm_fltc(p);
            break;

        case t_FLTH:
            success = asm_flth(p);
            break;

        case t_FLTL:
            success = asm_fltl(p);
            break;

        case t_FLTNC:
            success = asm_fltnc(p);
            break;

        case t_FLTNOT:
            success = asm_fltnot(p);
            break;

        case t_FLTNZ:
            success = asm_fltnz(p);
            break;

        case t_FLTRND:
            success = asm_fltrnd(p);
            break;

        case t_FLTZ:
            success = asm_fltz(p);
            break;

        case t_GETBRK:
            success = asm_getbrk(p);
            break;

        case t_GETBYTE:
            success = asm_getbyte(p);
            break;

        case t_GETCT:
            success = asm_getct(p);
            break;

        case t_GETNIB:
            success = asm_getnib(p);
            break;

        case t_GETPTR:
            success = asm_getptr(p);
            break;

        case t_GETQX:
            success = asm_getqx(p);
            break;

        case t_GETQY:
            success = asm_getqy(p);
            break;

        case t_GETRND:
            success = asm_getrnd(p);
            break;

        case t_GETSCP:
            success = asm_getscp(p);
            break;

        case t_GETWORD:
            success = asm_getword(p);
            break;

        case t_GETXACC:
            success = asm_getxacc(p);
            break;

        case t_HUBSET:
            success = asm_hubset(p);
            break;

        case t_IJNZ:
            success = asm_ijnz(p);
            break;

        case t_IJZ:
            success = asm_ijz(p);
            break;

        case t_INCMOD:
            success = asm_incmod(p);
            break;

        case t_JATN:
            success = asm_jatn(p);
            break;

        case t_JCT1:
            success = asm_jct1(p);
            break;

        case t_JCT2:
            success = asm_jct2(p);
            break;

        case t_JCT3:
            success = asm_jct3(p);
            break;

        case t_JFBW:
            success = asm_jfbw(p);
            break;

        case t_JINT:
            success = asm_jint(p);
            break;

        case t_JMP:
            success = asm_jmp(p);
            break;

        case t_JMPREL:
            success = asm_jmprel(p);
            break;

        case t_JNATN:
            success = asm_jnatn(p);
            break;

        case t_JNCT1:
            success = asm_jnct1(p);
            break;

        case t_JNCT2:
            success = asm_jnct2(p);
            break;

        case t_JNCT3:
            success = asm_jnct3(p);
            break;

        case t_JNFBW:
            success = asm_jnfbw(p);
            break;

        case t_JNINT:
            success = asm_jnint(p);
            break;

        case t_JNPAT:
            success = asm_jnpat(p);
            break;

        case t_JNQMT:
            success = asm_jnqmt(p);
            break;

        case t_JNSE1:
            success = asm_jnse1(p);
            break;

        case t_JNSE2:
            success = asm_jnse2(p);
            break;

        case t_JNSE3:
            success = asm_jnse3(p);
            break;

        case t_JNSE4:
            success = asm_jnse4(p);
            break;

        case t_JNXFI:
            success = asm_jnxfi(p);
            break;

        case t_JNXMT:
            success = asm_jnxmt(p);
            break;

        case t_JNXRL:
            success = asm_jnxrl(p);
            break;

        case t_JNXRO:
            success = asm_jnxro(p);
            break;

        case t_JPAT:
            success = asm_jpat(p);
            break;

        case t_JQMT:
            success = asm_jqmt(p);
            break;

        case t_JSE1:
            success = asm_jse1(p);
            break;

        case t_JSE2:
            success = asm_jse2(p);
            break;

        case t_JSE3:
            success = asm_jse3(p);
            break;

        case t_JSE4:
            success = asm_jse4(p);
            break;

        case t_JXFI:
            success = asm_jxfi(p);
            break;

        case t_JXMT:
            success = asm_jxmt(p);
            break;

        case t_JXRL:
            success = asm_jxrl(p);
            break;

        case t_JXRO:
            success = asm_jxro(p);
            break;

        case t_LOC:
            success = asm_loc(p);
            break;

        case t_LOCKNEW:
            success = asm_locknew(p);
            break;

        case t_LOCKREL:
            success = asm_lockrel(p);
            break;

        case t_LOCKRET:
            success = asm_lockret(p);
            break;

        case t_LOCKTRY:
            success = asm_locktry(p);
            break;

        case t_MERGEB:
            success = asm_mergeb(p);
            break;

        case t_MERGEW:
            success = asm_mergew(p);
            break;

        case t_MIXPIX:
            success = asm_mixpix(p);
            break;

        case t_MODCZ:
            success = asm_modcz(p);
            break;

        case t_MOV:
            success = asm_mov(p);
            break;

        case t_MOVBYTS:
            success = asm_movbyts(p);
            break;

        case t_MUL:
            success = asm_mul(p);
            break;

        case t_MULPIX:
            success = asm_mulpix(p);
            break;

        case t_MULS:
            success = asm_muls(p);
            break;

        case t_MUXC:
            success = asm_muxc(p);
            break;

        case t_MUXNC:
            success = asm_muxnc(p);
            break;

        case t_MUXNIBS:
            success = asm_muxnibs(p);
            break;

        case t_MUXNITS:
            success = asm_muxnits(p);
            break;

        case t_MUXNZ:
            success = asm_muxnz(p);
            break;

        case t_MUXQ:
            success = asm_muxq(p);
            break;

        case t_MUXZ:
            success = asm_muxz(p);
            break;

        case t_NEG:
            success = asm_neg(p);
            break;

        case t_NEGC:
            success = asm_negc(p);
            break;

        case t_NEGNC:
            success = asm_negnc(p);
            break;

        case t_NEGNZ:
            success = asm_negnz(p);
            break;

        case t_NEGZ:
            success = asm_negz(p);
            break;

        case t_NIXINT1:
            success = asm_nixint1(p);
            break;

        case t_NIXINT2:
            success = asm_nixint2(p);
            break;

        case t_NIXINT3:
            success = asm_nixint3(p);
            break;

        case t_NOP:
            success = asm_nop(p);
            break;

        case t_NOT:
            success = asm_not(p);
            break;

        case t_ONES:
            success = asm_ones(p);
            break;

        case t_OR:
            success = asm_or(p);
            break;

        case t_OUTC:
            success = asm_outc(p);
            break;

        case t_OUTH:
            success = asm_outh(p);
            break;

        case t_OUTL:
            success = asm_outl(p);
            break;

        case t_OUTNC:
            success = asm_outnc(p);
            break;

        case t_OUTNOT:
            success = asm_outnot(p);
            break;

        case t_OUTNZ:
            success = asm_outnz(p);
            break;

        case t_OUTRND:
            success = asm_outrnd(p);
            break;

        case t_OUTZ:
            success = asm_outz(p);
            break;

        case t_POLLATN:
            success = asm_pollatn(p);
            break;

        case t_POLLCT1:
            success = asm_pollct1(p);
            break;

        case t_POLLCT2:
            success = asm_pollct2(p);
            break;

        case t_POLLCT3:
            success = asm_pollct3(p);
            break;

        case t_POLLFBW:
            success = asm_pollfbw(p);
            break;

        case t_POLLINT:
            success = asm_pollint(p);
            break;

        case t_POLLPAT:
            success = asm_pollpat(p);
            break;

        case t_POLLQMT:
            success = asm_pollqmt(p);
            break;

        case t_POLLSE1:
            success = asm_pollse1(p);
            break;

        case t_POLLSE2:
            success = asm_pollse2(p);
            break;

        case t_POLLSE3:
            success = asm_pollse3(p);
            break;

        case t_POLLSE4:
            success = asm_pollse4(p);
            break;

        case t_POLLXFI:
            success = asm_pollxfi(p);
            break;

        case t_POLLXMT:
            success = asm_pollxmt(p);
            break;

        case t_POLLXRL:
            success = asm_pollxrl(p);
            break;

        case t_POLLXRO:
            success = asm_pollxro(p);
            break;

        case t_POP:
            success = asm_pop(p);
            break;

        case t_POPA:
            success = asm_popa(p);
            break;

        case t_POPB:
            success = asm_popb(p);
            break;

        case t_PUSH:
            success = asm_push(p);
            break;

        case t_PUSHA:
            success = asm_pusha(p);
            break;

        case t_PUSHB:
            success = asm_pushb(p);
            break;

        case t_QDIV:
            success = asm_qdiv(p);
            break;

        case t_QEXP:
            success = asm_qexp(p);
            break;

        case t_QFRAC:
            success = asm_qfrac(p);
            break;

        case t_QLOG:
            success = asm_qlog(p);
            break;

        case t_QMUL:
            success = asm_qmul(p);
            break;

        case t_QROTATE:
            success = asm_qrotate(p);
            break;

        case t_QSQRT:
            success = asm_qsqrt(p);
            break;

        case t_QVECTOR:
            success = asm_qvector(p);
            break;

        case t_RCL:
            success = asm_rcl(p);
            break;

        case t_RCR:
            success = asm_rcr(p);
            break;

        case t_RCZL:
            success = asm_rczl(p);
            break;

        case t_RCZR:
            success = asm_rczr(p);
            break;

        case t_RDBYTE:
            success = asm_rdbyte(p);
            break;

        case t_RDFAST:
            success = asm_rdfast(p);
            break;

        case t_RDLONG:
            success = asm_rdlong(p);
            break;

        case t_RDLUT:
            success = asm_rdlut(p);
            break;

        case t_RDPIN:
            success = asm_rdpin(p);
            break;

        case t_RDWORD:
            success = asm_rdword(p);
            break;

        case t_REP:
            success = asm_rep(p);
            break;

        case t_RESI0:
            success = asm_resi0(p);
            break;

        case t_RESI1:
            success = asm_resi1(p);
            break;

        case t_RESI2:
            success = asm_resi2(p);
            break;

        case t_RESI3:
            success = asm_resi3(p);
            break;

        case t_RET:
            success = asm_ret(p);
            break;

        case t_RETA:
            success = asm_reta(p);
            break;

        case t_RETB:
            success = asm_retb(p);
            break;

        case t_RETI0:
            success = asm_reti0(p);
            break;

        case t_RETI1:
            success = asm_reti1(p);
            break;

        case t_RETI2:
            success = asm_reti2(p);
            break;

        case t_RETI3:
            success = asm_reti3(p);
            break;

        case t_REV:
            success = asm_rev(p);
            break;

        case t_RFBYTE:
            success = asm_rfbyte(p);
            break;

        case t_RFLONG:
            success = asm_rflong(p);
            break;

        case t_RFVAR:
            success = asm_rfvar(p);
            break;

        case t_RFVARS:
            success = asm_rfvars(p);
            break;

        case t_RFWORD:
            success = asm_rfword(p);
            break;

        case t_RGBEXP:
            success = asm_rgbexp(p);
            break;

        case t_RGBSQZ:
            success = asm_rgbsqz(p);
            break;

        case t_ROL:
            success = asm_rol(p);
            break;

        case t_ROLBYTE:
            success = asm_rolbyte(p);
            break;

        case t_ROLNIB:
            success = asm_rolnib(p);
            break;

        case t_ROLWORD:
            success = asm_rolword(p);
            break;

        case t_ROR:
            success = asm_ror(p);
            break;

        case t_RQPIN:
            success = asm_rqpin(p);
            break;

        case t_SAL:
            success = asm_sal(p);
            break;

        case t_SAR:
            success = asm_sar(p);
            break;

        case t_SCA:
            success = asm_sca(p);
            break;

        case t_SCAS:
            success = asm_scas(p);
            break;

        case t_SETBYTE:
            success = asm_setbyte(p);
            break;

        case t_SETCFRQ:
            success = asm_setcfrq(p);
            break;

        case t_SETCI:
            success = asm_setci(p);
            break;

        case t_SETCMOD:
            success = asm_setcmod(p);
            break;

        case t_SETCQ:
            success = asm_setcq(p);
            break;

        case t_SETCY:
            success = asm_setcy(p);
            break;

        case t_SETD:
            success = asm_setd(p);
            break;

        case t_SETDACS:
            success = asm_setdacs(p);
            break;

        case t_SETINT1:
            success = asm_setint1(p);
            break;

        case t_SETINT2:
            success = asm_setint2(p);
            break;

        case t_SETINT3:
            success = asm_setint3(p);
            break;

        case t_SETLUTS:
            success = asm_setluts(p);
            break;

        case t_SETNIB:
            success = asm_setnib(p);
            break;

        case t_SETPAT:
            success = asm_setpat(p);
            break;

        case t_SETPIV:
            success = asm_setpiv(p);
            break;

        case t_SETPIX:
            success = asm_setpix(p);
            break;

        case t_SETQ:
            success = asm_setq(p);
            break;

        case t_SETQ2:
            success = asm_setq2(p);
            break;

        case t_SETR:
            success = asm_setr(p);
            break;

        case t_SETS:
            success = asm_sets(p);
            break;

        case t_SETSCP:
            success = asm_setscp(p);
            break;

        case t_SETSE1:
            success = asm_setse1(p);
            break;

        case t_SETSE2:
            success = asm_setse2(p);
            break;

        case t_SETSE3:
            success = asm_setse3(p);
            break;

        case t_SETSE4:
            success = asm_setse4(p);
            break;

        case t_SETWORD:
            success = asm_setword(p);
            break;

        case t_SETXFRQ:
            success = asm_setxfrq(p);
            break;

        case t_SEUSSF:
            success = asm_seussf(p);
            break;

        case t_SEUSSR:
            success = asm_seussr(p);
            break;

        case t_SHL:
            success = asm_shl(p);
            break;

        case t_SHR:
            success = asm_shr(p);
            break;

        case t_SIGNX:
            success = asm_signx(p);
            break;

        case t_SKIP:
            success = asm_skip(p);
            break;

        case t_SKIPF:
            success = asm_skipf(p);
            break;

        case t_SPLITB:
            success = asm_splitb(p);
            break;

        case t_SPLITW:
            success = asm_splitw(p);
            break;

        case t_STALLI:
            success = asm_stalli(p);
            break;

        case t_SUB:
            success = asm_sub(p);
            break;

        case t_SUBR:
            success = asm_subr(p);
            break;

        case t_SUBS:
            success = asm_subs(p);
            break;

        case t_SUBSX:
            success = asm_subsx(p);
            break;

        case t_SUBX:
            success = asm_subx(p);
            break;

        case t_SUMC:
            success = asm_sumc(p);
            break;

        case t_SUMNC:
            success = asm_sumnc(p);
            break;

        case t_SUMNZ:
            success = asm_sumnz(p);
            break;

        case t_SUMZ:
            success = asm_sumz(p);
            break;

        case t_TEST:
            success = asm_test(p);
            break;

        case t_TESTB:
            success = asm_testb_w(p);
            break;

        case t_TESTBN:
            success = asm_testbn_w(p);
            break;

        case t_TESTN:
            success = asm_testn(p);
            break;

        case t_TESTP:
            success = asm_testp_w(p);
            break;

        case t_TESTPN:
            success = asm_testpn_w(p);
            break;

        case t_TJF:
            success = asm_tjf(p);
            break;

        case t_TJNF:
            success = asm_tjnf(p);
            break;

        case t_TJNS:
            success = asm_tjns(p);
            break;

        case t_TJNZ:
            success = asm_tjnz(p);
            break;

        case t_TJS:
            success = asm_tjs(p);
            break;

        case t_TJV:
            success = asm_tjv(p);
            break;

        case t_TJZ:
            success = asm_tjz(p);
            break;

        case t_TRGINT1:
            success = asm_trgint1(p);
            break;

        case t_TRGINT2:
            success = asm_trgint2(p);
            break;

        case t_TRGINT3:
            success = asm_trgint3(p);
            break;

        case t_WAITATN:
            success = asm_waitatn(p);
            break;

        case t_WAITCT1:
            success = asm_waitct1(p);
            break;

        case t_WAITCT2:
            success = asm_waitct2(p);
            break;

        case t_WAITCT3:
            success = asm_waitct3(p);
            break;

        case t_WAITFBW:
            success = asm_waitfbw(p);
            break;

        case t_WAITINT:
            success = asm_waitint(p);
            break;

        case t_WAITPAT:
            success = asm_waitpat(p);
            break;

        case t_WAITSE1:
            success = asm_waitse1(p);
            break;

        case t_WAITSE2:
            success = asm_waitse2(p);
            break;

        case t_WAITSE3:
            success = asm_waitse3(p);
            break;

        case t_WAITSE4:
            success = asm_waitse4(p);
            break;

        case t_WAITX:
            success = asm_waitx(p);
            break;

        case t_WAITXFI:
            success = asm_waitxfi(p);
            break;

        case t_WAITXMT:
            success = asm_waitxmt(p);
            break;

        case t_WAITXRL:
            success = asm_waitxrl(p);
            break;

        case t_WAITXRO:
            success = asm_waitxro(p);
            break;

        case t_WFBYTE:
            success = asm_wfbyte(p);
            break;

        case t_WFLONG:
            success = asm_wflong(p);
            break;

        case t_WFWORD:
            success = asm_wfword(p);
            break;

        case t_WMLONG:
            success = asm_wmlong(p);
            break;

        case t_WRBYTE:
            success = asm_wrbyte(p);
            break;

        case t_WRC:
            success = asm_wrc(p);
            break;

        case t_WRFAST:
            success = asm_wrfast(p);
            break;

        case t_WRLONG:
            success = asm_wrlong(p);
            break;

        case t_WRLUT:
            success = asm_wrlut(p);
            break;

        case t_WRNC:
            success = asm_wrnc(p);
            break;

        case t_WRNZ:
            success = asm_wrnz(p);
            break;

        case t_WRPIN:
            success = asm_wrpin(p);
            break;

        case t_WRWORD:
            success = asm_wrword(p);
            break;

        case t_WRZ:
            success = asm_wrz(p);
            break;

        case t_WXPIN:
            success = asm_wxpin(p);
            break;

        case t_WYPIN:
            success = asm_wypin(p);
            break;

        case t_XCONT:
            success = asm_xcont(p);
            break;

        case t_XINIT:
            success = asm_xinit(p);
            break;

        case t_XOR:
            success = asm_xor(p);
            break;

        case t_XORO32:
            success = asm_xoro32(p);
            break;

        case t_XSTOP:
            success = asm_xstop(p);
            break;

        case t_XZERO:
            success = asm_xzero(p);
            break;

        case t_ZEROX:
            success = asm_zerox(p);
            break;

        case t__BYTE:
            success = asm_byte(p);
            break;

        case t__WORD:
            success = asm_word(p);
            break;

        case t__LONG:
            success = asm_long(p);
            break;

        case t__RES:
            success = asm_res(p);
            break;

        case t__FIT:
            success = asm_fit(p);
            break;

        case t__ORG:
            success = asm_org(p);
            break;

        case t__ORGH:
            success = asm_orgh(p);
            break;

        case t__ASSIGN:
            success = asm_assign(p);
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
            p.error = tr("Not an instruction: %1").arg(line);
            emit Error(p.lineno, p.error);
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
            p.error = tr("Multiple conditionals in line: %1").arg(line);
            emit Error(p.lineno, p.error);
            break;

        default:
            // Handle non-instruction tokens
            break;
        }

        if (success) {
            results(p, true);
            // Calculate next PC for regular instructions
            if (p.curr_pc < PC_LONGS) {
                p.next_pc = p.curr_pc + p.advance;
            } else {
                p.next_pc = p.curr_pc + p.advance * 4;
            }
            if (p.next_pc > p.last_pc)
                p.last_pc = p.next_pc;
        } else {
            results(p);
        }
    }

    return true;
}

bool P2Asm::assemble(P2Params& p, const QStringList& source)
{
    bool success = true;
    p.source = source;
    success = assemble_pass(p) &&
              assemble_pass(p);
    return success;
}

/**
 * @brief Assemble a source file
 * @param filename name of the SPIN2 source
 * @return QByteArray containing the binary
 */
bool P2Asm::assemble(P2Params& p, const QString& filename)
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
    return assemble(p, source);
}

/**
 * @brief Return number of tokens/words left
 * @param p reference to P2Params
 * @return
 */
int P2Asm::left(const P2Params& p)
{
    return p.cnt - p.idx;
}

/**
 * @brief Store the results and append a line to the listing
 * @param p reference to P2Params
 * @param opcode true, if the p.IR field contains an opcode
 */
void P2Asm::results(P2Params& p, bool opcode)
{
    const QString line = p.line;
    const int lineno = p.lineno;
    const P2LONG PC = p.curr_pc;
    const p2_opcode_u IR = p.IR;

    QString output;
    if (opcode) {
        if (p.advance) {
            p.h_PC.insert(lineno, PC);
            p.h_IR.insert(lineno, IR);
            // opcode was constructed
            output = QString("%1 %2 [%3] %4")
                      .arg(lineno, -6)
                      .arg(PC, 6, 16, QChar('0'))
                      .arg(IR.opcode, 8, 16, QChar('0'))
                      .arg(line);
        } else {
            // assignment to symbol
            p.h_IR.insert(lineno, IR);
            output = QString("%1 %2 <%3> %4")
                      .arg(lineno, -6)
                      .arg(PC, 6, 16, QChar('0'))
                      .arg(IR.opcode, 8, 16, QChar('0'))
                      .arg(line);
        }
    } else {
        // comment or non code generating instruction
        output = QString("%1 %2 -%3- %4")
                  .arg(lineno, -6)
                  .arg(PC, 6, 16, QChar('0'))
                  .arg(QStringLiteral("--------"))
                  .arg(line);
    }
    p.listing.append(output);
    p.h_tokens.insert(lineno, p.tokens);
    if (p.error.isEmpty())
        return;
    p.h_errors.insert(p.lineno, p.error);
}

/**
 * @brief Convert a string of binary digits into an unsigned value
 * @param pos current position in str
 * @param str binary digits
 * @return value of binary digits in str
 */
P2Atom P2Asm::from_bin(int& pos, const QString& str, const QString& stop)
{
    P2QUAD bits = 0;
    int nbits = 0;
    P2Atom value;

    if (QChar('%') == str.at(pos))
        ++pos;

    for (/**/; pos < str.length(); pos++) {
        QChar ch = str[pos];
        if (stop.contains(ch))
            break;
        const int idx = bin_digits.indexOf(ch);
        if (idx < 0)
            continue;
        bits = (bits << 1) | static_cast<uint>(idx);
        nbits += 1;
        while (nbits >= 32) {
            value.append(32, bits);
            bits >>= 32;
            nbits -= 32;
        }
    }
    value.append(nbits, bits);
    return value;
}

/**
 * @brief Convert a string of octal digits into an unsigned value
 * @param pos current position in str
 * @param str octal digits
 * @return value of octal digits in str
 */
P2Atom P2Asm::from_oct(int& pos, const QString& str, const QString& stop)
{
    P2QUAD bits = 0;
    int nbits = 0;
    P2Atom value;

    if (QChar('%') == str.at(pos))
        ++pos;

    for (/**/; pos < str.length(); pos++) {
        QChar ch = str[pos];
        if (stop.contains(ch))
            break;
        const int idx = oct_digits.indexOf(ch);
        if (idx < 0)
            continue;
        bits = (bits << 3) | static_cast<uint>(idx);
        nbits += 3;
        while (nbits >= 32) {
            value.append(32, bits);
            bits >>= 32;
            nbits -= 32;
        }
    }

    value.append(nbits, bits);
    return value;
}

/**
 * @brief Convert a string of decimal digits into an unsigned value
 * @param pos current position in str
 * @param str decimal digits
 * @return value of decimal digits in str
 */
P2Atom P2Asm::from_dec(int& pos, const QString& str, const QString& stop)
{
    P2QUAD bits = 0;
    P2Atom value;

    if (QChar('%') == str.at(pos))
        ++pos;

    for (/**/; pos < str.length(); pos++) {
        QChar ch = str[pos];
        if (stop.contains(ch))
            break;
        const int idx = dec_digits.indexOf(ch);
        if (idx < 0)
            continue;
        bits = (bits * 10) | static_cast<uint>(idx);
        // no way to handle overflow ?
    }
    // append all significant bits
    while (bits) {
        value.append(8, bits);
        bits >>= 8;
    }
    return value;
}

/**
 * @brief Convert a string of hexadecimal digits into an unsigned value
 * @param str hexadecimal digits
 * @return value of hexadecimal digits in str
 */
P2Atom P2Asm::from_hex(int& pos, const QString& str, const QString& stop)
{
    P2QUAD bits = 0;
    int nbits = 0;
    P2Atom value;

    if (QChar('%') == str.at(pos))
        ++pos;

    for (/**/; pos < str.length(); pos++) {
        QChar ch = str[pos];
        if (stop.contains(ch))
            break;
        const int idx = hex_digits.indexOf(ch);
        if (idx < 0)
            continue;
        bits = (bits << 4) | static_cast<uint>(idx);
        nbits += 4;
        while (nbits >= 32) {
            value.append(32, bits);
            bits >>= 32;
            nbits -= 32;
        }
    }

    value.append(nbits, bits);
    return value;
}

/**
 * @brief Convert a string of ASCII characters into an unsigned value
 * @param str ASCII characters
 * @param stop characters to stop at
 * @return value as 64 bit unsigned
 */
P2Atom P2Asm::from_str(int& pos, const QString& str)
{
    P2Atom value;
    QChar instring = str.at(pos++);
    bool escaped = false;

    while (pos < str.length()) {
        QChar ch = str.at(pos);
        if (escaped) {
            value.append(8, static_cast<P2QUAD>(ch.toLatin1()));
            escaped = false;
        } else if (ch == instring) {
            return value;
        } else if (ch == QChar('\\')) {
            escaped = true;
        } else {
            value.append(8, static_cast<P2QUAD>(ch.toLatin1()));
        }
        pos++;
    }

    return value;
}

/**
 * @brief Skip over spaces in %str at %pos
 * @param pos position in string
 * @param str reference to string
 */
void P2Asm::skip_spc(int &pos, const QString& str)
{
    while (pos < str.length() && str.at(pos).isSpace())
        pos++;
}

/**
 * @brief Parse an atomic part of an expression
 * @param p reference to P2Params
 * @param pos position in word where to start
 * @param str string to parse
 * @return value of the atom
 */
P2Atom P2Asm::parse_atom(P2Params& p, int& pos, const QString& str)
{
    P2Atom atom;

    // Unary minus
    bool negate = false;
    for (/* */; pos < str.length(); pos++) {
        skip_spc(pos, str);
        if (QChar('-') == str.at(pos)) {
            negate = !negate;
            continue;
        }
        break;
    }

    // Rest of the string
    QString rest = str.mid(pos);

    QString symbol;
    if (rest.startsWith(QChar('.'))) {
        symbol = QString("%1::%2%3").arg(p.section).arg(p.function).arg(rest);
    } else {
        symbol = QString("%1::%2").arg(p.section).arg(rest);
    }

    // TODO: find symbol in other sections?
    bool is_symbol = p.symbols.contains(symbol);

    const p2_token_e tok = p.tokens.value(p.idx, t_nothing);
    switch (tok) {
    case t_nothing:
        break;
    case t_local:
    case t_name:
        p.symbols.addReference(symbol, p.lineno);
        if (is_symbol) {
            atom.append(32, p.symbols.value<P2LONG>(symbol));
        }
        break;

    case t_value_bin:
        atom = from_bin(pos, str, stop_expr);
        break;

    case t_value_oct:
        atom = from_oct(pos, str, stop_expr);
        break;

    case t_value_dec:
        atom = from_dec(pos, str, stop_expr);
        break;

    case t_value_hex:
        atom = from_hex(pos, str, stop_expr);
        break;

    case t_string:
        atom = from_str(pos, str);
        break;

    case t_PA:
        pos += 2;
        atom = offs_PA;
        break;

    case t_PB:
        pos += 2;
        atom = offs_PB;
        break;

    case t_PTRA:
        pos += 4;
        atom = offs_PTRA;
        break;

    case t_PTRB:
        pos += 4;
        atom = offs_PTRB;
        break;

    default:
        p.error = tr("Reserved word used as parameter: %1").arg(str);
        return atom;
    }
    return atom;
}

/**
 * @brief Parse an an expression of factors (multiplicators or divisors)
 * @param p reference to P2Params
 * @param pos position in %str where to start
 * @param str string to parse
 * @return P2Atom containing the result
 */
P2Atom P2Asm::parse_factors(P2Params& p, int& pos, const QString& str)
{
    P2Atom atom = parse_atom(p, pos, str);

    for (;;) {
        skip_spc(pos, str);
        if (pos >= str.length())
            return atom;

        p2_token_e op = Token.at_mulop(pos, str);
        if (t_invalid == op)
            return atom;

        pos += 1;
        P2Atom atom2 = parse_atom(p, pos, str);
        if (!atom2.isValid()) {
            p.error = tr("Invalid character in expression (factors): %1").arg(str.mid(pos));
            return atom;
        }

        P2QUAD value1 = atom.toQUAD();
        P2QUAD value2 = atom2.toQUAD();
        switch (op) {
        case t__MUL:
            atom = value1 * value2;
            break;
        case t__DIV:
            atom = value2 ? value1 / value2 : ~Q_UINT64_C(0);
            break;
        case t__MOD:
            atom = value2 ? value1 % value2 : ~Q_UINT64_C(0);
            break;
        default:
            Q_ASSERT_X(false, "factors ops", "invalid op");
        }
    }
}


/**
 * @brief Parse an an expression of summands (addends and minuends)
 * @param p reference to P2Params
 * @param pos position in %str where to start
 * @param str string to parse
 * @return P2Atom containing the result
 */
P2Atom P2Asm::parse_summands(P2Params& p, int& pos, const QString& str)
{
    P2Atom atom = parse_factors(p, pos, str);

    for (;;) {
        skip_spc(pos, str);
        if (pos >= str.length())
            return atom;

        p2_token_e op = Token.at_addop(pos, str);
        if (t_invalid == op)
            return atom;

        pos += 1;
        P2Atom atom2 = parse_factors(p, pos, str);
        if (!atom2.isValid()) {
            p.error = tr("Invalid character in expression (summands): %1").arg(str.mid(pos));
            return atom;
        }

        P2QUAD value1 = atom.toQUAD();
        P2QUAD value2 = atom2.toQUAD();
        switch (op) {
        case t__ADD:
            value1 = value1 + value2;
            atom.set(64, value1);
            break;
        case t__SUB:
            value1 = value1 - value2;
            atom.set(64, value1);
            break;
        default:
            Q_ASSERT_X(false, "summands ops", "Invalid op");
        }
    }
}

/**
 * @brief Parse an an expression of binary operations (and, or, xor, left shift, right shift)
 * @param p reference to P2Params
 * @param pos position in %str where to start
 * @param str string to parse
 * @return P2Atom containing the result
 */
P2Atom P2Asm::parse_binops(P2Params& p, int& pos, const QString& str)
{
    P2Atom atom = parse_summands(p, pos, str);

    for (;;) {
        skip_spc(pos, str);
        if (pos >= str.length())
            return atom;

        p2_token_e op = Token.at_binop(pos, str);
        if (t_invalid == op)
            return atom;

        P2Atom atom2 = parse_summands(p, pos, str);
        if (!atom2.isValid()) {
            p.error = tr("Invalid character in expression (summands): %1").arg(str.mid(pos));
            return atom;
        }

        P2QUAD value1 = atom.toQUAD();
        P2QUAD value2 = atom2.toQUAD();
        switch (op) {
        case t__AND:
            atom = value1 & value2;
            break;
        case t__OR:
            atom = value1 | value2;
            break;
        case t__XOR:
            atom = value1 ^ value2;
            break;
        case t__SHL:
            atom = value1 << value2;
            break;
        case t__SHR:
            atom = value1 >> value2;
            break;
        default:
            Q_ASSERT_X(false, "binary ops", "Invalid op");
        }
    }
}

/**
 * @brief Evaluate an expression
 * @param p reference to P2Params
 * @param imm_to put immediate flag into: -1=nowhere (default), 0=imm, 1=wz, 2=wc
 * @return QVariant with the value of the expression
 */
P2Atom P2Asm::parse_expression(P2Params& p, imm_to_e imm_to)
{
    if (p.idx >= p.cnt)
        return P2Atom();

    QString str = p.words.value(p.idx);
    bool imm = str.startsWith(QChar('#'));
    bool imm2 = false;

    int pos = 0;
    if (imm) {
        pos++;
    }

    skip_spc(pos, str);

    // skip a second immediate
    if (pos < str.length() && QChar('#') == str.at(pos)) {
        imm2 = true;
        pos++;
    }
    // TODO: special meaning of this?
    Q_UNUSED(imm2)

    P2Atom value = parse_binops(p, pos, str);

    // Set immediate flag, if specified
    switch (imm_to) {
    case immediate_imm:
        p.IR.op.im = imm;
        break;
    case immediate_wz:
        p.IR.op.wz = imm;
        break;
    case immediate_wc:
        p.IR.op.wc = imm;
        break;
    default:
        // silence compiler warnings
        break;
    }

    p.idx++;
    return value;
}

/**
 * @brief Expect end of line, i.e. no more parameters
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::end_of_line(P2Params& p, bool binary)
{
    if (p.idx < p.cnt) {
        // ignore extra parameters?
        p.error = tr("Found extra parameters: %1")
                       .arg(p.words.mid(p.idx).join(QChar::Space));
        return false;
    }

    if (binary) {
        if (p.curr_pc < PC_LONGS) {
            p.MEM.L[p.curr_pc] = p.IR.opcode;
        } else if (p.curr_pc < MEM_SIZE) {
            p.MEM.L[p.curr_pc / 4] = p.IR.opcode;
        }
    }
    return true;
}

/**
 * @brief A comma is expected
 * @brief params reference to the assembler parameters
 * @return true if comma found, false otherwise
 */
bool P2Asm::parse_comma(P2Params& p)
{
    if (p.idx >= p.cnt) {
        p.error = tr("Expected %1 but found %2.")
                       .arg(Token.string(t_comma))
                       .arg(tr("end of line"));
        return false;
    }
    if (t_comma != p.tokens.at(p.idx)) {
        p.error = tr("Expected %1 but found %2.")
                       .arg(Token.string(t_comma))
                       .arg(Token.string(p.tokens.at(p.idx)));
        return false;
    }
    p.idx++;
    return true;
}

/**
 * @brief An optioncal comma is skipped
 * @brief params reference to the assembler parameters
 */
void P2Asm::optional_comma(P2Params& p)
{
    if (p.idx >= p.cnt)
        return;
    if (t_comma != p.tokens.at(p.idx))
        return;
    p.idx++;
}

/**
 * @brief Optional WC, WZ, or WCZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::optional_wcz(P2Params& p)
{
    if (p.idx < p.cnt) {
        p2_token_e tok = p.tokens.value(p.idx);
        switch (tok) {
        case t_WC:
            p.IR.op.wc = true;
            p.idx++;
            break;
        case t_WZ:
            p.IR.op.wz = true;
            p.idx++;
            break;
        case t_WCZ:
            p.IR.op.wc = true;
            p.IR.op.wz = true;
            p.idx++;
            break;
        default:
            p.error = tr("Unexpected flag update '%1' not %2")
                           .arg(p.words.value(p.idx))
                           .arg(tr("WC, WZ, or WCZ"));
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
bool P2Asm::optional_wc(P2Params& p)
{
    if (p.idx < p.cnt) {
        p2_token_e tok = p.tokens.value(p.idx);
        switch (tok) {
        case t_WC:
            p.IR.op.wc = true;
            p.idx++;
            break;
        default:
            p.error = tr("Unexpected flag update '%1' not %2")
                           .arg(p.words.value(p.idx)
                                .arg(tr("WC")));
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
bool P2Asm::optional_wz(P2Params& p)
{
    if (p.idx < p.cnt) {
        p2_token_e tok = p.tokens.value(p.idx);
        switch (tok) {
        case t_WZ:
            p.IR.op.wz = true;
            p.idx++;
            break;
        default:
            p.error = tr("Unexpected flag update '%1' not %2")
                           .arg(p.words.value(p.idx))
                           .arg(tr("WZ"));
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
bool P2Asm::asm_assign(P2Params& p)
{
    p.idx++;           // skip over token
    p.advance = 0;     // No PC increment
    P2Atom atom = parse_expression(p);
    const P2LONG value = atom.toLONG();
    p.IR.opcode = value;
    p.symbols.setValue(p.symbol, value);
    return end_of_line(p, false);
}

/**
 * @brief Origin operation
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::asm_org(P2Params& p)
{
    p.idx++;           // skip over token
    p.advance = 0;     // No PC increment
    P2Atom atom = parse_expression(p);
    P2LONG value = atom.toLONG();
    if (atom.isEmpty())
        value = p.last_pc;
    p.IR.opcode = p.curr_pc = p.next_pc = value;
    p.symbols.setValue(p.symbol, value);
    return end_of_line(p, false);
}

/**
 * @brief Origin high operation
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::asm_orgh(P2Params& p)
{
    p.idx++;           // skip over token
    p.advance = 0;     // No PC increment
    P2Atom atom = parse_expression(p);
    P2LONG value = atom.toLONG();
    if (atom.isEmpty())
        value = HUB_ADDR;
    p.IR.opcode = p.next_pc =  p.last_pc = value;
    p.symbols.setValue(p.symbol, value);
    return end_of_line(p, false);
}

/**
 * @brief Expect instruction with optional WC, WZ, or WCZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_with_wcz(P2Params& p)
{
    optional_wcz(p);
    return end_of_line(p);
}

/**
 * @brief Expect instruction with optional WC
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_with_wc(P2Params& p)
{
    optional_wc(p);
    return end_of_line(p);
}


/**
 * @brief Expect instruction with optional WZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_with_wz(P2Params& p)
{
    optional_wz(p);
    return end_of_line(p);
}

/**
 * @brief Expect no more parameters after instruction
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_inst(P2Params& p)
{
    return end_of_line(p);
}


/**
 * @brief Expect parameters for D and {#}S
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_d_imm_s(P2Params& p)
{
    P2Atom dst = parse_expression(p);
    if (!parse_comma(p))
        return false;
    P2Atom src = parse_expression(p, immediate_imm);
    p.IR.op.dst = dst.toLONG();
    p.IR.op.src = src.toLONG();
    return end_of_line(p);
}

/**
 * @brief Expect parameters for D and optional WC, WZ, or WCZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_d_cz(P2Params& p)
{
    P2Atom dst = parse_expression(p);
    p.IR.op.dst = dst.toLONG();
    optional_wcz(p);
    return end_of_line(p);
}

/**
 * @brief Expect optional WC, WZ, or WCZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_cz(P2Params& p)
{
    optional_wcz(p);
    return end_of_line(p);
}

/**
 * @brief Expect conditional for C, conditional for Z, and optional WC, WZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_cccc_zzzz_wcz(P2Params& p)
{
    p2_cond_e cccc = parse_modcz(p, p.tokens.value(p.idx));
    if (!parse_comma(p))
        return false;
    p2_cond_e zzzz = parse_modcz(p, p.tokens.value(p.idx));
    p.IR.op.dst = static_cast<P2LONG>((cccc << 4) | zzzz);
    optional_wcz(p);
    return end_of_line(p);
}

/**
 * @brief Expect parameters for D
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_d(P2Params& p)
{
    P2Atom dst = parse_expression(p);
    p.IR.op.dst = dst.toLONG();
    return end_of_line(p);
}

/**
 * @brief Expect parameters for {#}D setting WZ for immediate
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_wz_d(P2Params& p)
{
    P2Atom dst = parse_expression(p, immediate_wz);
    p.IR.op.dst = dst.toLONG();
    return end_of_line(p);
}

/**
 * @brief Expect parameters for {#}D
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_imm_d(P2Params& p)
{
    P2Atom dst = parse_expression(p, immediate_imm);
    p.IR.op.dst = dst.toLONG();
    return end_of_line(p);
}

/**
 * @brief Expect parameters for {#}D, and optional WC, WZ, or WCZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_imm_d_wcz(P2Params& p)
{
    P2Atom dst = parse_expression(p, immediate_imm);
    p.IR.op.dst = dst.toLONG();
    optional_wcz(p);
    return end_of_line(p);
}

/**
 * @brief Expect parameters for {#}D, and optional WC
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_imm_d_wc(P2Params& p)
{
    P2Atom dst = parse_expression(p, immediate_imm);
    p.IR.op.dst = dst.toLONG();
    optional_wc(p);
    return end_of_line(p);
}

/**
 * @brief Expect parameters for D, and {#}S, and optional WC, WZ, or WCZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_d_imm_s_wcz(P2Params& p)
{
    P2Atom dst = parse_expression(p);
    if (!parse_comma(p))
        return false;
    P2Atom src = parse_expression(p, immediate_imm);
    p.IR.op.dst = dst.toLONG();
    p.IR.op.src = src.toLONG();
    optional_wcz(p);
    return end_of_line(p);
}

/**
 * @brief Expect parameters for D, and {#}S, and optional WC
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_d_imm_s_wc(P2Params& p)
{
    P2Atom dst = parse_expression(p);
    if (!parse_comma(p))
        return false;
    P2Atom src = parse_expression(p, immediate_imm);
    p.IR.op.dst = dst.toLONG();
    p.IR.op.src = src.toLONG();
    optional_wc(p);
    return end_of_line(p);
}

/**
 * @brief Expect parameters for D, and {#}S, and optional WZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_d_imm_s_wz(P2Params& p)
{
    P2Atom dst = parse_expression(p);
    if (!parse_comma(p))
        return false;
    P2Atom src = parse_expression(p, immediate_imm);
    p.IR.op.dst = dst.toLONG();
    p.IR.op.src = src.toLONG();
    optional_wz(p);
    return end_of_line(p);
}

/**
 * @brief Expect parameters for {#}D, and {#}S
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_wz_d_imm_s(P2Params& p)
{
    P2Atom dst = parse_expression(p, immediate_wz);
    if (!parse_comma(p))
        return false;
    P2Atom src = parse_expression(p, immediate_imm);
    p.IR.op.dst = dst.toLONG();
    p.IR.op.src = src.toLONG();
    return end_of_line(p);
}

/**
 * @brief Expect parameters for D, and {#}S, and #N (0 … 7)
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_d_imm_s_nnn(P2Params& p, uint max)
{
    P2Atom dst = parse_expression(p);
    if (!parse_comma(p))
        return false;
    P2Atom src = parse_expression(p, immediate_imm);
    p.IR.op.dst = dst.toLONG();
    p.IR.op.src = src.toLONG();
    if (parse_comma(p)) {
        if (p.idx < p.cnt) {
            P2Atom n = parse_expression(p);
            if (n.isEmpty()) {
                p.error = tr("Expected immediate #n");
                return false;
            }
            if (n.toLONG() > max) {
                p.error = tr("Immediate #n not in 0-%1 (%2)")
                          .arg(max)
                          .arg(n.toLONG());
                return false;
            }
            P2LONG opcode = static_cast<P2LONG>(n.toLONG() & max) << 18;
            p.IR.opcode |= opcode;
        } else {
            p.error = tr("Missing immediate #n");
            return false;
        }
    } else {
        // Default is 0
    }
    return end_of_line(p);
}

/**
 * @brief Expect parameters for {#}S
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_imm_s(P2Params& p)
{
    P2Atom src = parse_expression(p, immediate_imm);
    p.IR.op.src = src.toLONG();
    return end_of_line(p);
}

/**
 * @brief Expect parameters for {#}S, and optional WC
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_imm_s_wcz(P2Params& p)
{
    P2Atom src = parse_expression(p, immediate_imm);
    p.IR.op.src = src.toLONG();
    return end_of_line(p);
}


/**
 * @brief Expect parameters for #AAAAAAAAAAAA (19 bit address for CALL/CALLA/CALLB/LOC)
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_ptr_pc_abs(P2Params& p)
{
    p2_token_e dst = p.tokens.value(p.idx);
    switch (dst) {
    case t_PA:
        break;
    case t_PB:
        p.IR.op.wz = true;
        break;
    case t_PTRA:
        p.IR.op.wc = true;
        break;
    case t_PTRB:
        p.IR.op.wz = true;
        p.IR.op.wc = true;
        break;
    default:
        p.error = tr("Invalid pointer parameter: %1")
                       .arg(p.words.mid(p.idx).join(QChar::Space));
        return false;
    }
    p.idx++;
    if (!parse_comma(p))
        return false;

    P2Atom atom = parse_expression(p);
    P2LONG addr = atom.toLONG();
    p.IR.opcode |= addr & A20MASK;

    return end_of_line(p);
}

/**
 * @brief Expect parameters for #AAAAAAAAAAAA (19 bit address for CALL/CALLA/CALLB)
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_pc_abs(P2Params& p)
{
    P2Atom atom = parse_expression(p);
    P2LONG addr = atom.toLONG();
    p.IR.opcode |= addr & A20MASK;

    return end_of_line(p);
}

/**
 * @brief Expect parameters for #AAAAAAAAAAAA (23 bit address for AUGD/AUGS)
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::parse_imm23(P2Params& p, QVector<p2_inst7_e> aug)
{
    P2Atom atom = parse_expression(p);
    P2LONG addr = atom.toLONG();
    p.IR.op.inst = static_cast<p2_inst7_e>(p.IR.op.inst | aug[(addr >> 21) & 3]);

    return end_of_line(p);
}

/**
 * @brief Expect one or more bytes of data
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::asm_byte(P2Params& p)
{
    p.idx++;
    while (p.idx < p.cnt) {
        P2Atom data = parse_expression(p);
        p.IR.opcode = data.toLONG();
        optional_comma(p);
    }
    return end_of_line(p);
}

/**
 * @brief Expect one or more bytes of data
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::asm_word(P2Params& p)
{
    p.idx++;
    while (p.idx < p.cnt) {
        P2Atom data = parse_expression(p);
        p.IR.opcode = data.toLONG();
        optional_comma(p);
    }
    return end_of_line(p);
}

/**
 * @brief Expect one or more bytes of data
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::asm_long(P2Params& p)
{
    p.idx++;
    while (p.idx < p.cnt) {
        P2Atom data = parse_expression(p);
        p.IR.opcode = data.toLONG();
        optional_comma(p);
    }
    return end_of_line(p);
}

/**
 * @brief Expect one or more bytes of data
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::asm_res(P2Params& p)
{
    p.idx++;
    while (p.idx < p.cnt) {
        P2Atom data = parse_expression(p);
        p.IR.opcode = 0;
        optional_comma(p);
    }
    return end_of_line(p);
}

/**
 * @brief Expect an address which is checked for being >= p.curr_pc
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::asm_fit(P2Params& p)
{
    p.idx++;
    P2Atom data = parse_expression(p);
    if (data.toLONG() <= p.curr_pc) {
        p.error = tr("Code does not fit below $%1 (origin == $%2)")
                  .arg(data.toLONG(), 0, 16)
                  .arg(p.curr_pc, 0, 16);
    }
    return end_of_line(p);
}

/**
 * @brief No operation.
 *
 * 0000 0000000 000 000000000 000000000
 *
 * NOP
 *
 *
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_nop(P2Params& p)
{
    p.idx++;
    p.IR.opcode = 0;
    return parse_inst(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_ror(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_ROR;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rol(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_ROR;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_shr(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_SHR;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_shl(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_SHL;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rcr(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_RCR;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rcl(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_RCL;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_sar(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_SAR;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_sal(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_SAL;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_add(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_ADD;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_addx(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_ADDX;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_adds(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_ADDS;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_addsx(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_ADDSX;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_sub(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_SUB;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_subx(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_SUBX;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_subs(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_SUBS;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_subsx(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_SUBSX;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_cmp(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_CMP;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_cmpx(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_CMPX;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_cmps(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_CMPS;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_cmpsx(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_CMPSX;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_cmpr(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_CMPR;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_cmpm(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_CMPM;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_subr(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_SUBR;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_cmpsub(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_CMPSUB;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_fge(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_FGE;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_fle(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_FLE;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_fges(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_FGES;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_fles(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_FLES;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_sumc(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_SUMC;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_sumnc(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_SUMNC;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_sumz(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_SUMZ;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_sumnz(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_SUMNZ;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_testb_w(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_TESTB_W;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_testbn_w(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_TESTBN_W;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_testb_and(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_TESTB_AND;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_testbn_and(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_TESTBN_AND;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_testb_or(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_TESTB_OR;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_testbn_or(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_TESTBN_OR;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_testb_xor(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_TESTB_XOR;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_testbn_xor(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_TESTBN_XOR;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_bitl(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_BITL;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_bith(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_BITH;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_bitc(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_BITC;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_bitnc(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_BITNC;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_bitz(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_BITZ;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_bitnz(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_BITNZ;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_bitrnd(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_BITRND;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_bitnot(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_BITNOT;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_and(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_AND;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_andn(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_ANDN;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_or(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OR;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_xor(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_XOR;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_muxc(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_MUXC;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_muxnc(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_MUXNC;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_muxz(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_MUXZ;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_muxnz(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_MUXNZ;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_mov(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_MOV;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_not(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_NOT;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_abs(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_ABS;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_neg(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_NEG;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_negc(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_NEGC;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_negnc(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_NEGNC;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_negz(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_NEGZ;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_negnz(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_NEGNZ;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_incmod(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_INCMOD;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_decmod(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_DECMOD;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_zerox(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_ZEROX;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_signx(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_SIGNX;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_encod(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_ENCOD;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_ones(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_ONES;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_test(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_TEST;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_testn(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_TESTN;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setnib(P2Params& p)
{
    if (left(p) < 3)
        return asm_setnib_altsn(p);
    p.idx++;
    p.IR.op.inst = p2_SETNIB_0;
    return parse_d_imm_s_nnn(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setnib_altsn(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_SETNIB_0;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_getnib(P2Params& p)
{
    if (left(p) < 3)
        return asm_getnib_altgn(p);
    p.idx++;
    p.IR.op.inst = p2_GETNIB_0;
    return parse_d_imm_s_nnn(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_getnib_altgn(P2Params& p)
{
    p.IR.op.inst = p2_GETNIB_0;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rolnib(P2Params& p)
{
    if (left(p) < 3)
        return asm_rolnib_altgn(p);
    p.idx++;
    p.IR.op.inst = p2_ROLNIB_0;
    return parse_d_imm_s_nnn(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rolnib_altgn(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_ROLNIB_0;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setbyte(P2Params& p)
{
    if (left(p) < 3)
        return asm_setbyte_altsb(p);
    p.idx++;
    p.IR.op.inst = p2_SETBYTE;
    return parse_d_imm_s_nnn(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setbyte_altsb(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_SETBYTE;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_getbyte(P2Params& p)
{
    if (left(p) < 3)
        return asm_getbyte_altgb(p);
    p.idx++;
    p.IR.op.inst = p2_GETBYTE;
    return parse_d_imm_s_nnn(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_getbyte_altgb(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_GETBYTE;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rolbyte(P2Params& p)
{
    if (left(p) < 3)
        return asm_rolbyte_altgb(p);
    p.idx++;
    p.IR.op.inst = p2_ROLBYTE;
    return parse_d_imm_s_nnn(p, 3);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rolbyte_altgb(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_ROLBYTE;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setword(P2Params& p)
{
    if (left(p) < 3)
        return asm_setword_altsw(p);
    p.idx++;
    p.IR.op9.inst = p2_SETWORD;
    return parse_d_imm_s_nnn(p, 1);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setword_altsw(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_SETWORD_ALTSW;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_getword(P2Params& p)
{
    if (left(p) < 3)
        return asm_getword_altgw(p);
    p.idx++;
    p.IR.op9.inst = p2_GETWORD;
    return parse_d_imm_s_nnn(p, 1);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_getword_altgw(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_GETWORD_ALTGW;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rolword(P2Params& p)
{
    if (left(p) < 3)
        return asm_rolword_altgw(p);
    p.idx++;
    p.IR.op9.inst = p2_ROLWORD;
    return parse_d_imm_s_nnn(p, 1);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rolword_altgw(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_ROLWORD_ALTGW;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_altsn(P2Params& p)
{
    if (left(p) < 3)
        return asm_altsn_d(p);
    p.idx++;
    p.IR.op9.inst = p2_ALTSN;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_altsn_d(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_ALTSN;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_altgn(P2Params& p)
{
    if (left(p) < 3)
        return asm_altgn_d(p);
    p.idx++;
    p.IR.op9.inst = p2_ALTGN;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_altgn_d(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_ALTGN;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_altsb(P2Params& p)
{
    if (left(p) < 3)
        return asm_altsb_d(p);
    p.idx++;
    p.IR.op9.inst = p2_ALTSB;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_altsb_d(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_ALTSB;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_altgb(P2Params& p)
{
    if (left(p) < 3)
        return asm_altgb_d(p);
    p.idx++;
    p.IR.op9.inst = p2_ALTGB;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_altgb_d(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_ALTGB;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_altsw(P2Params& p)
{
    if (left(p) < 3)
        return asm_altsw_d(p);
    p.idx++;
    p.IR.op9.inst = p2_ALTSW;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_altsw_d(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_ALTSW;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_altgw(P2Params& p)
{
    if (left(p) < 3)
        return asm_altgw_d(p);
    p.idx++;
    p.IR.op9.inst = p2_ALTGW;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_altgw_d(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_ALTGW;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_altr(P2Params& p)
{
    if (left(p) < 3)
        return asm_altr_d(p);
    p.idx++;
    p.IR.op9.inst = p2_ALTR;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_altr_d(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_ALTD;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_altd(P2Params& p)
{
    if (left(p) < 3)
        return asm_altd_d(p);
    p.idx++;
    p.IR.op9.inst = p2_ALTD;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_altd_d(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_ALTD;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_alts(P2Params& p)
{
    if (left(p) < 3)
        return asm_alts_d(p);
    p.idx++;
    p.IR.op9.inst = p2_ALTS;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_alts_d(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_ALTS;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_altb(P2Params& p)
{
    if (left(p) < 3)
        return asm_altb_d(p);
    p.idx++;
    p.IR.op9.inst = p2_ALTB;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_altb_d(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_ALTB;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_alti(P2Params& p)
{
    if (left(p) < 3)
        return asm_alti_d(p);
    p.idx++;
    p.IR.op9.inst = p2_ALTI;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_alti_d(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_ALTI;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setr(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_SETR;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setd(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_SETD;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_sets(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_SETS;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_decod(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_DECOD;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_decod_d(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_DECOD;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_bmask(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_BMASK;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_bmask_d(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_BMASK;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_crcbit(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_CRCBIT;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_crcnib(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_CRCNIB;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_muxnits(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_MUXNITS;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_muxnibs(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_MUXNIBS;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_muxq(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_MUXQ;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_movbyts(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_MOVBYTS;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_mul(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_MUL;
    return parse_d_imm_s_wz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_muls(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_MULS;
    return parse_d_imm_s_wz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_sca(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_SCA;
    return parse_d_imm_s_wz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_scas(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_SCAS;
    return parse_d_imm_s_wz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_addpix(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_ADDPIX;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_mulpix(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_MULPIX;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_blnpix(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_BLNPIX;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_mixpix(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_MIXPIX;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_addct1(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_ADDCT1;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_addct2(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_ADDCT2;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_addct3(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_ADDCT3;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_wmlong(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_WMLONG;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rqpin(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_RQPIN;
    return parse_d_imm_s_wc(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rdpin(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_RDPIN;
    return parse_d_imm_s_wc(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rdlut(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_RDLUT;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rdbyte(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_RDBYTE;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rdword(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_RDWORD;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rdlong(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_RDLONG;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_popa(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_RDLONG;
    p.IR.op.im = true;
    p.IR.op.src = 0x15f;
    return parse_d_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_popb(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_RDLONG;
    p.IR.op.im = true;
    p.IR.op.src = 0x1df;
    return parse_d_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_calld(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_CALLD;
    return parse_d_imm_s_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_resi3(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_CALLD;
    p.IR.op.wc = true;
    p.IR.op.wz = true;
    p.IR.op.dst = offs_IJMP3;
    p.IR.op.src = offs_IRET3;
    return parse_inst(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_resi2(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_CALLD;
    p.IR.op.wc = true;
    p.IR.op.wz = true;
    p.IR.op.dst = offs_IJMP2;
    p.IR.op.src = offs_IRET2;
    return parse_inst(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_resi1(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_CALLD;
    p.IR.op.wc = true;
    p.IR.op.wz = true;
    p.IR.op.dst = offs_IJMP1;
    p.IR.op.src = offs_IRET1;
    return parse_inst(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_resi0(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_CALLD;
    p.IR.op.wc = true;
    p.IR.op.wz = true;
    p.IR.op.dst = offs_INA;
    p.IR.op.src = offs_INB;
    return parse_inst(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_reti3(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_CALLD;
    p.IR.op.wc = true;
    p.IR.op.wz = true;
    p.IR.op.dst = offs_INB;
    p.IR.op.src = offs_IRET3;
    return parse_inst(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_reti2(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_CALLD;
    p.IR.op.wc = true;
    p.IR.op.wz = true;
    p.IR.op.dst = offs_INB;
    p.IR.op.src = offs_IRET2;
    return parse_inst(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_reti1(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_CALLD;
    p.IR.op.wc = true;
    p.IR.op.wz = true;
    p.IR.op.dst = offs_INB;
    p.IR.op.src = offs_IRET1;
    return parse_inst(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_reti0(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_CALLD;
    p.IR.op.wc = true;
    p.IR.op.wz = true;
    p.IR.op.dst = offs_INB;
    p.IR.op.src = offs_INB;
    return parse_inst(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_callpa(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_CALLPA;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_callpb(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_CALLPB;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_djz(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_DJZ;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_djnz(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_DJNZ;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_djf(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_DJF;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_djnf(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_DJNF;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_ijz(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_IJZ;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_ijnz(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_IJNZ;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_tjz(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_TJZ;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_tjnz(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_TJNZ;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_tjf(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_TJF;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_tjnf(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_TJNF;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_tjs(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_TJS;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_tjns(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_TJNS;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_tjv(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_TJV;
    return parse_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jint(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JINT;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jct1(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JCT1;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jct2(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JCT2;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jct3(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JCT3;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jse1(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JSE1;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jse2(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JSE2;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jse3(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JSE3;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jse4(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JSE4;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jpat(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JPAT;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jfbw(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JFBW;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jxmt(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JXMT;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jxfi(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JXFI;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jxro(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JXRO;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jxrl(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JXRL;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jatn(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JATN;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jqmt(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JQMT;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jnint(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JNINT;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jnct1(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JNCT1;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jnct2(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JNCT2;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jnct3(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JNCT3;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jnse1(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JNSE1;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jnse2(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JNSE2;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jnse3(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JNSE3;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jnse4(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JNSE4;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jnpat(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JNPAT;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jnfbw(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JNFBW;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jnxmt(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JNXMT;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jnxfi(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JNXFI;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jnxro(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JNXRO;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jnxrl(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JNXRL;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jnatn(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JNATN;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jnqmt(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_OPDST;
    p.IR.op.dst = p2_OPDST_JNQMT;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_1011110_1(P2Params& p)
{
    p.idx++;
    p.IR.op9.inst = p2_1011110_10;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_1011111_0(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_1011111_0;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setpat(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_SETPAT;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_wrpin(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_WRPIN;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_akpin(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_WRPIN;
    p.IR.op8.wz = true;
    p.IR.op.dst = 1;
    return parse_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_wxpin(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_WXPIN;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_wypin(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_WYPIN;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_wrlut(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_WRLUT;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_wrbyte(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_WRBYTE;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_wrword(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_WRWORD;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_wrlong(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_WRLONG;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_pusha(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_WRLONG;
    p.IR.op8.im = true;
    p.IR.op8.src = 0x161;
    return parse_wz_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_pushb(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_WRLONG;
    p.IR.op8.im = true;
    p.IR.op8.src = 0x1e1;
    return parse_wz_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rdfast(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_RDFAST;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_wrfast(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_WRFAST;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_fblock(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_FBLOCK;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_xinit(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_XINIT;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_xstop(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_XINIT;
    p.IR.op8.wz = true;
    p.IR.op8.im = true;
    p.IR.op8.dst = 0x000;
    p.IR.op8.src = 0x000;
    return parse_inst(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_xzero(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_XZERO;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_xcont(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_XCONT;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rep(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_REP;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_coginit(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_COGINIT;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_qmul(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_QMUL;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_qdiv(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_QDIV;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_qfrac(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_QFRAC;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_qsqrt(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_QSQRT;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_qrotate(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_QROTATE;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_qvector(P2Params& p)
{
    p.idx++;
    p.IR.op8.inst = p2_QVECTOR;
    return parse_wz_d_imm_s(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_hubset(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_HUBSET;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_cogid(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_COGID;
    return parse_imm_d_wc(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_cogstop(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_COGSTOP;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_locknew(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_LOCKNEW;
    return parse_imm_d_wc(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_lockret(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_LOCKRET;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_locktry(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_LOCKTRY;
    return parse_imm_d_wc(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_lockrel(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_LOCKREL;
    return parse_imm_d_wc(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_qlog(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_QLOG;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_qexp(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_QEXP;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rfbyte(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_RFBYTE;
    return parse_d_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rfword(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_RFWORD;
    return parse_d_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rflong(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_RFLONG;
    return parse_d_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rfvar(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_RFVAR;
    return parse_d_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rfvars(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_RFVARS;
    return parse_d_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_wfbyte(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_WFBYTE;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_wfword(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_WFWORD;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_wflong(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_WFLONG;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_getqx(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_GETQX;
    return parse_d_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_getqy(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_GETQY;
    return parse_d_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_getct(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_GETCT;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_getrnd(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_GETRND;
    return parse_d_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_getrnd_cz(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_GETRND;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setdacs(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SETDACS;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setxfrq(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SETXFRQ;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_getxacc(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_GETXACC;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_waitx(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_WAITX;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setse1(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SETSE1;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setse2(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SETSE2;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setse3(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SETSE3;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setse4(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SETSE4;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_pollint(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_POLLINT;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_pollct1(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_POLLCT1;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_pollct2(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_POLLCT2;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_pollct3(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_POLLCT3;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_pollse1(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_POLLSE1;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_pollse2(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_POLLSE2;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_pollse3(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_POLLSE3;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_pollse4(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_POLLSE4;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_pollpat(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_POLLPAT;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_pollfbw(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_POLLFBW;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_pollxmt(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_POLLXMT;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_pollxfi(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_POLLXFI;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_pollxro(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_POLLXRO;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_pollxrl(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_POLLXRL;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_pollatn(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_POLLATN;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_pollqmt(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_POLLQMT;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_waitint(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_WAITINT;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_waitct1(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_WAITCT1;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_waitct2(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_WAITCT2;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_waitct3(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_WAITCT3;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_waitse1(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_WAITSE1;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_waitse2(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_WAITSE2;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_waitse3(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_WAITSE3;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_waitse4(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_WAITSE4;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_waitpat(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_WAITPAT;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_waitfbw(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_WAITFBW;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_waitxmt(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_WAITXMT;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_waitxfi(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_WAITXFI;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_waitxro(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_WAITXRO;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_waitxrl(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_WAITXRL;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_waitatn(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_WAITATN;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_allowi(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_ALLOWI;
    return parse_inst(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_stalli(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_STALLI;
    return parse_inst(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_trgint1(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_TRGINT1;
    return parse_inst(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_trgint2(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_TRGINT2;
    return parse_inst(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_trgint3(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_TRGINT3;
    return parse_inst(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_nixint1(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_NIXINT1;
    return parse_inst(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_nixint2(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_NIXINT2;
    return parse_inst(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_nixint3(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_X24;
    p.IR.op.dst = p2_OPX24_NIXINT3;
    return parse_inst(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setint1(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SETINT1;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setint2(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SETINT2;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setint3(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SETINT3;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setq(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SETQ;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setq2(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SETQ2;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_push(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_PUSH;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_pop(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_POP;
    return parse_d_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jmp(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_JMP;
    return parse_d_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_call(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_CALL_RET;
    return parse_d_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_ret(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.im = true;
    p.IR.op.dst = 0x000;
    p.IR.op.src = p2_OPSRC_CALL_RET;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_calla(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_CALLA_RETA;
    return parse_d_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_reta(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.im = true;
    p.IR.op.dst = 0x000;
    p.IR.op.src = p2_OPSRC_CALLA_RETA;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_callb(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_CALLB_RETB;
    return parse_d_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_retb(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.im = true;
    p.IR.op.dst = 0x000;
    p.IR.op.src = p2_OPSRC_CALLB_RETB;
    return parse_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jmprel(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_JMPREL;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_skip(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SKIP;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_skipf(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SKIPF;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_execf(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_EXECF;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_getptr(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_GETPTR;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_getbrk(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.im = false;
    p.IR.op.src = p2_OPSRC_COGBRK;
    return parse_d_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_cogbrk(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_COGBRK;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_brk(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_BRK;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setluts(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SETLUTS;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setcy(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SETCY;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setci(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SETCI;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setcq(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SETCQ;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setcfrq(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SETCFRQ;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setcmod(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SETCMOD;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setpiv(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SETPIV;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setpix(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SETPIX;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_cogatn(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_COGATN;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_testp_w(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_TESTP_W_DIRL;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_testpn_w(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_TESTPN_W;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_testp_and(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_TESTP_AND;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_testpn_and(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_TESTPN_AND;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_testp_or(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_TESTP_OR;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_testpn_or(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_TESTPN_OR;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_testp_xor(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_TESTP_XOR;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_testpn_xor(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_TESTPN_XOR;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_dirl(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_DIRL;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_dirh(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_DIRH;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_dirc(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_DIRC;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_dirnc(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_DIRNC;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_dirz(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_DIRZ;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_dirnz(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_DIRNZ;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_dirrnd(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_DIRRND;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_dirnot(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_DIRNOT;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_outl(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_OUTL;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_outh(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_OUTH;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_outc(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_OUTC;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_outnc(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_OUTNC;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_outz(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_OUTZ;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_outnz(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_OUTNZ;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_outrnd(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_OUTRND;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_outnot(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_OUTNOT;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_fltl(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_FLTL;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_flth(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_FLTH;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_fltc(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_FLTC;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_fltnc(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_FLTNC;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_fltz(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_FLTZ;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_fltnz(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_FLTNZ;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_fltrnd(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_FLTRND;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_fltnot(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_FLTNOT;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_drvl(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_DRVL;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_drvh(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_DRVH;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_drvc(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_DRVC;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_drvnc(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_DRVNC;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_drvz(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_DRVZ;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_drvnz(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_DRVNZ;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_drvrnd(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_DRVRND;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_drvnot(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_DRVNOT;
    return parse_imm_d_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_splitb(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SPLITB;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_mergeb(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_MERGEB;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_splitw(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SPLITW;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_mergew(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_MERGEW;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_seussf(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SEUSSF;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_seussr(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SEUSSR;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rgbsqz(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_RGBSQZ;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rgbexp(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_RGBEXP;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_xoro32(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_XORO32;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rev(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_REV;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rczr(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_RCZR;
    return parse_d_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_rczl(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_RCZL;
    return parse_d_cz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_wrc(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_WRC;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_wrnc(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_WRNC;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_wrz(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_WRZ;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_wrnz(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_WRNZ_MODCZ;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_modcz(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_WRNZ_MODCZ;
    return parse_cccc_zzzz_wcz(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_setscp(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_SETSCP;
    return parse_imm_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_getscp(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_GETSCP;
    return parse_d(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_jmp_abs(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_JMP;
    return parse_pc_abs(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_call_abs(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_CALL_RET;
    return parse_pc_abs(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_calla_abs(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_CALLA_RETA;
    return parse_pc_abs(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_callb_abs(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_OPSRC;
    p.IR.op.src = p2_OPSRC_CALLB_RETB;
    return parse_pc_abs(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_calld_pa_abs(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_CALLD_PA_ABS;
    return parse_pc_abs(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_calld_pb_abs(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_CALLD_PB_ABS;
    return parse_pc_abs(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_calld_ptra_abs(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_CALLD_PTRA_ABS;
    return parse_pc_abs(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_calld_ptrb_abs(P2Params& p)
{
    p.idx++;
    p.IR.op.inst = p2_CALLD_PTRB_ABS;
    return parse_pc_abs(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_loc(P2Params& p)
{
    p.idx++;
    p2_token_e tok = p.tokens.value(p.idx);
    bool success = false;
    switch (tok) {
    case t_PA:
        p.idx++;
        parse_comma(p);
        success = asm_loc_pa(p);
        break;
    case t_PB:
        p.idx++;
        parse_comma(p);
        success = asm_loc_pb(p);
        break;
    case t_PTRA:
        p.idx++;
        parse_comma(p);
        success = asm_loc_ptra(p);
        break;
    case t_PTRB:
        p.idx++;
        parse_comma(p);
        success = asm_loc_ptrb(p);
        break;
    default:
        p.error = tr("Invalid pointer type '%1'; expected one of %2.")
                       .arg(p.words.value(p.idx))
                       .arg(tr("PA, PB, PTRA, or PTRB"));
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_loc_pa(P2Params& p)
{
    p.IR.op.inst = p2_LOC_PA;
    return parse_pc_abs(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_loc_pb(P2Params& p)
{
    p.IR.op.inst = p2_LOC_PB;
    return parse_pc_abs(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_loc_ptra(P2Params& p)
{
    p.IR.op.inst = p2_LOC_PTRA;
    return parse_pc_abs(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_loc_ptrb(P2Params& p)
{
    p.IR.op.inst = p2_LOC_PTRB;
    return parse_pc_abs(p);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_augs(P2Params& p)
{
    static const QVector<p2_inst7_e> augs = QVector<p2_inst7_e>()
                                           << p2_AUGS_00
                                           << p2_AUGS_01
                                           << p2_AUGS_10
                                           << p2_AUGS_11;
    p.idx++;
    p.IR.op.inst = p2_AUGS_00;
    return parse_imm23(p, augs);
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
 * @param p reference to the assembler parameters
 */
bool P2Asm::asm_augd(P2Params& p)
{
    static const QVector<p2_inst7_e> augd = QVector<p2_inst7_e>()
                                           << p2_AUGD_00
                                           << p2_AUGD_01
                                           << p2_AUGD_10
                                           << p2_AUGD_11;
    p.idx++;
    p.IR.op.inst = p2_AUGD_00;
    return parse_imm23(p, augd);
}
