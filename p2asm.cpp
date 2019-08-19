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

        if (line.startsWith(QStringLiteral("{{")))
            multi_comment = true;

        if (multi_comment) {
            if (line.endsWith("}}"))
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
        QString symbol;
        P2AsmSymbol sym;
        if (params.idx < params.cnt && params.tokens.at(params.idx) == t_nothing) {
            symbol = params.words.at(params.idx);
            sym = params.symbols.value(symbol);
            if (sym.isEmpty()) {
                params.symbols.insert(symbol, params.curr_pc, &sym);
            } else {
                emit Error(params.lineno, tr("Symbol already defined: %1").arg(symbol));
            }
            // skip over first word
            params.idx++;
        }

        // Calculate next PC for regular instructions
        if (params.curr_pc < 0x400) {
            params.next_pc = params.curr_pc + 1;
        } else {
            params.next_pc = params.curr_pc + 4;
        }
        if (params.next_pc > params.last_pc)
            params.last_pc = params.next_pc;

        qDebug("%s: line=%-5d pc=$%06x cnt=%-3d : %s", __func__,
               params.lineno, params.curr_pc, params.cnt, qPrintable(line));

        // Skip if no more tokens were found
        if (params.idx >= params.tokens.count())
            continue;

        // Reset all instruction bits
        params.IR.word = 0;

        // Conditional execution prefix
        const p2_token_e cond = params.tokens.at(params.idx);
        const p2_cond_e eeee = conditional(params, cond);
        params.IR.op.cond = eeee;

        // Expect a token for an instruction
        const p2_token_e inst = params.tokens.at(params.idx);
        switch (inst) {
        case t_ABS:
            params.IR.op.inst = p2_ABS;
            params_d_imm_s_cz(params);
            break;

        case t_ADD:
            params.IR.op.inst = p2_ADD;
            params_d_imm_s_cz(params);
            break;

        case t_ADDCT1:
            params.IR.op9.inst = p2_ADDCT1;
            params_d_imm_s(params);
            break;

        case t_ADDCT2:
            params.IR.op9.inst = p2_ADDCT2;
            params_d_imm_s(params);
            break;

        case t_ADDCT3:
            params.IR.op9.inst = p2_ADDCT3;
            params_d_imm_s(params);
            break;

        case t_ADDPIX:
            params.IR.op9.inst = p2_ADDPIX;
            params_d_imm_s(params);
            break;

        case t_ADDS:
            params.IR.op.inst = p2_ADDS;
            params_d_imm_s_cz(params);
            break;

        case t_ADDSX:
            params.IR.op.inst = p2_ADDSX;
            params_d_imm_s_cz(params);
            break;

        case t_ADDX:
            params.IR.op.inst = p2_ADDX;
            params_d_imm_s_cz(params);
            break;

        case t_AKPIN:
            params.IR.op9.inst = p2_WRPIN;
            params_d_imm_s_cz(params);
            params.IR.op.wz = 1;
            params.IR.op.dst = 1;
            break;

        case t_ALLOWI:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_X24;
            params.IR.op.dst = p2_OPX24_ALLOWI;
            params_inst(params);
            break;

        case t_ALTB:
            params.IR.op9.inst = p2_ALTB;
            params_d_imm_s(params);
            break;

        case t_ALTD:
            params.IR.op9.inst = p2_ALTD;
            params_d_imm_s(params);
            break;

        case t_ALTGB:
            params.IR.op9.inst = p2_ALTGB;
            params_d_imm_s(params);
            break;

        case t_ALTGN:
            params.IR.op9.inst = p2_ALTGN;
            params_d_imm_s(params);
            break;

        case t_ALTGW:
            params.IR.op9.inst = p2_ALTGW;
            params_d_imm_s(params);
            break;

        case t_ALTI:
            params.IR.op9.inst = p2_ALTI;
            params_d_imm_s(params);
            break;

        case t_ALTR:
            params.IR.op9.inst = p2_ALTR;
            params_d_imm_s(params);
            break;

        case t_ALTS:
            params.IR.op9.inst = p2_ALTS;
            params_d_imm_s(params);
            break;

        case t_ALTSB:
            params.IR.op9.inst = p2_ALTSB;
            params_d_imm_s(params);
            break;

        case t_ALTSN:
            params.IR.op9.inst = p2_ALTSN;
            params_d_imm_s(params);
            break;

        case t_ALTSW:
            params.IR.op9.inst = p2_ALTSW;
            params_d_imm_s(params);
            break;

        case t_AND:
            params.IR.op.inst = p2_AND;
            params_d_imm_s_cz(params);
            break;

        case t_ANDN:
            params.IR.op.inst = p2_ANDN;
            params_d_imm_s_cz(params);
            break;

        case t_AUGD:
            params.IR.op.inst = p2_AUGD_00;
            params_imm23(params);
            break;

        case t_AUGS:
            params.IR.op.inst = p2_AUGS_00;
            params_imm23(params);
            break;

        case t_BITC:
            params.IR.op9.inst = p2_BITC;
            params_d_imm_s_cz(params);
            break;

        case t_BITH:
            params.IR.op9.inst = p2_BITH;
            params_d_imm_s_cz(params);
            break;

        case t_BITL:
            params.IR.op9.inst = p2_BITL;
            params_d_imm_s_cz(params);
            break;

        case t_BITNC:
            params.IR.op9.inst = p2_BITNC;
            params_d_imm_s_cz(params);
            break;

        case t_BITNOT:
            params.IR.op9.inst = p2_BITNOT;
            params_d_imm_s_cz(params);
            break;

        case t_BITNZ:
            params.IR.op9.inst = p2_BITNZ;
            params_d_imm_s_cz(params);
            break;

        case t_BITRND:
            params.IR.op9.inst = p2_BITRND;
            params_d_imm_s_cz(params);
            break;

        case t_BITZ:
            params.IR.op9.inst = p2_BITZ;
            params_d_imm_s_cz(params);
            break;

        case t_BLNPIX:
            params.IR.op9.inst = p2_BLNPIX;
            params_d_imm_s(params);
            break;

        case t_BMASK:
            params.IR.op9.inst = p2_BMASK;
            params_d_imm_s(params);
            break;

        case t_BRK:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_BRK;
            params_imm_d(params);
            break;

        case t_CALL:
            params.IR.op.inst = p2_CALL_ABS;
            params_pc_abs(params);
            break;

        case t_CALLA:
            params.IR.op.inst = p2_CALLA_ABS;
            params_pc_abs(params);
            break;

        case t_CALLB:
            params.IR.op.inst = p2_CALLB_ABS;
            params_pc_abs(params);
            break;

        case t_CALLD:
            params.IR.op.inst = p2_CALLD;
            params_ptr_pc_abs(params);
            break;

        case t_CALLPA:
            break;
        case t_CALLPB:
            break;

        case t_CMP:
            params.IR.op.inst = p2_CMP;
            params_d_imm_s_cz(params);
            break;

        case t_CMPM:
            params.IR.op.inst = p2_CMPM;
            params_d_imm_s_cz(params);
            break;

        case t_CMPR:
            params.IR.op.inst = p2_CMPR;
            params_d_imm_s_cz(params);
            break;

        case t_CMPS:
            params.IR.op.inst = p2_CMPS;
            params_d_imm_s_cz(params);
            break;

        case t_CMPSUB:
            params.IR.op.inst = p2_CMPSUB;
            params_d_imm_s_cz(params);
            break;

        case t_CMPSX:
            params.IR.op.inst = p2_CMPSX;
            params_d_imm_s_cz(params);
            break;

        case t_CMPX:
            params.IR.op.inst = p2_CMPX;
            params_d_imm_s_cz(params);
            break;

        case t_COGATN:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_COGATN;
            params_imm_d(params);
            break;

        case t_COGBRK:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_COGBRK;
            params_imm_d(params);
            break;

        case t_COGID:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_COGID;
            params_imm_d_c(params);
            break;

        case t_COGINIT:
            params.IR.op.inst = p2_COGINIT;
            params_wz_d_imm_s(params);
            break;

        case t_COGSTOP:
            break;

        case t_CRCBIT:
            break;

        case t_CRCNIB:
            break;

        case t_DAT:
            break;

        case t_DECMOD:
            break;

        case t_DECOD:
            break;

        case t_DIRC:
            break;

        case t_DIRH:
            break;

        case t_DIRL:
            break;

        case t_DIRNC:
            break;

        case t_DIRNOT:
            break;

        case t_DIRNZ:
            break;

        case t_DIRRND:
            break;

        case t_DIRZ:
            break;

        case t_DJF:
            break;

        case t_DJNF:
            break;

        case t_DJNZ:
            break;

        case t_DJZ:
            break;

        case t_DRVC:
            break;

        case t_DRVH:
            break;

        case t_DRVL:
            break;

        case t_DRVNC:
            break;

        case t_DRVNOT:
            break;

        case t_DRVNZ:
            break;

        case t_DRVRND:
            break;

        case t_DRVZ:
            break;

        case t_ENCOD:
            break;

        case t_EXECF:
            break;

        case t_FBLOCK:
            break;

        case t_FGE:
            break;

        case t_FGES:
            break;

        case t_FIT:
            break;

        case t_FLE:
            break;

        case t_FLES:
            break;

        case t_FLTC:
            break;

        case t_FLTH:
            break;

        case t_FLTL:
            break;

        case t_FLTNC:
            break;

        case t_FLTNOT:
            break;

        case t_FLTNZ:
            break;

        case t_FLTRND:
            break;

        case t_FLTZ:
            break;

        case t_GETBRK:
            break;

        case t_GETBYTE:
            break;

        case t_GETCT:
            break;

        case t_GETNIB:
            break;

        case t_GETPTR:
            break;

        case t_GETQX:
            break;

        case t_GETQY:
            break;

        case t_GETRND:
            break;

        case t_GETSCP:
            break;

        case t_GETWORD:
            break;

        case t_GETXACC:
            break;

        case t_HUBSET:
            break;

        case t_IJNZ:
            break;

        case t_IJZ:
            break;

        case t_INCMOD:
            params.IR.op.inst = p2_INCMOD;
            params_d_imm_s_cz(params);
            break;

        case t_JATN:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JATN;
            params_with_cz(params);
            break;

        case t_JCT1:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JCT1;
            params_with_cz(params);
            break;

        case t_JCT2:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JCT2;
            params_with_cz(params);
            break;

        case t_JCT3:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JCT3;
            params_with_cz(params);
            break;

        case t_JFBW:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JFBW;
            params_with_cz(params);
            break;

        case t_JINT:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JINT;
            params_with_cz(params);
            break;

        case t_JMP:
            params.IR.op9.inst = p2_JMP_ABS;
            params_pc_abs(params);
            break;

        case t_JMPREL:
            break;

        case t_JNATN:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JNATN;
            params_with_cz(params);
            break;

        case t_JNCT1:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JNCT1;
            params_with_cz(params);
            break;

        case t_JNCT2:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JNCT2;
            params_with_cz(params);
            break;

        case t_JNCT3:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JNCT3;
            params_with_cz(params);
            break;

        case t_JNFBW:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JNFBW;
            params_with_cz(params);
            break;

        case t_JNINT:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JNINT;
            params_with_cz(params);
            break;

        case t_JNPAT:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JNPAT;
            params_with_cz(params);
            break;

        case t_JNQMT:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JNQMT;
            params_with_cz(params);
            break;

        case t_JNSE1:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JNSE1;
            params_with_cz(params);
            break;

        case t_JNSE2:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JNSE2;
            params_with_cz(params);
            break;

        case t_JNSE3:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JNSE3;
            params_with_cz(params);
            break;

        case t_JNSE4:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JNSE4;
            params_with_cz(params);
            break;

        case t_JNXFI:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JNXFI;
            params_with_cz(params);
            break;

        case t_JNXMT:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JNXMT;
            params_with_cz(params);
            break;

        case t_JNXRL:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JNXRL;
            params_with_cz(params);
            break;

        case t_JNXRO:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JNXRO;
            params_with_cz(params);
            break;

        case t_JPAT:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JPAT;
            params_with_cz(params);
            break;

        case t_JQMT:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JQMT;
            params_with_cz(params);
            break;

        case t_JSE1:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JSE1;
            params_with_cz(params);
            break;

        case t_JSE2:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JSE2;
            params_with_cz(params);
            break;

        case t_JSE3:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JSE3;
            params_with_cz(params);
            break;

        case t_JSE4:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JSE4;
            params_with_cz(params);
            break;

        case t_JXFI:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JXFI;
            params_with_cz(params);
            break;

        case t_JXMT:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JXMT;
            params_with_cz(params);
            break;

        case t_JXRL:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JXRL;
            params_with_cz(params);
            break;

        case t_JXRO:
            params.IR.op9.inst = p2_TJV_OPDST;
            params.IR.op9.dst = p2_OPDST_JXRO;
            params_with_cz(params);
            break;

        case t_LOC:
            params.IR.op.inst = p2_LOC_PA;
            params_ptr_pc_abs(params);
            break;

        case t_LOCKNEW:
            break;
        case t_LOCKREL:
            break;
        case t_LOCKRET:
            break;
        case t_LOCKTRY:
            break;

        case t_MERGEB:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_MERGEB;
            params_d(params);
            break;

        case t_MERGEW:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_MERGEW;
            params_d(params);
            break;


        case t_MIXPIX:
            params.IR.op9.inst = p2_MIXPIX;
            params_d_imm_s(params);
            break;

        case t_MODCZ:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_WRNZ_MODCZ;
            params_cccc_zzzz_cz(params);
            break;

        case t_MOV:
            params.IR.op.inst = p2_MOV;
            params_d_imm_s_cz(params);
            break;

        case t_MOVBYTS:
        case t_MUL:
            break;

        case t_MULPIX:
            params.IR.op9.inst = p2_MULPIX;
            params_d_imm_s(params);
            break;

        case t_MULS:
            break;

        case t_MUXC:
            break;

        case t_MUXNC:
            break;

        case t_MUXNIBS:
            break;

        case t_MUXNITS:
            break;

        case t_MUXNZ:
            break;

        case t_MUXQ:
            break;

        case t_MUXZ:
            break;

        case t_NEG:
            break;

        case t_NEGC:
            break;

        case t_NEGNC:
            break;

        case t_NEGNZ:
            break;

        case t_NEGZ:
            break;

        case t_ALWAYS:
            break;

        case t_NIXINT1:
            break;

        case t_NIXINT2:
            break;

        case t_NIXINT3:
            break;

        case t_NOP:
            params_inst(params);
            break;

        case t_NOT:
            params.IR.op.inst = p2_NOT;
            params_d_imm_s_cz(params);
            break;

        case t_ONES:

        case t_OR:
            params.IR.op.inst = p2_XOR;
            params_d_imm_s_cz(params);
            break;

        case t_OUTC:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_OUTC;
            params_imm_d(params);
            break;

        case t_OUTH:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_OUTH;
            params_imm_d(params);
            break;

        case t_OUTL:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_OUTL;
            params_imm_d(params);
            break;

        case t_OUTNC:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_OUTNC;
            params_imm_d(params);
            break;

        case t_OUTNOT:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_OUTNOT;
            params_imm_d(params);
            break;

        case t_OUTNZ:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_OUTNZ;
            params_imm_d(params);
            break;

        case t_OUTRND:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_OUTRND;
            params_imm_d(params);
            break;

        case t_OUTZ:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_OUTZ;
            params_imm_d(params);
            break;

        case t_POLLATN:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_X24;
            params.IR.op.dst = p2_OPX24_POLLATN;
            params_with_cz(params);
            break;

        case t_POLLCT1:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_X24;
            params.IR.op.dst = p2_OPX24_POLLCT1;
            params_with_cz(params);
            break;

        case t_POLLCT2:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_X24;
            params.IR.op.dst = p2_OPX24_POLLCT2;
            params_with_cz(params);
            break;

        case t_POLLCT3:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_X24;
            params.IR.op.dst = p2_OPX24_POLLCT3;
            params_with_cz(params);
            break;

        case t_POLLFBW:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_X24;
            params.IR.op.dst = p2_OPX24_POLLFBW;
            params_with_cz(params);
            break;

        case t_POLLINT:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_X24;
            params.IR.op.dst = p2_OPX24_POLLINT;
            params_with_cz(params);
            break;

        case t_POLLPAT:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_X24;
            params.IR.op.dst = p2_OPX24_POLLPAT;
            params_with_cz(params);
            break;

        case t_POLLQMT:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_X24;
            params.IR.op.dst = p2_OPX24_POLLQMT;
            params_with_cz(params);
            break;

        case t_POLLSE1:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_X24;
            params.IR.op.dst = p2_OPX24_POLLSE1;
            params_with_cz(params);
            break;

        case t_POLLSE2:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_X24;
            params.IR.op.dst = p2_OPX24_POLLSE2;
            params_with_cz(params);
            break;

        case t_POLLSE3:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_X24;
            params.IR.op.dst = p2_OPX24_POLLSE3;
            params_with_cz(params);
            break;

        case t_POLLSE4:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_X24;
            params.IR.op.dst = p2_OPX24_POLLSE4;
            params_with_cz(params);
            break;

        case t_POLLXFI:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_X24;
            params.IR.op.dst = p2_OPX24_POLLXFI;
            params_with_cz(params);
            break;

        case t_POLLXMT:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_X24;
            params.IR.op.dst = p2_OPX24_POLLXMT;
            params_with_cz(params);
            break;

        case t_POLLXRL:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_X24;
            params.IR.op.dst = p2_OPX24_POLLXRL;
            params_with_cz(params);
            break;

        case t_POLLXRO:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_X24;
            params.IR.op.dst = p2_OPX24_POLLXRO;
            params_with_cz(params);
            break;

        case t_POP:
            break;

        case t_POPA:
            break;

        case t_POPB:
            break;

        case t_PUSH:
            break;

        case t_PUSHA:
            break;

        case t_PUSHB:
            break;

        case t_QDIV:
            break;

        case t_QEXP:
            break;

        case t_QFRAC:
            break;

        case t_QLOG:
            break;

        case t_QMUL:
            break;

        case t_QROTATE:
            break;

        case t_QSQRT:
            break;

        case t_QVECTOR:
            break;

        case t_RCL:
            params.IR.op.inst = p2_RCL;
            params_d_imm_s_cz(params);
            break;

        case t_RCR:
            params.IR.op.inst = p2_RCR;
            params_d_imm_s_cz(params);
            break;

        case t_RCZL:
            break;

        case t_RCZR:
            break;

        case t_RDBYTE:
            break;

        case t_RDFAST:
            break;

        case t_RDLONG:
            break;

        case t_RDLUT:
            break;

        case t_RDPIN:
            break;

        case t_RDWORD:
            break;

        case t_REP:
            break;

        case t_RESI0:
            break;

        case t_RESI1:
            break;

        case t_RESI2:
            break;

        case t_RESI3:
            break;

        case t_RET:
            break;

        case t_RETA:
            break;

        case t_RETB:
            break;

        case t_RETI0:
            break;

        case t_RETI1:
            break;

        case t_RETI2:
            break;

        case t_RETI3:
            break;

        case t_REV:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_REV;
            params_d(params);
            break;


        case t_RFBYTE:
            break;

        case t_RFLONG:
            break;

        case t_RFVAR:
            break;

        case t_RFVARS:
            break;

        case t_RFWORD:
            break;

        case t_RGBEXP:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_RGBEXP;
            params_imm_d(params);
            break;

        case t_RGBSQZ:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_RGBSQZ;
            params_imm_d(params);
            break;

        case t_ROL:
            params.IR.op.inst = p2_ROL;
            params_d_imm_s_cz(params);
            break;

        case t_ROLBYTE:
            break;

        case t_ROLNIB:
            break;

        case t_ROLWORD:
            break;

        case t_ROR:
            params.IR.op.inst = p2_ROR;
            params_d_imm_s_cz(params);
            break;

        case t_RQPIN:
            break;

        case t_SAL:
            break;

        case t_SAR:
            break;

        case t_SCA:
            break;

        case t_SCAS:
            break;

        case t_SETBYTE:
            break;

        case t_SETCFRQ:
            break;

        case t_SETCI:
            break;

        case t_SETCMOD:
            break;

        case t_SETCQ:
            break;

        case t_SETCY:
            break;

        case t_SETD:
            break;

        case t_SETDACS:
            break;

        case t_SETINT1:
            break;

        case t_SETINT2:
            break;

        case t_SETINT3:
            break;

        case t_SETLUTS:
            break;

        case t_SETNIB:
            break;

        case t_SETPAT:
            break;

        case t_SETPIV:
            break;

        case t_SETPIX:
            break;

        case t_SETQ:
            break;

        case t_SETQ2:
            break;

        case t_SETR:
            break;

        case t_SETS:
            break;

        case t_SETSCP:
            break;

        case t_SETSE1:
            break;

        case t_SETSE2:
            break;

        case t_SETSE3:
            break;

        case t_SETSE4:
            break;

        case t_SETWORD:
            break;

        case t_SETXFRQ:
            break;

        case t_SEUSSF:
            break;

        case t_SEUSSR:
            break;

        case t_SHL:
            break;

        case t_SHR:
            break;

        case t_SIGNX:
            break;

        case t_SKIP:
            break;

        case t_SKIPF:
            break;

        case t_SPACE:
            break;

        case t_SPLITB:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_SPLITB;
            params_d(params);
            break;

        case t_SPLITW:
            params.IR.op.inst = p2_OPSRC;
            params.IR.op.src = p2_OPSRC_SPLITW;
            params_d(params);
            break;

        case t_STALLI:
            break;

        case t_SUB:
            break;

        case t_SUBR:
            break;

        case t_SUBS:
            break;

        case t_SUBSX:
            break;

        case t_SUBX:
            break;

        case t_SUMC:
            break;

        case t_SUMNC:
            break;

        case t_SUMNZ:
            break;

        case t_SUMZ:
            break;

        case t_TEST:
            break;

        case t_TESTB:
            break;

        case t_TESTBN:
            break;

        case t_TESTN:
            break;

        case t_TESTNB:
            break;

        case t_TESTP:
            break;

        case t_TESTPN:
            break;

        case t_TJF:
            break;

        case t_TJNF:
            break;

        case t_TJNS:
            break;

        case t_TJNZ:
            break;

        case t_TJS:
            break;

        case t_TJV:
            break;

        case t_TJZ:
            break;

        case t_TRGINT1:
            break;

        case t_TRGINT2:
            break;

        case t_TRGINT3:
            break;

        case t_WAITATN:
            break;

        case t_WAITCT1:
            break;

        case t_WAITCT2:
            break;

        case t_WAITCT3:
            break;

        case t_WAITFBW:
            break;

        case t_WAITINT:
            break;

        case t_WAITPAT:
            break;

        case t_WAITSE1:
            break;

        case t_WAITSE2:
            break;

        case t_WAITSE3:
            break;

        case t_WAITSE4:
            break;

        case t_WAITX:
            break;

        case t_WAITXFI:
            break;

        case t_WAITXMT:
            break;

        case t_WAITXRL:
            break;

        case t_WAITXRO:
            break;

        case t_WFBYTE:
            break;

        case t_WFLONG:
            break;

        case t_WFWORD:
            break;

        case t_WMLONG:
            break;

        case t_WRBYTE:
            break;

        case t_WRC:
            break;

        case t_WRFAST:
            break;

        case t_WRLONG:
            break;

        case t_WRLUT:
            break;

        case t_WRNC:
            break;

        case t_WRNZ:
            break;

        case t_WRPIN:
            break;

        case t_WRWORD:
            break;

        case t_WRZ:
            break;

        case t_WXPIN:
            break;

        case t_WYPIN:
            break;

        case t_XCONT:
            break;

        case t_XINIT:
            break;

        case t_XOR:
            params.IR.op.inst = p2_XOR;
            params_d_imm_s_cz(params);
            break;

        case t_XORO32:
            break;

        case t_XSTOP:
            break;

        case t_XZERO:
            break;

        case t_ZEROX:
            break;

        case t__BYTE:
            params_byte(params);
            break;

        case t__WORD:
            params_word(params);
            break;

        case t__LONG:
            params_long(params);
            break;

        case t__RES:
            params_res(params);
            break;

        case t__ORG:
            params_org(params, sym);
            break;

        case t__ORGH:
            params_orgh(params, sym);
            break;

        case t__ASSIGN:
            params_assign(params, sym);
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
            emit Error(params.lineno, tr("Not an instruction: %1").arg(line));
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
            emit Error(params.lineno, tr("Multiple conditionals in line: %1").arg(line));
            break;

        default:
            // Handle non-instruction tokens
            break;
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
    QByteArray binary;
    QByteArray source = file.readAll();
    QStringList lines = QString::fromUtf8(source).split(QChar::LineFeed);
    return assemble(params, lines);
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
        if (str.at(pos).isSpace())
            continue;
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
                    value = from_bin(pos, str, stop);
                    break;
                case '0':   // Octal constant
                    value = from_oct(pos, str, stop);
                    break;
                case '1': case '2': case '3':
                case '4': case '5': case '6':
                case '7': case '8': case '9':   // decimal constant
                    value = from_dec(pos, str, stop);
                    break;
                case '"':   // string constant
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
bool P2Asm::end_of_line(P2Params& params)
{
    if (params.idx < params.cnt) {
        // ignore extra parameters?
        params.error = tr("Ignored extra parameters: %1")
                       .arg(params.words.mid(params.idx).join(QChar::Space));
        return false;
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
    return end_of_line(params);
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
    return end_of_line(params);
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
    return end_of_line(params);
}

/**
 * @brief Assignment operation
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::params_assign(P2Params& params, P2AsmSymbol& sym)
{
    params.idx++;
    QVariant value = expression(params);
    sym.setValue(value);
    return true;
}

/**
 * @brief Origin operation
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::params_org(P2Params& params, P2AsmSymbol& sym)
{
    params.idx++;
    QVariant value = expression(params);
    if (value.isNull())
        value = params.last_pc;
    params.next_pc = value.toUInt();
    if (!sym.isEmpty())
        sym.setValue(value);
    return true;
}

/**
 * @brief Origin high operation
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::params_orgh(P2Params& params, P2AsmSymbol& sym)
{
    params.idx++;
    QVariant value = expression(params);
    if (value.isNull())
        value = 0x00400;
    params.next_pc = value.toUInt();
    if (!sym.isEmpty())
        sym.setValue(value);
    return true;
}

/**
 * @brief Expect instruction with optional WC, WZ, or WCZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::params_with_cz(P2Params& params)
{
    params.idx++;
    return optional_wcz(params);
}

/**
 * @brief Expect instruction with optional WC
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::params_with_c(P2Params& params)
{
    params.idx++;
    return optional_wc(params);
}


/**
 * @brief Expect instruction with optional WZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::params_with_z(P2Params& params)
{
    params.idx++;
    return optional_wz(params);
}

/**
 * @brief Expect no more parameters after instruction
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::params_inst(P2Params& params)
{
    params.idx++;
    return end_of_line(params);
}


/**
 * @brief Expect parameters for D and {#}S
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::params_d_imm_s(P2Params& params)
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
bool P2Asm::params_d_cz(P2Params& params)
{
    params.idx++;
    QVariant dst = expression(params);
    params.IR.op.dst = dst.toUInt();
    return optional_wcz(params);
}

/**
 * @brief Expect optional WC, WZ, or WCZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::params_cz(P2Params& params)
{
    params.idx++;
    return optional_wcz(params);
}

/**
 * @brief Expect conditional for C, conditional for Z, and optional WC, WZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::params_cccc_zzzz_cz(P2Params& params)
{
    params.idx++;
    p2_cond_e cccc = conditional(params, params.tokens.value(params.idx));
    p2_cond_e zzzz = conditional(params, params.tokens.value(params.idx));
    params.IR.op.dst = static_cast<P2LONG>((cccc << 4) | zzzz);
    return optional_wcz(params);
}

/**
 * @brief Expect parameters for D
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::params_d(P2Params& params)
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
bool P2Asm::params_wz_d(P2Params& params)
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
bool P2Asm::params_imm_d(P2Params& params)
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
bool P2Asm::params_imm_d_cz(P2Params& params)
{
    params.idx++;
    QVariant dst = expression(params, immediate_imm);
    params.IR.op.dst = dst.toUInt();
    return optional_wcz(params);
}

/**
 * @brief Expect parameters for {#}D, and optional WC
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::params_imm_d_c(P2Params& params)
{
    params.idx++;
    QVariant dst = expression(params, immediate_imm);
    params.IR.op.dst = dst.toUInt();
    return optional_wc(params);
}

/**
 * @brief Expect parameters for D, and {#}S, and optional WC, WZ, or WCZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::params_d_imm_s_cz(P2Params& params)
{
    params.idx++;
    QVariant dst = expression(params);
    QVariant src = expression(params, immediate_imm);
    params.IR.op.dst = dst.toUInt();
    params.IR.op.src = dst.toUInt();
    return optional_wcz(params);
}

/**
 * @brief Expect parameters for D, and {#}S, and optional WC
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::params_d_imm_s_c(P2Params& params)
{
    params.idx++;
    QVariant dst = expression(params);
    QVariant src = expression(params, immediate_imm);
    params.IR.op.dst = dst.toUInt();
    params.IR.op.src = dst.toUInt();
    return optional_wc(params);
}

/**
 * @brief Expect parameters for D, and {#}S, and optional WZ
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::params_d_imm_s_z(P2Params& params)
{
    params.idx++;
    QVariant dst = expression(params);
    QVariant src = expression(params, immediate_imm);
    params.IR.op.dst = dst.toUInt();
    params.IR.op.src = dst.toUInt();
    return optional_wz(params);
}

/**
 * @brief Expect parameters for {#}D, and {#}S
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::params_wz_d_imm_s(P2Params& params)
{
    params.idx++;
    QVariant dst = expression(params, immediate_wz);
    QVariant src = expression(params, immediate_imm);
    params.IR.op.dst = dst.toUInt();
    params.IR.op.src = dst.toUInt();
    return end_of_line(params);
}

/**
 * @brief Expect parameters for D, and {#}S, and #N (0 … 7)
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::params_d_imm_s_nnn(P2Params& params)
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
bool P2Asm::params_d_imm_s_n(P2Params& params)
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
bool P2Asm::params_imm_s(P2Params& params)
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
bool P2Asm::params_ptr_pc_abs(P2Params& params)
{
    params.idx++;
    p2_token_e dst = params.tokens.value(params.idx);
    switch (dst) {
    case t_PA:
        params.IR.op.wz = false;
        params.IR.op.wc = false;
        break;
    case t_PB:
        params.IR.op.wz = true;
        params.IR.op.wc = false;
        break;
    case t_PTRA:
        params.IR.op.wz = false;
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
    params.IR.word |= addr & A20MASK;

    return end_of_line(params);
}

/**
 * @brief Expect parameters for #AAAAAAAAAAAA (19 bit address for CALL/CALLA/CALLB)
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::params_pc_abs(P2Params& params)
{
    params.idx++;
    quint64 addr = expression(params).toULongLong();
    params.IR.word |= addr & A20MASK;

    return end_of_line(params);
}

/**
 * @brief Expect parameters for #AAAAAAAAAAAA (23 bit address for AUGD/AUGS)
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::params_imm23(P2Params& params)
{
    params.idx++;
    quint64 addr = expression(params).toULongLong();
    params.IR.op.inst |= (addr >> 21) & 3;

    return end_of_line(params);
}

/**
 * @brief Expect one or more bytes of data
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::params_byte(P2Params& params)
{
    params.idx++;
    while (params.idx < params.cnt) {
        QVariant data = expression(params);
        qDebug("%s: byte %#x", __func__, data.toUInt());
    }
    return end_of_line(params);
}

/**
 * @brief Expect one or more bytes of data
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::params_word(P2Params& params)
{
    params.idx++;
    while (params.idx < params.cnt) {
        QVariant data = expression(params);
        qDebug("%s: word %#x", __func__, data.toUInt());
    }
    return end_of_line(params);
}

/**
 * @brief Expect one or more bytes of data
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::params_long(P2Params& params)
{
    params.idx++;
    while (params.idx < params.cnt) {
        QVariant data = expression(params);
        qDebug("%s: long %#x", __func__, data.toUInt());
    }
    return end_of_line(params);
}

/**
 * @brief Expect one or more bytes of data
 * @brief params reference to the assembler parameters
 * @return true on success, or false on error
 */
bool P2Asm::params_res(P2Params& params)
{
    while (params.idx < params.cnt) {
        QVariant data = expression(params);
    }
    return end_of_line(params);
}
