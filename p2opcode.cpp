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
#include "p2doc.h"

P2Opcode::P2Opcode(const p2_LONG opcode, p2_ORIGIN_t cog_hub)
    : m_u({opcode})
    , m_origin(cog_hub)
    , m_type(type_none)
    , m_hubmode(false)
    , m_dst_imm(ignore)
    , m_src_imm(ignore)
    , m_augd()
    , m_augs()
    , m_data()
    , m_assigned()
    , m_error_code(err_none)
    , m_error_value(0)
{
}

P2Opcode::P2Opcode(const p2_LONG opcode, p2_LONG _cog, p2_LONG _hub)
    : P2Opcode(opcode, p2_ORIGIN_t({_cog, _hub}))
{
}

P2Opcode::P2Opcode(const p2_INST5_e inst5, p2_ORIGIN_t cog_hub)
    : P2Opcode(0, cog_hub)
{
    set_inst5(inst5);
}

P2Opcode::P2Opcode(const p2_INST7_e inst7, p2_ORIGIN_t cog_hub)
    : P2Opcode(0, cog_hub)
{
    set_inst7(inst7);
}

P2Opcode::P2Opcode(const p2_INST8_e inst8, p2_ORIGIN_t cog_hub)
    : P2Opcode(0, cog_hub)
{
    set_inst8(inst8);
}

P2Opcode::P2Opcode(const p2_INST9_e inst9, p2_ORIGIN_t cog_hub)
    : P2Opcode(0, cog_hub)
{
    set_inst9(inst9);
}

/**
 * @brief clear the members to their initial state
 * @param opcode optional opcode
 */
void P2Opcode::clear(const p2_LONG opcode, p2_ORIGIN_t origin)
{
    m_u.opcode = opcode;
    m_origin = origin;
    m_type = type_none;
    m_hubmode = false;
    m_dst_imm = ignore;
    m_src_imm = ignore;
    m_augd.clear();
    m_augs.clear();
    m_data.clear();
    m_assigned.clear();
    m_error_code = err_none;
    m_error_value = 0;
}

void P2Opcode::clear(const p2_LONG opcode, p2_LONG _cog, p2_LONG _hub)
{
    clear(opcode, p2_ORIGIN_t({_cog,_hub}));
}

/**
 * @brief Return the current assignment (equals), if any
 * @return const reference to the P2Atom with the value, or invalid P2Atom
 */
P2Atom P2Opcode::assigned() const
{
    if (type_assign != m_type)
        return P2Atom();
    return m_assigned;
}

/**
 * @brief Return the current ORG/ORGH pair
 * @return QPair<p2_LONG,p2_LONG> with ORG in first, ORGH in second
 */
const p2_ORIGIN_t P2Opcode::origin() const
{
    return m_origin;
}

/**
 * @brief Return the current ORG
 * @return p2_LONG with ORG
 */
p2_LONG P2Opcode::cogaddr() const
{
    return m_origin._cog;
}

/**
 * @brief Return the current ORGH
 * @return p2_LONG with ORGH
 */
p2_LONG P2Opcode::hubaddr() const
{
    return m_origin._hub;
}

/**
 * @brief Set where to store the destination immediate flag
 * @param flag where-to-store info
 */
void P2Opcode::set_dst_imm(P2Opcode::ImmFlag flag)
{
    m_dst_imm = flag;
}

/**
 * @brief Return a const reference to the current AUGD value
 * @return const reference to a QVariant; invalid if no AUGD generated
 */
const QVariant& P2Opcode::augd() const
{
    return m_augd;
}

/**
 * @brief Set the current AUGD value (or clear if value is QVariant(), the default)
 */
void P2Opcode::set_augd(const QVariant& value)
{
    m_augd = value;
}

/**
 * @brief Return the AUGD value as p2_LONG
 * @return p2_LONG with the AUGS value (including the lower 9 bits)
 */
p2_LONG P2Opcode::augd_value() const
{
    return m_augd.toUInt() & AUG_MASK;
}

/**
 * @brief Return true, if the AUGD value is valid
 * @return true if valid, or false otherwise
 */
bool P2Opcode::augd_valid() const
{
    return m_augd.isValid();
}

/**
 * @brief Set where to store the source immediate flag
 * @param flag where-to-store info
 */
void P2Opcode::set_src_imm(P2Opcode::ImmFlag flag)
{
    m_src_imm = flag;
}

/**
 * @brief Return a const reference to the current AUGS value
 * @return const reference to a QVariant; invalid if no AUGS generated
 */
const QVariant& P2Opcode::augs() const
{
    return m_augs;
}

/**
 * @brief Set the current AUGS value (or clear if value is QVariant(), the default)
 */
void P2Opcode::set_augs(const QVariant& value)
{
    m_augs = value;
}

/**
 * @brief Return true, if the AUGS value is valid
 * @return true if valid, or false otherwise
 */
bool P2Opcode::augs_valid() const
{
    return m_augs.isValid();
}

/**
 * @brief Return the AUGS value as p2_LONG
 * @return p2_LONG with the AUGS value (including the lower 9 bits)
 */
p2_LONG P2Opcode::augs_value() const
{
    return m_augs.toUInt();
}

/**
 * @brief Return true, if the current opcode is to be interpreted as instruction
 * @return true if instruction, or false otherwise
 */
bool P2Opcode::is_instruction() const
{
    return type_instruction == m_type;
}

/**
 * @brief Return true, if the instruction was defined in HUB mode
 * @return true if instruction, or false otherwise
 */
bool P2Opcode::is_hubmode() const
{
    return m_hubmode;
}

/**
 * @brief Return true, if the current P2Opcode is to be interpreted as assignment (equals)
 * @return true if assignment, or false otherwise
 */
bool P2Opcode::is_assign() const
{
    return type_assign == m_type;
}

/**
 * @brief Return true, if the current P2Opcode is to be interpreted as data
 * @return true if data, or false otherwise
 */
bool P2Opcode::is_data() const
{
    return type_data == m_type;
}

/**
 * @brief Return the current instruction as union of formats
 * @return p2_opcode_u with the instruction
 */
p2_opcode_u P2Opcode::ir() const
{
    return m_u;
}

/**
 * @brief Return the current opcode as p2_LONG
 * @return p2_LONG with the opcode value
 */
p2_LONG P2Opcode::opcode() const
{
    return m_u.opcode;
}

/**
 * @brief Return the current condition (IF_...)
 * @return
 */
p2_Cond_e P2Opcode::cond() const
{
    return static_cast<p2_Cond_e>(m_u.op7.cond);
}

/**
 * @brief Return the current instruction's 7 bit inst value
 * @return p2_inst7_e enumeration value
 */
p2_INST7_e P2Opcode::inst7() const
{
    return static_cast<p2_INST7_e>(m_u.op7.inst);
}

/**
 * @brief Return the current instruction's 8 bit inst value
 * @return p2_inst8_e enumeration value
 */
p2_INST8_e P2Opcode::inst8() const
{
    return static_cast<p2_INST8_e>(m_u.op8.inst);
}

/**
 * @brief Return the current instruction's 9 bit inst value
 * @return p2_inst9_e enumeration value
 */
p2_INST9_e P2Opcode::inst9() const
{
    return static_cast<p2_INST9_e>(m_u.op9.inst);
}

/**
 * @brief Return the current instruction's opsrc value
 * @return p2_opsrc_e enumeration value; p2_OPSRC_INVALID if not a p2_OPSRC instruction
 */
p2_OPSRC_e P2Opcode::opsrc() const
{
    if (p2_OPSRC == inst7())
        return static_cast<p2_OPSRC_e>(m_u.op7.src);
    return p2_OPSRC_INVALID;
}

/**
 * @brief Return the current instruction's opx24 value
 * @return p2_opx24_e enumeration value; p2_OPX24_INVALID if not a p2_OPSRC / p2_OPSRC_X24 instruction
 */
p2_OPX24_e P2Opcode::opx24() const
{
    if (p2_OPSRC_X24 == opsrc())
        return static_cast<p2_OPX24_e>(m_u.op7.dst);
    return p2_OPX24_INVALID;
}

/**
 * @brief Return the current opcodes's WC flag
 * @return true if WC is set, or false otherwise
 */
bool P2Opcode::wc() const
{
    return m_u.op7.wc;
}

/**
 * @brief Return the current opcodes's WZ flag
 * @return true if WZ is set, or false otherwise
 */
bool P2Opcode::wz() const
{
    return m_u.op7.wz;
}

/**
 * @brief Return the current opcodes's IM flag
 * @return true if IM is set, or false otherwise
 */
bool P2Opcode::im() const
{
    return m_u.op7.im;
}

/**
 * @brief Return the current opcodes's destination field
 * @return 9 bit destination value
 */
p2_LONG P2Opcode::dst() const
{
    return m_u.op7.dst;
}

/**
 * @brief Return the current opcodes's source field
 * @return 9 bit source value
 */
p2_LONG P2Opcode::src() const
{
    return m_u.op7.src;
}

/**
 * @brief Return the current opcodes's 20 bit address from the opcode field
 * @return 20 bit address value
 */
p2_LONG P2Opcode::a20() const
{
    return m_u.opcode & A20MASK;
}

/**
 * @brief Return the current opcodes's 23 bit augment value from the opcode field
 * @return 23 bit augment value
 */
p2_LONG P2Opcode::imm23() const
{
    return (m_u.opcode << AUG_SHIFT) & AUG_MASK;
}

/**
 * @brief Return a const reference to the P2Atom keeping the current data
 * @return const reference to a P2Atom (invalid if no data)
 */
const P2Atom& P2Opcode::data() const
{
    return m_data;
}

/**
 * @brief Set the opcode's type to none
 */
void P2Opcode::set_none()
{
    m_type = type_none;
}

/**
 * @brief Return the most recent error code when generating the AUGS/AUGD instructions
 * @return Error code indicating the type of error
 */
P2Opcode::Error P2Opcode::aug_error_code() const
{
    return m_error_code;
}

/**
 * @brief Return the most recent error value when generating the AUGS/AUGD instructions
 * @return value which would have been used for AUGS/AUGD
 */
p2_LONG P2Opcode::aug_error_value() const
{
    return m_error_value;
}

/**
 * @brief Set the current ORG/ORGH
 * @param origin pair of p2_LONG with ORG and ORGH values
 */
void P2Opcode::set_origin(p2_ORIGIN_t origin)
{
    m_origin = origin;
}

/**
 * @brief Set the current ORG/ORGH pair
 * @param origin pair of p2_LONG with ORG and ORGH values
 */
void P2Opcode::set_origin(p2_LONG _org, p2_LONG _orgh)
{
    m_origin = p2_ORIGIN_t({_org,_orgh});
}

/**
 * @brief Set the opcode's data to the P2Atom %data
 * Also sets IR / EQU modes to false.
 * @param data const reference to the P2Atom to set as data
 * @return true on success (currently always)
 */
void P2Opcode::set_data(const P2Atom& data)
{
    m_data = data;
    m_type = type_data;
}

/**
 * @brief Set the opcode's assignment (equals) value
 * Also sets IR mode to false, EQU mode to true.
 * @param atom const reference to the P2Atom keeping the value
 * @return true on success (currently always)
 */
bool P2Opcode::set_assign(const P2Atom& atom)
{
    m_assigned = atom;
    m_type = type_assign;
    return true;
}

/**
 * @brief Set the opcode's HUB mode flag
 * @param hubmode true if HUB mode
 */
void P2Opcode::set_hubmode(bool hubmode)
{
    m_hubmode = hubmode;
}

/**
 * @brief Set the entire opcode field
 * @param opcode new value for the opcode field
 */
void P2Opcode::set_opcode(const p2_LONG opcode)
{
    m_u.opcode = opcode;
    m_type = type_instruction;
}

/**
 * @brief Set the opcode's cond field to %cond
 * @param cond p2_cond_e enumeration value
 */
void P2Opcode::set_cond(const p2_Cond_e cond)
{
    m_u.op7.cond = cond;
    m_type = type_instruction;
}

/**
 * @brief Set the opcode's instruction field to a 5 bit enumeration value
 * @param inst p2_inst5_e instruction to set
 */
void P2Opcode::set_inst5(const p2_INST5_e inst, bool rel, p2_LONG address)
{
    m_u.op5.inst = static_cast<uint>(inst);
    m_u.op5.rel = rel;
    m_u.op5.address = address;
    m_type = type_instruction;
}

/**
 * @brief Set the opcode's instruction field to a 7 bit enumeration value
 * @param inst p2_inst7_e instruction to set
 */
void P2Opcode::set_inst7(const p2_INST7_e inst)
{
    m_u.op7.inst = static_cast<uint>(inst);
    m_type = type_instruction;
}

/**
 * @brief Set the opcode's instruction field to a 8 bit enumeration value
 * @param inst p2_inst8_e instruction to set
 */
void P2Opcode::set_inst8(const p2_INST8_e inst)
{
    m_u.op8.inst = static_cast<uint>(inst);
    m_type = type_instruction;
}

/**
 * @brief Set the opcode's instruction field to a 9 bit enumeration value
 * @param inst p2_inst9_e instruction to set
 */
void P2Opcode::set_inst9(const p2_INST9_e inst)
{
    m_u.op9.inst = static_cast<uint>(inst);
    m_type = type_instruction;
}

/**
 * @brief Set the opcode's destination field to a 9 bit enumeration value
 * Also set the instruction's 9 bit field to p2_OPDST
 * @param inst instruction to set
 */
void P2Opcode::set_opdst(const p2_OPDST_e inst)
{
    set_inst9(p2_OPDST);
    m_u.op7.dst = static_cast<uint>(inst);
    m_type = type_instruction;
}

/**
 * @brief Set the opcode's source field to a 9 bit enumeration value
 * Also set the instruction's 7 bit field to p2_OPSRC
 * @param inst instruction to set
 */
void P2Opcode::set_opsrc(const p2_OPSRC_e inst)
{
    set_inst7(p2_OPSRC);
    m_u.op7.src = static_cast<uint>(inst);
    m_type = type_instruction;
}

/**
 * @brief Set the opcode's destination field to a 9 bit enumeration value
 * Also set the instruction's 7 bit field to p2_OPDST, and source to p2_OPSRC_X24.
 * @param inst instruction to set
 */
void P2Opcode::set_opx24(const p2_OPX24_e inst)
{
    set_opsrc(p2_OPSRC_X24);
    m_u.op7.dst = static_cast<uint>(inst);
    m_type = type_instruction;
}

/**
 * @brief Set the opcode's destination field to 9 bit offset or immediate
 * @param src offset to put into destination
 */
void P2Opcode::set_dst(const p2_LONG dst)
{
    m_u.op7.dst = static_cast<uint>(dst);
    m_type = type_instruction;
}

/**
 * @brief Set the opcode's source field to 9 bit offset or immediate
 * @param src offset to put into source
 */
void P2Opcode::set_src(const p2_LONG src)
{
    m_u.op7.src = static_cast<uint>(src);
    m_type = type_instruction;
}

/**
 * @brief Set the opcode's destination and source fields to 9 bit offsets
 * @param dst offset to put into destination
 * @param src offset to put into source
 */
void P2Opcode::set_dst_src(const p2_LONG dst, const p2_LONG src)
{
    Q_ASSERT(0 == (dst & ~p2_mask9));
    m_u.op7.dst = static_cast<uint>(dst);
    m_u.op7.src = static_cast<uint>(src);
    m_type = type_instruction;
}

/**
 * @brief Set the opcode's 20 bit address field (AAAAAAAAAAAAAAAAAAAA) to addr
 * @param addr 20 bit relative address
 */
void P2Opcode::set_r20(const p2_LONG addr)
{
    Q_ASSERT(0 == (addr & ~A20MASK));
    m_u.opcode |= addr & A20MASK;
    m_u.opcode |= 1u << 20;
    m_type = type_instruction;
}

/**
 * @brief Set the opcode's 20 bit address field (AAAAAAAAAAAAAAAAAAAA) to addr
 * @param addr 20 bit absolute address
 */
void P2Opcode::set_a20(const p2_LONG addr)
{
    Q_ASSERT(0 == (addr & ~A20MASK));
    m_u.opcode |= addr & A20MASK;
    m_type = type_instruction;
}

/**
 * @brief Set the opcode's bits [22:0] from AUGS/AUGD [31:9]
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
    m_u.op7.wc = on;
    m_u.op7.wz = on;
}


/**
 * @brief Set the opcode's with C flag (wc) on or off
 * @param on true to set, false to clear
 */
void P2Opcode::set_wc(bool on)
{
    m_u.op7.wc = on;
}

/**
 * @brief Set the opcode's with Z flag (wz) on or off
 * @param on true to set, false to clear
 */
void P2Opcode::set_wz(bool on)
{
    m_u.op7.wz = on;
}

/**
 * @brief Set the opcode's immediate flag (im) on or off
 * @param on true to set, false to clear
 */
void P2Opcode::set_im(bool on)
{
    m_u.op7.im = on;
}

/**
 * @brief Set the immediate flag specified in imm_to to %on
 * @param on true to set, false to clear the flag
 */
void P2Opcode::set_im_flags(bool on)
{
    switch (m_src_imm) {
    case ignore:
        break;
    case immediate_I:
        set_im(on);
        break;
    case immediate_L:
        Q_ASSERT_X(false, "set WZ for SRC?", "This is wrong");
        set_wz(on);
        break;
    }
    switch (m_dst_imm) {
    case ignore:
        break;
    case immediate_I:
        set_im(on);
        break;
    case immediate_L:
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
 * @param atom value to set
 * @param cogaddr COG address
 * @param hubaddr HUB address
 * @return true on success, or false on error
 */
bool P2Opcode::set_dst(const P2Atom& atom, const p2_LONG cogaddr, const p2_LONG hubaddr)
{
    const bool hubmode = atom.has_trait(tr_HUBMODE | tr_HUBADDRESS | tr_AUGMENTED);
    const bool relative = atom.has_trait(tr_RELATIVE) && !atom.has_trait(tr_AUGMENTED);
    const p2_LONG value = atom.get_addr(hubmode);
    bool result = true;

    m_augs.clear();
    if (relative) {
        const int rvalue = static_cast<int>(value - (hubmode ? hubaddr : cogaddr)) / sz_LONG;
        if (rvalue >= -256 && rvalue < 256) {
            m_u.op7.dst = static_cast<p2_LONG>(rvalue) & COG_MASK;
        } else {
            m_error_code = dst_relative;
            m_error_value = static_cast<p2_LONG>(rvalue);
            result = false;
        }
    } else {
        if (value >= HUB_ADDR0 || atom.has_trait(tr_AUGMENTED)) {
            m_u.op7.dst = value & COG_MASK;
            if (atom.has_trait(tr_AUGMENTED)) {
                switch (m_dst_imm) {
                case ignore:
                    break;
                case immediate_I:
                    set_im();
                    break;
                case immediate_L:
                    set_wz();
                    break;
                }
            } else {
                switch (m_dst_imm) {
                case ignore:
                    break;
                case immediate_I:
                    if (!im()) {
                        m_error_code = dst_augd_im;
                        m_error_value = value;
                        result = false;
                    }
                    break;
                case immediate_L:
                    if (!wz()) {
                        m_error_code = dst_augd_wz;
                        m_error_value = value;
                        result = false;
                    }
                    break;
                }
            }
            m_augd = value & ~COG_MASK;
        } else {
            m_u.op7.dst = (value / sz_LONG) & COG_MASK;
        }
    }
    return result;
}

/**
 * @brief Set the opcode's src field and possibly set the AUGS value
 * @param atom value to set
 * @param cogaddr COG address
 * @param hubaddr HUB address
 * @return true on success, or false on error
 */
bool P2Opcode::set_src(const P2Atom& atom, const p2_LONG cogaddr, const p2_LONG hubaddr)
{
    const bool hubmode = atom.has_trait(tr_HUBMODE | tr_HUBADDRESS | tr_AUGMENTED);
    const bool relative = atom.has_trait(tr_RELATIVE) && !atom.has_trait(tr_AUGMENTED);
    const p2_LONG value = atom.get_addr(hubmode);
    bool result = true;

    m_augs.clear();
    if (relative) {
        const int rvalue = static_cast<int>(value - (hubmode ? hubaddr : cogaddr)) / sz_LONG;
        if (rvalue >= -256 && rvalue < 256) {
            m_u.op7.src = static_cast<p2_LONG>(rvalue) & COG_MASK;
        } else {
            m_error_code = dst_relative;
            m_error_value = value;
            result = false;
        }
    } else {
        if (value >= HUB_ADDR0 || atom.has_trait(tr_AUGMENTED)) {
            m_u.op7.src = value & COG_MASK;
            if (atom.has_trait(tr_AUGMENTED)) {
                switch (m_src_imm) {
                case ignore:
                    break;
                case immediate_I:
                    set_im();
                    break;
                case immediate_L:
                    Q_ASSERT_X(m_src_imm == immediate_L, "impossible instruction WZ for S", "set_src");
                    break;
                }
            } else {
                switch (m_src_imm) {
                case ignore:
                    break;
                case immediate_I:
                    if (!im()) {
                        m_error_code = src_augs_im;
                        m_error_value = value;
                        result = false;
                    }
                    break;
                case immediate_L:
                    Q_ASSERT_X(m_src_imm == immediate_L, "impossible instruction WZ for S", "set_src");
                    break;
                }
            }
            m_augs = value & ~COG_MASK;
        } else {
            m_u.op7.src = (value / sz_LONG) & COG_MASK;
        }
    }
    return result;
}

static inline QString bin(const p2_QUAD val, int digits = 1)
{
    return QString("%1").arg(val, digits, 2, QChar('0'));
}

static inline QString byt(const p2_QUAD val, int digits = 0)
{
    return QString("%1").arg(val, digits, 16, QChar('0'));
}

static inline QString dec(const p2_QUAD val, int digits = 0)
{
    return QString("%1").arg(val, digits, 10, QChar('0'));
}

static inline QString hex(const p2_QUAD val, int digits = 0)
{
    return QString("%1").arg(val, digits, 16, QChar('0'));
}

static QString pad(int digits)
{
    QString pad;
    pad.fill('-', digits);
    return pad;
}

QString P2Opcode::format_opcode_bin(const P2Opcode& ir)
{
    return QString("%1 %2 %3%4%5 %6 %7")
            .arg(bin(ir.cond(),  4))
            .arg(bin(ir.inst7(), 7))
            .arg(bin(ir.wc(),    1))
            .arg(bin(ir.wz(),    1))
            .arg(bin(ir.im(),    1))
            .arg(bin(ir.dst(),   9))
            .arg(bin(ir.src(),   9));

}

QString P2Opcode::format_opcode_bit(const P2Opcode& ir)
{
    return QString("%1 %2 %3%4%5 %6 %7")
            .arg(byt(ir.cond(),  1))
            .arg(byt(ir.inst7(), 2))
            .arg(byt(ir.wc(),    1))
            .arg(byt(ir.wz(),    1))
            .arg(byt(ir.im(),    1))
            .arg(byt(ir.dst(),   3))
            .arg(byt(ir.src(),   3));

}

QString P2Opcode::format_opcode_dec(const P2Opcode& ir)
{
    // 4294967295
    return dec(ir.opcode(), 10);
}

QString P2Opcode::format_opcode_hex(const P2Opcode& ir)
{
    // FFFFFFFF
    return hex(ir.opcode(), 8);
}

QString P2Opcode::format_opcode_doc(const P2Opcode& ir)
{
    return QString("[%1] %2")
            .arg(Doc.pattern(ir.opcode()))
            .arg(Doc.instr(ir.opcode()));
}

P2Opcode P2Opcode::make_AUGD(const P2Opcode& ir)
{
    P2Opcode IR(p2_AUGD, ir.origin());
    p2_LONG value = ir.augd_value();
    IR.set_cond(ir.cond()); // FIXME: use cond of ir?
    IR.set_imm23(value);
    IR.set_origin(IR.origin());
    return IR;
}

P2Opcode P2Opcode::make_AUGS(const P2Opcode& ir)
{
    P2Opcode IR(p2_AUGS, ir.origin());
    p2_LONG value = ir.augs_value();
    IR.set_cond(ir.cond()); // FIXME: use cond of ir?
    IR.set_imm23(value);
    IR.set_origin(IR.origin());
    return IR;
}

QString P2Opcode::format_opcode(const P2Opcode& ir, p2_FORMAT_e fmt)
{
    QStringList list;
    switch (fmt) {
    case fmt_bin:
        if (ir.augd_valid())
            list += format_opcode_bin(make_AUGD(ir));
        if (ir.augs_valid())
            list += format_opcode_bin(make_AUGS(ir));
        list += format_opcode_bin(ir);
        break;
    case fmt_bit:
        if (ir.augd_valid())
            list += format_opcode_bit(make_AUGD(ir));
        if (ir.augs_valid())
            list += format_opcode_bit(make_AUGS(ir));
        list += format_opcode_bit(ir);
        break;
    case fmt_dec:
        if (ir.augd_valid())
            list += format_opcode_dec(make_AUGD(ir));
        if (ir.augs_valid())
            list += format_opcode_dec(make_AUGS(ir));
        list += format_opcode_dec(ir);
        break;
    case fmt_hex:
        if (ir.augd_valid())
            list += format_opcode_hex(make_AUGD(ir));
        if (ir.augs_valid())
            list += format_opcode_hex(make_AUGS(ir));
        list += format_opcode_hex(ir);
        break;
    case fmt_doc:
        if (ir.augd_valid())
            list += format_opcode_doc(make_AUGD(ir));
        if (ir.augs_valid())
            list += format_opcode_doc(make_AUGS(ir));
        list += format_opcode_doc(ir);
        break;
    }
    return list.join(QChar::LineFeed);
}

QString P2Opcode::format_assign_bin(const P2Opcode& ir, bool prefix)
{
    QString result;
    P2Atom atom = ir.assigned();
    if (prefix)
        result += QStringLiteral("%");
    switch (atom.type()) {
    case ut_Invalid:
        break;
    case ut_Bool:
        result += bin(atom.get_bool() & 1, 1);
        break;
    case ut_Byte:
        result += bin(atom.get_byte(), 8);
        break;
    case ut_Word:
        {
            p2_WORD _word = atom.get_word();
            result += bin((_word >>  8) & 0xff, 8);
            result += chr_skip_digit;
            result += bin((_word >>  0) & 0xff, 8);
        }
        break;
    case ut_Addr:
    case ut_Long:
        {
            p2_LONG _long = atom.get_long();
            result += bin((_long >> 24) & 0xff, 8);
            result += chr_skip_digit;
            result += bin((_long >> 16) & 0xff, 8);
            result += chr_skip_digit;
            result += bin((_long >>  8) & 0xff, 8);
            result += chr_skip_digit;
            result += bin((_long >>  0) & 0xff, 8);
        }
        break;
    case ut_Quad:
        {
            p2_QUAD _quad = atom.get_quad();
            if (_quad & HMAX) {
                result += bin((_quad >> 56) & 0xff, 8);
                result += chr_skip_digit;
                result += bin((_quad >> 48) & 0xff, 8);
                result += chr_skip_digit;
                result += bin((_quad >> 40) & 0xff, 8);
                result += chr_skip_digit;
                result += bin((_quad >> 32) & 0xff, 8);
            }
            result += bin((_quad >> 24) & 0xff, 8);
            result += chr_skip_digit;
            result += bin((_quad >> 16) & 0xff, 8);
            result += chr_skip_digit;
            result += bin((_quad >>  8) & 0xff, 8);
            result += chr_skip_digit;
            result += bin((_quad >>  0) & 0xff, 8);
        }
        break;
    case ut_Real:
        result += QString("%1").arg(ir.assigned().get_real(), 0, 'f');
        break;
    case ut_String:
        result += ir.assigned().string();
        break;
    }
    result += chr_larrow;
    return result;
}

QString P2Opcode::format_assign_bit(const P2Opcode& ir, bool prefix)
{
    QString result;
    P2Atom atom = ir.assigned();
    if (prefix)
        result += QStringLiteral("%%");
    switch (atom.type()) {
    case ut_Invalid:
        break;
    case ut_Bool:
        result += hex(atom.get_bool() & 1, 1);
        break;
    case ut_Byte:
        result += hex(atom.get_byte(), 2);
        break;
    case ut_Word:
        {
            p2_WORD _word = atom.get_word();
            result += hex((_word >>  8) & 0xff, 2);
            result += chr_skip_digit;
            result += hex((_word >>  0) & 0xff, 2);
        }
        break;
    case ut_Addr:
    case ut_Long:
        {
            p2_LONG _long = atom.get_long();
            result += hex((_long >> 24) & 0xff, 2);
            result += chr_skip_digit;
            result += hex((_long >> 16) & 0xff, 2);
            result += chr_skip_digit;
            result += hex((_long >>  8) & 0xff, 2);
            result += chr_skip_digit;
            result += hex((_long >>  0) & 0xff, 2);
        }
        break;
    case ut_Quad:
        {
            p2_QUAD _quad = atom.get_quad();
            if (_quad & HMAX) {
                result += hex((_quad >> 56) & 0xff, 2);
                result += chr_skip_digit;
                result += hex((_quad >> 48) & 0xff, 2);
                result += chr_skip_digit;
                result += hex((_quad >> 40) & 0xff, 2);
                result += chr_skip_digit;
                result += hex((_quad >> 32) & 0xff, 2);
            }
            result += hex((_quad >> 24) & 0xff, 2);
            result += chr_skip_digit;
            result += hex((_quad >> 16) & 0xff, 2);
            result += chr_skip_digit;
            result += hex((_quad >>  8) & 0xff, 2);
            result += chr_skip_digit;
            result += hex((_quad >>  0) & 0xff, 2);
        }
        break;
    case ut_Real:
        result += QString("%1").arg(ir.assigned().get_real(), 0, 'f');
        break;
    case ut_String:
        result += ir.assigned().string();
        break;
    }
    result += chr_larrow;
    return result;
}

QString P2Opcode::format_assign_dec(const P2Opcode& ir, bool prefix)
{
    QString result;
    P2Atom atom = ir.assigned();
    Q_UNUSED(prefix)
    // 4294967295
    switch (atom.type()) {
    case ut_Invalid:
        break;
    case ut_Bool:
        result += dec(atom.get_bool() & 1, 1);
        break;
    case ut_Byte:
        result += dec(atom.get_byte(), 3);
        break;
    case ut_Word:
        result += dec(atom.get_word(), 5);
        break;
    case ut_Addr:
    case ut_Long:
        result += dec(atom.get_long(), 0);
        break;
    case ut_Quad:
        result += dec(atom.get_quad(), 0);
        break;
    case ut_Real:
        result += QString("%1").arg(ir.data().get_real(), 0, 'f');
        break;
    case ut_String:
        result += ir.assigned().string();
        break;
    }
    result += chr_larrow;
    return result;
}

QString P2Opcode::format_assign_hex(const P2Opcode& ir, bool prefix)
{
    QString result;
    P2Atom atom = ir.assigned();
    if (prefix)
        result += QStringLiteral("$");
    switch (atom.type()) {
    case ut_Invalid:
        break;
    case ut_Bool:
        result += hex(atom.get_bool() & 1, 1);
        break;
    case ut_Byte:
        result += hex(atom.get_byte(), 2);
        break;
    case ut_Word:
        result += hex(atom.get_word(), 4);
        break;
    case ut_Addr:
    case ut_Long:
        result += hex(atom.get_long(), 8);
        break;
    case ut_Quad:
        result += hex(atom.get_quad(), 0);
        break;
    case ut_Real:
        result += QString("%1").arg(ir.data().get_real(), 0, 'f');
        break;
    case ut_String:
        result += ir.assigned().string();
        break;
    }
    result += chr_larrow;
    return result;
}

QString P2Opcode::format_assign(const P2Opcode& ir, p2_FORMAT_e fmt)
{
    QStringList list;
    if (ir.is_assign()) {
        switch (fmt) {
        case fmt_bin:
            list += format_assign_bin(ir);
            break;
        case fmt_bit:
            list += format_assign_bit(ir);
            break;
        case fmt_dec:
            list += format_assign_dec(ir);
            break;
        case fmt_hex:
            list += format_assign_hex(ir);
            break;
        default:
            list += format_assign_hex(ir);
            break;
        }
    }
    return list.join(QChar::LineFeed);
}

static p2_BYTES align_data_long(const P2Opcode& ir, p2_BYTES& bytes)
{
    const P2Atom& atom = ir.data();
    p2_BYTES mask;

    switch (atom.type()) {
    case ut_Invalid:
        return mask;
    case ut_Bool:
    case ut_Byte:
    case ut_String:
        bytes = atom.get_bytes();
        break;

    case ut_Word:
        {
            p2_WORDS words = atom.get_words();
            bytes.resize(words.size() * sz_WORD);
            memcpy(bytes.data(), words.constData(), static_cast<size_t>(bytes.size()));
        }
        break;

    case ut_Addr:
    case ut_Long:
        {
            p2_LONGS longs = atom.get_longs();
            bytes.resize(longs.size() * sz_LONG);
            memcpy(bytes.data(), longs.constData(), static_cast<size_t>(bytes.size()));
        }
        break;

    case ut_Quad:
        {
            p2_LONGS longs = atom.get_longs(true);
            bytes.resize(longs.size() * sz_LONG);
            memcpy(bytes.data(), longs.constData(), static_cast<size_t>(bytes.size()));
        }
        break;

    default:
        return mask;
    }


    // create a mask of the size of bytes
    mask.fill(0xff, bytes.size());

    // inset 00s for unused positions in LONGs
    const int offs = ir.hubaddr() & 3;
    if (offs > 0) {
        bytes.insert(0, offs, 0x00);
        mask.insert(0, offs, 0x00);
    }

    // pad to multiples of 4 bytes
    const int pad = (mask.size() + 3) & ~3;
    if (pad > mask.size()) {
        bytes.resize(pad);
        mask.resize(pad);
    }
    return mask;
}

QStringList P2Opcode::format_data_bin(const P2Opcode& ir, bool prefix, int* limit)
{
    p2_BYTES bytes;
    p2_BYTES mask = align_data_long(ir, bytes);
    QStringList result;
    QString line;


    for (int i = 0; i < mask.size(); i++) {
        if (prefix && 0 == i % 4)
            line += chr_percent;
        if (mask[i ^ 3] == 0xff) {
            line += bin(bytes[i ^ 3], 8);
        } else {
            line += pad(8);
        }
        if (3 == i % 4) {
            result += line;
            line.clear();
        } else {
            line += chr_skip_digit;
        }
        if (limit && i >= *limit) {
            *limit = mask.size();
            break;
        }
    }

    return result;
}

QStringList P2Opcode::format_data_bit(const P2Opcode& ir, bool prefix, int* limit)
{
    p2_BYTES bytes;
    p2_BYTES mask = align_data_long(ir, bytes);
    QStringList result;
    QString line;

    for (int i = 0; i < mask.size(); i++) {
        if (prefix && 0 == i % 4)
            line += str_byt_prefix;
        if (mask[i] == 0xff) {
            line += byt(bytes[i], 2);
        } else {
            line += pad(2);
        }
        if (3 == i % 4) {
            result += line;
            line.clear();
        } else {
            line += chr_comma;
        }
        if (limit && i >= *limit) {
            *limit = mask.size();
            break;
        }
    }

    return result;
}

QStringList P2Opcode::format_data_dec(const P2Opcode& ir, bool prefix, int* limit)
{
    p2_BYTES bytes;
    p2_BYTES mask = align_data_long(ir, bytes);
    QStringList result;
    QString line;
    Q_UNUSED(prefix)

    for (int i = 0; i < mask.size(); i++) {
        if (mask[i] == 0xff) {
            line += dec(bytes[i], 0);
        } else {
            line += pad(1);
        }
        if (3 == i % 4) {
            result += line;
            line.clear();
        } else {
            line += chr_comma;
        }
        if (limit && i >= *limit) {
            *limit = mask.size();
            break;
        }
    }

    return result;
}

QStringList P2Opcode::format_data_hex(const P2Opcode& ir, bool prefix, int* limit)
{
    p2_BYTES bytes;
    p2_BYTES mask = align_data_long(ir, bytes);
    QStringList result;
    QString line;

    for (int i = 0; i < mask.size(); i++) {
        if (prefix && 0 == i % 4)
            line += chr_dollar;
        if (mask[i ^ 3] == 0xff) {
            line += hex(bytes[i ^ 3], 2);
        } else {
            line += pad(2);
        }
        if (3 == i % 4) {
            result += line;
            line.clear();
        }
        if (limit && i >= *limit) {
            *limit = mask.size();
            break;
        }
    }

    return result;
}

QString P2Opcode::format_data(const P2Opcode& ir, p2_FORMAT_e fmt, int* limit)
{
    QStringList list;
    if (!ir.data().isEmpty()) {
        switch (fmt) {
        case fmt_bin:
            list += format_data_bin(ir, false, limit);
            break;
        case fmt_bit:
            list += format_data_bit(ir, false, limit);
            break;
        case fmt_dec:
            list += format_data_dec(ir, false, limit);
            break;
        case fmt_hex:
        default:
            list += format_data_hex(ir, false, limit);
            break;
        }
    }
    return list.join(QChar::LineFeed);
}
