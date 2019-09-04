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
#include "p2defs.h"
#include "p2atom.h"

//! A pair of p2_LONG where the first is the PC, the second the ORGH address
typedef QPair<p2_LONG,p2_LONG> p2_ORG_ORGH_t;

/**
 * @brief The class P2Opcode is used to keep the data generated per line together.
 */
class P2Opcode
{
public:
    explicit P2Opcode(const p2_LONG opcode = 0, const p2_ORG_ORGH_t& org_orgh = p2_ORG_ORGH_t(0,0));
    P2Opcode(const p2_inst7_e inst7, const p2_ORG_ORGH_t& org_orgh = p2_ORG_ORGH_t(0,0));
    P2Opcode(const p2_inst8_e inst8, const p2_ORG_ORGH_t& org_orgh = p2_ORG_ORGH_t(0,0));
    P2Opcode(const p2_inst9_e inst9, const p2_ORG_ORGH_t& org_orgh = p2_ORG_ORGH_t(0,0));

    //! enumeration of possible targets for the immediate (#) prefix(es)
    enum ImmFlag {
        imm_none,       //!< don't care about immediate
        imm_to_im,      //!< if immediate mode, set the immediate (im) flag in IR
        imm_to_wz       //!< if immediate mode, set the with-zero (wz) flag in IR
    };

    enum Error {
        none,
        dst_augd_none,  //!< DST constant larger than $1ff but no imediate mode
        dst_augd_im,    //!< DST constant larger than $1ff but im is not set for L
        dst_augd_wz,    //!< DST constant larger than $1ff but wz is not set for L
        src_augs_none,  //!< SRC constant larger than $1ff but no imediate mode
        src_augs_im,    //!< SRC constant larger than $1ff but im is not set for I
        src_augs_wz,    //!< SRC constant larger than $1ff but wz is not set for I
    };

    void clear(const p2_LONG opcode = 0, const p2_ORG_ORGH_t& pc_orgh = p2_ORG_ORGH_t(0,0));

    const P2Atom& equ() const;
    const p2_ORG_ORGH_t org_orgh() const;

    void set_dst_imm(ImmFlag flag);
    const QVariant& augd() const;
    void augd_clear();
    bool augd_valid() const;
    template <typename T> T augd_value() const { return qvariant_cast<T>(m_augd) & AUG_MASK; }

    void set_src_imm(ImmFlag flag);
    const QVariant& augs() const;
    void augs_clear();
    bool augs_valid() const;
    template <typename T> T augs_value() const { return qvariant_cast<T>(m_augs) & AUG_MASK; }

    Error aug_error() const;

    bool as_ir() const;
    bool as_equ() const;

    p2_opcode_u ir() const;
    p2_LONG opcode() const;
    p2_cond_e cond() const;
    p2_inst7_e inst7() const;
    p2_inst8_e inst8() const;
    p2_inst9_e inst9() const;
    p2_opsrc_e opsrc() const;
    p2_opx24_e opx24() const;
    bool wc() const;
    bool wz() const;
    bool im() const;
    p2_LONG dst() const;
    p2_LONG src() const;

    const P2Atom& data() const;

    bool set_as_IR(bool on = true);
    bool set_org_orgh(p2_ORG_ORGH_t org_orgh);
    bool set_data(const P2Atom& data);
    bool set_equ(const P2Atom& value);

    void set_opcode(const p2_LONG opcode);
    void set_cond(const p2_cond_e cond);
    void set_inst7(const p2_inst7_e inst);
    void set_inst8(const p2_inst8_e inst);
    void set_inst9(const p2_inst9_e inst);
    void set_opdst(const p2_opdst_e inst);
    void set_opsrc(const p2_opsrc_e inst);
    void set_opx24(const p2_opx24_e inst);
    void set_dst(const p2_LONG dst);
    void set_src(const p2_LONG src);
    void set_dst_src(const p2_LONG dst, const p2_LONG src);
    void set_a20(const p2_LONG addr);
    void set_imm23(const p2_LONG addr);
    void set_wcz(bool on = true);
    void set_wc(bool on = true);
    void set_wz(bool on = true);
    void set_im(bool on = true);
    void set_to(bool on = true);
    void set_nnn(const p2_LONG nnn);
    void set_nn(const p2_LONG nn);
    void set_n(const p2_LONG n);
    bool set_dst(const P2Atom& value, const p2_LONG ORG, const p2_LONG ORGH);
    bool set_src(const P2Atom& value, const p2_LONG ORG, const p2_LONG ORGH);

private:
    p2_opcode_u m_u;                //!< instruction opcode or assignment value
    p2_ORG_ORGH_t m_org_orgh;       //!< QPair of the instruction's ORG and ORGH values
    ImmFlag m_dst_imm_flag;         //!< where to store destination (D) immediate flag
    ImmFlag m_src_imm_flag;         //!< where to store source (S) immediate flag
    bool m_as_ir;                   //!< if true, the p2_opcode_u contains an instruction
    bool m_as_equ;                  //!< if true, the p2_opcode_u contains an assignment
    QVariant m_augd;                //!< optional value in case an AUGD is required
    QVariant m_augs;                //!< optional value in case an AUGS is required
    P2Atom m_data;                  //!< optional data generated from BYTE, WORD, LONG, FILE, etc.
    P2Atom m_equ;                   //!< optional atom from an assignment (=)
    Error m_error;                  //!< error set when set_dst() or set_src() return false
};

Q_DECLARE_METATYPE(P2Opcode);
