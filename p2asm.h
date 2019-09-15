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
#pragma once
#include <QObject>
#include <QString>
#include <QVariant>
#include <QHash>
#include "p2defs.h"
#include "p2opcode.h"
#include "p2token.h"
#include "p2atom.h"
#include "p2symboltable.h"


//! A QHash of ORG and ORGH per line number
typedef QHash<int,p2_ORIGIN_t> p2_ORIGIN_hash_t;

//! A QHash of P2Opcode per line number
typedef QHash<int,P2Opcode> p2_opcode_hash_t;

//! A QHash of QStringList with errors per line number
typedef QHash<int,QStringList> p2_error_hash_t;

/**
 * @brief The P2Asm class implements an Propeller2 assembler
 */
class P2Asm : public QObject
{
    Q_OBJECT

public:
    enum Section {
        dat_section,
        con_section,
        pub_section,
        pri_section,
        var_section
    };

    Q_ENUM(Section)

    explicit P2Asm(QObject *parent = nullptr);
    ~P2Asm();

    void clear();
    void pass_clear();
    void line_clear(int i);

    int pass() const;

    const QString& pathname() const;
    const QStringList& source() const;
    const QString source(int idx) const;

    int count() const;
    const p2_ORIGIN_hash_t& ORIGIN_hash() const;
    p2_ORIGIN_t get_ORIGIN(int lineno) const;
    bool has_ORIGIN(int lineno) const;

    const p2_opcode_hash_t& IR_hash() const;
    P2Opcode get_IR(int lineno) const;
    bool has_IR(int lineno) const;

    const p2_words_hash_t& words_hash() const;
    P2Words words(int lineno) const;
    bool has_words(int lineno) const;

    const p2_error_hash_t& error_hash() const;
    QStringList errors(int lineno) const;

    const QStringList& listing() const;
    const P2SymbolTable& symbols() const;

    bool assemble(const QStringList& source);
    bool assemble(const QString& filename);

signals:
    void Error(int pass, int lineno, QString message);

public slots:
    bool set_pathname(const QString& pathname);
    bool load(const QString& filename);
    bool set_source(int idx, const QString& source);
    bool set_source(const QStringList& source);

private:
    bool m_pnut;                            //!< use PNut compatible listing mode
    bool m_v33mode;                         //!< use V33 mode in index expressions?
    int m_pass;                             //!< current pass
    QString m_pathname;                     //!< current path name for FILE "filename.ext"
    QStringList m_source;                   //!< source code as QStringList
    QVector<const QString*> m_sourceptr;    //!< pointers to strings in m_source
    QStringList m_listing;                  //!< listing as QStringList
    p2_ORIGIN_hash_t m_hash_ORIGIN;         //!< optional ORIGINs per line (ORG and ORGH)
    p2_opcode_hash_t m_hash_IR;             //!< optional P2Opcode per line
    p2_words_hash_t m_hash_words;           //!< optional words per line
    p2_error_hash_t m_hash_error;           //!< optional (multiple) error messages per line
    P2SymbolTable m_symbols;                //!< symbol table
    int m_lineno;                           //!< current line number
    int m_in_curly;                         //!< parser inside curly braces comment level
    const QString* m_lineptr;               //!< pointer to current line of source
    QStringList m_errors;                   //!< error message(s) from parameters parser
    bool m_hubmode;                         //!< true if current address mode is HUB
    p2_LONG m_cogaddr;                      //!< current program counter (origin of the instruction)
    p2_LONG m_coglimit;                     //!< current limit for m_cogaddr
    p2_LONG m_hubaddr;                      //!< current origin, i.e. where the data is stored (COG, LUT, or HUB)
    p2_LONG m_advance;                      //!< advance by n longs
    P2Opcode m_IR;                          //!< current opcode with instruction register
    P2Atom m_data;                          //!< data generated by BYTE, WORD, LONG instructions
    P2Atom m_enum;                          //!< current enumeration value
    P2Words m_words;                        //!< current P2AsmWords on the line
    p2_TOKEN_e m_instr;                     //!< current instruction token
    QString m_symbol;                       //!< currently defined symbol (first name on the line before an instruction token)
    QHash<Section,QString> m_function;      //!< currently defined function symbol, i.e. a name w/o initial dot (.)
    Section m_section;                      //!< currently selected section
    int m_cnt;                              //!< count of (relevant) words
    int m_idx;                              //!< token (and word) index

    union {
        p2_BYTE BYTES[MEM_SIZE];            //!< as BYTEs
        p2_WORD WORDS[MEM_SIZE/2];          //!< as WORDs
        p2_LONG LONGS[MEM_SIZE/4];          //!< as LONGs
    } MEM;                                  //!< binary data

private:
    QHash<Section,QString> m_sections;      //!< section names as strings
    QHash<p2_TOKEN_e,p2_LONG> m_traits;     //!< traits for specific tokens

    int commata_left() const;
    bool find_tok(p2_TOKEN_e tok) const;

    static QString hub_cog(const P2Opcode& IR);

    QStringList results_instruction(bool wr_mem);
    QString results_assignment();
    QString results_comment();
    QStringList results_data(bool wr_mem);
    void results();

    QString expand_tabs(const QString& src);
    bool skip_comments();
    bool eol();
    bool get_words();

    P2Word curr_word() const;
    const QStringRef curr_ref() const;
    const QString curr_str() const;
    p2_TOKEN_e curr_tok() const;

    P2Word next_word() const;
    const QStringRef next_ref() const;
    const QString next_str() const;
    p2_TOKEN_e next_tok() const;

    bool next();
    bool prev();

    p2_Cond_e conditional();
    p2_Cond_e parse_modcz();
    P2Atom make_atom();
    bool bin_const(P2Atom& atom, const QString& str);
    bool byt_const(P2Atom& atom, const QString& str);
    bool dec_const(P2Atom& atom, const QString& str);
    bool hex_const(P2Atom& atom, const QString& str);
    bool real_const(P2Atom& atom, const QString& str);
    bool str_const(P2Atom& atom, const QString& str);

    QString find_symbol(Section sect = con_section, const QString& func = QString(), bool all_sections = false);
    QString find_locsym(Section sect = con_section, const QString& local = QString());
    P2Symbol get_symbol(Section sect = con_section, const QString& func = QString(), bool all_sections = false);
    P2Symbol get_locsym(Section sect = con_section, const QString& local = QString());
    bool define_symbol(const QString& symbol, const P2Atom& atom);
    void add_const_symbol(const QString& pfx, const P2Word& word = P2Word(), const P2Atom& atom = P2Atom());

    bool assemble_con_section();
    bool assemble_dat_section();
    bool assemble_pass();

    bool parse_atom(P2Atom& atom, int level);
    bool parse_primary(P2Atom& atom, int level);
    bool parse_unary(P2Atom& atom, int level);
    bool parse_mulops(P2Atom& atom, int level);
    bool parse_addops(P2Atom& atom, int level);
    bool parse_shiftops(P2Atom& atom, int level);
    bool parse_binops(P2Atom& atom, int level);
    p2_Traits_e parse_traits();
    P2Atom parse_expression(int level = 0);

    bool error_dst_or_src();
    P2Atom parse_dst(P2Opcode::ImmFlag flag = P2Opcode::ignore);
    P2Atom parse_src(P2Opcode::ImmFlag flag = P2Opcode::ignore);
    P2Atom parse_src_ptrx(int scale = 1, P2Opcode::ImmFlag flag = P2Opcode::immediate_I);
    bool parse_index(int scale, P2Atom& src);
    bool end_of_line();
    bool mandatory_COMMA();
    bool optional_COMMA();
    bool optional_WCZ();
    bool optional_WC();
    bool optional_WZ();
    bool mandatory_ANDC_ANDZ();
    bool mandatory_ORC_ORZ();
    bool mandatory_XORC_XORZ();

    bool parse_INST();
    bool parse_D_IM_S_WCZ();
    bool parse_D_IM_S_WC();
    bool parse_D_IM_S_WZ();
    bool parse_D_IM_S_ANDCZ();
    bool parse_D_IM_S_ORCZ();
    bool parse_D_IM_S_XORCZ();
    bool parse_WZ_D_IM_S();
    bool parse_WZ_D_REL_S();
    bool parse_WZ_D_IM_S_PTRx(int scale = 1);
    bool parse_WZ_D_IM_S_WC();
    bool parse_D_IM_S_NNN(uint max = 7);
    bool parse_D_IM_S();
    bool parse_D_REL_S();
    bool parse_D_REL_S_WCZ();
    bool parse_D_IM_S_PTRx_WCZ(int scale = 1);
    bool parse_D_IM_S_PTRx(int scale = 1);
    bool parse_D_WCZ();
    bool parse_WCZ();
    bool parse_D();
    bool parse_WZ_D();
    bool parse_IM_D();
    bool parse_IM_D_WCZ();
    bool parse_IM_D_WC();
    bool parse_IM_D_ANDC_ANDZ();
    bool parse_IM_D_ORC_ORZ();
    bool parse_IM_D_XORC_XORZ();
    bool parse_IM_S();
    bool parse_IM_S_WC();
    bool parse_REL();
    bool parse_REL_S();
    bool parse_REL_S_WCZ();
    bool parse_PC_A20();
    bool parse_PTRx_PC_A20();
    bool parse_IMM23();

    bool asm_assign();
    bool asm_enum_initial();
    bool asm_enum_continue();
    bool asm_ALIGNW();
    bool asm_ALIGNL();
    bool asm_ORG();
    bool asm_ORGF();
    bool asm_ORGH();

    bool asm_DAT();
    bool asm_CON();
    bool asm_PUB();
    bool asm_PRI();
    bool asm_VAR();

    bool asm_BYTE();
    bool asm_WORD();
    bool asm_LONG();
    bool asm_RES();
    bool asm_FILE();
    bool asm_FIT();

    bool asm_NOP();
    bool asm_ROR();
    bool asm_ROL();
    bool asm_SHR();
    bool asm_SHL();
    bool asm_RCR();
    bool asm_RCL();
    bool asm_SAR();
    bool asm_SAL();

    bool asm_ADD();
    bool asm_ADDX();
    bool asm_ADDS();
    bool asm_ADDSX();
    bool asm_SUB();
    bool asm_SUBS();
    bool asm_SUBX();
    bool asm_SUBSX();

    bool asm_CMP();
    bool asm_CMPX();
    bool asm_CMPS();
    bool asm_CMPSX();
    bool asm_CMPR();
    bool asm_CMPM();
    bool asm_SUBR();
    bool asm_CMPSUB();

    bool asm_FGE();
    bool asm_FLE();
    bool asm_FGES();
    bool asm_FLES();
    bool asm_SUMC();
    bool asm_SUMNC();
    bool asm_SUMZ();
    bool asm_SUMNZ();

    bool asm_TESTB_W();
    bool asm_TESTBN_W();
    bool asm_TESTB_AND();
    bool asm_TESTBN_AND();
    bool asm_TESTB_OR();
    bool asm_TESTBN_OR();
    bool asm_TESTB_XOR();
    bool asm_TESTBN_XOR();
    bool asm_TESTB();
    bool asm_TESTBN();

    bool asm_BITL();
    bool asm_BITH();
    bool asm_BITC();
    bool asm_BITNC();
    bool asm_BITZ();
    bool asm_BITNZ();
    bool asm_BITRND();
    bool asm_BITNOT();

    bool asm_AND();
    bool asm_ANDN();
    bool asm_OR();
    bool asm_XOR();
    bool asm_MUXC();
    bool asm_MUXNC();
    bool asm_MUXZ();
    bool asm_MUXNZ();

    bool asm_MOV();
    bool asm_NOT();
    bool asm_ABS();
    bool asm_NEG();
    bool asm_NEGC();
    bool asm_NEGNC();
    bool asm_NEGZ();
    bool asm_NEGNZ();

    bool asm_INCMOD();
    bool asm_DECMOD();
    bool asm_ZEROX();
    bool asm_SIGNX();
    bool asm_ENCOD();
    bool asm_ONES();
    bool asm_TEST();
    bool asm_TESTN();

    bool asm_SETNIB();
    bool asm_SETNIB_ALTSN();
    bool asm_GETNIB();
    bool asm_GETNIB_ALTGN();
    bool asm_ROLNIB();
    bool asm_ROLNIB_ALTGN();
    bool asm_SETBYTE();
    bool asm_SETBYTE_ALTSB();
    bool asm_GETBYTE();
    bool asm_GETBYTE_ALTGB();
    bool asm_ROLBYTE();
    bool asm_ROLBYTE_ALTGB();

    bool asm_SETWORD();
    bool asm_SETWORD_ALTSW();
    bool asm_GETWORD();
    bool asm_GETWORD_ALTGW();
    bool asm_ROLWORD();
    bool asm_ROLWORD_ALTGW();

    bool asm_ALTSN();
    bool asm_ALTSN_D();
    bool asm_ALTGN();
    bool asm_ALTGN_D();
    bool asm_ALTSB();
    bool asm_ALTSB_D();
    bool asm_ALTGB();
    bool asm_ALTGB_D();

    bool asm_ALTSW();
    bool asm_ALTSW_D();
    bool asm_ALTGW();
    bool asm_ALTGW_D();

    bool asm_ALTR();
    bool asm_ALTR_D();
    bool asm_ALTD();
    bool asm_ALTD_D();
    bool asm_ALTS();
    bool asm_ALTS_D();
    bool asm_ALTB();
    bool asm_ALTB_D();
    bool asm_ALTI();
    bool asm_ALTI_D();

    bool asm_SETR();
    bool asm_SETD();
    bool asm_SETS();
    bool asm_DECOD();
    bool asm_BMASK();
    bool asm_CRCBIT();
    bool asm_CRCNIB();

    bool asm_MUXNITS();
    bool asm_MUXNIBS();
    bool asm_MUXQ();
    bool asm_MOVBYTS();
    bool asm_MUL();
    bool asm_MULS();
    bool asm_SCA();
    bool asm_SCAS();

    bool asm_ADDPIX();
    bool asm_MULPIX();
    bool asm_BLNPIX();
    bool asm_MIXPIX();
    bool asm_ADDCT1();
    bool asm_ADDCT2();
    bool asm_ADDCT3();

    bool asm_WMLONG();
    bool asm_RQPIN();
    bool asm_RDPIN();
    bool asm_RDLUT();
    bool asm_RDBYTE();
    bool asm_RDWORD();
    bool asm_RDLONG();

    bool asm_POPA();
    bool asm_POPB();
    bool asm_CALLD();
    bool asm_RESI3();
    bool asm_RESI2();
    bool asm_RESI1();
    bool asm_RESI0();
    bool asm_RETI3();
    bool asm_RETI2();
    bool asm_RETI1();
    bool asm_RETI0();
    bool asm_CALLPA();
    bool asm_CALLPB();

    bool asm_DJZ();
    bool asm_DJNZ();
    bool asm_DJF();
    bool asm_DJNF();
    bool asm_IJZ();
    bool asm_IJNZ();
    bool asm_TJZ();
    bool asm_TJNZ();
    bool asm_TJF();
    bool asm_TJNF();
    bool asm_TJS();
    bool asm_TJNS();
    bool asm_TJV();

    bool asm_JINT();
    bool asm_JCT1();
    bool asm_JCT2();
    bool asm_JCT3();
    bool asm_JSE1();
    bool asm_JSE2();
    bool asm_JSE3();
    bool asm_JSE4();
    bool asm_JPAT();
    bool asm_JFBW();
    bool asm_JXMT();
    bool asm_JXFI();
    bool asm_JXRO();
    bool asm_JXRL();
    bool asm_JATN();
    bool asm_JQMT();

    bool asm_JNINT();
    bool asm_JNCT1();
    bool asm_JNCT2();
    bool asm_JNCT3();
    bool asm_JNSE1();
    bool asm_JNSE2();
    bool asm_JNSE3();
    bool asm_JNSE4();
    bool asm_JNPAT();
    bool asm_JNFBW();
    bool asm_JNXMT();
    bool asm_JNXFI();
    bool asm_JNXRO();
    bool asm_JNXRL();
    bool asm_JNATN();
    bool asm_JNQMT();
    bool asm_1011110_1();
    bool asm_1011111_0();
    bool asm_SETPAT();

    bool asm_WRPIN();
    bool asm_AKPIN();
    bool asm_WXPIN();
    bool asm_WYPIN();
    bool asm_WRLUT();
    bool asm_WRBYTE();
    bool asm_WRWORD();
    bool asm_WRLONG();

    bool asm_PUSHA();
    bool asm_PUSHB();
    bool asm_RDFAST();
    bool asm_WRFAST();
    bool asm_FBLOCK();
    bool asm_XINIT();
    bool asm_XSTOP();
    bool asm_XZERO();
    bool asm_XCONT();

    bool asm_REP();
    bool asm_COGINIT();

    bool asm_QMUL();
    bool asm_QDIV();
    bool asm_QFRAC();
    bool asm_QSQRT();
    bool asm_QROTATE();
    bool asm_QVECTOR();

    bool asm_HUBSET();
    bool asm_COGID();
    bool asm_COGSTOP();
    bool asm_LOCKNEW();
    bool asm_LOCKRET();
    bool asm_LOCKTRY();
    bool asm_LOCKREL();
    bool asm_QLOG();
    bool asm_QEXP();

    bool asm_RFBYTE();
    bool asm_RFWORD();
    bool asm_RFLONG();
    bool asm_RFVAR();
    bool asm_RFVARS();
    bool asm_WFBYTE();
    bool asm_WFWORD();
    bool asm_WFLONG();

    bool asm_GETQX();
    bool asm_GETQY();
    bool asm_GETCT();
    bool asm_GETRND();
    bool asm_GETRND_CZ();

    bool asm_SETDACS();
    bool asm_SETXFRQ();
    bool asm_GETXACC();
    bool asm_WAITX();
    bool asm_SETSE1();
    bool asm_SETSE2();
    bool asm_SETSE3();
    bool asm_SETSE4();
    bool asm_POLLINT();
    bool asm_POLLCT1();
    bool asm_POLLCT2();
    bool asm_POLLCT3();
    bool asm_POLLSE1();
    bool asm_POLLSE2();
    bool asm_POLLSE3();
    bool asm_POLLSE4();
    bool asm_POLLPAT();
    bool asm_POLLFBW();
    bool asm_POLLXMT();
    bool asm_POLLXFI();
    bool asm_POLLXRO();
    bool asm_POLLXRL();
    bool asm_POLLATN();
    bool asm_POLLQMT();

    bool asm_WAITINT();
    bool asm_WAITCT1();
    bool asm_WAITCT2();
    bool asm_WAITCT3();
    bool asm_WAITSE1();
    bool asm_WAITSE2();
    bool asm_WAITSE3();
    bool asm_WAITSE4();
    bool asm_WAITPAT();
    bool asm_WAITFBW();
    bool asm_WAITXMT();
    bool asm_WAITXFI();
    bool asm_WAITXRO();
    bool asm_WAITXRL();
    bool asm_WAITATN();

    bool asm_ALLOWI();
    bool asm_STALLI();
    bool asm_TRGINT1();
    bool asm_TRGINT2();
    bool asm_TRGINT3();
    bool asm_NIXINT1();
    bool asm_NIXINT2();
    bool asm_NIXINT3();
    bool asm_SETINT1();
    bool asm_SETINT2();
    bool asm_SETINT3();
    bool asm_SETQ();
    bool asm_SETQ2();

    bool asm_PUSH();
    bool asm_POP();
    bool asm_JMP();
    bool asm_CALL();
    bool asm_RET();
    bool asm_CALLA();
    bool asm_RETA();
    bool asm_CALLB();
    bool asm_RETB();

    bool asm_JMPREL();
    bool asm_SKIP();
    bool asm_SKIPF();
    bool asm_EXECF();
    bool asm_GETPTR();
    bool asm_GETBRK();
    bool asm_COGBRK();
    bool asm_BRK();

    bool asm_SETLUTS();
    bool asm_SETCY();
    bool asm_SETCI();
    bool asm_SETCQ();
    bool asm_SETCFRQ();
    bool asm_SETCMOD();
    bool asm_SETPIV();
    bool asm_SETPIX();
    bool asm_COGATN();

    bool asm_TESTP_W();
    bool asm_TESTPN_W();
    bool asm_TESTP_AND();
    bool asm_TESTPN_AND();
    bool asm_TESTP_OR();
    bool asm_TESTPN_OR();
    bool asm_TESTP_XOR();
    bool asm_TESTPN_XOR();
    bool asm_TESTP();
    bool asm_TESTPN();

    bool asm_DIRL();
    bool asm_DIRH();
    bool asm_DIRC();
    bool asm_DIRNC();
    bool asm_DIRZ();
    bool asm_DIRNZ();
    bool asm_DIRRND();
    bool asm_DIRNOT();

    bool asm_OUTL();
    bool asm_OUTH();
    bool asm_OUTC();
    bool asm_OUTNC();
    bool asm_OUTZ();
    bool asm_OUTNZ();
    bool asm_OUTRND();
    bool asm_OUTNOT();

    bool asm_FLTL();
    bool asm_FLTH();
    bool asm_FLTC();
    bool asm_FLTNC();
    bool asm_FLTZ();
    bool asm_FLTNZ();
    bool asm_FLTRND();
    bool asm_FLTNOT();

    bool asm_DRVL();
    bool asm_DRVH();
    bool asm_DRVC();
    bool asm_DRVNC();
    bool asm_DRVZ();
    bool asm_DRVNZ();
    bool asm_DRVRND();
    bool asm_DRVNOT();

    bool asm_SPLITB();
    bool asm_MERGEB();
    bool asm_SPLITW();
    bool asm_MERGEW();
    bool asm_SEUSSF();
    bool asm_SEUSSR();
    bool asm_RGBSQZ();
    bool asm_RGBEXP();
    bool asm_XORO32();

    bool asm_REV();
    bool asm_RCZR();
    bool asm_RCZL();
    bool asm_WRC();
    bool asm_WRNC();
    bool asm_WRZ();
    bool asm_WRNZ();
    bool asm_MODCZ();
    bool asm_MODC();
    bool asm_MODZ();
    bool asm_SETSCP();
    bool asm_GETSCP();

    bool asm_JMP_ABS();
    bool asm_CALL_ABS();
    bool asm_CALLA_ABS();
    bool asm_CALLB_ABS();
    bool asm_CALLD_ABS_PA();
    bool asm_CALLD_ABS_PB();
    bool asm_CALLD_ABS_PTRA();
    bool asm_CALLD_ABS_PTRB();
    bool asm_LOC();

    bool asm_AUGS();
    bool asm_AUGD();
};
