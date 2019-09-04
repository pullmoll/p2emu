/****************************************************************************
 *
 * Propeller2 assembler opcode data implementation
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
#include "p2opcode.h"

P2Opcode::P2Opcode(const p2_LONG opcode, const p2_ORG_ORGH_t& org_orgh)
    : m_u()
    , m_org_orgh(org_orgh)
    , m_dst_imm_flag(imm_none)
    , m_src_imm_flag(imm_none)
    , m_as_ir(false)
    , m_as_equ(false)
    , m_augd()
    , m_augs()
    , m_data()
    , m_equ()
    , m_error(none)
{
    m_u.opcode = opcode;
}

P2Opcode::P2Opcode(const p2_inst7_e inst7, const p2_ORG_ORGH_t& org_orgh)
    : m_u()
    , m_org_orgh(org_orgh)
    , m_dst_imm_flag(imm_none)
    , m_src_imm_flag(imm_none)
    , m_as_ir(false)
    , m_as_equ(false)
    , m_augd()
    , m_augs()
    , m_data()
    , m_equ()
    , m_error(none)
{
    set_inst7(inst7);
}

P2Opcode::P2Opcode(const p2_inst8_e inst8, const p2_ORG_ORGH_t& pc_orgh)
    : m_u()
    , m_org_orgh(pc_orgh)
    , m_dst_imm_flag(imm_none)
    , m_src_imm_flag(imm_none)
    , m_as_ir(false)
    , m_as_equ(false)
    , m_augd()
    , m_augs()
    , m_data()
    , m_equ()
    , m_error(none)
{
    set_inst8(inst8);
}

P2Opcode::P2Opcode(const p2_inst9_e inst9, const p2_ORG_ORGH_t& pc_orgh)
    : m_u()
    , m_org_orgh(pc_orgh)
    , m_dst_imm_flag(imm_none)
    , m_src_imm_flag(imm_none)
    , m_as_ir(false)
    , m_as_equ(false)
    , m_augd()
    , m_augs()
    , m_data()
    , m_equ()
    , m_error(none)
{
    set_inst9(inst9);
}

/**
 * @brief clear the members to their initial state
 * @param opcode optional opcode
 */
void P2Opcode::clear(const p2_LONG opcode, const p2_ORG_ORGH_t& pc_orgh)
{
    m_dst_imm_flag = imm_none;
    m_src_imm_flag = imm_none;
    m_as_ir = false;
    m_as_equ = false;
    m_augd.clear();
    m_augs.clear();
    m_org_orgh = pc_orgh;
    m_u.opcode = opcode;
    m_data.clear();
    m_equ.clear();
}

const P2Atom& P2Opcode::equ() const
{
    static P2Atom empty;
    if (!m_as_equ)
        return empty;
    return m_equ;
}

const p2_ORG_ORGH_t P2Opcode::org_orgh() const
{
    return m_org_orgh;
}

void P2Opcode::set_dst_imm(P2Opcode::ImmFlag flag)
{
    m_dst_imm_flag = flag;
}

const QVariant& P2Opcode::augd() const
{
    return m_augd;
}

void P2Opcode::augd_clear()
{
    m_augd.clear();
}

bool P2Opcode::augd_valid() const
{
    return m_augd.isValid();
}

void P2Opcode::set_src_imm(P2Opcode::ImmFlag flag)
{
    m_src_imm_flag = flag;
}

const QVariant& P2Opcode::augs() const
{
    return m_augs;
}

void P2Opcode::augs_clear()
{
    m_augs.clear();
}

bool P2Opcode::augs_valid() const
{
    return m_augs.isValid();
}

bool P2Opcode::as_ir() const
{
    return m_as_ir;
}

bool P2Opcode::as_equ() const
{
    return m_as_equ;
}

p2_opcode_u P2Opcode::ir() const
{
    return m_u;
}

p2_LONG P2Opcode::opcode() const
{
    return m_u.opcode;
}

p2_cond_e P2Opcode::cond() const
{
    return static_cast<p2_cond_e>(m_u.op.cond);
}

p2_inst7_e P2Opcode::inst7() const
{
    return static_cast<p2_inst7_e>(m_u.op.inst);
}

p2_inst8_e P2Opcode::inst8() const
{
    return static_cast<p2_inst8_e>(m_u.op8.inst);
}

p2_inst9_e P2Opcode::inst9() const
{
    return static_cast<p2_inst9_e>(m_u.op9.inst);
}

p2_opsrc_e P2Opcode::opsrc() const
{
    if (p2_OPSRC == inst7())
        return static_cast<p2_opsrc_e>(m_u.op.src);
    return p2_OPSRC_INVALID;
}

p2_opx24_e P2Opcode::opx24() const
{
    if (p2_OPSRC_X24 == opsrc())
        return static_cast<p2_opx24_e>(m_u.op.dst);
    return p2_OPX24_INVALID;
}

bool P2Opcode::wc() const
{
    return m_u.op.wc;
}

bool P2Opcode::wz() const
{
    return m_u.op.wz;
}

bool P2Opcode::im() const
{
    return m_u.op.im;
}

p2_LONG P2Opcode::dst() const
{
    return m_u.op.dst;
}

p2_LONG P2Opcode::src() const
{
    return m_u.op.src;
}

const P2Atom& P2Opcode::data() const
{
    return m_data;
}

P2Opcode::Error P2Opcode::aug_error() const
{
    return m_error;
}

bool P2Opcode::set_as_IR(bool on)
{
    m_as_ir = on;
    return true;
}

bool P2Opcode::set_org_orgh(p2_ORG_ORGH_t org_orgh)
{
    m_org_orgh = org_orgh;
    return true;
}

bool P2Opcode::set_data(const P2Atom& data)
{
    m_as_ir = false;
    m_as_equ = false;
    m_data = data;
    return true;
}

bool P2Opcode::set_equ(const P2Atom& value)
{
    m_as_ir = false;
    m_as_equ = true;
    m_equ = value;
    return true;
}

/**
 * @brief Set the opcode field
 * @param opcode new value for the opcode field
 */
void P2Opcode::set_opcode(const p2_LONG opcode)
{
    m_u.opcode = opcode;
}

/**
 * @brief Set the opcode's cond field to %cond
 * @param cond enumeration value
 */
void P2Opcode::set_cond(const p2_cond_e cond)
{
    m_u.op.cond = cond;
}

/**
 * @brief Set the opcode's instruction field to a 7 bit enumeration value
 * @param inst instruction to set
 */
void P2Opcode::set_inst7(const p2_inst7_e inst)
{
    m_u.op.inst = static_cast<uint>(inst);
}

/**
 * @brief Set the opcode's instruction field to a 8 bit enumeration value
 * @param inst instruction to set
 */
void P2Opcode::set_inst8(const p2_inst8_e inst)
{
    m_u.op8.inst = static_cast<uint>(inst);
}

/**
 * @brief Set the opcode's instruction field to a 9 bit enumeration value
 * @param inst instruction to set
 */
void P2Opcode::set_inst9(const p2_inst9_e inst)
{
    m_u.op9.inst = static_cast<uint>(inst);
}

/**
 * @brief Set the opcode's destination field to a 9 bit enumeration value
 * Also set the instruction 9 bit field to p2_OPDST
 * @param inst instruction to set
 */
void P2Opcode::set_opdst(const p2_opdst_e inst)
{
    set_inst9(p2_OPDST);
    m_u.op.dst = static_cast<uint>(inst);
}

/**
 * @brief Set the opcode's source field to a 9 bit enumeration value
 * Also set the instruction 7 bit field to p2_OPSRC
 * @param inst instruction to set
 */
void P2Opcode::set_opsrc(const p2_opsrc_e inst)
{
    set_inst7(p2_OPSRC);
    m_u.op.src = static_cast<uint>(inst);
}

/**
 * @brief Set the opcode's destination field to a 9 bit enumeration value
 * @param inst instruction to set
 */
void P2Opcode::set_opx24(const p2_opx24_e inst)
{
    set_opsrc(p2_OPSRC_X24);
    m_u.op.dst = static_cast<uint>(inst);
}

/**
 * @brief Set the opcode's destination field to 9 bit offset
 * @param src offset to put into destination
 */
void P2Opcode::set_dst(const p2_LONG dst)
{
    m_u.op.dst = static_cast<uint>(dst);
}

/**
 * @brief Set the opcode's source field to 9 bit offset
 * @param src offset to put into source
 */
void P2Opcode::set_src(const p2_LONG src)
{
    m_u.op.src = static_cast<uint>(src);
}

/**
 * @brief Set the opcode's destination and source fields to 9 bit offsets
 * @param dst offset to put into destination
 * @param src offset to put into source
 */
void P2Opcode::set_dst_src(const p2_LONG dst, const p2_LONG src)
{
    m_u.op.dst = static_cast<uint>(dst);
    m_u.op.src = static_cast<uint>(src);
}

void P2Opcode::set_a20(const p2_LONG addr)
{
    Q_ASSERT(0 == (addr & ~A20MASK));
    m_u.opcode |= addr & A20MASK;
}

/**
 * @brief Set the opcode's bits [23:0] from AUGS/AUGD [31:9]
 * @param addr augment address to set
 */
void P2Opcode::set_imm23(const p2_LONG addr)
{
    m_u.opcode |= addr >> AUG_SHIFT;
}

/**
 * @brief Set the opcode's with C and with Z flags (wc, wz) on or off
 * @param on true to set, false to clear
 */
void P2Opcode::set_wcz(bool on)
{
    m_u.op.wc = on;
    m_u.op.wz = on;
}


/**
 * @brief Set the opcode's with C flag (wc) on or off
 * @param on true to set, false to clear
 */
void P2Opcode::set_wc(bool on)
{
    m_u.op.wc = on;
}

/**
 * @brief Set the opcode's with Z flag (wz) on or off
 * @param on true to set, false to clear
 */
void P2Opcode::set_wz(bool on)
{
    m_u.op.wz = on;
}

/**
 * @brief Set the opcode's immediate flag (im) on or off
 * @param on true to set, false to clear
 */
void P2Opcode::set_im(bool on)
{
    m_u.op.im = on;
}

/**
 * @brief Set the immediate flag specified in imm_to to %on
 * @param on true to set, false to clear the flag
 */
void P2Opcode::set_to(bool on)
{
    switch (m_src_imm_flag) {
    case imm_none:
        break;
    case imm_to_im:
        set_im(on);
        break;
    case imm_to_wz:
        Q_ASSERT_X(false, "set WZ for SRC?", "This is wrong");
        set_wz(on);
        break;
    }
    switch (m_dst_imm_flag) {
    case imm_none:
        break;
    case imm_to_im:
        set_im(on);
        break;
    case imm_to_wz:
        set_wz(on);
        break;
    }
}

/**
 * @brief Set the opcode's nibble index to nnn (0 … 7)
 * @param nnn index to set
 */
void P2Opcode::set_nnn(const p2_LONG nnn)
{
    Q_ASSERT(0 == (nnn & ~7u));
    m_u.opcode |= (nnn & 7u) << p2_shift_NNN;
}

/**
 * @brief Set the opcode's byte index to nn (0 … 3)
 * @param nn index to set
 */
void P2Opcode::set_nn(const p2_LONG nn)
{
    Q_ASSERT(0 == (nn & ~3u));
    m_u.opcode |= (nn & 3u) << p2_shift_NN;
}

/**
 * @brief Set the opcode's word index to n (0 … 1)
 * @param n index to set
 */
void P2Opcode::set_n(const p2_LONG n)
{
    Q_ASSERT(0 == (n & ~1u));
    m_u.opcode |= (n & 1u) << p2_shift_N;
}

/**
 * @brief Set the opcode's dst field and possibly set the AUGD value
 * @param value value to set
 * @param imm_to where to find the immediate bit (L)
 * @return true on success, or false on error
 */
bool P2Opcode::set_dst(const P2Atom& atom, const p2_LONG ORG, const p2_LONG ORGH)
{
    p2_LONG value;

    switch (atom.trait()) {
    case P2Atom::Relative:
        value = atom.to_long();
        break;
    case P2Atom::AddressHub:
        value = atom.to_long() - ORG + ORGH;
        break;
    default:
        value = atom.to_long();
        break;
    }

    m_u.op.dst = value & COG_MASK;
    if (value > COG_MASK || atom.trait() == P2Atom::Augmented) {
        switch (m_dst_imm_flag) {
        case imm_none:
            return true;
        case imm_to_im:
            if (m_u.op.im)
                break;
            m_error = dst_augd_im;
            return false;
        case imm_to_wz:
            if (m_u.op.wz)
                break;
            m_error = dst_augd_wz;
            return false;
        }
        m_augd = value & ~COG_MASK;
    } else {
        m_augd.clear();
    }
    return true;
}

/**
 * @brief Set the opcode's src field and possibly set the AUGS value
 * @param value value to set
 * @param imm_to where to find the immediate bit (I)
 * @return true on success, or false on error
 */
bool P2Opcode::set_src(const P2Atom& atom, const p2_LONG ORG, const p2_LONG ORGH)
{
    p2_LONG value;

    switch (atom.trait()) {
    case P2Atom::Relative:
        value = atom.to_long();
        break;
    case P2Atom::AddressHub:
        value = atom.to_long() - ORG + ORGH;
        break;
    default:
        value = atom.to_long();
    }

    m_u.op.src = value & COG_MASK;
    if (value > COG_MASK || atom.trait() == P2Atom::Augmented) {
        switch (m_src_imm_flag) {
        case imm_none:
            return true;
        case imm_to_im:
            if (m_u.op.im)
                break;
            // XXX: can we do this...
            m_u.op.im = true;
            // instead of printing an error?
            // error = src_augs_im;
            return false;
        case imm_to_wz:
            if (m_u.op.wz)
                break;
            m_error = src_augs_wz;
            return false;
        }
        m_augs = value & ~COG_MASK;
    } else {
        m_augs.clear();
    }
    return true;
}
