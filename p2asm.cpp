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
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QLinkedList>
#include <QRegExp>
#include "p2asm.h"
#include "p2util.h"

#define DEBUG_EXPR  0 //! set to 1 to debug expression parsing
#define DEBUG_CON   0 //! set to 1 to debug CON section parsing
#define DEBUG_DAT   0 //! set to 1 to debug DAT section parsing

#define V33MODE     1

static const QString p2_section_dat = QStringLiteral("DAT");
static const QString p2_section_con = QStringLiteral("CON");
static const QString p2_section_pub = QStringLiteral("PUB");
static const QString p2_section_pri = QStringLiteral("PRI");
static const QString p2_section_var = QStringLiteral("VAR");

static const QString p2_prefix_bin_const = QStringLiteral("_BIN=");
static const QString p2_prefix_byt_const = QStringLiteral("_BYT=");
static const QString p2_prefix_dec_const = QStringLiteral("_DEC=");
static const QString p2_prefix_hex_const = QStringLiteral("_HEX=");
static const QString p2_prefix_str_const = QStringLiteral("_STR=");
static const QString p2_prefix_real_const = QStringLiteral("_REAL=");
static const QString p2_prefix_offs_const = QStringLiteral("_OFFS=");


#if DEBUG_EXPR
//! debug expression parsing
#define DBG_EXPR(x,...) do { if (m_pass > 1) { qDebug(x,__VA_ARGS__); } } while (0)
#else
#define DBG_EXPR(x,...)
#endif

#if DEBUG_CON
//! debug expression parsing
#define DBG_CON(x,...) do { if (m_pass > 1) { qDebug(x,__VA_ARGS__); } } while (0)
#else
#define DBG_CON(x,...)
#endif

/**
 * @brief P2Asm constructor
 * @param parent pointer to parent QObject
 */
P2Asm::P2Asm(QObject *parent)
    : QObject(parent)
    , m_v33mode(false)
    , m_pass(0)
    , m_pathname(".")
    , m_source()
    , m_listing()
    , m_hash_PC()
    , m_hash_IR()
    , m_hash_words()
    , m_hash_error()
    , m_symbols(P2SymbolTable(new P2SymbolTableClass()))
    , m_lineno(0)
    , m_in_curly(0)
    , m_lineptr(nullptr)
    , m_errors()
    , m_ORG(0)
    , m_ORGH(0)
    , m_advance(4)
    , m_IR()
    , m_data()
    , m_enum()
    , m_words()
    , m_instr()
    , m_symbol()
    , m_function()
    , m_section(dat_section)
    , m_cnt(0)
    , m_idx(0)
    , MEM()
{
    m_sections.insert(dat_section, p2_section_dat);
    m_sections.insert(con_section, p2_section_con);
    m_sections.insert(pub_section, p2_section_pub);
    m_sections.insert(pri_section, p2_section_pri);
    m_sections.insert(var_section, p2_section_var);
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
    m_pass = -1;
    m_symbols->clear();
    m_hash_words.clear();
    pass_clear();
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
    m_errors.clear();
    m_ORG = 0;
    m_ORGH = 0;
    m_advance = 0;
    m_IR.clear();
    m_data.clear();
    m_enum.clear();
    m_words.clear();
    m_instr = t_invalid;
    m_symbol.clear();
    m_function.clear();
    m_section = dat_section;
    m_cnt = 0;
    m_idx = 0;
    memset(MEM.BYTES, 0, sizeof(MEM.BYTES));
}

/**
 * @brief Clear the results of the last line
 */
void P2Asm::line_clear()
{
    // Parse the next line
    m_lineptr = &m_source[m_lineno];
    m_lineno += 1;

    // Reset some state
    m_errors.clear();
    m_words.clear();
    m_data.clear();
    m_advance = 0;
    m_IR.clear(0, p2_ORG_ORGH_t(m_ORG, m_ORGH));
}

/**
 * @brief Return the current assembler pass
 * @return value between 0 … 2
 */
int P2Asm::pass() const
{
    return m_pass;
}

/**
 * @brief Return the current path name for FILE includes
 * @return QString with the path
 */
const QString& P2Asm::pathname() const
{
    return m_pathname;
}

/**
 * @brief Return a const reference to the source lines
 * @return QStringList of source lnes
 */
const QStringList& P2Asm::source() const
{
    return m_source;
}

/**
 * @brief Return a specific source line
 * @param idx line index (0 … count() - 1)
 * @return QString with the source line
 */
const QString P2Asm::source(int idx) const
{
    return m_source.value(idx);
}

/**
 * @brief Set new source code
 * @param source const reference to a QStringList with source lines
 */
bool P2Asm::set_source(const QStringList& source)
{
    clear();
    m_source.clear();
    foreach(const QString& line, source)
        m_source += expand_tabs(line);
    return true;
}

/**
 * @brief Set new source code for line at %idx
 *
 * If %idx is greater or equal to m_source.count() the line is appended.
 * The assembler state is cleared.
 *
 * @param idx index in range 0 … m_source.count()
 * @param line source code line
 * @return true if changed, unchaged, or appended; false otherwise
 */
bool P2Asm::set_source(int idx, const QString& line)
{
    if (idx < 0)
        return false;
    QString source = expand_tabs(line);
    if (source == m_source[idx])
        return true;

    if (idx >= m_source.count())
        m_source.append(source);
    else
        m_source.replace(idx, source);
    m_pass = -1;
    clear();

    return true;
}

/**
 * @brief Return the number of source lines
 * @return line count
 */
int P2Asm::count() const
{
    return m_source.count();
}

/**
 * @brief Return a const reference to the has of ORG and ORGH values per line
 * @return const reference to the hash
 */
const p2_org_orgh_hash_t& P2Asm::PC_ORGH_hash() const
{
    return m_hash_PC;
}

/**
 * @brief Return a pair of values for ORG and ORGH for the line
 * @param lineno line number
 * @return pair of values
 */
p2_ORG_ORGH_t P2Asm::get_PC_ORGH(int lineno) const
{
    return m_hash_PC.value(lineno);
}

/**
 * @brief Return true, if a line number has ORG and ORGH values
 * @param lineno line number
 * @return true if available, or false otherwise
 */
bool P2Asm::has_PC_ORGH(int lineno) const
{
    return m_hash_PC.contains(lineno);
}

/**
 * @brief Return a const reference to the P2Opcode (opcode, data, equates) hash
 * @return const reference to the instruction register
 */
const p2_opcode_hash_t& P2Asm::IR_hash() const
{
    return m_hash_IR;
}

/**
 * @brief Return the P2Opcode for the line %lineno
 * @param lineno line number
 * @return P2Opcode for that line
 */
P2Opcode P2Asm::get_IR(int lineno) const
{
    return m_hash_IR.value(lineno);
}

/**
 * @brief Return true, if the line number %lineno has a P2Opcode
 * @param lineno line number
 * @return true if available, or false otherwise
 */
bool P2Asm::has_IR(int lineno) const
{
    return m_hash_IR.contains(lineno);
}

/**
 * @brief P2Asm::words_hash
 * @return
 */
const p2_words_hash_t& P2Asm::words_hash() const
{
    return m_hash_words;
}

P2Words P2Asm::words(int lineno) const
{
    return m_hash_words.value(lineno, P2Words());
}

bool P2Asm::has_words(int lineno) const
{
    return m_hash_words.contains(lineno);
}

const p2_error_hash_t& P2Asm::error_hash() const
{
    return m_hash_error;
}

QStringList P2Asm::errors(int lineno) const
{
    return m_hash_error.value(lineno);
}

const QStringList& P2Asm::listing() const
{
    return m_listing;
}

const P2SymbolTable& P2Asm::symbols() const
{
    return m_symbols;
}

p2_cond_e P2Asm::conditional()
{
    p2_cond_e result = cc_always;
    p2_token_e cond = curr_tok();
    if (Token.is_type(cond, tm_conditional)) {
        result = Token.conditional(cond, cc_always);
        next();
    }
    return result;
}

p2_cond_e P2Asm::parse_modcz()
{
    p2_token_e cond = curr_tok();
    if (Token.is_type(cond, tm_modcz_param)) {
        p2_cond_e result = Token.modcz_param(cond, cc_clr);
        next();
        return result;
    }
    P2Atom atom = parse_expression();
    if (!atom.isValid()) {
        m_errors += tr("Expected MODCZ param or immediate.");
        emit Error(m_pass, m_lineno, m_errors.last());
        return cc_clr;
    }
    p2_LONG value = atom.get_long();
    if (value > 15) {
        m_errors += tr("Expected MODCZ immediate in range 0…15.");
        emit Error(m_pass, m_lineno, m_errors.last());
        return cc_clr;
    }
    return static_cast<p2_cond_e>(value);
}

/**
 * @brief Tokenize a line
 *
 * In pass #1 tokenize the line.
 * In pass #2 used the previously generated tokens for that line.
 *
 * @return true on success (currently always)
 */
bool P2Asm::tokenize()
{
    Q_ASSERT(m_in_curly >= 0);
    if (m_pass < 2) {
        m_words = Token.tokenize(&m_source[m_lineno-1], m_lineno, m_in_curly);
    } else {
        m_words = m_hash_words.value(m_lineno);
    }
    m_cnt = m_words.count();
    m_idx = 0;

    return true;
}

/**
 * @brief Return the current P2Word
 * @return const reference to the current word
 */
const P2Word& P2Asm::curr_word() const
{
    static const P2Word empty = P2Word();
    if (m_idx >= m_cnt)
        return empty;
    return m_words[m_idx];
}

/**
 * @brief Return the current P2Word's QStringRef
 * @return const reference to the current word
 */
const QStringRef P2Asm::curr_ref() const
{
    return curr_word().ref();
}

/**
 * @brief Return the current P2Word's QString
 * @return const reference to the current word
 */
const QString P2Asm::curr_str() const
{
    return curr_word().str();
}

/**
 * @brief Return the current P2Word's token
 * @return token enumeration value
 */
p2_token_e P2Asm::curr_tok() const
{
    return curr_word().tok();
}

/**
 * @brief Go to the next P2Word
 * @return true if more words, or false at end-of-line
 */
bool P2Asm::next()
{
    return ++m_idx < m_cnt;
}

/**
 * @brief Go to the previous P2Word
 * @return true if >= 0, or false if off scope
 */
bool P2Asm::prev()
{
    return --m_idx >= 0;
}

/**
 * @brief Define a symbol value
 * @param symbol string for the symbol
 * @param atom initial or new value for the symbol
 * @return true on success, or false if already defined
 */
bool P2Asm::define_symbol(const QString& symbol, const P2Atom& atom)
{
    const P2Word& word = curr_word();
    P2Symbol sym = m_symbols->symbol(symbol);
    next();

    if (sym.isEmpty()) {
        // Not defined yet
        m_symbols->insert(symbol, atom.value());
        m_symbols->add_reference(m_lineno, symbol, word);
        return true;
    }

    if (m_pass > 1) {
        // Redefine in pass 2
        m_symbols->set_value(symbol, atom.value());
        return true;
    }

    if (sym.isNull()) {
        // Invalid
        return true;
    }

    // Already defined
    const P2Union& old_value = sym.value();
    if (ut_Real == old_value.type()) {
        const p2_REAL value = old_value.get_real();
        m_errors += tr("Symbol '%1' already defined in line #%2 (Value: %3).")
                  .arg(symbol)
                  .arg(sym.definition().lineno())
                  .arg(value, 2, 'f');
    } else {
        const p2_LONG value = old_value.get_long();
        m_errors += tr("Symbol '%1' already defined in line #%2 (Value: %3, $%4, %%5).")
                    .arg(symbol)
                    .arg(sym.definition().lineno())
                    .arg(value)
                    .arg(value, 6, 16, QChar('0'))
                    .arg(value, 32, 2, QChar('0'));
    }
    return false;
}

bool P2Asm::assemble_con_section()
{
    while (m_idx < m_cnt) {

        switch (curr_tok()) {
        case t_comment:
        case t_comment_eol:
        case t_comment_lcurly:
        case t_comment_rcurly:
            next();
            continue;

        case t_locsym:
            // append local name to section::function / section
            m_symbol = find_locsym(m_section, curr_str());
            break;

        case t_symbol:
            // append global name to section::symbol
            m_symbol = find_symbol(m_section, curr_str());
            m_function.insert(m_section, curr_str());
            break;

        default:
            m_symbol.clear();
        }

        if (!m_symbol.isEmpty()) {
            // defining a symbol with the current enumeration value
            define_symbol(m_symbol, m_enum);
            // increase enumerator
            m_enum.unary_inc(1);
        }
        break;
    }

    // Return if no more words/tokens were found
    if (!skip_comments())
        return true;

    m_advance = 0;      // The instruction does not advance
    m_IR.clear();       // Clear the opcode

    // Expect a token for an instruction
    bool success = false;
    while (skip_comments()) {
        m_instr = curr_tok();
        switch (m_instr) {
        case t_comment:
            next();
            break;

        case t_DAT:
            success = asm_DAT();
            success = assemble_dat_section();
            break;

        case t_CON: // redundant
            success = asm_CON();
            break;

        case t_PUB:
            success = asm_PUB();
            success = assemble_dat_section();
            break;

        case t_PRI:
            success = asm_PRI();
            success = assemble_dat_section();
            break;

        case t_VAR:
            success = asm_VAR();
            success = assemble_dat_section();
            break;

        case t_ASSIGN:
            success = asm_assign();
            break;

        case t_IMMEDIATE:
            success = asm_enum_initial();
            break;

        case t_COMMA:
            success = asm_enum_continue();
            break;

        default:
            m_errors += tr("Not an CON section token '%1'.")
                      .arg(curr_str());
            emit Error(m_pass, m_lineno, m_errors.last());
            m_idx = m_cnt;
        }
    }
    return success;
}

bool P2Asm::assemble_dat_section()
{
    while (m_idx < m_cnt) {
        switch (curr_tok()) {
        case t_comment:
        case t_comment_eol:
        case t_comment_lcurly:
        case t_comment_rcurly:
            next();
            continue;

        case t_locsym:
            // append local name to section::function / section
            m_symbol = find_locsym(m_section, curr_str());
            break;

        case t_symbol:
            // append global name to section::symbol
            m_symbol = find_symbol(m_section, curr_str());
            m_function.insert(m_section, curr_str());
            break;

        default:
            m_symbol.clear();
        }

        if (!m_symbol.isEmpty()) {
            const p2_LONG org = m_ORG < HUB_ADDR0 ? m_ORG / 4 : m_ORG;
            // defining a symbol at the current PC
            P2Atom atom(org);
            atom.set_type(ut_Addr);
            define_symbol(m_symbol, atom);
        }
        break;
    }

    // Return if no more words/tokens were found
    if (!skip_comments()) {
        m_IR.set_as_IR(false);
        results();
        return true;
    }

    m_advance = 4;      // Assume the instruction advances by 4 bytes
    m_IR.clear();       // Clear the opcode
    m_IR.set_as_IR(true);  // Assume the instruction emits IR

    // Conditional execution prefix
    m_IR.set_cond(conditional());

    // Expect a token for an instruction
    bool success = false;
    while (skip_comments()) {
        m_instr = m_words.value(m_idx).tok();
        switch (m_instr) {
        case t_comment:
            next();
            break;

        case t_DAT:
            success = asm_DAT();
            break;

        case t_CON:
            success = asm_CON();
            success = assemble_con_section();
            break;

        case t_PUB:
            success = asm_PUB();
            break;

        case t_PRI:
            success = asm_PRI();
            break;

        case t_VAR:
            success = asm_VAR();
            break;

        case t_ABS:
            success = asm_ABS();
            break;

        case t_ADD:
            success = asm_ADD();
            break;

        case t_ADDCT1:
            success = asm_ADDCT1();
            break;

        case t_ADDCT2:
            success = asm_ADDCT2();
            break;

        case t_ADDCT3:
            success = asm_ADDCT3();
            break;

        case t_ADDPIX:
            success = asm_ADDPIX();
            break;

        case t_ADDS:
            success = asm_ADDS();
            break;

        case t_ADDSX:
            success = asm_ADDSX();
            break;

        case t_ADDX:
            success = asm_ADDX();
            break;

        case t_AKPIN:
            success = asm_AKPIN();
            break;

        case t_ALLOWI:
            success = asm_ALLOWI();
            break;

        case t_ALTB:
            success = asm_ALTB();
            break;

        case t_ALTD:
            success = asm_ALTD();
            break;

        case t_ALTGB:
            success = asm_ALTGB();
            break;

        case t_ALTGN:
            success = asm_ALTGN();
            break;

        case t_ALTGW:
            success = asm_ALTGW();
            break;

        case t_ALTI:
            success = asm_ALTI();
            break;

        case t_ALTR:
            success = asm_ALTR();
            break;

        case t_ALTS:
            success = asm_ALTS();
            break;

        case t_ALTSB:
            success = asm_ALTSB();
            break;

        case t_ALTSN:
            success = asm_ALTSN();
            break;

        case t_ALTSW:
            success = asm_ALTSW();
            break;

        case t_AND:
            success = asm_AND();
            break;

        case t_ANDN:
            success = asm_ANDN();
            break;

        case t_AUGD:
            success = asm_AUGD();
            break;

        case t_AUGS:
            success = asm_AUGS();
            break;

        case t_BITC:
            success = asm_BITC();
            break;

        case t_BITH:
            success = asm_BITH();
            break;

        case t_BITL:
            success = asm_BITL();
            break;

        case t_BITNC:
            success = asm_BITNC();
            break;

        case t_BITNOT:
            success = asm_BITNOT();
            break;

        case t_BITNZ:
            success = asm_BITNZ();
            break;

        case t_BITRND:
            success = asm_BITRND();
            break;

        case t_BITZ:
            success = asm_BITZ();
            break;

        case t_BLNPIX:
            success = asm_BLNPIX();
            break;

        case t_BMASK:
            success = asm_BMASK();
            break;

        case t_BRK:
            success = asm_BRK();
            break;

        case t_CALL:
            success = asm_CALL();
            break;

        case t_CALLA:
            success = asm_CALLA();
            break;

        case t_CALLB:
            success = asm_CALLB();
            break;

        case t_CALLD:
            success = asm_CALLD();
            break;

        case t_CALLPA:
            success = asm_CALLPA();
            break;

        case t_CALLPB:
            success = asm_CALLPB();
            break;

        case t_CMP:
            success = asm_CMP();
            break;

        case t_CMPM:
            success = asm_CMPM();
            break;

        case t_CMPR:
            success = asm_CMPR();
            break;

        case t_CMPS:
            success = asm_CMPS();
            break;

        case t_CMPSUB:
            success = asm_CMPSUB();
            break;

        case t_CMPSX:
            success = asm_CMPSX();
            break;

        case t_CMPX:
            success = asm_CMPX();
            break;

        case t_COGATN:
            success = asm_COGATN();
            break;

        case t_COGBRK:
            success = asm_COGBRK();
            break;

        case t_COGID:
            success = asm_COGID();
            break;

        case t_COGINIT:
            success = asm_COGINIT();
            break;

        case t_COGSTOP:
            success = asm_COGSTOP();
            break;

        case t_CRCBIT:
            success = asm_CRCBIT();
            break;

        case t_CRCNIB:
            success = asm_CRCNIB();
            break;

        case t_DECMOD:
            success = asm_DECMOD();
            break;

        case t_DECOD:
            success = asm_DECOD();
            break;

        case t_DIRC:
            success = asm_DIRC();
            break;

        case t_DIRH:
            success = asm_DIRH();
            break;

        case t_DIRL:
            success = asm_DIRL();
            break;

        case t_DIRNC:
            success = asm_DIRNC();
            break;

        case t_DIRNOT:
            success = asm_DIRNOT();
            break;

        case t_DIRNZ:
            success = asm_DIRNZ();
            break;

        case t_DIRRND:
            success = asm_DIRRND();
            break;

        case t_DIRZ:
            success = asm_DIRZ();
            break;

        case t_DJF:
            success = asm_DJF();
            break;

        case t_DJNF:
            success = asm_DJNF();
            break;

        case t_DJNZ:
            success = asm_DJNZ();
            break;

        case t_DJZ:
            success = asm_DJZ();
            break;

        case t_DRVC:
            success = asm_DRVC();
            break;

        case t_DRVH:
            success = asm_DRVH();
            break;

        case t_DRVL:
            success = asm_DRVL();
            break;

        case t_DRVNC:
            success = asm_DRVNC();
            break;

        case t_DRVNOT:
            success = asm_DRVNOT();
            break;

        case t_DRVNZ:
            success = asm_DRVNZ();
            break;

        case t_DRVRND:
            success = asm_DRVRND();
            break;

        case t_DRVZ:
            success = asm_DRVZ();
            break;

        case t_ENCOD:
            success = asm_ENCOD();
            break;

        case t_EXECF:
            success = asm_EXECF();
            break;

        case t_FBLOCK:
            success = asm_FBLOCK();
            break;

        case t_FGE:
            success = asm_FGE();
            break;

        case t_FGES:
            success = asm_FGES();
            break;

        case t_FLE:
            success = asm_FLE();
            break;

        case t_FLES:
            success = asm_FLES();
            break;

        case t_FLTC:
            success = asm_FLTC();
            break;

        case t_FLTH:
            success = asm_FLTH();
            break;

        case t_FLTL:
            success = asm_FLTL();
            break;

        case t_FLTNC:
            success = asm_FLTNC();
            break;

        case t_FLTNOT:
            success = asm_FLTNOT();
            break;

        case t_FLTNZ:
            success = asm_FLTNZ();
            break;

        case t_FLTRND:
            success = asm_FLTRND();
            break;

        case t_FLTZ:
            success = asm_FLTZ();
            break;

        case t_GETBRK:
            success = asm_GETBRK();
            break;

        case t_GETBYTE:
            success = asm_GETBYTE();
            break;

        case t_GETCT:
            success = asm_GETCT();
            break;

        case t_GETNIB:
            success = asm_GETNIB();
            break;

        case t_GETPTR:
            success = asm_GETPTR();
            break;

        case t_GETQX:
            success = asm_GETQX();
            break;

        case t_GETQY:
            success = asm_GETQY();
            break;

        case t_GETRND:
            success = asm_GETRND();
            break;

        case t_GETSCP:
            success = asm_GETSCP();
            break;

        case t_GETWORD:
            success = asm_GETWORD();
            break;

        case t_GETXACC:
            success = asm_GETXACC();
            break;

        case t_HUBSET:
            success = asm_HUBSET();
            break;

        case t_IJNZ:
            success = asm_IJNZ();
            break;

        case t_IJZ:
            success = asm_IJZ();
            break;

        case t_INCMOD:
            success = asm_INCMOD();
            break;

        case t_JATN:
            success = asm_JATN();
            break;

        case t_JCT1:
            success = asm_JCT1();
            break;

        case t_JCT2:
            success = asm_JCT2();
            break;

        case t_JCT3:
            success = asm_JCT3();
            break;

        case t_JFBW:
            success = asm_JFBW();
            break;

        case t_JINT:
            success = asm_JINT();
            break;

        case t_JMP:
            success = asm_JMP();
            break;

        case t_JMPREL:
            success = asm_JMPREL();
            break;

        case t_JNATN:
            success = asm_JNATN();
            break;

        case t_JNCT1:
            success = asm_JNCT1();
            break;

        case t_JNCT2:
            success = asm_JNCT2();
            break;

        case t_JNCT3:
            success = asm_JNCT3();
            break;

        case t_JNFBW:
            success = asm_JNFBW();
            break;

        case t_JNINT:
            success = asm_JNINT();
            break;

        case t_JNPAT:
            success = asm_JNPAT();
            break;

        case t_JNQMT:
            success = asm_JNQMT();
            break;

        case t_JNSE1:
            success = asm_JNSE1();
            break;

        case t_JNSE2:
            success = asm_JNSE2();
            break;

        case t_JNSE3:
            success = asm_JNSE3();
            break;

        case t_JNSE4:
            success = asm_JNSE4();
            break;

        case t_JNXFI:
            success = asm_JNXFI();
            break;

        case t_JNXMT:
            success = asm_JNXMT();
            break;

        case t_JNXRL:
            success = asm_JNXRL();
            break;

        case t_JNXRO:
            success = asm_JNXRO();
            break;

        case t_JPAT:
            success = asm_JPAT();
            break;

        case t_JQMT:
            success = asm_JQMT();
            break;

        case t_JSE1:
            success = asm_JSE1();
            break;

        case t_JSE2:
            success = asm_JSE2();
            break;

        case t_JSE3:
            success = asm_JSE3();
            break;

        case t_JSE4:
            success = asm_JSE4();
            break;

        case t_JXFI:
            success = asm_JXFI();
            break;

        case t_JXMT:
            success = asm_JXMT();
            break;

        case t_JXRL:
            success = asm_JXRL();
            break;

        case t_JXRO:
            success = asm_JXRO();
            break;

        case t_LOC:
            success = asm_LOC();
            break;

        case t_LOCKNEW:
            success = asm_LOCKNEW();
            break;

        case t_LOCKREL:
            success = asm_LOCKREL();
            break;

        case t_LOCKRET:
            success = asm_LOCKRET();
            break;

        case t_LOCKTRY:
            success = asm_LOCKTRY();
            break;

        case t_MERGEB:
            success = asm_MERGEB();
            break;

        case t_MERGEW:
            success = asm_MERGEW();
            break;

        case t_MIXPIX:
            success = asm_MIXPIX();
            break;

        case t_MODCZ:
            success = asm_MODCZ();
            break;

        case t_MODC:
            success = asm_MODC();
            break;

        case t_MODZ:
            success = asm_MODZ();
            break;

        case t_MOV:
            success = asm_MOV();
            break;

        case t_MOVBYTS:
            success = asm_MOVBYTS();
            break;

        case t_MUL:
            success = asm_MUL();
            break;

        case t_MULPIX:
            success = asm_MULPIX();
            break;

        case t_MULS:
            success = asm_MULS();
            break;

        case t_MUXC:
            success = asm_MUXC();
            break;

        case t_MUXNC:
            success = asm_MUXNC();
            break;

        case t_MUXNIBS:
            success = asm_MUXNIBS();
            break;

        case t_MUXNITS:
            success = asm_MUXNITS();
            break;

        case t_MUXNZ:
            success = asm_MUXNZ();
            break;

        case t_MUXQ:
            success = asm_MUXQ();
            break;

        case t_MUXZ:
            success = asm_MUXZ();
            break;

        case t_NEG:
            success = asm_NEG();
            break;

        case t_NEGC:
            success = asm_NEGC();
            break;

        case t_NEGNC:
            success = asm_NEGNC();
            break;

        case t_NEGNZ:
            success = asm_NEGNZ();
            break;

        case t_NEGZ:
            success = asm_NEGZ();
            break;

        case t_NIXINT1:
            success = asm_NIXINT1();
            break;

        case t_NIXINT2:
            success = asm_NIXINT2();
            break;

        case t_NIXINT3:
            success = asm_NIXINT3();
            break;

        case t_NOP:
            success = asm_NOP();
            break;

        case t_NOT:
            success = asm_NOT();
            break;

        case t_ONES:
            success = asm_ONES();
            break;

        case t_OR:
            success = asm_OR();
            break;

        case t_OUTC:
            success = asm_OUTC();
            break;

        case t_OUTH:
            success = asm_OUTH();
            break;

        case t_OUTL:
            success = asm_OUTL();
            break;

        case t_OUTNC:
            success = asm_OUTNC();
            break;

        case t_OUTNOT:
            success = asm_OUTNOT();
            break;

        case t_OUTNZ:
            success = asm_OUTNZ();
            break;

        case t_OUTRND:
            success = asm_OUTRND();
            break;

        case t_OUTZ:
            success = asm_OUTZ();
            break;

        case t_POLLATN:
            success = asm_POLLATN();
            break;

        case t_POLLCT1:
            success = asm_POLLCT1();
            break;

        case t_POLLCT2:
            success = asm_POLLCT2();
            break;

        case t_POLLCT3:
            success = asm_POLLCT3();
            break;

        case t_POLLFBW:
            success = asm_POLLFBW();
            break;

        case t_POLLINT:
            success = asm_POLLINT();
            break;

        case t_POLLPAT:
            success = asm_POLLPAT();
            break;

        case t_POLLQMT:
            success = asm_POLLQMT();
            break;

        case t_POLLSE1:
            success = asm_POLLSE1();
            break;

        case t_POLLSE2:
            success = asm_POLLSE2();
            break;

        case t_POLLSE3:
            success = asm_POLLSE3();
            break;

        case t_POLLSE4:
            success = asm_POLLSE4();
            break;

        case t_POLLXFI:
            success = asm_POLLXFI();
            break;

        case t_POLLXMT:
            success = asm_POLLXMT();
            break;

        case t_POLLXRL:
            success = asm_POLLXRL();
            break;

        case t_POLLXRO:
            success = asm_POLLXRO();
            break;

        case t_POP:
            success = asm_POP();
            break;

        case t_POPA:
            success = asm_POPA();
            break;

        case t_POPB:
            success = asm_POPB();
            break;

        case t_PUSH:
            success = asm_PUSH();
            break;

        case t_PUSHA:
            success = asm_PUSHA();
            break;

        case t_PUSHB:
            success = asm_PUSHB();
            break;

        case t_QDIV:
            success = asm_QDIV();
            break;

        case t_QEXP:
            success = asm_QEXP();
            break;

        case t_QFRAC:
            success = asm_QFRAC();
            break;

        case t_QLOG:
            success = asm_QLOG();
            break;

        case t_QMUL:
            success = asm_QMUL();
            break;

        case t_QROTATE:
            success = asm_QROTATE();
            break;

        case t_QSQRT:
            success = asm_QSQRT();
            break;

        case t_QVECTOR:
            success = asm_QVECTOR();
            break;

        case t_RCL:
            success = asm_RCL();
            break;

        case t_RCR:
            success = asm_RCR();
            break;

        case t_RCZL:
            success = asm_RCZL();
            break;

        case t_RCZR:
            success = asm_RCZR();
            break;

        case t_RDBYTE:
            success = asm_RDBYTE();
            break;

        case t_RDFAST:
            success = asm_RDFAST();
            break;

        case t_RDLONG:
            success = asm_RDLONG();
            break;

        case t_RDLUT:
            success = asm_RDLUT();
            break;

        case t_RDPIN:
            success = asm_RDPIN();
            break;

        case t_RDWORD:
            success = asm_RDWORD();
            break;

        case t_REP:
            success = asm_REP();
            break;

        case t_RESI0:
            success = asm_RESI0();
            break;

        case t_RESI1:
            success = asm_RESI1();
            break;

        case t_RESI2:
            success = asm_RESI2();
            break;

        case t_RESI3:
            success = asm_RESI3();
            break;

        case t_RET:
            success = asm_RET();
            break;

        case t_RETA:
            success = asm_RETA();
            break;

        case t_RETB:
            success = asm_RETB();
            break;

        case t_RETI0:
            success = asm_RETI0();
            break;

        case t_RETI1:
            success = asm_RETI1();
            break;

        case t_RETI2:
            success = asm_RETI2();
            break;

        case t_RETI3:
            success = asm_RETI3();
            break;

        case t_REV:
            success = asm_REV();
            break;

        case t_RFBYTE:
            success = asm_RFBYTE();
            break;

        case t_RFLONG:
            success = asm_RFLONG();
            break;

        case t_RFVAR:
            success = asm_RFVAR();
            break;

        case t_RFVARS:
            success = asm_RFVARS();
            break;

        case t_RFWORD:
            success = asm_RFWORD();
            break;

        case t_RGBEXP:
            success = asm_RGBEXP();
            break;

        case t_RGBSQZ:
            success = asm_RGBSQZ();
            break;

        case t_ROL:
            success = asm_ROL();
            break;

        case t_ROLBYTE:
            success = asm_ROLBYTE();
            break;

        case t_ROLNIB:
            success = asm_ROLNIB();
            break;

        case t_ROLWORD:
            success = asm_ROLWORD();
            break;

        case t_ROR:
            success = asm_ROR();
            break;

        case t_RQPIN:
            success = asm_RQPIN();
            break;

        case t_SAL:
            success = asm_SAL();
            break;

        case t_SAR:
            success = asm_SAR();
            break;

        case t_SCA:
            success = asm_SCA();
            break;

        case t_SCAS:
            success = asm_SCAS();
            break;

        case t_SETBYTE:
            success = asm_SETBYTE();
            break;

        case t_SETCFRQ:
            success = asm_SETCFRQ();
            break;

        case t_SETCI:
            success = asm_SETCI();
            break;

        case t_SETCMOD:
            success = asm_SETCMOD();
            break;

        case t_SETCQ:
            success = asm_SETCQ();
            break;

        case t_SETCY:
            success = asm_SETCY();
            break;

        case t_SETD:
            success = asm_SETD();
            break;

        case t_SETDACS:
            success = asm_SETDACS();
            break;

        case t_SETINT1:
            success = asm_SETINT1();
            break;

        case t_SETINT2:
            success = asm_SETINT2();
            break;

        case t_SETINT3:
            success = asm_SETINT3();
            break;

        case t_SETLUTS:
            success = asm_SETLUTS();
            break;

        case t_SETNIB:
            success = asm_SETNIB();
            break;

        case t_SETPAT:
            success = asm_SETPAT();
            break;

        case t_SETPIV:
            success = asm_SETPIV();
            break;

        case t_SETPIX:
            success = asm_SETPIX();
            break;

        case t_SETQ:
            success = asm_SETQ();
            break;

        case t_SETQ2:
            success = asm_SETQ2();
            break;

        case t_SETR:
            success = asm_SETR();
            break;

        case t_SETS:
            success = asm_SETS();
            break;

        case t_SETSCP:
            success = asm_SETSCP();
            break;

        case t_SETSE1:
            success = asm_SETSE1();
            break;

        case t_SETSE2:
            success = asm_SETSE2();
            break;

        case t_SETSE3:
            success = asm_SETSE3();
            break;

        case t_SETSE4:
            success = asm_SETSE4();
            break;

        case t_SETWORD:
            success = asm_SETWORD();
            break;

        case t_SETXFRQ:
            success = asm_SETXFRQ();
            break;

        case t_SEUSSF:
            success = asm_SEUSSF();
            break;

        case t_SEUSSR:
            success = asm_SEUSSR();
            break;

        case t_SHL:
            success = asm_SHL();
            break;

        case t_SHR:
            success = asm_SHR();
            break;

        case t_SIGNX:
            success = asm_SIGNX();
            break;

        case t_SKIP:
            success = asm_SKIP();
            break;

        case t_SKIPF:
            success = asm_SKIPF();
            break;

        case t_SPLITB:
            success = asm_SPLITB();
            break;

        case t_SPLITW:
            success = asm_SPLITW();
            break;

        case t_STALLI:
            success = asm_STALLI();
            break;

        case t_SUB:
            success = asm_SUB();
            break;

        case t_SUBR:
            success = asm_SUBR();
            break;

        case t_SUBS:
            success = asm_SUBS();
            break;

        case t_SUBSX:
            success = asm_SUBSX();
            break;

        case t_SUBX:
            success = asm_SUBX();
            break;

        case t_SUMC:
            success = asm_SUMC();
            break;

        case t_SUMNC:
            success = asm_SUMNC();
            break;

        case t_SUMNZ:
            success = asm_SUMNZ();
            break;

        case t_SUMZ:
            success = asm_SUMZ();
            break;

        case t_TEST:
            success = asm_TEST();
            break;

        case t_TESTB:
            success = asm_TESTB();
            break;

        case t_TESTBN:
            success = asm_TESTBN();
            break;

        case t_TESTN:
            success = asm_TESTN();
            break;

        case t_TESTP:
            success = asm_TESTP();
            break;

        case t_TESTPN:
            success = asm_TESTPN();
            break;

        case t_TJF:
            success = asm_TJF();
            break;

        case t_TJNF:
            success = asm_TJNF();
            break;

        case t_TJNS:
            success = asm_TJNS();
            break;

        case t_TJNZ:
            success = asm_TJNZ();
            break;

        case t_TJS:
            success = asm_TJS();
            break;

        case t_TJV:
            success = asm_TJV();
            break;

        case t_TJZ:
            success = asm_TJZ();
            break;

        case t_TRGINT1:
            success = asm_TRGINT1();
            break;

        case t_TRGINT2:
            success = asm_TRGINT2();
            break;

        case t_TRGINT3:
            success = asm_TRGINT3();
            break;

        case t_WAITATN:
            success = asm_WAITATN();
            break;

        case t_WAITCT1:
            success = asm_WAITCT1();
            break;

        case t_WAITCT2:
            success = asm_WAITCT2();
            break;

        case t_WAITCT3:
            success = asm_WAITCT3();
            break;

        case t_WAITFBW:
            success = asm_WAITFBW();
            break;

        case t_WAITINT:
            success = asm_WAITINT();
            break;

        case t_WAITPAT:
            success = asm_WAITPAT();
            break;

        case t_WAITSE1:
            success = asm_WAITSE1();
            break;

        case t_WAITSE2:
            success = asm_WAITSE2();
            break;

        case t_WAITSE3:
            success = asm_WAITSE3();
            break;

        case t_WAITSE4:
            success = asm_WAITSE4();
            break;

        case t_WAITX:
            success = asm_WAITX();
            break;

        case t_WAITXFI:
            success = asm_WAITXFI();
            break;

        case t_WAITXMT:
            success = asm_WAITXMT();
            break;

        case t_WAITXRL:
            success = asm_WAITXRL();
            break;

        case t_WAITXRO:
            success = asm_WAITXRO();
            break;

        case t_WFBYTE:
            success = asm_WFBYTE();
            break;

        case t_WFLONG:
            success = asm_WFLONG();
            break;

        case t_WFWORD:
            success = asm_WFWORD();
            break;

        case t_WMLONG:
            success = asm_WMLONG();
            break;

        case t_WRBYTE:
            success = asm_WRBYTE();
            break;

        case t_WRC:
            success = asm_WRC();
            break;

        case t_WRFAST:
            success = asm_WRFAST();
            break;

        case t_WRLONG:
            success = asm_WRLONG();
            break;

        case t_WRLUT:
            success = asm_WRLUT();
            break;

        case t_WRNC:
            success = asm_WRNC();
            break;

        case t_WRNZ:
            success = asm_WRNZ();
            break;

        case t_WRPIN:
            success = asm_WRPIN();
            break;

        case t_WRWORD:
            success = asm_WRWORD();
            break;

        case t_WRZ:
            success = asm_WRZ();
            break;

        case t_WXPIN:
            success = asm_WXPIN();
            break;

        case t_WYPIN:
            success = asm_WYPIN();
            break;

        case t_XCONT:
            success = asm_XCONT();
            break;

        case t_XINIT:
            success = asm_XINIT();
            break;

        case t_XOR:
            success = asm_XOR();
            break;

        case t_XORO32:
            success = asm_XORO32();
            break;

        case t_XSTOP:
            success = asm_XSTOP();
            break;

        case t_XZERO:
            success = asm_XZERO();
            break;

        case t_ZEROX:
            success = asm_ZEROX();
            break;

        case t_BYTE:
            success = asm_BYTE();
            break;

        case t_WORD:
            success = asm_WORD();
            break;

        case t_LONG:
            success = asm_LONG();
            break;

        case t_FILE:
            success = asm_FILE();
            break;

        case t_RES:
            success = asm_RES();
            break;

        case t_ALIGNW:
            success = asm_ALIGNW();
            break;

        case t_ALIGNL:
            success = asm_ALIGNL();
            break;

        case t_ORG:
            success = asm_ORG();
            break;

        case t_ORGF:
            success = asm_ORGF();
            break;

        case t_ORGH:
            success = asm_ORGH();
            break;

        case t_FIT:
            success = asm_FIT();
            break;

        default:
            if (Token.is_type(m_instr, tm_mnemonic)) {
                // Missing handling of an instruction token
                m_errors += tr("Missing handling of instruction token '%1'.")
                      .arg(Token.string(m_instr));
                emit Error(m_pass, m_lineno, m_errors.last());
                m_idx = m_cnt;
                break;
            }

            if (Token.is_type(m_instr, tm_constant)) {
                // Unexpected constant token
                m_errors += tr("Constant '%1' used as an instruction.")
                          .arg(Token.string(m_instr));
                emit Error(m_pass, m_lineno, m_errors.last());
                m_idx = m_cnt;
                break;
            }

            if (Token.is_type(m_instr, tm_conditional)) {
                // Unexpected conditional token
                m_errors += tr("Extraneous conditional '%1'.")
                          .arg(Token.string(m_instr));
                emit Error(m_pass, m_lineno, m_errors.last());
                m_idx = m_cnt;
                break;
            }

            if (Token.is_type(m_instr, tm_modcz_param)) {
                // Unexpected MODCZ parameter token
                m_errors += tr("Extraneous MODCZ parameter '%1'.")
                          .arg(Token.string(m_instr));
                emit Error(m_pass, m_lineno, m_errors.last());
                m_idx = m_cnt;
                break;
            }

            m_errors += tr("Not an instruction token '%1'.")
                      .arg(Token.string(m_instr));
            emit Error(m_pass, m_lineno, m_errors.last());
            m_idx = m_cnt;
        }
    }
    return success;
}

/**
 * @brief Assemble a QStringList of lines of SPIN2 source code
 * @param source code
 * @return true on success
 */
bool P2Asm::assemble_pass()
{
    pass_clear();

    for (int i = 0; i < m_source.count(); i++) {
        line_clear();
        tokenize();

        // Ignore empty lines
        if (m_idx < m_cnt) {
            bool success = false;
            switch (m_section) {
            case dat_section:
                success = assemble_dat_section();
                break;
            case con_section:
                success = assemble_con_section();
                break;
            case pub_section:
            case pri_section:
            case var_section:
                success = assemble_dat_section();
                break;
            }
            Q_UNUSED(success)
        }
        // Store the results, emit listing, etc.
        results();

    }

    return true;
}

bool P2Asm::assemble(const QStringList& list)
{
    clear();
    m_source = list;
    bool success = true;

    for (int pass = 0; pass < 2; pass++)
        success &= assemble_pass();
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
        emit Error(0, 0, tr("Can not open '%1' for reading.").arg(filename));
        return false;
    }

    QTextStream stream(&file);
    QStringList list;
    while (!stream.atEnd())
        list += stream.readLine();

    return assemble(list);
}

bool P2Asm::set_pathname(const QString& pathname)
{
    if (pathname == m_pathname)
        return false;
    QDir dir(pathname);
    if (!dir.exists())
        return false;
    m_pathname = dir.canonicalPath();
    return true;
}

bool P2Asm::load(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QTextStream stream(&file);
    QStringList source;
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        source += line;
    }
    return set_source(source);
}

/**
 * @brief Return number of commata following in the words/tokens
 * @return number of commata following
 */
int P2Asm::commata_left() const
{
    int commata = 0;
    for (int i = m_idx; i < m_cnt; i++)
        if (t_COMMA == m_words[i].tok())
            commata++;
    return commata;
}

/**
 * @brief Return true, if %tok is found in the following words
 * @return true if %tok is found, or false otherwise
 */
bool P2Asm::find_tok(p2_token_e tok) const
{
    for (int i = m_idx; i < m_cnt; i++)
        if (tok == m_words[i].tok())
            return true;
    return false;
}

/**
 * @brief Instruction register was constructed
 * @param wr_mem if true, store opcode into MEM
 * @return formatted string
 */
QStringList P2Asm::results_instruction(bool wr_mem)
{
    QStringList output;
    p2_LONG org = m_ORG;
    p2_LONG orgh = m_ORGH;

    QString* lineptr = m_lineptr;
    Q_ASSERT(m_advance == 4);

    m_IR.set_org_orgh(p2_ORG_ORGH_t(m_ORG, m_ORGH));
    m_hash_PC.insert(m_lineno, p2_ORG_ORGH_t(m_ORG, m_ORGH));
    m_hash_IR.insert(m_lineno, m_IR);

    // Do we need to generate an AUGD instruction?
    if (m_IR.augd_valid()) {
        p2_LONG value = m_IR.augd_value();
        P2Opcode IR(p2_AUGD, p2_ORG_ORGH_t(org, orgh));
        IR.set_imm23(value);

        if (m_pass > 1) {
            output += QString("%1 %2 [%3] %4")
                     .arg(m_lineno, -6)
                     .arg(org, 6, 16, QChar('0'))
                     .arg(IR.opcode(), 8, 16, QChar('0'))
                     .arg(lineptr ? *lineptr : QString());
        }
        if (wr_mem && orgh < MEM_SIZE) {
            // write the opcode to memory as well
            MEM.LONGS[orgh/4] = IR.opcode();
        }

        org += 4;
        orgh += 4;
        m_IR.set_org_orgh(IR.org_orgh());
        m_advance += 4;
        m_IR.set_AUGD();
        lineptr = nullptr;
    }

    // Do we need to generate an AUGS instruction?
    if (m_IR.augs_valid()) {
        p2_LONG value = m_IR.augs_value();
        P2Opcode IR(p2_AUGS, p2_ORG_ORGH_t(org, orgh));
        IR.set_imm23(value);

        if (m_pass > 1) {
            output += QString("%1 %2 [%3] %4")
                     .arg(m_lineno, -6)
                     .arg(org, 6, 16, QChar('0'))
                     .arg(IR.opcode(), 8, 16, QChar('0'))
                     .arg(lineptr ? *lineptr : QString());
        }
        if (wr_mem && orgh < MEM_SIZE) {
            // write the opcode to memory as well
            MEM.LONGS[orgh/4] = IR.opcode();
        }

        org += 4;
        orgh += 4;
        m_IR.set_org_orgh(IR.org_orgh());
        m_advance += 4;
        m_IR.set_augs();
        lineptr = nullptr;
    }

    if (m_pass > 1) {
        output += QString("%1 %2 [%3] %4")
                  .arg(m_lineno, -6)
                  .arg(org, 6, 16, QChar('0'))
                  .arg(m_IR.opcode(), 8, 16, QChar('0'))
                  .arg(lineptr ? *lineptr : QString());
    }

    if (wr_mem && orgh < MEM_SIZE) {
        // write the opcode to memory as well
        MEM.LONGS[orgh/4] = m_IR.opcode();
    }
    return output;
}

/**
 * @brief Assignment to symbol was made
 * @return formatted string
 */
QString P2Asm::results_assignment()
{
    QString output;
    m_IR.set_as_IR(false);
    m_hash_IR.insert(m_lineno, m_IR);
    if (m_pass > 1) {
        output = QString("%1 %2 <%3> %4")
                 .arg(m_lineno, -6)
                 .arg(m_ORG, 6, 16, QChar('0'))
                 .arg(m_IR.opcode(), 8, 16, QChar('0'))
                 .arg(*m_lineptr);
    }
    return output;
}

/**
 * @brief Data was constructed
 * @param wr_mem if true, store opcode into MEM
 * @return formatted string for the last data byte/word/long
 */
QStringList P2Asm::results_data(bool wr_mem)
{
    QStringList output;
    const p2_LONG start = m_ORG;
    p2_LONG offset = m_ORG;
    p2_ORG_ORGH_t org_orgh(m_ORG, m_ORGH);

    m_IR.set_data(m_data);
    m_IR.set_org_orgh(org_orgh);
    m_hash_PC.insert(m_lineno, org_orgh);
    m_hash_IR.insert(m_lineno, m_IR);

    if (wr_mem) {
        const p2_BYTES& _bytes = m_data.get_bytes();
        p2_LONG addr = m_ORGH;
        foreach(p2_BYTE b, _bytes) {
            if (addr >= MEM_SIZE)
                break;
            MEM.BYTES[addr] = b;
            addr++;
        }
    }

    if (m_pass > 1) {
        output += P2Opcode::format_data(m_IR, fmt_hex).split(QChar::LineFeed);
    }

    m_advance = offset - start;

    return output;
}

/**
 * @brief Comment or non code generating instruction
 * @return formatted string
 */
QString P2Asm::results_comment()
{
    QString output;
    if (m_pass > 1) {
        output = QString("%1 %2 -%3- %4")
                 .arg(m_lineno, -6)
                 .arg(m_ORG, 6, 16, QChar('0'))
                 .arg(QStringLiteral("--------"))
                 .arg(*m_lineptr);
    }
    return output;
}

/**
 * @brief Store the results and append a line to the listing
 * @param opcode true, if the IR field contains an opcode
 */
void P2Asm::results()
{
    const bool binary = true;

    if (!m_data.isEmpty()) {
        m_IR.set_data(m_data);
    }

    if (m_IR.is_instruction()) {
        m_listing += results_instruction(binary);
    } else if (m_IR.is_assign()) {
        m_listing += results_assignment();
    } else if (m_IR.is_data()) {
        m_listing += results_data(binary);
    } else {
        m_listing += results_comment();
    }

    // Calculate next ORG and PC values by adding m_advance
    m_ORGH += m_advance;
    m_ORG += m_advance;

    if (!m_words.isEmpty())
        m_hash_words.insert(m_lineno, m_words);
    if (!m_errors.isEmpty())
        m_hash_error.insert(m_lineno, m_errors);
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
 * @brief Skip over t_comments tokens
 * @return true if skipped (or none), false if at end of line
 */
bool P2Asm::skip_comments()
{
    if (m_idx >= m_cnt)
        return false;
    while (Token.is_type(curr_tok(), tm_comment))
        m_idx++;
    return m_idx < m_cnt;
}

/**
 * @brief Skip over t_comments tokens and return true if end-of-line
 * @return true if at end of line, or false otherwise
 */
bool P2Asm::eol()
{
    if (!skip_comments())
        return true;
    return false;
}

/**
 * @brief Make an initial P2Atom based on the instruction token
 * @return P2Atom with type set up
 */
P2Atom P2Asm::make_atom()
{
    P2Atom atom;
    switch (m_instr) {
    case t_BYTE:
        atom.set_type(ut_Byte);
        break;
    case t_WORD:
        atom.set_type(ut_Word);
        break;
    case t_LONG:
        atom.set_type(ut_Long);
        break;
    case t_FILE:
        atom.set_type(ut_String);
        break;
    default:
        atom.set_type(ut_Long);
    }
    return atom;
}

/**
 * @brief Convert a string of binary digits into an unsigned value
 * @param str binary digits
 * @return P2Atom value of binary digits in str
 */
bool P2Asm::bin_const(P2Atom& atom, const QString& str)
{
    QString num = str;
    if (num.startsWith(chr_percent))
        num.remove(0, 1);
    num.remove(chr_skip_digit);
    bool ok;
    p2_QUAD quad = num.toULongLong(&ok, 2);
    if (ok)
        atom.set_value(P2Union(quad));
    return ok;
}

/**
 * @brief Convert a string of byte indices into an unsigned value
 * @param str byte index digits
 * @return P2Atom value of byte indices in str
 */
bool P2Asm::byt_const(P2Atom &atom, const QString& str)
{
    QString num = str;
    if (num.startsWith(str_byt_prefix))
        num.remove(0, 2);
    num.remove(chr_skip_digit);
    bool ok;
    p2_QUAD quad = num.toULongLong(&ok, 4);
    if (ok)
        atom.set_value(P2Union(quad));
    return ok;
}

/**
 * @brief Convert a string of decimal digits into an unsigned value
 * @param str decimal digits
 * @return P2Atom value of decimal digits in str
 */
bool P2Asm::dec_const(P2Atom& atom, const QString& str)
{
    QString num = str;
    num.remove(chr_skip_digit);
    bool ok;
    p2_QUAD quad = num.toULongLong(&ok, 10);
    if (ok)
        atom.set_value(P2Union(quad));
    return ok;
}

/**
 * @brief Convert a string of hexadecimal digits into an unsigned value
 * @param str hexadecimal digits
 * @return P2Atom value of hexadecimal digits in str
 */
bool P2Asm::hex_const(P2Atom& atom, const QString& str)
{
    QString num = str;
    if (num.startsWith(chr_dollar))
        num.remove(0, 1);
    num.remove(chr_skip_digit);
    bool ok;
    p2_QUAD quad = num.toULongLong(&ok, 16);
    if (ok)
        atom.set_value(P2Union(quad));
    return ok;
}

/**
 * @brief Convert a string of decimal digits into an unsigned value
 * @param str decimal digits
 * @return P2Atom value of decimal digits in str
 */
bool P2Asm::real_const(P2Atom& atom, const QString& str)
{
    QString num = str;
    num.remove(chr_skip_digit);
    bool ok;
    p2_REAL real = num.toDouble(&ok);
    if (ok)
        atom.set_value(P2Union(real));
    return ok;
}

/**
 * @brief Convert a string of ASCII characters into an unsigned value
 * @param str ASCII characters
 * @param stop characters to stop at
 * @return P2Atom value of in string
 */
bool P2Asm::str_const(P2Atom& atom, const QString& str)
{
    int pos = 0;
    QChar ch = str.at(pos++);
    QChar in_string = ch;
    bool escaped = false;
    p2_CHARS chars;

    for (/**/; pos < str.length(); pos++) {
        ch = str.at(pos);
        if (escaped) {
            chars += ch.toLatin1();
            escaped = false;
        } else if (ch == in_string) {
            pos++;
            break;
        } else if (ch == str_escape) {
            escaped = true;
        } else {
            chars += ch.toLatin1();
        }
    }
    atom.set_chars(chars);

    return true;
}

/**
 * @brief Find a symbol name using either %func, or %func and %sym appended
 * @param sect primary section where to search
 * @param func name of the function
 * @param local name of a local symbol
 * @return QString with the name of the symbol
 */
QString P2Asm::find_symbol(Section sect, const QString& func, bool all_sections)
{
    QString section = m_sections.value(sect);
    QString symbol;

    if (all_sections) {
        QStringList sections = m_sections.values();
        if (!section.isEmpty()) {
            sections.removeAll(section);
            sections.insert(0, section);
        }
        foreach (const QString& section, sections) {
            symbol = QString("%1::%2")
                     .arg(section.toUpper())
                     .arg(func.toUpper());
            if (m_symbols->contains(symbol))
                return symbol;
        }
    }

    // use original section
    symbol = QString("%1::%2")
             .arg(section.toUpper())
             .arg(func.toUpper());
    return symbol;
}

/**
 * @brief Find a symbol name using either %func, or %func and %sym appended
 * @param sect primary section where to search
 * @param func name of the function
 * @param local name of a local symbol
 * @return QString with the name of the symbol
 */
QString P2Asm::find_locsym(Section sect, const QString& local)
{
    QString section = m_sections.value(sect);
    QString func = m_function.value(sect);
    QString symbol = QString("%1::%2%3")
             .arg(section.toUpper())
             .arg(func.toUpper())
             .arg(local.toUpper());

    if (m_symbols->contains(symbol))
        return symbol;
    return symbol;
}

/**
 * @brief Add a constant "symbol" to the table and reference it
 * @param pfx string for the prefix
 * @param str string for the constant
 * @param atom atom value of the constant
 */
void P2Asm::add_const_symbol(const QString& pfx, const P2Word& word, const P2Atom& atom)
{
    QString symbol = QString("%1%2")
                     .arg(pfx)
                     .arg(word.str().toUpper());
    if (!m_symbols->contains(symbol))
        m_symbols->insert(P2Symbol(symbol, atom.value()));
    m_symbols->add_reference(m_lineno, symbol, word);
}

/**
 * @brief Parse an atomic part of an expression
 * @return value of the atom
 */
bool P2Asm::parse_atom(P2Atom& atom, int level)
{
    QString symbol;

    if (eol())
        return false;

    const P2Word& word = curr_word();
    QString str = word.str();
    p2_token_e tok = word.tok();

    switch (tok) {
    case t__FLOAT:
        next();
        DBG_EXPR(" atom float function: %s", qPrintable(str));
        atom = parse_expression(level+1);
        atom.set_type(ut_Real);
        prev();
        break;

    case t__ROUND:
        DBG_EXPR(" atom round function: %s", qPrintable(str));
        next();
        atom = parse_expression(level+1);
        atom.set_type(ut_Long);
        prev();
        DBG_EXPR(" atom round atom = %s", qPrintable(atom.str()));
        break;

    case t__TRUNC:
        DBG_EXPR(" atom trunc function: %s", qPrintable(str));
        next();
        atom = parse_expression(level+1);
        atom.set_type(ut_Long);
        prev();
        DBG_EXPR(" atom trunc atom = %s", qPrintable(atom.str()));
        break;

    case t__LPAREN:
        // precedence 0
        DBG_EXPR(" atom lparen: %s", qPrintable(curr_str()));
        next();
        atom = parse_expression(level+1);
        prev();
        break;

    case t__RPAREN:
        DBG_EXPR(" atom rparen: %s", qPrintable(curr_str()));
        next();
        break;

    case t__LBRACKET:
        // precedence 0
        DBG_EXPR(" atom lbracket: %s", qPrintable(curr_str()));
        next();
        parse_expression(level+1);  // Note: index is set in parse_expression()
        prev();
        break;

    case t__RBRACKET:
        DBG_EXPR(" atom rbracket: %s", qPrintable(curr_str()));
        next();
        break;

    case t_locsym:
        symbol = find_locsym(m_section, str);
        if (m_symbols->contains(symbol)) {
            P2Symbol sym = m_symbols->symbol(symbol);
            DBG_EXPR(" atom found locsym: %s = %s", qPrintable(symbol), qPrintable(sym.()));
            m_symbols->add_reference(m_lineno, symbol, word);
            break;
        }
        DBG_EXPR(" atom undefined locsym: %s", qPrintable(symbol));
        break;

    case t_symbol:
        symbol = find_symbol(m_section, str, true);
        if (m_symbols->contains(symbol)) {
            P2Symbol sym = m_symbols->symbol(symbol);
            atom.set_value(sym.value());
            DBG_EXPR(" atom found symbol: %s = %s", qPrintable(symbol), qPrintable(atom.str()));
            m_symbols->add_reference(m_lineno, symbol, word);
            break;
        }
        DBG_EXPR(" atom undefined symbol: %s", qPrintable(symbol));
        break;

    case t_bin_const:
        DBG_EXPR(" atom bin const: %s", qPrintable(str));
        bin_const(atom, str);
        add_const_symbol(p2_prefix_bin_const, word, atom);
        DBG_EXPR(" atom bin atom = %s", qPrintable(atom.str()));
        break;

    case t_byt_const:
        DBG_EXPR(" atom byt const: %s", qPrintable(str));
        byt_const(atom, str);
        add_const_symbol(p2_prefix_byt_const, word, atom);
        DBG_EXPR(" atom byt atom = %s", qPrintable(atom.str()));
        break;

    case t_dec_const:
        DBG_EXPR(" atom dec const: %s", qPrintable(str));
        dec_const(atom, str);
        add_const_symbol(p2_prefix_dec_const, word, atom);
        DBG_EXPR(" atom dec atom = %s", qPrintable(atom.str()));
        break;

    case t_hex_const:
        DBG_EXPR(" atom hex const: %s", qPrintable(str));
        hex_const(atom, str);
        add_const_symbol(p2_prefix_hex_const, word, atom);
        DBG_EXPR(" atom hex atom = %s", qPrintable(atom.str()));
        break;

    case t_str_const:
        DBG_EXPR(" atom str const: %s", qPrintable(str));
        str_const(atom, str);
        add_const_symbol(p2_prefix_str_const, word, atom);
        DBG_EXPR(" atom str atom = %s", qPrintable(atom.str()));
        break;

    case t_real_const:
        DBG_EXPR(" atom real const: %s", qPrintable(str));
        real_const(atom, str);
        atom.set_type(ut_Real);
        add_const_symbol(p2_prefix_real_const, word, atom);
        DBG_EXPR(" atom real atom = %s", qPrintable(atom.str()));
        break;

    case t_DIRA:
        DBG_EXPR(" atom DIRA: %s", qPrintable(str));
        atom.set_type(ut_Addr);
        atom.set_value(P2Union(offs_DIRA));
        add_const_symbol(p2_prefix_offs_const, word, atom);
        DBG_EXPR(" atom DIRA atom = %s", qPrintable(atom.str()));
        break;

    case t_DIRB:
        DBG_EXPR(" atom DIRB: %s", qPrintable(str));
        atom.set_type(ut_Addr);
        atom.set_value(P2Union(offs_DIRB));
        add_const_symbol(p2_prefix_offs_const, word, atom);
        DBG_EXPR(" atom DIRB atom = %s", qPrintable(atom.str()));
        break;

    case t_INA:
        DBG_EXPR(" atom INA: %s", qPrintable(str));
        atom.set_type(ut_Addr);
        atom.set_value(P2Union(offs_INA));
        add_const_symbol(p2_prefix_offs_const, word, atom);
        DBG_EXPR(" atom INA atom = %s", qPrintable(atom.str()));
        break;

    case t_INB:
        DBG_EXPR(" atom INB: %s", qPrintable(str));
        atom.set_type(ut_Addr);
        atom.set_value(P2Union(offs_INB));
        add_const_symbol(p2_prefix_offs_const, word, atom);
        DBG_EXPR(" atom INB atom = %s", qPrintable(atom.str()));
        break;

    case t_OUTA:
        DBG_EXPR(" atom OUTA: %s", qPrintable(str));
        atom.set_type(ut_Addr);
        atom.set_value(P2Union(offs_OUTA));
        add_const_symbol(p2_prefix_offs_const, word, atom);
        DBG_EXPR(" atom OUTA atom = %s", qPrintable(atom.str()));
        break;

    case t_OUTB:
        DBG_EXPR(" atom OUTB: %s", qPrintable(str));
        atom.set_type(ut_Addr);
        atom.set_value(P2Union(offs_OUTB));
        add_const_symbol(p2_prefix_offs_const, word, atom);
        DBG_EXPR(" atom OUTB atom = %s", qPrintable(atom.str()));
        break;

    case t_PA:
        DBG_EXPR(" atom PA: %s", qPrintable(str));
        atom.set_type(ut_Addr);
        atom.set_value(P2Union(offs_PA));
        add_const_symbol(p2_prefix_offs_const, word, atom);
        DBG_EXPR(" atom PA atom = %s", qPrintable(atom.str()));
        break;

    case t_PB:
        DBG_EXPR(" atom PB: %s", qPrintable(str));
        atom.set_type(ut_Addr);
        atom.set_value(P2Union(offs_PB));
        add_const_symbol(p2_prefix_offs_const, word, atom);
        DBG_EXPR(" atom PB atom = %s", qPrintable(atom.str()));
        break;

    case t_PTRA:
    case t_PTRA_postinc:
    case t_PTRA_postdec:
    case t_PTRA_preinc:
    case t_PTRA_predec:
        DBG_EXPR(" atom PTRA: %s", qPrintable(str));
        atom.set_type(ut_Addr);
        atom.set_value(P2Union(offs_PTRA));
        add_const_symbol(p2_prefix_offs_const, word, atom);
        if (t_PTRA_preinc == tok || t_PTRA_predec == tok)
            atom.add_trait(tr_PRE);
        if (t_PTRA_postinc == tok || t_PTRA_postdec == tok)
            atom.add_trait(tr_POST);
        if (t_PTRA_preinc == tok || t_PTRA_postinc == tok)
            atom.add_trait(tr_INC);
        if (t_PTRA_predec == tok || t_PTRA_postdec == tok)
            atom.add_trait(tr_DEC);
        DBG_EXPR(" atom PTRA atom = %s", qPrintable(atom.str()));
        break;

    case t_PTRB:
    case t_PTRB_postinc:
    case t_PTRB_postdec:
    case t_PTRB_preinc:
    case t_PTRB_predec:
        DBG_EXPR(" atom PTRB: %s", qPrintable(str));
        atom.set_type(ut_Addr);
        atom.set_value(P2Union(offs_PTRB));
        add_const_symbol(p2_prefix_offs_const, word, atom);
        if (t_PTRB_preinc == tok || t_PTRB_predec == tok)
            atom.add_trait(tr_PRE);
        if (t_PTRB_postinc == tok || t_PTRB_postdec == tok)
            atom.add_trait(tr_POST);
        if (t_PTRB_preinc == tok || t_PTRB_postinc == tok)
            atom.add_trait(tr_INC);
        if (t_PTRB_predec == tok || t_PTRB_postdec == tok)
            atom.add_trait(tr_DEC);
        DBG_EXPR(" atom PTRB atom = %s", qPrintable(atom.str()));
        break;

    case t_DOLLAR:
        DBG_EXPR(" atom current PC: %s", qPrintable(str));
        atom.set_type(ut_Addr);
        atom.set_value(P2Union(m_ORG / 4));    // FIXME: use m_ORG or m_ORG/4 ?
        DBG_EXPR(" atom $ atom = %s", qPrintable(atom.str()));
        break;

    default:
        DBG_EXPR(" not atomic: %s (%s)", qPrintable(str), qPrintable(Token.type_names(word.tok()).join(QStringLiteral(", "))));
        prev();
        break;
    }

    next();

    return true;
}

/**
 * @brief Parse an expression of primary operations
 * @param str string to parse
 * @return P2Atom containing the result
 */
bool P2Asm::parse_primary(P2Atom& atom, int level)
{
    if (eol())
        return false;

    p2_LONG inc_by = 0;
    p2_LONG dec_by = 0;

    p2_token_e op = curr_tok();
    while (Token.is_type(op, tm_primary)) {

        switch (op) {
        case t__INC:    // ++
            DBG_EXPR(" primary inc: %s", qPrintable(Token.string(op)));
            inc_by++;
            break;

        case t__DEC:    // --
            DBG_EXPR(" primary dec: %s", qPrintable(Token.string(op)));
            dec_by--;
            break;

        default:
            Q_ASSERT_X(false, "primary", "Invalid op");
            return false;
        }

        if (eol())
            break;

        op = curr_tok();
    }

    parse_atom(atom, level);

    // apply primary ops
    atom.unary_dec(dec_by);
    atom.unary_inc(inc_by);
    DBG_EXPR(" primary atom = %s", qPrintable(atom.str()));

    return true;
}

/**
 * @brief Parse an expression of unary operations
 * @param str string to parse
 * @return P2Atom containing the result
 */
bool P2Asm::parse_unary(P2Atom& atom, int level)
{
    if (eol())
        return false;

    bool do_logical_not = false;
    bool do_complement2 = false;
    bool do_complement1 = false;

    p2_token_e op = curr_tok();
    while (Token.is_type(op, tm_unary)) {

        switch (op) {
        case t__NEG:
            DBG_EXPR(" unary neg: %s", qPrintable(Token.string(op)));
            do_logical_not = !do_logical_not;
            next();
            break;

        case t__NOT:
            DBG_EXPR(" unary not: %s", qPrintable(Token.string(op)));
            do_complement1 = !do_complement1;
            next();
            break;

        case t__MINUS:
            DBG_EXPR(" unary minus: %s", qPrintable(Token.string(op)));
            do_complement2 = !do_complement2;
            next();
            break;

        case t__PLUS:
            DBG_EXPR(" unary plus: %s", qPrintable(Token.string(op)));
            next();
            break;

        default:
            Q_ASSERT_X(false, "shiftops", "Invalid op");
            parse_primary(atom, level);
        }

        if (eol())
            break;

        op = curr_tok();
    }

    parse_primary(atom, level);

    // apply unary ops
    atom.complement1(do_complement1);
    atom.complement2(do_complement2);
    atom.logical_not(do_logical_not);
    DBG_EXPR(" unary atom = %s", qPrintable(atom.str()));

    return true;
}

/**
 * @brief Parse an expression of multiply operations
 * @param str string to parse
 * @return P2Atom containing the result
 */
bool P2Asm::parse_mulops(P2Atom& atom, int level)
{
    parse_unary(atom, level);

    if (eol()) {
        DBG_EXPR(" mulops atom = %s", qPrintable(atom.str()));
        return true;
    }

    p2_token_e op = curr_tok();
    while (Token.is_type(op, tm_mulop)) {

        if (!next()) {
            DBG_EXPR(" missing rvalue (%s)", "mulops");
            m_errors += tr("End of line in expression (%1)")
                      .arg(tr("mulops"));
            emit Error(m_pass, m_lineno, m_errors.last());
            break;
        }

        P2Atom rvalue = parse_expression(level);
        if (!rvalue.isValid()) {
            DBG_EXPR(" invalid rvalue: %s", qPrintable(curr_str()));
            m_errors += tr("Invalid character in expression (%1): %2")
                        .arg(tr("mulops"))
                        .arg(curr_str());
            emit Error(m_pass, m_lineno, m_errors.last());
            break;
        }

#if 0
        // propagate atom to Real if rvalue is Real
        if (ut_Real == rvalue.type())
            atom.make_real();
#endif

        switch (op) {
        case t__MUL:
            DBG_EXPR(" mulop MUL: %s %s %s", qPrintable(atom.str()), qPrintable(Token.string(op)), qPrintable(rvalue.str()));
            atom.arith_mul(rvalue);
            DBG_EXPR(" mulop atom = %s", qPrintable(atom.str()));
            break;
        case t__DIV:
            DBG_EXPR(" mulop DIV: %s %s %s", qPrintable(atom.str()), qPrintable(Token.string(op)), qPrintable(rvalue.str()));
            atom.arith_div(rvalue);
            DBG_EXPR(" mulop atom = %s", qPrintable(atom.str()));
            break;
        case t__MOD:
            DBG_EXPR(" mulop MOD: %s %s %s", qPrintable(atom.str()), qPrintable(Token.string(op)), qPrintable(rvalue.str()));
            atom.arith_mod(rvalue);
            DBG_EXPR(" mulop atom = %s", qPrintable(atom.str()));
            break;
        default:
            Q_ASSERT_X(false, "mulops", "invalid op");
            return false;
        }

        if (eol())
            break;

        op = curr_tok();
    }
    DBG_EXPR(" mulops atom = %s", qPrintable(atom.str()));

    return true;
}

/**
 * @brief Parse an expression of shift operations
 * @param str string to parse
 * @return P2Atom containing the result
 */
bool P2Asm::parse_shiftops(P2Atom& atom, int level)
{
    parse_mulops(atom, level);

    if (eol()) {
        DBG_EXPR(" shiftops atom = %s", qPrintable(atom.str()));
        return true;
    }

    p2_token_e op = curr_tok();
    while (Token.is_type(op, tm_shiftop)) {

        if (!next()) {
            DBG_EXPR(" missing rvalue (%s)", "shiftops");
            m_errors += tr("End of line in expression (%1)")
                      .arg(tr("shiftops"));
            emit Error(m_pass, m_lineno, m_errors.last());
            break;
        }

        P2Atom rvalue = parse_expression(level);
        if (!rvalue.isValid()) {
            DBG_EXPR(" invalid rvalue: %s", qPrintable(curr_str()));
            m_errors += tr("Invalid character in expression (%1): %2")
                        .arg(tr("shiftops"))
                        .arg(curr_str());
            emit Error(m_pass, m_lineno, m_errors.last());
            break;
        }

#if 0
        // propagate atom to Real if rvalue is Real
        if (ut_Real == rvalue.type())
            atom.make_real();
#endif

        switch (op) {
        case t__SHL:
            DBG_EXPR(" shiftop SHL: %s %s %s", qPrintable(atom.str()), qPrintable(Token.string(op)), qPrintable(rvalue.str()));
            atom.binary_shl(rvalue);
            DBG_EXPR(" shiftop atom = %s", qPrintable(atom.str()));
            break;
        case t__SHR:
            DBG_EXPR(" shiftop SHR: %s %s %s", qPrintable(atom.str()), qPrintable(Token.string(op)), qPrintable(rvalue.str()));
            atom.binary_shr(rvalue);
            DBG_EXPR(" shiftop atom = %s", qPrintable(atom.str()));
            break;
        default:
            Q_ASSERT_X(false, "shiftops", "Invalid op");
            return false;
        }

        if (eol())
            break;

        op = curr_tok();
    }
    DBG_EXPR(" shiftops atom = %s", qPrintable(atom.str()));

    return true;
}

/**
 * @brief Parse an expression of add/subtract operations
 * @param str string to parse
 * @return P2Atom containing the result
 */
bool P2Asm::parse_addops(P2Atom& atom, int level)
{
    parse_shiftops(atom, level);

    if (eol()) {
        DBG_EXPR(" addops atom = %s", qPrintable(atom.str()));
        return true;
    }

    p2_token_e op = curr_tok();
    while (Token.is_type(op, tm_addop)) {

        if (!next()) {
            DBG_EXPR(" missing rvalue (%s)", "addops");
            m_errors += tr("End of line in expression (%1)")
                      .arg(tr("addops"));
            emit Error(m_pass, m_lineno, m_errors.last());
            break;
        }

        P2Atom rvalue = parse_expression(level);
        if (!rvalue.isValid()) {
            DBG_EXPR(" invalid rvalue %s", qPrintable(curr_str()));
            m_errors += tr("Invalid character in expression (%1): %2")
                        .arg(tr("addops"))
                        .arg(curr_str());
            emit Error(m_pass, m_lineno, m_errors.last());
            break;
        }

#if 0
        // propagate atom to Real if rvalue is Real
        if (ut_Real == rvalue.type())
            atom.make_real();
#endif

        switch (op) {
        case t__PLUS:
            DBG_EXPR(" addop ADD: %s %s %s", qPrintable(atom.str()), qPrintable(Token.string(op)), qPrintable(rvalue.str()));
            atom.arith_add(rvalue);
            DBG_EXPR(" addop atom = %s", qPrintable(atom.str()));
            break;
        case t__MINUS:
            DBG_EXPR(" addop SUB: %s %s %s", qPrintable(atom.str()), qPrintable(Token.string(op)), qPrintable(rvalue.str()));
            atom.arith_sub(rvalue);
            DBG_EXPR(" addop atom = %s", qPrintable(atom.str()));
            break;
        default:
            Q_ASSERT_X(false, "addops", "Invalid op");
            return false;
        }

        if (eol())
            break;

        op = curr_tok();
    }
    DBG_EXPR(" addops atom = %s", qPrintable(atom.str()));

    return true;
}

/**
 * @brief Parse an expression of binary operations
 * @param str string to parse
 * @return P2Atom containing the result
 */
bool P2Asm::parse_binops(P2Atom& atom, int level)
{
    parse_addops(atom, level);

    if (eol()) {
        DBG_EXPR(" binops atom = %s", qPrintable(atom.str()));
        return true;
    }

    p2_token_e op = curr_tok();
    while (Token.is_type(op, tm_binop)) {

        if (!next()) {
            DBG_EXPR(" missing rvalue (%s)", "binops");
            m_errors += tr("End of line in expression (%1)")
                      .arg(tr("binops"));
            emit Error(m_pass, m_lineno, m_errors.last());
            break;
        }

        P2Atom rvalue = parse_expression(level);
        if (!rvalue.isValid()) {
            DBG_EXPR(" invalid rvalue: %s", qPrintable(curr_str()));
            m_errors += tr("Invalid character in expression (%1): %2")
                        .arg(tr("binops"))
                        .arg(curr_str());
            emit Error(m_pass, m_lineno, m_errors.last());
            break;
        }

#if 0
        // propagate lvalue to Real if rvalue is Real
        if (ut_Real == atom.type())
            atom.make_real();
#endif

        switch (op) {
        case t__AND:
            DBG_EXPR(" binop AND: %s %s %s", qPrintable(atom.str()), qPrintable(Token.string(op)), qPrintable(rvalue.str()));
            atom.binary_and(rvalue);
            DBG_EXPR(" binop atom = %s", qPrintable(atom.str()));
            break;
        case t__XOR:
            DBG_EXPR(" binop XOR: %s %s %s", qPrintable(atom.str()), qPrintable(Token.string(op)), qPrintable(rvalue.str()));
            atom.binary_xor(rvalue);
            DBG_EXPR(" binop atom = %s", qPrintable(atom.str()));
            break;
        case t__OR:
            DBG_EXPR(" binop OR: %s %s %s", qPrintable(atom.str()), qPrintable(Token.string(op)), qPrintable(rvalue.str()));
            atom.binary_or(rvalue);
            DBG_EXPR(" binop atom = %s", qPrintable(atom.str()));
            break;
        case t__REV:
            DBG_EXPR(" binop REV: %s %s %s", qPrintable(atom.str()), qPrintable(Token.string(op)), qPrintable(rvalue.str()));
            atom.reverse(rvalue);
            DBG_EXPR(" binop atom = %s", qPrintable(atom.str()));
            break;
        case t__ENCOD:
            DBG_EXPR(" binop ENCOD: %s %s %s", qPrintable(atom.str()), qPrintable(Token.string(op)), qPrintable(rvalue.str()));
            atom.encode(rvalue);
            DBG_EXPR(" binop atom = %s", qPrintable(atom.str()));
            break;
        case t__DECOD:
            DBG_EXPR(" binop DECOD: %s %s %s", qPrintable(atom.str()), qPrintable(Token.string(op)), qPrintable(rvalue.str()));
            atom.decode(rvalue);
            DBG_EXPR(" binop atom = %s", qPrintable(atom.str()));
            break;
        default:
            Q_ASSERT_X(false, "binops", "Invalid op");
            return false;
        }

        if (eol())
            break;

        op = curr_tok();
    }
    DBG_EXPR(" binops atom = %s", qPrintable(atom.str()));

    return true;
}

p2_traits_e P2Asm::parse_traits()
{
    p2_token_e tok = curr_tok();
    p2_traits_e trait = tr_none;
    while (Token.is_type(tok, tm_traits)) {

        switch (tok) {
        case t_IMMEDIATE:
            DBG_EXPR(" trait immediate: %s", qPrintable(Token.string(tok)));
            p2_set_trait(trait, tr_IMMEDIATE);
            next();
            break;
        case t_AUGMENTED:
            DBG_EXPR(" trait force AUGS/AUGD: %s", qPrintable(Token.string(tok)));
            p2_set_trait(trait, tr_AUGMENTED);
            next();
            break;
        case t_RELATIVE:
            DBG_EXPR(" trait relative: %s", qPrintable(Token.string(tok)));
            p2_set_trait(trait, tr_RELATIVE);
            next();
            break;
        case t_ABSOLUTE:
            DBG_EXPR(" trait absolute: %s", qPrintable(Token.string(tok)));
            p2_set_trait(trait, tr_ABSOLUTE);
            next();
            break;
        case t_ADDRESS_HUB:
            DBG_EXPR(" trait address HUB: %s", qPrintable(Token.string(tok)));
            p2_set_trait(trait, tr_ADDRESS_HUB);
            next();
            break;
        case t_RELATIVE_HUB:
            DBG_EXPR(" trait relative HUB: %s", qPrintable(Token.string(tok)));
            p2_set_trait(trait, tr_RELATIVE_HUB);
            next();
            break;
        default:
            break;
        }

        if (eol())
            return trait;

        tok = curr_tok();
    }
    return trait;
}

/**
 * @brief Evaluate an expression
 * @param level expression nesting level
 * @return QVariant with the value of the expression
 */
P2Atom P2Asm::parse_expression(int level)
{
    P2Atom atom = make_atom();
    DBG_EXPR("»»»» %d", level);

    if (eol()) {
        DBG_EXPR(" expr atom = %s", qPrintable(atom.str()));
        return atom;
    }

    p2_traits_e traits = parse_traits();

    // Check for subexpression enclosed in parenthesis
    p2_token_e tok = curr_tok();
    while (Token.is_type(tok, tm_parens)) {

        switch (tok) {
        case t__LPAREN:
            // precedence 0
            DBG_EXPR(" expr lparen: %s", qPrintable(curr_str()));
            next();
            atom = parse_expression(level+1);
            prev();
            break;

        case t__RPAREN:
            DBG_EXPR(" expr rparen: %s", qPrintable(curr_str()));
            next();
            break;

        default:
            DBG_EXPR(" not parens: %s (%s)", qPrintable(curr_str()), qPrintable(Token.string(tok)));
            prev();
        }

        if (eol()) {
            DBG_EXPR(" expr atom = %s", qPrintable(atom.str()));
            return atom;
        }
        tok = curr_tok();
    }

    parse_binops(atom, level);  // evaluate the expression into atom
    DBG_EXPR(" expr atom = %s", qPrintable(atom.str()));

    // Check for index expression enclosed in square brackets
    P2Atom index = P2Atom(ut_Long);
    tok = curr_tok();
    while (Token.is_type(tok, tm_brackets)) {
        switch (tok) {
        case t__LBRACKET:
            // precedence 0
            DBG_EXPR(" expr lbracket: %s", qPrintable(curr_str()));
            next();
            index = parse_expression(level+1);  // evaluate the expression into index
            p2_set_trait(traits, tr_INDEX);
            if (index.has_trait(tr_AUGMENTED))  // pass on tr_AUGMENTED to the atom traits
                p2_set_trait(traits, tr_AUGMENTED);
            break;

        case t__RBRACKET:
            DBG_EXPR(" expr rbracket: %s", qPrintable(curr_str()));
            next();
            break;

        default:
            DBG_EXPR(" expr not bracket: %s (%s)", qPrintable(curr_str()), qPrintable(Token.string(tok)));
            Q_ASSERT_X(false, "not rbracket", "tm_brackets");
            prev();
        }

        if (t__RBRACKET == tok) {
            // break out of the loop
            break;
        }

        if (eol()) {
            DBG_EXPR(" expr index: %s", "end-of-line");
            break;
        }
        tok = curr_tok();
    }

    if (traits != tr_none) {

        DBG_EXPR(" expr add traits: %#x", traits);
        atom.add_trait(traits);

        if (traits & tr_INDEX) {
            DBG_EXPR(" expr set index: %#x", index.get_long());
            atom.set_index(QVariant::fromValue(index));
        }
    }

    // Set immediate flag according to traits
    if ((traits & tr_IMMEDIATE) || (traits & tr_AUGMENTED) || (traits & tr_ADDRESS_HUB) || (traits & tr_RELATIVE_HUB)) {
        m_IR.set_im_flags(true);
    }

    tok = curr_tok();
    while (m_idx < m_cnt) {

        switch (tok) {
        case t_comment_eol:
        case t_COMMA:
        case t__LPAREN:
        case t__LBRACKET:
            DBG_EXPR(" expr **** terminal: %s", qPrintable(curr_str()));
            break;

        case t__RPAREN:
        case t__RBRACKET:
            DBG_EXPR(" expr **** terminal: %s", qPrintable(curr_str()));
            next();
            break;

        default:
            DBG_EXPR(" expr **** non-terminal: %s", qPrintable(curr_str()));
            break;
        }

        if (!Token.is_type(tok, tm_comment))
            break;

        if (eol())
            break;
        tok = curr_tok();
    }
    DBG_EXPR(" expr atom = %s", qPrintable(atom.str()));

    DBG_EXPR("«««« %d", level);
    return atom;
}

/**
 * @brief Encode a PTRA/PTRB with or without pre/post inc/dec index expression
 * @param index const reference to the P2Atom with the index value
 * @return true on success, or false on error
 */
bool P2Asm::encode_ptr_index(P2Atom& index)
{
    if (index.isEmpty())
        return true;
    // TODO: encode ...
    p2_LONG value = index.get_long();
    Q_UNUSED(value)
    return false;
}

/**
 * @brief Check for the type of error of P2Opcode::set_dst() or P2Opcode::set_src()
 * @return true if no error, or false for error
 */
bool P2Asm::error_dst_or_src()
{
    // No error in pass 1
    if (m_pass < 2)
        return true;

    switch (m_IR.aug_error_code()) {
    case P2Opcode::err_none:
        return true;
    case P2Opcode::dst_augd_none:
        m_errors += tr("%1 constant $%2 is > $1ff but %3.")
                    .arg(QStringLiteral("DST"))
                    .arg(m_IR.aug_error_value(), 0, 16)
                    .arg(tr("no imediate mode"));
        break;

    case P2Opcode::dst_augd_im:
        m_errors += tr("%1 constant $%2 is > $1ff but %3.")
                    .arg(QStringLiteral("DST"))
                    .arg(m_IR.aug_error_value(), 0, 16)
                    .arg(tr("%1 is not set for %2")
                         .arg(QStringLiteral("IM"))
                         .arg(QStringLiteral("L")));
        break;

    case P2Opcode::dst_augd_wz:
        m_errors += tr("%1 constant $%2 is > $1ff but %3.")
                    .arg(QStringLiteral("DST"))
                    .arg(m_IR.aug_error_value(), 0, 16)
                    .arg(tr("%1 is not set for %2")
                         .arg(QStringLiteral("WZ"))
                         .arg(QStringLiteral("L")));
        break;


    case P2Opcode::src_augs_none:
        m_errors += tr("%1 constant $%2 is > $1ff but %3.")
                    .arg(QStringLiteral("SRC"))
                    .arg(m_IR.aug_error_value(), 0, 16)
                    .arg(tr("no imediate mode"));
        break;

    case P2Opcode::src_augs_im:
        m_errors += tr("%1 constant $%2 is > $1ff but %3.")
                    .arg(QStringLiteral("SRC"))
                    .arg(m_IR.aug_error_value(), 0, 16)
                    .arg(tr("%1 is not set for %2")
                         .arg(QStringLiteral("IM"))
                         .arg(QStringLiteral("I")));
        break;
    }
    emit Error(m_pass, m_lineno, m_errors.last());
    return false;
}

/**
 * @brief Set the destination %dst in the opcode and check for errors
 * @return true if no error, or false for error
 */
P2Atom P2Asm::parse_dst(P2Opcode::ImmFlag flag)
{
    m_IR.set_dst_imm(flag);
    P2Atom dst = parse_expression();
    if (!m_IR.set_dst(dst, m_ORG, m_ORGH))
            error_dst_or_src();
    return dst;
}

/**
 * @brief Set the source %src in the opcode and check for errors
 * @return true if no error, or false for error
 */
P2Atom P2Asm::parse_src(P2Opcode::ImmFlag flag)
{
    m_IR.set_src_imm(flag);
    P2Atom src = parse_expression();
    if (!m_IR.set_src(src, m_ORG, m_ORGH))
        error_dst_or_src();
    return src;
}

/**
 * @brief Evaluate the atom %src to create an index using PTRA or PTRB
 * @param src reference to the atom
 * @return true on success, or false on error
 */
bool P2Asm::parse_index(P2Atom& src, int scale)
{
    bool result = true;
    const bool ptrb = offs_PTRB == src.get_long();
    const bool pre = src.has_trait(tr_PRE);
    const bool post = src.has_trait(tr_POST);
    const bool inc = src.has_trait(tr_INC);
    const bool dec = src.has_trait(tr_DEC);
    bool negate = false;
    p2_index9_t index = {0};
    p2_index23_t index_augs = {0};

    m_IR.set_im(true);
    Q_UNUSED(scale)

    index.i.sup = true;
    /*
     * Structure of the 9 bit non-augmented index
     *
     * This structure defines the 9 bits of S/#/PTRx without AUGS
     * 0:   %0AAAAAAAA
     * 1:   %1SUPNNNNN
     * The topmost bit (%sup) determines whether the
     * lower 8 bits are to be interpreted as:
     * 0:   AAAAAAAA: 8 bits of address (0…255)
     * 1:   SUPNNNNN: structure definining the
     *      + S: PTRx false for PTRA, true for PTRB
     *      + U: false to keep PTRx same, true to update PTRx (PTRx += INDEX*SCALE)
     *      + P: false to use PTRx + INDEX*SCALE, true to use PTRx (post-modify)
     *      + NNNNN: index -16…15 for simple offsets, 0…15 for ++'s, 0…16 for --'s
     * SCALE is:
     *      = 1: for RDBYTE/WRBYTE
     *      = 2: for RDWORD/WRWORD
     *      = 4: for RDLONG/WRLONG/WMLONG
     */
    if (pre && inc) {
        index.i.U = true;
        index.i.P = false;
        index.i.N = 1;
    }

    if (pre && dec) {
        negate = true;
        index.i.U = true;
        index.i.P = false;
        index.i.N = -1;
    }

    index.i.S = ptrb;

    if (post && inc) {
        index.i.U = true;
        index.i.P = true;
        index.i.N = 1;
    }

    if (post && dec) {
        negate = true;
        index.i.U = true;
        index.i.P = true;
        index.i.N = -1;
    }

    if (src.has_trait(tr_INDEX)) {
        p2_LONG value = src.index_long();

        if (src.has_trait(tr_AUGMENTED)) {
            /*
             * Structure of the 23 bit augmented (AUGS) index
             *
             * This structure defines the 23 bits of S/#/PTRx with AUGS
             * 0:   %000000000000AAAAAAAAAAA_AAAAAAAAA
             * 1:   %000000001SUPNNNNNNNNNNN_NNNNNNNNN
             * The topmost bit (sup) determines whether the
             * less significant 23 bits are to be interpreted as:
             * 0:   000AAAAAAAAAAA_AAAAAAAAA: structure defining the
             *      + A: 20 bit of address ($000000…$fffff)
             * 1:   SUPNNNNNNNNNNN_NNNNNNNNN: structure definining the
             *      + S: PTRx false for PTRA, true for PTRB
             *      + U: false to keep PTRx same, true to update PTRx (PTRx += INDEX)
             *      + P: false to use PTRx + INDEX, true to use PTRx (post-modify)
             *      + NNNNNNNNNNN_NNNNNNNNN: 20 bit unscaled index
             */
            if (negate)
                value = -value;
            const int ivalue = static_cast<qint32>(value);
            if (ivalue < -0x80000 || ivalue > 0x7ffff) {
                m_errors += tr("Pointer index $%1 is out of bounds.")
                            .arg(value, 8, 16, QChar('0'));
                emit Error(m_pass, m_lineno, m_errors.last());
            }
            index_augs.i.sup = index.i.sup;
            index_augs.i.S = index.i.S;
            index_augs.i.U = index.i.U;
            index_augs.i.P = index.i.P;
            index_augs.i.N = value & A20MASK;

            m_IR.set_augs(index_augs.aug);

        } else {

            if (negate)
                value = -value;
            const int ivalue = static_cast<qint32>(value);

            if (m_v33mode) {
                if (index.i.U) {
                    if (ivalue < -16 || ivalue > 16 || ivalue == 0) {
                        m_errors += tr("Pointer index $%1 is invalid.")
                                    .arg(value, 8, 16, QChar('0'));
                        emit Error(m_pass, m_lineno, m_errors.last());
                        result = false;
                    }
                    if (value == 16)
                        value = 0;
                    index.i.N = static_cast<char>(value & 31);
                } else {
                    if (ivalue < -32 || ivalue > 31) {
                        m_errors += tr("Pointer index $%1 is invalid.")
                                    .arg(value, 8, 16, QChar('0'));
                        emit Error(m_pass, m_lineno, m_errors.last());
                        result = false;
                    }
                    index.i.P = ivalue < 0;
                    index.i.N = static_cast<char>(value & 31);
                }
            } else {
                if (ivalue < -16 || ivalue > 15) {
                    m_errors += tr("Pointer index $%1 is invalid.")
                                .arg(value, 8, 16, QChar('0'));
                    emit Error(m_pass, m_lineno, m_errors.last());
                    result = false;
                }
                index.i.N = static_cast<char>(value & 31);
            }

            m_IR.set_augs();
        }
    }

    m_IR.set_src(index.src);
    return result;
}

/**
 * @brief Expect end of line, i.e. no more parameters
 *
 * @return true on success, or false on error
 */
bool P2Asm::end_of_line()
{
    if (!skip_comments())
        return true;
    if (m_idx < m_cnt) {
        // ignore extra parameters?
        m_errors += tr("Found extra parameters: %1")
                  .arg(curr_str());
        emit Error(m_pass, m_lineno, m_errors.last());
    }
    return false;
}

/**
 * @brief A comma is expected
 *
 * @return true if comma found, false otherwise
 */
bool P2Asm::mandatory_COMMA()
{
    if (eol()) {
        m_errors += tr("Expected %1 but found %2.")
                  .arg(Token.string(t_COMMA))
                  .arg(tr("end of line"));
        emit Error(m_pass, m_lineno, m_errors.last());
        return false;
    }
    if (t_COMMA != curr_tok()) {
        m_errors += tr("Expected %1 but found %2.")
                  .arg(Token.string(t_COMMA))
                  .arg(Token.string(m_words.value(m_idx).tok()));
        emit Error(m_pass, m_lineno, m_errors.last());
        return false;
    }
    next();
    return true;
}

/**
 * @brief An optional comma is skipped
 *
 */
void P2Asm::optional_COMMA()
{
    if (!skip_comments())
        return;
    if (t_COMMA != curr_tok())
        return;
    next();
}

/**
 * @brief Optional WC, WZ, or WCZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::optional_WCZ()
{
    while (skip_comments()) {
        p2_token_e tok = curr_tok();
        switch (tok) {
        case t_WC:
            m_IR.set_wc();
            next();
            break;
        case t_WZ:
            m_IR.set_wz();
            next();
            break;
        case t_WCZ:
            m_IR.set_wcz();
            next();
            break;
        case t_COMMA:
            // expect more flags
            next();
            break;
        default:
            m_errors += tr("Unexpected flag update '%1' not %2")
                      .arg(m_words.value(m_idx).str())
                      .arg(tr("WC, WZ, or WCZ"));
            emit Error(m_pass, m_lineno, m_errors.last());
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
bool P2Asm::optional_WC()
{
    if (!skip_comments())
        return true;
    p2_token_e tok = m_words.value(m_idx).tok();
    switch (tok) {
    case t_WC:
        m_IR.set_wc();
        next();
        break;
    default:
        m_errors += tr("Unexpected flag update '%1' not %2")
                  .arg(m_words.value(m_idx).str())
                  .arg(tr("WC"));
        emit Error(m_pass, m_lineno, m_errors.last());
        return false;
    }
    return true;
}

/**
 * @brief Optional WZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::optional_WZ()
{
    if (!skip_comments())
        return true;
    p2_token_e tok = m_words.value(m_idx).tok();
    switch (tok) {
    case t_WZ:
        m_IR.set_wz();
        next();
        break;
    default:
        m_errors += tr("Unexpected flag update '%1' not %2")
                  .arg(m_words.value(m_idx).str())
                  .arg(tr("WZ"));
        emit Error(m_pass, m_lineno, m_errors.last());
        return false;
    }
    return true;
}

/**
 * @brief Mandatory ANDC or ANDZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::mandatory_ANDC_ANDZ()
{
    if (!skip_comments())
        return false;
    p2_token_e tok = m_words.value(m_idx).tok();
    switch (tok) {
    case t_ANDC:
        m_IR.set_wc();
        next();
        break;
    case t_ANDZ:
        m_IR.set_wz();
        next();
        break;
    default:
        m_errors += tr("Unexpected flag update '%1' not %2")
                  .arg(m_words.value(m_idx).str())
                  .arg(tr("ANDC/ANDZ"));
        emit Error(m_pass, m_lineno, m_errors.last());
        return false;
    }
    return true;
}

/**
 * @brief Mandatory ORC or ORZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::mandatory_ORC_ORZ()
{
    if (!skip_comments())
        return false;
    p2_token_e tok = m_words.value(m_idx).tok();
    switch (tok) {
    case t_ORC:
        m_IR.set_wc();
        next();
        break;
    case t_ORZ:
        m_IR.set_wz();
        next();
        break;
    default:
        m_errors += tr("Unexpected flag update '%1' not %2")
                  .arg(m_words.value(m_idx).str())
                  .arg(tr("ORC/ORZ"));
        emit Error(m_pass, m_lineno, m_errors.last());
        return false;
    }
    return true;
}

/**
 * @brief Mandatory XORC or XORZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::mandatory_XORC_XORZ()
{
    if (!skip_comments())
        return false;
    p2_token_e tok = m_words.value(m_idx).tok();
    switch (tok) {
    case t_XORC:
        m_IR.set_wc();
        next();
        break;
    case t_XORZ:
        m_IR.set_wz();
        next();
        break;
    default:
        m_errors += tr("Unexpected flag update '%1' not %2")
                  .arg(m_words.value(m_idx).str())
                  .arg(tr("XORC/XORZ"));
        emit Error(m_pass, m_lineno, m_errors.last());
        return false;
    }
    return true;
}

/**
 * @brief Expect no more parameters after instruction
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_INST()
{
    return end_of_line();
}

/**
 * @brief Expect parameters for D and {#}S
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_D_IM_S()
{
    if (commata_left() < 1) {
        const int idx = m_idx;
        P2Atom dst = parse_dst();
        m_idx = idx;
        P2Atom src = parse_src();
    } else {
        P2Atom dst = parse_dst();
        if (!mandatory_COMMA())
            return false;
        P2Atom src = parse_src(P2Opcode::imm_to_im);
    }
    return end_of_line();
}

/**
 * @brief Expect parameters for D and {#}S / PTRx followed by optional WC, WZ, WCZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_D_IM_S_PTRx_WCZ(int scale)
{
    if (commata_left() < 1) {
        const int idx = m_idx;
        P2Atom dst = parse_dst();
        m_idx = idx;
        P2Atom src = parse_src();
    } else {
        P2Atom dst = parse_dst();
        if (!mandatory_COMMA())
            return false;
        P2Atom src = parse_src(P2Opcode::imm_to_im);
        if (offs_PTRA == src.get_long() || offs_PTRB == src.get_long())
            parse_index(src, scale);
    }
    optional_WCZ();
    return end_of_line();
}

/**
 * @brief Expect parameters for D and {#}S / PTRx
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_D_IM_S_PTRx(int scale)
{
    if (commata_left() < 1) {
        const int idx = m_idx;
        P2Atom dst = parse_dst();
        m_idx = idx;
        P2Atom src = parse_src();
    } else {
        P2Atom dst = parse_dst();
        if (!mandatory_COMMA())
            return false;
        P2Atom src = parse_src(P2Opcode::imm_to_im);
        if (offs_PTRA == src.get_long() || offs_PTRB == src.get_long())
            parse_index(src, scale);
    }
    return end_of_line();
}

/**
 * @brief Expect parameters for D and optional WC, WZ, or WCZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_D_WCZ()
{
    P2Atom dst = parse_dst();
    optional_WCZ();
    return end_of_line();
}

/**
 * @brief Expect optional WC, WZ, or WCZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_WCZ()
{
    optional_WCZ();
    return end_of_line();
}

/**
 * @brief Expect parameters for D
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_D()
{
    P2Atom dst = parse_dst();
    return end_of_line();
}

/**
 * @brief Expect parameters for {#}D setting WZ for immediate
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_WZ_D()
{
    P2Atom dst = parse_dst(P2Opcode::imm_to_wz);
    return end_of_line();
}

/**
 * @brief Expect parameters for {#}D
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_IM_D()
{
    P2Atom dst = parse_dst(P2Opcode::imm_to_im);
    return end_of_line();
}

/**
 * @brief Expect parameters for {#}D, and optional WC, WZ, or WCZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_IM_D_WCZ()
{
    P2Atom dst = parse_dst(P2Opcode::imm_to_im);
    optional_WCZ();
    return end_of_line();
}

/**
 * @brief Expect parameters for {#}D, and optional WC
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_IM_D_WC()
{
    P2Atom dst = parse_dst(P2Opcode::imm_to_im);
    optional_WC();
    return end_of_line();
}

/**
 * @brief Expect parameters for {#}D, and mandatory ANDC or ANDZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_IM_D_ANDC_ANDZ()
{
    P2Atom dst = parse_dst(P2Opcode::imm_to_im);
    mandatory_ANDC_ANDZ();
    return end_of_line();
}

/**
 * @brief Expect parameters for {#}D, and mandatory ORC or ORZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_IM_D_ORC_ORZ()
{
    P2Atom dst = parse_dst(P2Opcode::imm_to_im);
    mandatory_ORC_ORZ();
    return end_of_line();
}

/**
 * @brief Expect parameters for {#}D, and mandatory XORC or XORZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_IM_D_XORC_XORZ()
{
    P2Atom dst = parse_dst(P2Opcode::imm_to_im);
    mandatory_XORC_XORZ();
    return end_of_line();
}

/**
 * @brief Expect parameters for D, and {#}S, and optional WC, WZ, or WCZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_D_IM_S_WCZ()
{
    if (commata_left() < 1) {
        // if there is no comma, expect D = S and no #S
        const int idx = m_idx;
        P2Atom dst = parse_dst();
        m_idx = idx;
        P2Atom src = parse_src();
    } else {
        P2Atom dst = parse_dst();
        if (!mandatory_COMMA())
            return false;
        P2Atom src = parse_src(P2Opcode::imm_to_im);
    }

    optional_WCZ();
    return end_of_line();
}

/**
 * @brief Expect parameters for D, and {#}S, and mandatory ANDC or ANDZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_D_IM_S_ANDCZ()
{
    if (commata_left() < 1) {
        // if there is no comma, expect D = S and no #S
        const int idx = m_idx;
        P2Atom dst = parse_dst();
        m_idx = idx;
        P2Atom src = parse_src();
    } else {
        P2Atom dst = parse_dst();
        if (!mandatory_COMMA())
            return false;
        P2Atom src = parse_src(P2Opcode::imm_to_im);
    }
    mandatory_ANDC_ANDZ();
    return end_of_line();
}

/**
 * @brief Expect parameters for D, and {#}S, and mandatory ORC or ORZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_D_IM_S_ORCZ()
{
    if (commata_left() < 1) {
        // if there is no comma, expect D = S and no #S
        const int idx = m_idx;
        P2Atom dst = parse_dst();
        m_idx = idx;
        P2Atom src = parse_src();
    } else {
        P2Atom dst = parse_dst();
        if (!mandatory_COMMA())
            return false;
        P2Atom src = parse_src(P2Opcode::imm_to_im);
    }

    mandatory_ORC_ORZ();
    return end_of_line();
}

/**
 * @brief Expect parameters for D, and {#}S, and mandatory XORC or XORZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_D_IM_S_XORCZ()
{
    if (commata_left() < 1) {
        // if there is no comma, expect D = S and no #S
        const int idx = m_idx;
        P2Atom dst = parse_dst();
        m_idx = idx;
        P2Atom src = parse_src();
    } else {
        P2Atom dst = parse_dst();
        if (!mandatory_COMMA())
            return false;
        P2Atom src = parse_src(P2Opcode::imm_to_im);
    }

    mandatory_XORC_XORZ();
    return end_of_line();
}

/**
 * @brief Expect parameters for D, and {#}S, and optional WC
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_D_IM_S_WC()
{
    if (commata_left() < 1) {
        // if there is no comma, expect D = S and no #S
        const int idx = m_idx;
        P2Atom dst = parse_dst();
        m_idx = idx;
        P2Atom src = parse_src();
    } else {
        P2Atom dst = parse_dst();
        if (!mandatory_COMMA())
            return false;
        P2Atom src = parse_src(P2Opcode::imm_to_im);
    }

    optional_WC();
    return end_of_line();
}

/**
 * @brief Expect parameters for D, and {#}S, and optional WZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_D_IM_S_WZ()
{
    if (commata_left() < 1) {
        // if there is no comma, expect D = S and no #S
        const int idx = m_idx;
        P2Atom dst = parse_dst();
        m_idx = idx;
        P2Atom src = parse_src();
    } else {
        P2Atom dst = parse_dst();
        if (!mandatory_COMMA())
            return false;
        P2Atom src = parse_src(P2Opcode::imm_to_im);
    }

    optional_WZ();
    return end_of_line();
}

/**
 * @brief Expect parameters for {#}D, and {#}S
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_WZ_D_IM_S()
{
    if (commata_left() < 1) {
        // if there is no comma, expect D = S and no #S
        const int idx = m_idx;
        P2Atom dst = parse_dst();
        m_idx = idx;
        P2Atom src = parse_src();
    } else {
        P2Atom dst = parse_dst(P2Opcode::imm_to_wz);
        if (!mandatory_COMMA())
            return false;
        P2Atom src = parse_src(P2Opcode::imm_to_im);
    }
    return end_of_line();
}

/**
 * @brief Expect parameters for {#}D, and {#}S or PTRx[index]
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_WZ_D_IM_S_PTRx(int scale)
{
    if (commata_left() < 1) {
        // if there is no comma, expect D = S and no #S
        const int idx = m_idx;
        P2Atom dst = parse_dst();
        m_idx = idx;
        P2Atom src = parse_src();
    } else {
        P2Atom dst = parse_dst(P2Opcode::imm_to_wz);
        if (!mandatory_COMMA())
            return false;
        P2Atom src = parse_src(P2Opcode::imm_to_im);
        if (offs_PTRA == src.get_long() || offs_PTRB == src.get_long())
            parse_index(src, scale);
    }
    return end_of_line();
}

/**
 * @brief Expect parameters for {#}D, and {#}S, followed by an optional WC
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_WZ_D_IM_S_WC()
{
    if (commata_left() < 1) {
        // if there is no comma, expect D = S and no #S
        const int idx = m_idx;
        P2Atom dst = parse_dst();
        m_idx = idx;
        P2Atom src = parse_src();
    } else {
        P2Atom dst = parse_dst(P2Opcode::imm_to_wz);
        if (!mandatory_COMMA())
            return false;
        P2Atom src = parse_src(P2Opcode::imm_to_im);
    }
    return optional_WC();
}

/**
 * @brief Expect parameters for D, and {#}S, and #N (0 … 7)
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_D_IM_S_NNN(uint max)
{
    P2Atom dst = parse_dst();
    if (!mandatory_COMMA())
        return false;
    P2Atom src = parse_src(P2Opcode::imm_to_im);
    if (!mandatory_COMMA())
        return false;
    if (m_idx < m_cnt) {
        P2Atom n = parse_expression();
        if (n.isEmpty()) {
            m_errors += tr("Expected immediate #n");
            emit Error(m_pass, m_lineno, m_errors.last());
            return false;
        }
        if (n.get_long() > max) {
            m_errors += tr("Immediate #n not in 0-%1 (%2)")
                      .arg(max)
                      .arg(n.get_long());
            emit Error(m_pass, m_lineno, m_errors.last());
            return false;
        }
        m_IR.set_nnn(n.get_long());
    } else {
        m_errors += tr("Missing immediate #n");
        emit Error(m_pass, m_lineno, m_errors.last());
        return false;
    }
    return end_of_line();
}

/**
 * @brief Expect parameters for {#}S
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_IM_S()
{
    P2Atom src = parse_src(P2Opcode::imm_to_im);
    return end_of_line();
}

/**
 * @brief Expect parameters for {#}S, and optional WC
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_IM_S_WC()
{
    P2Atom src = parse_src(P2Opcode::imm_to_im);
    optional_WC();
    return end_of_line();
}


/**
 * @brief Expect parameters for #AAAAAAAAAAAA (20 bit address for CALL/CALLA/CALLB/LOC)
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_PTRx_PC_A20()
{
    P2Atom dst = parse_expression();
    switch (dst.get_long()) {
    case offs_PA:
        break;
    case offs_PB:
        m_IR.set_wz();
        break;
    case offs_PTRA:
        m_IR.set_wc();
        break;
    case offs_PTRB:
        m_IR.set_wcz();
        break;
    default:
        m_errors += tr("Invalid pointer parameter: %1")
                  .arg(dst.get_long());
        emit Error(m_pass, m_lineno, m_errors.last());
        return false;
    }
    if (!mandatory_COMMA())
        return false;

    P2Atom atom = parse_expression();
    p2_LONG addr = atom.get_long();
    m_IR.set_a20(addr);

    return end_of_line();
}

/**
 * @brief Expect parameters for #AAAAAAAAAAAAAAAAAAAA (20 bit address for CALL/CALLA/CALLB)
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_PC_A20()
{
    P2Atom atom = parse_expression();
    p2_LONG addr = atom.get_long();
    if (0 == (atom.traits() & tr_ABSOLUTE)) {
        // Check if a relative address is shorter
        p2_LONG base = m_ORG < LUT_ADDR0 ? COG_ADDR0 : LUT_ADDR0;
        p2_LONG raddr = base < addr ? addr - base : base - addr;
        if (raddr < HUB_ADDR0)
            raddr = raddr / 4;
        if (raddr < addr) {
            m_IR.set_r20(raddr);
        } else {
            m_IR.set_a20(addr);
        }
    } else {
        m_IR.set_a20(addr);
    }

    return end_of_line();
}

/**
 * @brief Expect parameters for #NNNNNNNNNNNNNNNNNNNNNNN (23 bit address for AUGD/AUGS)
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_IMM23()
{
    P2Atom atom = parse_expression();
    p2_LONG addr = atom.get_long();
    m_IR.set_imm23(addr);

    return end_of_line();
}

/**
 * @brief Assignment operation
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_assign()
{
    next();
    m_advance = 0;      // Don't advance PC
    P2Atom atom = parse_expression();
    P2Symbol sym = m_symbols->symbol(m_symbol);
    m_symbols->set_value(m_symbol, atom.value());
    m_IR.set_equ(atom);
    if (con_section != m_section) {
        m_errors += tr("Not in constant section (CON) but found %1.")
                    .arg(tr("assignment"));
        emit Error(m_pass, m_lineno, m_errors.last());
    }
    return end_of_line();
}

/**
 * @brief Start value for an enumeration, i.e. list of constant symbols
 *
 * Example:
 *      #32, port32, port33, port34, led1, led2, led3
 *
 * Result:
 *      port32  = 32
 *      port33  = 33
 *      port34  = 34
 *      led1    = 35
 *      led2    = 36
 *      led3    = 37
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_enum_initial()
{
    next();             // skip over "#"
    m_advance = 0;      // Don't advance PC
    P2Atom atom = parse_expression();
    m_IR.set_equ(atom);

    if (con_section != m_section) {
        m_errors += tr("Not in constant section (CON) but found %1.")
                    .arg(tr("enumeration start"));
        emit Error(m_pass, m_lineno, m_errors.last());
        return end_of_line();
    }

    if (eol())
        return false;
    if (!mandatory_COMMA())
        return false;
    while (!eol()) {
        if (t_COMMA == curr_tok()) {
            // skip next comma
            next();
            continue;
        }

        if (t_symbol == curr_tok()) {
            QString symbol = find_symbol(m_section, curr_str());
            define_symbol(symbol, atom);    // append global name to section::symbol
            if (t__LBRACKET == curr_tok()) {
                qDebug("%s: %s with index", __func__, qPrintable(symbol));
            }
            prev();
            atom = parse_expression();
            if (atom.has_trait(tr_INDEX))
                atom += atom.index_long();
            else
                atom += 1u; // increase enumerator
            continue;
        }

        m_errors += tr("Unexpected token %1 in enumeration: %2.")
                    .arg(curr_tok())
                    .arg(curr_str());
        emit Error(m_pass, m_lineno, m_errors.last());
        m_idx = m_cnt;
    }
    m_enum = atom;
    return end_of_line();
}

bool P2Asm::asm_enum_continue()
{
    // first symbol was already taken and defined
    m_advance = 0;      // Don't advance PC
    m_IR.set_as_IR(false);
    P2Atom atom = m_enum;
    m_IR.set_equ(atom);


    if (con_section != m_section) {
        m_errors += tr("Not in constant section (CON) but found %1.")
                    .arg(tr("enumeration continuation"));
        emit Error(m_pass, m_lineno, m_errors.last());
        return end_of_line();
    }

    if (!mandatory_COMMA())
        return false;

    while (!eol()) {

        if (t_COMMA == curr_tok()) {
            // skip comma
            next();
            continue;
        }

        if (t_symbol == curr_tok()) {
            QString symbol = find_symbol(m_section, curr_str());
            // append global name to section::symbol
            define_symbol(symbol, atom);
            if (t__LBRACKET == curr_tok()) {
                qDebug("%s: %s with index", __func__, qPrintable(symbol));
            }
            prev();
            atom = parse_expression();
            if (atom.has_trait(tr_INDEX))
                atom += atom.index_long();
            else
                atom += 1u; // increase enumerator
            continue;
        }

        m_errors += tr("Unexpected token %1 in enumeration: %2.")
                    .arg(curr_tok())
                    .arg(curr_str());
        emit Error(m_pass, m_lineno, m_errors.last());
        m_idx = m_cnt;
    }
    m_enum = atom;
    return end_of_line();
}

/**
 * @brief Align to next WORD operation
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_ALIGNW()
{
    next();
    m_IR.set_as_IR(false);
    m_advance = 1u - (m_ORG & 1u);
    return end_of_line();
}

/**
 * @brief Align to next LONG operation
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_ALIGNL()
{
    next();
    m_IR.set_as_IR(false);
    if (m_ORG & 3u)
        m_advance = 4u - (m_ORG & 3u);
    return end_of_line();
}

/**
 * @brief Origin operation
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_ORG()
{
    next();
    m_advance = 0;      // Don't advance PC
    P2Atom atom = parse_expression();
    p2_LONG value = atom.isEmpty() ? 0 : 4 * atom.get_long();
    if (value >= HUB_ADDR0) {
        m_errors += tr("COG origin ($%1) exceeds limit %2.")
                    .arg(value, 0, 16, QChar('0'))
                    .arg(QLatin1String("$1ff"));
        emit Error(m_pass, m_lineno, m_errors.last());
    } else {
        m_ORG = value;
    }
    m_IR.set_equ(m_ORG);
    m_symbols->set_value(m_symbol, P2Union(value));
    return end_of_line();
}

/**
 * @brief Origin with fill operation
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_ORGF()
{
    next();
    m_advance = 0;      // Don't advance PC
    m_IR.set_as_IR(false);
    P2Atom atom = parse_expression();
    p2_LONG value = atom.isEmpty() ? 0 : 4 * atom.get_long();
    if (value >= HUB_ADDR0) {
        m_errors += tr("COG origin ($%1) exceeds limit %2.")
                    .arg(value, 0, 16, QChar('0'))
                    .arg(QLatin1String("$1ff"));
        emit Error(m_pass, m_lineno, m_errors.last());
        value = 0;
    }
    for (p2_LONG pc = m_ORG; pc < value; pc += 4) {
        p2_LONG empty = 0;
        m_data.add_long(empty);
    }
    m_advance = m_data.usize();
    m_IR.set_equ(m_ORG);
    m_symbols->set_value(m_symbol, P2Union(value));
    return end_of_line();
}

/**
 * @brief Origin high operation
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_ORGH()
{
    next();
    m_advance = 0;      // Don't advance PC
    m_IR.set_as_IR(false);
    P2Atom atom = parse_expression();
    p2_LONG value = atom.isEmpty() ? HUB_ADDR0 : atom.get_long();
    if (value <= HUB_ADDR0)
        value *= 4;
    if (value >= MEM_SIZE) {
        m_errors += tr("HUB address exceeds limit $%1.")
                    .arg(value, 0, 16, QChar('0'));
        emit Error(m_pass, m_lineno, m_errors.last());
        value = MEM_SIZE;
    }
    m_ORGH = value;
    m_IR.set_equ(m_ORGH);
    m_symbols->set_value(m_symbol, P2Union(value));
    return end_of_line();
}

/**
 * @brief Expect an address which is checked for being < curr_pc
 * @return true on success, or false on error
 */
bool P2Asm::asm_FIT()
{
    next();
    m_advance = 0;
    m_IR.set_as_IR(false);
    P2Atom atom = parse_expression();
    const p2_LONG fit = atom.isNull() ? HUB_ADDR0 : atom.get_long();
    const p2_LONG org = m_ORG / 4;
    if (fit < org) {
        m_errors += tr("Code does not fit below $%1 (ORG is $%2)")
                  .arg(fit, 0, 16)
                  .arg(org, 0, 16);
        emit Error(m_pass, m_lineno, m_errors.last());
    }
    return end_of_line();
}

/**
 * @brief Switch to data section
 * @return true on success
 */
bool P2Asm::asm_DAT()
{
    next();
    m_advance = 0;
    m_IR.set_as_IR(false);
    m_section = dat_section;
    return true;
}

/**
 * @brief Switch to constant section
 * @return true on success
 */
bool P2Asm::asm_CON()
{
    next();
    m_advance = 0;
    m_IR.set_as_IR(false);
    m_section = con_section;
    return true;
}

/**
 * @brief Switch to publics section
 * @return true on success
 */
bool P2Asm::asm_PUB()
{
    next();
    m_advance = 0;
    m_IR.set_as_IR(false);
    m_section = pub_section;
    return true;
}

/**
 * @brief Switch to private section
 * @return true on success
 */
bool P2Asm::asm_PRI()
{
    next();
    m_advance = 0;
    m_IR.set_as_IR(false);
    m_section = pri_section;
    return true;
}

/**
 * @brief Switch to variable section
 * @return true on success
 */
bool P2Asm::asm_VAR()
{
    next();
    m_advance = 0;
    m_IR.set_as_IR(false);
    m_section = var_section;
    return true;
}

/**
 * @brief Expect one or more bytes of data
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_BYTE()
{
    P2Atom atom;
    next();
    m_advance = 0;      // Don't advance PC, as it's done based on m_data
    m_IR.set_as_IR(false);

    while (m_idx < m_cnt) {
        P2Atom atom = parse_expression();
        p2_BYTES _bytes = atom.get_bytes();
        p2_LONG count = atom.index_long();
        m_data.set_type(ut_Byte);
        m_data.add_bytes(_bytes);
        if (count > 0) {
            Q_ASSERT(count < 4096);
            while (count-- > 1)
                m_data.add_bytes(_bytes);
            m_data.set_type(ut_String);
        }
        optional_COMMA();
    }
    return end_of_line();
}

/**
 * @brief Expect one or more words of data
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_WORD()
{
    P2Atom atom(ut_Word);
    next();
    m_advance = 0;      // Don't advance PC, as it's done based on m_data
    m_IR.set_as_IR(false);

    while (m_idx < m_cnt) {
        P2Atom atom = parse_expression();
        p2_WORD _word = atom.get_word();
        p2_LONG count = atom.index_long();
        m_data.set_type(ut_Word);
        m_data.add_word(_word);
        if (count > 0) {
            Q_ASSERT(count < 4096);
            while (count-- > 1)
                m_data.add_word(_word);
        }
        optional_COMMA();
    }

    return end_of_line();
}

/**
 * @brief Expect one or more longs of data
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_LONG()
{
    next();
    m_advance = 0;      // Don't advance PC, as it's done based on m_data
    m_IR.set_as_IR(false);

    while (m_idx < m_cnt) {
        P2Atom atom = parse_expression();
        p2_LONG _long = atom.get_long();
        p2_LONG count = atom.index_long();
        m_data.set_type(ut_Long);
        m_data.add_long(_long);
        if (count > 0) {
            Q_ASSERT(count < 4096);
            while (count-- > 1)
                m_data.add_long(_long);
        }
        optional_COMMA();
    }

    return end_of_line();
}

/**
 * @brief Reserve a number of bytes of data
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_RES()
{
    next();
    m_advance = 0;      // Don't advance PC if no value is specified
    m_IR.set_as_IR(false);

    while (m_idx < m_cnt) {
        P2Atom atom = parse_expression();
        p2_LONG count = atom.get_long();
        p2_BYTE b = 0;
        while (count-- > 0)
            m_data.add_byte(b);
        optional_COMMA();
    }
    m_data.set_type(ut_Byte);

    return end_of_line();
}

/**
 * @brief Expect one or more filenames in double quotes
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_FILE()
{
    next();
    m_advance = 0;      // Don't advance PC, as it's done based on m_data
    m_IR.set_as_IR(false);

    while (m_idx < m_cnt) {
        P2Atom atom = parse_expression();
        QString filename = QString("%1/%2")
                           .arg(m_pathname)
                           .arg(atom.string());
        QFile data(filename);
        if (data.open(QIODevice::ReadOnly)) {
            m_data.add_array(data.readAll());
            m_data.set_type(ut_String);
            data.close();
        } else {
            m_errors += tr("Could not open file \"%1\" for reading.")
                        .arg(filename);
            emit Error(m_pass, m_lineno, m_errors.last());
            return false;
        }
        optional_COMMA();
    }
    return end_of_line();

}

/**
 * @brief No operation.
 *<pre>
 * 0000 0000000 000 000000000 000000000
 *
 * NOP
 *</pre>
 */
bool P2Asm::asm_NOP()
{
    next();
    m_IR.set_opcode(0);
    return parse_INST();
}

/**
 * @brief Rotate right.
 *<pre>
 * EEEE 0000000 CZI DDDDDDDDD SSSSSSSSS
 *
 * ROR     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [31:0]  of ({D[31:0], D[31:0]}     >> S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[0].
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_ROR()
{
    next();
    m_IR.set_inst7(p2_ROR);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Rotate left.
 *<pre>
 * EEEE 0000001 CZI DDDDDDDDD SSSSSSSSS
 *
 * ROL     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [63:32] of ({D[31:0], D[31:0]}     << S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[31].
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_ROL()
{
    next();
    m_IR.set_inst7(p2_ROR);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Shift right.
 *<pre>
 * EEEE 0000010 CZI DDDDDDDDD SSSSSSSSS
 *
 * SHR     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [31:0]  of ({32'b0, D[31:0]}       >> S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[0].
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_SHR()
{
    next();
    m_IR.set_inst7(p2_SHR);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Shift left.
 *<pre>
 * EEEE 0000011 CZI DDDDDDDDD SSSSSSSSS
 *
 * SHL     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [63:32] of ({D[31:0], 32'b0}       << S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[31].
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_SHL()
{
    next();
    m_IR.set_inst7(p2_SHL);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Rotate carry right.
 *<pre>
 * EEEE 0000100 CZI DDDDDDDDD SSSSSSSSS
 *
 * RCR     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [31:0]  of ({{32{C}}, D[31:0]}     >> S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[0].
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_RCR()
{
    next();
    m_IR.set_inst7(p2_RCR);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Rotate carry left.
 *<pre>
 * EEEE 0000101 CZI DDDDDDDDD SSSSSSSSS
 *
 * RCL     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [63:32] of ({D[31:0], {32{C}}}     << S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[31].
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_RCL()
{
    next();
    m_IR.set_inst7(p2_RCL);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Shift arithmetic right.
 *<pre>
 * EEEE 0000110 CZI DDDDDDDDD SSSSSSSSS
 *
 * SAR     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [31:0]  of ({{32{D[31]}}, D[31:0]} >> S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[0].
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_SAR()
{
    next();
    m_IR.set_inst7(p2_SAR);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Shift arithmetic left.
 *<pre>
 * EEEE 0000111 CZI DDDDDDDDD SSSSSSSSS
 *
 * SAL     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [63:32] of ({D[31:0], {32{D[0]}}}  << S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[31].
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_SAL()
{
    next();
    m_IR.set_inst7(p2_SAL);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Add S into D.
 *<pre>
 * EEEE 0001000 CZI DDDDDDDDD SSSSSSSSS
 *
 * ADD     D,{#}S   {WC/WZ/WCZ}
 *
 * D = D + S.
 * C = carry of (D + S).
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_ADD()
{
    next();
    m_IR.set_inst7(p2_ADD);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Add (S + C) into D, extended.
 *<pre>
 * EEEE 0001001 CZI DDDDDDDDD SSSSSSSSS
 *
 * ADDX    D,{#}S   {WC/WZ/WCZ}
 *
 * D = D + S + C.
 * C = carry of (D + S + C).
 * Z = Z AND (result == 0).
 *</pre>
 */
bool P2Asm::asm_ADDX()
{
    next();
    m_IR.set_inst7(p2_ADDX);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Add S into D, signed.
 *<pre>
 * EEEE 0001010 CZI DDDDDDDDD SSSSSSSSS
 *
 * ADDS    D,{#}S   {WC/WZ/WCZ}
 *
 * D = D + S.
 * C = correct sign of (D + S).
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_ADDS()
{
    next();
    m_IR.set_inst7(p2_ADDS);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Add (S + C) into D, signed and extended.
 *<pre>
 * EEEE 0001011 CZI DDDDDDDDD SSSSSSSSS
 *
 * ADDSX   D,{#}S   {WC/WZ/WCZ}
 *
 * D = D + S + C.
 * C = correct sign of (D + S + C).
 * Z = Z AND (result == 0).
 *</pre>
 */
bool P2Asm::asm_ADDSX()
{
    next();
    m_IR.set_inst7(p2_ADDSX);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Subtract S from D.
 *<pre>
 * EEEE 0001100 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUB     D,{#}S   {WC/WZ/WCZ}
 *
 * D = D - S.
 * C = borrow of (D - S).
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_SUB()
{
    next();
    m_IR.set_inst7(p2_SUB);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Subtract (S + C) from D, extended.
 *<pre>
 * EEEE 0001101 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUBX    D,{#}S   {WC/WZ/WCZ}
 *
 * D = D - (S + C).
 * C = borrow of (D - (S + C)).
 * Z = Z AND (result == 0).
 *</pre>
 */
bool P2Asm::asm_SUBX()
{
    next();
    m_IR.set_inst7(p2_SUBX);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Subtract S from D, signed.
 *<pre>
 * EEEE 0001110 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUBS    D,{#}S   {WC/WZ/WCZ}
 *
 * D = D - S.
 * C = correct sign of (D - S).
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_SUBS()
{
    next();
    m_IR.set_inst7(p2_SUBS);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Subtract (S + C) from D, signed and extended.
 *<pre>
 * EEEE 0001111 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUBSX   D,{#}S   {WC/WZ/WCZ}
 *
 * D = D - (S + C).
 * C = correct sign of (D - (S + C)).
 * Z = Z AND (result == 0).
 *</pre>
 */
bool P2Asm::asm_SUBSX()
{
    next();
    m_IR.set_inst7(p2_SUBSX);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Compare D to S.
 *<pre>
 * EEEE 0010000 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMP     D,{#}S   {WC/WZ/WCZ}
 *
 * C = borrow of (D - S).
 * Z = (D == S).
 *</pre>
 */
bool P2Asm::asm_CMP()
{
    next();
    m_IR.set_inst7(p2_CMP);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Compare D to (S + C), extended.
 *<pre>
 * EEEE 0010001 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMPX    D,{#}S   {WC/WZ/WCZ}
 *
 * C = borrow of (D - (S + C)).
 * Z = Z AND (D == S + C).
 *</pre>
 */
bool P2Asm::asm_CMPX()
{
    next();
    m_IR.set_inst7(p2_CMPX);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Compare D to S, signed.
 *<pre>
 * EEEE 0010010 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMPS    D,{#}S   {WC/WZ/WCZ}
 *
 * C = correct sign of (D - S).
 * Z = (D == S).
 *</pre>
 */
bool P2Asm::asm_CMPS()
{
    next();
    m_IR.set_inst7(p2_CMPS);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Compare D to (S + C), signed and extended.
 *<pre>
 * EEEE 0010011 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMPSX   D,{#}S   {WC/WZ/WCZ}
 *
 * C = correct sign of (D - (S + C)).
 * Z = Z AND (D == S + C).
 *</pre>
 */
bool P2Asm::asm_CMPSX()
{
    next();
    m_IR.set_inst7(p2_CMPSX);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Compare S to D (reverse).
 *<pre>
 * EEEE 0010100 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMPR    D,{#}S   {WC/WZ/WCZ}
 *
 * C = borrow of (S - D).
 * Z = (D == S).
 *</pre>
 */
bool P2Asm::asm_CMPR()
{
    next();
    m_IR.set_inst7(p2_CMPR);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Compare D to S, get MSB of difference into C.
 *<pre>
 * EEEE 0010101 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMPM    D,{#}S   {WC/WZ/WCZ}
 *
 * C = MSB of (D - S).
 * Z = (D == S).
 *</pre>
 */
bool P2Asm::asm_CMPM()
{
    next();
    m_IR.set_inst7(p2_CMPM);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Subtract D from S (reverse).
 *<pre>
 * EEEE 0010110 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUBR    D,{#}S   {WC/WZ/WCZ}
 *
 * D = S - D.
 * C = borrow of (S - D).
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_SUBR()
{
    next();
    m_IR.set_inst7(p2_SUBR);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Compare and subtract S from D if D >= S.
 *<pre>
 * EEEE 0010111 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMPSUB  D,{#}S   {WC/WZ/WCZ}
 *
 * If D => S then D = D - S and C = 1, else D same and C = 0.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_CMPSUB()
{
    next();
    m_IR.set_inst7(p2_CMPSUB);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Force D >= S.
 *<pre>
 * EEEE 0011000 CZI DDDDDDDDD SSSSSSSSS
 *
 * FGE     D,{#}S   {WC/WZ/WCZ}
 *
 * If D < S then D = S and C = 1, else D same and C = 0.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_FGE()
{
    next();
    m_IR.set_inst7(p2_FGE);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Force D <= S.
 *<pre>
 * EEEE 0011001 CZI DDDDDDDDD SSSSSSSSS
 *
 * FLE     D,{#}S   {WC/WZ/WCZ}
 *
 * If D > S then D = S and C = 1, else D same and C = 0.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_FLE()
{
    next();
    m_IR.set_inst7(p2_FLE);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Force D >= S, signed.
 *<pre>
 * EEEE 0011010 CZI DDDDDDDDD SSSSSSSSS
 *
 * FGES    D,{#}S   {WC/WZ/WCZ}
 *
 * If D < S then D = S and C = 1, else D same and C = 0.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_FGES()
{
    next();
    m_IR.set_inst7(p2_FGES);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Force D <= S, signed.
 *<pre>
 * EEEE 0011011 CZI DDDDDDDDD SSSSSSSSS
 *
 * FLES    D,{#}S   {WC/WZ/WCZ}
 *
 * If D > S then D = S and C = 1, else D same and C = 0.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_FLES()
{
    next();
    m_IR.set_inst7(p2_FLES);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Sum +/-S into D by  C.
 *<pre>
 * EEEE 0011100 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUMC    D,{#}S   {WC/WZ/WCZ}
 *
 * If C = 1 then D = D - S, else D = D + S.
 * C = correct sign of (D +/- S).
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_SUMC()
{
    next();
    m_IR.set_inst7(p2_SUMC);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Sum +/-S into D by !C.
 *<pre>
 * EEEE 0011101 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUMNC   D,{#}S   {WC/WZ/WCZ}
 *
 * If C = 0 then D = D - S, else D = D + S.
 * C = correct sign of (D +/- S).
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_SUMNC()
{
    next();
    m_IR.set_inst7(p2_SUMNC);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Sum +/-S into D by  Z.
 *<pre>
 * EEEE 0011110 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUMZ    D,{#}S   {WC/WZ/WCZ}
 *
 * If Z = 1 then D = D - S, else D = D + S.
 * C = correct sign of (D +/- S).
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_SUMZ()
{
    next();
    m_IR.set_inst7(p2_SUMZ);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Sum +/-S into D by !Z.
 *<pre>
 * EEEE 0011111 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUMNZ   D,{#}S   {WC/WZ/WCZ}
 *
 * If Z = 0 then D = D - S, else D = D + S.
 * C = correct sign of (D +/- S).
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_SUMNZ()
{
    next();
    m_IR.set_inst7(p2_SUMNZ);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Test bit S[4:0] of  D, write to C/Z.
 *<pre>
 * EEEE 0100000 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTB   D,{#}S         WC/WZ
 *
 * C/Z =          D[S[4:0]].
 *</pre>
 */
bool P2Asm::asm_TESTB_W()
{
    next();
    m_IR.set_inst7(p2_TESTB_W);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Test bit S[4:0] of !D, write to C/Z.
 *<pre>
 * EEEE 0100001 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTBN  D,{#}S         WC/WZ
 *
 * C/Z =         !D[S[4:0]].
 *</pre>
 */
bool P2Asm::asm_TESTBN_W()
{
    next();
    m_IR.set_inst7(p2_TESTBN_W);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Test bit S[4:0] of  D, AND into C/Z.
 *<pre>
 * EEEE 0100010 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTB   D,{#}S     ANDC/ANDZ
 *
 * C/Z = C/Z AND  D[S[4:0]].
 *</pre>
 */
bool P2Asm::asm_TESTB_AND()
{
    next();
    m_IR.set_inst7(p2_TESTB_AND);
    return parse_D_IM_S_ANDCZ();
}

/**
 * @brief Test bit S[4:0] of !D, AND into C/Z.
 *<pre>
 * EEEE 0100011 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTBN  D,{#}S     ANDC/ANDZ
 *
 * C/Z = C/Z AND !D[S[4:0]].
 *</pre>
 */
bool P2Asm::asm_TESTBN_AND()
{
    next();
    m_IR.set_inst7(p2_TESTBN_AND);
    return parse_D_IM_S_ANDCZ();
}

/**
 * @brief Test bit S[4:0] of  D, OR  into C/Z.
 *<pre>
 * EEEE 0100100 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTB   D,{#}S       ORC/ORZ
 *
 * C/Z = C/Z OR   D[S[4:0]].
 *</pre>
 */
bool P2Asm::asm_TESTB_OR()
{
    next();
    m_IR.set_inst7(p2_TESTB_OR);
    return parse_D_IM_S_ORCZ();
}

/**
 * @brief Test bit S[4:0] of !D, OR  into C/Z.
 *<pre>
 * EEEE 0100101 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTBN  D,{#}S       ORC/ORZ
 *
 * C/Z = C/Z OR  !D[S[4:0]].
 *</pre>
 */
bool P2Asm::asm_TESTBN_OR()
{
    next();
    m_IR.set_inst7(p2_TESTBN_OR);
    return parse_D_IM_S_ORCZ();
}

/**
 * @brief Test bit S[4:0] of  D, XOR into C/Z.
 *<pre>
 * EEEE 0100110 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTB   D,{#}S     XORC/XORZ
 *
 * C/Z = C/Z XOR  D[S[4:0]].
 *</pre>
 */
bool P2Asm::asm_TESTB_XOR()
{
    next();
    m_IR.set_inst7(p2_TESTB_XOR);
    return parse_D_IM_S_XORCZ();
}

/**
 * @brief Test bit S[4:0] of !D, XOR into C/Z.
 *<pre>
 * EEEE 0100111 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTBN  D,{#}S     XORC/XORZ
 *
 * C/Z = C/Z XOR !D[S[4:0]].
 *</pre>
 */
bool P2Asm::asm_TESTBN_XOR()
{
    next();
    m_IR.set_inst7(p2_TESTBN_XOR);
    return parse_D_IM_S_XORCZ();
}

/**
 * @brief Test bit S[4:0] of  D[5:0], SET/AND/OR/XOR nto C/Z.
 * @return true on success, or false on error
 */
bool P2Asm::asm_TESTB()
{
    if (find_tok(t_WC) || find_tok(t_WZ))
        return asm_TESTB_W();
    if (find_tok(t_ANDC) || find_tok(t_ANDZ))
        return asm_TESTB_AND();
    if (find_tok(t_ORC) || find_tok(t_ORZ))
        return asm_TESTB_OR();
    if (find_tok(t_XORC) || find_tok(t_XORZ))
        return asm_TESTB_XOR();
    return false;
}

/**
 * @brief Test bit S[4:0] of !D[5:0], SET/AND/OR/XOR nto C/Z.
 * @return true on success, or false on error
 */
bool P2Asm::asm_TESTBN()
{
    if (find_tok(t_WC) || find_tok(t_WZ))
        return asm_TESTBN_W();
    if (find_tok(t_ANDC) || find_tok(t_ANDZ))
        return asm_TESTBN_AND();
    if (find_tok(t_ORC) || find_tok(t_ORZ))
        return asm_TESTBN_OR();
    if (find_tok(t_XORC) || find_tok(t_XORZ))
        return asm_TESTBN_XOR();
    return false;
}

/**
 * @brief Bit S[4:0] of D = 0,    C,Z = D[S[4:0]].
 *<pre>
 * EEEE 0100000 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITL    D,{#}S         {WCZ}
 *</pre>
 */
bool P2Asm::asm_BITL()
{
    next();
    m_IR.set_inst7(p2_BITL_W);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Bit S[4:0] of D = 1,    C,Z = D[S[4:0]].
 *<pre>
 * EEEE 0100001 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITH    D,{#}S         {WCZ}
 *</pre>
 */
bool P2Asm::asm_BITH()
{
    next();
    m_IR.set_inst7(p2_BITH_W);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Bit S[4:0] of D = C,    C,Z = D[S[4:0]].
 *<pre>
 * EEEE 0100010 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITC    D,{#}S         {WCZ}
 *</pre>
 */
bool P2Asm::asm_BITC()
{
    next();
    m_IR.set_inst7(p2_BITC_W);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Bit S[4:0] of D = !C,   C,Z = D[S[4:0]].
 *<pre>
 * EEEE 0100011 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNC   D,{#}S         {WCZ}
 *</pre>
 */
bool P2Asm::asm_BITNC()
{
    next();
    m_IR.set_inst7(p2_BITNC_W);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Bit S[4:0] of D = Z,    C,Z = D[S[4:0]].
 *<pre>
 * EEEE 0100100 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITZ    D,{#}S         {WCZ}
 *</pre>
 */
bool P2Asm::asm_BITZ()
{
    next();
    m_IR.set_inst7(p2_BITZ_W);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Bit S[4:0] of D = !Z,   C,Z = D[S[4:0]].
 *<pre>
 * EEEE 0100101 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNZ   D,{#}S         {WCZ}
 *</pre>
 */
bool P2Asm::asm_BITNZ()
{
    next();
    m_IR.set_inst7(p2_BITNZ_W);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Bit S[4:0] of D = RND,  C,Z = D[S[4:0]].
 *<pre>
 * EEEE 0100110 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITRND  D,{#}S         {WCZ}
 *</pre>
 */
bool P2Asm::asm_BITRND()
{
    next();
    m_IR.set_inst7(p2_BITRND_W);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Bit S[4:0] of D = !bit, C,Z = D[S[4:0]].
 *<pre>
 * EEEE 0100111 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNOT  D,{#}S         {WCZ}
 *</pre>
 */
bool P2Asm::asm_BITNOT()
{
    next();
    m_IR.set_inst7(p2_BITNOT_W);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief AND S into D.
 *<pre>
 * EEEE 0101000 CZI DDDDDDDDD SSSSSSSSS
 *
 * AND     D,{#}S   {WC/WZ/WCZ}
 *
 * D = D & S.
 * C = parity of result.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_AND()
{
    next();
    m_IR.set_inst7(p2_AND);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief AND !S into D.
 *<pre>
 * EEEE 0101001 CZI DDDDDDDDD SSSSSSSSS
 *
 * ANDN    D,{#}S   {WC/WZ/WCZ}
 *
 * D = D & !S.
 * C = parity of result.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_ANDN()
{
    next();
    m_IR.set_inst7(p2_ANDN);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief OR S into D.
 *<pre>
 * EEEE 0101010 CZI DDDDDDDDD SSSSSSSSS
 *
 * OR      D,{#}S   {WC/WZ/WCZ}
 *
 * D = D | S.
 * C = parity of result.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_OR()
{
    next();
    m_IR.set_inst7(p2_OR);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief XOR S into D.
 *<pre>
 * EEEE 0101011 CZI DDDDDDDDD SSSSSSSSS
 *
 * XOR     D,{#}S   {WC/WZ/WCZ}
 *
 * D = D ^ S.
 * C = parity of result.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_XOR()
{
    next();
    m_IR.set_inst7(p2_XOR);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Mux C into each D bit that is '1' in S.
 *<pre>
 * EEEE 0101100 CZI DDDDDDDDD SSSSSSSSS
 *
 * MUXC    D,{#}S   {WC/WZ/WCZ}
 *
 * D = (!S & D ) | (S & {32{ C}}).
 * C = parity of result.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_MUXC()
{
    next();
    m_IR.set_inst7(p2_MUXC);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Mux !C into each D bit that is '1' in S.
 *<pre>
 * EEEE 0101101 CZI DDDDDDDDD SSSSSSSSS
 *
 * MUXNC   D,{#}S   {WC/WZ/WCZ}
 *
 * D = (!S & D ) | (S & {32{!C}}).
 * C = parity of result.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_MUXNC()
{
    next();
    m_IR.set_inst7(p2_MUXNC);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Mux Z into each D bit that is '1' in S.
 *<pre>
 * EEEE 0101110 CZI DDDDDDDDD SSSSSSSSS
 *
 * MUXZ    D,{#}S   {WC/WZ/WCZ}
 *
 * D = (!S & D ) | (S & {32{ Z}}).
 * C = parity of result.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_MUXZ()
{
    next();
    m_IR.set_inst7(p2_MUXZ);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Mux !Z into each D bit that is '1' in S.
 *<pre>
 * EEEE 0101111 CZI DDDDDDDDD SSSSSSSSS
 *
 * MUXNZ   D,{#}S   {WC/WZ/WCZ}
 *
 * D = (!S & D ) | (S & {32{!Z}}).
 * C = parity of result.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_MUXNZ()
{
    next();
    m_IR.set_inst7(p2_MUXNZ);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Move S into D.
 *<pre>
 * EEEE 0110000 CZI DDDDDDDDD SSSSSSSSS
 *
 * MOV     D,{#}S   {WC/WZ/WCZ}
 *
 * D = S.
 * C = S[31].
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_MOV()
{
    next();
    m_IR.set_inst7(p2_MOV);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Get !S into D.
 *<pre>
 * EEEE 0110001 CZI DDDDDDDDD SSSSSSSSS
 *
 * NOT     D,{#}S   {WC/WZ/WCZ}
 *
 * D = !S.
 * C = !S[31].
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_NOT()
{
    next();
    m_IR.set_inst7(p2_NOT);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Get absolute value of S into D.
 *<pre>
 * EEEE 0110010 CZI DDDDDDDDD SSSSSSSSS
 *
 * ABS     D,{#}S   {WC/WZ/WCZ}
 *
 * D = ABS(S).
 * C = S[31].
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_ABS()
{
    next();
    m_IR.set_inst7(p2_ABS);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Negate S into D.
 *<pre>
 * EEEE 0110011 CZI DDDDDDDDD SSSSSSSSS
 *
 * NEG     D,{#}S   {WC/WZ/WCZ}
 *
 * D = -S.
 * C = MSB of result.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_NEG()
{
    next();
    m_IR.set_inst7(p2_NEG);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Negate S by  C into D.
 *<pre>
 * EEEE 0110100 CZI DDDDDDDDD SSSSSSSSS
 *
 * NEGC    D,{#}S   {WC/WZ/WCZ}
 *
 * If C = 1 then D = -S, else D = S.
 * C = MSB of result.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_NEGC()
{
    next();
    m_IR.set_inst7(p2_NEGC);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Negate S by !C into D.
 *<pre>
 * EEEE 0110101 CZI DDDDDDDDD SSSSSSSSS
 *
 * NEGNC   D,{#}S   {WC/WZ/WCZ}
 *
 * If C = 0 then D = -S, else D = S.
 * C = MSB of result.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_NEGNC()
{
    next();
    m_IR.set_inst7(p2_NEGNC);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Negate S by  Z into D.
 *<pre>
 * EEEE 0110110 CZI DDDDDDDDD SSSSSSSSS
 *
 * NEGZ    D,{#}S   {WC/WZ/WCZ}
 *
 * If Z = 1 then D = -S, else D = S.
 * C = MSB of result.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_NEGZ()
{
    next();
    m_IR.set_inst7(p2_NEGZ);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Negate S by !Z into D.
 *<pre>
 * EEEE 0110111 CZI DDDDDDDDD SSSSSSSSS
 *
 * NEGNZ   D,{#}S   {WC/WZ/WCZ}
 *
 * If Z = 0 then D = -S, else D = S.
 * C = MSB of result.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_NEGNZ()
{
    next();
    m_IR.set_inst7(p2_NEGNZ);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Increment with modulus.
 *<pre>
 * EEEE 0111000 CZI DDDDDDDDD SSSSSSSSS
 *
 * INCMOD  D,{#}S   {WC/WZ/WCZ}
 *
 * If D = S then D = 0 and C = 1, else D = D + 1 and C = 0.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_INCMOD()
{
    next();
    m_IR.set_inst7(p2_INCMOD);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Decrement with modulus.
 *<pre>
 * EEEE 0111001 CZI DDDDDDDDD SSSSSSSSS
 *
 * DECMOD  D,{#}S   {WC/WZ/WCZ}
 *
 * If D = 0 then D = S and C = 1, else D = D - 1 and C = 0.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_DECMOD()
{
    next();
    m_IR.set_inst7(p2_DECMOD);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Zero-extend D above bit S[4:0].
 *<pre>
 * EEEE 0111010 CZI DDDDDDDDD SSSSSSSSS
 *
 * ZEROX   D,{#}S   {WC/WZ/WCZ}
 *
 * C = MSB of result.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_ZEROX()
{
    next();
    m_IR.set_inst7(p2_ZEROX);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Sign-extend D from bit S[4:0].
 *<pre>
 * EEEE 0111011 CZI DDDDDDDDD SSSSSSSSS
 *
 * SIGNX   D,{#}S   {WC/WZ/WCZ}
 *
 * C = MSB of result.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_SIGNX()
{
    next();
    m_IR.set_inst7(p2_SIGNX);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Get bit position of top-most '1' in S into D.
 *<pre>
 * EEEE 0111100 CZI DDDDDDDDD SSSSSSSSS
 *
 * ENCOD   D,{#}S   {WC/WZ/WCZ}
 *
 * D = position of top '1' in S (0..31).
 * C = (S != 0).
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_ENCOD()
{
    next();
    m_IR.set_inst7(p2_ENCOD);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Get number of '1's in S into D.
 *<pre>
 * EEEE 0111101 CZI DDDDDDDDD SSSSSSSSS
 *
 * ONES    D,{#}S   {WC/WZ/WCZ}
 *
 * D = number of '1's in S (0..32).
 * C = LSB of result.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_ONES()
{
    next();
    m_IR.set_inst7(p2_ONES);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Test D with S.
 *<pre>
 * EEEE 0111110 CZI DDDDDDDDD SSSSSSSSS
 *
 * TEST    D,{#}S   {WC/WZ/WCZ}
 *
 * C = parity of (D & S).
 * Z = ((D & S) == 0).
 *</pre>
 */
bool P2Asm::asm_TEST()
{
    next();
    m_IR.set_inst7(p2_TEST);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Test D with !S.
 *<pre>
 * EEEE 0111111 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTN   D,{#}S   {WC/WZ/WCZ}
 *
 * C = parity of (D & !S).
 * Z = ((D & !S) == 0).
 *</pre>
 */
bool P2Asm::asm_TESTN()
{
    next();
    m_IR.set_inst7(p2_TESTN);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Set S[3:0] into nibble N in D, keeping rest of D same.
 *<pre>
 * EEEE 100000N NNI DDDDDDDDD SSSSSSSSS
 *
 * SETNIB  D,{#}S,#N
 *</pre>
 */
bool P2Asm::asm_SETNIB()
{
    if (commata_left() < 1)
        return asm_SETNIB_ALTSN();
    next();
    m_IR.set_inst7(p2_SETNIB_0);
    return parse_D_IM_S_NNN();
}

/**
 * @brief Set S[3:0] into nibble established by prior ALTSN instruction.
 *<pre>
 * EEEE 1000000 00I 000000000 SSSSSSSSS
 *
 * SETNIB  {#}S
 *</pre>
 */
bool P2Asm::asm_SETNIB_ALTSN()
{
    next();
    m_IR.set_inst7(p2_SETNIB_0);
    return parse_IM_S();
}

/**
 * @brief Get nibble N of S into D.
 *<pre>
 * EEEE 100001N NNI DDDDDDDDD SSSSSSSSS
 *
 * GETNIB  D,{#}S,#N
 *
 * D = {28'b0, S.NIBBLE[N]).
 *</pre>
 */
bool P2Asm::asm_GETNIB()
{
    if (commata_left() < 1)
        return asm_GETNIB_ALTGN();
    next();
    m_IR.set_inst7(p2_GETNIB_0);
    return parse_D_IM_S_NNN();
}

/**
 * @brief Get nibble established by prior ALTGN instruction into D.
 *<pre>
 * EEEE 1000010 000 DDDDDDDDD 000000000
 *
 * GETNIB  D
 *</pre>
 */
bool P2Asm::asm_GETNIB_ALTGN()
{
    m_IR.set_inst7(p2_GETNIB_0);
    return parse_IM_S();
}

/**
 * @brief Rotate-left nibble N of S into D.
 *<pre>
 * EEEE 100010N NNI DDDDDDDDD SSSSSSSSS
 *
 * ROLNIB  D,{#}S,#N
 *
 * D = {D[27:0], S.NIBBLE[N]).
 *</pre>
 */
bool P2Asm::asm_ROLNIB()
{
    if (commata_left() < 1)
        return asm_ROLNIB_ALTGN();
    next();
    m_IR.set_inst7(p2_ROLNIB_0);
    return parse_D_IM_S_NNN();
}

/**
 * @brief Rotate-left nibble established by prior ALTGN instruction into D.
 *<pre>
 * EEEE 1000100 000 DDDDDDDDD 000000000
 *
 * ROLNIB  D
 *</pre>
 */
bool P2Asm::asm_ROLNIB_ALTGN()
{
    next();
    m_IR.set_inst7(p2_ROLNIB_0);
    return parse_D();
}

/**
 * @brief Set S[7:0] into byte N in D, keeping rest of D same.
 *<pre>
 * EEEE 1000110 NNI DDDDDDDDD SSSSSSSSS
 *
 * SETBYTE D,{#}S,#N
 *</pre>
 */
bool P2Asm::asm_SETBYTE()
{
    if (commata_left() < 1)
        return asm_SETBYTE_ALTSB();
    next();
    m_IR.set_inst7(p2_SETBYTE);
    return parse_D_IM_S_NNN();
}

/**
 * @brief Set S[7:0] into byte established by prior ALTSB instruction.
 *<pre>
 * EEEE 1000110 00I 000000000 SSSSSSSSS
 *
 * SETBYTE {#}S
 *</pre>
 */
bool P2Asm::asm_SETBYTE_ALTSB()
{
    next();
    m_IR.set_inst7(p2_SETBYTE);
    return parse_IM_S();
}

/**
 * @brief Get byte N of S into D.
 *<pre>
 * EEEE 1000111 NNI DDDDDDDDD SSSSSSSSS
 *
 * GETBYTE D,{#}S,#N
 *
 * D = {24'b0, S.BYTE[N]).
 *</pre>
 */
bool P2Asm::asm_GETBYTE()
{
    if (commata_left() < 1)
        return asm_GETBYTE_ALTGB();
    next();
    m_IR.set_inst7(p2_GETBYTE);
    return parse_D_IM_S_NNN();
}

/**
 * @brief Get byte established by prior ALTGB instruction into D.
 *<pre>
 * EEEE 1000111 000 DDDDDDDDD 000000000
 *
 * GETBYTE D
 *</pre>
 */
bool P2Asm::asm_GETBYTE_ALTGB()
{
    next();
    m_IR.set_inst7(p2_GETBYTE);
    return parse_D();
}

/**
 * @brief Rotate-left byte N of S into D.
 *<pre>
 * EEEE 1001000 NNI DDDDDDDDD SSSSSSSSS
 *
 * ROLBYTE D,{#}S,#N
 *
 * D = {D[23:0], S.BYTE[N]).
 *</pre>
 */
bool P2Asm::asm_ROLBYTE()
{
    if (commata_left() < 1)
        return asm_ROLBYTE_ALTGB();
    next();
    m_IR.set_inst7(p2_ROLBYTE);
    return parse_D_IM_S_NNN(3);
}

/**
 * @brief Rotate-left byte established by prior ALTGB instruction into D.
 *<pre>
 * EEEE 1001000 000 DDDDDDDDD 000000000
 *
 * ROLBYTE D
 *</pre>
 */
bool P2Asm::asm_ROLBYTE_ALTGB()
{
    next();
    m_IR.set_inst7(p2_ROLBYTE);
    return parse_D();
}

/**
 * @brief Set S[15:0] into word N in D, keeping rest of D same.
 *<pre>
 * EEEE 1001001 0NI DDDDDDDDD SSSSSSSSS
 *
 * SETWORD D,{#}S,#N
 *</pre>
 */
bool P2Asm::asm_SETWORD()
{
    if (commata_left() < 1)
        return asm_SETWORD_ALTSW();
    next();
    m_IR.set_inst9(p2_SETWORD);
    return parse_D_IM_S_NNN(1);
}

/**
 * @brief Set S[15:0] into word established by prior ALTSW instruction.
 *<pre>
 * EEEE 1001001 00I 000000000 SSSSSSSSS
 *
 * SETWORD {#}S
 *</pre>
 */
bool P2Asm::asm_SETWORD_ALTSW()
{
    next();
    m_IR.set_inst9(p2_SETWORD_ALTSW);
    return parse_IM_S();
}

/**
 * @brief Get word N of S into D.
 *<pre>
 * EEEE 1001001 1NI DDDDDDDDD SSSSSSSSS
 *
 * GETWORD D,{#}S,#N
 *
 * D = {16'b0, S.WORD[N]).
 *</pre>
 */
bool P2Asm::asm_GETWORD()
{
    if (commata_left() < 1)
        return asm_GETWORD_ALTGW();
    next();
    m_IR.set_inst9(p2_GETWORD);
    return parse_D_IM_S_NNN(1);
}

/**
 * @brief Get word established by prior ALTGW instruction into D.
 *<pre>
 * EEEE 1001001 100 DDDDDDDDD 000000000
 *
 * GETWORD D
 *</pre>
 */
bool P2Asm::asm_GETWORD_ALTGW()
{
    next();
    m_IR.set_inst9(p2_GETWORD_ALTGW);
    return parse_D();
}

/**
 * @brief Rotate-left word N of S into D.
 *<pre>
 * EEEE 1001010 0NI DDDDDDDDD SSSSSSSSS
 *
 * ROLWORD D,{#}S,#N
 *
 * D = {D[15:0], S.WORD[N]).
 *</pre>
 */
bool P2Asm::asm_ROLWORD()
{
    if (commata_left() < 1)
        return asm_ROLWORD_ALTGW();
    next();
    m_IR.set_inst9(p2_ROLWORD);
    return parse_D_IM_S_NNN(1);
}

/**
 * @brief Rotate-left word established by prior ALTGW instruction into D.
 *<pre>
 * EEEE 1001010 000 DDDDDDDDD 000000000
 *
 * ROLWORD D
 *</pre>
 */
bool P2Asm::asm_ROLWORD_ALTGW()
{
    next();
    m_IR.set_inst9(p2_ROLWORD_ALTGW);
    return parse_D();
}

/**
 * @brief Alter subsequent SETNIB instruction.
 *<pre>
 * EEEE 1001010 10I DDDDDDDDD SSSSSSSSS
 *
 * ALTSN   D,{#}S
 *
 * Next D field = (D[11:3] + S) & $1FF, N field = D[2:0].
 * D += sign-extended S[17:9].
 *</pre>
 */
bool P2Asm::asm_ALTSN()
{
    if (commata_left() < 1)
        return asm_ALTSN_D();
    next();
    m_IR.set_inst9(p2_ALTSN);
    return parse_D_IM_S();
}

/**
 * @brief Alter subsequent SETNIB instruction.
 *<pre>
 * EEEE 1001010 101 DDDDDDDDD 000000000
 *
 * ALTSN   D
 *
 * Next D field = D[11:3], N field = D[2:0].
 *</pre>
 */
bool P2Asm::asm_ALTSN_D()
{
    next();
    m_IR.set_inst9(p2_ALTSN);
    return parse_D();
}

/**
 * @brief Alter subsequent GETNIB/ROLNIB instruction.
 *<pre>
 * EEEE 1001010 11I DDDDDDDDD SSSSSSSSS
 *
 * ALTGN   D,{#}S
 *
 * Next S field = (D[11:3] + S) & $1FF, N field = D[2:0].
 * D += sign-extended S[17:9].
 *</pre>
 */
bool P2Asm::asm_ALTGN()
{
    if (commata_left() < 1)
        return asm_ALTGN_D();
    next();
    m_IR.set_inst9(p2_ALTGN);
    return parse_D_IM_S();
}

/**
 * @brief Alter subsequent GETNIB/ROLNIB instruction.
 *<pre>
 * EEEE 1001010 111 DDDDDDDDD 000000000
 *
 * ALTGN   D
 *
 * Next S field = D[11:3], N field = D[2:0].
 *</pre>
 */
bool P2Asm::asm_ALTGN_D()
{
    next();
    m_IR.set_inst9(p2_ALTGN);
    return parse_D();
}

/**
 * @brief Alter subsequent SETBYTE instruction.
 *<pre>
 * EEEE 1001011 00I DDDDDDDDD SSSSSSSSS
 *
 * ALTSB   D,{#}S
 *
 * Next D field = (D[10:2] + S) & $1FF, N field = D[1:0].
 * D += sign-extended S[17:9].
 *</pre>
 */
bool P2Asm::asm_ALTSB()
{
    if (commata_left() < 1)
        return asm_ALTSB_D();
    next();
    m_IR.set_inst9(p2_ALTSB);
    return parse_D_IM_S();
}

/**
 * @brief Alter subsequent SETBYTE instruction.
 *<pre>
 * EEEE 1001011 001 DDDDDDDDD 000000000
 *
 * ALTSB   D
 *
 * Next D field = D[10:2], N field = D[1:0].
 *</pre>
 */
bool P2Asm::asm_ALTSB_D()
{
    next();
    m_IR.set_inst9(p2_ALTSB);
    return parse_D();
}

/**
 * @brief Alter subsequent GETBYTE/ROLBYTE instruction.
 *<pre>
 * EEEE 1001011 01I DDDDDDDDD SSSSSSSSS
 *
 * ALTGB   D,{#}S
 *
 * Next S field = (D[10:2] + S) & $1FF, N field = D[1:0].
 * D += sign-extended S[17:9].
 *</pre>
 */
bool P2Asm::asm_ALTGB()
{
    if (commata_left() < 1)
        return asm_ALTGB_D();
    next();
    m_IR.set_inst9(p2_ALTGB);
    return parse_D_IM_S();
}

/**
 * @brief Alter subsequent GETBYTE/ROLBYTE instruction.
 *<pre>
 * EEEE 1001011 011 DDDDDDDDD 000000000
 *
 * ALTGB   D
 *
 * Next S field = D[10:2], N field = D[1:0].
 *</pre>
 */
bool P2Asm::asm_ALTGB_D()
{
    next();
    m_IR.set_inst9(p2_ALTGB);
    return parse_D();
}

/**
 * @brief Alter subsequent SETWORD instruction.
 *<pre>
 * EEEE 1001011 10I DDDDDDDDD SSSSSSSSS
 *
 * ALTSW   D,{#}S
 *
 * Next D field = (D[9:1] + S) & $1FF, N field = D[0].
 * D += sign-extended S[17:9].
 *</pre>
 */
bool P2Asm::asm_ALTSW()
{
    if (commata_left() < 1)
        return asm_ALTSW_D();
    next();
    m_IR.set_inst9(p2_ALTSW);
    return parse_D_IM_S();
}

/**
 * @brief Alter subsequent SETWORD instruction.
 *<pre>
 * EEEE 1001011 101 DDDDDDDDD 000000000
 *
 * ALTSW   D
 *
 * Next D field = D[9:1], N field = D[0].
 *</pre>
 */
bool P2Asm::asm_ALTSW_D()
{
    next();
    m_IR.set_inst9(p2_ALTSW);
    return parse_D();
}

/**
 * @brief Alter subsequent GETWORD/ROLWORD instruction.
 *<pre>
 * EEEE 1001011 11I DDDDDDDDD SSSSSSSSS
 *
 * ALTGW   D,{#}S
 *
 * Next S field = ((D[9:1] + S) & $1FF), N field = D[0].
 * D += sign-extended S[17:9].
 *</pre>
 */
bool P2Asm::asm_ALTGW()
{
    if (commata_left() < 1)
        return asm_ALTGW_D();
    next();
    m_IR.set_inst9(p2_ALTGW);
    return parse_D_IM_S();
}

/**
 * @brief Alter subsequent GETWORD/ROLWORD instruction.
 *<pre>
 * EEEE 1001011 111 DDDDDDDDD 000000000
 *
 * ALTGW   D
 *
 * Next S field = D[9:1], N field = D[0].
 *</pre>
 */
bool P2Asm::asm_ALTGW_D()
{
    next();
    m_IR.set_inst9(p2_ALTGW);
    return parse_D();
}

/**
 * @brief Alter result register address (normally D field) of next instruction to (D + S) & $1FF.
 *<pre>
 * EEEE 1001100 00I DDDDDDDDD SSSSSSSSS
 *
 * ALTR    D,{#}S
 *
 * D += sign-extended S[17:9].
 *</pre>
 */
bool P2Asm::asm_ALTR()
{
    if (commata_left() < 1)
        return asm_ALTR_D();
    next();
    m_IR.set_inst9(p2_ALTR);
    return parse_D_IM_S();
}

/**
 * @brief Alter result register address (normally D field) of next instruction to D[8:0].
 *<pre>
 * EEEE 1001100 001 DDDDDDDDD 000000000
 *
 * ALTR    D
 *</pre>
 */
bool P2Asm::asm_ALTR_D()
{
    next();
    m_IR.set_inst9(p2_ALTD);
    return parse_D();
}

/**
 * @brief Alter D field of next instruction to (D + S) & $1FF.
 *<pre>
 * EEEE 1001100 01I DDDDDDDDD SSSSSSSSS
 *
 * ALTD    D,{#}S
 *
 * D += sign-extended S[17:9].
 *</pre>
 */
bool P2Asm::asm_ALTD()
{
    if (commata_left() < 1)
        return asm_ALTD_D();
    next();
    m_IR.set_inst9(p2_ALTD);
    return parse_D_IM_S();
}

/**
 * @brief Alter D field of next instruction to D[8:0].
 *<pre>
 * EEEE 1001100 011 DDDDDDDDD 000000000
 *
 * ALTD    D
 *</pre>
 */
bool P2Asm::asm_ALTD_D()
{
    next();
    m_IR.set_inst9(p2_ALTD);
    return parse_D();
}

/**
 * @brief Alter S field of next instruction to (D + S) & $1FF.
 *<pre>
 * EEEE 1001100 10I DDDDDDDDD SSSSSSSSS
 *
 * ALTS    D,{#}S
 *
 * D += sign-extended S[17:9].
 *</pre>
 */
bool P2Asm::asm_ALTS()
{
    if (commata_left() < 1)
        return asm_ALTS_D();
    next();
    m_IR.set_inst9(p2_ALTS);
    return parse_D_IM_S();
}

/**
 * @brief Alter S field of next instruction to D[8:0].
 *<pre>
 * EEEE 1001100 101 DDDDDDDDD 000000000
 *
 * ALTS    D
 *</pre>
 */
bool P2Asm::asm_ALTS_D()
{
    next();
    m_IR.set_inst9(p2_ALTS);
    return parse_D();
}

/**
 * @brief Alter D field of next instruction to (D[13:5] + S) & $1FF.
 *<pre>
 * EEEE 1001100 11I DDDDDDDDD SSSSSSSSS
 *
 * ALTB    D,{#}S
 *
 * D += sign-extended S[17:9].
 *</pre>
 */
bool P2Asm::asm_ALTB()
{
    if (commata_left() < 1)
        return asm_ALTB_D();
    next();
    m_IR.set_inst9(p2_ALTB);
    return parse_D_IM_S();
}

/**
 * @brief Alter D field of next instruction to D[13:5].
 *<pre>
 * EEEE 1001100 111 DDDDDDDDD 000000000
 *
 * ALTB    D
 *</pre>
 */
bool P2Asm::asm_ALTB_D()
{
    next();
    m_IR.set_inst9(p2_ALTB);
    return parse_D();
}

/**
 * @brief Substitute next instruction's I/R/D/S fields with fields from D, per S.
 *<pre>
 * EEEE 1001101 00I DDDDDDDDD SSSSSSSSS
 *
 * ALTI    D,{#}S
 *
 * Modify D per S.
 *</pre>
 */
bool P2Asm::asm_ALTI()
{
    if (commata_left() < 1)
        return asm_ALTI_D();
    next();
    m_IR.set_inst9(p2_ALTI);
    return parse_D_IM_S();
}

/**
 * @brief Execute D in place of next instruction.
 *<pre>
 * EEEE 1001101 001 DDDDDDDDD 101100100
 *
 * ALTI    D
 *
 * D stays same.
 *</pre>
 */
bool P2Asm::asm_ALTI_D()
{
    next();
    m_IR.set_inst9(p2_ALTI);
    return parse_D();
}

/**
 * @brief Set R field of D to S[8:0].
 *<pre>
 * EEEE 1001101 01I DDDDDDDDD SSSSSSSSS
 *
 * SETR    D,{#}S
 *
 * D = {D[31:28], S[8:0], D[18:0]}.
 *</pre>
 */
bool P2Asm::asm_SETR()
{
    next();
    m_IR.set_inst9(p2_SETR);
    return parse_D_IM_S();
}

/**
 * @brief Set D field of D to S[8:0].
 *<pre>
 * EEEE 1001101 10I DDDDDDDDD SSSSSSSSS
 *
 * SETD    D,{#}S
 *
 * D = {D[31:18], S[8:0], D[8:0]}.
 *</pre>
 */
bool P2Asm::asm_SETD()
{
    next();
    m_IR.set_inst9(p2_SETD);
    return parse_D_IM_S();
}

/**
 * @brief Set S field of D to S[8:0].
 *<pre>
 * EEEE 1001101 11I DDDDDDDDD SSSSSSSSS
 *
 * SETS    D,{#}S
 *
 * D = {D[31:9], S[8:0]}.
 *</pre>
 */
bool P2Asm::asm_SETS()
{
    next();
    m_IR.set_inst9(p2_SETS);
    return parse_D_IM_S();
}

/**
 * @brief Decode S[4:0] into D.
 *<pre>
 * EEEE 1001110 00I DDDDDDDDD SSSSSSSSS
 *
 * DECOD   D,{#}S
 *
 * D = 1 << S[4:0].
 *</pre>
 */
bool P2Asm::asm_DECOD()
{
    next();
    m_IR.set_inst9(p2_DECOD);
    return parse_D_IM_S();
}

/**
 * @brief Get LSB-justified bit mask of size (S[4:0] + 1) into D.
 *<pre>
 * EEEE 1001110 01I DDDDDDDDD SSSSSSSSS
 *
 * BMASK   D,{#}S
 *
 * D = ($0000_0002 << S[4:0]) - 1.
 *</pre>
 */
bool P2Asm::asm_BMASK()
{
    next();
    m_IR.set_inst9(p2_BMASK);
    return parse_D_IM_S();
}

/**
 * @brief Iterate CRC value in D using C and polynomial in S.
 *<pre>
 * EEEE 1001110 10I DDDDDDDDD SSSSSSSSS
 *
 * CRCBIT  D,{#}S
 *
 * If (C XOR D[0]) then D = (D >> 1) XOR S, else D = (D >> 1).
 *</pre>
 */
bool P2Asm::asm_CRCBIT()
{
    next();
    m_IR.set_inst9(p2_CRCBIT);
    return parse_D_IM_S();
}

/**
 * @brief Iterate CRC value in D using Q[31:28] and polynomial in S.
 *<pre>
 * EEEE 1001110 11I DDDDDDDDD SSSSSSSSS
 *
 * CRCNIB  D,{#}S
 *
 * Like CRCBIT, but 4x.
 * Q = Q << 4.
 * Use SETQ+CRCNIB+CRCNIB+CRCNIB.
 *</pre>
 */
bool P2Asm::asm_CRCNIB()
{
    next();
    m_IR.set_inst9(p2_CRCNIB);
    return parse_D_IM_S();
}

/**
 * @brief For each non-zero bit pair in S, copy that bit pair into the corresponding D bits, else leave that D bit pair the same.
 *<pre>
 * EEEE 1001111 00I DDDDDDDDD SSSSSSSSS
 *
 * MUXNITS D,{#}S
 *</pre>
 */
bool P2Asm::asm_MUXNITS()
{
    next();
    m_IR.set_inst9(p2_MUXNITS);
    return parse_D_IM_S();
}

/**
 * @brief For each non-zero nibble in S, copy that nibble into the corresponding D nibble, else leave that D nibble the same.
 *<pre>
 * EEEE 1001111 01I DDDDDDDDD SSSSSSSSS
 *
 * MUXNIBS D,{#}S
 *</pre>
 */
bool P2Asm::asm_MUXNIBS()
{
    next();
    m_IR.set_inst9(p2_MUXNIBS);
    return parse_D_IM_S();
}

/**
 * @brief Used after SETQ.
 *<pre>
 * EEEE 1001111 10I DDDDDDDDD SSSSSSSSS
 *
 * MUXQ    D,{#}S
 *
 * For each '1' bit in Q, copy the corresponding bit in S into D.
 * D = (D & !Q) | (S & Q).
 *</pre>
 */
bool P2Asm::asm_MUXQ()
{
    next();
    m_IR.set_inst9(p2_MUXQ);
    return parse_D_IM_S();
}

/**
 * @brief Move bytes within D, per S.
 *<pre>
 * EEEE 1001111 11I DDDDDDDDD SSSSSSSSS
 *
 * MOVBYTS D,{#}S
 *
 * D = {D.BYTE[S[7:6]], D.BYTE[S[5:4]], D.BYTE[S[3:2]], D.BYTE[S[1:0]]}.
 *</pre>
 */
bool P2Asm::asm_MOVBYTS()
{
    next();
    m_IR.set_inst9(p2_MOVBYTS);
    return parse_D_IM_S();
}

/**
 * @brief D = unsigned (D[15:0] * S[15:0]).
 *<pre>
 * EEEE 1010000 0ZI DDDDDDDDD SSSSSSSSS
 *
 * MUL     D,{#}S          {WZ}
 *
 * Z = (S == 0) | (D == 0).
 *</pre>
 */
bool P2Asm::asm_MUL()
{
    next();
    m_IR.set_inst8(p2_MUL);
    return parse_D_IM_S_WZ();
}

/**
 * @brief D = signed (D[15:0] * S[15:0]).
 *<pre>
 * EEEE 1010000 1ZI DDDDDDDDD SSSSSSSSS
 *
 * MULS    D,{#}S          {WZ}
 *
 * Z = (S == 0) | (D == 0).
 *</pre>
 */
bool P2Asm::asm_MULS()
{
    next();
    m_IR.set_inst8(p2_MULS);
    return parse_D_IM_S_WZ();
}

/**
 * @brief Next instruction's S value = unsigned (D[15:0] * S[15:0]) >> 16.
 *<pre>
 * EEEE 1010001 0ZI DDDDDDDDD SSSSSSSSS
 *
 * SCA     D,{#}S          {WZ}
 *
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_SCA()
{
    next();
    m_IR.set_inst8(p2_SCA);
    return parse_D_IM_S_WZ();
}

/**
 * @brief Next instruction's S value = signed (D[15:0] * S[15:0]) >> 14.
 *<pre>
 * EEEE 1010001 1ZI DDDDDDDDD SSSSSSSSS
 *
 * SCAS    D,{#}S          {WZ}
 *
 * In this scheme, $4000 = 1.0 and $C000 = -1.0.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_SCAS()
{
    next();
    m_IR.set_inst8(p2_SCAS);
    return parse_D_IM_S_WZ();
}

/**
 * @brief Add bytes of S into bytes of D, with $FF saturation.
 *<pre>
 * EEEE 1010010 00I DDDDDDDDD SSSSSSSSS
 *
 * ADDPIX  D,{#}S
 *</pre>
 */
bool P2Asm::asm_ADDPIX()
{
    next();
    m_IR.set_inst9(p2_ADDPIX);
    return parse_D_IM_S();
}

/**
 * @brief Multiply bytes of S into bytes of D, where $FF = 1.
 *<pre>
 * EEEE 1010010 01I DDDDDDDDD SSSSSSSSS
 *
 * MULPIX  D,{#}S
 *
 * 0.
 *</pre>
 */
bool P2Asm::asm_MULPIX()
{
    next();
    m_IR.set_inst9(p2_MULPIX);
    return parse_D_IM_S();
}

/**
 * @brief Alpha-blend bytes of S into bytes of D, using SETPIV value.
 *<pre>
 * EEEE 1010010 10I DDDDDDDDD SSSSSSSSS
 *
 * BLNPIX  D,{#}S
 *</pre>
 */
bool P2Asm::asm_BLNPIX()
{
    next();
    m_IR.set_inst9(p2_BLNPIX);
    return parse_D_IM_S();
}

/**
 * @brief Mix bytes of S into bytes of D, using SETPIX and SETPIV values.
 *<pre>
 * EEEE 1010010 11I DDDDDDDDD SSSSSSSSS
 *
 * MIXPIX  D,{#}S
 *</pre>
 */
bool P2Asm::asm_MIXPIX()
{
    next();
    m_IR.set_inst9(p2_MIXPIX);
    return parse_D_IM_S();
}

/**
 * @brief Set CT1 event to trigger on CT = D + S.
 *<pre>
 * EEEE 1010011 00I DDDDDDDDD SSSSSSSSS
 *
 * ADDCT1  D,{#}S
 *
 * Adds S into D.
 *</pre>
 */
bool P2Asm::asm_ADDCT1()
{
    next();
    m_IR.set_inst9(p2_ADDCT1);
    return parse_D_IM_S();
}

/**
 * @brief Set CT2 event to trigger on CT = D + S.
 *<pre>
 * EEEE 1010011 01I DDDDDDDDD SSSSSSSSS
 *
 * ADDCT2  D,{#}S
 *
 * Adds S into D.
 *</pre>
 */
bool P2Asm::asm_ADDCT2()
{
    next();
    m_IR.set_inst9(p2_ADDCT2);
    return parse_D_IM_S();
}

/**
 * @brief Set CT3 event to trigger on CT = D + S.
 *<pre>
 * EEEE 1010011 10I DDDDDDDDD SSSSSSSSS
 *
 * ADDCT3  D,{#}S
 *
 * Adds S into D.
 *</pre>
 */
bool P2Asm::asm_ADDCT3()
{
    next();
    m_IR.set_inst9(p2_ADDCT3);
    return parse_D_IM_S();
}

/**
 * @brief Write only non-$00 bytes in D[31:0] to hub address {#}S/PTRx.
 *<pre>
 * EEEE 1010011 11I DDDDDDDDD SSSSSSSSS
 *
 * WMLONG  D,{#}S/P
 *
 * Prior SETQ/SETQ2 invokes cog/LUT block transfer.
 *</pre>
 */
bool P2Asm::asm_WMLONG()
{
    next();
    m_IR.set_inst9(p2_WMLONG);
    return parse_D_IM_S_PTRx(sz_LONG);
}

/**
 * @brief Read smart pin S[5:0] result "Z" into D, don't acknowledge smart pin ("Q" in RQPIN means "quiet").
 *<pre>
 * EEEE 1010100 C0I DDDDDDDDD SSSSSSSSS
 *
 * RQPIN   D,{#}S          {WC}
 *
 * C = modal result.
 *</pre>
 */
bool P2Asm::asm_RQPIN()
{
    next();
    m_IR.set_inst8(p2_RQPIN);
    return parse_D_IM_S_WC();
}

/**
 * @brief Read smart pin S[5:0] result "Z" into D, acknowledge smart pin.
 *<pre>
 * EEEE 1010100 C1I DDDDDDDDD SSSSSSSSS
 *
 * RDPIN   D,{#}S          {WC}
 *
 * C = modal result.
 *</pre>
 */
bool P2Asm::asm_RDPIN()
{
    next();
    m_IR.set_inst8(p2_RDPIN);
    return parse_D_IM_S_WC();
}

/**
 * @brief Read LUT data from address S[8:0] into D.
 *<pre>
 * EEEE 1010101 CZI DDDDDDDDD SSSSSSSSS
 *
 * RDLUT   D,{#}S   {WC/WZ/WCZ}
 *
 * C = MSB of data.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_RDLUT()
{
    next();
    m_IR.set_inst7(p2_RDLUT);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Read zero-extended byte from hub address {#}S/PTRx into D.
 *<pre>
 * EEEE 1010110 CZI DDDDDDDDD SSSSSSSSS
 *
 * RDBYTE  D,{#}S/P {WC/WZ/WCZ}
 *
 * C = MSB of byte.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_RDBYTE()
{
    next();
    m_IR.set_inst7(p2_RDBYTE);
    return parse_D_IM_S_PTRx_WCZ(sz_BYTE);
}

/**
 * @brief Read zero-extended word from hub address {#}S/PTRx into D.
 *<pre>
 * EEEE 1010111 CZI DDDDDDDDD SSSSSSSSS
 *
 * RDWORD  D,{#}S/P {WC/WZ/WCZ}
 *
 * C = MSB of word.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_RDWORD()
{
    next();
    m_IR.set_inst7(p2_RDWORD);
    return parse_D_IM_S_PTRx_WCZ(sz_WORD);
}

/**
 * @brief Read long from hub address {#}S/PTRx into D.
 * <pre>
 *
 * EEEE 1011000 CZI DDDDDDDDD SSSSSSSSS
 *
 * RDLONG  D,{#}S/P {WC/WZ/WCZ}
 *
 * C = MSB of long.
 * Prior SETQ/SETQ2 invokes cog/LUT block transfer.
 *</pre>
 */
bool P2Asm::asm_RDLONG()
{
    next();
    m_IR.set_inst7(p2_RDLONG);
    return parse_D_IM_S_PTRx_WCZ(sz_LONG);
}

/**
 * @brief Read long from hub address --PTRA into D.
 *<pre>
 * EEEE 1011000 CZ1 DDDDDDDDD 101011111
 *
 * POPA    D        {WC/WZ/WCZ}
 *
 * C = MSB of long.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_POPA()
{
    next();
    m_IR.set_inst7(p2_RDLONG);
    m_IR.set_im(true);
    m_IR.set_src(0x15f);
    return parse_D_WCZ();
}

/**
 * @brief Read long from hub address --PTRB into D.
 *<pre>
 * EEEE 1011000 CZ1 DDDDDDDDD 111011111
 *
 * POPB    D        {WC/WZ/WCZ}
 *
 * C = MSB of long.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_POPB()
{
    next();
    m_IR.set_inst7(p2_RDLONG);
    m_IR.set_im(true);
    m_IR.set_src(0x1df);
    return parse_D_WCZ();
}

/**
 * @brief Call to S** by writing {C, Z, 10'b0, PC[19:0]} to D.
 *<pre>
 * EEEE 1011001 CZI DDDDDDDDD SSSSSSSSS
 *
 * CALLD   D,{#}S   {WC/WZ/WCZ}
 *
 * C = S[31], Z = S[30].
 *</pre>
 */
bool P2Asm::asm_CALLD()
{
    next();
    m_IR.set_inst7(p2_CALLD);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Resume from INT3.
 *<pre>
 * EEEE 1011001 110 111110000 111110001
 *
 * RESI3
 *
 * (CALLD $1F0,$1F1 WC,WZ).
 *</pre>
 */
bool P2Asm::asm_RESI3()
{
    next();
    m_IR.set_inst7(p2_CALLD);
    m_IR.set_wc();
    m_IR.set_wz();
    m_IR.set_dst_src(offs_IJMP3, offs_IRET3);
    return parse_INST();
}

/**
 * @brief Resume from INT2.
 *<pre>
 * EEEE 1011001 110 111110010 111110011
 *
 * RESI2
 *
 * (CALLD $1F2,$1F3 WC,WZ).
 *</pre>
 */
bool P2Asm::asm_RESI2()
{
    next();
    m_IR.set_inst7(p2_CALLD);
    m_IR.set_wcz(true);
    m_IR.set_dst_src(offs_IJMP2, offs_IRET2);
    return parse_INST();
}

/**
 * @brief Resume from INT1.
 *<pre>
 * EEEE 1011001 110 111110100 111110101
 *
 * RESI1
 *
 * (CALLD $1F4,$1F5 WC,WZ).
 *</pre>
 */
bool P2Asm::asm_RESI1()
{
    next();
    m_IR.set_inst7(p2_CALLD);
    m_IR.set_wcz(true);
    m_IR.set_dst_src(offs_IJMP1, offs_IRET1);
    return parse_INST();
}

/**
 * @brief Resume from INT0.
 *<pre>
 * EEEE 1011001 110 111111110 111111111
 *
 * RESI0
 *
 * (CALLD $1FE,$1FF WC,WZ).
 *</pre>
 */
bool P2Asm::asm_RESI0()
{
    next();
    m_IR.set_inst7(p2_CALLD);
    m_IR.set_wcz(true);
    m_IR.set_dst_src(offs_INA, offs_INB);
    return parse_INST();
}

/**
 * @brief Return from INT3.
 *<pre>
 * EEEE 1011001 110 111111111 111110001
 *
 * RETI3
 *
 * (CALLD $1FF,$1F1 WC,WZ).
 *</pre>
 */
bool P2Asm::asm_RETI3()
{
    next();
    m_IR.set_inst7(p2_CALLD);
    m_IR.set_wcz(true);
    m_IR.set_dst_src(offs_INB, offs_IRET3);
    return parse_INST();
}

/**
 * @brief Return from INT2.
 *<pre>
 * EEEE 1011001 110 111111111 111110011
 *
 * RETI2
 *
 * (CALLD $1FF,$1F3 WC,WZ).
 *</pre>
 */
bool P2Asm::asm_RETI2()
{
    next();
    m_IR.set_inst7(p2_CALLD);
    m_IR.set_wcz(true);
    m_IR.set_dst_src(offs_INB, offs_IRET2);
    return parse_INST();
}

/**
 * @brief Return from INT1.
 *<pre>
 * EEEE 1011001 110 111111111 111110101
 *
 * RETI1
 *
 * (CALLD $1FF,$1F5 WC,WZ).
 *</pre>
 */
bool P2Asm::asm_RETI1()
{
    next();
    m_IR.set_inst7(p2_CALLD);
    m_IR.set_wcz(true);
    m_IR.set_dst_src(offs_INB, offs_IRET1);
    return parse_INST();
}

/**
 * @brief Return from INT0.
 *<pre>
 * EEEE 1011001 110 111111111 111111111
 *
 * RETI0
 *
 * (CALLD $1FF,$1FF WC,WZ).
 *</pre>
 */
bool P2Asm::asm_RETI0()
{
    next();
    m_IR.set_inst7(p2_CALLD);
    m_IR.set_wcz(true);
    m_IR.set_dst_src(offs_INB, offs_INB);
    return parse_INST();
}

/**
 * @brief Call to S** by pushing {C, Z, 10'b0, PC[19:0]} onto stack, copy D to PA.
 *<pre>
 * EEEE 1011010 0LI DDDDDDDDD SSSSSSSSS
 *
 * CALLPA  {#}D,{#}S
 *</pre>
 */
bool P2Asm::asm_CALLPA()
{
    next();
    m_IR.set_inst8(p2_CALLPA);
    return parse_WZ_D_IM_S();
}

/**
 * @brief Call to S** by pushing {C, Z, 10'b0, PC[19:0]} onto stack, copy D to PB.
 *<pre>
 * EEEE 1011010 1LI DDDDDDDDD SSSSSSSSS
 *
 * CALLPB  {#}D,{#}S
 *</pre>
 */
bool P2Asm::asm_CALLPB()
{
    next();
    m_IR.set_inst8(p2_CALLPB);
    return parse_WZ_D_IM_S();
}

/**
 * @brief Decrement D and jump to S** if result is zero.
 *<pre>
 * EEEE 1011011 00I DDDDDDDDD SSSSSSSSS
 *
 * DJZ     D,{#}S
 *</pre>
 */
bool P2Asm::asm_DJZ()
{
    next();
    m_IR.set_inst9(p2_DJZ);
    return parse_D_IM_S();
}

/**
 * @brief Decrement D and jump to S** if result is not zero.
 *<pre>
 * EEEE 1011011 01I DDDDDDDDD SSSSSSSSS
 *
 * DJNZ    D,{#}S
 *</pre>
 */
bool P2Asm::asm_DJNZ()
{
    next();
    m_IR.set_inst9(p2_DJNZ);
    return parse_D_IM_S();
}

/**
 * @brief Decrement D and jump to S** if result is $FFFF_FFFF.
 *<pre>
 * EEEE 1011011 10I DDDDDDDDD SSSSSSSSS
 *
 * DJF     D,{#}S
 *</pre>
 */
bool P2Asm::asm_DJF()
{
    next();
    m_IR.set_inst9(p2_DJF);
    return parse_D_IM_S();
}

/**
 * @brief Decrement D and jump to S** if result is not $FFFF_FFFF.
 *<pre>
 * EEEE 1011011 11I DDDDDDDDD SSSSSSSSS
 *
 * DJNF    D,{#}S
 *</pre>
 */
bool P2Asm::asm_DJNF()
{
    next();
    m_IR.set_inst9(p2_DJNF);
    return parse_D_IM_S();
}

/**
 * @brief Increment D and jump to S** if result is zero.
 *<pre>
 * EEEE 1011100 00I DDDDDDDDD SSSSSSSSS
 *
 * IJZ     D,{#}S
 *</pre>
 */
bool P2Asm::asm_IJZ()
{
    next();
    m_IR.set_inst9(p2_IJZ);
    return parse_D_IM_S();
}

/**
 * @brief Increment D and jump to S** if result is not zero.
 *<pre>
 * EEEE 1011100 01I DDDDDDDDD SSSSSSSSS
 *
 * IJNZ    D,{#}S
 *</pre>
 */
bool P2Asm::asm_IJNZ()
{
    next();
    m_IR.set_inst9(p2_IJNZ);
    return parse_D_IM_S();
}

/**
 * @brief Test D and jump to S** if D is zero.
 *<pre>
 * EEEE 1011100 10I DDDDDDDDD SSSSSSSSS
 *
 * TJZ     D,{#}S
 *</pre>
 */
bool P2Asm::asm_TJZ()
{
    next();
    m_IR.set_inst9(p2_TJZ);
    return parse_D_IM_S();
}

/**
 * @brief Test D and jump to S** if D is not zero.
 *<pre>
 * EEEE 1011100 11I DDDDDDDDD SSSSSSSSS
 *
 * TJNZ    D,{#}S
 *</pre>
 */
bool P2Asm::asm_TJNZ()
{
    next();
    m_IR.set_inst9(p2_TJNZ);
    return parse_D_IM_S();
}

/**
 * @brief Test D and jump to S** if D is full (D = $FFFF_FFFF).
 *<pre>
 * EEEE 1011101 00I DDDDDDDDD SSSSSSSSS
 *
 * TJF     D,{#}S
 *</pre>
 */
bool P2Asm::asm_TJF()
{
    next();
    m_IR.set_inst9(p2_TJF);
    return parse_D_IM_S();
}

/**
 * @brief Test D and jump to S** if D is not full (D != $FFFF_FFFF).
 *<pre>
 * EEEE 1011101 01I DDDDDDDDD SSSSSSSSS
 *
 * TJNF    D,{#}S
 *</pre>
 */
bool P2Asm::asm_TJNF()
{
    next();
    m_IR.set_inst9(p2_TJNF);
    return parse_D_IM_S();
}

/**
 * @brief Test D and jump to S** if D is signed (D[31] = 1).
 *<pre>
 * EEEE 1011101 10I DDDDDDDDD SSSSSSSSS
 *
 * TJS     D,{#}S
 *</pre>
 */
bool P2Asm::asm_TJS()
{
    next();
    m_IR.set_inst9(p2_TJS);
    return parse_D_IM_S();
}

/**
 * @brief Test D and jump to S** if D is not signed (D[31] = 0).
 *<pre>
 * EEEE 1011101 11I DDDDDDDDD SSSSSSSSS
 *
 * TJNS    D,{#}S
 *</pre>
 */
bool P2Asm::asm_TJNS()
{
    next();
    m_IR.set_inst9(p2_TJNS);
    return parse_D_IM_S();
}

/**
 * @brief Test D and jump to S** if D overflowed (D[31] != C, C = 'correct sign' from last addition/subtraction).
 *<pre>
 * EEEE 1011110 00I DDDDDDDDD SSSSSSSSS
 *
 * TJV     D,{#}S
 *</pre>
 */
bool P2Asm::asm_TJV()
{
    next();
    m_IR.set_inst9(p2_TJV);
    return parse_D_IM_S();
}

/**
 * @brief Jump to S** if INT event flag is set.
 *<pre>
 * EEEE 1011110 01I 000000000 SSSSSSSSS
 *
 * JINT    {#}S
 *</pre>
 */
bool P2Asm::asm_JINT()
{
    next();
    m_IR.set_opdst(p2_OPDST_JINT);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if CT1 event flag is set.
 *<pre>
 * EEEE 1011110 01I 000000001 SSSSSSSSS
 *
 * JCT1    {#}S
 *</pre>
 */
bool P2Asm::asm_JCT1()
{
    next();
    m_IR.set_opdst(p2_OPDST_JCT1);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if CT2 event flag is set.
 *<pre>
 * EEEE 1011110 01I 000000010 SSSSSSSSS
 *
 * JCT2    {#}S
 *</pre>
 */
bool P2Asm::asm_JCT2()
{
    next();
    m_IR.set_opdst(p2_OPDST_JCT2);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if CT3 event flag is set.
 *<pre>
 * EEEE 1011110 01I 000000011 SSSSSSSSS
 *
 * JCT3    {#}S
 *</pre>
 */
bool P2Asm::asm_JCT3()
{
    next();
    m_IR.set_opdst(p2_OPDST_JCT3);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if SE1 event flag is set.
 *<pre>
 * EEEE 1011110 01I 000000100 SSSSSSSSS
 *
 * JSE1    {#}S
 *</pre>
 */
bool P2Asm::asm_JSE1()
{
    next();
    m_IR.set_opdst(p2_OPDST_JSE1);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if SE2 event flag is set.
 *<pre>
 * EEEE 1011110 01I 000000101 SSSSSSSSS
 *
 * JSE2    {#}S
 *</pre>
 */
bool P2Asm::asm_JSE2()
{
    next();
    m_IR.set_opdst(p2_OPDST_JSE2);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if SE3 event flag is set.
 *<pre>
 * EEEE 1011110 01I 000000110 SSSSSSSSS
 *
 * JSE3    {#}S
 *</pre>
 */
bool P2Asm::asm_JSE3()
{
    next();
    m_IR.set_opdst(p2_OPDST_JSE3);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if SE4 event flag is set.
 *<pre>
 * EEEE 1011110 01I 000000111 SSSSSSSSS
 *
 * JSE4    {#}S
 *</pre>
 */
bool P2Asm::asm_JSE4()
{
    next();
    m_IR.set_opdst(p2_OPDST_JSE4);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if PAT event flag is set.
 *<pre>
 * EEEE 1011110 01I 000001000 SSSSSSSSS
 *
 * JPAT    {#}S
 *</pre>
 */
bool P2Asm::asm_JPAT()
{
    next();
    m_IR.set_opdst(p2_OPDST_JPAT);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if FBW event flag is set.
 *<pre>
 * EEEE 1011110 01I 000001001 SSSSSSSSS
 *
 * JFBW    {#}S
 *</pre>
 */
bool P2Asm::asm_JFBW()
{
    next();
    m_IR.set_opdst(p2_OPDST_JFBW);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if XMT event flag is set.
 *<pre>
 * EEEE 1011110 01I 000001010 SSSSSSSSS
 *
 * JXMT    {#}S
 *</pre>
 */
bool P2Asm::asm_JXMT()
{
    next();
    m_IR.set_opdst(p2_OPDST_JXMT);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if XFI event flag is set.
 *<pre>
 * EEEE 1011110 01I 000001011 SSSSSSSSS
 *
 * JXFI    {#}S
 *</pre>
 */
bool P2Asm::asm_JXFI()
{
    next();
    m_IR.set_opdst(p2_OPDST_JXFI);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if XRO event flag is set.
 *<pre>
 * EEEE 1011110 01I 000001100 SSSSSSSSS
 *
 * JXRO    {#}S
 *</pre>
 */
bool P2Asm::asm_JXRO()
{
    next();
    m_IR.set_opdst(p2_OPDST_JXRO);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if XRL event flag is set.
 *<pre>
 * EEEE 1011110 01I 000001101 SSSSSSSSS
 *
 * JXRL    {#}S
 *</pre>
 */
bool P2Asm::asm_JXRL()
{
    next();
    m_IR.set_opdst(p2_OPDST_JXRL);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if ATN event flag is set.
 *<pre>
 * EEEE 1011110 01I 000001110 SSSSSSSSS
 *
 * JATN    {#}S
 *</pre>
 */
bool P2Asm::asm_JATN()
{
    next();
    m_IR.set_opdst(p2_OPDST_JATN);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if QMT event flag is set.
 *<pre>
 * EEEE 1011110 01I 000001111 SSSSSSSSS
 *
 * JQMT    {#}S
 *</pre>
 */
bool P2Asm::asm_JQMT()
{
    next();
    m_IR.set_opdst(p2_OPDST_JQMT);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if INT event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000010000 SSSSSSSSS
 *
 * JNINT   {#}S
 *</pre>
 */
bool P2Asm::asm_JNINT()
{
    next();
    m_IR.set_opdst(p2_OPDST_JNINT);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if CT1 event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000010001 SSSSSSSSS
 *
 * JNCT1   {#}S
 *</pre>
 */
bool P2Asm::asm_JNCT1()
{
    next();
    m_IR.set_opdst(p2_OPDST_JNCT1);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if CT2 event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000010010 SSSSSSSSS
 *
 * JNCT2   {#}S
 *</pre>
 */
bool P2Asm::asm_JNCT2()
{
    next();
    m_IR.set_opdst(p2_OPDST_JNCT2);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if CT3 event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000010011 SSSSSSSSS
 *
 * JNCT3   {#}S
 *</pre>
 */
bool P2Asm::asm_JNCT3()
{
    next();
    m_IR.set_opdst(p2_OPDST_JNCT3);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if SE1 event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000010100 SSSSSSSSS
 *
 * JNSE1   {#}S
 *</pre>
 */
bool P2Asm::asm_JNSE1()
{
    next();
    m_IR.set_opdst(p2_OPDST_JNSE1);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if SE2 event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000010101 SSSSSSSSS
 *
 * JNSE2   {#}S
 *</pre>
 */
bool P2Asm::asm_JNSE2()
{
    next();
    m_IR.set_opdst(p2_OPDST_JNSE2);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if SE3 event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000010110 SSSSSSSSS
 *
 * JNSE3   {#}S
 *</pre>
 */
bool P2Asm::asm_JNSE3()
{
    next();
    m_IR.set_opdst(p2_OPDST_JNSE3);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if SE4 event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000010111 SSSSSSSSS
 *
 * JNSE4   {#}S
 *</pre>
 */
bool P2Asm::asm_JNSE4()
{
    next();
    m_IR.set_opdst(p2_OPDST_JNSE4);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if PAT event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000011000 SSSSSSSSS
 *
 * JNPAT   {#}S
 *</pre>
 */
bool P2Asm::asm_JNPAT()
{
    next();
    m_IR.set_opdst(p2_OPDST_JNPAT);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if FBW event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000011001 SSSSSSSSS
 *
 * JNFBW   {#}S
 *</pre>
 */
bool P2Asm::asm_JNFBW()
{
    next();
    m_IR.set_opdst(p2_OPDST_JNFBW);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if XMT event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000011010 SSSSSSSSS
 *
 * JNXMT   {#}S
 *</pre>
 */
bool P2Asm::asm_JNXMT()
{
    next();
    m_IR.set_opdst(p2_OPDST_JNXMT);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if XFI event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000011011 SSSSSSSSS
 *
 * JNXFI   {#}S
 *</pre>
 */
bool P2Asm::asm_JNXFI()
{
    next();
    m_IR.set_opdst(p2_OPDST_JNXFI);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if XRO event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000011100 SSSSSSSSS
 *
 * JNXRO   {#}S
 *</pre>
 */
bool P2Asm::asm_JNXRO()
{
    next();
    m_IR.set_opdst(p2_OPDST_JNXRO);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if XRL event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000011101 SSSSSSSSS
 *
 * JNXRL   {#}S
 *</pre>
 */
bool P2Asm::asm_JNXRL()
{
    next();
    m_IR.set_opdst(p2_OPDST_JNXRL);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if ATN event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000011110 SSSSSSSSS
 *
 * JNATN   {#}S
 *</pre>
 */
bool P2Asm::asm_JNATN()
{
    next();
    m_IR.set_opdst(p2_OPDST_JNATN);
    return parse_IM_S();
}

/**
 * @brief Jump to S** if QMT event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000011111 SSSSSSSSS
 *
 * JNQMT   {#}S
 *</pre>
 */
bool P2Asm::asm_JNQMT()
{
    next();
    m_IR.set_opdst(p2_OPDST_JNQMT);
    return parse_IM_S();
}

/**
 * @brief <empty>.
 *<pre>
 * EEEE 1011110 1LI DDDDDDDDD SSSSSSSSS
 *
 * <empty> {#}D,{#}S
 *</pre>
 */
bool P2Asm::asm_1011110_1()
{
    next();
    m_IR.set_inst8(p2_1011110_1);
    return parse_WZ_D_IM_S();
}

/**
 * @brief <empty>.
 *<pre>
 * EEEE 1011111 0LI DDDDDDDDD SSSSSSSSS
 *
 * <empty> {#}D,{#}S
 *</pre>
 */
bool P2Asm::asm_1011111_0()
{
    next();
    m_IR.set_inst8(p2_1011111_0);
    return parse_WZ_D_IM_S();
}

/**
 * @brief Set pin pattern for PAT event.
 *<pre>
 * EEEE 1011111 1LI DDDDDDDDD SSSSSSSSS
 *
 * SETPAT  {#}D,{#}S
 *
 * C selects INA/INB, Z selects =/!=, D provides mask value, S provides match value.
 *</pre>
 */
bool P2Asm::asm_SETPAT()
{
    next();
    m_IR.set_inst8(p2_SETPAT);
    return parse_WZ_D_IM_S();
}

/**
 * @brief Write D to mode register of smart pin S[5:0], acknowledge smart pin.
 *<pre>
 * EEEE 1100000 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRPIN   {#}D,{#}S
 *</pre>
 */
bool P2Asm::asm_WRPIN()
{
    next();
    m_IR.set_inst8(p2_WRPIN);
    return parse_WZ_D_IM_S();
}

/**
 * @brief Acknowledge smart pin S[5:0].
 *<pre>
 * EEEE 1100000 01I 000000001 SSSSSSSSS
 *
 * AKPIN   {#}S
 *</pre>
 */
bool P2Asm::asm_AKPIN()
{
    next();
    m_IR.set_inst8(p2_WRPIN);
    m_IR.set_wz(true);
    m_IR.set_dst(0x001);
    return parse_IM_S();
}

/**
 * @brief Write D to parameter "X" of smart pin S[5:0], acknowledge smart pin.
 *<pre>
 * EEEE 1100000 1LI DDDDDDDDD SSSSSSSSS
 *
 * WXPIN   {#}D,{#}S
 *</pre>
 */
bool P2Asm::asm_WXPIN()
{
    next();
    m_IR.set_inst8(p2_WXPIN);
    return parse_WZ_D_IM_S();
}

/**
 * @brief Write D to parameter "Y" of smart pin S[5:0], acknowledge smart pin.
 *<pre>
 * EEEE 1100001 0LI DDDDDDDDD SSSSSSSSS
 *
 * WYPIN   {#}D,{#}S
 *</pre>
 */
bool P2Asm::asm_WYPIN()
{
    next();
    m_IR.set_inst8(p2_WYPIN);
    return parse_WZ_D_IM_S();
}

/**
 * @brief Write D to LUT address S[8:0].
 *<pre>
 * EEEE 1100001 1LI DDDDDDDDD SSSSSSSSS
 *
 * WRLUT   {#}D,{#}S
 *</pre>
 */
bool P2Asm::asm_WRLUT()
{
    next();
    m_IR.set_inst8(p2_WRLUT);
    return parse_WZ_D_IM_S();
}

/**
 * @brief Write byte in D[7:0] to hub address {#}S/PTRx.
 *<pre>
 * EEEE 1100010 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRBYTE  {#}D,{#}S/P
 *</pre>
 */
bool P2Asm::asm_WRBYTE()
{
    next();
    m_IR.set_inst8(p2_WRBYTE);
    return parse_WZ_D_IM_S_PTRx(sz_BYTE);
}

/**
 * @brief Write word in D[15:0] to hub address {#}S/PTRx.
 *<pre>
 * EEEE 1100010 1LI DDDDDDDDD SSSSSSSSS
 *
 * WRWORD  {#}D,{#}S/P
 *</pre>
 */
bool P2Asm::asm_WRWORD()
{
    next();
    m_IR.set_inst8(p2_WRWORD);
    return parse_WZ_D_IM_S_PTRx(sz_WORD);
}

/**
 * @brief Write long in D[31:0] to hub address {#}S/PTRx.
 * Prior <pre>SETQ/SETQ2 invokes cog/LUT block transfer.
 *
 * EEEE 1100011 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRLONG  {#}D,{#}S/P
 *</pre>
 */
bool P2Asm::asm_WRLONG()
{
    next();
    m_IR.set_inst8(p2_WRLONG);
    return parse_WZ_D_IM_S_PTRx(sz_LONG);
}

/**
 * @brief Write long in D[31:0] to hub address PTRA++.
 *<pre>
 * EEEE 1100011 0L1 DDDDDDDDD 101100001
 *
 * PUSHA   {#}D
 *</pre>
 */
bool P2Asm::asm_PUSHA()
{
    next();
    m_IR.set_inst8(p2_WRLONG);
    m_IR.set_im(true);
    m_IR.set_src(0x161);
    return parse_WZ_D();
}

/**
 * @brief Write long in D[31:0] to hub address PTRB++.
 *<pre>
 * EEEE 1100011 0L1 DDDDDDDDD 111100001
 *
 * PUSHB   {#}D
 *</pre>
 */
bool P2Asm::asm_PUSHB()
{
    next();
    m_IR.set_inst8(p2_WRLONG);
    m_IR.set_im(true);
    m_IR.set_src(0x1e1);
    return parse_WZ_D();
}

/**
 * @brief Begin new fast hub read via FIFO.
 *<pre>
 * EEEE 1100011 1LI DDDDDDDDD SSSSSSSSS
 *
 * RDFAST  {#}D,{#}S
 *
 * D[31] = no wait, D[13:0] = block size in 64-byte units (0 = max), S[19:0] = block start address.
 *</pre>
 */
bool P2Asm::asm_RDFAST()
{
    next();
    m_IR.set_inst8(p2_RDFAST);
    return parse_WZ_D_IM_S();
}

/**
 * @brief Begin new fast hub write via FIFO.
 *<pre>
 * EEEE 1100100 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRFAST  {#}D,{#}S
 *
 * D[31] = no wait, D[13:0] = block size in 64-byte units (0 = max), S[19:0] = block start address.
 *</pre>
 */
bool P2Asm::asm_WRFAST()
{
    next();
    m_IR.set_inst8(p2_WRFAST);
    return parse_WZ_D_IM_S();
}

/**
 * @brief Set next block for when block wraps.
 *<pre>
 * EEEE 1100100 1LI DDDDDDDDD SSSSSSSSS
 *
 * FBLOCK  {#}D,{#}S
 *
 * D[13:0] = block size in 64-byte units (0 = max), S[19:0] = block start address.
 *</pre>
 */
bool P2Asm::asm_FBLOCK()
{
    next();
    m_IR.set_inst8(p2_FBLOCK);
    return parse_WZ_D_IM_S();
}

/**
 * @brief Issue streamer command immediately, zeroing phase.
 *<pre>
 * EEEE 1100101 0LI DDDDDDDDD SSSSSSSSS
 *
 * XINIT   {#}D,{#}S
 *</pre>
 */
bool P2Asm::asm_XINIT()
{
    next();
    m_IR.set_inst8(p2_XINIT);
    return parse_WZ_D_IM_S();
}

/**
 * @brief Stop streamer immediately.
 *<pre>
 * EEEE 1100101 011 000000000 000000000
 *
 * XSTOP
 *</pre>
 */
bool P2Asm::asm_XSTOP()
{
    next();
    m_IR.set_inst8(p2_XINIT);
    m_IR.set_wz(true);
    m_IR.set_im(true);
    m_IR.set_dst_src(0x000,0x000);
    return parse_INST();
}

/**
 * @brief Buffer new streamer command to be issued on final NCO rollover of current command, zeroing phase.
 *<pre>
 * EEEE 1100101 1LI DDDDDDDDD SSSSSSSSS
 *
 * XZERO   {#}D,{#}S
 *</pre>
 */
bool P2Asm::asm_XZERO()
{
    next();
    m_IR.set_inst8(p2_XZERO);
    return parse_WZ_D_IM_S();
}

/**
 * @brief Buffer new streamer command to be issued on final NCO rollover of current command, continuing phase.
 *<pre>
 * EEEE 1100110 0LI DDDDDDDDD SSSSSSSSS
 *
 * XCONT   {#}D,{#}S
 *</pre>
 */
bool P2Asm::asm_XCONT()
{
    next();
    m_IR.set_inst8(p2_XCONT);
    return parse_WZ_D_IM_S();
}

/**
 * @brief Execute next D[8:0] instructions S times.
 *<pre>
 * EEEE 1100110 1LI DDDDDDDDD SSSSSSSSS
 *
 * REP     {#}D,{#}S
 *
 * If S = 0, repeat infinitely.
 * If D[8:0] = 0, nothing repeats.
 *</pre>
 */
bool P2Asm::asm_REP()
{
    next();
    m_IR.set_inst8(p2_REP);
    return parse_WZ_D_IM_S();
}

/**
 * @brief Start cog selected by D.
 *<pre>
 * EEEE 1100111 CLI DDDDDDDDD SSSSSSSSS
 *
 * COGINIT {#}D,{#}S       {WC}
 *
 * S[19:0] sets hub startup address and PTRB of cog.
 * Prior SETQ sets PTRA of cog.
 *</pre>
 */
bool P2Asm::asm_COGINIT()
{
    next();
    m_IR.set_inst7(p2_COGINIT);
    return parse_WZ_D_IM_S_WC();
}

/**
 * @brief Begin CORDIC unsigned multiplication of D * S.
 *<pre>
 * EEEE 1101000 0LI DDDDDDDDD SSSSSSSSS
 *
 * QMUL    {#}D,{#}S
 *
 * GETQX/GETQY retrieves lower/upper product.
 *</pre>
 */
bool P2Asm::asm_QMUL()
{
    next();
    m_IR.set_inst8(p2_QMUL);
    return parse_WZ_D_IM_S();
}

/**
 * @brief Begin CORDIC unsigned division of {SETQ value or 32'b0, D} / S.
 *<pre>
 * EEEE 1101000 1LI DDDDDDDDD SSSSSSSSS
 *
 * QDIV    {#}D,{#}S
 *
 * GETQX/GETQY retrieves quotient/remainder.
 *</pre>
 */
bool P2Asm::asm_QDIV()
{
    next();
    m_IR.set_inst8(p2_QDIV);
    return parse_WZ_D_IM_S();
}

/**
 * @brief Begin CORDIC unsigned division of {D, SETQ value or 32'b0} / S.
 *<pre>
 * EEEE 1101001 0LI DDDDDDDDD SSSSSSSSS
 *
 * QFRAC   {#}D,{#}S
 *
 * GETQX/GETQY retrieves quotient/remainder.
 *</pre>
 */
bool P2Asm::asm_QFRAC()
{
    next();
    m_IR.set_inst8(p2_QFRAC);
    return parse_WZ_D_IM_S();
}

/**
 * @brief Begin CORDIC square root of {S, D}.
 *<pre>
 * EEEE 1101001 1LI DDDDDDDDD SSSSSSSSS
 *
 * QSQRT   {#}D,{#}S
 *
 * GETQX retrieves root.
 *</pre>
 */
bool P2Asm::asm_QSQRT()
{
    next();
    m_IR.set_inst8(p2_QSQRT);
    return parse_WZ_D_IM_S();
}

/**
 * @brief Begin CORDIC rotation of point (D, SETQ value or 32'b0) by angle S.
 *<pre>
 * EEEE 1101010 0LI DDDDDDDDD SSSSSSSSS
 *
 * QROTATE {#}D,{#}S
 *
 * GETQX/GETQY retrieves X/Y.
 *</pre>
 */
bool P2Asm::asm_QROTATE()
{
    next();
    m_IR.set_inst8(p2_QROTATE);
    return parse_WZ_D_IM_S();
}

/**
 * @brief Begin CORDIC vectoring of point (D, S).
 *<pre>
 * EEEE 1101010 1LI DDDDDDDDD SSSSSSSSS
 *
 * QVECTOR {#}D,{#}S
 *
 * GETQX/GETQY retrieves length/angle.
 *</pre>
 */
bool P2Asm::asm_QVECTOR()
{
    next();
    m_IR.set_inst8(p2_QVECTOR);
    return parse_WZ_D_IM_S();
}

/**
 * @brief Set hub configuration to D.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000000000
 *
 * HUBSET  {#}D
 *</pre>
 */
bool P2Asm::asm_HUBSET()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_HUBSET);
    return parse_IM_D();
}

/**
 * @brief If D is register and no WC, get cog ID (0 to 15) into D.
 *<pre>
 * EEEE 1101011 C0L DDDDDDDDD 000000001
 *
 * COGID   {#}D            {WC}
 *
 * If WC, check status of cog D[3:0], C = 1 if on.
 *</pre>
 */
bool P2Asm::asm_COGID()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_COGID);
    return parse_IM_D_WC();
}

/**
 * @brief Stop cog D[3:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000000011
 *
 * COGSTOP {#}D
 *</pre>
 */
bool P2Asm::asm_COGSTOP()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_COGSTOP);
    return parse_IM_D();
}

/**
 * @brief Request a LOCK.
 *<pre>
 * EEEE 1101011 C00 DDDDDDDDD 000000100
 *
 * LOCKNEW D               {WC}
 *
 * D will be written with the LOCK number (0 to 15).
 * C = 1 if no LOCK available.
 *</pre>
 */
bool P2Asm::asm_LOCKNEW()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_LOCKNEW);
    return parse_IM_D_WC();
}

/**
 * @brief Return LOCK D[3:0] for reallocation.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000000101
 *
 * LOCKRET {#}D
 *</pre>
 */
bool P2Asm::asm_LOCKRET()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_LOCKRET);
    return parse_IM_D();
}

/**
 * @brief Try to get LOCK D[3:0].
 *<pre>
 * EEEE 1101011 C0L DDDDDDDDD 000000110
 *
 * LOCKTRY {#}D            {WC}
 *
 * C = 1 if got LOCK.
 * LOCKREL releases LOCK.
 * LOCK is also released if owner cog stops or restarts.
 *</pre>
 */
bool P2Asm::asm_LOCKTRY()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_LOCKTRY);
    return parse_IM_D_WC();
}

/**
 * @brief Release LOCK D[3:0].
 *<pre>
 * EEEE 1101011 C0L DDDDDDDDD 000000111
 *
 * LOCKREL {#}D            {WC}
 *
 * If D is a register and WC, get current/last cog id of LOCK owner into D and LOCK status into C.
 *</pre>
 */
bool P2Asm::asm_LOCKREL()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_LOCKREL);
    return parse_IM_D_WC();
}

/**
 * @brief Begin CORDIC number-to-logarithm conversion of D.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000001110
 *
 * QLOG    {#}D
 *
 * GETQX retrieves log {5'whole_exponent, 27'fractional_exponent}.
 *</pre>
 */
bool P2Asm::asm_QLOG()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_QLOG);
    return parse_IM_D();
}

/**
 * @brief Begin CORDIC logarithm-to-number conversion of D.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000001111
 *
 * QEXP    {#}D
 *
 * GETQX retrieves number.
 *</pre>
 */
bool P2Asm::asm_QEXP()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_QEXP);
    return parse_IM_D();
}

/**
 * @brief Read zero-extended byte from FIFO into D. Used after RDFAST.
 *<pre>
 * EEEE 1101011 CZ0 DDDDDDDDD 000010000
 *
 * RFBYTE  D        {WC/WZ/WCZ}
 *
 * C = MSB of byte.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_RFBYTE()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_RFBYTE);
    return parse_D_WCZ();
}

/**
 * @brief Read zero-extended word from FIFO into D. Used after RDFAST.
 *<pre>
 * EEEE 1101011 CZ0 DDDDDDDDD 000010001
 *
 * RFWORD  D        {WC/WZ/WCZ}
 *
 * C = MSB of word.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_RFWORD()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_RFWORD);
    return parse_D_WCZ();
}

/**
 * @brief Read long from FIFO into D. Used after RDFAST.
 *<pre>
 * EEEE 1101011 CZ0 DDDDDDDDD 000010010
 *
 * RFLONG  D        {WC/WZ/WCZ}
 *
 * C = MSB of long.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_RFLONG()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_RFLONG);
    return parse_D_WCZ();
}

/**
 * @brief Read zero-extended 1..4-byte value from FIFO into D. Used after RDFAST.
 *<pre>
 * EEEE 1101011 CZ0 DDDDDDDDD 000010011
 *
 * RFVAR   D        {WC/WZ/WCZ}
 *
 * C = 0.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_RFVAR()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_RFVAR);
    return parse_D_WCZ();
}

/**
 * @brief Read sign-extended 1..4-byte value from FIFO into D. Used after RDFAST.
 *<pre>
 * EEEE 1101011 CZ0 DDDDDDDDD 000010100
 *
 * RFVARS  D        {WC/WZ/WCZ}
 *
 * C = MSB of value.
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_RFVARS()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_RFVARS);
    return parse_D_WCZ();
}

/**
 * @brief Write byte in D[7:0] into FIFO. Used after WRFAST.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000010101
 *
 * WFBYTE  {#}D
 *</pre>
 */
bool P2Asm::asm_WFBYTE()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_WFBYTE);
    return parse_IM_D();
}

/**
 * @brief Write word in D[15:0] into FIFO. Used after WRFAST.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000010110
 *
 * WFWORD  {#}D
 *</pre>
 */
bool P2Asm::asm_WFWORD()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_WFWORD);
    return parse_IM_D();
}

/**
 * @brief Write long in D[31:0] into FIFO. Used after WRFAST.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000010111
 *
 * WFLONG  {#}D
 *</pre>
 */
bool P2Asm::asm_WFLONG()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_WFLONG);
    return parse_IM_D();
}

/**
 * @brief Retrieve CORDIC result X into D.
 *<pre>
 * EEEE 1101011 CZ0 DDDDDDDDD 000011000
 *
 * GETQX   D        {WC/WZ/WCZ}
 *
 * Waits, in case result not ready.
 * C = X[31].
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_GETQX()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_GETQX);
    return parse_D_WCZ();
}

/**
 * @brief Retrieve CORDIC result Y into D.
 *<pre>
 * EEEE 1101011 CZ0 DDDDDDDDD 000011001
 *
 * GETQY   D        {WC/WZ/WCZ}
 *
 * Waits, in case result no ready.
 * C = Y[31].
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_GETQY()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_GETQY);
    return parse_D_WCZ();
}

/**
 * @brief Get CT into D.
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 000011010
 *
 * GETCT   D
 *
 * CT is the free-running 32-bit system counter that increments on every clock.
 *</pre>
 */
bool P2Asm::asm_GETCT()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_GETCT);
    return parse_D();
}

/**
 * @brief Get RND into D/C/Z.
 *<pre>
 * EEEE 1101011 CZ0 DDDDDDDDD 000011011
 *
 * GETRND  D        {WC/WZ/WCZ}
 *
 * RND is the PRNG that updates on every clock.
 * D = RND[31:0], C = RND[31], Z = RND[30], unique per cog.
 *</pre>
 */
bool P2Asm::asm_GETRND()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_GETRND);
    return parse_D_WCZ();
}

/**
 * @brief Get RND into C/Z.
 *<pre>
 * EEEE 1101011 CZ1 000000000 000011011
 *
 * GETRND            WC/WZ/WCZ
 *
 * C = RND[31], Z = RND[30], unique per cog.
 *</pre>
 */
bool P2Asm::asm_GETRND_CZ()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_GETRND);
    return parse_WCZ();
}

/**
 * @brief DAC3 = D[31:24], DAC2 = D[23:16], DAC1 = D[15:8], DAC0 = D[7:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000011100
 *
 * SETDACS {#}D
 *</pre>
 */
bool P2Asm::asm_SETDACS()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SETDACS);
    return parse_IM_D();
}

/**
 * @brief Set streamer NCO frequency to D.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000011101
 *
 * SETXFRQ {#}D
 *</pre>
 */
bool P2Asm::asm_SETXFRQ()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SETXFRQ);
    return parse_IM_D();
}

/**
 * @brief Get the streamer's Goertzel X accumulator into D and the Y accumulator into the next instruction's S, clear accumulators.
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 000011110
 *
 * GETXACC D
 *</pre>
 */
bool P2Asm::asm_GETXACC()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_GETXACC);
    return parse_D();
}

/**
 * @brief Wait 2 + D clocks if no WC/WZ/WCZ.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000011111
 *
 * WAITX   {#}D     {WC/WZ/WCZ}
 *
 * If WC/WZ/WCZ, wait 2 + (D & RND) clocks.
 * C/Z = 0.
 *</pre>
 */
bool P2Asm::asm_WAITX()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_WAITX);
    return parse_IM_D_WCZ();
}

/**
 * @brief Set SE1 event configuration to D[8:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000100000
 *
 * SETSE1  {#}D
 *</pre>
 */
bool P2Asm::asm_SETSE1()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SETSE1);
    return parse_IM_D();
}

/**
 * @brief Set SE2 event configuration to D[8:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000100001
 *
 * SETSE2  {#}D
 *</pre>
 */
bool P2Asm::asm_SETSE2()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SETSE2);
    return parse_IM_D();
}

/**
 * @brief Set SE3 event configuration to D[8:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000100010
 *
 * SETSE3  {#}D
 *</pre>
 */
bool P2Asm::asm_SETSE3()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SETSE3);
    return parse_IM_D();
}

/**
 * @brief Set SE4 event configuration to D[8:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000100011
 *
 * SETSE4  {#}D
 *</pre>
 */
bool P2Asm::asm_SETSE4()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SETSE4);
    return parse_IM_D();
}

/**
 * @brief Get INT event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000000000 000100100
 *
 * POLLINT          {WC/WZ/WCZ}
 *</pre>
 */
bool P2Asm::asm_POLLINT()
{
    next();
    m_IR.set_opx24(p2_OPX24_POLLINT);
    return parse_WCZ();
}

/**
 * @brief Get CT1 event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000000001 000100100
 *
 * POLLCT1          {WC/WZ/WCZ}
 *</pre>
 */
bool P2Asm::asm_POLLCT1()
{
    next();
    m_IR.set_opx24(p2_OPX24_POLLCT1);
    return parse_WCZ();
}

/**
 * @brief Get CT2 event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000000010 000100100
 *
 * POLLCT2          {WC/WZ/WCZ}
 *</pre>
 */
bool P2Asm::asm_POLLCT2()
{
    next();
    m_IR.set_opx24(p2_OPX24_POLLCT2);
    return parse_WCZ();
}

/**
 * @brief Get CT3 event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000000011 000100100
 *
 * POLLCT3          {WC/WZ/WCZ}
 *</pre>
 */
bool P2Asm::asm_POLLCT3()
{
    next();
    m_IR.set_opx24(p2_OPX24_POLLCT3);
    return parse_WCZ();
}

/**
 * @brief Get SE1 event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000000100 000100100
 *
 * POLLSE1          {WC/WZ/WCZ}
 *</pre>
 */
bool P2Asm::asm_POLLSE1()
{
    next();
    m_IR.set_opx24(p2_OPX24_POLLSE1);
    return parse_WCZ();
}

/**
 * @brief Get SE2 event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000000101 000100100
 *
 * POLLSE2          {WC/WZ/WCZ}
 *</pre>
 */
bool P2Asm::asm_POLLSE2()
{
    next();
    m_IR.set_opx24(p2_OPX24_POLLSE2);
    return parse_WCZ();
}

/**
 * @brief Get SE3 event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000000110 000100100
 *
 * POLLSE3          {WC/WZ/WCZ}
 *</pre>
 */
bool P2Asm::asm_POLLSE3()
{
    next();
    m_IR.set_opx24(p2_OPX24_POLLSE3);
    return parse_WCZ();
}

/**
 * @brief Get SE4 event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000000111 000100100
 *
 * POLLSE4          {WC/WZ/WCZ}
 *</pre>
 */
bool P2Asm::asm_POLLSE4()
{
    next();
    m_IR.set_opx24(p2_OPX24_POLLSE4);
    return parse_WCZ();
}

/**
 * @brief Get PAT event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000001000 000100100
 *
 * POLLPAT          {WC/WZ/WCZ}
 *</pre>
 */
bool P2Asm::asm_POLLPAT()
{
    next();
    m_IR.set_opx24(p2_OPX24_POLLPAT);
    return parse_WCZ();
}

/**
 * @brief Get FBW event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000001001 000100100
 *
 * POLLFBW          {WC/WZ/WCZ}
 *</pre>
 */
bool P2Asm::asm_POLLFBW()
{
    next();
    m_IR.set_opx24(p2_OPX24_POLLFBW);
    return parse_WCZ();
}

/**
 * @brief Get XMT event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000001010 000100100
 *
 * POLLXMT          {WC/WZ/WCZ}
 *</pre>
 */
bool P2Asm::asm_POLLXMT()
{
    next();
    m_IR.set_opx24(p2_OPX24_POLLXMT);
    return parse_WCZ();
}

/**
 * @brief Get XFI event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000001011 000100100
 *
 * POLLXFI          {WC/WZ/WCZ}
 *</pre>
 */
bool P2Asm::asm_POLLXFI()
{
    next();
    m_IR.set_opx24(p2_OPX24_POLLXFI);
    return parse_WCZ();
}

/**
 * @brief Get XRO event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000001100 000100100
 *
 * POLLXRO          {WC/WZ/WCZ}
 *</pre>
 */
bool P2Asm::asm_POLLXRO()
{
    next();
    m_IR.set_opx24(p2_OPX24_POLLXRO);
    return parse_WCZ();
}

/**
 * @brief Get XRL event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000001101 000100100
 *
 * POLLXRL          {WC/WZ/WCZ}
 *</pre>
 */
bool P2Asm::asm_POLLXRL()
{
    next();
    m_IR.set_opx24(p2_OPX24_POLLXRL);
    return parse_WCZ();
}

/**
 * @brief Get ATN event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000001110 000100100
 *
 * POLLATN          {WC/WZ/WCZ}
 *</pre>
 */
bool P2Asm::asm_POLLATN()
{
    next();
    m_IR.set_opx24(p2_OPX24_POLLATN);
    return parse_WCZ();
}

/**
 * @brief Get QMT event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000001111 000100100
 *
 * POLLQMT          {WC/WZ/WCZ}
 *</pre>
 */
bool P2Asm::asm_POLLQMT()
{
    next();
    m_IR.set_opx24(p2_OPX24_POLLQMT);
    return parse_WCZ();
}

/**
 * @brief Wait for INT event flag, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000010000 000100100
 *
 * WAITINT          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *</pre>
 */
bool P2Asm::asm_WAITINT()
{
    next();
    m_IR.set_opx24(p2_OPX24_WAITINT);
    return parse_WCZ();
}

/**
 * @brief Wait for CT1 event flag, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000010001 000100100
 *
 * WAITCT1          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *</pre>
 */
bool P2Asm::asm_WAITCT1()
{
    next();
    m_IR.set_opx24(p2_OPX24_WAITCT1);
    return parse_WCZ();
}

/**
 * @brief Wait for CT2 event flag, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000010010 000100100
 *
 * WAITCT2          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *</pre>
 */
bool P2Asm::asm_WAITCT2()
{
    next();
    m_IR.set_opx24(p2_OPX24_WAITCT2);
    return parse_WCZ();
}

/**
 * @brief Wait for CT3 event flag, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000010011 000100100
 *
 * WAITCT3          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *</pre>
 */
bool P2Asm::asm_WAITCT3()
{
    next();
    m_IR.set_opx24(p2_OPX24_WAITCT3);
    return parse_WCZ();
}

/**
 * @brief Wait for SE1 event flag, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000010100 000100100
 *
 * WAITSE1          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *</pre>
 */
bool P2Asm::asm_WAITSE1()
{
    next();
    m_IR.set_opx24(p2_OPX24_WAITSE1);
    return parse_WCZ();
}

/**
 * @brief Wait for SE2 event flag, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000010101 000100100
 *
 * WAITSE2          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *</pre>
 */
bool P2Asm::asm_WAITSE2()
{
    next();
    m_IR.set_opx24(p2_OPX24_WAITSE2);
    return parse_WCZ();
}

/**
 * @brief Wait for SE3 event flag, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000010110 000100100
 *
 * WAITSE3          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *</pre>
 */
bool P2Asm::asm_WAITSE3()
{
    next();
    m_IR.set_opx24(p2_OPX24_WAITSE3);
    return parse_WCZ();
}

/**
 * @brief Wait for SE4 event flag, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000010111 000100100
 *
 * WAITSE4          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *</pre>
 */
bool P2Asm::asm_WAITSE4()
{
    next();
    m_IR.set_opx24(p2_OPX24_WAITSE4);
    return parse_WCZ();
}

/**
 * @brief Wait for PAT event flag, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000011000 000100100
 *
 * WAITPAT          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *</pre>
 */
bool P2Asm::asm_WAITPAT()
{
    next();
    m_IR.set_opx24(p2_OPX24_WAITPAT);
    return parse_WCZ();
}

/**
 * @brief Wait for FBW event flag, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000011001 000100100
 *
 * WAITFBW          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *</pre>
 */
bool P2Asm::asm_WAITFBW()
{
    next();
    m_IR.set_opx24(p2_OPX24_WAITFBW);
    return parse_WCZ();
}

/**
 * @brief Wait for XMT event flag, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000011010 000100100
 *
 * WAITXMT          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *</pre>
 */
bool P2Asm::asm_WAITXMT()
{
    next();
    m_IR.set_opx24(p2_OPX24_WAITXMT);
    return parse_WCZ();
}

/**
 * @brief Wait for XFI event flag, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000011011 000100100
 *
 * WAITXFI          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *</pre>
 */
bool P2Asm::asm_WAITXFI()
{
    next();
    m_IR.set_opx24(p2_OPX24_WAITXFI);
    return parse_WCZ();
}

/**
 * @brief Wait for XRO event flag, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000011100 000100100
 *
 * WAITXRO          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *</pre>
 */
bool P2Asm::asm_WAITXRO()
{
    next();
    m_IR.set_opx24(p2_OPX24_WAITXRO);
    return parse_WCZ();
}

/**
 * @brief Wait for XRL event flag, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000011101 000100100
 *
 * WAITXRL          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *</pre>
 */
bool P2Asm::asm_WAITXRL()
{
    next();
    m_IR.set_opx24(p2_OPX24_WAITXRL);
    return parse_WCZ();
}

/**
 * @brief Wait for ATN event flag, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000011110 000100100
 *
 * WAITATN          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 *</pre>
 */
bool P2Asm::asm_WAITATN()
{
    next();
    m_IR.set_opx24(p2_OPX24_WAITATN);
    return parse_WCZ();
}

/**
 * @brief Allow interrupts (default).
 *<pre>
 * EEEE 1101011 000 000100000 000100100
 *
 * ALLOWI
 *</pre>
 */
bool P2Asm::asm_ALLOWI()
{
    next();
    m_IR.set_opx24(p2_OPX24_ALLOWI);
    return parse_INST();
}

/**
 * @brief Stall Interrupts.
 *<pre>
 * EEEE 1101011 000 000100001 000100100
 *
 * STALLI
 *</pre>
 */
bool P2Asm::asm_STALLI()
{
    next();
    m_IR.set_opx24(p2_OPX24_STALLI);
    return parse_INST();
}

/**
 * @brief Trigger INT1, regardless of STALLI mode.
 *<pre>
 * EEEE 1101011 000 000100010 000100100
 *
 * TRGINT1
 *</pre>
 */
bool P2Asm::asm_TRGINT1()
{
    next();
    m_IR.set_opx24(p2_OPX24_TRGINT1);
    return parse_INST();
}

/**
 * @brief Trigger INT2, regardless of STALLI mode.
 *<pre>
 * EEEE 1101011 000 000100011 000100100
 *
 * TRGINT2
 *</pre>
 */
bool P2Asm::asm_TRGINT2()
{
    next();
    m_IR.set_opx24(p2_OPX24_TRGINT2);
    return parse_INST();
}

/**
 * @brief Trigger INT3, regardless of STALLI mode.
 *<pre>
 * EEEE 1101011 000 000100100 000100100
 *
 * TRGINT3
 *</pre>
 */
bool P2Asm::asm_TRGINT3()
{
    next();
    m_IR.set_opx24(p2_OPX24_TRGINT3);
    return parse_INST();
}

/**
 * @brief Cancel INT1.
 *<pre>
 * EEEE 1101011 000 000100101 000100100
 *
 * NIXINT1
 *</pre>
 */
bool P2Asm::asm_NIXINT1()
{
    next();
    m_IR.set_opx24(p2_OPX24_NIXINT1);
    return parse_INST();
}

/**
 * @brief Cancel INT2.
 *<pre>
 * EEEE 1101011 000 000100110 000100100
 *
 * NIXINT2
 *</pre>
 */
bool P2Asm::asm_NIXINT2()
{
    next();
    m_IR.set_opx24(p2_OPX24_NIXINT2);
    return parse_INST();
}

/**
 * @brief Cancel INT3.
 *<pre>
 * EEEE 1101011 000 000100111 000100100
 *
 * NIXINT3
 *</pre>
 */
bool P2Asm::asm_NIXINT3()
{
    next();
    m_IR.set_opx24(p2_OPX24_NIXINT3);
    return parse_INST();
}

/**
 * @brief Set INT1 source to D[3:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000100101
 *
 * SETINT1 {#}D
 *</pre>
 */
bool P2Asm::asm_SETINT1()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SETINT1);
    return parse_IM_D();
}

/**
 * @brief Set INT2 source to D[3:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000100110
 *
 * SETINT2 {#}D
 *</pre>
 */
bool P2Asm::asm_SETINT2()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SETINT2);
    return parse_IM_D();
}

/**
 * @brief Set INT3 source to D[3:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000100111
 *
 * SETINT3 {#}D
 *</pre>
 */
bool P2Asm::asm_SETINT3()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SETINT3);
    return parse_IM_D();
}

/**
 * @brief Set Q to D.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000101000
 *
 * SETQ    {#}D
 *
 * Use before RDLONG/WRLONG/WMLONG to set block transfer.
 * Also used before MUXQ/COGINIT/QDIV/QFRAC/QROTATE/WAITxxx.
 *</pre>
 */
bool P2Asm::asm_SETQ()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SETQ);
    return parse_IM_D();
}

/**
 * @brief Set Q to D.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000101001
 *
 * SETQ2   {#}D
 *
 * Use before RDLONG/WRLONG/WMLONG to set LUT block transfer.
 *</pre>
 */
bool P2Asm::asm_SETQ2()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SETQ2);
    return parse_IM_D();
}

/**
 * @brief Push D onto stack.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000101010
 *
 * PUSH    {#}D
 *</pre>
 */
bool P2Asm::asm_PUSH()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_PUSH);
    return parse_IM_D();
}

/**
 * @brief Pop stack (K).
 *<pre>
 * EEEE 1101011 CZ0 DDDDDDDDD 000101011
 *
 * POP     D        {WC/WZ/WCZ}
 *
 * C = K[31], Z = K[30], D = K.
 *</pre>
 */
bool P2Asm::asm_POP()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_POP);
    return parse_D_WCZ();
}

/**
 * @brief Jump to D.
 *<pre>
 * EEEE 1101011 CZ0 DDDDDDDDD 000101100
 *
 * JMP     {#}D     {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 *</pre>
 */
bool P2Asm::asm_JMP()
{
    int idx = m_idx;
    next();
    p2_token_e tok = curr_tok();
    if (t_IMMEDIATE == tok || t_AUGMENTED == tok) {
        P2Atom atom = parse_expression();
        if (atom.get_long() > COG_MASK) {
            m_idx = idx;
            return asm_JMP_abs();
        }
        m_idx = idx + 1;
    }
    m_IR.set_opsrc(p2_OPSRC_JMP);
    return parse_IM_D_WCZ();
}

/**
 * @brief Call to D by pushing {C, Z, 10'b0, PC[19:0]} onto stack.
 *<pre>
 * EEEE 1101011 CZ0 DDDDDDDDD 000101101
 *
 * CALL    {#}D     {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 *</pre>
 */
bool P2Asm::asm_CALL()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_CALL_RET);
    return parse_IM_D_WCZ();
}

/**
 * @brief Return by popping stack (K).
 *<pre>
 * EEEE 1101011 CZ1 000000000 000101101
 *
 * RET              {WC/WZ/WCZ}
 *
 * C = K[31], Z = K[30], PC = K[19:0].
 *</pre>
 */
bool P2Asm::asm_RET()
{
    next();
    m_IR.set_im(true);
    m_IR.set_dst(0x000);
    m_IR.set_opsrc(p2_OPSRC_CALL_RET);
    return parse_WCZ();
}

/**
 * @brief Call to D by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRA++.
 *<pre>
 * EEEE 1101011 CZ0 DDDDDDDDD 000101110
 *
 * CALLA   D        {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 *</pre>
 */
bool P2Asm::asm_CALLA()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_CALLA_RETA);
    return parse_D_WCZ();
}

/**
 * @brief Return by reading hub long (L) at --PTRA.
 *<pre>
 * EEEE 1101011 CZ1 000000000 000101110
 *
 * RETA             {WC/WZ/WCZ}
 *
 * C = L[31], Z = L[30], PC = L[19:0].
 *</pre>
 */
bool P2Asm::asm_RETA()
{
    next();
    m_IR.set_im(true);
    m_IR.set_dst(0x000);
    m_IR.set_opsrc(p2_OPSRC_CALLA_RETA);
    return parse_WCZ();
}

/**
 * @brief Call to D by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRB++.
 *<pre>
 * EEEE 1101011 CZ0 DDDDDDDDD 000101111
 *
 * CALLB   D        {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 *</pre>
 */
bool P2Asm::asm_CALLB()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_CALLB_RETB);
    return parse_D_WCZ();
}

/**
 * @brief Return by reading hub long (L) at --PTRB.
 *<pre>
 * EEEE 1101011 CZ1 000000000 000101111
 *
 * RETB             {WC/WZ/WCZ}
 *
 * C = L[31], Z = L[30], PC = L[19:0].
 *</pre>
 */
bool P2Asm::asm_RETB()
{
    next();
    m_IR.set_im(true);
    m_IR.set_dst(0x000);
    m_IR.set_opsrc(p2_OPSRC_CALLB_RETB);
    return parse_WCZ();
}

/**
 * @brief Jump ahead/back by D instructions.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000110000
 *
 * JMPREL  {#}D
 *
 * For cogex, PC += D[19:0].
 * For hubex, PC += D[17:0] << 2.
 *</pre>
 */
bool P2Asm::asm_JMPREL()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_JMPREL);
    return parse_IM_D();
}

/**
 * @brief Skip instructions per D.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000110001
 *
 * SKIP    {#}D
 *
 * Subsequent instructions 0..31 get cancelled for each '1' bit in D[0]..D[31].
 *</pre>
 */
bool P2Asm::asm_SKIP()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SKIP);
    return parse_IM_D();
}

/**
 * @brief Skip cog/LUT instructions fast per D.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000110010
 *
 * SKIPF   {#}D
 *
 * Like SKIP, but instead of cancelling instructions, the PC leaps over them.
 *</pre>
 */
bool P2Asm::asm_SKIPF()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SKIPF);
    return parse_IM_D();
}

/**
 * @brief Jump to D[9:0] in cog/LUT and set SKIPF pattern to D[31:10].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000110011
 *
 * EXECF   {#}D
 *
 * PC = {10'b0, D[9:0]}.
 *</pre>
 */
bool P2Asm::asm_EXECF()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_EXECF);
    return parse_IM_D();
}

/**
 * @brief Get current FIFO hub pointer into D.
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 000110100
 *
 * GETPTR  D
 *</pre>
 */
bool P2Asm::asm_GETPTR()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_GETPTR);
    return parse_D();
}

/**
 * @brief Get breakpoint status into D according to WC/WZ/WCZ.
 *<pre>
 * EEEE 1101011 CZ0 DDDDDDDDD 000110101
 *
 * GETBRK  D          WC/WZ/WCZ
 *
 * C = 0.
 * Z = 0.
 *</pre>
 */
bool P2Asm::asm_GETBRK()
{
    next();
    m_IR.set_im(false);
    m_IR.set_opsrc(p2_OPSRC_COGBRK);
    return parse_D_WCZ();
}

/**
 * @brief If in debug ISR, trigger asynchronous breakpoint in cog D[3:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000110101
 *
 * COGBRK  {#}D
 *
 * Cog D[3:0] must have asynchronous breakpoint enabled.
 *</pre>
 */
bool P2Asm::asm_COGBRK()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_COGBRK);
    return parse_IM_D();
}

/**
 * @brief If in debug ISR, set next break condition to D.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000110110
 *
 * BRK     {#}D
 *
 * Else, trigger break if enabled, conditionally write break code to D[7:0].
 *</pre>
 */
bool P2Asm::asm_BRK()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_BRK);
    return parse_IM_D();
}

/**
 * @brief If D[0] = 1 then enable LUT sharing, where LUT writes within the adjacent odd/even companion cog are copied to this LUT.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000110111
 *
 * SETLUTS {#}D
 *</pre>
 */
bool P2Asm::asm_SETLUTS()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SETLUTS);
    return parse_IM_D();
}

/**
 * @brief Set the colorspace converter "CY" parameter to D[31:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000111000
 *
 * SETCY   {#}D
 *</pre>
 */
bool P2Asm::asm_SETCY()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SETCY);
    return parse_IM_D();
}

/**
 * @brief Set the colorspace converter "CI" parameter to D[31:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000111001
 *
 * SETCI   {#}D
 *</pre>
 */
bool P2Asm::asm_SETCI()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SETCI);
    return parse_IM_D();
}

/**
 * @brief Set the colorspace converter "CQ" parameter to D[31:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000111010
 *
 * SETCQ   {#}D
 *</pre>
 */
bool P2Asm::asm_SETCQ()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SETCQ);
    return parse_IM_D();
}

/**
 * @brief Set the colorspace converter "CFRQ" parameter to D[31:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000111011
 *
 * SETCFRQ {#}D
 *</pre>
 */
bool P2Asm::asm_SETCFRQ()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SETCFRQ);
    return parse_IM_D();
}

/**
 * @brief Set the colorspace converter "CMOD" parameter to D[6:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000111100
 *
 * SETCMOD {#}D
 *</pre>
 */
bool P2Asm::asm_SETCMOD()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SETCMOD);
    return parse_IM_D();
}

/**
 * @brief Set BLNPIX/MIXPIX blend factor to D[7:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000111101
 *
 * SETPIV  {#}D
 *</pre>
 */
bool P2Asm::asm_SETPIV()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SETPIV);
    return parse_IM_D();
}

/**
 * @brief Set MIXPIX mode to D[5:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000111110
 *
 * SETPIX  {#}D
 *</pre>
 */
bool P2Asm::asm_SETPIX()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SETPIX);
    return parse_IM_D();
}

/**
 * @brief Strobe "attention" of all cogs whose corresponging bits are high in D[15:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000111111
 *
 * COGATN  {#}D
 *</pre>
 */
bool P2Asm::asm_COGATN()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_COGATN);
    return parse_IM_D();
}

/**
 * @brief Test  IN bit of pin D[5:0], write to C/Z.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001000000
 *
 * TESTP   {#}D           WC/WZ
 *
 * C/Z =          IN[D[5:0]].
 *</pre>
 */
bool P2Asm::asm_TESTP_W()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_TESTP_W_DIRL);
    return parse_IM_D_WCZ();
}

/**
 * @brief Test !IN bit of pin D[5:0], write to C/Z.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001000001
 *
 * TESTPN  {#}D           WC/WZ
 *
 * C/Z =         !IN[D[5:0]].
 *</pre>
 */
bool P2Asm::asm_TESTPN_W()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_TESTPN_W);
    return parse_IM_D_WCZ();
}

/**
 * @brief Test  IN bit of pin D[5:0], AND into C/Z.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001000010
 *
 * TESTP   {#}D       ANDC/ANDZ
 *
 * C/Z = C/Z AND  IN[D[5:0]].
 *</pre>
 */
bool P2Asm::asm_TESTP_AND()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_TESTP_AND);
    return parse_IM_D_ANDC_ANDZ();
}

/**
 * @brief Test !IN bit of pin D[5:0], AND into C/Z.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001000011
 *
 * TESTPN  {#}D       ANDC/ANDZ
 *
 * C/Z = C/Z AND !IN[D[5:0]].
 *</pre>
 */
bool P2Asm::asm_TESTPN_AND()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_TESTPN_AND);
    return parse_IM_D_ANDC_ANDZ();
}

/**
 * @brief Test  IN bit of pin D[5:0], OR  into C/Z.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001000100
 *
 * TESTP   {#}D         ORC/ORZ
 *
 * C/Z = C/Z OR   IN[D[5:0]].
 *</pre>
 */
bool P2Asm::asm_TESTP_OR()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_TESTP_OR);
    return parse_IM_D_ORC_ORZ();
}

/**
 * @brief Test !IN bit of pin D[5:0], OR  into C/Z.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001000101
 *
 * TESTPN  {#}D         ORC/ORZ
 *
 * C/Z = C/Z OR  !IN[D[5:0]].
 *</pre>
 */
bool P2Asm::asm_TESTPN_OR()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_TESTPN_OR);
    return parse_IM_D_ORC_ORZ();
}

/**
 * @brief Test  IN bit of pin D[5:0], XOR into C/Z.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001000110
 *
 * TESTP   {#}D       XORC/XORZ
 *
 * C/Z = C/Z XOR  IN[D[5:0]].
 *</pre>
 */
bool P2Asm::asm_TESTP_XOR()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_TESTP_XOR);
    return parse_IM_D_XORC_XORZ();
}

/**
 * @brief Test !IN bit of pin D[5:0], XOR into C/Z.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001000111
 *
 * TESTPN  {#}D       XORC/XORZ
 *
 * C/Z = C/Z XOR !IN[D[5:0]].
 *</pre>
 */
bool P2Asm::asm_TESTPN_XOR()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_TESTPN_XOR);
    return parse_IM_D_XORC_XORZ();
}

/**
 * @brief Test IN bit of pin D[5:0], SET/AND/OR/XOR nto C/Z.
 * @return true on success, or false on error
 */
bool P2Asm::asm_TESTP()
{
    if (find_tok(t_WC) || find_tok(t_WZ))
        return asm_TESTP_W();
    if (find_tok(t_ANDC) || find_tok(t_ANDZ))
        return asm_TESTP_AND();
    if (find_tok(t_ORC) || find_tok(t_ORZ))
        return asm_TESTP_OR();
    if (find_tok(t_XORC) || find_tok(t_XORZ))
        return asm_TESTP_XOR();
    return false;
}

/**
 * @brief Test !IN bit of pin D[5:0], SET, AND, OR, XOR into C/Z.
 * @return true on success, or false on error
 */
bool P2Asm::asm_TESTPN()
{
    if (find_tok(t_WC) || find_tok(t_WZ))
        return asm_TESTPN_W();
    if (find_tok(t_ANDC) || find_tok(t_ANDZ))
        return asm_TESTPN_AND();
    if (find_tok(t_ORC) || find_tok(t_ORZ))
        return asm_TESTPN_OR();
    if (find_tok(t_XORC) || find_tok(t_XORZ))
        return asm_TESTPN_XOR();
    return false;
}

/**
 * @brief DIR bit of pin D[5:0] = 0.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001000000
 *
 * DIRL    {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *</pre>
 */
bool P2Asm::asm_DIRL()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_DIRL);
    return parse_IM_D_WCZ();
}

/**
 * @brief DIR bit of pin D[5:0] = 1.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001000001
 *
 * DIRH    {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *</pre>
 */
bool P2Asm::asm_DIRH()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_DIRH);
    return parse_IM_D_WCZ();
}

/**
 * @brief DIR bit of pin D[5:0] = C.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001000010
 *
 * DIRC    {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *</pre>
 */
bool P2Asm::asm_DIRC()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_DIRC);
    return parse_IM_D_WCZ();
}

/**
 * @brief DIR bit of pin D[5:0] = !C.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001000011
 *
 * DIRNC   {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *</pre>
 */
bool P2Asm::asm_DIRNC()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_DIRNC);
    return parse_IM_D_WCZ();
}

/**
 * @brief DIR bit of pin D[5:0] = Z.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001000100
 *
 * DIRZ    {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *</pre>
 */
bool P2Asm::asm_DIRZ()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_DIRZ);
    return parse_IM_D_WCZ();
}

/**
 * @brief DIR bit of pin D[5:0] = !Z.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001000101
 *
 * DIRNZ   {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *</pre>
 */
bool P2Asm::asm_DIRNZ()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_DIRNZ);
    return parse_IM_D_WCZ();
}

/**
 * @brief DIR bit of pin D[5:0] = RND.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001000110
 *
 * DIRRND  {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *</pre>
 */
bool P2Asm::asm_DIRRND()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_DIRRND);
    return parse_IM_D_WCZ();
}

/**
 * @brief DIR bit of pin D[5:0] = !bit.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001000111
 *
 * DIRNOT  {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *</pre>
 */
bool P2Asm::asm_DIRNOT()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_DIRNOT);
    return parse_IM_D_WCZ();
}

/**
 * @brief OUT bit of pin D[5:0] = 0.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001001000
 *
 * OUTL    {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *</pre>
 */
bool P2Asm::asm_OUTL()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_OUTL);
    return parse_IM_D_WCZ();
}

/**
 * @brief OUT bit of pin D[5:0] = 1.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001001001
 *
 * OUTH    {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *</pre>
 */
bool P2Asm::asm_OUTH()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_OUTH);
    return parse_IM_D_WCZ();
}

/**
 * @brief OUT bit of pin D[5:0] = C.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001001010
 *
 * OUTC    {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *</pre>
 */
bool P2Asm::asm_OUTC()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_OUTC);
    return parse_IM_D_WCZ();
}

/**
 * @brief OUT bit of pin D[5:0] = !C.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001001011
 *
 * OUTNC   {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *</pre>
 */
bool P2Asm::asm_OUTNC()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_OUTNC);
    return parse_IM_D_WCZ();
}

/**
 * @brief OUT bit of pin D[5:0] = Z.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001001100
 *
 * OUTZ    {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *</pre>
 */
bool P2Asm::asm_OUTZ()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_OUTZ);
    return parse_IM_D_WCZ();
}

/**
 * @brief OUT bit of pin D[5:0] = !Z.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001001101
 *
 * OUTNZ   {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *</pre>
 */
bool P2Asm::asm_OUTNZ()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_OUTNZ);
    return parse_IM_D_WCZ();
}

/**
 * @brief OUT bit of pin D[5:0] = RND.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001001110
 *
 * OUTRND  {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *</pre>
 */
bool P2Asm::asm_OUTRND()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_OUTRND);
    return parse_IM_D_WCZ();
}

/**
 * @brief OUT bit of pin D[5:0] = !bit.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001001111
 *
 * OUTNOT  {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *</pre>
 */
bool P2Asm::asm_OUTNOT()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_OUTNOT);
    return parse_IM_D_WCZ();
}

/**
 * @brief OUT bit of pin D[5:0] = 0.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001010000
 *
 * FLTL    {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 *</pre>
 */
bool P2Asm::asm_FLTL()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_FLTL);
    return parse_IM_D_WCZ();
}

/**
 * @brief OUT bit of pin D[5:0] = 1.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001010001
 *
 * FLTH    {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 *</pre>
 */
bool P2Asm::asm_FLTH()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_FLTH);
    return parse_IM_D_WCZ();
}

/**
 * @brief OUT bit of pin D[5:0] = C.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001010010
 *
 * FLTC    {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 *</pre>
 */
bool P2Asm::asm_FLTC()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_FLTC);
    return parse_IM_D_WCZ();
}

/**
 * @brief OUT bit of pin D[5:0] = !C.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001010011
 *
 * FLTNC   {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 *</pre>
 */
bool P2Asm::asm_FLTNC()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_FLTNC);
    return parse_IM_D_WCZ();
}

/**
 * @brief OUT bit of pin D[5:0] = Z.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001010100
 *
 * FLTZ    {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 *</pre>
 */
bool P2Asm::asm_FLTZ()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_FLTZ);
    return parse_IM_D_WCZ();
}

/**
 * @brief OUT bit of pin D[5:0] = !Z.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001010101
 *
 * FLTNZ   {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 *</pre>
 */
bool P2Asm::asm_FLTNZ()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_FLTNZ);
    return parse_IM_D_WCZ();
}

/**
 * @brief OUT bit of pin D[5:0] = RND.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001010110
 *
 * FLTRND  {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 *</pre>
 */
bool P2Asm::asm_FLTRND()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_FLTRND);
    return parse_IM_D_WCZ();
}

/**
 * @brief OUT bit of pin D[5:0] = !bit.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001010111
 *
 * FLTNOT  {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 *</pre>
 */
bool P2Asm::asm_FLTNOT()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_FLTNOT);
    return parse_IM_D_WCZ();
}

/**
 * @brief OUT bit of pin D[5:0] = 0.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001011000
 *
 * DRVL    {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 *</pre>
 */
bool P2Asm::asm_DRVL()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_DRVL);
    return parse_IM_D_WCZ();
}

/**
 * @brief OUT bit of pin D[5:0] = 1.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001011001
 *
 * DRVH    {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 *</pre>
 */
bool P2Asm::asm_DRVH()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_DRVH);
    return parse_IM_D_WCZ();
}

/**
 * @brief OUT bit of pin D[5:0] = C.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001011010
 *
 * DRVC    {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 *</pre>
 */
bool P2Asm::asm_DRVC()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_DRVC);
    return parse_IM_D_WCZ();
}

/**
 * @brief OUT bit of pin D[5:0] = !C.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001011011
 *
 * DRVNC   {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 *</pre>
 */
bool P2Asm::asm_DRVNC()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_DRVNC);
    return parse_IM_D_WCZ();
}

/**
 * @brief OUT bit of pin D[5:0] = Z.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001011100
 *
 * DRVZ    {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 *</pre>
 */
bool P2Asm::asm_DRVZ()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_DRVZ);
    return parse_IM_D_WCZ();
}

/**
 * @brief OUT bit of pin D[5:0] = !Z.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001011101
 *
 * DRVNZ   {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 *</pre>
 */
bool P2Asm::asm_DRVNZ()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_DRVNZ);
    return parse_IM_D_WCZ();
}

/**
 * @brief OUT bit of pin D[5:0] = RND.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001011110
 *
 * DRVRND  {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 *</pre>
 */
bool P2Asm::asm_DRVRND()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_DRVRND);
    return parse_IM_D_WCZ();
}

/**
 * @brief OUT bit of pin D[5:0] = !bit.
 *<pre>
 * EEEE 1101011 CZL DDDDDDDDD 001011111
 *
 * DRVNOT  {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 *</pre>
 */
bool P2Asm::asm_DRVNOT()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_DRVNOT);
    return parse_IM_D_WCZ();
}

/**
 * @brief Split every 4th bit of S into bytes of D.
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 001100000
 *
 * SPLITB  D
 *
 * D = {S[31], S[27], S[23], S[19], ... S[12], S[8], S[4], S[0]}.
 *</pre>
 */
bool P2Asm::asm_SPLITB()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SPLITB);
    return parse_D();
}

/**
 * @brief Merge bits of bytes in S into D.
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 001100001
 *
 * MERGEB  D
 *
 * D = {S[31], S[23], S[15], S[7], ... S[24], S[16], S[8], S[0]}.
 *</pre>
 */
bool P2Asm::asm_MERGEB()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_MERGEB);
    return parse_D();
}

/**
 * @brief Split bits of S into words of D.
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 001100010
 *
 * SPLITW  D
 *
 * D = {S[31], S[29], S[27], S[25], ... S[6], S[4], S[2], S[0]}.
 *</pre>
 */
bool P2Asm::asm_SPLITW()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SPLITW);
    return parse_D();
}

/**
 * @brief Merge bits of words in S into D.
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 001100011
 *
 * MERGEW  D
 *
 * D = {S[31], S[15], S[30], S[14], ... S[17], S[1], S[16], S[0]}.
 *</pre>
 */
bool P2Asm::asm_MERGEW()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_MERGEW);
    return parse_D();
}

/**
 * @brief Relocate and periodically invert bits from S into D.
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 001100100
 *
 * SEUSSF  D
 *
 * Returns to original value on 32nd iteration.
 * Forward pattern.
 *</pre>
 */
bool P2Asm::asm_SEUSSF()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SEUSSF);
    return parse_D();
}

/**
 * @brief Relocate and periodically invert bits from S into D.
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 001100101
 *
 * SEUSSR  D
 *
 * Returns to original value on 32nd iteration.
 * Reverse pattern.
 *</pre>
 */
bool P2Asm::asm_SEUSSR()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SEUSSR);
    return parse_D();
}

/**
 * @brief Squeeze 8:8:8 RGB value in S[31:8] into 5:6:5 value in D[15:0].
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 001100110
 *
 * RGBSQZ  D
 *
 * D = {15'b0, S[31:27], S[23:18], S[15:11]}.
 *</pre>
 */
bool P2Asm::asm_RGBSQZ()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_RGBSQZ);
    return parse_D();
}

/**
 * @brief Expand 5:6:5 RGB value in S[15:0] into 8:8:8 value in D[31:8].
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 001100111
 *
 * RGBEXP  D
 *
 * D = {S[15:11,15:13], S[10:5,10:9], S[4:0,4:2], 8'b0}.
 *</pre>
 */
bool P2Asm::asm_RGBEXP()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_RGBEXP);
    return parse_D();
}

/**
 * @brief Iterate D with xoroshiro32+ PRNG algorithm and put PRNG result into next instruction's S.
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 001101000
 *
 * XORO32  D
 *</pre>
 */
bool P2Asm::asm_XORO32()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_XORO32);
    return parse_D();
}

/**
 * @brief Reverse D bits.
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 001101001
 *
 * REV     D
 *
 * D = D[0:31].
 *</pre>
 */
bool P2Asm::asm_REV()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_REV);
    return parse_D();
}

/**
 * @brief Rotate C,Z right through D.
 *<pre>
 * EEEE 1101011 CZ0 DDDDDDDDD 001101010
 *
 * RCZR    D        {WC/WZ/WCZ}
 *
 * D = {C, Z, D[31:2]}.
 * C = D[1],  Z = D[0].
 *</pre>
 */
bool P2Asm::asm_RCZR()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_RCZR);
    return parse_D_WCZ();
}

/**
 * @brief Rotate C,Z left through D.
 *<pre>
 * EEEE 1101011 CZ0 DDDDDDDDD 001101011
 *
 * RCZL    D        {WC/WZ/WCZ}
 *
 * D = {D[29:0], C, Z}.
 * C = D[31], Z = D[30].
 *</pre>
 */
bool P2Asm::asm_RCZL()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_RCZL);
    return parse_D_WCZ();
}

/**
 * @brief Write 0 or 1 to D, according to  C.
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 001101100
 *
 * WRC     D
 *
 * D = {31'b0,  C).
 *</pre>
 */
bool P2Asm::asm_WRC()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_WRC);
    return parse_D();
}

/**
 * @brief Write 0 or 1 to D, according to !C.
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 001101101
 *
 * WRNC    D
 *
 * D = {31'b0, !C).
 *</pre>
 */
bool P2Asm::asm_WRNC()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_WRNC);
    return parse_D();
}

/**
 * @brief Write 0 or 1 to D, according to  Z.
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 001101110
 *
 * WRZ     D
 *
 * D = {31'b0,  Z).
 *</pre>
 */
bool P2Asm::asm_WRZ()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_WRZ);
    return parse_D();
}

/**
 * @brief Write 0 or 1 to D, according to !Z.
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 001101111
 *
 * WRNZ    D
 *
 * D = {31'b0, !Z).
 *</pre>
 */
bool P2Asm::asm_WRNZ()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_WRNZ_MODCZ);
    return parse_D();
}

/**
 * @brief Modify C and Z according to cccc and zzzz.
 *<pre>
 * EEEE 1101011 CZ1 0cccczzzz 001101111
 *
 * MODCZ   c,z      {WC/WZ/WCZ}
 *
 * C = cccc[{C,Z}], Z = zzzz[{C,Z}].
 *</pre>
 */
bool P2Asm::asm_MODCZ()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_WRNZ_MODCZ);

    p2_cond_e cccc = parse_modcz();
    if (!mandatory_COMMA())
        return false;

    p2_cond_e zzzz = parse_modcz();
    m_IR.set_dst(static_cast<p2_LONG>((cccc << 4) | zzzz), m_ORG, m_ORGH);

    optional_WCZ();
    return end_of_line();
}

/**
 * @brief Modify C according to cccc.
 *<pre>
 * EEEE 1101011 CZ1 0cccc0000 001101111
 *
 * MODC    c        {WC/WZ/WCZ}
 *
 * C = cccc[{C,Z}].
 *</pre>
 */
bool P2Asm::asm_MODC()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_WRNZ_MODCZ);

    p2_cond_e cccc = parse_modcz();
    m_IR.set_dst(static_cast<p2_LONG>(cccc << 4), m_ORG, m_ORGH);

    optional_WCZ();
    return end_of_line();
}

/**
 * @brief Modify Z according to zzzz.
 *<pre>
 * EEEE 1101011 CZ1 00000zzzz 001101111
 *
 * MODCZ   z        {WC/WZ/WCZ}
 *
 * Z = zzzz[{C,Z}].
 *</pre>
 */
bool P2Asm::asm_MODZ()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_WRNZ_MODCZ);

    p2_cond_e zzzz = parse_modcz();
    m_IR.set_dst(static_cast<p2_LONG>(zzzz), m_ORG, m_ORGH);

    optional_WCZ();
    return end_of_line();
}

/**
 * @brief Set scope mode.
 *<pre>
 * SETSCP points the scope mux to a set of four pins starting
 * at (D[5:0] AND $3C), with D[6]=1 to enable scope operation.
 *
 * EEEE 1101011 00L DDDDDDDDD 001110000
 *
 * SETSCP  {#}D
 *
 * Pins D[5:2], enable D[6].
 *</pre>
 */
bool P2Asm::asm_SETSCP()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_SETSCP);
    return parse_IM_D();
}

/**
 * @brief Get scope values.
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 001110001
 *
 * Any time GETSCP is executed, the lower bytes of those four pins' RDPIN values are returned in D.
 * This feature will mainly be useful on the next silicon, as the FPGAs don't have ADC-capable pins.
 *
 * GETSCP  D
 *</pre>
 */
bool P2Asm::asm_GETSCP()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_GETSCP);
    return parse_D();
}

/**
 * @brief Jump to A.
 *<pre>
 * EEEE 1101100 RAA AAAAAAAAA AAAAAAAAA
 *
 * JMP     #A
 *
 * If R = 1, PC += A, else PC = A.
 *</pre>
 */
bool P2Asm::asm_JMP_abs()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_JMP);
    return parse_PC_A20();
}

/**
 * @brief Call to A by pushing {C, Z, 10'b0, PC[19:0]} onto stack.
 *<pre>
 * EEEE 1101101 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALL    #A
 *
 * If R = 1, PC += A, else PC = A.
 *</pre>
 */
bool P2Asm::asm_CALL_abs()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_CALL_RET);
    return parse_PC_A20();
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRA++.
 *<pre>
 * EEEE 1101110 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLA   #A
 *
 * If R = 1, PC += A, else PC = A.
 *</pre>
 */
bool P2Asm::asm_CALLA_abs()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_CALLA_RETA);
    return parse_PC_A20();
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRB++.
 *<pre>
 * EEEE 1101111 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLB   #A
 *
 * If R = 1, PC += A, else PC = A.
 *</pre>
 */
bool P2Asm::asm_CALLB_abs()
{
    next();
    m_IR.set_opsrc(p2_OPSRC_CALLB_RETB);
    return parse_PC_A20();
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PA/PB/PTRA/PTRB (per W).
 *<pre>
 * EEEE 1110000 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PA,#A
 *
 * If R = 1, PC += A, else PC = A.
 *</pre>
 */
bool P2Asm::asm_CALLD_abs_PA()
{
    next();
    m_IR.set_inst7(p2_CALLD_ABS_PA);
    return parse_PC_A20();
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PA/PB/PTRA/PTRB (per W).
 *<pre>
 * EEEE 1110001 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PB,#A
 *
 * If R = 1, PC += A, else PC = A.
 *</pre>
 */
bool P2Asm::asm_CALLD_abs_PB()
{
    next();
    m_IR.set_inst7(p2_CALLD_ABS_PB);
    return parse_PC_A20();
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PTRA (per W).
 *<pre>
 * EEEE 1110010 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PTRA,#A
 *
 * If R = 1, PC += A, else PC = A.
 *</pre>
 */
bool P2Asm::asm_CALLD_abs_PTRA()
{
    next();
    m_IR.set_inst7(p2_CALLD_ABS_PTRA);
    return parse_PC_A20();
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PTRB (per W).
 *<pre>
 * EEEE 1110011 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PTRB,#A
 *
 * If R = 1, PC += A, else PC = A.
 *</pre>
 */
bool P2Asm::asm_CALLD_abs_PTRB()
{
    next();
    m_IR.set_inst7(p2_CALLD_ABS_PTRB);
    return parse_PC_A20();
}

/**
 * @brief Get {12'b0, address[19:0]} into PA/PB/PTRA/PTRB (per W).
 *<pre>
 * EEEE 11101WW RAA AAAAAAAAA AAAAAAAAA
 *
 * LOC     PA/PB/PTRA/PTRB,#A
 *
 * If R = 1, address = PC + A, else address = A.
 *</pre>
 */
bool P2Asm::asm_LOC()
{
    next();
    const int idx = m_idx;
    P2Atom ptr = parse_dst();
    m_IR.set_inst5(p2_LOC);
    switch (ptr.get_long()) {
    case offs_PA:
        m_IR.set_inst7(p2_LOC_PA);
        break;
    case offs_PB:
        m_IR.set_inst7(p2_LOC_PB);
        break;
    case offs_PTRA:
        m_IR.set_inst7(p2_LOC_PTRA);
        break;
    case offs_PTRB:
        m_IR.set_inst7(p2_LOC_PTRB);
        break;
    default:
        m_idx = idx;
        m_errors += tr("Invalid pointer '%1'; expected one of %2.")
                  .arg(curr_str())
                  .arg(tr("PA, PB, PTRA, or PTRB"));
        emit Error(m_pass, m_lineno, m_errors.last());
    }
    if (!mandatory_COMMA())
        return false;
    return parse_PC_A20();
}

/**
 * @brief Queue #N[31:9] to be used as upper 23 bits for next #S occurrence, so that the next 9-bit #S will be augmented to 32 bits.
 *<pre>
 * EEEE 11110NN NNN NNNNNNNNN NNNNNNNNN
 *
 * AUGS    #N
 *</pre>
 */
bool P2Asm::asm_AUGS()
{
    next();
    m_IR.set_inst7(p2_AUGS_00);
    return parse_IMM23();
}

/**
 * @brief Queue #N[31:9] to be used as upper 23 bits for next #D occurrence, so that the next 9-bit #D will be augmented to 32 bits.
 *<pre>
 * EEEE 11111NN NNN NNNNNNNNN NNNNNNNNN
 *
 * AUGD    #N
 *</pre>
 */
bool P2Asm::asm_AUGD()
{
    next();
    m_IR.set_inst7(p2_AUGD_00);
    return parse_IMM23();
}
