#include "p2opcode.h"

P2Opcode::P2Opcode(const p2_LONG opcode, const p2_PC_ORGH_t& pc_orgh)
    : u()
    , PC_ORGH(pc_orgh)
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
    as_IR = false;
    as_EQU = false;
    AUGD.clear();
    AUGS.clear();
    PC_ORGH = pc_orgh;
    u.opcode = opcode;
    DATA.clear();
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
 * @brief Set the opcode's dst field and possibly set the AUGD value
 * @param value value to set
 * @param imm_to where to find the immediate bit (L)
 * @return true on success, or false on error
 */
bool P2Opcode::set_dst(const P2Atom& value, imm_to_e imm_to)
{
    const p2_LONG val = value.to_long();
    u.op.dst = val & 0x1ffu;
    if (val & ~0x1ffu) {
        switch (imm_to) {
        case immediate_none:
            error = dst_augd_none;
            return false;
        case immediate_im:
            if (u.op.im)
                break;
            error = dst_augd_im;
            return false;
        case immediate_wz:
            if (u.op.wz)
                break;
            error = dst_augd_wz;
            return false;
        case immediate_wc:
            if (u.op.wc)
                break;
            error = dst_augd_wc;
            return false;
        }
        AUGD = val >> 9;
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
bool P2Opcode::set_src(const P2Atom& value, imm_to_e imm_to)
{
    const p2_LONG val = value.to_long();
    u.op.src = val & 0x1ffu;
    if (val & ~0x1ffu) {
        switch (imm_to) {
        case immediate_none:
            error = src_augs_none;
            return false;
        case immediate_im:
            if (u.op.im)
                break;
            error = src_augs_im;
            return false;
        case immediate_wz:
            if (u.op.wz)
                break;
            error = src_augs_wz;
            return false;
        case immediate_wc:
            if (u.op.wc)
                break;
            error = src_augs_wc;
            return false;
        }
        AUGS = val >> 9;
    } else {
        AUGS.clear();
    }
    return true;
}
