/****************************************************************************
 *
 * Propeller2 assembler opcode data class
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
#include <QVariant>
#include "p2defs.h"
#include "p2atom.h"

/**
 * @brief The class P2Opcode is used to keep the data generated per line together.
 */
class P2Opcode
{
public:
    explicit P2Opcode(const p2_LONG opcode = 0, p2_ORIGIN_t origin = p2_ORIGIN_t({0,0}));
    P2Opcode(const p2_INST5_e inst7, p2_ORIGIN_t origin = p2_ORIGIN_t({0,0}));
    P2Opcode(const p2_INST7_e inst7, p2_ORIGIN_t origin = p2_ORIGIN_t({0,0}));
    P2Opcode(const p2_INST8_e inst8, p2_ORIGIN_t origin = p2_ORIGIN_t({0,0}));
    P2Opcode(const p2_INST9_e inst9, p2_ORIGIN_t origin = p2_ORIGIN_t({0,0}));

    enum Type {
        type_none,          //!< type of the contents is unspecified
        type_instruction,   //!< type of the contents is an instruction
        type_assign,        //!< type of the contents is an assignment (CON section)
        type_data           //!< type of the contents is data (BYTE, WORD, LONG, etc.)
    };

    //! enumeration of possible targets for the immediate (#) prefix(es)
    enum ImmFlag {
        ignore,             //!< don't care about immediate
        immediate_I,        //!< if immediate mode, set the immediate I flag in IR
        immediate_L         //!< if immediate mode, set the with-zero L (Z) flag in IR
    };

    //! enumeration of possible results from setting the AUGD and/or AUGS flags
    enum Error {
        err_none,           //!< no error when creating AUGD and/or AUGS values
        dst_augd_none,      //!< DST constant larger than $1ff but no imediate mode
        dst_augd_im,        //!< DST constant larger than $1ff but I is not set for I
        dst_augd_wz,        //!< DST constant larger than $1ff but Z is not set for L
        dst_relative,       //!< DST relative range error: -$100 … $0FF
        src_augs_none,      //!< SRC constant larger than $1ff but no imediate mode
        src_augs_im,        //!< SRC constant larger than $1ff but I is not set for I
        src_relative,       //!< SRC relative range error: -$100 … $0FF
    };

    void clear(const p2_LONG opcode, p2_ORIGIN_t origin);
    void clear(const p2_LONG opcode = 0, p2_LONG _cog = 0, p2_LONG _hub = 0);

    P2Atom assigned() const;
    const p2_ORIGIN_t origin() const;
    p2_LONG cogaddr() const;
    p2_LONG hubaddr() const;

    void set_dst_imm(ImmFlag flag);
    const QVariant& augd() const;
    void set_augd(const QVariant& value = QVariant());
    bool augd_valid() const;
    p2_LONG augd_value() const;

    void set_src_imm(ImmFlag flag);
    const QVariant& augs() const;
    void set_augs(const QVariant& value = QVariant());
    bool augs_valid() const;
    p2_LONG augs_value() const;

    Error aug_error_code() const;
    p2_LONG aug_error_value() const;

    bool is_instruction() const;
    bool is_assign() const;
    bool is_data() const;

    p2_opcode_u ir() const;
    p2_LONG opcode() const;
    p2_Cond_e cond() const;
    p2_INST7_e inst7() const;
    p2_INST8_e inst8() const;
    p2_INST9_e inst9() const;
    p2_OPSRC_e opsrc() const;
    p2_OPX24_e opx24() const;
    bool wc() const;
    bool wz() const;
    bool im() const;
    p2_LONG dst() const;
    p2_LONG src() const;
    p2_LONG a20() const;
    p2_LONG imm23() const;

    const P2Atom& data() const;

    void set_as_IR(bool on = true);
    void set_origin(p2_ORIGIN_t origin);
    void set_origin(p2_LONG _org, p2_LONG _orgh);
    void set_data(const P2Atom& data);
    bool set_assign(const P2Atom& atom);

    void set_opcode(const p2_LONG opcode);
    void set_cond(const p2_Cond_e cond);
    void set_inst5(const p2_INST5_e inst);
    void set_inst7(const p2_INST7_e inst);
    void set_inst8(const p2_INST8_e inst);
    void set_inst9(const p2_INST9_e inst);
    void set_opdst(const p2_OPDST_e inst);
    void set_opsrc(const p2_OPSRC_e inst);
    void set_opx24(const p2_OPX24_e inst);
    void set_dst(const p2_LONG dst);
    void set_src(const p2_LONG src);
    void set_dst_src(const p2_LONG dst, const p2_LONG src);
    void set_r20(const p2_LONG addr);
    void set_a20(const p2_LONG addr);
    void set_imm23(const p2_LONG addr);
    void set_wcz(bool on = true);
    void set_wc(bool on = true);
    void set_wz(bool on = true);
    void set_im(bool on = true);
    void set_im_flags(bool on = true);
    void set_nnn(const p2_LONG nnn);
    void set_nn(const p2_LONG nn);
    void set_n(const p2_LONG n);
    bool set_dst(const P2Atom& value, const p2_LONG cogaddr, const p2_LONG hubaddr);
    bool set_src(const P2Atom& value, const p2_LONG cogaddr, const p2_LONG ORGH);

    static QString format_opcode_bin(const P2Opcode& ir);
    static QString format_opcode_bit(const P2Opcode& ir);
    static QString format_opcode_dec(const P2Opcode& ir);
    static QString format_opcode_hex(const P2Opcode& ir);
    static QString format_opcode_doc(const P2Opcode& ir);
    static QString format_opcode(const P2Opcode& ir, p2_FORMAT_e fmt = fmt_bin);

    static QString format_assign_bin(const P2Opcode& ir, bool prefix = false);
    static QString format_assign_bit(const P2Opcode& ir, bool prefix = false);
    static QString format_assign_dec(const P2Opcode& ir, bool prefix = false);
    static QString format_assign_hex(const P2Opcode& ir, bool prefix = false);
    static QString format_assign(const P2Opcode& ir, p2_FORMAT_e fmt = fmt_bin);

    static QStringList format_data_bin(const P2Opcode& ir, bool prefix = false, int* limit = nullptr);
    static QStringList format_data_bit(const P2Opcode& ir, bool prefix = false, int* limit = nullptr);
    static QStringList format_data_dec(const P2Opcode& ir, bool prefix = false, int* limit = nullptr);
    static QStringList format_data_hex(const P2Opcode& ir, bool prefix = false, int* limit = nullptr);
    static QString format_data(const P2Opcode& ir, p2_FORMAT_e fmt = fmt_bin, int* limit = nullptr);

private:
    p2_opcode_u m_u;                //!< instruction opcode or assignment value
    p2_ORIGIN_t m_origin;           //!< instruction's ORG and ORGH values
    Type m_type;                    //!< What type of information is stored in the opcode
    ImmFlag m_dst_imm;              //!< where to store destination (D) immediate flag
    ImmFlag m_src_imm;              //!< where to store source (S) immediate flag
    QVariant m_augd;                //!< optional value in case an AUGD is required
    QVariant m_augs;                //!< optional value in case an AUGS is required
    P2Atom m_data;                  //!< optional data generated from BYTE, WORD, LONG, FILE, etc.
    P2Atom m_assigned;              //!< optional atom from an assignment (=) or ORG/ORGF/ORGH/FIT value
    Error m_error_code;             //!< error set when set_dst() or set_src() return false
    p2_LONG m_error_value;          //!< error value when set_dst() or set_src() return false

    static P2Opcode make_AUGD(const P2Opcode& ir);
    static P2Opcode make_AUGS(const P2Opcode& ir);
};



Q_DECLARE_METATYPE(P2Opcode);
