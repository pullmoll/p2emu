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
#include <QLinkedList>
#include <QRegExp>
#include "p2asm.h"

static const QString p2_section_dat = QStringLiteral("DAT");
static const QString p2_section_con = QStringLiteral("CON");
static const QString p2_section_pub = QStringLiteral("PUB");
static const QString p2_section_pri = QStringLiteral("PRI");
static const QString p2_section_var = QStringLiteral("VAR");

#if 0
//! debug expression parsing
#define DEBUG_EXPR(x,...) qDebug(x,__VA_ARGS__)
#else
#define DEBUG_EXPR(x,...)
#endif

/**
 * @brief P2Asm constructor
 * @param parent pointer to parent QObject
 */
P2Asm::P2Asm(QObject *parent)
    : QObject(parent)
    , m_pass(0)
    , m_source()
    , m_listing()
    , m_hash_PC()
    , m_hash_IR()
    , m_hash_words()
    , m_hash_error()
    , m_symbols()
    , m_lineno(0)
    , m_in_curly(0)
    , m_line()
    , m_error()
    , m_next_PC(0)
    , m_curr_PC(0)
    , m_last_PC(0)
    , m_advance(4)
    , m_emit_IR(false)
    , m_IR()
    , m_words()
    , m_instr()
    , m_symbol()
    , m_function()
    , m_section()
    , m_wcnt(0)
    , m_widx(0)
    , MEM()
{
    m_sections.insert(sec_dat, p2_section_dat);
    m_sections.insert(sec_con, p2_section_con);
    m_sections.insert(sec_pub, p2_section_pub);
    m_sections.insert(sec_pri, p2_section_pri);
    m_sections.insert(sec_var, p2_section_var);
}

/**
 * @brief P2Asm destructor
 */
P2Asm::~P2Asm()
{
}

/**
 * @brief Clear the results and restore defaults after a previous assembly
 */
void P2Asm::clear()
{
    pass_clear();
    if (0 == m_pass)
        m_symbols.clear();
}

/**
 * @brief Clear the results of the first pass
 */
void P2Asm::pass_clear()
{
    // next pass
    m_pass++;
    m_listing.clear();
    m_hash_PC.clear();
    m_hash_IR.clear();
    m_hash_error.clear();
    m_lineno = 0;
    m_in_curly = 0;
    m_line.clear();
    m_error.clear();
    m_next_PC = 0;
    m_curr_PC = 0;
    m_last_PC = 0;
    m_advance = 0;
    m_emit_IR = false;
    m_IR.opcode = 0;
    m_data.clear();
    m_words.clear();
    m_instr = t_invalid;
    m_symbol.clear();
    m_function.clear();
    m_section = sec_dat;
    m_wcnt = 0;
    m_widx = 0;
    memset(MEM.BYTES, 0, sizeof(MEM.BYTES));
}

int P2Asm::pass() const
{
    return m_pass;
}

const QStringList& P2Asm::source() const
{
    return m_source;
}

const QString& P2Asm::source(int idx) const
{
    static const QString empty;
    if (idx  < 0 || idx >= m_source.count())
        return empty;
    return m_source.at(idx);
}

void P2Asm::setSource(const QStringList& source)
{
    clear();
    m_source.clear();
    foreach(const QString& line, source)
        m_source += expand_tabs(line);
}

int P2Asm::count() const
{
    return m_source.count();
}

const p2_PC_hash_t& P2Asm::PC_hash() const
{
    return m_hash_PC;
}

p2_LONG P2Asm::PC_value(int lineno) const
{
    return m_hash_PC.value(lineno);
}

bool P2Asm::PC_available(int lineno) const
{
    return m_hash_PC.contains(lineno);
}

const p2_IR_hash_t& P2Asm::IR_hash() const
{
    return m_hash_IR;
}

p2_opcode_u P2Asm::IR_value(int lineno) const
{
    return m_hash_IR.value(lineno);
}

bool P2Asm::IR_available(int lineno) const
{
    return m_hash_IR.contains(lineno);
}

const p2_words_hash_t& P2Asm::words_hash() const
{
    return m_hash_words;
}

P2AsmWords P2Asm::words(int lineno) const
{
    return m_hash_words.value(lineno, P2AsmWords());
}

bool P2Asm::words_available(int lineno) const
{
    return m_hash_words.contains(lineno);
}

const p2_error_hash_t& P2Asm::error_hash() const
{
    return m_hash_error;
}

QStringList P2Asm::errors(int lineno) const
{
    return m_hash_error.values(lineno);
}

const QStringList& P2Asm::listing() const
{
    return m_listing;
}

const P2AsmSymTbl& P2Asm::symbols() const
{
    return m_symbols;
}

p2_cond_e P2Asm::conditional(p2_token_e cond)
{
    if (!Token.is_type(cond, tt_conditional))
        return cc_always;
    p2_cond_e result = Token.conditional(cond, cc_always);
    m_widx += 1;
    return result;
}

p2_cond_e P2Asm::parse_modcz(p2_token_e cond)
{
    p2_cond_e result = Token.modcz_param(cond, cc_clr);
    m_widx += 1;
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
bool P2Asm::split_and_tokenize(const QString& line)
{
    P2AsmWords words;
    QString word;
    QChar in_string = QChar::Null;
    bool escaped = false;
    bool comment = false;
    int pos = 0;
    int end = -1;

    foreach(QChar ch, line) {
        ++end;  // next character

        if (escaped) {
            // previous character was an escape (\)
            escaped = false;
            word += ch;
            continue;
        }

        if (in_string != QChar::Null) {
            // inside string
            if (ch == in_string) {
                // end of string
                in_string = QChar::Null;
            }
            word += ch;
            continue;
        }

        if (m_in_curly) {
            // inside curly brace comment
            if (ch == QChar('{')) {
                // next level of curly brace comment
                ++m_in_curly;
            }
            if (ch == QChar('}')) {
                // previous level of curly brace comment
                --m_in_curly;
            }
            continue;
        } else if (ch == QChar('{')) {
            // start of curly brace comment
            ++m_in_curly;
            continue;
        }

        if (ch.isSpace()) {
            // found white space
            if (word.isEmpty())
                continue;
            // non empty white space separated word
            words += P2AsmWord(word, pos, end);
            pos = end;
            word.clear();
            continue;
        }

        if (ch == ch_comment) {
            // start of comment until end of line
            comment = true;
            break;
        }

        if (ch == ch_comma) {
            // found a comma
            if (!word.isEmpty()) {
                // previous word ends here
                words += P2AsmWord(word, pos, end);
                pos = end;
            }
            // make the comma (,) a separate token
            word = ch;
            words += P2AsmWord(t_comma, word, end, end);
            pos = end;
            word.clear();
            continue;
        }

        if (ch == QChar('"')) {
            // found start of a string
            word += ch;
            in_string = ch;
            continue;
        }

        // append character to word
        word += ch;
    }

    // if not trailing comment, append the last word to words
    if (!comment && !word.isEmpty()) {
        words += P2AsmWord(word, pos, end);
    }

    // Now tokenize the words
    p2_token_v tokens;
    p2_token_e prev = t_invalid;
    for (int i = 0; i < words.count(); /* */) {
        P2AsmWord word = words.at(i);
        p2_token_e tok = Token.token(word.str());

        // join words delimited by space but starting with operators
        if (t_invalid != prev && Token.is_operation(tok)) {
            words[i-1].append(word.str(), word.end());
            words.removeAt(i);
            // if there is a next word, also join it
            if (i < words.count()) {
                P2AsmWord word = words[i];
                words[i-1].append(word.str(), word.end());
                words.removeAt(i);
            }
            // re-tokenize the previous word
            words[i-1].setToken(Token.token(words[i-1].str()));
            continue;
        }
        words[i].setToken(tok);
        prev = tok;
        i++;
    }
    m_words = words;
    m_wcnt = words.count();
    m_widx = 0;

    return true;
}

/**
 * @brief Assemble a QStringList of lines of SPIN2 source code
 * @param source code
 * @return true on success
 */
bool P2Asm::assemble_pass()
{
    pass_clear();

    foreach(m_line, m_source) {
        // Parse the next line
        m_lineno += 1;

        // Reset some state
        m_error.clear();
        m_words.clear();
        m_data.clear();
        m_advance = 0;
        m_emit_IR = false;

        // Skip over empty lines
        if (m_line.isEmpty()) {
            results();
            continue;
        }

        // Split line into words and tokenize it
        split_and_tokenize(m_line);

        // Set current program counter from next
        m_curr_PC = m_next_PC;

        // Whenever the line starts with a token t_nothing, i.e. not an reserved name,
        // the first word is defined as a symbol for the current program counter value
        m_symbol.clear();
        if (m_widx < m_wcnt) {
            switch (m_words.at(m_widx).tok()) {
            case t_locsym:
                // append local name to section::function / section
                m_symbol = find_symbol(m_section, m_function.value(m_section), m_words.at(m_widx).str());
                break;

            case t_symbol:
                // append global name to section::symbol
                m_function.insert(m_section, m_words.at(m_widx).str());
                m_symbol = find_symbol(m_section, m_function.value(m_section));
                break;

            default:
                m_symbol.clear();
            }

            // defining a symbol at the current PC
            if (!m_symbol.isEmpty()) {
                const p2_LONG PC = m_curr_PC / (m_curr_PC < HUB_ADDR ? 4 : 1);
                P2AsmSymbol sym = m_symbols.value(m_symbol);
                if (sym.isEmpty()) {
                    // Not defined yet
                    m_symbols.insert(m_symbol, P2Atom(PC));
                    m_symbols.addReference(m_symbol, m_lineno);
                } else if (m_pass > 1) {
                    m_symbols.setValue(m_symbol, PC);
                } else {
                    // Already defined
                    m_error = tr("Symbol '%1' already defined in line %2: $%3")
                              .arg(m_symbol)
                              .arg(sym.reference(0))
                              .arg(sym.value<p2_LONG>(), 6, 16, QChar('0'));
                    emit Error(m_lineno, m_error);
                }
                m_widx++;   // skip over first word
            }
        }

        // Skip if no more words/tokens were found
        if (m_widx >= m_wcnt) {
            results();
            continue;
        }

        // Assume the instruction advances by 4 bytes
        m_advance = 4;
        // Assume the instruction emits IR
        m_emit_IR = true;
        // Reset all instruction bits
        m_IR.opcode = 0;
        // Conditional execution prefix
        m_IR.op.cond = conditional(m_words[m_widx].tok());

        // Expect a token for an instruction
        m_instr = m_words[m_widx].tok();
        bool success = false;

        switch (m_instr) {
        case t_ABS:
            success = asm_abs();
            break;

        case t_ADD:
            success = asm_add();
            break;

        case t_ADDCT1:
            success = asm_addct1();
            break;

        case t_ADDCT2:
            success = asm_addct2();
            break;

        case t_ADDCT3:
            success = asm_addct3();
            break;

        case t_ADDPIX:
            success = asm_addpix();
            break;

        case t_ADDS:
            success = asm_adds();
            break;

        case t_ADDSX:
            success = asm_addsx();
            break;

        case t_ADDX:
            success = asm_addx();
            break;

        case t_AKPIN:
            success = asm_akpin();
            break;

        case t_ALLOWI:
            success = asm_allowi();
            break;

        case t_ALTB:
            success = asm_altb();
            break;

        case t_ALTD:
            success = asm_altd();
            break;

        case t_ALTGB:
            success = asm_altgb();
            break;

        case t_ALTGN:
            success = asm_altgn();
            break;

        case t_ALTGW:
            success = asm_altgw();
            break;

        case t_ALTI:
            success = asm_alti();
            break;

        case t_ALTR:
            success = asm_altr();
            break;

        case t_ALTS:
            success = asm_alts();
            break;

        case t_ALTSB:
            success = asm_altsb();
            break;

        case t_ALTSN:
            success = asm_altsn();
            break;

        case t_ALTSW:
            success = asm_altsw();
            break;

        case t_AND:
            success = asm_and();
            break;

        case t_ANDN:
            success = asm_andn();
            break;

        case t_AUGD:
            success = asm_augd();
            break;

        case t_AUGS:
            success = asm_augs();
            break;

        case t_BITC:
            success = asm_bitc();
            break;

        case t_BITH:
            success = asm_bith();
            break;

        case t_BITL:
            success = asm_bitl();
            break;

        case t_BITNC:
            success = asm_bitnc();
            break;

        case t_BITNOT:
            success = asm_bitnot();
            break;

        case t_BITNZ:
            success = asm_bitnz();
            break;

        case t_BITRND:
            success = asm_bitrnd();
            break;

        case t_BITZ:
            success = asm_bitz();
            break;

        case t_BLNPIX:
            success = asm_blnpix();
            break;

        case t_BMASK:
            success = asm_bmask();
            break;

        case t_BRK:
            success = asm_brk();
            break;

        case t_CALL:
            success = asm_call();
            break;

        case t_CALLA:
            success = asm_calla();
            break;

        case t_CALLB:
            success = asm_callb();
            break;

        case t_CALLD:
            success = asm_calld();
            break;

        case t_CALLPA:
            success = asm_callpa();
            break;

        case t_CALLPB:
            success = asm_callpb();
            break;

        case t_CMP:
            success = asm_cmp();
            break;

        case t_CMPM:
            success = asm_cmpm();
            break;

        case t_CMPR:
            success = asm_cmpr();
            break;

        case t_CMPS:
            success = asm_cmps();
            break;

        case t_CMPSUB:
            success = asm_cmpsub();
            break;

        case t_CMPSX:
            success = asm_cmpsx();
            break;

        case t_CMPX:
            success = asm_cmpx();
            break;

        case t_COGATN:
            success = asm_cogatn();
            break;

        case t_COGBRK:
            success = asm_cogbrk();
            break;

        case t_COGID:
            success = asm_cogid();
            break;

        case t_COGINIT:
            success = asm_coginit();
            break;

        case t_COGSTOP:
            success = asm_cogstop();
            break;

        case t_CRCBIT:
            success = asm_crcbit();
            break;

        case t_CRCNIB:
            success = asm_crcnib();
            break;

        case t_DECMOD:
            success = asm_decmod();
            break;

        case t_DECOD:
            success = asm_decod();
            break;

        case t_DIRC:
            success = asm_dirc();
            break;

        case t_DIRH:
            success = asm_dirh();
            break;

        case t_DIRL:
            success = asm_dirl();
            break;

        case t_DIRNC:
            success = asm_dirnc();
            break;

        case t_DIRNOT:
            success = asm_dirnot();
            break;

        case t_DIRNZ:
            success = asm_dirnz();
            break;

        case t_DIRRND:
            success = asm_dirrnd();
            break;

        case t_DIRZ:
            success = asm_dirz();
            break;

        case t_DJF:
            success = asm_djf();
            break;

        case t_DJNF:
            success = asm_djnf();
            break;

        case t_DJNZ:
            success = asm_djnz();
            break;

        case t_DJZ:
            success = asm_djz();
            break;

        case t_DRVC:
            success = asm_drvc();
            break;

        case t_DRVH:
            success = asm_drvh();
            break;

        case t_DRVL:
            success = asm_drvl();
            break;

        case t_DRVNC:
            success = asm_drvnc();
            break;

        case t_DRVNOT:
            success = asm_drvnot();
            break;

        case t_DRVNZ:
            success = asm_drvnz();
            break;

        case t_DRVRND:
            success = asm_drvrnd();
            break;

        case t_DRVZ:
            success = asm_drvz();
            break;

        case t_ENCOD:
            success = asm_encod();
            break;

        case t_EXECF:
            success = asm_execf();
            break;

        case t_FBLOCK:
            success = asm_fblock();
            break;

        case t_FGE:
            success = asm_fge();
            break;

        case t_FGES:
            success = asm_fges();
            break;

        case t_FLE:
            success = asm_fle();
            break;

        case t_FLES:
            success = asm_fles();
            break;

        case t_FLTC:
            success = asm_fltc();
            break;

        case t_FLTH:
            success = asm_flth();
            break;

        case t_FLTL:
            success = asm_fltl();
            break;

        case t_FLTNC:
            success = asm_fltnc();
            break;

        case t_FLTNOT:
            success = asm_fltnot();
            break;

        case t_FLTNZ:
            success = asm_fltnz();
            break;

        case t_FLTRND:
            success = asm_fltrnd();
            break;

        case t_FLTZ:
            success = asm_fltz();
            break;

        case t_GETBRK:
            success = asm_getbrk();
            break;

        case t_GETBYTE:
            success = asm_getbyte();
            break;

        case t_GETCT:
            success = asm_getct();
            break;

        case t_GETNIB:
            success = asm_getnib();
            break;

        case t_GETPTR:
            success = asm_getptr();
            break;

        case t_GETQX:
            success = asm_getqx();
            break;

        case t_GETQY:
            success = asm_getqy();
            break;

        case t_GETRND:
            success = asm_getrnd();
            break;

        case t_GETSCP:
            success = asm_getscp();
            break;

        case t_GETWORD:
            success = asm_getword();
            break;

        case t_GETXACC:
            success = asm_getxacc();
            break;

        case t_HUBSET:
            success = asm_hubset();
            break;

        case t_IJNZ:
            success = asm_ijnz();
            break;

        case t_IJZ:
            success = asm_ijz();
            break;

        case t_INCMOD:
            success = asm_incmod();
            break;

        case t_JATN:
            success = asm_jatn();
            break;

        case t_JCT1:
            success = asm_jct1();
            break;

        case t_JCT2:
            success = asm_jct2();
            break;

        case t_JCT3:
            success = asm_jct3();
            break;

        case t_JFBW:
            success = asm_jfbw();
            break;

        case t_JINT:
            success = asm_jint();
            break;

        case t_JMP:
            success = asm_jmp();
            break;

        case t_JMPREL:
            success = asm_jmprel();
            break;

        case t_JNATN:
            success = asm_jnatn();
            break;

        case t_JNCT1:
            success = asm_jnct1();
            break;

        case t_JNCT2:
            success = asm_jnct2();
            break;

        case t_JNCT3:
            success = asm_jnct3();
            break;

        case t_JNFBW:
            success = asm_jnfbw();
            break;

        case t_JNINT:
            success = asm_jnint();
            break;

        case t_JNPAT:
            success = asm_jnpat();
            break;

        case t_JNQMT:
            success = asm_jnqmt();
            break;

        case t_JNSE1:
            success = asm_jnse1();
            break;

        case t_JNSE2:
            success = asm_jnse2();
            break;

        case t_JNSE3:
            success = asm_jnse3();
            break;

        case t_JNSE4:
            success = asm_jnse4();
            break;

        case t_JNXFI:
            success = asm_jnxfi();
            break;

        case t_JNXMT:
            success = asm_jnxmt();
            break;

        case t_JNXRL:
            success = asm_jnxrl();
            break;

        case t_JNXRO:
            success = asm_jnxro();
            break;

        case t_JPAT:
            success = asm_jpat();
            break;

        case t_JQMT:
            success = asm_jqmt();
            break;

        case t_JSE1:
            success = asm_jse1();
            break;

        case t_JSE2:
            success = asm_jse2();
            break;

        case t_JSE3:
            success = asm_jse3();
            break;

        case t_JSE4:
            success = asm_jse4();
            break;

        case t_JXFI:
            success = asm_jxfi();
            break;

        case t_JXMT:
            success = asm_jxmt();
            break;

        case t_JXRL:
            success = asm_jxrl();
            break;

        case t_JXRO:
            success = asm_jxro();
            break;

        case t_LOC:
            success = asm_loc();
            break;

        case t_LOCKNEW:
            success = asm_locknew();
            break;

        case t_LOCKREL:
            success = asm_lockrel();
            break;

        case t_LOCKRET:
            success = asm_lockret();
            break;

        case t_LOCKTRY:
            success = asm_locktry();
            break;

        case t_MERGEB:
            success = asm_mergeb();
            break;

        case t_MERGEW:
            success = asm_mergew();
            break;

        case t_MIXPIX:
            success = asm_mixpix();
            break;

        case t_MODCZ:
            success = asm_modcz();
            break;

        case t_MOV:
            success = asm_mov();
            break;

        case t_MOVBYTS:
            success = asm_movbyts();
            break;

        case t_MUL:
            success = asm_mul();
            break;

        case t_MULPIX:
            success = asm_mulpix();
            break;

        case t_MULS:
            success = asm_muls();
            break;

        case t_MUXC:
            success = asm_muxc();
            break;

        case t_MUXNC:
            success = asm_muxnc();
            break;

        case t_MUXNIBS:
            success = asm_muxnibs();
            break;

        case t_MUXNITS:
            success = asm_muxnits();
            break;

        case t_MUXNZ:
            success = asm_muxnz();
            break;

        case t_MUXQ:
            success = asm_muxq();
            break;

        case t_MUXZ:
            success = asm_muxz();
            break;

        case t_NEG:
            success = asm_neg();
            break;

        case t_NEGC:
            success = asm_negc();
            break;

        case t_NEGNC:
            success = asm_negnc();
            break;

        case t_NEGNZ:
            success = asm_negnz();
            break;

        case t_NEGZ:
            success = asm_negz();
            break;

        case t_NIXINT1:
            success = asm_nixint1();
            break;

        case t_NIXINT2:
            success = asm_nixint2();
            break;

        case t_NIXINT3:
            success = asm_nixint3();
            break;

        case t_NOP:
            success = asm_nop();
            break;

        case t_NOT:
            success = asm_not();
            break;

        case t_ONES:
            success = asm_ones();
            break;

        case t_OR:
            success = asm_or();
            break;

        case t_OUTC:
            success = asm_outc();
            break;

        case t_OUTH:
            success = asm_outh();
            break;

        case t_OUTL:
            success = asm_outl();
            break;

        case t_OUTNC:
            success = asm_outnc();
            break;

        case t_OUTNOT:
            success = asm_outnot();
            break;

        case t_OUTNZ:
            success = asm_outnz();
            break;

        case t_OUTRND:
            success = asm_outrnd();
            break;

        case t_OUTZ:
            success = asm_outz();
            break;

        case t_POLLATN:
            success = asm_pollatn();
            break;

        case t_POLLCT1:
            success = asm_pollct1();
            break;

        case t_POLLCT2:
            success = asm_pollct2();
            break;

        case t_POLLCT3:
            success = asm_pollct3();
            break;

        case t_POLLFBW:
            success = asm_pollfbw();
            break;

        case t_POLLINT:
            success = asm_pollint();
            break;

        case t_POLLPAT:
            success = asm_pollpat();
            break;

        case t_POLLQMT:
            success = asm_pollqmt();
            break;

        case t_POLLSE1:
            success = asm_pollse1();
            break;

        case t_POLLSE2:
            success = asm_pollse2();
            break;

        case t_POLLSE3:
            success = asm_pollse3();
            break;

        case t_POLLSE4:
            success = asm_pollse4();
            break;

        case t_POLLXFI:
            success = asm_pollxfi();
            break;

        case t_POLLXMT:
            success = asm_pollxmt();
            break;

        case t_POLLXRL:
            success = asm_pollxrl();
            break;

        case t_POLLXRO:
            success = asm_pollxro();
            break;

        case t_POP:
            success = asm_pop();
            break;

        case t_POPA:
            success = asm_popa();
            break;

        case t_POPB:
            success = asm_popb();
            break;

        case t_PUSH:
            success = asm_push();
            break;

        case t_PUSHA:
            success = asm_pusha();
            break;

        case t_PUSHB:
            success = asm_pushb();
            break;

        case t_QDIV:
            success = asm_qdiv();
            break;

        case t_QEXP:
            success = asm_qexp();
            break;

        case t_QFRAC:
            success = asm_qfrac();
            break;

        case t_QLOG:
            success = asm_qlog();
            break;

        case t_QMUL:
            success = asm_qmul();
            break;

        case t_QROTATE:
            success = asm_qrotate();
            break;

        case t_QSQRT:
            success = asm_qsqrt();
            break;

        case t_QVECTOR:
            success = asm_qvector();
            break;

        case t_RCL:
            success = asm_rcl();
            break;

        case t_RCR:
            success = asm_rcr();
            break;

        case t_RCZL:
            success = asm_rczl();
            break;

        case t_RCZR:
            success = asm_rczr();
            break;

        case t_RDBYTE:
            success = asm_rdbyte();
            break;

        case t_RDFAST:
            success = asm_rdfast();
            break;

        case t_RDLONG:
            success = asm_rdlong();
            break;

        case t_RDLUT:
            success = asm_rdlut();
            break;

        case t_RDPIN:
            success = asm_rdpin();
            break;

        case t_RDWORD:
            success = asm_rdword();
            break;

        case t_REP:
            success = asm_rep();
            break;

        case t_RESI0:
            success = asm_resi0();
            break;

        case t_RESI1:
            success = asm_resi1();
            break;

        case t_RESI2:
            success = asm_resi2();
            break;

        case t_RESI3:
            success = asm_resi3();
            break;

        case t_RET:
            success = asm_ret();
            break;

        case t_RETA:
            success = asm_reta();
            break;

        case t_RETB:
            success = asm_retb();
            break;

        case t_RETI0:
            success = asm_reti0();
            break;

        case t_RETI1:
            success = asm_reti1();
            break;

        case t_RETI2:
            success = asm_reti2();
            break;

        case t_RETI3:
            success = asm_reti3();
            break;

        case t_REV:
            success = asm_rev();
            break;

        case t_RFBYTE:
            success = asm_rfbyte();
            break;

        case t_RFLONG:
            success = asm_rflong();
            break;

        case t_RFVAR:
            success = asm_rfvar();
            break;

        case t_RFVARS:
            success = asm_rfvars();
            break;

        case t_RFWORD:
            success = asm_rfword();
            break;

        case t_RGBEXP:
            success = asm_rgbexp();
            break;

        case t_RGBSQZ:
            success = asm_rgbsqz();
            break;

        case t_ROL:
            success = asm_rol();
            break;

        case t_ROLBYTE:
            success = asm_rolbyte();
            break;

        case t_ROLNIB:
            success = asm_rolnib();
            break;

        case t_ROLWORD:
            success = asm_rolword();
            break;

        case t_ROR:
            success = asm_ror();
            break;

        case t_RQPIN:
            success = asm_rqpin();
            break;

        case t_SAL:
            success = asm_sal();
            break;

        case t_SAR:
            success = asm_sar();
            break;

        case t_SCA:
            success = asm_sca();
            break;

        case t_SCAS:
            success = asm_scas();
            break;

        case t_SETBYTE:
            success = asm_setbyte();
            break;

        case t_SETCFRQ:
            success = asm_setcfrq();
            break;

        case t_SETCI:
            success = asm_setci();
            break;

        case t_SETCMOD:
            success = asm_setcmod();
            break;

        case t_SETCQ:
            success = asm_setcq();
            break;

        case t_SETCY:
            success = asm_setcy();
            break;

        case t_SETD:
            success = asm_setd();
            break;

        case t_SETDACS:
            success = asm_setdacs();
            break;

        case t_SETINT1:
            success = asm_setint1();
            break;

        case t_SETINT2:
            success = asm_setint2();
            break;

        case t_SETINT3:
            success = asm_setint3();
            break;

        case t_SETLUTS:
            success = asm_setluts();
            break;

        case t_SETNIB:
            success = asm_setnib();
            break;

        case t_SETPAT:
            success = asm_setpat();
            break;

        case t_SETPIV:
            success = asm_setpiv();
            break;

        case t_SETPIX:
            success = asm_setpix();
            break;

        case t_SETQ:
            success = asm_setq();
            break;

        case t_SETQ2:
            success = asm_setq2();
            break;

        case t_SETR:
            success = asm_setr();
            break;

        case t_SETS:
            success = asm_sets();
            break;

        case t_SETSCP:
            success = asm_setscp();
            break;

        case t_SETSE1:
            success = asm_setse1();
            break;

        case t_SETSE2:
            success = asm_setse2();
            break;

        case t_SETSE3:
            success = asm_setse3();
            break;

        case t_SETSE4:
            success = asm_setse4();
            break;

        case t_SETWORD:
            success = asm_setword();
            break;

        case t_SETXFRQ:
            success = asm_setxfrq();
            break;

        case t_SEUSSF:
            success = asm_seussf();
            break;

        case t_SEUSSR:
            success = asm_seussr();
            break;

        case t_SHL:
            success = asm_shl();
            break;

        case t_SHR:
            success = asm_shr();
            break;

        case t_SIGNX:
            success = asm_signx();
            break;

        case t_SKIP:
            success = asm_skip();
            break;

        case t_SKIPF:
            success = asm_skipf();
            break;

        case t_SPLITB:
            success = asm_splitb();
            break;

        case t_SPLITW:
            success = asm_splitw();
            break;

        case t_STALLI:
            success = asm_stalli();
            break;

        case t_SUB:
            success = asm_sub();
            break;

        case t_SUBR:
            success = asm_subr();
            break;

        case t_SUBS:
            success = asm_subs();
            break;

        case t_SUBSX:
            success = asm_subsx();
            break;

        case t_SUBX:
            success = asm_subx();
            break;

        case t_SUMC:
            success = asm_sumc();
            break;

        case t_SUMNC:
            success = asm_sumnc();
            break;

        case t_SUMNZ:
            success = asm_sumnz();
            break;

        case t_SUMZ:
            success = asm_sumz();
            break;

        case t_TEST:
            success = asm_test();
            break;

        case t_TESTB:
            success = asm_testb_w();
            break;

        case t_TESTBN:
            success = asm_testbn_w();
            break;

        case t_TESTN:
            success = asm_testn();
            break;

        case t_TESTP:
            success = asm_testp_w();
            break;

        case t_TESTPN:
            success = asm_testpn_w();
            break;

        case t_TJF:
            success = asm_tjf();
            break;

        case t_TJNF:
            success = asm_tjnf();
            break;

        case t_TJNS:
            success = asm_tjns();
            break;

        case t_TJNZ:
            success = asm_tjnz();
            break;

        case t_TJS:
            success = asm_tjs();
            break;

        case t_TJV:
            success = asm_tjv();
            break;

        case t_TJZ:
            success = asm_tjz();
            break;

        case t_TRGINT1:
            success = asm_trgint1();
            break;

        case t_TRGINT2:
            success = asm_trgint2();
            break;

        case t_TRGINT3:
            success = asm_trgint3();
            break;

        case t_WAITATN:
            success = asm_waitatn();
            break;

        case t_WAITCT1:
            success = asm_waitct1();
            break;

        case t_WAITCT2:
            success = asm_waitct2();
            break;

        case t_WAITCT3:
            success = asm_waitct3();
            break;

        case t_WAITFBW:
            success = asm_waitfbw();
            break;

        case t_WAITINT:
            success = asm_waitint();
            break;

        case t_WAITPAT:
            success = asm_waitpat();
            break;

        case t_WAITSE1:
            success = asm_waitse1();
            break;

        case t_WAITSE2:
            success = asm_waitse2();
            break;

        case t_WAITSE3:
            success = asm_waitse3();
            break;

        case t_WAITSE4:
            success = asm_waitse4();
            break;

        case t_WAITX:
            success = asm_waitx();
            break;

        case t_WAITXFI:
            success = asm_waitxfi();
            break;

        case t_WAITXMT:
            success = asm_waitxmt();
            break;

        case t_WAITXRL:
            success = asm_waitxrl();
            break;

        case t_WAITXRO:
            success = asm_waitxro();
            break;

        case t_WFBYTE:
            success = asm_wfbyte();
            break;

        case t_WFLONG:
            success = asm_wflong();
            break;

        case t_WFWORD:
            success = asm_wfword();
            break;

        case t_WMLONG:
            success = asm_wmlong();
            break;

        case t_WRBYTE:
            success = asm_wrbyte();
            break;

        case t_WRC:
            success = asm_wrc();
            break;

        case t_WRFAST:
            success = asm_wrfast();
            break;

        case t_WRLONG:
            success = asm_wrlong();
            break;

        case t_WRLUT:
            success = asm_wrlut();
            break;

        case t_WRNC:
            success = asm_wrnc();
            break;

        case t_WRNZ:
            success = asm_wrnz();
            break;

        case t_WRPIN:
            success = asm_wrpin();
            break;

        case t_WRWORD:
            success = asm_wrword();
            break;

        case t_WRZ:
            success = asm_wrz();
            break;

        case t_WXPIN:
            success = asm_wxpin();
            break;

        case t_WYPIN:
            success = asm_wypin();
            break;

        case t_XCONT:
            success = asm_xcont();
            break;

        case t_XINIT:
            success = asm_xinit();
            break;

        case t_XOR:
            success = asm_xor();
            break;

        case t_XORO32:
            success = asm_xoro32();
            break;

        case t_XSTOP:
            success = asm_xstop();
            break;

        case t_XZERO:
            success = asm_xzero();
            break;

        case t_ZEROX:
            success = asm_zerox();
            break;

        case t__BYTE:
            success = asm_byte();
            break;

        case t__WORD:
            success = asm_word();
            break;

        case t__LONG:
            success = asm_long();
            break;

        case t__FILE:
            success = asm_file();
            break;

        case t__RES:
            success = asm_res();
            break;

        case t__FIT:
            success = asm_fit();
            break;

        case t__ORG:
            success = asm_org();
            break;

        case t__ORGH:
            success = asm_orgh();
            break;

        case t__ASSIGN:
            success = asm_assign();
            break;

        case t__DOLLAR:
            break;

        case t_PA:
        case t_PB:
        case t_PTRA:
        case t_PTRB:
            m_error = tr("Not an instruction: %1").arg(m_line);
            emit Error(m_lineno, m_error);
            break;

        case t__DAT:
            success = asm_dat();
            break;

        case t__CON:
            success = asm_con();
            break;

        case t__PUB:
            success = asm_pub();
            break;

        case t__PRI:
            success = asm_pri();
            break;

        case t__VAR:
            success = asm_var();
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
            m_error = tr("Multiple conditionals in line: %1").arg(m_line);
            emit Error(m_lineno, m_error);
            break;

        case t_MODCZ__CLR:       // cond_never
        case t_MODCZ__NC_AND_NZ: // cond_nc_and_nz
        case t_MODCZ__NZ_AND_NC: // alias for cond_nc_and_nz
        case t_MODCZ__GT:        // alias for cond_nc_and_nz
        case t_MODCZ__NC_AND_Z:  // cond_nc_and_z
        case t_MODCZ__Z_AND_NC:  // alias for cond_nc_and_z
        case t_MODCZ__NC:        // cond_nc
        case t_MODCZ__GE:        // alias for cond_nc
        case t_MODCZ__C_AND_NZ:  // cond_c_and_nz
        case t_MODCZ__NZ_AND_C:  // alias for cond_c_and_nz
        case t_MODCZ__NZ:        // cond_nz
        case t_MODCZ__NE:        // alias for cond_nz
        case t_MODCZ__C_NE_Z:    // cond_c_ne_z
        case t_MODCZ__Z_NE_C:    // alias for cond_c_ne_z
        case t_MODCZ__NC_OR_NZ:  // cond_nc_or_nz
        case t_MODCZ__NZ_OR_NC:  // alias for cond_nc_or_nz
        case t_MODCZ__C_AND_Z:   // cond_c_and_z
        case t_MODCZ__Z_AND_C:   // alias for cond_c_and_z
        case t_MODCZ__C_EQ_Z:    // cond_c_eq_z
        case t_MODCZ__Z_EQ_C:    // alias for cond_c_eq_z
        case t_MODCZ__Z:         // cond_z
        case t_MODCZ__E:         // alias for cond_z
        case t_MODCZ__NC_OR_Z:   // cond_nc_or_z
        case t_MODCZ__Z_OR_NC:   // alias for cond_nc_or_z
        case t_MODCZ__C:         // cond_c
        case t_MODCZ__LT:        // alias for cond_c
        case t_MODCZ__C_OR_NZ:   // cond_c_or_nz
        case t_MODCZ__NZ_OR_C:   // alias for cond_c_or_nz
        case t_MODCZ__C_OR_Z:    // cond_c_or_z
        case t_MODCZ__Z_OR_C:    // alias for cond_c_or_z
        case t_MODCZ__LE:        // alias for cond_c_or_z
        case t_MODCZ__SET:       // cond_always
            m_error = tr("MODCZ parameter in line: %1").arg(m_line);
            emit Error(m_lineno, m_error);
            break;

        default:
            // Handle non-instruction tokens
            m_error = tr("Missing handling of token '%1' line: %2")
                      .arg(Token.string(m_instr))
                      .arg(m_line);
            emit Error(m_lineno, m_error);
            break;
        }

        // store the results, emit listing, etc.
        results();

        Q_UNUSED(success)
    }

    return true;
}

bool P2Asm::assemble(const QStringList& list)
{
    bool success = true;
    m_source = list;
    success = assemble_pass() &&
              assemble_pass();
    return success;
}

/**
 * @brief Assemble a source file
 * @param filename name of the SPIN2 source
 * @return QByteArray containing the binary
 */
bool P2Asm::assemble(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        emit Error(0, tr("Can not open '%1' for reading.").arg(filename));
        return false;
    }

    QTextStream stream(&file);
    QStringList list;
    while (!stream.atEnd())
        list += stream.readLine();

    return assemble(list);
}

/**
 * @brief Return number of tokens/words left
 * @return number of tokens/words left
 */
int P2Asm::left()
{
    return m_wcnt - m_widx;
}

/**
 * @brief Store the results and append a line to the listing
 * @param opcode true, if the IR field contains an opcode
 */
void P2Asm::results()
{
    const bool binary = false;
    const p2_LONG PC = m_curr_PC;

    QString output;
    if (m_emit_IR) {
        if (m_advance == 4) {
            m_hash_PC.insert(m_lineno, m_curr_PC);
            m_hash_IR.insert(m_lineno, m_IR);
            // opcode was constructed
            output = QString("%1 %2 [%3] %4")
                     .arg(m_lineno, -6)
                     .arg(PC, 6, 16, QChar('0'))
                     .arg(m_IR.opcode, 8, 16, QChar('0'))
                     .arg(m_line);

            if (binary && m_curr_PC < MEM_SIZE) {
                MEM.LONGS[m_curr_PC / 4] = m_IR.opcode;
            }
        }
    } else if (m_data.isEmpty()) {
        if (m_words.isEmpty()) {
            // comment or non code generating instruction
            output = QString("%1 %2 -%3- %4")
                     .arg(m_lineno, -6)
                     .arg(PC, 6, 16, QChar('0'))
                     .arg(QStringLiteral("--------"))
                     .arg(m_line);
        } else {
            // assignment to symbol
            m_hash_IR.insert(m_lineno, m_IR);
            output = QString("%1 %2 <%3> %4")
                     .arg(m_lineno, -6)
                     .arg(PC, 6, 16, QChar('0'))
                     .arg(m_IR.opcode, 8, 16, QChar('0'))
                     .arg(m_line);
        }

    } else {
        p2_BYTES bytes = m_data.to_bytes();
        p2_LONG offset = PC;
        p2_LONG datalong = 0;
        while (!bytes.isEmpty()) {
            datalong |= static_cast<p2_LONG>(bytes.takeFirst()) << (8 * (offset & 3));
            ++offset;

            if (m_data.isEmpty() || 0 == (offset & 3)) {
                // end of data or datalong contains 32 bits
                QString hex;

                if (m_curr_PC & 3) {
                    // unaligned
                    const int digits = 8 - (2 * (m_curr_PC & 3));
                    hex = QString("%1").arg(datalong, digits, 16, QChar('0'));

                    while (0 != (m_curr_PC & 3)) {
                        // align m_curr_PC to long
                        hex.insert(0, QStringLiteral("--"));
                        m_curr_PC++;
                    }
                } else {
                    // aligned
                    const int digits = 8 - (2 * (offset & 3));
                    hex = QString("%1").arg(datalong, digits, 16, QChar('0'));
                }

                // pad until offset is aligned
                while (offset & 3) {
                    hex.append(QStringLiteral("--"));
                    offset++;
                }

                // another p2_LONG to output
                output = QString("%1 %2 {%3} %4")
                         .arg(m_lineno, -6)
                         .arg(PC, 6, 16, QChar('0'))
                         .arg(hex)
                         .arg(m_line);

                if (!m_data.isEmpty()) {
                    // more data follows
                    m_listing.append(output);
                }
                datalong = 0;
            }
        }
    }
    m_listing.append(output);

    // Calculate next PC for regular instructions
    m_next_PC = m_curr_PC + m_advance;
    if (m_next_PC > m_last_PC)
        m_last_PC = m_next_PC;

    if (!m_words.isEmpty())
        m_hash_words.insert(m_lineno, m_words);
    if (!m_error.isEmpty())
        m_hash_error.insert(m_lineno, m_error);
}

QString P2Asm::expand_tabs(const QString& src)
{
    QString result;
    QChar in_string = QChar::Null;
    bool escaped = false;
    int pos = 0;

    foreach(const QChar ch, src) {
        if (in_string != QChar::Null) {
            // inside string
            if (escaped) {
                // previous character was an escape (\)
                escaped = false;
            } else if (ch == in_string) {
                // end of string
                in_string = QChar::Null;
            } else if (ch == str_escape) {
                escaped = true;
            }
            result += ch;
            continue;
        }

        if (ch == QChar::Tabulation) {
            result += QChar::Space;
            pos++;
            while ((pos & 7) != 0) {
                result += QChar::Space;
                pos++;
            }
        } else {
            result += ch;
            pos++;
        }
    }
    return result;
}

/**
 * @brief Make an initial P2Atom based on the instruction token
 * @return P2Atom with type set up
 */
P2Atom P2Asm::make_atom()
{
    P2Atom atom;
    switch (m_instr) {
    case t__BYTE: atom.setType(P2Atom::Byte); break;
    case t__WORD: atom.setType(P2Atom::Word); break;
    case t__LONG: atom.setType(P2Atom::Long); break;
    case t__FILE: atom.setType(P2Atom::String); break;
    default:      atom.setType(P2Atom::Long);
    }
    return atom;
}

/**
 * @brief Convert a string of binary digits into an unsigned value
 * @param pos current position in str
 * @param str binary digits
 * @return P2Atom value of binary digits in str
 */
P2Atom P2Asm::from_bin(int& pos, const QString& str)
{
    static const QString& digits = bin_digits;
    P2Atom atom = make_atom();
    p2_QUAD bits = 0;
    int nbits = 0;

    if (str.startsWith(QStringLiteral("%")))
        ++pos;

    for (/**/; pos < str.length(); pos++) {
        QChar ch = str[pos].toUpper();
        const int idx = digits.indexOf(ch);
        if (idx < 0)
            break;
        if (skip_digit == digits.at(idx))
            continue;
        bits = (bits << 1) + static_cast<uint>(idx);
        nbits += 1;
        while (nbits >= 32) {
            atom.append(bits);
            bits >>= 32;
            nbits -= 32;
        }
    }
    atom.append(nbits, bits);
    return atom;
}

/**
 * @brief Convert a string of byte indices into an unsigned value
 * @param pos current position in str
 * @param str byte inidex digits
 * @return P2Atom value of byte indices in str
 */
P2Atom P2Asm::from_byt(int& pos, const QString& str)
{
    static const QString& digits = byt_digits;
    P2Atom atom = make_atom();
    p2_QUAD bits = 0;
    int nbits = 0;

    if (str.startsWith(QStringLiteral("%%")))
        pos += 2;

    for (/**/; pos < str.length(); pos++) {
        QChar ch = str[pos].toUpper();
        const int idx = digits.indexOf(ch);
        if (idx < 0)
            break;
        if (skip_digit == digits.at(idx))
            continue;
        bits = (bits << 2) + static_cast<uint>(idx);
        nbits += 2;
        while (nbits >= 32) {
            atom.append(bits);
            bits >>= 32;
            nbits -= 32;
        }
    }
    atom.append(nbits, bits);
    return atom;
}

/**
 * @brief Convert a string of octal digits into an unsigned value
 * @param pos current position in str
 * @param str octal digits
 * @return P2Atom value of octal digits in str
 */
P2Atom P2Asm::from_oct(int& pos, const QString& str)
{
    static const QString& digits = oct_digits;
    P2Atom atom = make_atom();
    p2_QUAD bits = 0;
    int nbits = 0;

    for (/**/; pos < str.length(); pos++) {
        QChar ch = str[pos].toUpper();
        const int idx = digits.indexOf(ch);
        if (idx < 0)
            break;
        if (skip_digit == digits.at(idx))
            continue;
        bits = (bits << 3) + static_cast<uint>(idx);
        nbits += 3;
        while (nbits >= 32) {
            atom.append(bits);
            bits >>= 32;
            nbits -= 32;
        }
    }

    atom.append(nbits, bits);
    return atom;
}

/**
 * @brief Convert a string of decimal digits into an unsigned value
 * @param pos current position in str
 * @param str decimal digits
 * @return P2Atom value of decimal digits in str
 */
P2Atom P2Asm::from_dec(int& pos, const QString& str)
{
    static const QString& digits = dec_digits;
    P2Atom atom = make_atom();
    p2_QUAD bits = 0;

    for (/**/; pos < str.length(); pos++) {
        QChar ch = str[pos].toUpper();
        const int idx = digits.indexOf(ch);
        if (idx < 0)
            break;
        if (skip_digit == digits.at(idx))
            continue;
        bits = (bits * 10) + static_cast<uint>(idx);
        // FIXME: no way to handle 64 bit overflow ?
    }
    // append all significant bits
    atom.append(bits);
    return atom;
}

/**
 * @brief Convert a string of hexadecimal digits into an unsigned value
 * @param str hexadecimal digits
 * @return P2Atom value of hexadecimal digits in str
 */
P2Atom P2Asm::from_hex(int& pos, const QString& str)
{
    static const QString& digits = hex_digits;
    P2Atom atom = make_atom();
    p2_QUAD bits = 0;
    int nbits = 0;

    if (QChar('$') == str.at(pos))
        ++pos;

    for (/**/; pos < str.length(); pos++) {
        QChar ch = str[pos].toUpper();
        const int idx = digits.indexOf(ch);
        if (idx < 0)
            break;
        if (skip_digit == digits.at(idx))
            continue;
        bits = (bits << 4) + static_cast<uint>(idx);
        nbits += 4;
        while (nbits >= 32) {
            atom.append(bits);
            bits >>= 32;
            nbits -= 32;
        }
    }
    atom.append(nbits, bits);
    return atom;
}

/**
 * @brief Convert a string of ASCII characters into an unsigned value
 * @param str ASCII characters
 * @param stop characters to stop at
 * @return P2Atom value of in string
 */
P2Atom P2Asm::from_str(int& pos, const QString& str)
{
    P2Atom atom = make_atom();
    QChar ch = str.at(pos++);
    QChar in_string = ch;
    bool escaped = false;

    for (/**/; pos < str.length(); pos++) {
        ch = str.at(pos);
        if (escaped) {
            atom.append(P2Atom::Byte, static_cast<p2_BYTE>(ch.toLatin1()));
            escaped = false;
        } else if (ch == in_string) {
            pos++;
            return atom;
        } else if (ch == str_escape) {
            escaped = true;
        } else {
            atom.append(P2Atom::Byte, static_cast<p2_BYTE>(ch.toLatin1()));
        }
    }

    return atom;
}

/**
 * @brief Skip over spaces in %str at %pos
 * @param pos position in string
 * @param str reference to string
 * @return false if beyond end of str, true if spaces skipped
 */
bool P2Asm::skip_spc(int &pos, const QString& str)
{
    if (pos >= str.length())
        return false;
    while (pos < str.length() && str.at(pos).isSpace())
        pos++;
    return true;
}

/**
 * @brief Find a symbol name using either %func, or %func and %sym appended
 * @param sect primary section where to search
 * @param func name of the function
 * @param local name of a local symbol
 * @return QString with the name of the symbol
 */
QString P2Asm::find_symbol(Section sect, const QString& func, const QString& local)
{
    QString symbol;
    QString section = m_sections.value(sect);
    QStringList sections = m_sections.values();
    if (!section.isEmpty()) {
        sections.removeAll(section);
        sections.insert(0, section);
    }

    foreach (const QString& section, sections) {
        if (local.isEmpty()) {
            symbol = QString("%1::%2")
                     .arg(section.toUpper())
                     .arg(func.toUpper());
        } else {
            symbol = QString("%1::%2%3")
                     .arg(section.toUpper())
                     .arg(func.toUpper())
                     .arg(local.toUpper());
        }
        if (m_symbols.contains(symbol))
            return symbol;
    }
    // use original section
    if (local.isEmpty()) {
        symbol = QString("%1::%2")
                 .arg(section.toUpper())
                 .arg(func.toUpper());
    } else {
        symbol = QString("%1::%2%3")
                 .arg(section.toUpper())
                 .arg(func.toUpper())
                 .arg(local.toUpper());
    }
    return symbol;
}

/**
 * @brief Parse an atomic part of an expression
 * @param pos position in word where to start
 * @param str string to parse
 * @return value of the atom
 */
P2Atom P2Asm::parse_atom(int& pos, const QString& str)
{
    p2_token_e tok = t_invalid;
    int epos = str.length();
    bool do_not = false;
    bool do_neg = false;
    bool do_comp = false;
    bool do_inc = false;
    bool do_dec = false;
    P2Atom atom;
    QString symbol;
    QString rest;

    tok = Token.at_types(pos, str, tm_primary_unary);
    while (tok != t_invalid) {
        switch (tok) {
        // Handle primary operators (precedence 1)
        case t__INC:    // ++
            DEBUG_EXPR(" unary inc: %s", qPrintable(str.mid(pos)));
            do_inc = true;
            break;
        case t__DEC:    // --
            DEBUG_EXPR(" unary dec: %s", qPrintable(str.mid(pos)));
            do_dec = true;
            break;
            // Handle unary operators (precedence 2)
        case t__NEG:    // !
            DEBUG_EXPR(" unary neg: %s", qPrintable(str.mid(pos)));
            do_not = !do_not;
            break;
        case t__NOT:    // ~
            DEBUG_EXPR(" unary not: %s", qPrintable(str.mid(pos)));
            do_comp = !do_comp;
            break;
        case t__SUB:    // -
            DEBUG_EXPR(" unary minus: %s", qPrintable(str.mid(pos)));
            do_neg = !do_neg;
            break;
        case t__ADD:    // +
            break;
        default:
            Q_ASSERT_X(false, "unary ops", "invalid op");
        }
        skip_spc(pos, str);
        tok = Token.at_types(pos, str, tm_primary_unary);
    }

    // Rest of the string
    rest = str.mid(pos);

    tok = m_words[m_widx].tok();
    epos = str.length();
    if (t_unknown == tok || t_symbol == tok || t_locsym == tok) {
        tok = Token.token(rest, true, &epos);
        DEBUG_EXPR(" *** tokenize: '%s' -> '%s'", qPrintable(rest), qPrintable(rest.left(epos)));
        epos = pos + epos;
    }

    switch (tok) {
    case t__LPAREN:
        // precedence 1
        DEBUG_EXPR(" lparen: %s", qPrintable(str.mid(pos)));
        atom = parse_subexpression(pos, str);
        break;

    case t__RPAREN:
        DEBUG_EXPR(" rparen: %s", qPrintable(str.mid(pos)));
        break;

    case t_locsym:
        symbol = rest.mid(pos, epos);
        symbol = find_symbol(m_section, m_function.value(m_section), symbol);
        if (m_symbols.contains(symbol)) {
            DEBUG_EXPR(" found locsym: %s", qPrintable(symbol));
            P2AsmSymbol sym = m_symbols.value(symbol);
            atom = sym.value<P2Atom>();
            m_symbols.addReference(symbol, m_lineno);
            pos = epos;
        } else {
            DEBUG_EXPR(" undefined locsym: %s", qPrintable(symbol));
            atom.set(P2Atom::Long, 0);
        }
        break;

    case t_symbol:
        symbol = rest.mid(pos, epos);
        symbol = find_symbol(m_section, symbol);
        if (m_symbols.contains(symbol)) {
            DEBUG_EXPR(" found symbol: %s", qPrintable(symbol));
            P2AsmSymbol sym = m_symbols.value(symbol);
            atom = sym.value<P2Atom>();
            m_symbols.addReference(symbol, m_lineno);
            pos = epos;
        } else {
            DEBUG_EXPR(" undefined symbol: %s", qPrintable(symbol));
            atom.set(P2Atom::Long, 0);
        }
        break;

    case t_bin_const:
        DEBUG_EXPR(" bin value: %s", qPrintable(str.mid(pos)));
        atom = from_bin(pos, str);
        break;

    case t_byt_const:
        DEBUG_EXPR(" byt value: %s", qPrintable(str.mid(pos)));
        atom = from_byt(pos, str);
        break;

    case t_oct_const:
        DEBUG_EXPR(" oct value: %s", qPrintable(str.mid(pos)));
        atom = from_oct(pos, str);
        break;

    case t_dec_const:
        DEBUG_EXPR(" dec value: %s", qPrintable(str.mid(pos)));
        atom = from_dec(pos, str);
        break;

    case t_hex_const:
        DEBUG_EXPR(" hex value: %s", qPrintable(str.mid(pos)));
        atom = from_hex(pos, str);
        break;

    case t_string:
        DEBUG_EXPR(" str value: %s", qPrintable(str.mid(pos)));
        atom = from_str(pos, str);
        break;

    case t_PA:
        DEBUG_EXPR(" PA value: %s", qPrintable(str.mid(pos)));
        atom.set(P2Atom::Long, offs_PA);
        pos = epos;
        break;

    case t_PB:
        DEBUG_EXPR(" PB value: %s", qPrintable(str.mid(pos)));
        atom.set(P2Atom::Long, offs_PB);
        pos = epos;
        break;

    case t_PTRA:
    case t_PTRA_postinc:
    case t_PTRA_postdec:
    case t_PTRA_preinc:
    case t_PTRA_predec:
        DEBUG_EXPR(" PTRA value: %s", qPrintable(str.mid(pos)));
        atom.set(P2Atom::Long, offs_PTRA);
        pos = epos;
        break;

    case t_PTRB:
    case t_PTRB_postinc:
    case t_PTRB_postdec:
    case t_PTRB_preinc:
    case t_PTRB_predec:
        DEBUG_EXPR(" PTRB: value %s", qPrintable(str.mid(pos)));
        atom.set(P2Atom::Long, offs_PTRB);
        pos = epos;
        break;

    case t__DOLLAR:
        DEBUG_EXPR(" DOLLAR value: %s", qPrintable(str.mid(pos)));
        atom.set(P2Atom::Long, m_curr_PC);
        pos = epos;
        break;

    default:
        DEBUG_EXPR(" reserved word (%s): %s", qPrintable(Token.string(tok)), qPrintable(str.mid(pos)));
        m_error = tr("Reserved word (%1) used as parameter: %2")
                  .arg(Token.string(tok))
                  .arg(str);
        pos = epos;
        return atom;
    }

    // apply unary ops
    atom.complement1(do_comp);
    atom.complement2(do_neg);
    atom.make_bool(do_not);
    // apply primary ops
    atom.unary_dec(do_dec);
    atom.unary_inc(do_inc);

    return atom;
}

/**
 * @brief Parse an an expression of mulops (precedence 3)
 * @param pos position in %str where to start
 * @param str string to parse
 * @return P2Atom containing the result
 */
P2Atom P2Asm::parse_mulops(int& pos, const QString& str)
{
    P2Atom lvalue = parse_atom(pos, str);
    const int spos = pos; Q_UNUSED(spos)
            for (;;) {

        if (!skip_spc(pos, str))
            break;

        p2_token_e op = Token.at_type(pos, str, tt_mulop);
        if (t_invalid == op)
            break;

        P2Atom rvalue = parse_atom(pos, str);
        if (!rvalue.isValid()) {
            DEBUG_EXPR(" invalid rvalue: %s", qPrintable(str.mid(pos)));
            m_error = tr("Invalid character in expression (mulops): %1").arg(str.mid(spos));
            break;
        }

        p2_QUAD r = rvalue.to_quad();
        switch (op) {
        case t__MUL:
            DEBUG_EXPR(" mul: %s", qPrintable(str.mid(spos)));
            lvalue.arith_mul(r);
            break;
        case t__DIV:
            DEBUG_EXPR(" div: %s", qPrintable(str.mid(spos)));
            lvalue.arith_div(r);
            break;
        case t__MOD:
            DEBUG_EXPR(" mod: %s", qPrintable(str.mid(spos)));
            lvalue.arith_mod(r);
            break;
        default:
            Q_ASSERT_X(false, "mulops", "invalid op");
        }
    }
    return lvalue;
}

/**
 * @brief Parse an an expression of addops (precedence 4)
 * @param pos position in %str where to start
 * @param str string to parse
 * @return P2Atom containing the result
 */
P2Atom P2Asm::parse_addops(int& pos, const QString& str)
{
    P2Atom lvalue = parse_mulops(pos, str);
    const int spos = pos; Q_UNUSED(spos)
            for (;;) {

        if (!skip_spc(pos, str))
            break;

        p2_token_e op = Token.at_type(pos, str, tt_addop);
        if (t_invalid == op)
            break;

        P2Atom rvalue = parse_mulops(pos, str);
        if (!rvalue.isValid()) {
            DEBUG_EXPR(" invalid rvalue: %s", qPrintable(str.mid(spos)));
            m_error = tr("Invalid character in expression (addops): %1")
                      .arg(str.mid(spos));
            break;
        }

        p2_QUAD r = rvalue.to_quad();
        switch (op) {
        case t__ADD:
            DEBUG_EXPR(" add: %s", qPrintable(str.mid(spos)));
            lvalue.arith_add(r);
            break;
        case t__SUB:
            DEBUG_EXPR(" sub: %s", qPrintable(str.mid(spos)));
            lvalue.arith_sub(r);
            break;
        default:
            Q_ASSERT_X(false, "addops", "Invalid op");
        }
    }
    return lvalue;
}

/**
 * @brief Parse an an expression of shift operations (precedence 5)
 * @param pos position in %str where to start
 * @param str string to parse
 * @return P2Atom containing the result
 */
P2Atom P2Asm::parse_shiftops(int& pos, const QString& str)
{
    P2Atom lvalue = parse_addops(pos, str);
    const int spos = pos; Q_UNUSED(spos)
            for (;;) {

        if (!skip_spc(pos, str))
            break;

        p2_token_e op = Token.at_type(pos, str, tt_shiftop);
        if (t_invalid == op)
            break;

        P2Atom rvalue = parse_addops(pos, str);
        if (!rvalue.isValid()) {
            DEBUG_EXPR(" invalid rvalue: %s", qPrintable(str.mid(spos)));
            m_error = tr("Invalid character in expression (shiftops): %1")
                      .arg(str.mid(spos));
            break;
        }

        p2_QUAD r = rvalue.to_quad();
        switch (op) {
        case t__SHL:
            DEBUG_EXPR(" shl: %s", qPrintable(str.mid(spos)));
            lvalue.binary_shl(r);
            break;
        case t__SHR:
            DEBUG_EXPR(" shr: %s", qPrintable(str.mid(spos)));
            lvalue.binary_shr(r);
            break;
        default:
            Q_ASSERT_X(false, "shiftops", "Invalid op");
        }
    }
    return lvalue;
}

/**
 * @brief Parse an an expression of binary operations (precedence 8, 9, 10)
 * @param pos position in %str where to start
 * @param str string to parse
 * @return P2Atom containing the result
 */
P2Atom P2Asm::parse_binops(int& pos, const QString& str)
{
    P2Atom lvalue = parse_shiftops(pos, str);
    const int spos = pos; Q_UNUSED(spos)

            for (;;) {

        if (!skip_spc(pos, str))
            break;

        p2_token_e op = Token.at_type(pos, str, tm_binops);
        if (t_invalid == op)
            break;

        P2Atom rvalue = parse_shiftops(pos, str);
        if (!rvalue.isValid()) {
            DEBUG_EXPR(" invalid rvalue: %s", qPrintable(str.mid(spos)));
            m_error = tr("Invalid character in expression (binops): %1")
                      .arg(str.mid(pos));
            break;
        }

        p2_QUAD r = rvalue.to_quad();
        switch (op) {
        case t__AND:
            DEBUG_EXPR(" binary and: %s", qPrintable(str.mid(spos)));
            lvalue.binary_and(r);
            break;
        case t__XOR:
            DEBUG_EXPR(" binary xor: %s", qPrintable(str.mid(spos)));
            lvalue.binary_xor(r);
            break;
        case t__OR:
            DEBUG_EXPR(" binary or: %s", qPrintable(str.mid(spos)));
            lvalue.binary_or(r);
            break;
        default:
            Q_ASSERT_X(false, "binops", "Invalid op");
        }
    }
    return lvalue;
}

/**
 * @brief Evaluate a sub expression enclosed in parenthesis
 * @param pos position where the expression starts
 * @param str const reference to the expression string
 * @return P2Atom with the result
 */
P2Atom P2Asm::parse_subexpression(int& pos, const QString& str)
{
    P2Atom atom;

    // Find the matching closing parenthesis, skip over strings
    // Skip left parenthesis
    pos += 1;
    int level = 1;
    int epos = pos;
    bool escaped = false;
    QChar instring = QChar::Null;
    while (epos < str.length() && level > 0) {
        QChar ch = str.at(epos++);
        if (instring != QChar::Null) {
            // inside a string
            if (escaped) {
                // character was escaped: ignore
                escaped = false;
            } else if (ch == QChar('"')) {
                // character is doubleqoute: end of string
                instring = false;
            } else if (ch == str_escape) {
                // character is backslash: escape next character
                escaped = true;
            }
        } else {
            // not inside string
            if (ch == QChar('"')) {
                instring = ch;
            } else if (ch == QChar('(')) {
                level++;
            } else if (ch == QChar(')')) {
                if (0 == --level)
                    epos--;
            }
        }
    }

    if (epos >= str.length()) {
        m_error = tr("Missing closing parenthesis in expression %1")
                  .arg(str.mid(pos));
        return atom;
    }

    QString expr = str.mid(pos, epos - pos);
    pos = 0;
    DEBUG_EXPR(" sub expression: %s", qPrintable(expr));
    atom = parse_binops(pos, expr);
    pos = epos;

    // Expect right parenthesis
    if (pos < str.length()) {
        Q_ASSERT(str.at(pos) == QChar(')'));
        // Skip right parenthesis
        pos++;
    } else {
        m_error = tr("Missing closing parenthesis in expression %1")
                  .arg(str.mid(pos));
    }

    return atom;
}

/**
 * @brief Evaluate an expression
 * @param imm_to put immediate flag into: -1=nowhere (default), 0=imm, 1=wz, 2=wc
 * @return QVariant with the value of the expression
 */
P2Atom P2Asm::parse_expression(imm_to_e imm_to)
{
    P2Atom atom;
    int imm = 0;
    int amp = 0;
    int pos = 0;
    QString str = m_words.value(m_widx).str();

    if (m_widx >= m_wcnt)
        return atom;

    if (skip_spc(pos, str)) {
        while (pos < str.length() && QChar('#') == str.at(pos)) {
            ++imm;
            ++pos;
            skip_spc(pos, str);
        }

        skip_spc(pos, str);
        while (pos < str.length() && QChar('@') == str.at(pos)) {
            ++amp;
            ++pos;
            skip_spc(pos, str);
        }
    }

    atom = parse_binops(pos, str);

    // Set immediate flag, if specified
    switch (imm_to) {
    case immediate_im:
        m_IR.op.im = imm;
        break;
    case immediate_wz:
        m_IR.op.wz = imm;
        break;
    case immediate_wc:
        m_IR.op.wc = imm;
        break;
    default:
        // silence compiler warnings
        break;
    }

    m_widx++;
    return atom;
}

/**
 * @brief Expect end of line, i.e. no more parameters
 *
 * @return true on success, or false on error
 */
bool P2Asm::end_of_line()
{
    if (m_widx < m_wcnt) {
        // ignore extra parameters?
        m_error = tr("Found extra parameters: %1")
                  .arg(m_source[m_lineno].mid(m_words[m_widx].pos()));
        return false;
    }
    return true;
}

/**
 * @brief A comma is expected
 *
 * @return true if comma found, false otherwise
 */
bool P2Asm::parse_comma()
{
    if (m_widx >= m_wcnt) {
        m_error = tr("Expected %1 but found %2.")
                  .arg(Token.string(t_comma))
                  .arg(tr("end of line"));
        return false;
    }
    if (t_comma != m_words[m_widx].tok()) {
        m_error = tr("Expected %1 but found %2.")
                  .arg(Token.string(t_comma))
                  .arg(Token.string(m_words[m_widx].tok()));
        return false;
    }
    m_widx++;
    return true;
}

/**
 * @brief An optioncal comma is skipped
 *
 */
void P2Asm::optional_comma()
{
    if (m_widx >= m_wcnt)
        return;
    if (t_comma != m_words[m_widx].tok())
        return;
    m_widx++;
}

/**
 * @brief Optional WC, WZ, or WCZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::optional_wcz()
{
    while (m_widx < m_wcnt) {
        p2_token_e tok = m_words[m_widx].tok();
        switch (tok) {
        case t_WC:
            m_IR.op.wc = true;
            m_widx++;
            break;
        case t_WZ:
            m_IR.op.wz = true;
            m_widx++;
            break;
        case t_WCZ:
            m_IR.op.wc = true;
            m_IR.op.wz = true;
            m_widx++;
            break;
        case t_comma:
            // expect more flags
            m_widx++;
            break;
        default:
            m_error = tr("Unexpected flag update '%1' not %2")
                      .arg(m_words.value(m_widx).str())
                      .arg(tr("WC, WZ, or WCZ"));
            return false;
        }
    }
    return true;
}

/**
 * @brief Optional WC
 *
 * @return true on success, or false on error
 */
bool P2Asm::optional_wc()
{
    if (m_widx < m_wcnt) {
        p2_token_e tok = m_words[m_widx].tok();
        switch (tok) {
        case t_WC:
            m_IR.op.wc = true;
            m_widx++;
            break;
        default:
            m_error = tr("Unexpected flag update '%1' not %2")
                      .arg(m_words.value(m_widx).str()
                           .arg(tr("WC")));
            return false;
        }
    }
    return true;
}

/**
 * @brief Optional WZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::optional_wz()
{
    if (m_widx < m_wcnt) {
        p2_token_e tok = m_words[m_widx].tok();
        switch (tok) {
        case t_WZ:
            m_IR.op.wz = true;
            m_widx++;
            break;
        default:
            m_error = tr("Unexpected flag update '%1' not %2")
                      .arg(m_words.value(m_widx).str())
                      .arg(tr("WZ"));
            return false;
        }
    }
    return true;
}

/**
 * @brief Assignment operation
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_assign()
{
    m_widx++;           // skip over token
    m_advance = 0;      // No PC advancing
    m_emit_IR = false;
    P2Atom atom = parse_expression();
    const p2_LONG value = atom.to_long();
    m_IR.opcode = value;
    m_symbols.setValue(m_symbol, value);
    return end_of_line();
}

/**
 * @brief Origin operation
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_org()
{
    m_widx++;           // skip over token
    m_advance = 0;      // No PC advancing
    m_emit_IR = false;
    P2Atom atom = parse_expression();
    p2_LONG value = atom.to_long();
    if (atom.isEmpty())
        value = m_last_PC;
    m_curr_PC = m_next_PC = value;
    m_symbols.setValue(m_symbol, value);
    return end_of_line();
}

/**
 * @brief Origin high operation
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_orgh()
{
    m_widx++;           // skip over token
    m_advance = 0;      // No PC advancing
    m_emit_IR = false;
    P2Atom atom = parse_expression();
    p2_LONG value = atom.to_long();
    if (atom.isEmpty())
        value = HUB_ADDR;
    m_next_PC =  m_last_PC = value;
    m_symbols.setValue(m_symbol, value);
    return end_of_line();
}

/**
 * @brief Switch to data section
 * @return true on success
 */
bool P2Asm::asm_dat()
{
    m_advance = 0;
    m_emit_IR = false;
    m_section = sec_dat;
    return true;
}

/**
 * @brief Switch to constant section
 * @return true on success
 */
bool P2Asm::asm_con()
{
    m_advance = 0;
    m_emit_IR = false;
    m_section = sec_con;
    return true;
}

/**
 * @brief Switch to publics section
 * @return true on success
 */
bool P2Asm::asm_pub()
{
    m_advance = 0;
    m_emit_IR = false;
    m_section = sec_pub;
    return true;
}

/**
 * @brief Switch to private section
 * @return true on success
 */
bool P2Asm::asm_pri()
{
    m_advance = 0;
    m_emit_IR = false;
    m_section = sec_pri;
    return true;
}

/**
 * @brief Switch to variable section
 * @return true on success
 */
bool P2Asm::asm_var()
{
    m_advance = 0;
    m_emit_IR = false;
    m_section = sec_var;
    return true;
}

/**
 * @brief Expect no more parameters after instruction
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_inst()
{
    return end_of_line();
}

/**
 * @brief Expect parameters for D and {#}S
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_d_imm_s()
{
    P2Atom dst = parse_expression();
    if (!parse_comma())
        return false;
    P2Atom src = parse_expression(immediate_im);
    m_IR.op.dst = dst.to_long();
    m_IR.op.src = src.to_long();
    return end_of_line();
}

/**
 * @brief Expect parameters for D and optional WC, WZ, or WCZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_d_cz()
{
    P2Atom dst = parse_expression();
    m_IR.op.dst = dst.to_long();
    optional_wcz();
    return end_of_line();
}

/**
 * @brief Expect optional WC, WZ, or WCZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_cz()
{
    optional_wcz();
    return end_of_line();
}

/**
 * @brief Expect conditional for C, conditional for Z, and optional WC, WZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_cccc_zzzz_wcz()
{
    p2_cond_e cccc = parse_modcz(m_words[m_widx].tok());
    if (!parse_comma())
        return false;
    p2_cond_e zzzz = parse_modcz(m_words[m_widx].tok());
    m_IR.op.dst = static_cast<p2_LONG>((cccc << 4) | zzzz);
    optional_wcz();
    return end_of_line();
}

/**
 * @brief Expect parameters for D
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_d()
{
    P2Atom dst = parse_expression();
    m_IR.op.dst = dst.to_long();
    return end_of_line();
}

/**
 * @brief Expect parameters for {#}D setting WZ for immediate
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_wz_d()
{
    P2Atom dst = parse_expression(immediate_wz);
    m_IR.op.dst = dst.to_long();
    return end_of_line();
}

/**
 * @brief Expect parameters for {#}D
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_imm_d()
{
    P2Atom dst = parse_expression(immediate_im);
    m_IR.op.dst = dst.to_long();
    return end_of_line();
}

/**
 * @brief Expect parameters for {#}D, and optional WC, WZ, or WCZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_imm_d_wcz()
{
    P2Atom dst = parse_expression(immediate_im);
    m_IR.op.dst = dst.to_long();
    optional_wcz();
    return end_of_line();
}

/**
 * @brief Expect parameters for {#}D, and optional WC
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_imm_d_wc()
{
    P2Atom dst = parse_expression(immediate_im);
    m_IR.op.dst = dst.to_long();
    optional_wc();
    return end_of_line();
}

/**
 * @brief Expect parameters for D, and {#}S, and optional WC, WZ, or WCZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_d_imm_s_wcz()
{
    P2Atom dst = parse_expression();
    if (!parse_comma())
        return false;
    P2Atom src = parse_expression(immediate_im);
    m_IR.op.dst = dst.to_long();
    m_IR.op.src = src.to_long();
    optional_wcz();
    return end_of_line();
}

/**
 * @brief Expect parameters for D, and {#}S, and optional WC
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_d_imm_s_wc()
{
    P2Atom dst = parse_expression();
    if (!parse_comma())
        return false;
    P2Atom src = parse_expression(immediate_im);
    m_IR.op.dst = dst.to_long();
    m_IR.op.src = src.to_long();
    optional_wc();
    return end_of_line();
}

/**
 * @brief Expect parameters for D, and {#}S, and optional WZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_d_imm_s_wz()
{
    P2Atom dst = parse_expression();
    if (!parse_comma())
        return false;
    P2Atom src = parse_expression(immediate_im);
    m_IR.op.dst = dst.to_long();
    m_IR.op.src = src.to_long();
    optional_wz();
    return end_of_line();
}

/**
 * @brief Expect parameters for {#}D, and {#}S
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_wz_d_imm_s()
{
    P2Atom dst = parse_expression(immediate_wz);
    if (!parse_comma())
        return false;
    P2Atom src = parse_expression(immediate_im);
    m_IR.op.dst = dst.to_long();
    m_IR.op.src = src.to_long();
    return end_of_line();
}

/**
 * @brief Expect parameters for D, and {#}S, and #N (0 … 7)
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_d_imm_s_nnn(uint max)
{
    P2Atom dst = parse_expression();
    if (!parse_comma())
        return false;
    P2Atom src = parse_expression(immediate_im);
    m_IR.op.dst = dst.to_long();
    m_IR.op.src = src.to_long();
    if (parse_comma()) {
        if (m_widx < m_wcnt) {
            P2Atom n = parse_expression();
            if (n.isEmpty()) {
                m_error = tr("Expected immediate #n");
                return false;
            }
            if (n.to_long() > max) {
                m_error = tr("Immediate #n not in 0-%1 (%2)")
                          .arg(max)
                          .arg(n.to_long());
                return false;
            }
            p2_LONG opcode = static_cast<p2_LONG>(n.to_long() & max) << 18;
            m_IR.opcode |= opcode;
        } else {
            m_error = tr("Missing immediate #n");
            return false;
        }
    } else {
        // Default is 0
    }
    return end_of_line();
}

/**
 * @brief Expect parameters for {#}S
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_imm_s()
{
    P2Atom src = parse_expression(immediate_im);
    m_IR.op.src = src.to_long();
    return end_of_line();
}

/**
 * @brief Expect parameters for {#}S, and optional WC
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_imm_s_wcz()
{
    P2Atom src = parse_expression(immediate_im);
    m_IR.op.src = src.to_long();
    return end_of_line();
}


/**
 * @brief Expect parameters for #AAAAAAAAAAAA (19 bit address for CALL/CALLA/CALLB/LOC)
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_ptr_pc_abs()
{
    p2_token_e dst = m_words[m_widx].tok();
    switch (dst) {
    case t_PA:
        break;
    case t_PB:
        m_IR.op.wz = true;
        break;
    case t_PTRA:
        m_IR.op.wc = true;
        break;
    case t_PTRB:
        m_IR.op.wz = true;
        m_IR.op.wc = true;
        break;
    default:
        m_error = tr("Invalid pointer parameter: %1")
                  .arg(m_source[m_lineno].mid(m_words[m_widx].pos()));
        return false;
    }
    m_widx++;
    if (!parse_comma())
        return false;

    P2Atom atom = parse_expression();
    p2_LONG addr = atom.to_long();
    m_IR.opcode |= addr & A20MASK;

    return end_of_line();
}

/**
 * @brief Expect parameters for #AAAAAAAAAAAA (19 bit address for CALL/CALLA/CALLB)
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_pc_abs()
{
    P2Atom atom = parse_expression();
    p2_LONG addr = atom.to_long();
    m_IR.opcode |= addr & A20MASK;

    return end_of_line();
}

/**
 * @brief Expect parameters for #AAAAAAAAAAAA (23 bit address for AUGD/AUGS)
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_imm23(QVector<p2_inst7_e> aug)
{
    P2Atom atom = parse_expression();
    p2_LONG addr = atom.to_long();
    m_IR.op.inst = static_cast<p2_inst7_e>(m_IR.op.inst | aug[(addr >> 21) & 3]);

    return end_of_line();
}

/**
 * @brief Expect one or more bytes of data
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_byte()
{
    m_widx++;
    m_advance = 0;      // No PC advancing, as it's done based on m_data
    m_emit_IR = false;
    while (m_widx < m_wcnt) {
        P2Atom atom = parse_expression();
        p2_BYTE b = atom.to_byte();
        m_data.append(P2Atom::Byte, b);
        optional_comma();
    }
    return end_of_line();
}

/**
 * @brief Expect one or more bytes of data
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_word()
{
    m_widx++;
    m_advance = 0;      // No PC advancing, as it's done based on m_data
    m_emit_IR = false;
    while (m_widx < m_wcnt) {
        P2Atom atom = parse_expression();
        p2_WORD w = atom.to_word();
        m_data.append(P2Atom::Word, w);
        optional_comma();
    }
    // TODO: put data into output
    return end_of_line();
}

/**
 * @brief Expect one or more bytes of data
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_long()
{
    m_widx++;
    m_advance = 0;      // No PC advancing, as it's done based on m_data
    m_emit_IR = false;
    while (m_widx < m_wcnt) {
        P2Atom atom = parse_expression();
        p2_LONG l = atom.to_long();
        m_data.append(P2Atom::Long, l);
        optional_comma();
    }
    // TODO: put data into output
    return end_of_line();
}

/**
 * @brief Expect one or more bytes of data
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_res()
{
    m_widx++;
    m_advance = 0;      // No PC advancing if no value is specified
    m_emit_IR = false;
    while (m_widx < m_wcnt) {
        P2Atom atom = parse_expression();
        m_advance += atom.to_long();
        optional_comma();
    }
    return end_of_line();
}

/**
 * @brief Expect one or more filenames in double quotes
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_file()
{
    m_widx++;
    m_advance = 0;      // No PC advancing, as it's done based on m_data
    m_emit_IR = false;
    while (m_widx < m_wcnt) {
        P2Atom atom = parse_expression();
        QString filename = atom.to_string();
        QFile data(filename);
        if (data.open(QIODevice::ReadOnly)) {
            m_data.append(data.readAll());
            data.close();
        } else {
            m_error = tr("Could not open file %1 for reading.").arg(filename);
            return false;
        }
        optional_comma();
    }
    return end_of_line();

}

/**
 * @brief Expect an address which is checked for being < curr_pc
 * @return true on success, or false on error
 */
bool P2Asm::asm_fit()
{
    m_widx++;
    P2Atom atom = parse_expression();
    const p2_LONG fit = atom.to_long();
    if (fit < m_curr_PC) {
        m_error = tr("Code does not fit below $%1 (origin == $%2)")
                  .arg(fit, 0, 16)
                  .arg(m_curr_PC, 0, 16);
    }
    return end_of_line();
}

/**
 * @brief No operation.
 *
 * 0000 0000000 000 000000000 000000000
 *
 * NOP
 */
bool P2Asm::asm_nop()
{
    m_widx++;
    m_IR.opcode = 0;
    return parse_inst();
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
 */
bool P2Asm::asm_ror()
{
    m_widx++;
    m_IR.op.inst = p2_ROR;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_rol()
{
    m_widx++;
    m_IR.op.inst = p2_ROR;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_shr()
{
    m_widx++;
    m_IR.op.inst = p2_SHR;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_shl()
{
    m_widx++;
    m_IR.op.inst = p2_SHL;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_rcr()
{
    m_widx++;
    m_IR.op.inst = p2_RCR;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_rcl()
{
    m_widx++;
    m_IR.op.inst = p2_RCL;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_sar()
{
    m_widx++;
    m_IR.op.inst = p2_SAR;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_sal()
{
    m_widx++;
    m_IR.op.inst = p2_SAL;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_add()
{
    m_widx++;
    m_IR.op.inst = p2_ADD;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_addx()
{
    m_widx++;
    m_IR.op.inst = p2_ADDX;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_adds()
{
    m_widx++;
    m_IR.op.inst = p2_ADDS;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_addsx()
{
    m_widx++;
    m_IR.op.inst = p2_ADDSX;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_sub()
{
    m_widx++;
    m_IR.op.inst = p2_SUB;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_subx()
{
    m_widx++;
    m_IR.op.inst = p2_SUBX;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_subs()
{
    m_widx++;
    m_IR.op.inst = p2_SUBS;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_subsx()
{
    m_widx++;
    m_IR.op.inst = p2_SUBSX;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_cmp()
{
    m_widx++;
    m_IR.op.inst = p2_CMP;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_cmpx()
{
    m_widx++;
    m_IR.op.inst = p2_CMPX;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_cmps()
{
    m_widx++;
    m_IR.op.inst = p2_CMPS;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_cmpsx()
{
    m_widx++;
    m_IR.op.inst = p2_CMPSX;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_cmpr()
{
    m_widx++;
    m_IR.op.inst = p2_CMPR;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_cmpm()
{
    m_widx++;
    m_IR.op.inst = p2_CMPM;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_subr()
{
    m_widx++;
    m_IR.op.inst = p2_SUBR;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_cmpsub()
{
    m_widx++;
    m_IR.op.inst = p2_CMPSUB;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_fge()
{
    m_widx++;
    m_IR.op.inst = p2_FGE;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_fle()
{
    m_widx++;
    m_IR.op.inst = p2_FLE;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_fges()
{
    m_widx++;
    m_IR.op.inst = p2_FGES;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_fles()
{
    m_widx++;
    m_IR.op.inst = p2_FLES;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_sumc()
{
    m_widx++;
    m_IR.op.inst = p2_SUMC;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_sumnc()
{
    m_widx++;
    m_IR.op.inst = p2_SUMNC;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_sumz()
{
    m_widx++;
    m_IR.op.inst = p2_SUMZ;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_sumnz()
{
    m_widx++;
    m_IR.op.inst = p2_SUMNZ;
    return parse_d_imm_s_wcz();
}

/**
 * @brief Test bit S[4:0] of  D, write to C/Z.
 *
 * EEEE 0100000 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTB   D,{#}S         WC/WZ
 *
 * C/Z =          D[S[4:0]].
 */
bool P2Asm::asm_testb_w()
{
    m_widx++;
    m_IR.op.inst = p2_TESTB_W;
    return parse_d_imm_s_wcz();
}

/**
 * @brief Test bit S[4:0] of !D, write to C/Z.
 *
 * EEEE 0100001 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTBN  D,{#}S         WC/WZ
 *
 * C/Z =         !D[S[4:0]].
 */
bool P2Asm::asm_testbn_w()
{
    m_widx++;
    m_IR.op.inst = p2_TESTBN_W;
    return parse_d_imm_s_wcz();
}

/**
 * @brief Test bit S[4:0] of  D, AND into C/Z.
 *
 * EEEE 0100010 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTB   D,{#}S     ANDC/ANDZ
 *
 * C/Z = C/Z AND  D[S[4:0]].
 */
bool P2Asm::asm_testb_and()
{
    m_widx++;
    m_IR.op.inst = p2_TESTB_AND;
    return parse_d_imm_s_wcz();
}

/**
 * @brief Test bit S[4:0] of !D, AND into C/Z.
 *
 * EEEE 0100011 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTBN  D,{#}S     ANDC/ANDZ
 *
 * C/Z = C/Z AND !D[S[4:0]].
 */
bool P2Asm::asm_testbn_and()
{
    m_widx++;
    m_IR.op.inst = p2_TESTBN_AND;
    return parse_d_imm_s_wcz();
}

/**
 * @brief Test bit S[4:0] of  D, OR  into C/Z.
 *
 * EEEE 0100100 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTB   D,{#}S       ORC/ORZ
 *
 * C/Z = C/Z OR   D[S[4:0]].
 */
bool P2Asm::asm_testb_or()
{
    m_widx++;
    m_IR.op.inst = p2_TESTB_OR;
    return parse_d_imm_s_wcz();
}

/**
 * @brief Test bit S[4:0] of !D, OR  into C/Z.
 *
 * EEEE 0100101 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTBN  D,{#}S       ORC/ORZ
 *
 * C/Z = C/Z OR  !D[S[4:0]].
 */
bool P2Asm::asm_testbn_or()
{
    m_widx++;
    m_IR.op.inst = p2_TESTBN_OR;
    return parse_d_imm_s_wcz();
}

/**
 * @brief Test bit S[4:0] of  D, XOR into C/Z.
 *
 * EEEE 0100110 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTB   D,{#}S     XORC/XORZ
 *
 * C/Z = C/Z XOR  D[S[4:0]].
 */
bool P2Asm::asm_testb_xor()
{
    m_widx++;
    m_IR.op.inst = p2_TESTB_XOR;
    return parse_d_imm_s_wcz();
}

/**
 * @brief Test bit S[4:0] of !D, XOR into C/Z.
 *
 * EEEE 0100111 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTBN  D,{#}S     XORC/XORZ
 *
 * C/Z = C/Z XOR !D[S[4:0]].
 */
bool P2Asm::asm_testbn_xor()
{
    m_widx++;
    m_IR.op.inst = p2_TESTBN_XOR;
    return parse_d_imm_s_wcz();
}

/**
 * @brief Bit S[4:0] of D = 0,    C,Z = D[S[4:0]].
 *
 * EEEE 0100000 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITL    D,{#}S         {WCZ}
 */
bool P2Asm::asm_bitl()
{
    m_widx++;
    m_IR.op.inst = p2_BITL;
    return parse_d_imm_s_wcz();
}

/**
 * @brief Bit S[4:0] of D = 1,    C,Z = D[S[4:0]].
 *
 * EEEE 0100001 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITH    D,{#}S         {WCZ}
 */
bool P2Asm::asm_bith()
{
    m_widx++;
    m_IR.op.inst = p2_BITH;
    return parse_d_imm_s_wcz();
}

/**
 * @brief Bit S[4:0] of D = C,    C,Z = D[S[4:0]].
 *
 * EEEE 0100010 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITC    D,{#}S         {WCZ}
 */
bool P2Asm::asm_bitc()
{
    m_widx++;
    m_IR.op.inst = p2_BITC;
    return parse_d_imm_s_wcz();
}

/**
 * @brief Bit S[4:0] of D = !C,   C,Z = D[S[4:0]].
 *
 * EEEE 0100011 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNC   D,{#}S         {WCZ}
 */
bool P2Asm::asm_bitnc()
{
    m_widx++;
    m_IR.op.inst = p2_BITNC;
    return parse_d_imm_s_wcz();
}

/**
 * @brief Bit S[4:0] of D = Z,    C,Z = D[S[4:0]].
 *
 * EEEE 0100100 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITZ    D,{#}S         {WCZ}
 */
bool P2Asm::asm_bitz()
{
    m_widx++;
    m_IR.op.inst = p2_BITZ;
    return parse_d_imm_s_wcz();
}

/**
 * @brief Bit S[4:0] of D = !Z,   C,Z = D[S[4:0]].
 *
 * EEEE 0100101 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNZ   D,{#}S         {WCZ}
 */
bool P2Asm::asm_bitnz()
{
    m_widx++;
    m_IR.op.inst = p2_BITNZ;
    return parse_d_imm_s_wcz();
}

/**
 * @brief Bit S[4:0] of D = RND,  C,Z = D[S[4:0]].
 *
 * EEEE 0100110 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITRND  D,{#}S         {WCZ}
 */
bool P2Asm::asm_bitrnd()
{
    m_widx++;
    m_IR.op.inst = p2_BITRND;
    return parse_d_imm_s_wcz();
}

/**
 * @brief Bit S[4:0] of D = !bit, C,Z = D[S[4:0]].
 *
 * EEEE 0100111 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNOT  D,{#}S         {WCZ}
 */
bool P2Asm::asm_bitnot()
{
    m_widx++;
    m_IR.op.inst = p2_BITNOT;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_and()
{
    m_widx++;
    m_IR.op.inst = p2_AND;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_andn()
{
    m_widx++;
    m_IR.op.inst = p2_ANDN;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_or()
{
    m_widx++;
    m_IR.op.inst = p2_OR;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_xor()
{
    m_widx++;
    m_IR.op.inst = p2_XOR;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_muxc()
{
    m_widx++;
    m_IR.op.inst = p2_MUXC;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_muxnc()
{
    m_widx++;
    m_IR.op.inst = p2_MUXNC;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_muxz()
{
    m_widx++;
    m_IR.op.inst = p2_MUXZ;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_muxnz()
{
    m_widx++;
    m_IR.op.inst = p2_MUXNZ;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_mov()
{
    m_widx++;
    m_IR.op.inst = p2_MOV;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_not()
{
    m_widx++;
    m_IR.op.inst = p2_NOT;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_abs()
{
    m_widx++;
    m_IR.op.inst = p2_ABS;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_neg()
{
    m_widx++;
    m_IR.op.inst = p2_NEG;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_negc()
{
    m_widx++;
    m_IR.op.inst = p2_NEGC;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_negnc()
{
    m_widx++;
    m_IR.op.inst = p2_NEGNC;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_negz()
{
    m_widx++;
    m_IR.op.inst = p2_NEGZ;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_negnz()
{
    m_widx++;
    m_IR.op.inst = p2_NEGNZ;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_incmod()
{
    m_widx++;
    m_IR.op.inst = p2_INCMOD;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_decmod()
{
    m_widx++;
    m_IR.op.inst = p2_DECMOD;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_zerox()
{
    m_widx++;
    m_IR.op.inst = p2_ZEROX;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_signx()
{
    m_widx++;
    m_IR.op.inst = p2_SIGNX;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_encod()
{
    m_widx++;
    m_IR.op.inst = p2_ENCOD;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_ones()
{
    m_widx++;
    m_IR.op.inst = p2_ONES;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_test()
{
    m_widx++;
    m_IR.op.inst = p2_TEST;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_testn()
{
    m_widx++;
    m_IR.op.inst = p2_TESTN;
    return parse_d_imm_s_wcz();
}

/**
 * @brief Set S[3:0] into nibble N in D, keeping rest of D same.
 *
 * EEEE 100000N NNI DDDDDDDDD SSSSSSSSS
 *
 * SETNIB  D,{#}S,#N
 */
bool P2Asm::asm_setnib()
{
    if (left() < 3)
        return asm_setnib_altsn();
    m_widx++;
    m_IR.op.inst = p2_SETNIB_0;
    return parse_d_imm_s_nnn();
}

/**
 * @brief Set S[3:0] into nibble established by prior ALTSN instruction.
 *
 * EEEE 1000000 00I 000000000 SSSSSSSSS
 *
 * SETNIB  {#}S
 */
bool P2Asm::asm_setnib_altsn()
{
    m_widx++;
    m_IR.op.inst = p2_SETNIB_0;
    return parse_imm_s();
}

/**
 * @brief Get nibble N of S into D.
 *
 * EEEE 100001N NNI DDDDDDDDD SSSSSSSSS
 *
 * GETNIB  D,{#}S,#N
 *
 * D = {28'b0, S.NIBBLE[N]).
 */
bool P2Asm::asm_getnib()
{
    if (left() < 3)
        return asm_getnib_altgn();
    m_widx++;
    m_IR.op.inst = p2_GETNIB_0;
    return parse_d_imm_s_nnn();
}

/**
 * @brief Get nibble established by prior ALTGN instruction into D.
 *
 * EEEE 1000010 000 DDDDDDDDD 000000000
 *
 * GETNIB  D
 */
bool P2Asm::asm_getnib_altgn()
{
    m_IR.op.inst = p2_GETNIB_0;
    return parse_imm_s();
}

/**
 * @brief Rotate-left nibble N of S into D.
 *
 * EEEE 100010N NNI DDDDDDDDD SSSSSSSSS
 *
 * ROLNIB  D,{#}S,#N
 *
 * D = {D[27:0], S.NIBBLE[N]).
 */
bool P2Asm::asm_rolnib()
{
    if (left() < 3)
        return asm_rolnib_altgn();
    m_widx++;
    m_IR.op.inst = p2_ROLNIB_0;
    return parse_d_imm_s_nnn();
}

/**
 * @brief Rotate-left nibble established by prior ALTGN instruction into D.
 *
 * EEEE 1000100 000 DDDDDDDDD 000000000
 *
 * ROLNIB  D
 */
bool P2Asm::asm_rolnib_altgn()
{
    m_widx++;
    m_IR.op.inst = p2_ROLNIB_0;
    return parse_d();
}

/**
 * @brief Set S[7:0] into byte N in D, keeping rest of D same.
 *
 * EEEE 1000110 NNI DDDDDDDDD SSSSSSSSS
 *
 * SETBYTE D,{#}S,#N
 */
bool P2Asm::asm_setbyte()
{
    if (left() < 3)
        return asm_setbyte_altsb();
    m_widx++;
    m_IR.op.inst = p2_SETBYTE;
    return parse_d_imm_s_nnn();
}

/**
 * @brief Set S[7:0] into byte established by prior ALTSB instruction.
 *
 * EEEE 1000110 00I 000000000 SSSSSSSSS
 *
 * SETBYTE {#}S
 */
bool P2Asm::asm_setbyte_altsb()
{
    m_widx++;
    m_IR.op.inst = p2_SETBYTE;
    return parse_imm_s();
}

/**
 * @brief Get byte N of S into D.
 *
 * EEEE 1000111 NNI DDDDDDDDD SSSSSSSSS
 *
 * GETBYTE D,{#}S,#N
 *
 * D = {24'b0, S.BYTE[N]).
 */
bool P2Asm::asm_getbyte()
{
    if (left() < 3)
        return asm_getbyte_altgb();
    m_widx++;
    m_IR.op.inst = p2_GETBYTE;
    return parse_d_imm_s_nnn();
}

/**
 * @brief Get byte established by prior ALTGB instruction into D.
 *
 * EEEE 1000111 000 DDDDDDDDD 000000000
 *
 * GETBYTE D
 */
bool P2Asm::asm_getbyte_altgb()
{
    m_widx++;
    m_IR.op.inst = p2_GETBYTE;
    return parse_d();
}

/**
 * @brief Rotate-left byte N of S into D.
 *
 * EEEE 1001000 NNI DDDDDDDDD SSSSSSSSS
 *
 * ROLBYTE D,{#}S,#N
 *
 * D = {D[23:0], S.BYTE[N]).
 */
bool P2Asm::asm_rolbyte()
{
    if (left() < 3)
        return asm_rolbyte_altgb();
    m_widx++;
    m_IR.op.inst = p2_ROLBYTE;
    return parse_d_imm_s_nnn(3);
}

/**
 * @brief Rotate-left byte established by prior ALTGB instruction into D.
 *
 * EEEE 1001000 000 DDDDDDDDD 000000000
 *
 * ROLBYTE D
 */
bool P2Asm::asm_rolbyte_altgb()
{
    m_widx++;
    m_IR.op.inst = p2_ROLBYTE;
    return parse_d();
}

/**
 * @brief Set S[15:0] into word N in D, keeping rest of D same.
 *
 * EEEE 1001001 0NI DDDDDDDDD SSSSSSSSS
 *
 * SETWORD D,{#}S,#N
 */
bool P2Asm::asm_setword()
{
    if (left() < 3)
        return asm_setword_altsw();
    m_widx++;
    m_IR.op9.inst = p2_SETWORD;
    return parse_d_imm_s_nnn(1);
}

/**
 * @brief Set S[15:0] into word established by prior ALTSW instruction.
 *
 * EEEE 1001001 00I 000000000 SSSSSSSSS
 *
 * SETWORD {#}S
 */
bool P2Asm::asm_setword_altsw()
{
    m_widx++;
    m_IR.op9.inst = p2_SETWORD_ALTSW;
    return parse_imm_s();
}

/**
 * @brief Get word N of S into D.
 *
 * EEEE 1001001 1NI DDDDDDDDD SSSSSSSSS
 *
 * GETWORD D,{#}S,#N
 *
 * D = {16'b0, S.WORD[N]).
 */
bool P2Asm::asm_getword()
{
    if (left() < 3)
        return asm_getword_altgw();
    m_widx++;
    m_IR.op9.inst = p2_GETWORD;
    return parse_d_imm_s_nnn(1);
}

/**
 * @brief Get word established by prior ALTGW instruction into D.
 *
 * EEEE 1001001 100 DDDDDDDDD 000000000
 *
 * GETWORD D
 */
bool P2Asm::asm_getword_altgw()
{
    m_widx++;
    m_IR.op9.inst = p2_GETWORD_ALTGW;
    return parse_d();
}

/**
 * @brief Rotate-left word N of S into D.
 *
 * EEEE 1001010 0NI DDDDDDDDD SSSSSSSSS
 *
 * ROLWORD D,{#}S,#N
 *
 * D = {D[15:0], S.WORD[N]).
 */
bool P2Asm::asm_rolword()
{
    if (left() < 3)
        return asm_rolword_altgw();
    m_widx++;
    m_IR.op9.inst = p2_ROLWORD;
    return parse_d_imm_s_nnn(1);
}

/**
 * @brief Rotate-left word established by prior ALTGW instruction into D.
 *
 * EEEE 1001010 000 DDDDDDDDD 000000000
 *
 * ROLWORD D
 */
bool P2Asm::asm_rolword_altgw()
{
    m_widx++;
    m_IR.op9.inst = p2_ROLWORD_ALTGW;
    return parse_d();
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
 */
bool P2Asm::asm_altsn()
{
    if (left() < 3)
        return asm_altsn_d();
    m_widx++;
    m_IR.op9.inst = p2_ALTSN;
    return parse_d_imm_s();
}

/**
 * @brief Alter subsequent SETNIB instruction.
 *
 * EEEE 1001010 101 DDDDDDDDD 000000000
 *
 * ALTSN   D
 *
 * Next D field = D[11:3], N field = D[2:0].
 */
bool P2Asm::asm_altsn_d()
{
    m_widx++;
    m_IR.op9.inst = p2_ALTSN;
    return parse_d();
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
 */
bool P2Asm::asm_altgn()
{
    if (left() < 3)
        return asm_altgn_d();
    m_widx++;
    m_IR.op9.inst = p2_ALTGN;
    return parse_d_imm_s();
}

/**
 * @brief Alter subsequent GETNIB/ROLNIB instruction.
 *
 * EEEE 1001010 111 DDDDDDDDD 000000000
 *
 * ALTGN   D
 *
 * Next S field = D[11:3], N field = D[2:0].
 */
bool P2Asm::asm_altgn_d()
{
    m_widx++;
    m_IR.op9.inst = p2_ALTGN;
    return parse_d();
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
 */
bool P2Asm::asm_altsb()
{
    if (left() < 3)
        return asm_altsb_d();
    m_widx++;
    m_IR.op9.inst = p2_ALTSB;
    return parse_d_imm_s();
}

/**
 * @brief Alter subsequent SETBYTE instruction.
 *
 * EEEE 1001011 001 DDDDDDDDD 000000000
 *
 * ALTSB   D
 *
 * Next D field = D[10:2], N field = D[1:0].
 */
bool P2Asm::asm_altsb_d()
{
    m_widx++;
    m_IR.op9.inst = p2_ALTSB;
    return parse_d();
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
 */
bool P2Asm::asm_altgb()
{
    if (left() < 3)
        return asm_altgb_d();
    m_widx++;
    m_IR.op9.inst = p2_ALTGB;
    return parse_d_imm_s();
}

/**
 * @brief Alter subsequent GETBYTE/ROLBYTE instruction.
 *
 * EEEE 1001011 011 DDDDDDDDD 000000000
 *
 * ALTGB   D
 *
 * Next S field = D[10:2], N field = D[1:0].
 */
bool P2Asm::asm_altgb_d()
{
    m_widx++;
    m_IR.op9.inst = p2_ALTGB;
    return parse_d();
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
 */
bool P2Asm::asm_altsw()
{
    if (left() < 3)
        return asm_altsw_d();
    m_widx++;
    m_IR.op9.inst = p2_ALTSW;
    return parse_d_imm_s();
}

/**
 * @brief Alter subsequent SETWORD instruction.
 *
 * EEEE 1001011 101 DDDDDDDDD 000000000
 *
 * ALTSW   D
 *
 * Next D field = D[9:1], N field = D[0].
 */
bool P2Asm::asm_altsw_d()
{
    m_widx++;
    m_IR.op9.inst = p2_ALTSW;
    return parse_d();
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
 */
bool P2Asm::asm_altgw()
{
    if (left() < 3)
        return asm_altgw_d();
    m_widx++;
    m_IR.op9.inst = p2_ALTGW;
    return parse_d_imm_s();
}

/**
 * @brief Alter subsequent GETWORD/ROLWORD instruction.
 *
 * EEEE 1001011 111 DDDDDDDDD 000000000
 *
 * ALTGW   D
 *
 * Next S field = D[9:1], N field = D[0].
 */
bool P2Asm::asm_altgw_d()
{
    m_widx++;
    m_IR.op9.inst = p2_ALTGW;
    return parse_d();
}

/**
 * @brief Alter result register address (normally D field) of next instruction to (D + S) & $1FF.
 *
 * EEEE 1001100 00I DDDDDDDDD SSSSSSSSS
 *
 * ALTR    D,{#}S
 *
 * D += sign-extended S[17:9].
 */
bool P2Asm::asm_altr()
{
    if (left() < 3)
        return asm_altr_d();
    m_widx++;
    m_IR.op9.inst = p2_ALTR;
    return parse_d_imm_s();
}

/**
 * @brief Alter result register address (normally D field) of next instruction to D[8:0].
 *
 * EEEE 1001100 001 DDDDDDDDD 000000000
 *
 * ALTR    D
 */
bool P2Asm::asm_altr_d()
{
    m_widx++;
    m_IR.op9.inst = p2_ALTD;
    return parse_d();
}

/**
 * @brief Alter D field of next instruction to (D + S) & $1FF.
 *
 * EEEE 1001100 01I DDDDDDDDD SSSSSSSSS
 *
 * ALTD    D,{#}S
 *
 * D += sign-extended S[17:9].
 */
bool P2Asm::asm_altd()
{
    if (left() < 3)
        return asm_altd_d();
    m_widx++;
    m_IR.op9.inst = p2_ALTD;
    return parse_d_imm_s();
}

/**
 * @brief Alter D field of next instruction to D[8:0].
 *
 * EEEE 1001100 011 DDDDDDDDD 000000000
 *
 * ALTD    D
 */
bool P2Asm::asm_altd_d()
{
    m_widx++;
    m_IR.op9.inst = p2_ALTD;
    return parse_d();
}

/**
 * @brief Alter S field of next instruction to (D + S) & $1FF.
 *
 * EEEE 1001100 10I DDDDDDDDD SSSSSSSSS
 *
 * ALTS    D,{#}S
 *
 * D += sign-extended S[17:9].
 */
bool P2Asm::asm_alts()
{
    if (left() < 3)
        return asm_alts_d();
    m_widx++;
    m_IR.op9.inst = p2_ALTS;
    return parse_d_imm_s();
}

/**
 * @brief Alter S field of next instruction to D[8:0].
 *
 * EEEE 1001100 101 DDDDDDDDD 000000000
 *
 * ALTS    D
 */
bool P2Asm::asm_alts_d()
{
    m_widx++;
    m_IR.op9.inst = p2_ALTS;
    return parse_d();
}

/**
 * @brief Alter D field of next instruction to (D[13:5] + S) & $1FF.
 *
 * EEEE 1001100 11I DDDDDDDDD SSSSSSSSS
 *
 * ALTB    D,{#}S
 *
 * D += sign-extended S[17:9].
 */
bool P2Asm::asm_altb()
{
    if (left() < 3)
        return asm_altb_d();
    m_widx++;
    m_IR.op9.inst = p2_ALTB;
    return parse_d_imm_s();
}

/**
 * @brief Alter D field of next instruction to D[13:5].
 *
 * EEEE 1001100 111 DDDDDDDDD 000000000
 *
 * ALTB    D
 */
bool P2Asm::asm_altb_d()
{
    m_widx++;
    m_IR.op9.inst = p2_ALTB;
    return parse_d();
}

/**
 * @brief Substitute next instruction's I/R/D/S fields with fields from D, per S.
 *
 * EEEE 1001101 00I DDDDDDDDD SSSSSSSSS
 *
 * ALTI    D,{#}S
 *
 * Modify D per S.
 */
bool P2Asm::asm_alti()
{
    if (left() < 3)
        return asm_alti_d();
    m_widx++;
    m_IR.op9.inst = p2_ALTI;
    return parse_d_imm_s();
}

/**
 * @brief Execute D in place of next instruction.
 *
 * EEEE 1001101 001 DDDDDDDDD 101100100
 *
 * ALTI    D
 *
 * D stays same.
 */
bool P2Asm::asm_alti_d()
{
    m_widx++;
    m_IR.op9.inst = p2_ALTI;
    return parse_d();
}

/**
 * @brief Set R field of D to S[8:0].
 *
 * EEEE 1001101 01I DDDDDDDDD SSSSSSSSS
 *
 * SETR    D,{#}S
 *
 * D = {D[31:28], S[8:0], D[18:0]}.
 */
bool P2Asm::asm_setr()
{
    m_widx++;
    m_IR.op9.inst = p2_SETR;
    return parse_d_imm_s();
}

/**
 * @brief Set D field of D to S[8:0].
 *
 * EEEE 1001101 10I DDDDDDDDD SSSSSSSSS
 *
 * SETD    D,{#}S
 *
 * D = {D[31:18], S[8:0], D[8:0]}.
 */
bool P2Asm::asm_setd()
{
    m_widx++;
    m_IR.op9.inst = p2_SETD;
    return parse_d_imm_s();
}

/**
 * @brief Set S field of D to S[8:0].
 *
 * EEEE 1001101 11I DDDDDDDDD SSSSSSSSS
 *
 * SETS    D,{#}S
 *
 * D = {D[31:9], S[8:0]}.
 */
bool P2Asm::asm_sets()
{
    m_widx++;
    m_IR.op9.inst = p2_SETS;
    return parse_d_imm_s();
}

/**
 * @brief Decode S[4:0] into D.
 *
 * EEEE 1001110 00I DDDDDDDDD SSSSSSSSS
 *
 * DECOD   D,{#}S
 *
 * D = 1 << S[4:0].
 */
bool P2Asm::asm_decod()
{
    m_widx++;
    m_IR.op9.inst = p2_DECOD;
    return parse_d_imm_s();
}

/**
 * @brief Decode D[4:0] into D.
 *
 * EEEE 1001110 000 DDDDDDDDD DDDDDDDDD
 *
 * DECOD   D
 *
 * D = 1 << D[4:0].
 */
bool P2Asm::asm_decod_d()
{
    m_widx++;
    m_IR.op9.inst = p2_DECOD;
    return parse_d();
}

/**
 * @brief Get LSB-justified bit mask of size (S[4:0] + 1) into D.
 *
 * EEEE 1001110 01I DDDDDDDDD SSSSSSSSS
 *
 * BMASK   D,{#}S
 *
 * D = ($0000_0002 << S[4:0]) - 1.
 */
bool P2Asm::asm_bmask()
{
    m_widx++;
    m_IR.op9.inst = p2_BMASK;
    return parse_d_imm_s();
}

/**
 * @brief Get LSB-justified bit mask of size (D[4:0] + 1) into D.
 *
 * EEEE 1001110 010 DDDDDDDDD DDDDDDDDD
 *
 * BMASK   D
 *
 * D = ($0000_0002 << D[4:0]) - 1.
 */
bool P2Asm::asm_bmask_d()
{
    m_widx++;
    m_IR.op9.inst = p2_BMASK;
    return parse_d();
}

/**
 * @brief Iterate CRC value in D using C and polynomial in S.
 *
 * EEEE 1001110 10I DDDDDDDDD SSSSSSSSS
 *
 * CRCBIT  D,{#}S
 *
 * If (C XOR D[0]) then D = (D >> 1) XOR S, else D = (D >> 1).
 */
bool P2Asm::asm_crcbit()
{
    m_widx++;
    m_IR.op9.inst = p2_CRCBIT;
    return parse_d_imm_s();
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
 */
bool P2Asm::asm_crcnib()
{
    m_widx++;
    m_IR.op9.inst = p2_CRCNIB;
    return parse_d_imm_s();
}

/**
 * @brief For each non-zero bit pair in S, copy that bit pair into the corresponding D bits, else leave that D bit pair the same.
 *
 * EEEE 1001111 00I DDDDDDDDD SSSSSSSSS
 *
 * MUXNITS D,{#}S
 */
bool P2Asm::asm_muxnits()
{
    m_widx++;
    m_IR.op9.inst = p2_MUXNITS;
    return parse_d_imm_s();
}

/**
 * @brief For each non-zero nibble in S, copy that nibble into the corresponding D nibble, else leave that D nibble the same.
 *
 * EEEE 1001111 01I DDDDDDDDD SSSSSSSSS
 *
 * MUXNIBS D,{#}S
 */
bool P2Asm::asm_muxnibs()
{
    m_widx++;
    m_IR.op9.inst = p2_MUXNIBS;
    return parse_d_imm_s();
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
 */
bool P2Asm::asm_muxq()
{
    m_widx++;
    m_IR.op9.inst = p2_MUXQ;
    return parse_d_imm_s();
}

/**
 * @brief Move bytes within D, per S.
 *
 * EEEE 1001111 11I DDDDDDDDD SSSSSSSSS
 *
 * MOVBYTS D,{#}S
 *
 * D = {D.BYTE[S[7:6]], D.BYTE[S[5:4]], D.BYTE[S[3:2]], D.BYTE[S[1:0]]}.
 */
bool P2Asm::asm_movbyts()
{
    m_widx++;
    m_IR.op9.inst = p2_MOVBYTS;
    return parse_d_imm_s();
}

/**
 * @brief D = unsigned (D[15:0] * S[15:0]).
 *
 * EEEE 1010000 0ZI DDDDDDDDD SSSSSSSSS
 *
 * MUL     D,{#}S          {WZ}
 *
 * Z = (S == 0) | (D == 0).
 */
bool P2Asm::asm_mul()
{
    m_widx++;
    m_IR.op8.inst = p2_MUL;
    return parse_d_imm_s_wz();
}

/**
 * @brief D = signed (D[15:0] * S[15:0]).
 *
 * EEEE 1010000 1ZI DDDDDDDDD SSSSSSSSS
 *
 * MULS    D,{#}S          {WZ}
 *
 * Z = (S == 0) | (D == 0).
 */
bool P2Asm::asm_muls()
{
    m_widx++;
    m_IR.op8.inst = p2_MULS;
    return parse_d_imm_s_wz();
}

/**
 * @brief Next instruction's S value = unsigned (D[15:0] * S[15:0]) >> 16.
 *
 * EEEE 1010001 0ZI DDDDDDDDD SSSSSSSSS
 *
 * SCA     D,{#}S          {WZ}
 *
 * Z = (result == 0).
 */
bool P2Asm::asm_sca()
{
    m_widx++;
    m_IR.op8.inst = p2_SCA;
    return parse_d_imm_s_wz();
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
 */
bool P2Asm::asm_scas()
{
    m_widx++;
    m_IR.op8.inst = p2_SCAS;
    return parse_d_imm_s_wz();
}

/**
 * @brief Add bytes of S into bytes of D, with $FF saturation.
 *
 * EEEE 1010010 00I DDDDDDDDD SSSSSSSSS
 *
 * ADDPIX  D,{#}S
 */
bool P2Asm::asm_addpix()
{
    m_widx++;
    m_IR.op9.inst = p2_ADDPIX;
    return parse_d_imm_s();
}

/**
 * @brief Multiply bytes of S into bytes of D, where $FF = 1.
 *
 * EEEE 1010010 01I DDDDDDDDD SSSSSSSSS
 *
 * MULPIX  D,{#}S
 *
 * 0.
 */
bool P2Asm::asm_mulpix()
{
    m_widx++;
    m_IR.op9.inst = p2_MULPIX;
    return parse_d_imm_s();
}

/**
 * @brief Alpha-blend bytes of S into bytes of D, using SETPIV value.
 *
 * EEEE 1010010 10I DDDDDDDDD SSSSSSSSS
 *
 * BLNPIX  D,{#}S
 */
bool P2Asm::asm_blnpix()
{
    m_widx++;
    m_IR.op9.inst = p2_BLNPIX;
    return parse_d_imm_s();
}

/**
 * @brief Mix bytes of S into bytes of D, using SETPIX and SETPIV values.
 *
 * EEEE 1010010 11I DDDDDDDDD SSSSSSSSS
 *
 * MIXPIX  D,{#}S
 */
bool P2Asm::asm_mixpix()
{
    m_widx++;
    m_IR.op9.inst = p2_MIXPIX;
    return parse_d_imm_s();
}

/**
 * @brief Set CT1 event to trigger on CT = D + S.
 *
 * EEEE 1010011 00I DDDDDDDDD SSSSSSSSS
 *
 * ADDCT1  D,{#}S
 *
 * Adds S into D.
 */
bool P2Asm::asm_addct1()
{
    m_widx++;
    m_IR.op9.inst = p2_ADDCT1;
    return parse_d_imm_s();
}

/**
 * @brief Set CT2 event to trigger on CT = D + S.
 *
 * EEEE 1010011 01I DDDDDDDDD SSSSSSSSS
 *
 * ADDCT2  D,{#}S
 *
 * Adds S into D.
 */
bool P2Asm::asm_addct2()
{
    m_widx++;
    m_IR.op9.inst = p2_ADDCT2;
    return parse_d_imm_s();
}

/**
 * @brief Set CT3 event to trigger on CT = D + S.
 *
 * EEEE 1010011 10I DDDDDDDDD SSSSSSSSS
 *
 * ADDCT3  D,{#}S
 *
 * Adds S into D.
 */
bool P2Asm::asm_addct3()
{
    m_widx++;
    m_IR.op9.inst = p2_ADDCT3;
    return parse_d_imm_s();
}

/**
 * @brief Write only non-$00 bytes in D[31:0] to hub address {#}S/PTRx.
 *
 * EEEE 1010011 11I DDDDDDDDD SSSSSSSSS
 *
 * WMLONG  D,{#}S/P
 *
 * Prior SETQ/SETQ2 invokes cog/LUT block transfer.
 */
bool P2Asm::asm_wmlong()
{
    m_widx++;
    m_IR.op9.inst = p2_WMLONG;
    return parse_d_imm_s();
}

/**
 * @brief Read smart pin S[5:0] result "Z" into D, don't acknowledge smart pin ("Q" in RQPIN means "quiet").
 *
 * EEEE 1010100 C0I DDDDDDDDD SSSSSSSSS
 *
 * RQPIN   D,{#}S          {WC}
 *
 * C = modal result.
 */
bool P2Asm::asm_rqpin()
{
    m_widx++;
    m_IR.op8.inst = p2_RQPIN;
    return parse_d_imm_s_wc();
}

/**
 * @brief Read smart pin S[5:0] result "Z" into D, acknowledge smart pin.
 *
 * EEEE 1010100 C1I DDDDDDDDD SSSSSSSSS
 *
 * RDPIN   D,{#}S          {WC}
 *
 * C = modal result.
 */
bool P2Asm::asm_rdpin()
{
    m_widx++;
    m_IR.op8.inst = p2_RDPIN;
    return parse_d_imm_s_wc();
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
 */
bool P2Asm::asm_rdlut()
{
    m_widx++;
    m_IR.op.inst = p2_RDLUT;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_rdbyte()
{
    m_widx++;
    m_IR.op.inst = p2_RDBYTE;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_rdword()
{
    m_widx++;
    m_IR.op.inst = p2_RDWORD;
    return parse_d_imm_s_wcz();
}

/**
 * @brief Read long from hub address {#}S/PTRx into D.
 * Prior SETQ/SETQ2 invokes cog/LUT block transfer.
 *
 * EEEE 1011000 CZI DDDDDDDDD SSSSSSSSS
 *
 * RDLONG  D,{#}S/P {WC/WZ/WCZ}
 *
 * C = MSB of long.
 */
bool P2Asm::asm_rdlong()
{
    m_widx++;
    m_IR.op.inst = p2_RDLONG;
    return parse_d_imm_s_wcz();
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
 */
bool P2Asm::asm_popa()
{
    m_widx++;
    m_IR.op.inst = p2_RDLONG;
    m_IR.op.im = true;
    m_IR.op.src = 0x15f;
    return parse_d_cz();
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
 */
bool P2Asm::asm_popb()
{
    m_widx++;
    m_IR.op.inst = p2_RDLONG;
    m_IR.op.im = true;
    m_IR.op.src = 0x1df;
    return parse_d_cz();
}

/**
 * @brief Call to S** by writing {C, Z, 10'b0, PC[19:0]} to D.
 *
 * EEEE 1011001 CZI DDDDDDDDD SSSSSSSSS
 *
 * CALLD   D,{#}S   {WC/WZ/WCZ}
 *
 * C = S[31], Z = S[30].
 */
bool P2Asm::asm_calld()
{
    m_widx++;
    m_IR.op.inst = p2_CALLD;
    return parse_d_imm_s_wcz();
}

/**
 * @brief Resume from INT3.
 *
 * EEEE 1011001 110 111110000 111110001
 *
 * RESI3
 *
 * (CALLD $1F0,$1F1 WC,WZ).
 */
bool P2Asm::asm_resi3()
{
    m_widx++;
    m_IR.op.inst = p2_CALLD;
    m_IR.op.wc = true;
    m_IR.op.wz = true;
    m_IR.op.dst = offs_IJMP3;
    m_IR.op.src = offs_IRET3;
    return parse_inst();
}

/**
 * @brief Resume from INT2.
 *
 * EEEE 1011001 110 111110010 111110011
 *
 * RESI2
 *
 * (CALLD $1F2,$1F3 WC,WZ).
 */
bool P2Asm::asm_resi2()
{
    m_widx++;
    m_IR.op.inst = p2_CALLD;
    m_IR.op.wc = true;
    m_IR.op.wz = true;
    m_IR.op.dst = offs_IJMP2;
    m_IR.op.src = offs_IRET2;
    return parse_inst();
}

/**
 * @brief Resume from INT1.
 *
 * EEEE 1011001 110 111110100 111110101
 *
 * RESI1
 *
 * (CALLD $1F4,$1F5 WC,WZ).
 */
bool P2Asm::asm_resi1()
{
    m_widx++;
    m_IR.op.inst = p2_CALLD;
    m_IR.op.wc = true;
    m_IR.op.wz = true;
    m_IR.op.dst = offs_IJMP1;
    m_IR.op.src = offs_IRET1;
    return parse_inst();
}

/**
 * @brief Resume from INT0.
 *
 * EEEE 1011001 110 111111110 111111111
 *
 * RESI0
 *
 * (CALLD $1FE,$1FF WC,WZ).
 */
bool P2Asm::asm_resi0()
{
    m_widx++;
    m_IR.op.inst = p2_CALLD;
    m_IR.op.wc = true;
    m_IR.op.wz = true;
    m_IR.op.dst = offs_INA;
    m_IR.op.src = offs_INB;
    return parse_inst();
}

/**
 * @brief Return from INT3.
 *
 * EEEE 1011001 110 111111111 111110001
 *
 * RETI3
 *
 * (CALLD $1FF,$1F1 WC,WZ).
 */
bool P2Asm::asm_reti3()
{
    m_widx++;
    m_IR.op.inst = p2_CALLD;
    m_IR.op.wc = true;
    m_IR.op.wz = true;
    m_IR.op.dst = offs_INB;
    m_IR.op.src = offs_IRET3;
    return parse_inst();
}

/**
 * @brief Return from INT2.
 *
 * EEEE 1011001 110 111111111 111110011
 *
 * RETI2
 *
 * (CALLD $1FF,$1F3 WC,WZ).
 */
bool P2Asm::asm_reti2()
{
    m_widx++;
    m_IR.op.inst = p2_CALLD;
    m_IR.op.wc = true;
    m_IR.op.wz = true;
    m_IR.op.dst = offs_INB;
    m_IR.op.src = offs_IRET2;
    return parse_inst();
}

/**
 * @brief Return from INT1.
 *
 * EEEE 1011001 110 111111111 111110101
 *
 * RETI1
 *
 * (CALLD $1FF,$1F5 WC,WZ).
 */
bool P2Asm::asm_reti1()
{
    m_widx++;
    m_IR.op.inst = p2_CALLD;
    m_IR.op.wc = true;
    m_IR.op.wz = true;
    m_IR.op.dst = offs_INB;
    m_IR.op.src = offs_IRET1;
    return parse_inst();
}

/**
 * @brief Return from INT0.
 *
 * EEEE 1011001 110 111111111 111111111
 *
 * RETI0
 *
 * (CALLD $1FF,$1FF WC,WZ).
 */
bool P2Asm::asm_reti0()
{
    m_widx++;
    m_IR.op.inst = p2_CALLD;
    m_IR.op.wc = true;
    m_IR.op.wz = true;
    m_IR.op.dst = offs_INB;
    m_IR.op.src = offs_INB;
    return parse_inst();
}

/**
 * @brief Call to S** by pushing {C, Z, 10'b0, PC[19:0]} onto stack, copy D to PA.
 *
 * EEEE 1011010 0LI DDDDDDDDD SSSSSSSSS
 *
 * CALLPA  {#}D,{#}S
 */
bool P2Asm::asm_callpa()
{
    m_widx++;
    m_IR.op8.inst = p2_CALLPA;
    return parse_wz_d_imm_s();
}

/**
 * @brief Call to S** by pushing {C, Z, 10'b0, PC[19:0]} onto stack, copy D to PB.
 *
 * EEEE 1011010 1LI DDDDDDDDD SSSSSSSSS
 *
 * CALLPB  {#}D,{#}S
 */
bool P2Asm::asm_callpb()
{
    m_widx++;
    m_IR.op8.inst = p2_CALLPB;
    return parse_wz_d_imm_s();
}

/**
 * @brief Decrement D and jump to S** if result is zero.
 *
 * EEEE 1011011 00I DDDDDDDDD SSSSSSSSS
 *
 * DJZ     D,{#}S
 */
bool P2Asm::asm_djz()
{
    m_widx++;
    m_IR.op9.inst = p2_DJZ;
    return parse_d_imm_s();
}

/**
 * @brief Decrement D and jump to S** if result is not zero.
 *
 * EEEE 1011011 01I DDDDDDDDD SSSSSSSSS
 *
 * DJNZ    D,{#}S
 */
bool P2Asm::asm_djnz()
{
    m_widx++;
    m_IR.op9.inst = p2_DJNZ;
    return parse_d_imm_s();
}

/**
 * @brief Decrement D and jump to S** if result is $FFFF_FFFF.
 *
 * EEEE 1011011 10I DDDDDDDDD SSSSSSSSS
 *
 * DJF     D,{#}S
 */
bool P2Asm::asm_djf()
{
    m_widx++;
    m_IR.op9.inst = p2_DJF;
    return parse_d_imm_s();
}

/**
 * @brief Decrement D and jump to S** if result is not $FFFF_FFFF.
 *
 * EEEE 1011011 11I DDDDDDDDD SSSSSSSSS
 *
 * DJNF    D,{#}S
 */
bool P2Asm::asm_djnf()
{
    m_widx++;
    m_IR.op9.inst = p2_DJNF;
    return parse_d_imm_s();
}

/**
 * @brief Increment D and jump to S** if result is zero.
 *
 * EEEE 1011100 00I DDDDDDDDD SSSSSSSSS
 *
 * IJZ     D,{#}S
 */
bool P2Asm::asm_ijz()
{
    m_widx++;
    m_IR.op9.inst = p2_IJZ;
    return parse_d_imm_s();
}

/**
 * @brief Increment D and jump to S** if result is not zero.
 *
 * EEEE 1011100 01I DDDDDDDDD SSSSSSSSS
 *
 * IJNZ    D,{#}S
 */
bool P2Asm::asm_ijnz()
{
    m_widx++;
    m_IR.op9.inst = p2_IJNZ;
    return parse_d_imm_s();
}

/**
 * @brief Test D and jump to S** if D is zero.
 *
 * EEEE 1011100 10I DDDDDDDDD SSSSSSSSS
 *
 * TJZ     D,{#}S
 */
bool P2Asm::asm_tjz()
{
    m_widx++;
    m_IR.op9.inst = p2_TJZ;
    return parse_d_imm_s();
}

/**
 * @brief Test D and jump to S** if D is not zero.
 *
 * EEEE 1011100 11I DDDDDDDDD SSSSSSSSS
 *
 * TJNZ    D,{#}S
 */
bool P2Asm::asm_tjnz()
{
    m_widx++;
    m_IR.op9.inst = p2_TJNZ;
    return parse_d_imm_s();
}

/**
 * @brief Test D and jump to S** if D is full (D = $FFFF_FFFF).
 *
 * EEEE 1011101 00I DDDDDDDDD SSSSSSSSS
 *
 * TJF     D,{#}S
 */
bool P2Asm::asm_tjf()
{
    m_widx++;
    m_IR.op9.inst = p2_TJF;
    return parse_d_imm_s();
}

/**
 * @brief Test D and jump to S** if D is not full (D != $FFFF_FFFF).
 *
 * EEEE 1011101 01I DDDDDDDDD SSSSSSSSS
 *
 * TJNF    D,{#}S
 */
bool P2Asm::asm_tjnf()
{
    m_widx++;
    m_IR.op9.inst = p2_TJNF;
    return parse_d_imm_s();
}

/**
 * @brief Test D and jump to S** if D is signed (D[31] = 1).
 *
 * EEEE 1011101 10I DDDDDDDDD SSSSSSSSS
 *
 * TJS     D,{#}S
 */
bool P2Asm::asm_tjs()
{
    m_widx++;
    m_IR.op9.inst = p2_TJS;
    return parse_d_imm_s();
}

/**
 * @brief Test D and jump to S** if D is not signed (D[31] = 0).
 *
 * EEEE 1011101 11I DDDDDDDDD SSSSSSSSS
 *
 * TJNS    D,{#}S
 */
bool P2Asm::asm_tjns()
{
    m_widx++;
    m_IR.op9.inst = p2_TJNS;
    return parse_d_imm_s();
}

/**
 * @brief Test D and jump to S** if D overflowed (D[31] != C, C = 'correct sign' from last addition/subtraction).
 *
 * EEEE 1011110 00I DDDDDDDDD SSSSSSSSS
 *
 * TJV     D,{#}S
 */
bool P2Asm::asm_tjv()
{
    m_widx++;
    m_IR.op9.inst = p2_TJV;
    return parse_d_imm_s();
}

/**
 * @brief Jump to S** if INT event flag is set.
 *
 * EEEE 1011110 01I 000000000 SSSSSSSSS
 *
 * JINT    {#}S
 */
bool P2Asm::asm_jint()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JINT;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if CT1 event flag is set.
 *
 * EEEE 1011110 01I 000000001 SSSSSSSSS
 *
 * JCT1    {#}S
 */
bool P2Asm::asm_jct1()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JCT1;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if CT2 event flag is set.
 *
 * EEEE 1011110 01I 000000010 SSSSSSSSS
 *
 * JCT2    {#}S
 */
bool P2Asm::asm_jct2()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JCT2;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if CT3 event flag is set.
 *
 * EEEE 1011110 01I 000000011 SSSSSSSSS
 *
 * JCT3    {#}S
 */
bool P2Asm::asm_jct3()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JCT3;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if SE1 event flag is set.
 *
 * EEEE 1011110 01I 000000100 SSSSSSSSS
 *
 * JSE1    {#}S
 */
bool P2Asm::asm_jse1()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JSE1;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if SE2 event flag is set.
 *
 * EEEE 1011110 01I 000000101 SSSSSSSSS
 *
 * JSE2    {#}S
 */
bool P2Asm::asm_jse2()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JSE2;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if SE3 event flag is set.
 *
 * EEEE 1011110 01I 000000110 SSSSSSSSS
 *
 * JSE3    {#}S
 */
bool P2Asm::asm_jse3()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JSE3;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if SE4 event flag is set.
 *
 * EEEE 1011110 01I 000000111 SSSSSSSSS
 *
 * JSE4    {#}S
 */
bool P2Asm::asm_jse4()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JSE4;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if PAT event flag is set.
 *
 * EEEE 1011110 01I 000001000 SSSSSSSSS
 *
 * JPAT    {#}S
 */
bool P2Asm::asm_jpat()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JPAT;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if FBW event flag is set.
 *
 * EEEE 1011110 01I 000001001 SSSSSSSSS
 *
 * JFBW    {#}S
 */
bool P2Asm::asm_jfbw()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JFBW;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if XMT event flag is set.
 *
 * EEEE 1011110 01I 000001010 SSSSSSSSS
 *
 * JXMT    {#}S
 */
bool P2Asm::asm_jxmt()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JXMT;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if XFI event flag is set.
 *
 * EEEE 1011110 01I 000001011 SSSSSSSSS
 *
 * JXFI    {#}S
 */
bool P2Asm::asm_jxfi()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JXFI;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if XRO event flag is set.
 *
 * EEEE 1011110 01I 000001100 SSSSSSSSS
 *
 * JXRO    {#}S
 */
bool P2Asm::asm_jxro()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JXRO;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if XRL event flag is set.
 *
 * EEEE 1011110 01I 000001101 SSSSSSSSS
 *
 * JXRL    {#}S
 */
bool P2Asm::asm_jxrl()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JXRL;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if ATN event flag is set.
 *
 * EEEE 1011110 01I 000001110 SSSSSSSSS
 *
 * JATN    {#}S
 */
bool P2Asm::asm_jatn()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JATN;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if QMT event flag is set.
 *
 * EEEE 1011110 01I 000001111 SSSSSSSSS
 *
 * JQMT    {#}S
 */
bool P2Asm::asm_jqmt()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JQMT;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if INT event flag is clear.
 *
 * EEEE 1011110 01I 000010000 SSSSSSSSS
 *
 * JNINT   {#}S
 */
bool P2Asm::asm_jnint()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JNINT;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if CT1 event flag is clear.
 *
 * EEEE 1011110 01I 000010001 SSSSSSSSS
 *
 * JNCT1   {#}S
 */
bool P2Asm::asm_jnct1()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JNCT1;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if CT2 event flag is clear.
 *
 * EEEE 1011110 01I 000010010 SSSSSSSSS
 *
 * JNCT2   {#}S
 */
bool P2Asm::asm_jnct2()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JNCT2;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if CT3 event flag is clear.
 *
 * EEEE 1011110 01I 000010011 SSSSSSSSS
 *
 * JNCT3   {#}S
 */
bool P2Asm::asm_jnct3()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JNCT3;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if SE1 event flag is clear.
 *
 * EEEE 1011110 01I 000010100 SSSSSSSSS
 *
 * JNSE1   {#}S
 */
bool P2Asm::asm_jnse1()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JNSE1;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if SE2 event flag is clear.
 *
 * EEEE 1011110 01I 000010101 SSSSSSSSS
 *
 * JNSE2   {#}S
 */
bool P2Asm::asm_jnse2()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JNSE2;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if SE3 event flag is clear.
 *
 * EEEE 1011110 01I 000010110 SSSSSSSSS
 *
 * JNSE3   {#}S
 */
bool P2Asm::asm_jnse3()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JNSE3;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if SE4 event flag is clear.
 *
 * EEEE 1011110 01I 000010111 SSSSSSSSS
 *
 * JNSE4   {#}S
 */
bool P2Asm::asm_jnse4()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JNSE4;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if PAT event flag is clear.
 *
 * EEEE 1011110 01I 000011000 SSSSSSSSS
 *
 * JNPAT   {#}S
 */
bool P2Asm::asm_jnpat()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JNPAT;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if FBW event flag is clear.
 *
 * EEEE 1011110 01I 000011001 SSSSSSSSS
 *
 * JNFBW   {#}S
 */
bool P2Asm::asm_jnfbw()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JNFBW;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if XMT event flag is clear.
 *
 * EEEE 1011110 01I 000011010 SSSSSSSSS
 *
 * JNXMT   {#}S
 */
bool P2Asm::asm_jnxmt()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JNXMT;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if XFI event flag is clear.
 *
 * EEEE 1011110 01I 000011011 SSSSSSSSS
 *
 * JNXFI   {#}S
 */
bool P2Asm::asm_jnxfi()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JNXFI;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if XRO event flag is clear.
 *
 * EEEE 1011110 01I 000011100 SSSSSSSSS
 *
 * JNXRO   {#}S
 */
bool P2Asm::asm_jnxro()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JNXRO;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if XRL event flag is clear.
 *
 * EEEE 1011110 01I 000011101 SSSSSSSSS
 *
 * JNXRL   {#}S
 */
bool P2Asm::asm_jnxrl()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JNXRL;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if ATN event flag is clear.
 *
 * EEEE 1011110 01I 000011110 SSSSSSSSS
 *
 * JNATN   {#}S
 */
bool P2Asm::asm_jnatn()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JNATN;
    return parse_imm_s();
}

/**
 * @brief Jump to S** if QMT event flag is clear.
 *
 * EEEE 1011110 01I 000011111 SSSSSSSSS
 *
 * JNQMT   {#}S
 */
bool P2Asm::asm_jnqmt()
{
    m_widx++;
    m_IR.op9.inst = p2_OPDST;
    m_IR.op.dst = p2_OPDST_JNQMT;
    return parse_imm_s();
}

/**
 * @brief <empty>.
 *
 * EEEE 1011110 1LI DDDDDDDDD SSSSSSSSS
 *
 * <empty> {#}D,{#}S
 */
bool P2Asm::asm_1011110_1()
{
    m_widx++;
    m_IR.op9.inst = p2_1011110_10;
    return parse_wz_d_imm_s();
}

/**
 * @brief <empty>.
 *
 * EEEE 1011111 0LI DDDDDDDDD SSSSSSSSS
 *
 * <empty> {#}D,{#}S
 */
bool P2Asm::asm_1011111_0()
{
    m_widx++;
    m_IR.op8.inst = p2_1011111_0;
    return parse_wz_d_imm_s();
}

/**
 * @brief Set pin pattern for PAT event.
 *
 * EEEE 1011111 1LI DDDDDDDDD SSSSSSSSS
 *
 * SETPAT  {#}D,{#}S
 *
 * C selects INA/INB, Z selects =/!=, D provides mask value, S provides match value.
 */
bool P2Asm::asm_setpat()
{
    m_widx++;
    m_IR.op8.inst = p2_SETPAT;
    return parse_wz_d_imm_s();
}

/**
 * @brief Write D to mode register of smart pin S[5:0], acknowledge smart pin.
 *
 * EEEE 1100000 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRPIN   {#}D,{#}S
 */
bool P2Asm::asm_wrpin()
{
    m_widx++;
    m_IR.op8.inst = p2_WRPIN;
    return parse_wz_d_imm_s();
}

/**
 * @brief Acknowledge smart pin S[5:0].
 *
 * EEEE 1100000 01I 000000001 SSSSSSSSS
 *
 * AKPIN   {#}S
 */
bool P2Asm::asm_akpin()
{
    m_widx++;
    m_IR.op8.inst = p2_WRPIN;
    m_IR.op8.wz = true;
    m_IR.op.dst = 1;
    return parse_imm_s();
}

/**
 * @brief Write D to parameter "X" of smart pin S[5:0], acknowledge smart pin.
 *
 * EEEE 1100000 1LI DDDDDDDDD SSSSSSSSS
 *
 * WXPIN   {#}D,{#}S
 */
bool P2Asm::asm_wxpin()
{
    m_widx++;
    m_IR.op8.inst = p2_WXPIN;
    return parse_wz_d_imm_s();
}

/**
 * @brief Write D to parameter "Y" of smart pin S[5:0], acknowledge smart pin.
 *
 * EEEE 1100001 0LI DDDDDDDDD SSSSSSSSS
 *
 * WYPIN   {#}D,{#}S
 */
bool P2Asm::asm_wypin()
{
    m_widx++;
    m_IR.op8.inst = p2_WYPIN;
    return parse_wz_d_imm_s();
}

/**
 * @brief Write D to LUT address S[8:0].
 *
 * EEEE 1100001 1LI DDDDDDDDD SSSSSSSSS
 *
 * WRLUT   {#}D,{#}S
 */
bool P2Asm::asm_wrlut()
{
    m_widx++;
    m_IR.op8.inst = p2_WRLUT;
    return parse_wz_d_imm_s();
}

/**
 * @brief Write byte in D[7:0] to hub address {#}S/PTRx.
 *
 * EEEE 1100010 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRBYTE  {#}D,{#}S/P
 */
bool P2Asm::asm_wrbyte()
{
    m_widx++;
    m_IR.op8.inst = p2_WRBYTE;
    return parse_wz_d_imm_s();
}

/**
 * @brief Write word in D[15:0] to hub address {#}S/PTRx.
 *
 * EEEE 1100010 1LI DDDDDDDDD SSSSSSSSS
 *
 * WRWORD  {#}D,{#}S/P
 */
bool P2Asm::asm_wrword()
{
    m_widx++;
    m_IR.op8.inst = p2_WRWORD;
    return parse_wz_d_imm_s();
}

/**
 * @brief Write long in D[31:0] to hub address {#}S/PTRx.
 * Prior SETQ/SETQ2 invokes cog/LUT block transfer.
 *
 * EEEE 1100011 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRLONG  {#}D,{#}S/P
 */
bool P2Asm::asm_wrlong()
{
    m_widx++;
    m_IR.op8.inst = p2_WRLONG;
    return parse_wz_d_imm_s();
}

/**
 * @brief Write long in D[31:0] to hub address PTRA++.
 *
 * EEEE 1100011 0L1 DDDDDDDDD 101100001
 *
 * PUSHA   {#}D
 */
bool P2Asm::asm_pusha()
{
    m_widx++;
    m_IR.op8.inst = p2_WRLONG;
    m_IR.op8.im = true;
    m_IR.op8.src = 0x161;
    return parse_wz_d();
}

/**
 * @brief Write long in D[31:0] to hub address PTRB++.
 *
 * EEEE 1100011 0L1 DDDDDDDDD 111100001
 *
 * PUSHB   {#}D
 */
bool P2Asm::asm_pushb()
{
    m_widx++;
    m_IR.op8.inst = p2_WRLONG;
    m_IR.op8.im = true;
    m_IR.op8.src = 0x1e1;
    return parse_wz_d();
}

/**
 * @brief Begin new fast hub read via FIFO.
 *
 * EEEE 1100011 1LI DDDDDDDDD SSSSSSSSS
 *
 * RDFAST  {#}D,{#}S
 *
 * D[31] = no wait, D[13:0] = block size in 64-byte units (0 = max), S[19:0] = block start address.
 */
bool P2Asm::asm_rdfast()
{
    m_widx++;
    m_IR.op8.inst = p2_RDFAST;
    return parse_wz_d_imm_s();
}

/**
 * @brief Begin new fast hub write via FIFO.
 *
 * EEEE 1100100 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRFAST  {#}D,{#}S
 *
 * D[31] = no wait, D[13:0] = block size in 64-byte units (0 = max), S[19:0] = block start address.
 */
bool P2Asm::asm_wrfast()
{
    m_widx++;
    m_IR.op8.inst = p2_WRFAST;
    return parse_wz_d_imm_s();
}

/**
 * @brief Set next block for when block wraps.
 *
 * EEEE 1100100 1LI DDDDDDDDD SSSSSSSSS
 *
 * FBLOCK  {#}D,{#}S
 *
 * D[13:0] = block size in 64-byte units (0 = max), S[19:0] = block start address.
 */
bool P2Asm::asm_fblock()
{
    m_widx++;
    m_IR.op8.inst = p2_FBLOCK;
    return parse_wz_d_imm_s();
}

/**
 * @brief Issue streamer command immediately, zeroing phase.
 *
 * EEEE 1100101 0LI DDDDDDDDD SSSSSSSSS
 *
 * XINIT   {#}D,{#}S
 */
bool P2Asm::asm_xinit()
{
    m_widx++;
    m_IR.op8.inst = p2_XINIT;
    return parse_wz_d_imm_s();
}

/**
 * @brief Stop streamer immediately.
 *
 * EEEE 1100101 011 000000000 000000000
 *
 * XSTOP
 */
bool P2Asm::asm_xstop()
{
    m_widx++;
    m_IR.op8.inst = p2_XINIT;
    m_IR.op8.wz = true;
    m_IR.op8.im = true;
    m_IR.op8.dst = 0x000;
    m_IR.op8.src = 0x000;
    return parse_inst();
}

/**
 * @brief Buffer new streamer command to be issued on final NCO rollover of current command, zeroing phase.
 *
 * EEEE 1100101 1LI DDDDDDDDD SSSSSSSSS
 *
 * XZERO   {#}D,{#}S
 */
bool P2Asm::asm_xzero()
{
    m_widx++;
    m_IR.op8.inst = p2_XZERO;
    return parse_wz_d_imm_s();
}

/**
 * @brief Buffer new streamer command to be issued on final NCO rollover of current command, continuing phase.
 *
 * EEEE 1100110 0LI DDDDDDDDD SSSSSSSSS
 *
 * XCONT   {#}D,{#}S
 */
bool P2Asm::asm_xcont()
{
    m_widx++;
    m_IR.op8.inst = p2_XCONT;
    return parse_wz_d_imm_s();
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
 */
bool P2Asm::asm_rep()
{
    m_widx++;
    m_IR.op8.inst = p2_REP;
    return parse_wz_d_imm_s();
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
 */
bool P2Asm::asm_coginit()
{
    m_widx++;
    m_IR.op.inst = p2_COGINIT;
    return parse_wz_d_imm_s();
}

/**
 * @brief Begin CORDIC unsigned multiplication of D * S.
 *
 * EEEE 1101000 0LI DDDDDDDDD SSSSSSSSS
 *
 * QMUL    {#}D,{#}S
 *
 * GETQX/GETQY retrieves lower/upper product.
 */
bool P2Asm::asm_qmul()
{
    m_widx++;
    m_IR.op8.inst = p2_QMUL;
    return parse_wz_d_imm_s();
}

/**
 * @brief Begin CORDIC unsigned division of {SETQ value or 32'b0, D} / S.
 *
 * EEEE 1101000 1LI DDDDDDDDD SSSSSSSSS
 *
 * QDIV    {#}D,{#}S
 *
 * GETQX/GETQY retrieves quotient/remainder.
 */
bool P2Asm::asm_qdiv()
{
    m_widx++;
    m_IR.op8.inst = p2_QDIV;
    return parse_wz_d_imm_s();
}

/**
 * @brief Begin CORDIC unsigned division of {D, SETQ value or 32'b0} / S.
 *
 * EEEE 1101001 0LI DDDDDDDDD SSSSSSSSS
 *
 * QFRAC   {#}D,{#}S
 *
 * GETQX/GETQY retrieves quotient/remainder.
 */
bool P2Asm::asm_qfrac()
{
    m_widx++;
    m_IR.op8.inst = p2_QFRAC;
    return parse_wz_d_imm_s();
}

/**
 * @brief Begin CORDIC square root of {S, D}.
 *
 * EEEE 1101001 1LI DDDDDDDDD SSSSSSSSS
 *
 * QSQRT   {#}D,{#}S
 *
 * GETQX retrieves root.
 */
bool P2Asm::asm_qsqrt()
{
    m_widx++;
    m_IR.op8.inst = p2_QSQRT;
    return parse_wz_d_imm_s();
}

/**
 * @brief Begin CORDIC rotation of point (D, SETQ value or 32'b0) by angle S.
 *
 * EEEE 1101010 0LI DDDDDDDDD SSSSSSSSS
 *
 * QROTATE {#}D,{#}S
 *
 * GETQX/GETQY retrieves X/Y.
 */
bool P2Asm::asm_qrotate()
{
    m_widx++;
    m_IR.op8.inst = p2_QROTATE;
    return parse_wz_d_imm_s();
}

/**
 * @brief Begin CORDIC vectoring of point (D, S).
 *
 * EEEE 1101010 1LI DDDDDDDDD SSSSSSSSS
 *
 * QVECTOR {#}D,{#}S
 *
 * GETQX/GETQY retrieves length/angle.
 */
bool P2Asm::asm_qvector()
{
    m_widx++;
    m_IR.op8.inst = p2_QVECTOR;
    return parse_wz_d_imm_s();
}

/**
 * @brief Set hub configuration to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000000000
 *
 * HUBSET  {#}D
 */
bool P2Asm::asm_hubset()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_HUBSET;
    return parse_imm_d();
}

/**
 * @brief If D is register and no WC, get cog ID (0 to 15) into D.
 *
 * EEEE 1101011 C0L DDDDDDDDD 000000001
 *
 * COGID   {#}D            {WC}
 *
 * If WC, check status of cog D[3:0], C = 1 if on.
 */
bool P2Asm::asm_cogid()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_COGID;
    return parse_imm_d_wc();
}

/**
 * @brief Stop cog D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000000011
 *
 * COGSTOP {#}D
 */
bool P2Asm::asm_cogstop()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_COGSTOP;
    return parse_imm_d();
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
 */
bool P2Asm::asm_locknew()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_LOCKNEW;
    return parse_imm_d_wc();
}

/**
 * @brief Return LOCK D[3:0] for reallocation.
 *
 * EEEE 1101011 00L DDDDDDDDD 000000101
 *
 * LOCKRET {#}D
 */
bool P2Asm::asm_lockret()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_LOCKRET;
    return parse_imm_d();
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
 */
bool P2Asm::asm_locktry()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_LOCKTRY;
    return parse_imm_d_wc();
}

/**
 * @brief Release LOCK D[3:0].
 *
 * EEEE 1101011 C0L DDDDDDDDD 000000111
 *
 * LOCKREL {#}D            {WC}
 *
 * If D is a register and WC, get current/last cog id of LOCK owner into D and LOCK status into C.
 */
bool P2Asm::asm_lockrel()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_LOCKREL;
    return parse_imm_d_wc();
}

/**
 * @brief Begin CORDIC number-to-logarithm conversion of D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000001110
 *
 * QLOG    {#}D
 *
 * GETQX retrieves log {5'whole_exponent, 27'fractional_exponent}.
 */
bool P2Asm::asm_qlog()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_QLOG;
    return parse_imm_d();
}

/**
 * @brief Begin CORDIC logarithm-to-number conversion of D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000001111
 *
 * QEXP    {#}D
 *
 * GETQX retrieves number.
 */
bool P2Asm::asm_qexp()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_QEXP;
    return parse_imm_d();
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
 */
bool P2Asm::asm_rfbyte()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_RFBYTE;
    return parse_d_cz();
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
 */
bool P2Asm::asm_rfword()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_RFWORD;
    return parse_d_cz();
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
 */
bool P2Asm::asm_rflong()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_RFLONG;
    return parse_d_cz();
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
 */
bool P2Asm::asm_rfvar()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_RFVAR;
    return parse_d_cz();
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
 */
bool P2Asm::asm_rfvars()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_RFVARS;
    return parse_d_cz();
}

/**
 * @brief Write byte in D[7:0] into FIFO. Used after WRFAST.
 *
 * EEEE 1101011 00L DDDDDDDDD 000010101
 *
 * WFBYTE  {#}D
 */
bool P2Asm::asm_wfbyte()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_WFBYTE;
    return parse_imm_d();
}

/**
 * @brief Write word in D[15:0] into FIFO. Used after WRFAST.
 *
 * EEEE 1101011 00L DDDDDDDDD 000010110
 *
 * WFWORD  {#}D
 */
bool P2Asm::asm_wfword()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_WFWORD;
    return parse_imm_d();
}

/**
 * @brief Write long in D[31:0] into FIFO. Used after WRFAST.
 *
 * EEEE 1101011 00L DDDDDDDDD 000010111
 *
 * WFLONG  {#}D
 */
bool P2Asm::asm_wflong()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_WFLONG;
    return parse_imm_d();
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
 */
bool P2Asm::asm_getqx()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_GETQX;
    return parse_d_cz();
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
 */
bool P2Asm::asm_getqy()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_GETQY;
    return parse_d_cz();
}

/**
 * @brief Get CT into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 000011010
 *
 * GETCT   D
 *
 * CT is the free-running 32-bit system counter that increments on every clock.
 */
bool P2Asm::asm_getct()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_GETCT;
    return parse_d();
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
 */
bool P2Asm::asm_getrnd()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_GETRND;
    return parse_d_cz();
}

/**
 * @brief Get RND into C/Z.
 *
 * EEEE 1101011 CZ1 000000000 000011011
 *
 * GETRND            WC/WZ/WCZ
 *
 * C = RND[31], Z = RND[30], unique per cog.
 */
bool P2Asm::asm_getrnd_cz()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_GETRND;
    return parse_cz();
}

/**
 * @brief DAC3 = D[31:24], DAC2 = D[23:16], DAC1 = D[15:8], DAC0 = D[7:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000011100
 *
 * SETDACS {#}D
 */
bool P2Asm::asm_setdacs()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SETDACS;
    return parse_imm_d();
}

/**
 * @brief Set streamer NCO frequency to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000011101
 *
 * SETXFRQ {#}D
 */
bool P2Asm::asm_setxfrq()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SETXFRQ;
    return parse_imm_d();
}

/**
 * @brief Get the streamer's Goertzel X accumulator into D and the Y accumulator into the next instruction's S, clear accumulators.
 *
 * EEEE 1101011 000 DDDDDDDDD 000011110
 *
 * GETXACC D
 */
bool P2Asm::asm_getxacc()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_GETXACC;
    return parse_d();
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
 */
bool P2Asm::asm_waitx()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_WAITX;
    return parse_imm_d_wcz();
}

/**
 * @brief Set SE1 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100000
 *
 * SETSE1  {#}D
 */
bool P2Asm::asm_setse1()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SETSE1;
    return parse_imm_d();
}

/**
 * @brief Set SE2 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100001
 *
 * SETSE2  {#}D
 */
bool P2Asm::asm_setse2()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SETSE2;
    return parse_imm_d();
}

/**
 * @brief Set SE3 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100010
 *
 * SETSE3  {#}D
 */
bool P2Asm::asm_setse3()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SETSE3;
    return parse_imm_d();
}

/**
 * @brief Set SE4 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100011
 *
 * SETSE4  {#}D
 */
bool P2Asm::asm_setse4()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SETSE4;
    return parse_imm_d();
}

/**
 * @brief Get INT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000000 000100100
 *
 * POLLINT          {WC/WZ/WCZ}
 */
bool P2Asm::asm_pollint()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_POLLINT;
    return parse_cz();
}

/**
 * @brief Get CT1 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000001 000100100
 *
 * POLLCT1          {WC/WZ/WCZ}
 */
bool P2Asm::asm_pollct1()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_POLLCT1;
    return parse_cz();
}

/**
 * @brief Get CT2 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000010 000100100
 *
 * POLLCT2          {WC/WZ/WCZ}
 */
bool P2Asm::asm_pollct2()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_POLLCT2;
    return parse_cz();
}

/**
 * @brief Get CT3 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000011 000100100
 *
 * POLLCT3          {WC/WZ/WCZ}
 */
bool P2Asm::asm_pollct3()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_POLLCT3;
    return parse_cz();
}

/**
 * @brief Get SE1 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000100 000100100
 *
 * POLLSE1          {WC/WZ/WCZ}
 */
bool P2Asm::asm_pollse1()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_POLLSE1;
    return parse_cz();
}

/**
 * @brief Get SE2 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000101 000100100
 *
 * POLLSE2          {WC/WZ/WCZ}
 */
bool P2Asm::asm_pollse2()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_POLLSE2;
    return parse_cz();
}

/**
 * @brief Get SE3 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000110 000100100
 *
 * POLLSE3          {WC/WZ/WCZ}
 */
bool P2Asm::asm_pollse3()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_POLLSE3;
    return parse_cz();
}

/**
 * @brief Get SE4 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000111 000100100
 *
 * POLLSE4          {WC/WZ/WCZ}
 */
bool P2Asm::asm_pollse4()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_POLLSE4;
    return parse_cz();
}

/**
 * @brief Get PAT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001000 000100100
 *
 * POLLPAT          {WC/WZ/WCZ}
 */
bool P2Asm::asm_pollpat()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_POLLPAT;
    return parse_cz();
}

/**
 * @brief Get FBW event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001001 000100100
 *
 * POLLFBW          {WC/WZ/WCZ}
 */
bool P2Asm::asm_pollfbw()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_POLLFBW;
    return parse_cz();
}

/**
 * @brief Get XMT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001010 000100100
 *
 * POLLXMT          {WC/WZ/WCZ}
 */
bool P2Asm::asm_pollxmt()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_POLLXMT;
    return parse_cz();
}

/**
 * @brief Get XFI event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001011 000100100
 *
 * POLLXFI          {WC/WZ/WCZ}
 */
bool P2Asm::asm_pollxfi()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_POLLXFI;
    return parse_cz();
}

/**
 * @brief Get XRO event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001100 000100100
 *
 * POLLXRO          {WC/WZ/WCZ}
 */
bool P2Asm::asm_pollxro()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_POLLXRO;
    return parse_cz();
}

/**
 * @brief Get XRL event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001101 000100100
 *
 * POLLXRL          {WC/WZ/WCZ}
 */
bool P2Asm::asm_pollxrl()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_POLLXRL;
    return parse_cz();
}

/**
 * @brief Get ATN event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001110 000100100
 *
 * POLLATN          {WC/WZ/WCZ}
 */
bool P2Asm::asm_pollatn()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_POLLATN;
    return parse_cz();
}

/**
 * @brief Get QMT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001111 000100100
 *
 * POLLQMT          {WC/WZ/WCZ}
 */
bool P2Asm::asm_pollqmt()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_POLLQMT;
    return parse_cz();
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
 */
bool P2Asm::asm_waitint()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_WAITINT;
    return parse_cz();
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
 */
bool P2Asm::asm_waitct1()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_WAITCT1;
    return parse_cz();
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
 */
bool P2Asm::asm_waitct2()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_WAITCT2;
    return parse_cz();
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
 */
bool P2Asm::asm_waitct3()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_WAITCT3;
    return parse_cz();
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
 */
bool P2Asm::asm_waitse1()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_WAITSE1;
    return parse_cz();
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
 */
bool P2Asm::asm_waitse2()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_WAITSE2;
    return parse_cz();
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
 */
bool P2Asm::asm_waitse3()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_WAITSE3;
    return parse_cz();
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
 */
bool P2Asm::asm_waitse4()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_WAITSE4;
    return parse_cz();
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
 */
bool P2Asm::asm_waitpat()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_WAITPAT;
    return parse_cz();
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
 */
bool P2Asm::asm_waitfbw()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_WAITFBW;
    return parse_cz();
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
 */
bool P2Asm::asm_waitxmt()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_WAITXMT;
    return parse_cz();
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
 */
bool P2Asm::asm_waitxfi()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_WAITXFI;
    return parse_cz();
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
 */
bool P2Asm::asm_waitxro()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_WAITXRO;
    return parse_cz();
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
 */
bool P2Asm::asm_waitxrl()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_WAITXRL;
    return parse_cz();
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
 */
bool P2Asm::asm_waitatn()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_WAITATN;
    return parse_cz();
}

/**
 * @brief Allow interrupts (default).
 *
 * EEEE 1101011 000 000100000 000100100
 *
 * ALLOWI
 */
bool P2Asm::asm_allowi()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_ALLOWI;
    return parse_inst();
}

/**
 * @brief Stall Interrupts.
 *
 * EEEE 1101011 000 000100001 000100100
 *
 * STALLI
 */
bool P2Asm::asm_stalli()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_STALLI;
    return parse_inst();
}

/**
 * @brief Trigger INT1, regardless of STALLI mode.
 *
 * EEEE 1101011 000 000100010 000100100
 *
 * TRGINT1
 */
bool P2Asm::asm_trgint1()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_TRGINT1;
    return parse_inst();
}

/**
 * @brief Trigger INT2, regardless of STALLI mode.
 *
 * EEEE 1101011 000 000100011 000100100
 *
 * TRGINT2
 */
bool P2Asm::asm_trgint2()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_TRGINT2;
    return parse_inst();
}

/**
 * @brief Trigger INT3, regardless of STALLI mode.
 *
 * EEEE 1101011 000 000100100 000100100
 *
 * TRGINT3
 */
bool P2Asm::asm_trgint3()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_TRGINT3;
    return parse_inst();
}

/**
 * @brief Cancel INT1.
 *
 * EEEE 1101011 000 000100101 000100100
 *
 * NIXINT1
 */
bool P2Asm::asm_nixint1()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_NIXINT1;
    return parse_inst();
}

/**
 * @brief Cancel INT2.
 *
 * EEEE 1101011 000 000100110 000100100
 *
 * NIXINT2
 */
bool P2Asm::asm_nixint2()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_NIXINT2;
    return parse_inst();
}

/**
 * @brief Cancel INT3.
 *
 * EEEE 1101011 000 000100111 000100100
 *
 * NIXINT3
 */
bool P2Asm::asm_nixint3()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_X24;
    m_IR.op.dst = p2_OPX24_NIXINT3;
    return parse_inst();
}

/**
 * @brief Set INT1 source to D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100101
 *
 * SETINT1 {#}D
 */
bool P2Asm::asm_setint1()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SETINT1;
    return parse_imm_d();
}

/**
 * @brief Set INT2 source to D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100110
 *
 * SETINT2 {#}D
 */
bool P2Asm::asm_setint2()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SETINT2;
    return parse_imm_d();
}

/**
 * @brief Set INT3 source to D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100111
 *
 * SETINT3 {#}D
 */
bool P2Asm::asm_setint3()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SETINT3;
    return parse_imm_d();
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
 */
bool P2Asm::asm_setq()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SETQ;
    return parse_imm_d();
}

/**
 * @brief Set Q to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000101001
 *
 * SETQ2   {#}D
 *
 * Use before RDLONG/WRLONG/WMLONG to set LUT block transfer.
 */
bool P2Asm::asm_setq2()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SETQ2;
    return parse_imm_d();
}

/**
 * @brief Push D onto stack.
 *
 * EEEE 1101011 00L DDDDDDDDD 000101010
 *
 * PUSH    {#}D
 */
bool P2Asm::asm_push()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_PUSH;
    return parse_imm_d();
}

/**
 * @brief Pop stack (K).
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101011
 *
 * POP     D        {WC/WZ/WCZ}
 *
 * C = K[31], Z = K[30], D = K.
 */
bool P2Asm::asm_pop()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_POP;
    return parse_d_cz();
}

/**
 * @brief Jump to D.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101100
 *
 * JMP     D        {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 */
bool P2Asm::asm_jmp()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_JMP;
    return parse_d_cz();
}

/**
 * @brief Call to D by pushing {C, Z, 10'b0, PC[19:0]} onto stack.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101101
 *
 * CALL    D        {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 */
bool P2Asm::asm_call()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_CALL_RET;
    return parse_d_cz();
}

/**
 * @brief Return by popping stack (K).
 *
 * EEEE 1101011 CZ1 000000000 000101101
 *
 * RET              {WC/WZ/WCZ}
 *
 * C = K[31], Z = K[30], PC = K[19:0].
 */
bool P2Asm::asm_ret()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.im = true;
    m_IR.op.dst = 0x000;
    m_IR.op.src = p2_OPSRC_CALL_RET;
    return parse_cz();
}

/**
 * @brief Call to D by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRA++.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101110
 *
 * CALLA   D        {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 */
bool P2Asm::asm_calla()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_CALLA_RETA;
    return parse_d_cz();
}

/**
 * @brief Return by reading hub long (L) at --PTRA.
 *
 * EEEE 1101011 CZ1 000000000 000101110
 *
 * RETA             {WC/WZ/WCZ}
 *
 * C = L[31], Z = L[30], PC = L[19:0].
 */
bool P2Asm::asm_reta()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.im = true;
    m_IR.op.dst = 0x000;
    m_IR.op.src = p2_OPSRC_CALLA_RETA;
    return parse_cz();
}

/**
 * @brief Call to D by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRB++.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101111
 *
 * CALLB   D        {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 */
bool P2Asm::asm_callb()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_CALLB_RETB;
    return parse_d_cz();
}

/**
 * @brief Return by reading hub long (L) at --PTRB.
 *
 * EEEE 1101011 CZ1 000000000 000101111
 *
 * RETB             {WC/WZ/WCZ}
 *
 * C = L[31], Z = L[30], PC = L[19:0].
 */
bool P2Asm::asm_retb()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.im = true;
    m_IR.op.dst = 0x000;
    m_IR.op.src = p2_OPSRC_CALLB_RETB;
    return parse_cz();
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
 */
bool P2Asm::asm_jmprel()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_JMPREL;
    return parse_imm_d();
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
 */
bool P2Asm::asm_skip()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SKIP;
    return parse_imm_d();
}

/**
 * @brief Skip cog/LUT instructions fast per D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110010
 *
 * SKIPF   {#}D
 *
 * Like SKIP, but instead of cancelling instructions, the PC leaps over them.
 */
bool P2Asm::asm_skipf()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SKIPF;
    return parse_imm_d();
}

/**
 * @brief Jump to D[9:0] in cog/LUT and set SKIPF pattern to D[31:10].
 *
 * EEEE 1101011 00L DDDDDDDDD 000110011
 *
 * EXECF   {#}D
 *
 * PC = {10'b0, D[9:0]}.
 */
bool P2Asm::asm_execf()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_EXECF;
    return parse_imm_d();
}

/**
 * @brief Get current FIFO hub pointer into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 000110100
 *
 * GETPTR  D
 */
bool P2Asm::asm_getptr()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_GETPTR;
    return parse_d();
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
 */
bool P2Asm::asm_getbrk()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.im = false;
    m_IR.op.src = p2_OPSRC_COGBRK;
    return parse_d_cz();
}

/**
 * @brief If in debug ISR, trigger asynchronous breakpoint in cog D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000110101
 *
 * COGBRK  {#}D
 *
 * Cog D[3:0] must have asynchronous breakpoint enabled.
 */
bool P2Asm::asm_cogbrk()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_COGBRK;
    return parse_imm_d();
}

/**
 * @brief If in debug ISR, set next break condition to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110110
 *
 * BRK     {#}D
 *
 * Else, trigger break if enabled, conditionally write break code to D[7:0].
 */
bool P2Asm::asm_brk()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_BRK;
    return parse_imm_d();
}

/**
 * @brief If D[0] = 1 then enable LUT sharing, where LUT writes within the adjacent odd/even companion cog are copied to this LUT.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110111
 *
 * SETLUTS {#}D
 */
bool P2Asm::asm_setluts()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SETLUTS;
    return parse_imm_d();
}

/**
 * @brief Set the colorspace converter "CY" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111000
 *
 * SETCY   {#}D
 */
bool P2Asm::asm_setcy()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SETCY;
    return parse_imm_d();
}

/**
 * @brief Set the colorspace converter "CI" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111001
 *
 * SETCI   {#}D
 */
bool P2Asm::asm_setci()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SETCI;
    return parse_imm_d();
}

/**
 * @brief Set the colorspace converter "CQ" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111010
 *
 * SETCQ   {#}D
 */
bool P2Asm::asm_setcq()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SETCQ;
    return parse_imm_d();
}

/**
 * @brief Set the colorspace converter "CFRQ" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111011
 *
 * SETCFRQ {#}D
 */
bool P2Asm::asm_setcfrq()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SETCFRQ;
    return parse_imm_d();
}

/**
 * @brief Set the colorspace converter "CMOD" parameter to D[6:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111100
 *
 * SETCMOD {#}D
 */
bool P2Asm::asm_setcmod()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SETCMOD;
    return parse_imm_d();
}

/**
 * @brief Set BLNPIX/MIXPIX blend factor to D[7:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111101
 *
 * SETPIV  {#}D
 */
bool P2Asm::asm_setpiv()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SETPIV;
    return parse_imm_d();
}

/**
 * @brief Set MIXPIX mode to D[5:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111110
 *
 * SETPIX  {#}D
 */
bool P2Asm::asm_setpix()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SETPIX;
    return parse_imm_d();
}

/**
 * @brief Strobe "attention" of all cogs whose corresponging bits are high in D[15:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111111
 *
 * COGATN  {#}D
 */
bool P2Asm::asm_cogatn()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_COGATN;
    return parse_imm_d();
}

/**
 * @brief Test  IN bit of pin D[5:0], write to C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000000
 *
 * TESTP   {#}D           WC/WZ
 *
 * C/Z =          IN[D[5:0]].
 */
bool P2Asm::asm_testp_w()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_TESTP_W_DIRL;
    return parse_imm_d_wcz();
}

/**
 * @brief Test !IN bit of pin D[5:0], write to C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000001
 *
 * TESTPN  {#}D           WC/WZ
 *
 * C/Z =         !IN[D[5:0]].
 */
bool P2Asm::asm_testpn_w()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_TESTPN_W;
    return parse_imm_d_wcz();
}

/**
 * @brief Test  IN bit of pin D[5:0], AND into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000010
 *
 * TESTP   {#}D       ANDC/ANDZ
 *
 * C/Z = C/Z AND  IN[D[5:0]].
 */
bool P2Asm::asm_testp_and()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_TESTP_AND;
    return parse_imm_d_wcz();
}

/**
 * @brief Test !IN bit of pin D[5:0], AND into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000011
 *
 * TESTPN  {#}D       ANDC/ANDZ
 *
 * C/Z = C/Z AND !IN[D[5:0]].
 */
bool P2Asm::asm_testpn_and()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_TESTPN_AND;
    return parse_imm_d_wcz();
}

/**
 * @brief Test  IN bit of pin D[5:0], OR  into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000100
 *
 * TESTP   {#}D         ORC/ORZ
 *
 * C/Z = C/Z OR   IN[D[5:0]].
 */
bool P2Asm::asm_testp_or()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_TESTP_OR;
    return parse_imm_d_wcz();
}

/**
 * @brief Test !IN bit of pin D[5:0], OR  into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000101
 *
 * TESTPN  {#}D         ORC/ORZ
 *
 * C/Z = C/Z OR  !IN[D[5:0]].
 */
bool P2Asm::asm_testpn_or()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_TESTPN_OR;
    return parse_imm_d_wcz();
}

/**
 * @brief Test  IN bit of pin D[5:0], XOR into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000110
 *
 * TESTP   {#}D       XORC/XORZ
 *
 * C/Z = C/Z XOR  IN[D[5:0]].
 */
bool P2Asm::asm_testp_xor()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_TESTP_XOR;
    return parse_imm_d_wcz();
}

/**
 * @brief Test !IN bit of pin D[5:0], XOR into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000111
 *
 * TESTPN  {#}D       XORC/XORZ
 *
 * C/Z = C/Z XOR !IN[D[5:0]].
 */
bool P2Asm::asm_testpn_xor()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_TESTPN_XOR;
    return parse_imm_d_wcz();
}

/**
 * @brief DIR bit of pin D[5:0] = 0.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000000
 *
 * DIRL    {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 */
bool P2Asm::asm_dirl()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_DIRL;
    return parse_imm_d_wcz();
}

/**
 * @brief DIR bit of pin D[5:0] = 1.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000001
 *
 * DIRH    {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 */
bool P2Asm::asm_dirh()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_DIRH;
    return parse_imm_d_wcz();
}

/**
 * @brief DIR bit of pin D[5:0] = C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000010
 *
 * DIRC    {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 */
bool P2Asm::asm_dirc()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_DIRC;
    return parse_imm_d_wcz();
}

/**
 * @brief DIR bit of pin D[5:0] = !C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000011
 *
 * DIRNC   {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 */
bool P2Asm::asm_dirnc()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_DIRNC;
    return parse_imm_d_wcz();
}

/**
 * @brief DIR bit of pin D[5:0] = Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000100
 *
 * DIRZ    {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 */
bool P2Asm::asm_dirz()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_DIRZ;
    return parse_imm_d_wcz();
}

/**
 * @brief DIR bit of pin D[5:0] = !Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000101
 *
 * DIRNZ   {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 */
bool P2Asm::asm_dirnz()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_DIRNZ;
    return parse_imm_d_wcz();
}

/**
 * @brief DIR bit of pin D[5:0] = RND.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000110
 *
 * DIRRND  {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 */
bool P2Asm::asm_dirrnd()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_DIRRND;
    return parse_imm_d_wcz();
}

/**
 * @brief DIR bit of pin D[5:0] = !bit.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000111
 *
 * DIRNOT  {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 */
bool P2Asm::asm_dirnot()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_DIRNOT;
    return parse_imm_d_wcz();
}

/**
 * @brief OUT bit of pin D[5:0] = 0.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001000
 *
 * OUTL    {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 */
bool P2Asm::asm_outl()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_OUTL;
    return parse_imm_d_wcz();
}

/**
 * @brief OUT bit of pin D[5:0] = 1.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001001
 *
 * OUTH    {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 */
bool P2Asm::asm_outh()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_OUTH;
    return parse_imm_d_wcz();
}

/**
 * @brief OUT bit of pin D[5:0] = C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001010
 *
 * OUTC    {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 */
bool P2Asm::asm_outc()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_OUTC;
    return parse_imm_d_wcz();
}

/**
 * @brief OUT bit of pin D[5:0] = !C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001011
 *
 * OUTNC   {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 */
bool P2Asm::asm_outnc()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_OUTNC;
    return parse_imm_d_wcz();
}

/**
 * @brief OUT bit of pin D[5:0] = Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001100
 *
 * OUTZ    {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 */
bool P2Asm::asm_outz()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_OUTZ;
    return parse_imm_d_wcz();
}

/**
 * @brief OUT bit of pin D[5:0] = !Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001101
 *
 * OUTNZ   {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 */
bool P2Asm::asm_outnz()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_OUTNZ;
    return parse_imm_d_wcz();
}

/**
 * @brief OUT bit of pin D[5:0] = RND.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001110
 *
 * OUTRND  {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 */
bool P2Asm::asm_outrnd()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_OUTRND;
    return parse_imm_d_wcz();
}

/**
 * @brief OUT bit of pin D[5:0] = !bit.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001111
 *
 * OUTNOT  {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 */
bool P2Asm::asm_outnot()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_OUTNOT;
    return parse_imm_d_wcz();
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
 */
bool P2Asm::asm_fltl()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_FLTL;
    return parse_imm_d_wcz();
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
 */
bool P2Asm::asm_flth()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_FLTH;
    return parse_imm_d_wcz();
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
 */
bool P2Asm::asm_fltc()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_FLTC;
    return parse_imm_d_wcz();
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
 */
bool P2Asm::asm_fltnc()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_FLTNC;
    return parse_imm_d_wcz();
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
 */
bool P2Asm::asm_fltz()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_FLTZ;
    return parse_imm_d_wcz();
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
 */
bool P2Asm::asm_fltnz()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_FLTNZ;
    return parse_imm_d_wcz();
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
 */
bool P2Asm::asm_fltrnd()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_FLTRND;
    return parse_imm_d_wcz();
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
 */
bool P2Asm::asm_fltnot()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_FLTNOT;
    return parse_imm_d_wcz();
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
 */
bool P2Asm::asm_drvl()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_DRVL;
    return parse_imm_d_wcz();
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
 */
bool P2Asm::asm_drvh()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_DRVH;
    return parse_imm_d_wcz();
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
 */
bool P2Asm::asm_drvc()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_DRVC;
    return parse_imm_d_wcz();
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
 */
bool P2Asm::asm_drvnc()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_DRVNC;
    return parse_imm_d_wcz();
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
 */
bool P2Asm::asm_drvz()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_DRVZ;
    return parse_imm_d_wcz();
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
 */
bool P2Asm::asm_drvnz()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_DRVNZ;
    return parse_imm_d_wcz();
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
 */
bool P2Asm::asm_drvrnd()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_DRVRND;
    return parse_imm_d_wcz();
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
 */
bool P2Asm::asm_drvnot()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_DRVNOT;
    return parse_imm_d_wcz();
}

/**
 * @brief Split every 4th bit of S into bytes of D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100000
 *
 * SPLITB  D
 *
 * D = {S[31], S[27], S[23], S[19], ... S[12], S[8], S[4], S[0]}.
 */
bool P2Asm::asm_splitb()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SPLITB;
    return parse_d();
}

/**
 * @brief Merge bits of bytes in S into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100001
 *
 * MERGEB  D
 *
 * D = {S[31], S[23], S[15], S[7], ... S[24], S[16], S[8], S[0]}.
 */
bool P2Asm::asm_mergeb()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_MERGEB;
    return parse_d();
}

/**
 * @brief Split bits of S into words of D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100010
 *
 * SPLITW  D
 *
 * D = {S[31], S[29], S[27], S[25], ... S[6], S[4], S[2], S[0]}.
 */
bool P2Asm::asm_splitw()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SPLITW;
    return parse_d();
}

/**
 * @brief Merge bits of words in S into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100011
 *
 * MERGEW  D
 *
 * D = {S[31], S[15], S[30], S[14], ... S[17], S[1], S[16], S[0]}.
 */
bool P2Asm::asm_mergew()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_MERGEW;
    return parse_d();
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
 */
bool P2Asm::asm_seussf()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SEUSSF;
    return parse_d();
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
 */
bool P2Asm::asm_seussr()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SEUSSR;
    return parse_d();
}

/**
 * @brief Squeeze 8:8:8 RGB value in S[31:8] into 5:6:5 value in D[15:0].
 *
 * EEEE 1101011 000 DDDDDDDDD 001100110
 *
 * RGBSQZ  D
 *
 * D = {15'b0, S[31:27], S[23:18], S[15:11]}.
 */
bool P2Asm::asm_rgbsqz()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_RGBSQZ;
    return parse_d();
}

/**
 * @brief Expand 5:6:5 RGB value in S[15:0] into 8:8:8 value in D[31:8].
 *
 * EEEE 1101011 000 DDDDDDDDD 001100111
 *
 * RGBEXP  D
 *
 * D = {S[15:11,15:13], S[10:5,10:9], S[4:0,4:2], 8'b0}.
 */
bool P2Asm::asm_rgbexp()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_RGBEXP;
    return parse_d();
}

/**
 * @brief Iterate D with xoroshiro32+ PRNG algorithm and put PRNG result into next instruction's S.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101000
 *
 * XORO32  D
 */
bool P2Asm::asm_xoro32()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_XORO32;
    return parse_d();
}

/**
 * @brief Reverse D bits.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101001
 *
 * REV     D
 *
 * D = D[0:31].
 */
bool P2Asm::asm_rev()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_REV;
    return parse_d();
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
 */
bool P2Asm::asm_rczr()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_RCZR;
    return parse_d_cz();
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
 */
bool P2Asm::asm_rczl()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_RCZL;
    return parse_d_cz();
}

/**
 * @brief Write 0 or 1 to D, according to  C.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101100
 *
 * WRC     D
 *
 * D = {31'b0,  C).
 */
bool P2Asm::asm_wrc()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_WRC;
    return parse_d();
}

/**
 * @brief Write 0 or 1 to D, according to !C.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101101
 *
 * WRNC    D
 *
 * D = {31'b0, !C).
 */
bool P2Asm::asm_wrnc()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_WRNC;
    return parse_d();
}

/**
 * @brief Write 0 or 1 to D, according to  Z.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101110
 *
 * WRZ     D
 *
 * D = {31'b0,  Z).
 */
bool P2Asm::asm_wrz()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_WRZ;
    return parse_d();
}

/**
 * @brief Write 0 or 1 to D, according to !Z.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101111
 *
 * WRNZ    D
 *
 * D = {31'b0, !Z).
 */
bool P2Asm::asm_wrnz()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_WRNZ_MODCZ;
    return parse_d();
}

/**
 * @brief Modify C and Z according to cccc and zzzz.
 *
 * EEEE 1101011 CZ1 0cccczzzz 001101111
 *
 * MODCZ   c,z      {WC/WZ/WCZ}
 *
 * C = cccc[{C,Z}], Z = zzzz[{C,Z}].
 */
bool P2Asm::asm_modcz()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_WRNZ_MODCZ;
    return parse_cccc_zzzz_wcz();
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
 */
bool P2Asm::asm_setscp()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_SETSCP;
    return parse_imm_d();
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
 */
bool P2Asm::asm_getscp()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_GETSCP;
    return parse_d();
}

/**
 * @brief Jump to A.
 *
 * EEEE 1101100 RAA AAAAAAAAA AAAAAAAAA
 *
 * JMP     #A
 *
 * If R = 1, PC += A, else PC = A.
 */
bool P2Asm::asm_jmp_abs()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_JMP;
    return parse_pc_abs();
}

/**
 * @brief Call to A by pushing {C, Z, 10'b0, PC[19:0]} onto stack.
 *
 * EEEE 1101101 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALL    #A
 *
 * If R = 1, PC += A, else PC = A.
 */
bool P2Asm::asm_call_abs()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_CALL_RET;
    return parse_pc_abs();
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRA++.
 *
 * EEEE 1101110 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLA   #A
 *
 * If R = 1, PC += A, else PC = A.
 */
bool P2Asm::asm_calla_abs()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_CALLA_RETA;
    return parse_pc_abs();
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRB++.
 *
 * EEEE 1101111 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLB   #A
 *
 * If R = 1, PC += A, else PC = A.
 */
bool P2Asm::asm_callb_abs()
{
    m_widx++;
    m_IR.op.inst = p2_OPSRC;
    m_IR.op.src = p2_OPSRC_CALLB_RETB;
    return parse_pc_abs();
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 1110000 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PA,#A
 *
 * If R = 1, PC += A, else PC = A.
 */
bool P2Asm::asm_calld_pa_abs()
{
    m_widx++;
    m_IR.op.inst = p2_CALLD_PA_ABS;
    return parse_pc_abs();
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 1110001 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PB,#A
 *
 * If R = 1, PC += A, else PC = A.
 */
bool P2Asm::asm_calld_pb_abs()
{
    m_widx++;
    m_IR.op.inst = p2_CALLD_PB_ABS;
    return parse_pc_abs();
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PTRA (per W).
 *
 * EEEE 1110010 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PTRA,#A
 *
 * If R = 1, PC += A, else PC = A.
 */
bool P2Asm::asm_calld_ptra_abs()
{
    m_widx++;
    m_IR.op.inst = p2_CALLD_PTRA_ABS;
    return parse_pc_abs();
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PTRB (per W).
 *
 * EEEE 1110011 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PTRB,#A
 *
 * If R = 1, PC += A, else PC = A.
 */
bool P2Asm::asm_calld_ptrb_abs()
{
    m_widx++;
    m_IR.op.inst = p2_CALLD_PTRB_ABS;
    return parse_pc_abs();
}

/**
 * @brief Get {12'b0, address[19:0]} into PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 11101WW RAA AAAAAAAAA AAAAAAAAA
 *
 * LOC     PA/PB/PTRA/PTRB,#A
 *
 * If R = 1, address = PC + A, else address = A.
 */
bool P2Asm::asm_loc()
{
    m_widx++;
    p2_token_e tok = m_words[m_widx].tok();
    bool success = false;
    switch (tok) {
    case t_PA:
        m_widx++;
        parse_comma();
        success = asm_loc_pa();
        break;
    case t_PB:
        m_widx++;
        parse_comma();
        success = asm_loc_pb();
        break;
    case t_PTRA:
        m_widx++;
        parse_comma();
        success = asm_loc_ptra();
        break;
    case t_PTRB:
        m_widx++;
        parse_comma();
        success = asm_loc_ptrb();
        break;
    default:
        m_error = tr("Invalid pointer type '%1'; expected one of %2.")
                  .arg(m_words.value(m_widx).str())
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
 */
bool P2Asm::asm_loc_pa()
{
    m_IR.op.inst = p2_LOC_PA;
    return parse_pc_abs();
}

/**
 * @brief Get {12'b0, address[19:0]} into PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 11101WW RAA AAAAAAAAA AAAAAAAAA
 *
 * LOC     PA/PB/PTRA/PTRB,#A
 *
 * If R = 1, address = PC + A, else address = A.
 */
bool P2Asm::asm_loc_pb()
{
    m_IR.op.inst = p2_LOC_PB;
    return parse_pc_abs();
}

/**
 * @brief Get {12'b0, address[19:0]} into PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 11101WW RAA AAAAAAAAA AAAAAAAAA
 *
 * LOC     PA/PB/PTRA/PTRB,#A
 *
 * If R = 1, address = PC + A, else address = A.
 */
bool P2Asm::asm_loc_ptra()
{
    m_IR.op.inst = p2_LOC_PTRA;
    return parse_pc_abs();
}

/**
 * @brief Get {12'b0, address[19:0]} into PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 11101WW RAA AAAAAAAAA AAAAAAAAA
 *
 * LOC     PA/PB/PTRA/PTRB,#A
 *
 * If R = 1, address = PC + A, else address = A.
 */
bool P2Asm::asm_loc_ptrb()
{
    m_IR.op.inst = p2_LOC_PTRB;
    return parse_pc_abs();
}

/**
 * @brief Queue #N[31:9] to be used as upper 23 bits for next #S occurrence, so that the next 9-bit #S will be augmented to 32 bits.
 *
 * EEEE 11110NN NNN NNNNNNNNN NNNNNNNNN
 *
 * AUGS    #N
 */
bool P2Asm::asm_augs()
{
    static const QVector<p2_inst7_e> augs = QVector<p2_inst7_e>()
                                            << p2_AUGS_00
                                            << p2_AUGS_01
                                            << p2_AUGS_10
                                            << p2_AUGS_11;
    m_widx++;
    m_IR.op.inst = p2_AUGS_00;
    return parse_imm23(augs);
}

/**
 * @brief Queue #N[31:9] to be used as upper 23 bits for next #D occurrence, so that the next 9-bit #D will be augmented to 32 bits.
 *
 * EEEE 11111NN NNN NNNNNNNNN NNNNNNNNN
 *
 * AUGD    #N
 */
bool P2Asm::asm_augd()
{
    static const QVector<p2_inst7_e> augd = QVector<p2_inst7_e>()
                                            << p2_AUGD_00
                                            << p2_AUGD_01
                                            << p2_AUGD_10
                                            << p2_AUGD_11;
    m_widx++;
    m_IR.op.inst = p2_AUGD_00;
    return parse_imm23(augd);
}
