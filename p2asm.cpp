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
#include "p2util.h"

#define DBG_EXPR 0 //! set to 1 to debug expression parsing

static const QString p2_section_dat = QStringLiteral("DAT");
static const QString p2_section_con = QStringLiteral("CON");
static const QString p2_section_pub = QStringLiteral("PUB");
static const QString p2_section_pri = QStringLiteral("PRI");
static const QString p2_section_var = QStringLiteral("VAR");

static const QString p2_prefix_bin_const = QStringLiteral("_BIN::");
static const QString p2_prefix_byt_const = QStringLiteral("_BYT::");
static const QString p2_prefix_oct_const = QStringLiteral("_OCT::");
static const QString p2_prefix_dec_const = QStringLiteral("_DEC::");
static const QString p2_prefix_hex_const = QStringLiteral("_HEX::");
static const QString p2_prefix_str_const = QStringLiteral("_STR::");
static const QString p2_prefix_real_const = QStringLiteral("_REAL::");
static const QString p2_prefix_lut_const = QStringLiteral("_LUT::");


#if DBG_EXPR
//! debug expression parsing
#define DEBUG_EXPR(x,...) do { if (m_pass > 1) { qDebug(x,__VA_ARGS__); } } while (0)
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
    , m_hash_OPC()
    , m_hash_words()
    , m_hash_error()
    , m_symbols(P2SymbolTable(new P2SymbolTableObj()))
    , m_lineno(0)
    , m_in_curly(0)
    , m_line()
    , m_errors()
    , m_ORG(0)
    , m_ORG_MAX(0)
    , m_ORGH(0)
    , m_advance(4)
    , m_IR()
    , m_words()
    , m_instr()
    , m_symbol()
    , m_function()
    , m_section(sec_dat)
    , m_cnt(0)
    , m_idx(0)
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
    m_pass = -1;
    m_symbols->clear();
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
    m_hash_OPC.clear();
    m_hash_error.clear();
    m_lineno = 0;
    m_in_curly = 0;
    m_line.clear();
    m_errors.clear();
    m_ORG = 0;
    m_ORG_MAX = 0;
    m_ORGH = 0;
    m_advance = 0;
    m_IR.clear();
    m_data.clear();
    m_words.clear();
    m_instr = t_invalid;
    m_symbol.clear();
    m_function.clear();
    m_section = sec_dat;
    m_cnt = 0;
    m_idx = 0;
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

const QString P2Asm::source(int idx) const
{
    return m_source.value(idx);
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

const p2_pc_orgh_hash_t& P2Asm::PC_ORGH_hash() const
{
    return m_hash_PC;
}

p2_PC_ORGH_t P2Asm::get_PC_ORGH(int lineno) const
{
    return m_hash_PC.value(lineno);
}

bool P2Asm::has_PC_ORGH(int lineno) const
{
    return m_hash_PC.contains(lineno);
}

const p2_opcode_hash_t& P2Asm::IR_hash() const
{
    return m_hash_OPC;
}

P2Opcode P2Asm::get_IR(int lineno) const
{
    return m_hash_OPC.value(lineno);
}

bool P2Asm::has_IR(int lineno) const
{
    return m_hash_OPC.contains(lineno);
}

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
    p2_token_e cond = m_words.value(m_idx).tok();
    if (Token.is_type(cond, tm_conditional)) {
        result = Token.conditional(cond, cc_always);
        m_idx += 1;
    }
    return result;
}

p2_cond_e P2Asm::parse_modcz()
{
    p2_token_e cond = m_words.value(m_idx).tok();
    if (Token.is_type(cond, tm_modcz_param)) {
        p2_cond_e result = Token.modcz_param(cond, cc_clr);
        m_idx += 1;
        return result;
    }
    P2Atom atom = parse_expression();
    if (!atom.isValid()) {
        m_errors += tr("Expected MODCZ param or immediate.");
        emit Error(m_pass, m_lineno, m_errors.last());
        return cc_clr;
    }
    p2_LONG value = atom.to_long();
    if (value > 15) {
        m_errors += tr("Expected MODCZ immediate in range 0…15.");
        emit Error(m_pass, m_lineno, m_errors.last());
        return cc_clr;
    }
    return static_cast<p2_cond_e>(value);
}

/**
 * @brief Tokenize a line
 * @param line assembler source line
 * @return true on success (currently always)
 */
bool P2Asm::tokenize(const QString& line)
{
    // FIXME: curly braces counter below 0!
    Q_ASSERT(m_in_curly >= 0);
    const P2Words words = Token.tokenize(line, m_lineno, m_in_curly);
    m_words = words;
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
 * @brief Return the current P2Word's string
 * @return const reference to the current word
 */
const QString P2Asm::curr_str() const
{
    const P2Word& word = curr_word();
    return word.str();
}

/**
 * @brief Return the current P2Word's token
 * @return token enumeration value
 */
p2_token_e P2Asm::curr_tok() const
{
    const P2Word& word = curr_word();
    return word.tok();
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
        m_errors.clear();
        m_words.clear();
        m_data.clear();
        m_advance = 0;
        m_IR.as_IR = false;
        m_IR.PC_ORGH = p2_PC_ORGH_t(m_ORG, m_ORGH);
        m_IR.DATA.clear();

        // Split line into words and tokenize it
        tokenize(m_line);

        // Skip over empty lines
        if (m_words.isEmpty()) {
            results();
            continue;
        }

        m_symbol.clear();
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

            // defining a symbol at the current PC
            if (!m_symbol.isEmpty()) {
                const P2Word& word = curr_word();
                const p2_LONG PC = m_ORG < HUB_ADDR0 ? m_ORG / 4 : m_ORG;
                P2Symbol sym = m_symbols->symbol(m_symbol);
                next();

                if (sym.isEmpty()) {
                    P2Atom atom(PC, P2Atom::PC);
                    // Not defined yet
                    m_symbols->insert(m_symbol, atom);
                    m_symbols->add_reference(m_lineno, m_symbol, word);
                } else if (m_pass > 1) {
                    P2Atom atom(PC, P2Atom::PC);
                    m_symbols->set_atom(m_symbol, atom);
                } else if (!sym.isNull()) {
                    // Already defined
                    const p2_LONG value = sym.value<p2_LONG>();
                    m_errors += tr("Symbol '%1' already defined in line #%2 (Value: %3, $%4, %%5).")
                              .arg(m_symbol)
                              .arg(sym.definition().lineno())
                              .arg(value)
                              .arg(value, 6, 16, QChar('0'))
                              .arg(value, 32, 2, QChar('0'));
                    emit Error(m_pass, m_lineno, m_errors.last());
                }
            }
            break;
        }

        // Skip if no more words/tokens were found
        if (!skip_comments()) {
            m_IR.as_IR = false;
            results();
            continue;
        }

        // Assume the instruction advances by 4 bytes
        m_advance = 4;

        // Clear the opcode
        m_IR.clear();

        // Assume the instruction emits IR
        m_IR.as_IR = true;

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

            case t__ALIGNW:
                success = asm_alignw();
                break;

            case t__ALIGNL:
                success = asm_alignl();
                break;

            case t__ORG:
                success = asm_org();
                break;

            case t__ORGH:
                success = asm_orgh();
                break;

            case t__FIT:
                success = asm_fit();
                break;

            case t__ASSIGN:
                success = asm_assign();
                break;

            default:
                if (Token.is_type(m_instr, tm_mnemonic)) {
                    // Missing handling of an instruction token
                    m_errors += tr("Missing handling of instruction token '%1'.")
                          .arg(Token.string(m_instr));
                    emit Error(m_pass, m_lineno, m_errors.last());
                }
                if (Token.is_type(m_instr, tm_constant)) {
                    // Unexpected constant token
                    m_errors += tr("Constant '%1' used as an instruction.")
                              .arg(Token.string(m_instr));
                    emit Error(m_pass, m_lineno, m_errors.last());
                }
                if (Token.is_type(m_instr, tm_conditional)) {
                    // Unexpected conditional token
                    m_errors += tr("Extraneous conditional '%1'.")
                              .arg(Token.string(m_instr));
                    emit Error(m_pass, m_lineno, m_errors.last());
                }
                if (Token.is_type(m_instr, tm_modcz_param)) {
                    // Unexpected MODCZ parameter token
                    m_errors += tr("Extraneous MODCZ parameter '%1'.")
                              .arg(Token.string(m_instr));
                    emit Error(m_pass, m_lineno, m_errors.last());
                }
                m_idx = m_cnt;
            }
        }

        // store the results, emit listing, etc.
        results();

        Q_UNUSED(success)
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

bool P2Asm::setSource(int idx, const QString& source)
{
    if (idx < 0 || idx >= m_source.count())
        return false;
    if (source == m_source[idx])
        return true;

    m_source.replace(idx, source);
    m_pass = -1;
    clear();

    return true;
}

/**
 * @brief Return number of commata following in the words/tokens
 * @return number of commata following
 */
int P2Asm::commata_left() const
{
    int commata = 0;
    for (int i = m_idx; i < m_cnt; i++)
        if (t__COMMA == m_words[i].tok())
            commata++;
    return commata;
}

/**
 * @brief Instruction register was constructed
 * @param wr_mem if true, store opcode into MEM
 * @return formatted string
 */
QStringList P2Asm::results_instruction(bool wr_mem)
{
    QStringList output;
    p2_LONG PC = m_ORG;
    p2_LONG ORGH = m_ORGH;

    QString line = m_line;
    Q_ASSERT(m_advance == 4);

    m_IR.PC_ORGH = p2_PC_ORGH_t(m_ORG, m_ORGH);
    m_hash_PC.insert(m_lineno, m_IR.PC_ORGH);
    m_hash_OPC.insert(m_lineno, m_IR);

    // Do we need to generate an AUGD instruction?
    if (m_IR.AUGD.isValid()) {
        p2_LONG value = m_IR.AUGD.value<p2_LONG>() & AUG_MASK;
        P2Opcode IR(p2_AUGD, p2_PC_ORGH_t(PC, ORGH));
        IR.u.opcode |= value >> AUG_SHIFT;

        output += QString("%1 %2 [%3] %4")
                 .arg(m_lineno, -6)
                 .arg(PC, 6, 16, QChar('0'))
                 .arg(IR.u.opcode, 8, 16, QChar('0'))
                 .arg(line);

        PC += 4;
        ORGH += 4;
        m_IR.PC_ORGH = IR.PC_ORGH;
        m_advance += 4;
        m_IR.AUGD.clear();
        line.clear();
    }

    // Do we need to generate an AUGS instruction?
    if (m_IR.AUGS.isValid()) {
        p2_LONG value = m_IR.AUGS.value<p2_LONG>() & AUG_MASK;
        P2Opcode IR(p2_AUGS, p2_PC_ORGH_t(PC, ORGH));
        IR.u.opcode |= value >> AUG_SHIFT;

        output += QString("%1 %2 [%3] %4")
                 .arg(m_lineno, -6)
                 .arg(PC, 6, 16, QChar('0'))
                 .arg(IR.u.opcode, 8, 16, QChar('0'))
                 .arg(line);
        PC += 4;
        ORGH += 4;
        m_IR.PC_ORGH = IR.PC_ORGH;
        m_advance += 4;
        m_IR.AUGS.clear();
        line.clear();
    }

    output += QString("%1 %2 [%3] %4")
             .arg(m_lineno, -6)
             .arg(PC, 6, 16, QChar('0'))
             .arg(m_IR.u.opcode, 8, 16, QChar('0'))
             .arg(line);

    if (wr_mem && m_ORGH < MEM_SIZE) {
        MEM.LONGS[m_ORGH/4] = m_IR.u.opcode;
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
    m_IR.as_IR = false;
    m_hash_OPC.insert(m_lineno, m_IR);
    output = QString("%1 %2 <%3> %4")
             .arg(m_lineno, -6)
             .arg(m_ORG, 6, 16, QChar('0'))
             .arg(m_IR.u.opcode, 8, 16, QChar('0'))
             .arg(m_line);
    return output;
}

/**
 * @brief Comment or non code generating instruction
 * @return formatted string
 */
QString P2Asm::results_comment()
{
    QString output;
    output = QString("%1 %2 -%3- %4")
             .arg(m_lineno, -6)
             .arg(m_ORG, 6, 16, QChar('0'))
             .arg(QStringLiteral("--------"))
             .arg(m_line);
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
    const p2_BYTES& bytes = m_data.to_bytes();
    const p2_LONG start = m_ORG;
    p2_LONG offset = m_ORG;
    p2_LONG data = 0;
    p2_LONG mask = 0;

    m_IR.DATA = m_data;
    m_hash_PC.insert(m_lineno, p2_PC_ORGH_t(m_ORG, m_ORGH));
    m_hash_OPC.insert(m_lineno, m_IR);

    for (int i = 0; i < bytes.count(); i++) {
        const int shift = 8 * (offset & 3);
        data |= static_cast<p2_LONG>(bytes[i]) << shift;
        mask |= 0xffu << shift;
        if (0 == (++offset & 3)) {
            // another p2_LONG to output
            output += QString("%1 %2 {%3} %4")
                     .arg(m_lineno, -6)
                     .arg(offset - 4u, 6, 16, QChar('0'))
                     .arg(P2Atom::format_long_mask(data, mask))
                     .arg(m_line);
            mask = 0;
        }

        if (wr_mem) {
            p2_LONG addr = m_ORGH + static_cast<uint>(i);
            if (addr < MEM_SIZE)
                MEM.BYTES[addr] = bytes[i];
        }
    }

    if (offset & 3) {
        output += QString("%1 %2 {%3} %4")
                 .arg(m_lineno, -6)
                 .arg(offset & ~3u, 6, 16, QChar('0'))
                 .arg(P2Atom::format_long_mask(data, mask))
                 .arg(m_line);
    }

    m_advance = offset - start;

    return output;
}

/**
 * @brief Store the results and append a line to the listing
 * @param opcode true, if the IR field contains an opcode
 */
void P2Asm::results()
{
    const bool binary = true;

    if (m_IR.as_IR) {
        m_listing += results_instruction(binary);
    } else if (m_data.isEmpty()) {
        if (m_words.isEmpty() || Token.is_type(m_words[0].tok(), tm_comment)) {
            m_listing += results_comment();
        } else {
            m_listing += results_assignment();
        }
    } else {
        m_listing += results_data(binary);
    }

    // Calculate next ORG and PC values by adding m_advance
    m_ORGH += m_advance;
    m_ORG += m_advance;
    if (m_ORG > m_ORG_MAX)
        m_ORG_MAX = m_ORG;

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
    while (m_idx < m_cnt && Token.is_type(m_words.value(m_idx).tok(), tm_comment))
        m_idx++;
    return m_idx < m_cnt;
}

/**
 * @brief Make an initial P2Atom based on the instruction token
 * @return P2Atom with type set up
 */
P2Atom P2Asm::make_atom()
{
    switch (m_instr) {
    case t__BYTE:
        return P2Atom(P2Atom::Byte);
    case t__WORD:
        return P2Atom(P2Atom::Word);
    case t__LONG:
        return P2Atom(P2Atom::Long);
    case t__FILE:
        return P2Atom(P2Atom::String);
    default:
        return P2Atom(P2Atom::Long);
    }
}

/**
 * @brief Convert a string of binary digits into an unsigned value
 * @param str binary digits
 * @return P2Atom value of binary digits in str
 */
P2Atom P2Asm::bin_const(const QString& str)
{
    P2Atom atom = make_atom();
    QString num = str;
    if (num.startsWith(chr_percent))
        num.remove(0, 1);
    num.remove(chr_skip_digit);
    bool ok;
    p2_QUAD quad = num.toULongLong(&ok, 2);
    if (ok)
        atom.set(quad);
    return atom;
}

/**
 * @brief Convert a string of byte indices into an unsigned value
 * @param str byte inidex digits
 * @return P2Atom value of byte indices in str
 */
P2Atom P2Asm::byt_const(const QString& str)
{
    P2Atom atom = make_atom();
    QString num = str;
    if (num.startsWith(QStringLiteral("%%")))
        num.remove(0, 2);
    num.remove(chr_skip_digit);
    bool ok;
    p2_QUAD quad = num.toULongLong(&ok, 4);
    if (ok)
        atom.set(quad);
    return atom;
}

/**
 * @brief Convert a string of octal digits into an unsigned value
 * @param str octal digits
 * @return P2Atom value of octal digits in str
 */
P2Atom P2Asm::oct_const(const QString& str)
{
    P2Atom atom = make_atom();
    QString num = str;
    num.remove(chr_skip_digit);
    bool ok;
    p2_QUAD quad = num.toULongLong(&ok, 8);
    if (ok)
        atom.set(quad);
    return atom;
}

/**
 * @brief Convert a string of decimal digits into an unsigned value
 * @param str decimal digits
 * @return P2Atom value of decimal digits in str
 */
P2Atom P2Asm::dec_const(const QString& str)
{
    P2Atom atom = make_atom();
    QString num = str;
    num.remove(chr_skip_digit);
    bool ok;
    p2_QUAD quad = num.toULongLong(&ok, 10);
    if (ok)
        atom.set(quad);
    return atom;
}

/**
 * @brief Convert a string of hexadecimal digits into an unsigned value
 * @param str hexadecimal digits
 * @return P2Atom value of hexadecimal digits in str
 */
P2Atom P2Asm::hex_const(const QString& str)
{
    P2Atom atom = make_atom();
    QString num = str;
    if (num.startsWith(chr_dollar))
        num.remove(0, 1);
    num.remove(chr_skip_digit);
    bool ok;
    p2_QUAD quad = num.toULongLong(&ok, 16);
    if (ok)
        atom.set(quad);
    return atom;
}

/**
 * @brief Convert a string of ASCII characters into an unsigned value
 * @param str ASCII characters
 * @param stop characters to stop at
 * @return P2Atom value of in string
 */
P2Atom P2Asm::str_const(const QString& str)
{
    P2Atom atom = make_atom();
    int pos = 0;
    QChar ch = str.at(pos++);
    QChar in_string = ch;
    bool escaped = false;

    for (/**/; pos < str.length(); pos++) {
        ch = str.at(pos);
        if (escaped) {
            atom.append_uint(P2Atom::Byte, static_cast<p2_BYTE>(ch.toLatin1()));
            escaped = false;
        } else if (ch == in_string) {
            pos++;
            return atom;
        } else if (ch == str_escape) {
            escaped = true;
        } else {
            atom.append_uint(P2Atom::Byte, static_cast<p2_BYTE>(ch.toLatin1()));
        }
    }

    return atom;
}

/**
 * @brief Convert a string of decimal digits into an unsigned value
 * @param str decimal digits
 * @return P2Atom value of decimal digits in str
 */
P2Atom P2Asm::real_const(const QString& str)
{
    QString num = str;
    num.remove(chr_skip_digit);
    return P2Atom(static_cast<p2_REAL>(num.toDouble()));
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
    if (m_symbols->contains(symbol)) {
        m_symbols->set_atom(symbol, atom);
    } else {
        m_symbols->insert(P2Symbol(symbol, atom));
    }
    m_symbols->add_reference(m_lineno, symbol, word);
}

/**
 * @brief Parse an atomic part of an expression
 * @return value of the atom
 */
P2Atom P2Asm::parse_atom(int level)
{
    QString symbol;
    P2Atom atom = make_atom();
    if (!skip_comments())
        return atom;

    const P2Word& word = curr_word();
    QString str = word.str();

    switch (word.tok()) {
    case t__FLOAT:
        DEBUG_EXPR(" atom float function: %s", qPrintable(str));
        next();
        atom = parse_expression(immediate_none, level+1);
        atom.set_real(P2Atom::Real, atom.to_real());
        prev();
        break;

    case t__ROUND:
        DEBUG_EXPR(" atom round function: %s", qPrintable(str));
        next();
        atom = parse_expression(immediate_none, level+1);
        atom.set_real(P2Atom::Real, static_cast<p2_QUAD>(atom.to_real() + 0.5));
        prev();
        break;

    case t__TRUNC:
        DEBUG_EXPR(" atom trunc function: %s", qPrintable(str));
        next();
        atom = parse_expression(immediate_none, level+1);
        atom.set_uint(P2Atom::Quad, static_cast<p2_QUAD>(atom.to_real()));
        prev();
        break;

    case t__LPAREN:
        // precedence 0
        DEBUG_EXPR(" atom lparen: %s", qPrintable(str));
        next();
        atom = parse_expression(immediate_none, level+1);
        prev();
        break;

    case t__RPAREN:
        DEBUG_EXPR(" atom rparen: %s", qPrintable(str));
        prev();
        break;

    case t__LBRACKET:
        // precedence 0
        DEBUG_EXPR(" atom lbracket: %s", qPrintable(str));
        next();
        atom = parse_expression(immediate_none, level+1);
        prev();
        break;

    case t__RBRACKET:
        DEBUG_EXPR(" atom rbracket: %s", qPrintable(str));
        prev();
        break;

    case t__RELATIVE:
    case t__RELATIVE_HUB:
        DEBUG_EXPR(" atom relative: %s", qPrintable(str));
        next();
        atom = parse_expression(immediate_none, level+1);
        prev();
        break;

    case t_locsym:
        {
            symbol = find_locsym(m_section, str);
            if (m_symbols->contains(symbol)) {
                P2Symbol sym = m_symbols->symbol(symbol);
                atom = sym.value<P2Atom>();
                DEBUG_EXPR(" atom found locsym: %s = %u", qPrintable(symbol), atom.to_long());
                m_symbols->add_reference(m_lineno, symbol, word);
            } else {
                DEBUG_EXPR(" atom undefined locsym: %s", qPrintable(symbol));
            }
        }
        break;

    case t_symbol:
        {
            symbol = find_symbol(m_section, str, true);
            if (m_symbols->contains(symbol)) {
                P2Symbol sym = m_symbols->symbol(symbol);
                atom = sym.value<P2Atom>();
                atom.set_type(P2Atom::PC);
                sym.set_atom(atom);
                DEBUG_EXPR(" atom found symbol: %s = %u", qPrintable(symbol), atom.to_long());
                m_symbols->add_reference(m_lineno, symbol, word);
            } else {
                DEBUG_EXPR(" atom undefined symbol: %s", qPrintable(symbol));
            }
        }
        break;

    case t_bin_const:
        DEBUG_EXPR(" atom bin const: %s", qPrintable(str));
        atom = bin_const(str);
        add_const_symbol(p2_prefix_bin_const, word, atom);
        break;

    case t_byt_const:
        DEBUG_EXPR(" atom byt const: %s", qPrintable(str));
        atom = byt_const(str);
        add_const_symbol(p2_prefix_byt_const, word, atom);
        break;

    case t_oct_const:
        DEBUG_EXPR(" atom oct const: %s", qPrintable(str));
        atom = oct_const(str);
        add_const_symbol(p2_prefix_oct_const, word, atom);
        break;

    case t_dec_const:
        DEBUG_EXPR(" atom dec const: %s", qPrintable(str));
        atom = dec_const(str);
        add_const_symbol(p2_prefix_dec_const, word, atom);
        break;

    case t_hex_const:
        DEBUG_EXPR(" atom hex const: %s", qPrintable(str));
        atom = hex_const(str);
        add_const_symbol(p2_prefix_hex_const, word, atom);
        break;

    case t_str_const:
        DEBUG_EXPR(" atom str const: %s", qPrintable(str));
        atom = str_const(str);
        add_const_symbol(p2_prefix_str_const, word, atom);
        break;

    case t_real_const:
        DEBUG_EXPR(" atom real const: %s", qPrintable(str));
        atom = real_const(str);
        add_const_symbol(p2_prefix_real_const, word, atom);
        break;

    case t_DIRA:
        DEBUG_EXPR(" atom DIRA: %s", qPrintable(str));
        atom.set(P2Atom::Long, offs_DIRA);
        add_const_symbol(p2_prefix_lut_const, word, atom);
        break;

    case t_DIRB:
        DEBUG_EXPR(" atom DIRB: %s", qPrintable(str));
        atom.set(P2Atom::Long, offs_DIRB);
        add_const_symbol(p2_prefix_lut_const, word, atom);
        break;

    case t_INA:
        DEBUG_EXPR(" atom INA: %s", qPrintable(str));
        atom.set(P2Atom::Long, offs_INA);
        add_const_symbol(p2_prefix_lut_const, word, atom);
        break;

    case t_INB:
        DEBUG_EXPR(" atom INB: %s", qPrintable(str));
        atom.set(P2Atom::Long, offs_INB);
        add_const_symbol(p2_prefix_lut_const, word, atom);
        break;

    case t_OUTA:
        DEBUG_EXPR(" atom OUTA: %s", qPrintable(str));
        atom.set(P2Atom::Long, offs_OUTA);
        add_const_symbol(p2_prefix_lut_const, word, atom);
        break;

    case t_OUTB:
        DEBUG_EXPR(" atom OUTB: %s", qPrintable(str));
        atom.set(P2Atom::Long, offs_OUTB);
        add_const_symbol(p2_prefix_lut_const, word, atom);
        break;

    case t_PA:
        DEBUG_EXPR(" atom PA: %s", qPrintable(str));
        atom.set(P2Atom::Long, offs_PA);
        add_const_symbol(p2_prefix_lut_const, word, atom);
        break;

    case t_PB:
        DEBUG_EXPR(" atom PB: %s", qPrintable(str));
        atom.set(P2Atom::Long, offs_PB);
        add_const_symbol(p2_prefix_lut_const, word, atom);
        break;

    case t_PTRA:
    case t_PTRA_postinc:
    case t_PTRA_postdec:
    case t_PTRA_preinc:
    case t_PTRA_predec:
        DEBUG_EXPR(" atom PTRA: %s", qPrintable(str));
        atom.set(P2Atom::Long, offs_PTRA);
        add_const_symbol(p2_prefix_lut_const, word, atom);
        break;

    case t_PTRB:
    case t_PTRB_postinc:
    case t_PTRB_postdec:
    case t_PTRB_preinc:
    case t_PTRB_predec:
        DEBUG_EXPR(" atom PTRB: %s", qPrintable(str));
        atom.set(P2Atom::Long, offs_PTRB);
        add_const_symbol(p2_prefix_lut_const, word, atom);
        break;

    case t__DOLLAR:
        DEBUG_EXPR(" atom current PC: %s", qPrintable(str));
        atom.set(P2Atom::Long, m_ORG);
        break;

    default:
        DEBUG_EXPR(" not atomic: %s (%s)", qPrintable(str), qPrintable(Token.type_names(word.tok()).join(QStringLiteral(", "))));
        prev();
    }

    next();

    return atom;
}

/**
 * @brief Parse an expression of primary operations
 * @param str string to parse
 * @return P2Atom containing the result
 */
P2Atom P2Asm::parse_primary(int level)
{
    P2Atom atom = make_atom();
    if (!skip_comments())
        return atom;

    p2_LONG inc_by = 0;
    p2_LONG dec_by = 0;

    p2_token_e op = curr_tok();
    while (Token.is_type(op, tm_primary)) {

        switch (op) {
        case t__INC:    // ++
            DEBUG_EXPR(" primary inc: %s", qPrintable(curr_str()));
            inc_by++;
            break;

        case t__DEC:    // --
            DEBUG_EXPR(" primary dec: %s", qPrintable(curr_str()));
            dec_by--;
            break;

        default:
            Q_ASSERT_X(false, "primary", "Invalid op");
            return atom;
        }

        if (!skip_comments())
            return atom;

        op = curr_tok();
    }

    atom = parse_atom(level);

    // apply primary ops
    atom.unary_dec(dec_by);
    atom.unary_inc(inc_by);

    return atom;
}

/**
 * @brief Parse an expression of unary operations
 * @param str string to parse
 * @return P2Atom containing the result
 */
P2Atom P2Asm::parse_unary(int level)
{
    P2Atom atom = make_atom();
    if (!skip_comments())
        return atom;

    bool do_logical_not = false;
    bool do_complement2 = false;
    bool do_complement1 = false;

    p2_token_e op = curr_tok();
    while (Token.is_type(op, tm_unary)) {

        switch (op) {
        case t__NEG:
            DEBUG_EXPR(" unary neg: %s", qPrintable(curr_str()));
            do_logical_not = !do_logical_not;
            next();
            break;

        case t__NOT:
            DEBUG_EXPR(" unary not: %s", qPrintable(curr_str()));
            do_complement1 = !do_complement1;
            next();
            break;

        case t__MINUS:
            DEBUG_EXPR(" unary minus: %s", qPrintable(curr_str()));
            do_complement2 = !do_complement2;
            next();
            break;

        case t__PLUS:
            DEBUG_EXPR(" unary plus: %s", qPrintable(curr_str()));
            next();
            break;

        default:
            Q_ASSERT_X(false, "shiftops", "Invalid op");
            return parse_primary(level);
        }

        if (!skip_comments())
            return atom;

        op = curr_tok();
    }

    atom = parse_primary(level);

    // apply unary ops
    atom.complement1(do_complement1);
    atom.complement2(do_complement2);
    atom.logical_not(do_logical_not);

    return atom;
}

/**
 * @brief Parse an expression of multiply operations
 * @param str string to parse
 * @return P2Atom containing the result
 */
P2Atom P2Asm::parse_mulops(int level)
{
    P2Atom lvalue = parse_unary(level);
    P2Atom rvalue;
    if (!skip_comments())
        return lvalue;

    p2_token_e op = curr_tok();
    while (Token.is_type(op, tm_mulop)) {

        if (!next()) {
            DEBUG_EXPR(" mssing rvalue (%s)", "mulops");
            m_errors += tr("End of line in expression (%1)")
                      .arg(tr("mulops"));
            emit Error(m_pass, m_lineno, m_errors.last());
            break;
        }

        rvalue = parse_unary(level);
        if (!rvalue.isValid()) {
            DEBUG_EXPR(" invalid rvalue: %s", qPrintable(curr_str()));
            m_errors += tr("Invalid character in expression (%1): %2")
                        .arg(tr("mulops"))
                        .arg(curr_str());
            emit Error(m_pass, m_lineno, m_errors.last());
            break;
        }

        switch (op) {
        case t__MUL:
            DEBUG_EXPR(" mulop mul: %u %s %u", lvalue.to_long(), qPrintable(curr_str()), rvalue.to_long());
            lvalue *= rvalue;
            break;
        case t__DIV:
            DEBUG_EXPR(" mulop div: %u %s %u", lvalue.to_long(), qPrintable(curr_str()), rvalue.to_long());
            lvalue /= rvalue;
            break;
        case t__MOD:
            DEBUG_EXPR(" mulop mod: %u %s %u", lvalue.to_long(), qPrintable(curr_str()), rvalue.to_long());
            lvalue %= rvalue;
            break;
        default:
            Q_ASSERT_X(false, "mulops", "invalid op");
            return lvalue;
        }

        if (!skip_comments())
            break;

        op = curr_tok();
    }

    return lvalue;
}

/**
 * @brief Parse an expression of shift operations
 * @param str string to parse
 * @return P2Atom containing the result
 */
P2Atom P2Asm::parse_shiftops(int level)
{
    P2Atom lvalue = parse_mulops(level);
    P2Atom rvalue;
    if (!skip_comments())
        return lvalue;

    p2_token_e op = curr_tok();
    while (Token.is_type(op, tm_shiftop)) {

        if (!next()) {
            DEBUG_EXPR(" mssing rvalue (%s)", "shiftops");
            m_errors += tr("End of line in expression (%1)")
                      .arg(tr("shiftops"));
            emit Error(m_pass, m_lineno, m_errors.last());
            break;
        }

        rvalue = parse_mulops(level);
        if (!rvalue.isValid()) {
            DEBUG_EXPR(" invalid rvalue: %s", qPrintable(curr_str()));
            m_errors += tr("Invalid character in expression (%1): %2")
                        .arg(tr("shiftops"))
                        .arg(curr_str());
            emit Error(m_pass, m_lineno, m_errors.last());
            break;
        }

        switch (op) {
        case t__SHL:
            DEBUG_EXPR(" shiftop shl: %u %s %u", lvalue.to_long(), qPrintable(curr_str()), rvalue.to_long());
            lvalue <<= rvalue;
            break;
        case t__SHR:
            DEBUG_EXPR(" shiftop shr: %u %s %u", lvalue.to_long(), qPrintable(curr_str()), rvalue.to_long());
            lvalue >>= rvalue;
            break;
        default:
            Q_ASSERT_X(false, "shiftops", "Invalid op");
            return lvalue;
        }

        if (!skip_comments())
            break;

        op = curr_tok();
    }
    return lvalue;
}

/**
 * @brief Parse an expression of add/subtract operations
 * @param str string to parse
 * @return P2Atom containing the result
 */
P2Atom P2Asm::parse_addops(int level)
{
    P2Atom lvalue = parse_shiftops(level);
    P2Atom rvalue;
    if (!skip_comments())
        return lvalue;

    p2_token_e op = curr_tok();
    while (Token.is_type(op, tm_addop)) {

        if (!next()) {
            DEBUG_EXPR(" mssing rvalue (%s)", "addops");
            m_errors += tr("End of line in expression (%1)")
                      .arg(tr("addops"));
            emit Error(m_pass, m_lineno, m_errors.last());
            break;
        }

        rvalue = parse_shiftops(level);
        if (!rvalue.isValid()) {
            DEBUG_EXPR(" invalid rvalue %s", qPrintable(curr_str()));
            m_errors += tr("Invalid character in expression (%1): %2")
                        .arg(tr("addops"))
                        .arg(curr_str());
            emit Error(m_pass, m_lineno, m_errors.last());
            break;
        }

        switch (op) {
        case t__PLUS:
            DEBUG_EXPR(" addop add: %u %s %u", lvalue.to_long(), qPrintable(curr_str()), rvalue.to_long());
            lvalue += rvalue;
            break;
        case t__MINUS:
            DEBUG_EXPR(" addop sub: %u %s %u", lvalue.to_long(), qPrintable(curr_str()), rvalue.to_long());
            lvalue -= rvalue;
            break;
        default:
            Q_ASSERT_X(false, "addops", "Invalid op");
            return lvalue;
        }

        if (!skip_comments())
            break;

        op = curr_tok();
    }
    return lvalue;
}

/**
 * @brief Parse an expression of binary operations
 * @param str string to parse
 * @return P2Atom containing the result
 */
P2Atom P2Asm::parse_binops(int level)
{
    P2Atom lvalue = parse_addops(level);
    P2Atom rvalue;

    if (!skip_comments())
        return lvalue;

    p2_token_e op = curr_tok();
    while (Token.is_type(op, tm_binop)) {
        if (!next()) {
            DEBUG_EXPR(" mssing rvalue (%s)", "binops");
            m_errors += tr("End of line in expression (%1)")
                      .arg(tr("binops"));
            emit Error(m_pass, m_lineno, m_errors.last());
            break;
        }

        P2Atom rvalue = parse_addops(level);
        if (!rvalue.isValid()) {
            DEBUG_EXPR(" invalid rvalue: %s", qPrintable(curr_str()));
            m_errors += tr("Invalid character in expression (%1): %2")
                        .arg(tr("binops"))
                        .arg(curr_str());
            emit Error(m_pass, m_lineno, m_errors.last());
            break;
        }

        switch (op) {
        case t__AND:
            DEBUG_EXPR(" binop and: %u %s %u", lvalue.to_long(), qPrintable(curr_str()), rvalue.to_long());
            lvalue &= rvalue;
            break;
        case t__XOR:
            DEBUG_EXPR(" binop xor: %u %s %u", lvalue.to_long(), qPrintable(curr_str()), rvalue.to_long());
            lvalue ^= rvalue;
            break;
        case t__OR:
            DEBUG_EXPR(" binop or: %u %s %u", lvalue.to_long(), qPrintable(curr_str()), rvalue.to_long());
            lvalue |= rvalue;
            break;
        case t__REV:
            DEBUG_EXPR(" binop or: %u %s %u", lvalue.to_long(), qPrintable(curr_str()), rvalue.to_long());
            // lvalue.reverse(rvalue);
            break;
        default:
            Q_ASSERT_X(false, "binops", "Invalid op");
            return lvalue;
        }

        if (!skip_comments())
            return lvalue;

        op = curr_tok();
    }
    return lvalue;
}

/**
 * @brief Check for a relative prefix (@)
 * @return P2Atom with a possibly PC relative value
 */
P2Atom P2Asm::parse_relative(bool& rel, int level)
{
    P2Atom atom = make_atom();
    if (!skip_comments())
        return atom;

    p2_token_e tok = curr_tok();

    while (Token.is_type(tok, tm_relative)) {
        switch (tok) {
        case t__RELATIVE:
            DEBUG_EXPR(" relative: %s", qPrintable(curr_str()));
            rel = true;
            next();
            break;
        case t__RELATIVE_HUB:
            DEBUG_EXPR(" relative HUB: %s", qPrintable(curr_str()));
            rel = true;
            next();
            break;
        default:
            Q_ASSERT_X(false, "binops", "Invalid op");
            return parse_binops(level);
        }

        if (!skip_comments())
            return atom;

        tok = curr_tok();
    }

    return parse_binops(level);
}

/**
 * @brief Evaluate an expression
 * @param imm_to put immediate flag into: -1=nowhere (default), 0=imm, 1=wz, 2=wc
 * @return QVariant with the value of the expression
 */
P2Atom P2Asm::parse_expression(imm_to_e imm_to, int level)
{
    P2Atom atom = make_atom();

    DEBUG_EXPR("»»»» %d", level);

    if (!skip_comments())
        return atom;

    bool imm = false;
    bool rel = false;
    p2_token_e tok = curr_tok();

    while (Token.is_type(tok, tm_immediate)) {
        switch (tok) {
        case t__IMMEDIATE:
            DEBUG_EXPR(" expr immediate: %s", qPrintable(curr_str()));
            imm = true;
            next();
            break;
        case t__IMMEDIATE2:
            DEBUG_EXPR(" expr immediate2: %s", qPrintable(curr_str()));
            imm = true;
            next();
            break;
        default:
            break;
        }

        if (!skip_comments())
            return atom;
        tok = curr_tok();
    }

    atom = parse_relative(rel, level);

    // Relative value
    if (rel) {
        p2_LONG base = 0;
        p2_LONG addr = atom.to_long();
        if (addr < LUT_ADDR0) {
            DEBUG_EXPR(" COG offset: %u", atom.to_long());
            base = m_ORG / 4;
        } else if (addr < HUB_ADDR0) {
            DEBUG_EXPR(" LUT offset: %u", atom.to_long());
            base = m_ORG / 4;
        } else{
            base = COG_SIZE + LUT_SIZE;
            DEBUG_EXPR(" HUB offset: %u", atom.to_long());
        }

        atom.set(P2Atom::Long, addr - base);
    }


    // Set immediate flag, if specified
    switch (imm_to) {
    case immediate_im:
        DEBUG_EXPR(" expr set im: %d", imm & 1);
        m_IR.set_im(imm);
        break;
    case immediate_wz:
        DEBUG_EXPR(" expr set wz: %d", imm & 1);
        m_IR.set_wz(imm);
        break;
    case immediate_wc:
        DEBUG_EXPR(" expr set wc: %d", imm & 1);
        m_IR.set_wc(imm);
        break;
    default:
        if (imm) {
            DEBUG_EXPR(" expr ignore immediate: %d", imm & 1);
        }
        break;
    }

    while (m_idx < m_cnt) {
        if (!skip_comments())
            break;

        tok = curr_tok();

        switch (tok) {
        case t__COMMA:
        case t__LPAREN:
        case t__LBRACKET:
            DEBUG_EXPR(" expr **** terminal: %s", qPrintable(curr_str()));
            break;

        case t__RPAREN:
        case t__RBRACKET:
            DEBUG_EXPR(" expr **** terminal: %s", qPrintable(curr_str()));
            next();
            break;

        default:
            DEBUG_EXPR(" expr **** non-terminal: %s", qPrintable(curr_str()));
            // next();
            break;
        }

        if (!Token.is_type(tok, tm_comment))
            break;
    }

    DEBUG_EXPR("«««« %d", level);
    return atom;
}

/**
 * @brief Check for the type of error of P2Opcode::set_dst() or P2Opcode::set_src()
 * @return true if no error, or false for error
 */
bool P2Asm::check_dst_src()
{
    // No error in pass 1
    if (m_pass < 2 || P2Opcode::none == m_IR.error)
        return true;

    switch (m_IR.error) {
    case P2Opcode::dst_augd_none:
        m_errors += tr("%1 constant larger than $1ff but %2.")
                    .arg(QStringLiteral("DST"))
                    .arg(tr("no imediate mode"));
        break;

    case P2Opcode::dst_augd_im:
        m_errors += tr("%1 constant larger than $1ff but %2.")
                    .arg(QStringLiteral("DST"))
                    .arg(tr("%1 is not set for %2")
                         .arg(QStringLiteral("IM"))
                         .arg(QStringLiteral("L")));
        break;

    case P2Opcode::dst_augd_wz:
        m_errors += tr("%1 constant larger than $1ff but %2.")
                    .arg(QStringLiteral("DST"))
                    .arg(tr("%1 is not set for %2")
                         .arg(QStringLiteral("WZ"))
                         .arg(QStringLiteral("L")));
        break;

    case P2Opcode::dst_augd_wc:
        m_errors += tr("%1 constant larger than $1ff but %2.")
                    .arg(QStringLiteral("DST"))
                    .arg(tr("%1 is not set for %2")
                         .arg(QStringLiteral("WC"))
                         .arg(QStringLiteral("L")));
        break;

    case P2Opcode::src_augs_none:
        m_errors += tr("%1 constant larger than $1ff but %2.")
                    .arg(QStringLiteral("SRC"))
                    .arg(tr("no imediate mode"));
        break;

    case P2Opcode::src_augs_im:
        m_errors += tr("%1 constant larger than $1ff but %2.")
                    .arg(QStringLiteral("SRC"))
                    .arg(tr("%1 is not set for %2")
                         .arg(QStringLiteral("IM"))
                         .arg(QStringLiteral("L")));
        break;

    case P2Opcode::src_augs_wz:
        m_errors += tr("%1 constant larger than $1ff but %2.")
                    .arg(QStringLiteral("SRC"))
                    .arg(tr("%1 is not set for %2")
                         .arg(QStringLiteral("WZ"))
                         .arg(QStringLiteral("L")));
        break;

    case P2Opcode::src_augs_wc:
        m_errors += tr("%1 constant larger than $1ff but %2.")
                    .arg(QStringLiteral("SRC"))
                    .arg(tr("%1 is not set for %2")
                         .arg(QStringLiteral("WC"))
                         .arg(QStringLiteral("L")));
        break;

    default:
        m_errors += tr("Missing handling for P2Opcode enum value.");
        break;
    }
    emit Error(m_pass, m_lineno, m_errors.last());
    return false;
}

/**
 * @brief Expect end of line, i.e. no more parameters
 *
 * @return true on success, or false on error
 */
bool P2Asm::end_of_line()
{
    if (skip_comments()) {
        // ignore extra parameters?
        m_errors += tr("Found extra parameters: %1")
                  .arg(m_source[m_lineno].mid(m_words.value(m_idx).pos()));
        emit Error(m_pass, m_lineno, m_errors.last());
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
    if (!skip_comments()) {
        m_errors += tr("Expected %1 but found %2.")
                  .arg(Token.string(t__COMMA))
                  .arg(tr("end of line"));
        emit Error(m_pass, m_lineno, m_errors.last());
        return false;
    }
    if (t__COMMA != m_words.value(m_idx).tok()) {
        m_errors += tr("Expected %1 but found %2.")
                  .arg(Token.string(t__COMMA))
                  .arg(Token.string(m_words.value(m_idx).tok()));
        emit Error(m_pass, m_lineno, m_errors.last());
        return false;
    }
    next();
    return true;
}

/**
 * @brief An optioncal comma is skipped
 *
 */
void P2Asm::optional_comma()
{
    if (!skip_comments())
        return;
    if (t__COMMA != m_words.value(m_idx).tok())
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
        p2_token_e tok = m_words.value(m_idx).tok();
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
        case t__COMMA:
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
    if (skip_comments()) {
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
    if (skip_comments()) {
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
    P2Atom dst = parse_expression();
    if (!m_IR.set_dst(dst))
        check_dst_src();

    if (!parse_comma())
        return false;

    P2Atom src = parse_expression(immediate_im);
    if (!m_IR.set_src(src, immediate_im))
        check_dst_src();

    return end_of_line();
}

/**
 * @brief Expect parameters for D and optional WC, WZ, or WCZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_D_WCZ()
{
    P2Atom dst = parse_expression();
    if (!m_IR.set_dst(dst))
        check_dst_src();

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
    P2Atom dst = parse_expression();
    if (!m_IR.set_dst(dst))
        check_dst_src();
    return end_of_line();
}

/**
 * @brief Expect parameters for {#}D setting WZ for immediate
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_WZ_D()
{
    P2Atom dst = parse_expression(immediate_wz);
    if (!m_IR.set_dst(dst, immediate_wz))
        check_dst_src();
    return end_of_line();
}

/**
 * @brief Expect parameters for {#}D
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_IM_D()
{
    P2Atom dst = parse_expression(immediate_im);
    if (!m_IR.set_dst(dst, immediate_im))
        check_dst_src();
    return end_of_line();
}

/**
 * @brief Expect parameters for {#}D, and optional WC, WZ, or WCZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_IM_D_WCZ()
{
    P2Atom dst = parse_expression(immediate_im);
    if (!m_IR.set_dst(dst, immediate_im))
        check_dst_src();
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
    P2Atom dst = parse_expression(immediate_im);
    if (!m_IR.set_dst(dst, immediate_im))
        check_dst_src();

    optional_WC();
    return end_of_line();
}

/**
 * @brief Expect parameters for D, and {#}S, and optional WC, WZ, or WCZ
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_D_IM_S_WCZ()
{
    P2Atom dst = parse_expression();
    if (!m_IR.set_dst(dst))
        check_dst_src();

    if (!parse_comma())
        return false;

    P2Atom src = parse_expression(immediate_im);
    if (!m_IR.set_src(src, immediate_im))
        check_dst_src();

    optional_WCZ();
    return end_of_line();
}

/**
 * @brief Expect parameters for D, and {#}S, and optional WC
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_D_IM_S_WC()
{
    P2Atom dst = parse_expression();
    if (!m_IR.set_dst(dst))
        check_dst_src();

    if (!parse_comma())
        return false;

    P2Atom src = parse_expression(immediate_im);
    if (!m_IR.set_src(src, immediate_im))
        check_dst_src();

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
    P2Atom dst = parse_expression();
    if (!m_IR.set_dst(dst))
        check_dst_src();

    if (!parse_comma())
        return false;

    P2Atom src = parse_expression(immediate_im);
    if (!m_IR.set_src(src, immediate_im))
        check_dst_src();

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
    P2Atom dst = parse_expression(immediate_wz);
    if (!m_IR.set_dst(dst, immediate_wz))
        check_dst_src();

    if (!parse_comma())
        return false;

    P2Atom src = parse_expression(immediate_im);
    if (!m_IR.set_src(src, immediate_im))
        check_dst_src();

    return end_of_line();
}

/**
 * @brief Expect parameters for D, and {#}S, and #N (0 … 7)
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_D_IM_S_NNN(uint max)
{
    P2Atom dst = parse_expression();
    if (!m_IR.set_dst(dst))
        check_dst_src();

    if (!parse_comma())
        return false;

    P2Atom src = parse_expression(immediate_im);
    if (!m_IR.set_src(src, immediate_im))
        check_dst_src();

    if (parse_comma()) {
        if (m_idx < m_cnt) {
            P2Atom n = parse_expression();
            if (n.isEmpty()) {
                m_errors += tr("Expected immediate #n");
                emit Error(m_pass, m_lineno, m_errors.last());
                return false;
            }
            if (n.to_long() > max) {
                m_errors += tr("Immediate #n not in 0-%1 (%2)")
                          .arg(max)
                          .arg(n.to_long());
                emit Error(m_pass, m_lineno, m_errors.last());
                return false;
            }
            p2_LONG opcode = static_cast<p2_LONG>(n.to_long() & max) << 18;
            m_IR.u.opcode |= opcode;
        } else {
            m_errors += tr("Missing immediate #n");
            emit Error(m_pass, m_lineno, m_errors.last());
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
bool P2Asm::parse_IM_S()
{
    P2Atom src = parse_expression(immediate_im);
    if (!m_IR.set_src(src, immediate_im))
        check_dst_src();

    return end_of_line();
}

/**
 * @brief Expect parameters for {#}S, and optional WC
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_IM_S_WCZ()
{
    P2Atom src = parse_expression(immediate_im);
    if (!m_IR.set_src(src, immediate_im))
        check_dst_src();

    optional_WC();
    return end_of_line();
}


/**
 * @brief Expect parameters for #AAAAAAAAAAAA (19 bit address for CALL/CALLA/CALLB/LOC)
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_PTRx_PC_ABS()
{
    p2_token_e dst = m_words.value(m_idx).tok();
    switch (dst) {
    case t_PA:
        break;
    case t_PB:
        m_IR.set_wz();
        break;
    case t_PTRA:
        m_IR.set_wc();
        break;
    case t_PTRB:
        m_IR.set_wcz();
        break;
    default:
        m_errors += tr("Invalid pointer parameter: %1")
                  .arg(m_source[m_lineno].mid(m_words.value(m_idx).pos()));
        emit Error(m_pass, m_lineno, m_errors.last());
        return false;
    }
    next();
    if (!parse_comma())
        return false;

    P2Atom atom = parse_expression();
    p2_LONG addr = atom.to_long();
    m_IR.u.opcode |= addr & A20MASK;

    return end_of_line();
}

/**
 * @brief Expect parameters for #AAAAAAAAAAAA (19 bit address for CALL/CALLA/CALLB)
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_PC_ABS()
{
    P2Atom atom = parse_expression();
    p2_LONG addr = atom.to_long();
    m_IR.u.opcode |= addr & A20MASK;

    return end_of_line();
}

/**
 * @brief Expect parameters for #AAAAAAAAAAAA (23 bit address for AUGD/AUGS)
 *
 * @return true on success, or false on error
 */
bool P2Asm::parse_IMM23()
{
    P2Atom atom = parse_expression();
    p2_LONG addr = atom.to_long();
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
    m_IR.as_IR = false;
    P2Atom atom = parse_expression();
    atom.set_type(P2Atom::Long);
    m_symbols->set_atom(m_symbol, atom);
    m_IR.as_EQU = true;
    m_IR.u.opcode = atom.to_long();
    return end_of_line();
}

/**
 * @brief Align to next WORD operation
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_alignw()
{
    next();
    m_IR.as_IR = false;
    m_advance = (m_ORG & ~1u) + 2u - m_ORG;
    return end_of_line();
}

/**
 * @brief Align to next LONG operation
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_alignl()
{
    next();
    m_IR.as_IR = false;
    m_advance = (m_ORG & ~3u) + 4u - m_ORG;
    return end_of_line();
}

/**
 * @brief Origin operation
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_org()
{
    next();
    m_advance = 0;      // Don't advance PC
    m_IR.as_IR = false;
    P2Atom atom = parse_expression();
    p2_LONG value = atom.isEmpty() ? m_ORG_MAX : 4 * atom.to_long();
    if (value >= HUB_ADDR0) {
        m_errors += tr("COG origin exceeds limit ($%1)")
                    .arg(value, 0, 16, QChar('0'));
        emit Error(m_pass, m_lineno, m_errors.last());
        value = m_ORG_MAX;
    }
    m_ORG = value;
    m_symbols->set_atom(m_symbol, value);
    return end_of_line();
}

/**
 * @brief Origin high operation
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_orgh()
{
    next();
    m_advance = 0;      // Don't advance PC
    m_IR.as_IR = false;
    P2Atom atom = parse_expression();
    p2_LONG value = atom.isEmpty() ? HUB_ADDR0 : atom.to_long();
    if (value <= HUB_ADDR0)
        value *= 4;
    if (value >= MEM_SIZE) {
        m_errors += tr("HUB address exceeds limit ($%1)")
                    .arg(value, 0, 16, QChar('0'));
        emit Error(m_pass, m_lineno, m_errors.last());
        value = MEM_SIZE;
    }
    m_ORGH = value;
    m_symbols->set_atom(m_symbol, value);
    return end_of_line();
}

/**
 * @brief Expect an address which is checked for being < curr_pc
 * @return true on success, or false on error
 */
bool P2Asm::asm_fit()
{
    next();
    m_advance = 0;
    m_IR.as_IR = false;
    P2Atom atom = parse_expression();
    const p2_LONG fit = atom.isNull() ? HUB_ADDR0 : atom.to_long();
    const p2_LONG PC = m_ORG / 4;
    if (fit < PC) {
        m_errors += tr("Code does not fit below $%1 (origin == $%2)")
                  .arg(fit, 0, 16)
                  .arg(PC, 0, 16);
        emit Error(m_pass, m_lineno, m_errors.last());
    }
    return end_of_line();
}

/**
 * @brief Switch to data section
 * @return true on success
 */
bool P2Asm::asm_dat()
{
    next();
    m_advance = 0;
    m_IR.as_IR = false;
    m_section = sec_dat;
    return true;
}

/**
 * @brief Switch to constant section
 * @return true on success
 */
bool P2Asm::asm_con()
{
    next();
    m_advance = 0;
    m_IR.as_IR = false;
    m_section = sec_con;
    return true;
}

/**
 * @brief Switch to publics section
 * @return true on success
 */
bool P2Asm::asm_pub()
{
    next();
    m_advance = 0;
    m_IR.as_IR = false;
    m_section = sec_pub;
    return true;
}

/**
 * @brief Switch to private section
 * @return true on success
 */
bool P2Asm::asm_pri()
{
    next();
    m_advance = 0;
    m_IR.as_IR = false;
    m_section = sec_pri;
    return true;
}

/**
 * @brief Switch to variable section
 * @return true on success
 */
bool P2Asm::asm_var()
{
    next();
    m_advance = 0;
    m_IR.as_IR = false;
    m_section = sec_var;
    return true;
}

/**
 * @brief Expect one or more bytes of data
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_byte()
{
    P2Atom atom;
    QByteArray bytes;
    next();
    m_advance = 0;      // Don't advance PC, as it's done based on m_data
    m_IR.as_IR = false;

    while (m_idx < m_cnt) {
        if (t__LBRACKET == m_words.value(m_idx).tok()) {
            P2Atom atom2 = parse_expression();
            p2_LONG count = atom2.to_long();
            while (count-- > 1)
                m_data.append(atom);
        } else {
            atom = parse_expression();
            bytes = atom.to_array();
            m_data.append(bytes);
        }
        optional_comma();
    }
    if (m_data.size() > 0) {
        if (m_data.size() > 1)
            m_data.set_type(P2Atom::String);
        else
            m_data.set_type(P2Atom::Byte);
    }
    return end_of_line();
}

/**
 * @brief Expect one or more words of data
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_word()
{
    next();
    m_advance = 0;      // Don't advance PC, as it's done based on m_data
    m_IR.as_IR = false;

    while (m_idx < m_cnt) {
        P2Atom atom = parse_expression();
        m_data.append(atom);
        optional_comma();
    }
    if (m_data.size() > 0)
        m_data.set_type(P2Atom::Word);

    return end_of_line();
}

/**
 * @brief Expect one or more longs of data
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_long()
{
    P2Atom atom;
    next();
    m_advance = 0;      // Don't advance PC, as it's done based on m_data
    m_IR.as_IR = false;

    while (m_idx < m_cnt) {
        if (t__LBRACKET == m_words.value(m_idx).tok()) {
            P2Atom atom2 = parse_expression();
            p2_LONG l = atom.to_long();
            p2_LONG count = atom2.to_long();
            while (count-- > 1)
                m_data.append_uint(P2Atom::Long, l);
        } else {
            atom = parse_expression();
            p2_LONG l = atom.to_long();
            m_data.append_uint(P2Atom::Long, l);
        }
        optional_comma();
    }
    if (m_data.size() > 0)
        m_data.set_type(P2Atom::Long);

    return end_of_line();
}

/**
 * @brief Reserver a number of longs of data
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_res()
{
    next();
    m_advance = 0;      // Don't advance PC if no value is specified
    m_IR.as_IR = false;

    while (m_idx < m_cnt) {
        P2Atom atom = parse_expression();
        p2_LONG count = atom.to_long();
        m_data.append(QByteArray(4 * static_cast<int>(count), '\0'));
        optional_comma();
    }
    m_data.set_type(P2Atom::Long);

    return end_of_line();
}

/**
 * @brief Expect one or more filenames in double quotes
 *
 * @return true on success, or false on error
 */
bool P2Asm::asm_file()
{
    next();
    m_advance = 0;      // Don't advance PC, as it's done based on m_data
    m_IR.as_IR = false;

    while (m_idx < m_cnt) {
        P2Atom atom = parse_expression();
        QString filename = atom.to_string();
        QFile data(filename);
        if (data.open(QIODevice::ReadOnly)) {
            m_data.append(data.readAll());
            m_data.set_type(P2Atom::String);
            data.close();
        } else {
            m_errors += tr("Could not open file %1 for reading.")
                       .arg(filename);
            emit Error(m_pass, m_lineno, m_errors.last());
            return false;
        }
        optional_comma();
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
bool P2Asm::asm_nop()
{
    next();
    m_IR.u.opcode = 0;
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
bool P2Asm::asm_ror()
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
bool P2Asm::asm_rol()
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
bool P2Asm::asm_shr()
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
bool P2Asm::asm_shl()
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
bool P2Asm::asm_rcr()
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
bool P2Asm::asm_rcl()
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
bool P2Asm::asm_sar()
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
bool P2Asm::asm_sal()
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
bool P2Asm::asm_add()
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
bool P2Asm::asm_addx()
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
bool P2Asm::asm_adds()
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
bool P2Asm::asm_addsx()
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
bool P2Asm::asm_sub()
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
bool P2Asm::asm_subx()
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
bool P2Asm::asm_subs()
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
bool P2Asm::asm_subsx()
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
bool P2Asm::asm_cmp()
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
bool P2Asm::asm_cmpx()
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
bool P2Asm::asm_cmps()
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
bool P2Asm::asm_cmpsx()
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
bool P2Asm::asm_cmpr()
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
bool P2Asm::asm_cmpm()
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
bool P2Asm::asm_subr()
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
bool P2Asm::asm_cmpsub()
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
bool P2Asm::asm_fge()
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
bool P2Asm::asm_fle()
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
bool P2Asm::asm_fges()
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
bool P2Asm::asm_fles()
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
bool P2Asm::asm_sumc()
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
bool P2Asm::asm_sumnc()
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
bool P2Asm::asm_sumz()
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
bool P2Asm::asm_sumnz()
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
bool P2Asm::asm_testb_w()
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
bool P2Asm::asm_testbn_w()
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
bool P2Asm::asm_testb_and()
{
    next();
    m_IR.set_inst7(p2_TESTB_AND);
    return parse_D_IM_S_WCZ();
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
bool P2Asm::asm_testbn_and()
{
    next();
    m_IR.set_inst7(p2_TESTBN_AND);
    return parse_D_IM_S_WCZ();
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
bool P2Asm::asm_testb_or()
{
    next();
    m_IR.set_inst7(p2_TESTB_OR);
    return parse_D_IM_S_WCZ();
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
bool P2Asm::asm_testbn_or()
{
    next();
    m_IR.set_inst7(p2_TESTBN_OR);
    return parse_D_IM_S_WCZ();
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
bool P2Asm::asm_testb_xor()
{
    next();
    m_IR.set_inst7(p2_TESTB_XOR);
    return parse_D_IM_S_WCZ();
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
bool P2Asm::asm_testbn_xor()
{
    next();
    m_IR.set_inst7(p2_TESTBN_XOR);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Bit S[4:0] of D = 0,    C,Z = D[S[4:0]].
 *<pre>
 * EEEE 0100000 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITL    D,{#}S         {WCZ}
 *</pre>
 */
bool P2Asm::asm_bitl()
{
    next();
    m_IR.set_inst7(p2_BITL);
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
bool P2Asm::asm_bith()
{
    next();
    m_IR.set_inst7(p2_BITH);
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
bool P2Asm::asm_bitc()
{
    next();
    m_IR.set_inst7(p2_BITC);
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
bool P2Asm::asm_bitnc()
{
    next();
    m_IR.set_inst7(p2_BITNC);
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
bool P2Asm::asm_bitz()
{
    next();
    m_IR.set_inst7(p2_BITZ);
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
bool P2Asm::asm_bitnz()
{
    next();
    m_IR.set_inst7(p2_BITNZ);
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
bool P2Asm::asm_bitrnd()
{
    next();
    m_IR.set_inst7(p2_BITRND);
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
bool P2Asm::asm_bitnot()
{
    next();
    m_IR.set_inst7(p2_BITNOT);
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
bool P2Asm::asm_and()
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
bool P2Asm::asm_andn()
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
bool P2Asm::asm_or()
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
bool P2Asm::asm_xor()
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
bool P2Asm::asm_muxc()
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
bool P2Asm::asm_muxnc()
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
bool P2Asm::asm_muxz()
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
bool P2Asm::asm_muxnz()
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
bool P2Asm::asm_mov()
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
bool P2Asm::asm_not()
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
bool P2Asm::asm_abs()
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
bool P2Asm::asm_neg()
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
bool P2Asm::asm_negc()
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
bool P2Asm::asm_negnc()
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
bool P2Asm::asm_negz()
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
bool P2Asm::asm_negnz()
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
bool P2Asm::asm_incmod()
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
bool P2Asm::asm_decmod()
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
bool P2Asm::asm_zerox()
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
bool P2Asm::asm_signx()
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
bool P2Asm::asm_encod()
{
    if (commata_left() < 1)
        return asm_encod_d();
    next();
    m_IR.set_inst7(p2_ENCOD);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Get bit position of top-most '1' in S into D.
 *<pre>
 * EEEE 0111100 CZ0 DDDDDDDDD DDDDDDDDD
 *
 * ENCOD   D        {WC/WZ/WCZ}
 *
 * D = position of top '1' in S (0..31).
 * C = (S != 0).
 * Z = (result == 0).
 *</pre>
 */
bool P2Asm::asm_encod_d()
{
    next();
    m_IR.set_inst7(p2_ENCOD);
    return parse_D_WCZ();
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
bool P2Asm::asm_ones()
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
bool P2Asm::asm_test()
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
bool P2Asm::asm_testn()
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
bool P2Asm::asm_setnib()
{
    if (commata_left() < 1)
        return asm_setnib_altsn();
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
bool P2Asm::asm_setnib_altsn()
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
bool P2Asm::asm_getnib()
{
    if (commata_left() < 1)
        return asm_getnib_altgn();
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
bool P2Asm::asm_getnib_altgn()
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
bool P2Asm::asm_rolnib()
{
    if (commata_left() < 1)
        return asm_rolnib_altgn();
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
bool P2Asm::asm_rolnib_altgn()
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
bool P2Asm::asm_setbyte()
{
    if (commata_left() < 1)
        return asm_setbyte_altsb();
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
bool P2Asm::asm_setbyte_altsb()
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
bool P2Asm::asm_getbyte()
{
    if (commata_left() < 1)
        return asm_getbyte_altgb();
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
bool P2Asm::asm_getbyte_altgb()
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
bool P2Asm::asm_rolbyte()
{
    if (commata_left() < 1)
        return asm_rolbyte_altgb();
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
bool P2Asm::asm_rolbyte_altgb()
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
bool P2Asm::asm_setword()
{
    if (commata_left() < 1)
        return asm_setword_altsw();
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
bool P2Asm::asm_setword_altsw()
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
bool P2Asm::asm_getword()
{
    if (commata_left() < 1)
        return asm_getword_altgw();
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
bool P2Asm::asm_getword_altgw()
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
bool P2Asm::asm_rolword()
{
    if (commata_left() < 1)
        return asm_rolword_altgw();
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
bool P2Asm::asm_rolword_altgw()
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
bool P2Asm::asm_altsn()
{
    if (commata_left() < 1)
        return asm_altsn_d();
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
bool P2Asm::asm_altsn_d()
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
bool P2Asm::asm_altgn()
{
    if (commata_left() < 1)
        return asm_altgn_d();
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
bool P2Asm::asm_altgn_d()
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
bool P2Asm::asm_altsb()
{
    if (commata_left() < 1)
        return asm_altsb_d();
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
bool P2Asm::asm_altsb_d()
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
bool P2Asm::asm_altgb()
{
    if (commata_left() < 1)
        return asm_altgb_d();
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
bool P2Asm::asm_altgb_d()
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
bool P2Asm::asm_altsw()
{
    if (commata_left() < 1)
        return asm_altsw_d();
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
bool P2Asm::asm_altsw_d()
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
bool P2Asm::asm_altgw()
{
    if (commata_left() < 1)
        return asm_altgw_d();
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
bool P2Asm::asm_altgw_d()
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
bool P2Asm::asm_altr()
{
    if (commata_left() < 1)
        return asm_altr_d();
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
bool P2Asm::asm_altr_d()
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
bool P2Asm::asm_altd()
{
    if (commata_left() < 1)
        return asm_altd_d();
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
bool P2Asm::asm_altd_d()
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
bool P2Asm::asm_alts()
{
    if (commata_left() < 1)
        return asm_alts_d();
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
bool P2Asm::asm_alts_d()
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
bool P2Asm::asm_altb()
{
    if (commata_left() < 1)
        return asm_altb_d();
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
bool P2Asm::asm_altb_d()
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
bool P2Asm::asm_alti()
{
    if (commata_left() < 1)
        return asm_alti_d();
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
bool P2Asm::asm_alti_d()
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
bool P2Asm::asm_setr()
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
bool P2Asm::asm_setd()
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
bool P2Asm::asm_sets()
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
bool P2Asm::asm_decod()
{
    next();
    m_IR.set_inst9(p2_DECOD);
    return parse_D_IM_S();
}

/**
 * @brief Decode D[4:0] into D.
 *<pre>
 * EEEE 1001110 000 DDDDDDDDD DDDDDDDDD
 *
 * DECOD   D
 *
 * D = 1 << D[4:0].
 *</pre>
 */
bool P2Asm::asm_decod_d()
{
    next();
    m_IR.set_inst9(p2_DECOD);
    return parse_D();
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
bool P2Asm::asm_bmask()
{
    next();
    m_IR.set_inst9(p2_BMASK);
    return parse_D_IM_S();
}

/**
 * @brief Get LSB-justified bit mask of size (D[4:0] + 1) into D.
 *<pre>
 * EEEE 1001110 010 DDDDDDDDD DDDDDDDDD
 *
 * BMASK   D
 *
 * D = ($0000_0002 << D[4:0]) - 1.
 *</pre>
 */
bool P2Asm::asm_bmask_d()
{
    next();
    m_IR.set_inst9(p2_BMASK);
    return parse_D();
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
bool P2Asm::asm_crcbit()
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
bool P2Asm::asm_crcnib()
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
bool P2Asm::asm_muxnits()
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
bool P2Asm::asm_muxnibs()
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
bool P2Asm::asm_muxq()
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
bool P2Asm::asm_movbyts()
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
bool P2Asm::asm_mul()
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
bool P2Asm::asm_muls()
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
bool P2Asm::asm_sca()
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
bool P2Asm::asm_scas()
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
bool P2Asm::asm_addpix()
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
bool P2Asm::asm_mulpix()
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
bool P2Asm::asm_blnpix()
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
bool P2Asm::asm_mixpix()
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
bool P2Asm::asm_addct1()
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
bool P2Asm::asm_addct2()
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
bool P2Asm::asm_addct3()
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
bool P2Asm::asm_wmlong()
{
    next();
    m_IR.set_inst9(p2_WMLONG);
    return parse_D_IM_S();
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
bool P2Asm::asm_rqpin()
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
bool P2Asm::asm_rdpin()
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
bool P2Asm::asm_rdlut()
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
bool P2Asm::asm_rdbyte()
{
    next();
    m_IR.set_inst7(p2_RDBYTE);
    return parse_D_IM_S_WCZ();
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
bool P2Asm::asm_rdword()
{
    next();
    m_IR.set_inst7(p2_RDWORD);
    return parse_D_IM_S_WCZ();
}

/**
 * @brief Read long from hub address {#}S/PTRx into D.
 * Prior <pre>SETQ/SETQ2 invokes cog/LUT block transfer.
 *
 * EEEE 1011000 CZI DDDDDDDDD SSSSSSSSS
 *
 * RDLONG  D,{#}S/P {WC/WZ/WCZ}
 *
 * C = MSB of long.
 *</pre>
 */
bool P2Asm::asm_rdlong()
{
    next();
    m_IR.set_inst7(p2_RDLONG);
    return parse_D_IM_S_WCZ();
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
bool P2Asm::asm_popa()
{
    next();
    m_IR.set_inst7(p2_RDLONG);
    m_IR.set_im(true);
    m_IR.u.op.src = 0x15f;
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
bool P2Asm::asm_popb()
{
    next();
    m_IR.set_inst7(p2_RDLONG);
    m_IR.set_im(true);
    m_IR.u.op.src = 0x1df;
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
bool P2Asm::asm_calld()
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
bool P2Asm::asm_resi3()
{
    next();
    m_IR.set_inst7(p2_CALLD);
    m_IR.set_wc();
    m_IR.set_wz();
    m_IR.u.op.dst = offs_IJMP3;
    m_IR.u.op.src = offs_IRET3;
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
bool P2Asm::asm_resi2()
{
    next();
    m_IR.set_inst7(p2_CALLD);
    m_IR.set_wcz(true);
    m_IR.u.op.dst = offs_IJMP2;
    m_IR.u.op.src = offs_IRET2;
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
bool P2Asm::asm_resi1()
{
    next();
    m_IR.set_inst7(p2_CALLD);
    m_IR.set_wcz(true);
    m_IR.u.op.dst = offs_IJMP1;
    m_IR.u.op.src = offs_IRET1;
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
bool P2Asm::asm_resi0()
{
    next();
    m_IR.set_inst7(p2_CALLD);
    m_IR.set_wcz(true);
    m_IR.u.op.dst = offs_INA;
    m_IR.u.op.src = offs_INB;
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
bool P2Asm::asm_reti3()
{
    next();
    m_IR.set_inst7(p2_CALLD);
    m_IR.set_wcz(true);
    m_IR.u.op.dst = offs_INB;
    m_IR.u.op.src = offs_IRET3;
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
bool P2Asm::asm_reti2()
{
    next();
    m_IR.set_inst7(p2_CALLD);
    m_IR.set_wcz(true);
    m_IR.u.op.dst = offs_INB;
    m_IR.u.op.src = offs_IRET2;
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
bool P2Asm::asm_reti1()
{
    next();
    m_IR.set_inst7(p2_CALLD);
    m_IR.set_wcz(true);
    m_IR.u.op.dst = offs_INB;
    m_IR.u.op.src = offs_IRET1;
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
bool P2Asm::asm_reti0()
{
    next();
    m_IR.set_inst7(p2_CALLD);
    m_IR.set_wcz(true);
    m_IR.u.op.dst = offs_INB;
    m_IR.u.op.src = offs_INB;
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
bool P2Asm::asm_callpa()
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
bool P2Asm::asm_callpb()
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
bool P2Asm::asm_djz()
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
bool P2Asm::asm_djnz()
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
bool P2Asm::asm_djf()
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
bool P2Asm::asm_djnf()
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
bool P2Asm::asm_ijz()
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
bool P2Asm::asm_ijnz()
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
bool P2Asm::asm_tjz()
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
bool P2Asm::asm_tjnz()
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
bool P2Asm::asm_tjf()
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
bool P2Asm::asm_tjnf()
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
bool P2Asm::asm_tjs()
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
bool P2Asm::asm_tjns()
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
bool P2Asm::asm_tjv()
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
bool P2Asm::asm_jint()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JINT;
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
bool P2Asm::asm_jct1()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JCT1;
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
bool P2Asm::asm_jct2()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JCT2;
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
bool P2Asm::asm_jct3()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JCT3;
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
bool P2Asm::asm_jse1()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JSE1;
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
bool P2Asm::asm_jse2()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JSE2;
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
bool P2Asm::asm_jse3()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JSE3;
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
bool P2Asm::asm_jse4()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JSE4;
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
bool P2Asm::asm_jpat()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JPAT;
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
bool P2Asm::asm_jfbw()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JFBW;
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
bool P2Asm::asm_jxmt()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JXMT;
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
bool P2Asm::asm_jxfi()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JXFI;
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
bool P2Asm::asm_jxro()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JXRO;
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
bool P2Asm::asm_jxrl()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JXRL;
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
bool P2Asm::asm_jatn()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JATN;
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
bool P2Asm::asm_jqmt()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JQMT;
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
bool P2Asm::asm_jnint()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JNINT;
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
bool P2Asm::asm_jnct1()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JNCT1;
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
bool P2Asm::asm_jnct2()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JNCT2;
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
bool P2Asm::asm_jnct3()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JNCT3;
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
bool P2Asm::asm_jnse1()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JNSE1;
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
bool P2Asm::asm_jnse2()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JNSE2;
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
bool P2Asm::asm_jnse3()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JNSE3;
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
bool P2Asm::asm_jnse4()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JNSE4;
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
bool P2Asm::asm_jnpat()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JNPAT;
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
bool P2Asm::asm_jnfbw()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JNFBW;
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
bool P2Asm::asm_jnxmt()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JNXMT;
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
bool P2Asm::asm_jnxfi()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JNXFI;
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
bool P2Asm::asm_jnxro()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JNXRO;
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
bool P2Asm::asm_jnxrl()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JNXRL;
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
bool P2Asm::asm_jnatn()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JNATN;
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
bool P2Asm::asm_jnqmt()
{
    next();
    m_IR.set_inst9(p2_OPDST);
    m_IR.u.op.dst = p2_OPDST_JNQMT;
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
    m_IR.set_inst9(p2_1011110_10);
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
bool P2Asm::asm_setpat()
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
bool P2Asm::asm_wrpin()
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
bool P2Asm::asm_akpin()
{
    next();
    m_IR.set_inst8(p2_WRPIN);
    m_IR.u.op8.wz = true;
    m_IR.u.op.dst = 1;
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
bool P2Asm::asm_wxpin()
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
bool P2Asm::asm_wypin()
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
bool P2Asm::asm_wrlut()
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
bool P2Asm::asm_wrbyte()
{
    next();
    m_IR.set_inst8(p2_WRBYTE);
    return parse_WZ_D_IM_S();
}

/**
 * @brief Write word in D[15:0] to hub address {#}S/PTRx.
 *<pre>
 * EEEE 1100010 1LI DDDDDDDDD SSSSSSSSS
 *
 * WRWORD  {#}D,{#}S/P
 *</pre>
 */
bool P2Asm::asm_wrword()
{
    next();
    m_IR.set_inst8(p2_WRWORD);
    return parse_WZ_D_IM_S();
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
bool P2Asm::asm_wrlong()
{
    next();
    m_IR.set_inst8(p2_WRLONG);
    return parse_WZ_D_IM_S();
}

/**
 * @brief Write long in D[31:0] to hub address PTRA++.
 *<pre>
 * EEEE 1100011 0L1 DDDDDDDDD 101100001
 *
 * PUSHA   {#}D
 *</pre>
 */
bool P2Asm::asm_pusha()
{
    next();
    m_IR.set_inst8(p2_WRLONG);
    m_IR.u.op8.im = true;
    m_IR.u.op8.src = 0x161;
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
bool P2Asm::asm_pushb()
{
    next();
    m_IR.set_inst8(p2_WRLONG);
    m_IR.u.op8.im = true;
    m_IR.u.op8.src = 0x1e1;
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
bool P2Asm::asm_rdfast()
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
bool P2Asm::asm_wrfast()
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
bool P2Asm::asm_fblock()
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
bool P2Asm::asm_xinit()
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
bool P2Asm::asm_xstop()
{
    next();
    m_IR.set_inst8(p2_XINIT);
    m_IR.u.op8.wz = true;
    m_IR.u.op8.im = true;
    m_IR.u.op8.dst = 0x000;
    m_IR.u.op8.src = 0x000;
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
bool P2Asm::asm_xzero()
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
bool P2Asm::asm_xcont()
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
bool P2Asm::asm_rep()
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
bool P2Asm::asm_coginit()
{
    next();
    m_IR.set_inst7(p2_COGINIT);
    return parse_WZ_D_IM_S();
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
bool P2Asm::asm_qmul()
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
bool P2Asm::asm_qdiv()
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
bool P2Asm::asm_qfrac()
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
bool P2Asm::asm_qsqrt()
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
bool P2Asm::asm_qrotate()
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
bool P2Asm::asm_qvector()
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
bool P2Asm::asm_hubset()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_HUBSET;
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
bool P2Asm::asm_cogid()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_COGID;
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
bool P2Asm::asm_cogstop()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_COGSTOP;
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
bool P2Asm::asm_locknew()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_LOCKNEW;
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
bool P2Asm::asm_lockret()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_LOCKRET;
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
bool P2Asm::asm_locktry()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_LOCKTRY;
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
bool P2Asm::asm_lockrel()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_LOCKREL;
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
bool P2Asm::asm_qlog()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_QLOG;
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
bool P2Asm::asm_qexp()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_QEXP;
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
bool P2Asm::asm_rfbyte()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_RFBYTE;
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
bool P2Asm::asm_rfword()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_RFWORD;
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
bool P2Asm::asm_rflong()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_RFLONG;
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
bool P2Asm::asm_rfvar()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_RFVAR;
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
bool P2Asm::asm_rfvars()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_RFVARS;
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
bool P2Asm::asm_wfbyte()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_WFBYTE;
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
bool P2Asm::asm_wfword()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_WFWORD;
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
bool P2Asm::asm_wflong()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_WFLONG;
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
bool P2Asm::asm_getqx()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_GETQX;
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
bool P2Asm::asm_getqy()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_GETQY;
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
bool P2Asm::asm_getct()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_GETCT;
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
bool P2Asm::asm_getrnd()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_GETRND;
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
bool P2Asm::asm_getrnd_cz()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_GETRND;
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
bool P2Asm::asm_setdacs()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SETDACS;
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
bool P2Asm::asm_setxfrq()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SETXFRQ;
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
bool P2Asm::asm_getxacc()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_GETXACC;
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
bool P2Asm::asm_waitx()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_WAITX;
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
bool P2Asm::asm_setse1()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SETSE1;
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
bool P2Asm::asm_setse2()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SETSE2;
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
bool P2Asm::asm_setse3()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SETSE3;
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
bool P2Asm::asm_setse4()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SETSE4;
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
bool P2Asm::asm_pollint()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_POLLINT;
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
bool P2Asm::asm_pollct1()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_POLLCT1;
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
bool P2Asm::asm_pollct2()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_POLLCT2;
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
bool P2Asm::asm_pollct3()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_POLLCT3;
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
bool P2Asm::asm_pollse1()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_POLLSE1;
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
bool P2Asm::asm_pollse2()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_POLLSE2;
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
bool P2Asm::asm_pollse3()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_POLLSE3;
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
bool P2Asm::asm_pollse4()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_POLLSE4;
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
bool P2Asm::asm_pollpat()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_POLLPAT;
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
bool P2Asm::asm_pollfbw()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_POLLFBW;
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
bool P2Asm::asm_pollxmt()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_POLLXMT;
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
bool P2Asm::asm_pollxfi()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_POLLXFI;
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
bool P2Asm::asm_pollxro()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_POLLXRO;
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
bool P2Asm::asm_pollxrl()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_POLLXRL;
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
bool P2Asm::asm_pollatn()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_POLLATN;
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
bool P2Asm::asm_pollqmt()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_POLLQMT;
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
bool P2Asm::asm_waitint()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_WAITINT;
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
bool P2Asm::asm_waitct1()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_WAITCT1;
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
bool P2Asm::asm_waitct2()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_WAITCT2;
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
bool P2Asm::asm_waitct3()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_WAITCT3;
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
bool P2Asm::asm_waitse1()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_WAITSE1;
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
bool P2Asm::asm_waitse2()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_WAITSE2;
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
bool P2Asm::asm_waitse3()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_WAITSE3;
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
bool P2Asm::asm_waitse4()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_WAITSE4;
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
bool P2Asm::asm_waitpat()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_WAITPAT;
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
bool P2Asm::asm_waitfbw()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_WAITFBW;
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
bool P2Asm::asm_waitxmt()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_WAITXMT;
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
bool P2Asm::asm_waitxfi()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_WAITXFI;
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
bool P2Asm::asm_waitxro()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_WAITXRO;
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
bool P2Asm::asm_waitxrl()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_WAITXRL;
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
bool P2Asm::asm_waitatn()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_WAITATN;
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
bool P2Asm::asm_allowi()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_ALLOWI;
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
bool P2Asm::asm_stalli()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_STALLI;
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
bool P2Asm::asm_trgint1()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_TRGINT1;
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
bool P2Asm::asm_trgint2()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_TRGINT2;
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
bool P2Asm::asm_trgint3()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_TRGINT3;
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
bool P2Asm::asm_nixint1()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_NIXINT1;
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
bool P2Asm::asm_nixint2()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_NIXINT2;
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
bool P2Asm::asm_nixint3()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_X24;
    m_IR.u.op.dst = p2_OPX24_NIXINT3;
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
bool P2Asm::asm_setint1()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SETINT1;
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
bool P2Asm::asm_setint2()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SETINT2;
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
bool P2Asm::asm_setint3()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SETINT3;
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
bool P2Asm::asm_setq()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SETQ;
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
bool P2Asm::asm_setq2()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SETQ2;
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
bool P2Asm::asm_push()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_PUSH;
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
bool P2Asm::asm_pop()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_POP;
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
bool P2Asm::asm_jmp()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_JMP;
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
bool P2Asm::asm_call()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_CALL_RET;
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
bool P2Asm::asm_ret()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.set_im(true);
    m_IR.u.op.dst = 0x000;
    m_IR.u.op.src = p2_OPSRC_CALL_RET;
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
bool P2Asm::asm_calla()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_CALLA_RETA;
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
bool P2Asm::asm_reta()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.set_im(true);
    m_IR.u.op.dst = 0x000;
    m_IR.u.op.src = p2_OPSRC_CALLA_RETA;
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
bool P2Asm::asm_callb()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_CALLB_RETB;
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
bool P2Asm::asm_retb()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.set_im(true);
    m_IR.u.op.dst = 0x000;
    m_IR.u.op.src = p2_OPSRC_CALLB_RETB;
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
bool P2Asm::asm_jmprel()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_JMPREL;
    return parse_IM_D();
}

/**
 * @brief Skip instructions per D.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000110001
 *
 * SKIP    {#}D
 *
 * Subsequent instructions 0.
 * 31 get cancelled for each '1' bit in D[0].
 * D[31].
 *</pre>
 */
bool P2Asm::asm_skip()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SKIP;
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
bool P2Asm::asm_skipf()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SKIPF;
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
bool P2Asm::asm_execf()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_EXECF;
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
bool P2Asm::asm_getptr()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_GETPTR;
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
bool P2Asm::asm_getbrk()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.set_im(false);
    m_IR.u.op.src = p2_OPSRC_COGBRK;
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
bool P2Asm::asm_cogbrk()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_COGBRK;
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
bool P2Asm::asm_brk()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_BRK;
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
bool P2Asm::asm_setluts()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SETLUTS;
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
bool P2Asm::asm_setcy()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SETCY;
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
bool P2Asm::asm_setci()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SETCI;
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
bool P2Asm::asm_setcq()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SETCQ;
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
bool P2Asm::asm_setcfrq()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SETCFRQ;
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
bool P2Asm::asm_setcmod()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SETCMOD;
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
bool P2Asm::asm_setpiv()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SETPIV;
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
bool P2Asm::asm_setpix()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SETPIX;
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
bool P2Asm::asm_cogatn()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_COGATN;
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
bool P2Asm::asm_testp_w()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_TESTP_W_DIRL;
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
bool P2Asm::asm_testpn_w()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_TESTPN_W;
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
bool P2Asm::asm_testp_and()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_TESTP_AND;
    return parse_IM_D_WCZ();
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
bool P2Asm::asm_testpn_and()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_TESTPN_AND;
    return parse_IM_D_WCZ();
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
bool P2Asm::asm_testp_or()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_TESTP_OR;
    return parse_IM_D_WCZ();
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
bool P2Asm::asm_testpn_or()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_TESTPN_OR;
    return parse_IM_D_WCZ();
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
bool P2Asm::asm_testp_xor()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_TESTP_XOR;
    return parse_IM_D_WCZ();
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
bool P2Asm::asm_testpn_xor()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_TESTPN_XOR;
    return parse_IM_D_WCZ();
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
bool P2Asm::asm_dirl()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_DIRL;
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
bool P2Asm::asm_dirh()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_DIRH;
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
bool P2Asm::asm_dirc()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_DIRC;
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
bool P2Asm::asm_dirnc()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_DIRNC;
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
bool P2Asm::asm_dirz()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_DIRZ;
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
bool P2Asm::asm_dirnz()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_DIRNZ;
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
bool P2Asm::asm_dirrnd()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_DIRRND;
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
bool P2Asm::asm_dirnot()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_DIRNOT;
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
bool P2Asm::asm_outl()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_OUTL;
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
bool P2Asm::asm_outh()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_OUTH;
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
bool P2Asm::asm_outc()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_OUTC;
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
bool P2Asm::asm_outnc()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_OUTNC;
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
bool P2Asm::asm_outz()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_OUTZ;
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
bool P2Asm::asm_outnz()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_OUTNZ;
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
bool P2Asm::asm_outrnd()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_OUTRND;
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
bool P2Asm::asm_outnot()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_OUTNOT;
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
bool P2Asm::asm_fltl()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_FLTL;
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
bool P2Asm::asm_flth()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_FLTH;
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
bool P2Asm::asm_fltc()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_FLTC;
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
bool P2Asm::asm_fltnc()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_FLTNC;
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
bool P2Asm::asm_fltz()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_FLTZ;
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
bool P2Asm::asm_fltnz()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_FLTNZ;
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
bool P2Asm::asm_fltrnd()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_FLTRND;
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
bool P2Asm::asm_fltnot()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_FLTNOT;
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
bool P2Asm::asm_drvl()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_DRVL;
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
bool P2Asm::asm_drvh()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_DRVH;
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
bool P2Asm::asm_drvc()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_DRVC;
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
bool P2Asm::asm_drvnc()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_DRVNC;
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
bool P2Asm::asm_drvz()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_DRVZ;
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
bool P2Asm::asm_drvnz()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_DRVNZ;
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
bool P2Asm::asm_drvrnd()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_DRVRND;
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
bool P2Asm::asm_drvnot()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_DRVNOT;
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
bool P2Asm::asm_splitb()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SPLITB;
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
bool P2Asm::asm_mergeb()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_MERGEB;
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
bool P2Asm::asm_splitw()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SPLITW;
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
bool P2Asm::asm_mergew()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_MERGEW;
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
bool P2Asm::asm_seussf()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SEUSSF;
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
bool P2Asm::asm_seussr()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SEUSSR;
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
bool P2Asm::asm_rgbsqz()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_RGBSQZ;
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
bool P2Asm::asm_rgbexp()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_RGBEXP;
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
bool P2Asm::asm_xoro32()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_XORO32;
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
bool P2Asm::asm_rev()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_REV;
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
bool P2Asm::asm_rczr()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_RCZR;
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
bool P2Asm::asm_rczl()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_RCZL;
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
bool P2Asm::asm_wrc()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_WRC;
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
bool P2Asm::asm_wrnc()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_WRNC;
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
bool P2Asm::asm_wrz()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_WRZ;
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
bool P2Asm::asm_wrnz()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_WRNZ_MODCZ;
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
bool P2Asm::asm_modcz()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_WRNZ_MODCZ;

    p2_cond_e cccc = parse_modcz();
    if (!parse_comma())
        return false;

    p2_cond_e zzzz = parse_modcz();
    m_IR.set_dst(static_cast<p2_LONG>((cccc << 4) | zzzz));

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
bool P2Asm::asm_setscp()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_SETSCP;
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
 *
 * C = cccc[{C,Z}], Z = zzzz[{C,Z}].
 *</pre>
 */
bool P2Asm::asm_getscp()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_GETSCP;
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
bool P2Asm::asm_jmp_abs()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_JMP;
    return parse_PC_ABS();
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
bool P2Asm::asm_call_abs()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_CALL_RET;
    return parse_PC_ABS();
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
bool P2Asm::asm_calla_abs()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_CALLA_RETA;
    return parse_PC_ABS();
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
bool P2Asm::asm_callb_abs()
{
    next();
    m_IR.set_inst7(p2_OPSRC);
    m_IR.u.op.src = p2_OPSRC_CALLB_RETB;
    return parse_PC_ABS();
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
bool P2Asm::asm_calld_pa_abs()
{
    next();
    m_IR.set_inst7(p2_CALLD_PA_ABS);
    return parse_PC_ABS();
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
bool P2Asm::asm_calld_pb_abs()
{
    next();
    m_IR.set_inst7(p2_CALLD_PB_ABS);
    return parse_PC_ABS();
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
bool P2Asm::asm_calld_ptra_abs()
{
    next();
    m_IR.set_inst7(p2_CALLD_PTRA_ABS);
    return parse_PC_ABS();
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
bool P2Asm::asm_calld_ptrb_abs()
{
    next();
    m_IR.set_inst7(p2_CALLD_PTRB_ABS);
    return parse_PC_ABS();
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
bool P2Asm::asm_loc()
{
    next();
    p2_token_e tok = m_words.value(m_idx).tok();
    bool success = false;
    switch (tok) {
    case t_PA:
        next();
        parse_comma();
        success = asm_loc_pa();
        break;
    case t_PB:
        next();
        parse_comma();
        success = asm_loc_pb();
        break;
    case t_PTRA:
        next();
        parse_comma();
        success = asm_loc_ptra();
        break;
    case t_PTRB:
        next();
        parse_comma();
        success = asm_loc_ptrb();
        break;
    default:
        m_errors += tr("Invalid pointer type '%1'; expected one of %2.")
                  .arg(m_words.value(m_idx).str())
                  .arg(tr("PA, PB, PTRA, or PTRB"));
        emit Error(m_pass, m_lineno, m_errors.last());
    }
    return success;
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
bool P2Asm::asm_loc_pa()
{
    m_IR.set_inst7(p2_LOC_PA);
    return parse_PC_ABS();
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
bool P2Asm::asm_loc_pb()
{
    m_IR.set_inst7(p2_LOC_PB);
    return parse_PC_ABS();
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
bool P2Asm::asm_loc_ptra()
{
    m_IR.set_inst7(p2_LOC_PTRA);
    return parse_PC_ABS();
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
bool P2Asm::asm_loc_ptrb()
{
    m_IR.set_inst7(p2_LOC_PTRB);
    return parse_PC_ABS();
}

/**
 * @brief Queue #N[31:9] to be used as upper 23 bits for next #S occurrence, so that the next 9-bit #S will be augmented to 32 bits.
 *<pre>
 * EEEE 11110NN NNN NNNNNNNNN NNNNNNNNN
 *
 * AUGS    #N
 *</pre>
 */
bool P2Asm::asm_augs()
{
    next();
    m_IR.set_inst7(p2_AUGS);
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
bool P2Asm::asm_augd()
{
    next();
    m_IR.set_inst7(p2_AUGD);
    return parse_IMM23();
}
