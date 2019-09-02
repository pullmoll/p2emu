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

P2Opcode::P2Opcode(const p2_LONG opcode, const p2_PC_ORGH_t& pc_orgh)
    : u()
    , PC_ORGH(pc_orgh)
    , src_imm_flag(imm_none)
    , as_IR(false)
    , as_EQU(false)
    , AUGD()
    , AUGS()
    , DATA()
    , error(none)
{
    u.opcode = opcode;
}

P2Opcode::P2Opcode(const p2_inst7_e inst7, const p2_PC_ORGH_t& pc_orgh)
    : u()
    , PC_ORGH(pc_orgh)
    , src_imm_flag(imm_none)
    , as_IR(false)
    , as_EQU(false)
    , AUGD()
    , AUGS()
    , DATA()
    , error(none)
{
    set_inst7(inst7);
}

P2Opcode::P2Opcode(const p2_inst8_e inst8, const p2_PC_ORGH_t& pc_orgh)
    : u()
    , PC_ORGH(pc_orgh)
    , src_imm_flag(imm_none)
    , as_IR(false)
    , as_EQU(false)
    , AUGD()
    , AUGS()
    , DATA()
    , error(none)
{
    set_inst8(inst8);
}

P2Opcode::P2Opcode(const p2_inst9_e inst9, const p2_PC_ORGH_t& pc_orgh)
    : u()
    , PC_ORGH(pc_orgh)
    , src_imm_flag(imm_none)
    , as_IR(false)
    , as_EQU(false)
    , AUGD()
    , AUGS()
    , DATA()
    , error(none)
{
    set_inst9(inst9);
}

/**
 * @brief clear the members to their initial state
 * @param opcode optional opcode
 */
void P2Opcode::clear(const p2_LONG opcode, const p2_PC_ORGH_t& pc_orgh)
{
    src_imm_flag = imm_none;
    as_IR = false;
    as_EQU = false;
    AUGD.clear();
    AUGS.clear();
    PC_ORGH = pc_orgh;
    u.opcode = opcode;
    DATA.clear();
    EQU.clear();
}

const P2Atom& P2Opcode::equ() const
{
    static P2Atom empty;
    if (!as_EQU)
        return empty;
    return EQU;
}

p2_cond_e P2Opcode::cond() const
{
    return static_cast<p2_cond_e>(u.op.cond);
}

p2_inst7_e P2Opcode::inst7() const
{
    return static_cast<p2_inst7_e>(u.op.inst);
}

p2_inst8_e P2Opcode::inst8() const
{
    return static_cast<p2_inst8_e>(u.op8.inst);
}

p2_inst9_e P2Opcode::inst9() const
{
    return static_cast<p2_inst9_e>(u.op9.inst);
}

bool P2Opcode::wc() const
{
    return u.op.wc;
}

bool P2Opcode::wz() const
{
    return u.op.wz;
}

bool P2Opcode::im() const
{
    return u.op.im;
}

p2_LONG P2Opcode::dst() const
{
    return u.op.dst;
}

p2_LONG P2Opcode::src() const
{
    return u.op.src;
}

bool P2Opcode::set_equ(const P2Atom& value)
{
    as_IR = false;
    as_EQU = true;
    EQU = value;
    return true;
}

/**
 * @brief Set the opcode's cond field to %cond
 * @param cond enumeration value
 */
void P2Opcode::set_cond(const p2_cond_e cond)
{
    u.op.cond = cond;
}

/**
 * @brief Set the opcode's instruction field to a 7 bit enumeration value
 * @param inst instruction to set
 */
void P2Opcode::set_inst7(const p2_inst7_e inst)
{
    u.op.inst = static_cast<uint>(inst);
}

/**
 * @brief Set the opcode's instruction field to a 8 bit enumeration value
 * @param inst instruction to set
 */
void P2Opcode::set_inst8(const p2_inst8_e inst)
{
    u.op8.inst = static_cast<uint>(inst);
}

/**
 * @brief Set the opcode's instruction field to a 9 bit enumeration value
 * @param inst instruction to set
 */
void P2Opcode::set_inst9(const p2_inst9_e inst)
{
    u.op9.inst = static_cast<uint>(inst);
}

/**
 * @brief Set the opcode's bits [23:0] from AUGS/AUGD [31:9]
 * @param addr augment address to set
 */
void P2Opcode::set_imm23(const p2_LONG addr)
{
    u.opcode |= addr >> AUG_SHIFT;
}

/**
 * @brief Set the opcode's with C and with Z flags (wc, wz) on or off
 * @param on true to set, false to clear
 */
void P2Opcode::set_wcz(bool on)
{
    u.op.wc = on;
    u.op.wz = on;
}


/**
 * @brief Set the opcode's with C flag (wc) on or off
 * @param on true to set, false to clear
 */
void P2Opcode::set_wc(bool on)
{
    u.op.wc = on;
}

/**
 * @brief Set the opcode's with Z flag (wz) on or off
 * @param on true to set, false to clear
 */
void P2Opcode::set_wz(bool on)
{
    u.op.wz = on;
}

/**
 * @brief Set the opcode's immediate flag (im) on or off
 * @param on true to set, false to clear
 */
void P2Opcode::set_im(bool on)
{
    u.op.im = on;
}

/**
 * @brief Set the immediate flag specified in imm_to to %on
 * @param on true to set, false to clear the flag
 */
void P2Opcode::set_to(bool on)
{
    switch (src_imm_flag) {
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
    switch (dst_imm_flag) {
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

    u.op.dst = value & COG_MASK;
    if (value > COG_MASK || atom.trait() == P2Atom::Augmented) {
        switch (dst_imm_flag) {
        case imm_none:
            return true;
        case imm_to_im:
            if (u.op.im)
                break;
            error = dst_augd_im;
            return false;
        case imm_to_wz:
            if (u.op.wz)
                break;
            error = dst_augd_wz;
            return false;
        }
        AUGD = value & ~COG_MASK;
    } else {
        AUGD.clear();
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

    u.op.src = value & COG_MASK;
    if (value > COG_MASK || atom.trait() == P2Atom::Augmented) {
        switch (src_imm_flag) {
        case imm_none:
            return true;
        case imm_to_im:
            if (u.op.im)
                break;
            // XXX: can we do this...
            u.op.im = true;
            // instead of printing an error?
            // error = src_augs_im;
            return false;
        case imm_to_wz:
            if (u.op.wz)
                break;
            error = src_augs_wz;
            return false;
        }
        AUGS = value & ~COG_MASK;
    } else {
        AUGS.clear();
    }
    return true;
}
