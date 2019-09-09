/****************************************************************************
 *
 * P2 emulator Cog implementation
 *
 * Function bodies and comments generated from ":/P2 instruction set.csv"
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
#include "p2cog.h"
#include "p2util.h"

P2Cog::P2Cog(int cog_id, P2Hub* hub, QObject* parent)
    : QObject(parent)
    , HUB(hub)
    , ID(static_cast<p2_LONG>(cog_id))
    , PC(0xfc000)
    , WAIT()
    , FLAGS()
    , CT1(0)
    , CT2(0)
    , CT3(0)
    , PAT()
    , PIN()
    , INT()
    , IR()
    , D(0)
    , S(0)
    , Q(0)
    , C(0)
    , Z(0)
    , FIFO()
    , K(0)
    , STACK()
    , S_next()
    , S_aug()
    , D_aug()
    , R_aug()
    , IR_aug()
    , REP_instr()
    , REP_offset(0)
    , REP_times(0)
    , SKIP(0)
    , SKIPF(0)
    , PTRA0(0)
    , PTRB0(0)
    , HUBOP(0)
    , CORDIC_count(0)
    , QX_posted(false)
    , QY_posted(false)
    , RW_repeat(false)
    , RDL_mask(0)
    , RDL_flags0(0)
    , RDL_flags1(0)
    , WRL_mask(0)
    , WRL_flags0(0)
    , WRL_flags1(0)
    , COG()
    , LUT()
    , MEM(hub->mem())
    , MEMSIZE(hub->memsize())
{
}

p2_LONG P2Cog::rd_cog(p2_LONG addr) const
{
    return COG.RAM[addr & COG_MASK];
}

void P2Cog::wr_cog(p2_LONG addr, p2_LONG val)
{
    COG.RAM[addr & COG_MASK] = val;
}

p2_LONG P2Cog::rd_lut(p2_LONG addr) const
{
    return LUT.RAM[addr & LUT_MASK];
}

void P2Cog::wr_lut(p2_LONG addr, p2_LONG val)
{
    LUT.RAM[addr & LUT_MASK] = val;
}

p2_LONG P2Cog::rd_mem(p2_LONG addr) const
{
    p2_LONG result = 0;
    switch (addr & 0xfffe00) {
    case 0x000000:
        result = rd_cog(addr/4);
        break;
    case 0x000800:
        result = rd_lut(addr/4);
        break;
    default:
        Q_ASSERT(HUB);
        result = HUB->rd_LONG(addr);
    }
    return result;
}

void P2Cog::wr_mem(p2_LONG addr, p2_LONG val)
{
    switch (addr & 0xfffe00) {
    case 0x000000:
        wr_cog(addr/4, val);
        break;
    case 0x000800:
        wr_lut(addr/4, val);
        break;
    default:
        Q_ASSERT(HUB);
        HUB->wr_LONG(addr, val);
    }
}

/**
 * @brief Write PC address
 * @param addr address to jump to
 */
void P2Cog::wr_PC(p2_LONG addr)
{
    PC = addr & A20MASK;
    if (PC >= HUB_ADDR0)
        PC &= ~3u;
}

/**
 * @brief Write PTRA address
 * @param addr address to store in PTRA
 */
void P2Cog::wr_PTRA(p2_LONG addr)
{
    COG.REG.PTRA = addr & A20MASK;
}

/**
 * @brief Write PTRB address
 * @param addr address to store in PTRB
 */
void P2Cog::wr_PTRB(p2_LONG addr)
{
    COG.REG.PTRB = addr & A20MASK;
}

/**
 * @brief Update C if the WC flag bit is set
 * @param c new C flag
 */
void P2Cog::updateC(bool c) {
    if (IR.op7.wc)
        C = static_cast<p2_LONG>(c) & 1;
}

/**
 * @brief Update Z if the WZ flag bit is set
 * @param z new Z flag
 */
void P2Cog::updateZ(bool z) {
    if (IR.op7.wz)
        C = static_cast<p2_LONG>(z) & 1;
}

/**
 * @brief Update D, i.e. write result to COG
 * @param d new value for D
 */
void P2Cog::updateD(p2_LONG d)
{
    COG.RAM[IR.op7.dst] = d;
}

/**
 * @brief Update Q
 * @param d new value for Q
 */
void P2Cog::updateQ(p2_LONG q)
{
    Q = q;
}

/**
 * @brief Update PC (program counter)
 * @param pc new program counter
 */
void P2Cog::updatePC(p2_LONG pc)
{
    // TODO: handle switch between COG, LUT, and HUB ?
    PC = pc;
    // Stop REP, if any
    REP_instr.clear();
}

/**
 * @brief Write long %d to the LUT at address %addr
 * @param addr address (9 bit)
 * @param d data to write
 */
void P2Cog::updateLUT(p2_LONG addr, p2_LONG d)
{
    LUT.RAM[addr & 0x1ff] = d;
}

/**
 * @brief Update the SKIP flag word
 * @param d new value for SKIP flag
 */
void P2Cog::updateSKIP(p2_LONG d)
{
    SKIP = d;
}

/**
 * @brief Update the SKIPF flag word
 * @param d new value for SKIPF flag
 */
void P2Cog::updateSKIPF(p2_LONG d)
{
    SKIPF = d;
}

/**
 * @brief Push value %val to the 8 level stack
 * @param val value to push
 */
void P2Cog::pushK(p2_LONG val)
{
    K = (K - 1) & 7;
    STACK[K] = val;
}

/**
 * @brief Pop value %val from the 8 level stack
 * @return
 */
p2_LONG P2Cog::popK()
{
    p2_LONG val = STACK[K];
    K = (K + 1) & 7;
    return val;
}

/**
 * @brief Push value %val to PA
 * @param val value to push
 */
void P2Cog::pushPA(p2_LONG val)
{
    COG.REG.PA = val;
}

/**
 * @brief Push value %val to PA
 * @param val value to push
 */
void P2Cog::pushPB(p2_LONG val)
{
    COG.REG.PB = val;
}

/**
 * @brief Push value %val to HUB memory at PTRA++
 * @param val value to push
 */
void P2Cog::pushPTRA(p2_LONG val)
{
    HUB->wr_LONG(COG.REG.PTRA, val);
    COG.REG.PTRA = (COG.REG.PTRA + 4) & A20MASK;
}

/**
 * @brief Pop value %val from HUB memory at --PTRA
 * @return value from HUB memory
 */
p2_LONG P2Cog::popPTRA()
{
    COG.REG.PTRA = (COG.REG.PTRA - 4) & A20MASK;
    p2_LONG val = HUB->rd_LONG(COG.REG.PTRA);
    return val;
}

/**
 * @brief Push value %val to HUB memory at PTRB++
 * @param val value to push
 */
void P2Cog::pushPTRB(p2_LONG val)
{
    HUB->wr_LONG(COG.REG.PTRB, val);
    COG.REG.PTRB = (COG.REG.PTRB + 4) & A20MASK;
}

/**
 * @brief Pop value %val from HUB memory at --PTRB
 * @return value from HUB memory
 */
p2_LONG P2Cog::popPTRB()
{
    COG.REG.PTRA = (COG.REG.PTRB - 4) & A20MASK;
    p2_LONG val = HUB->rd_LONG(COG.REG.PTRB);
    return val;
}

/**
 * @brief Augment #S or use #S, if the flag bit is set, then possibly set S from next_S
 * @param f true if immediate mode
 */
void P2Cog::augmentS(bool f)
{
    if (f) {
        if (S_aug.isValid()) {
            S = S_aug.toUInt() | IR.op7.src;
            S_aug.clear();
        } else {
            S = IR.op7.src;
        }
    }
    if (S_next.isValid()) {
        // set S to next_S
        S = S_next.toUInt();
        S_next.clear();
    }
}

/**
 * @brief Augment #D or use #D, if the flag bit is set
 * @param f true if immediate mode
 */
void P2Cog::augmentD(bool f)
{
    if (f) {
        if (D_aug.isValid()) {
            D = D_aug.toUInt() | IR.op7.dst;
            D_aug.clear();
        } else {
            D = IR.op7.dst;
        }
    }
}

/**
 * @brief Update PA, i.e. write result to port A
 * @param d value to write
 */
void P2Cog::updatePA(p2_LONG d)
{
    Q_ASSERT(HUB);
    COG.REG.PA = d;
    HUB->wr_PA(COG.REG.PA);
}

/**
 * @brief Update PB, i.e. write result to port B
 * @param d value to write
 */
void P2Cog::updatePB(p2_LONG d)
{
    Q_ASSERT(HUB);
    COG.REG.PB = d;
    HUB->wr_PB(COG.REG.PB);
}

/**
 * @brief Update PTRA
 * @param d value to write
 */
void P2Cog::updatePTRA(p2_LONG d)
{
    COG.REG.PTRA = d;
}

/**
 * @brief Update PTRB
 * @param d value to write
 */
void P2Cog::updatePTRB(p2_LONG d)
{
    COG.REG.PTRB = d;
}

/**
 * @brief Update DIR bit
 * @param pin pin number to set direction for
 * @param io direction input (true), or output (false)
 */
void P2Cog::updateDIR(p2_LONG pin, bool io)
{
    HUB->wr_DIR(pin, static_cast<p2_LONG>(io) & 1);
}

/**
 * @brief Update OUT bit
 * @param pin pin number to set output for
 * @param io output value hi (true), or low (false)
 */
void P2Cog::updateOUT(p2_LONG pin, bool io)
{
    HUB->wr_OUT(pin, static_cast<p2_LONG>(io) & 1);
}

/**
 * @brief Setup COG to repeat a number instructions a number of times
 * @param instr number of instructions to repeat
 * @param times number of times to repeat (0 forever)
 */
void P2Cog::updateREP(p2_LONG instr, p2_LONG times)
{
    REP_times = times;
    REP_offset = 0;
    if (!instr) {
        REP_instr.clear();
    } else {
        REP_instr = instr;
    }
}

/**
 * @brief return conditional execution status for condition %cond
 * @param cond condition
 * @return true if met, false otherwise
 */
bool P2Cog::conditional(p2_cond_e cond)
{
    switch (cond) {
    case cc__ret_:        // execute always
        return true;
    case cc_nc_and_nz:    // execute if C = 0 and Z = 0
        return C == 0 && Z == 0;
    case cc_nc_and_z:     // execute if C = 0 and Z = 1
        return C == 0 && Z == 1;
    case cc_nc:           // execute if C = 0
        return C == 0;
    case cc_c_and_nz:     // execute if C = 1 and Z = 0
        return C == 1 && Z == 0;
    case cc_nz:           // execute if Z = 0
        return Z == 0;
    case cc_c_ne_z:       // execute if C != Z
        return C != Z;
    case cc_nc_or_nz:     // execute if C = 0 or Z = 0
        return C == 0 || Z == 0;
    case cc_c_and_z:      // execute if C = 1 and Z = 1
        return C == 1 && Z == 1;
    case cc_c_eq_z:       // execute if C = Z
        return C == Z;
    case cc_z:            // execute if Z = 1
        return Z == 1;
    case cc_nc_or_z:      // execute if C = 0 or Z = 1
        return C == 0 || Z == 1;
    case cc_c:            // execute if C = 1
        return C == 1;
    case cc_c_or_nz:      // execute if C = 1 or Z = 0
        return C == 1 || Z == 0;
    case cc_c_or_z:       // execute if C = 1 or Z = 1
        return C == 1 || Z == 1;
    case cc_always:
        return true;
    }
    return false;
}

/**
 * @brief Alias for conditional() with an unsigned parameter
 * @param cond condition (0 … 15)
 * @return true if met, false otherwise
 */
bool P2Cog::conditional(unsigned cond)
{
    return conditional(static_cast<p2_cond_e>(cond));
}

/**
 * @brief Return the current FIFO level
 * @return FIFO level 0 … 15
 */
p2_LONG P2Cog::fifo_level()
{
    return (FIFO.windex - FIFO.rindex) & 15;
}

void P2Cog::check_interrupt_flags()
{

    p2_LONG count = U32L(HUB->count());

    // Update counter flags
    if (count == CT1)
        FLAGS.f_CT1 = true;
    if (count == CT2)
        FLAGS.f_CT2 = true;
    if (count == CT3)
        FLAGS.f_CT3 = true;

    // Update pattern match/mismatch flag
    switch (PAT.mode) {
    case p2_PAT_PA_EQ: // (PA & mask) == match
        if (PAT.match == (HUB->rd_PA() & PAT.mask)) {
            PAT.mode = p2_PAT_NONE;
            FLAGS.f_PAT = true;
        }
        break;
    case p2_PAT_PA_NE: // (PA & mask) != match
        if (PAT.match != (HUB->rd_PA() & PAT.mask)) {
            PAT.mode = p2_PAT_NONE;
            FLAGS.f_PAT = true;
        }
        break;
    case p2_PAT_PB_EQ: // (PB & mask) == match
        if (PAT.match == (HUB->rd_PB() & PAT.mask)) {
            PAT.mode = p2_PAT_NONE;
            FLAGS.f_PAT = true;
        }
        break;
    case p2_PAT_PB_NE: // (PB & mask) != match
        if (PAT.match != (HUB->rd_PB() & PAT.mask)) {
            PAT.mode = p2_PAT_NONE;
            FLAGS.f_PAT = true;
        }
        break;
    default:
        PAT.mode = p2_PAT_NONE;
    }

    // Update PIN edge detection
    if (PIN.edge & 0xc0) {
        bool prev = ((PIN.edge >> 8) & 1) ? true : false;
        if (prev != HUB->rd_PIN(PIN.edge)) {
            switch (PIN.mode) {
            case p2_PIN_CHANGED_LO:
                if (prev)
                    INT.flags.PIN_active = true;
                break;
            case p2_PIN_CHANGED_HI:
                if (!prev)
                    INT.flags.PIN_active = true;
                break;
            case p2_PIN_CHANGED:
                INT.flags.PIN_active = true;
                break;
            default:
                INT.flags.PIN_active = false;
            }
            PIN.edge ^= 0x100;
        }
    }

    // Update RDLONG state
    if (RDL_mask & RDL_flags1) {
        INT.flags.RDL_active = true;
    }

    // Update WRLONG state
    if (WRL_mask & WRL_flags1) {
        INT.flags.WRL_active = true;
    }

    // Update LOCK edge state
    if (LOCK.edge & 0x30) {
        if (LOCK.prev != HUB->lockstate(LOCK.num)) {
            switch (LOCK.mode) {
            case p2_LOCK_NONE:
                INT.flags.LOCK_active = false;
                break;
            case p2_LOCK_CHANGED_LO:
                if (LOCK.prev)
                    INT.flags.LOCK_active = true;
                break;
            case p2_LOCK_CHANGED_HI:
                if (!LOCK.prev)
                    INT.flags.LOCK_active = true;
                break;
            case p2_LOCK_CHANGED:
                INT.flags.LOCK_active = true;
                break;
            }
        }
        LOCK.prev = !LOCK.prev;
    }
}

/**
 * @brief Check and update the interrupt state
 */
void P2Cog::check_wait_int_state()
{
    // Check if interrupts disabled or INT1 active
    if (INT.flags.disabled || INT.flags.INT1_active)
        return;

    // Check INT1
    if (INT.flags.INT1_source) {
        p2_LONG bitmask = 1u << INT.flags.INT1_source;
        if (INT.bits & bitmask) {
            INT.flags.disabled = true;
            return;
        }
    }

    // Check if interrupts disabled or INT1 or INT2 active
    if (INT.flags.disabled || INT.flags.INT1_active || INT.flags.INT2_active)
        return;

    // Check INT2
    if (INT.flags.INT2_source) {
        p2_LONG bitmask = 1u << INT.flags.INT2_source;
        if (INT.bits & bitmask) {
            INT.flags.disabled = true;
            return;
        }
    }

    // Check if interrupts disabled or INT1 or INT2 or INT3 active
    if (INT.flags.disabled || INT.flags.INT1_active || INT.flags.INT2_active || INT.flags.INT3_active)
        return;

    // Check INT3
    if (INT.flags.INT3_source) {
        p2_LONG bitmask = 1u << INT.flags.INT3_source;
        if (INT.bits & bitmask) {
            INT.flags.disabled = true;
            return;
        }
    }
}

/**
 * @brief Check and update the WAIT flags
 * @param IR instruction register
 * @param value1 1st value (D)
 * @param value2 2nd value (S)
 * @param streamflag true, if streaming is active
 * @return
 */
p2_LONG P2Cog::check_wait_flag(p2_opcode_u IR, p2_LONG value1, p2_LONG value2, bool streamflag)
{
    p2_LONG hubcycles;
    const p2_instx1_e inst1 = static_cast<p2_instx1_e>(IR.opcode & p2_INSTR_MASK1);
    const p2_instx2_e inst2 = static_cast<p2_instx2_e>(IR.opcode & p2_INSTR_MASK2);
    const p2_opcode_e opcode = static_cast<p2_opcode_e>(IR.op7.inst);

    if (WAIT.flag) {
        switch (WAIT.mode) {
        case p2_WAIT_CACHE:
            Q_ASSERT(false && "We should not be here!");
            break;

        case p2_WAIT_CORDIC:
            switch (inst1) {
            case p2_INSTR_GETQX:
                if (QX_posted)
                    WAIT.flag = 0;
                break;
            case p2_INSTR_GETQY:
                if (QY_posted)
                    WAIT.flag = 0;
                break;
            default:
                Q_ASSERT(false && "We should not be here!");
            }
            if (0 == WAIT.flag)
                WAIT.mode = p2_WAIT_NONE;
            return WAIT.flag;

        case p2_WAIT_FLAG:
            break;

        default:
            WAIT.flag--;
            if (0 == WAIT.flag) {
                if (WAIT.mode == p2_WAIT_HUB && streamflag)
                    WAIT.flag = 16;
                else {
                    WAIT.mode = p2_WAIT_NONE;
                }
            }
            return WAIT.flag;
        }
    }

    if (RW_repeat) {
        if (!streamflag)
            return 0;
        WAIT.flag = 16;
        WAIT.mode = p2_WAIT_HUB;
        return WAIT.flag;
    }

    if (0 == HUB->hubslots())
        hubcycles = 0;
    else
        hubcycles = ((ID + (value2 >> 2)) - HUB->cogindex()) & 15u;

    if (p2_INSTR_GETQX == inst1) {

        if (!QX_posted && CORDIC_count > 0) {
            WAIT.flag = 1;
            WAIT.mode = p2_WAIT_CORDIC;
        }

    } else if (p2_INSTR_GETQY == inst1) {

        if (!QY_posted && CORDIC_count > 0) {
            WAIT.flag = 1;
            WAIT.mode = p2_WAIT_CORDIC;
        }

    } else if (p2_INSTR_WAITXXX == inst2) {

        if (0x02024 == (IR.opcode & 0x3ffff))
            check_wait_int_state();

        switch (IR.op7.dst) {
        case 0x10:
            WAIT.flag = FLAGS.f_INT ? 0 : 1;
            WAIT.mode = FLAGS.f_INT ? p2_WAIT_NONE : p2_WAIT_FLAG;
            break;
        case 0x11:
            WAIT.flag = FLAGS.f_CT1 ? 0 : 1;
            WAIT.mode = FLAGS.f_CT1 ? p2_WAIT_NONE : p2_WAIT_FLAG;
            break;
        case 0x12:
            WAIT.flag = FLAGS.f_CT2 ? 0 : 1;
            WAIT.mode = FLAGS.f_CT2 ? p2_WAIT_NONE : p2_WAIT_FLAG;
            break;
        case 0x13:
            WAIT.flag = FLAGS.f_CT3 ? 0 : 1;
            WAIT.mode = FLAGS.f_CT3 ? p2_WAIT_NONE : p2_WAIT_FLAG;
            break;
        case 0x14:
            WAIT.flag = FLAGS.f_SE1 ? 0 : 1;
            WAIT.mode = FLAGS.f_SE1 ? p2_WAIT_NONE : p2_WAIT_FLAG;
            break;
        case 0x15:
            WAIT.flag = FLAGS.f_SE2 ? 0 : 1;
            WAIT.mode = FLAGS.f_SE2 ? p2_WAIT_NONE : p2_WAIT_FLAG;
            break;
        case 0x16:
            WAIT.flag = FLAGS.f_SE3 ? 0 : 1;
            WAIT.mode = FLAGS.f_SE3 ? p2_WAIT_NONE : p2_WAIT_FLAG;
            break;
        case 0x17:
            WAIT.flag = FLAGS.f_SE4 ? 0 : 1;
            WAIT.mode = FLAGS.f_SE4 ? p2_WAIT_NONE : p2_WAIT_FLAG;
            break;
        case 0x18:
            WAIT.flag = FLAGS.f_PAT ? 0 : 1;
            WAIT.mode = FLAGS.f_PAT ? p2_WAIT_NONE : p2_WAIT_FLAG;
            break;
        case 0x19:
            WAIT.flag = FLAGS.f_FBW ? 0 : 1;
            WAIT.mode = FLAGS.f_FBW ? p2_WAIT_NONE : p2_WAIT_FLAG;
            break;
        case 0x1a:
            WAIT.flag = FLAGS.f_XMT ? 0 : 1;
            WAIT.mode = FLAGS.f_XMT ? p2_WAIT_NONE : p2_WAIT_FLAG;
            break;
        case 0x1b:
            WAIT.flag = FLAGS.f_XFI ? 0 : 1;
            WAIT.mode = FLAGS.f_XFI ? p2_WAIT_NONE : p2_WAIT_FLAG;
            break;
        case 0x1c:
            WAIT.flag = FLAGS.f_XRO ? 0 : 1;
            WAIT.mode = FLAGS.f_XRO ? p2_WAIT_NONE : p2_WAIT_FLAG;
            break;
        case 0x1d:
            WAIT.flag = FLAGS.f_XRL ? 0 : 1;
            WAIT.mode = FLAGS.f_XRL ? p2_WAIT_NONE : p2_WAIT_FLAG;
            break;
        case 0x1e:
            WAIT.flag = FLAGS.f_ATN ? 0 : 1;
            WAIT.mode = FLAGS.f_ATN ? p2_WAIT_NONE : p2_WAIT_FLAG;
            break;
        }
    } else if (p2_INSTR_WAITX == inst1) {

        WAIT.flag = value1;
        WAIT.mode = p2_WAIT_CNT;

    } else if (opcode >= p2_OPCODE_RDBYTE && opcode <= p2_OPCODE_RDLONG) {

        HUBOP = 1;
        WAIT.flag = hubcycles + 2;
        WAIT.mode = p2_WAIT_HUB;

    } else if ((opcode == p2_OPCODE_WRBYTE) ||
               (opcode == p2_OPCODE_WRLONG && IR.op7.wc) ||
               (opcode == p2_OPCODE_WMLONG && IR.op7.wc && IR.op7.wz)) {

        HUBOP = 1;
        WAIT.flag = hubcycles + 2;
        WAIT.mode = p2_WAIT_HUB;

    } else if ((opcode >= p2_OPCODE_QMUL && opcode <= p2_OPCODE_QVECTOR) ||
               (inst1 == p2_INSTR_QLOG) || (inst1 == p2_INSTR_QEXP))
    {

        HUBOP = 1;
        WAIT.flag = ((ID - HUB->cogindex()) & 15) + 1;
        WAIT.mode = p2_WAIT_HUB;

    } else if (inst1 >= p2_INSTR_LOCKNEW && inst1 <= p2_INSTR_LOCKSET) {

        HUBOP = 1;
        WAIT.flag = ((ID - HUB->cogindex()) & 15) + 1;
        WAIT.mode = p2_WAIT_HUB;

    } else if (inst1 >= p2_INSTR_RFBYTE && inst1 <= p2_INSTR_RFLONG) {

        if (fifo_level() < 2) {
            WAIT.flag = 1;
            WAIT.mode = p2_WAIT_CACHE;
        } else {
            WAIT.flag = 0;
        }

    } else if (inst1 >= p2_INSTR_WFBYTE && inst1 <= p2_INSTR_WFLONG) {

        if (fifo_level() >= 15) {
            WAIT.flag = 1;
            WAIT.mode = p2_WAIT_CACHE;
        } else {
            WAIT.flag = 0;
        }

    } else {

        WAIT.flag = 0;

    }

    return HUBOP;
}

/**
 * @brief Compute the hub RAM address from the pointer instruction
 * @param inst instruction field (1SUPIIIII)
 * @param size size of instruction (0 = byte, 1 = word, 2 = long)
 * @return computed 20 bit RAM address
 */
p2_LONG P2Cog::get_pointer(p2_LONG inst, p2_LONG size)
{
    p2_LONG address = 0;
    p2_LONG offset = static_cast<p2_LONG>((static_cast<qint32>(inst) << 27) >> (27 - size));

    switch ((inst >> 5) & 7) {
    case 0: // PTRA[offset]
        address = COG.REG.PTRA + offset;
        break;
    case 1: // PTRA
        address = COG.REG.PTRA;
        break;
    case 2: // PTRA[++offset]
        address = COG.REG.PTRA + offset;
        PTRA0 = address;
        break;
    case 3: // PTRA[offset++]
        address = COG.REG.PTRA;
        PTRA0 = COG.REG.PTRA + offset;
        break;
    case 4: // PTRB[offset]
        address = COG.REG.PTRB + offset;
        break;
    case 5: // PTRB
        address = COG.REG.PTRB;
        break;
    case 6: // PTRB[++offset]
        address = COG.REG.PTRB + offset;
        PTRB0 = address;
        break;
    case 7: // PTRB[offset++]
        address = COG.REG.PTRB;
        PTRB0 = COG.REG.PTRB + offset;
        break;
    }
    return address & A20MASK;
}

/**
 * @brief Save pointer registers PTRA/PTRB
 */
void P2Cog::save_regs()
{
    PTRA0 = COG.REG.PTRA;
    PTRB0 = COG.REG.PTRB;
}

/**
 * @brief Update pointer registers PTRA/PTRB
 */
void P2Cog::update_regs()
{
    COG.REG.PTRA = PTRA0;
    COG.REG.PTRB = PTRB0;
}

/**
 * @brief Read and decode the next Propeller2 opcode
 * @return number of cycles
 */
int P2Cog::decode()
{
    int cycles = 2;

    check_interrupt_flags();

    do {
        switch (PC & 0xffe00) {
        case 0x00000:   // cogexec
            IR.opcode = COG.RAM[PC];
            PC++;       // increment PC
            break;
        case 0x00200:   // lutexec
            IR.opcode = LUT.RAM[PC - 0x200];
            PC++;       // increment PC
            break;
        default:        // hubexec
            IR.opcode = HUB->rd_LONG(PC);
            PC += 4;    // increment PC by 4
        }
        if (SKIPF)
            SKIPF >>= 1;
    } while (SKIPF & 1);

    S = COG.RAM[IR.op7.src]; // set S to COG[src]
    D = COG.RAM[IR.op7.dst]; // set D to COG[dst]

    // check for the condition
    if (!conditional(IR.op7.cond))
        return cycles;

    if (SKIP) {
        if (SKIP & 1) {
            SKIP >>= 1;
            return cycles;
        }
        SKIP >>= 1;
    }

    // Dispatch to op_xxx() functions
    switch (IR.op7.inst) {
    case p2_ROR:
        cycles = op_ROR();
        break;

    case p2_ROL:
        cycles = op_ROL();
        break;

    case p2_SHR:
        cycles = op_SHR();
        break;

    case p2_SHL:
        cycles = op_SHL();
        break;

    case p2_RCR:
        cycles = op_RCR();
        break;

    case p2_RCL:
        cycles = op_RCL();
        break;

    case p2_SAR:
        cycles = op_SAR();
        break;

    case p2_SAL:
        cycles = op_SAL();
        break;

    case p2_ADD:
        cycles = op_ADD();
        break;

    case p2_ADDX:
        cycles = op_ADDX();
        break;

    case p2_ADDS:
        cycles = op_ADDS();
        break;

    case p2_ADDSX:
        cycles = op_ADDSX();
        break;

    case p2_SUB:
        cycles = op_SUB();
        break;

    case p2_SUBX:
        cycles = op_SUBX();
        break;

    case p2_SUBS:
        cycles = op_SUBS();
        break;

    case p2_SUBSX:
        cycles = op_SUBSX();
        break;

    case p2_CMP:
        cycles = op_CMP();
        break;

    case p2_CMPX:
        cycles = op_CMPX();
        break;

    case p2_CMPS:
        cycles = op_CMPS();
        break;

    case p2_CMPSX:
        cycles = op_CMPSX();
        break;

    case p2_CMPR:
        cycles = op_CMPR();
        break;

    case p2_CMPM:
        cycles = op_CMPM();
        break;

    case p2_SUBR:
        cycles = op_SUBR();
        break;

    case p2_CMPSUB:
        cycles = op_CMPSUB();
        break;

    case p2_FGE:
        cycles = op_FGE();
        break;

    case p2_FLE:
        cycles = op_FLE();
        break;

    case p2_FGES:
        cycles = op_FGES();
        break;

    case p2_FLES:
        cycles = op_FLES();
        break;

    case p2_SUMC:
        cycles = op_SUMC();
        break;

    case p2_SUMNC:
        cycles = op_SUMNC();
        break;

    case p2_SUMZ:
        cycles = op_SUMZ();
        break;

    case p2_SUMNZ:
        cycles = op_SUMNZ();
        break;

    case p2_TESTB_W_BITL:
        switch (IR.op9.inst) {
        case p2_TESTB_WC:
        case p2_TESTB_WZ:
            cycles = op_TESTB_W();
            break;
        case p2_BITL:
        case p2_BITL_WCZ:
            cycles = op_BITL();
            break;
        default:
            Q_ASSERT_X(false, "TESTB WC|WZ, BITL {WCZ}", "inst9 error");
        }
        break;

    case p2_TESTBN_W_BITH:
        switch (IR.op9.inst) {
        case p2_TESTBN_WZ:
        case p2_TESTBN_WC:
            cycles = op_TESTBN_W();
            break;
        case p2_BITH:
        case p2_BITH_WCZ:
            cycles = op_BITH();
            break;
        default:
            Q_ASSERT_X(false, "TESTBN WC|WZ, BITH {WCZ}", "inst9 error");
        }
        break;
    case p2_TESTB_AND_BITC:
        switch (IR.op9.inst) {
        case p2_TESTB_ANDZ:
        case p2_TESTB_ANDC:
            cycles = op_TESTB_AND();
            break;
        case p2_BITC:
        case p2_BITC_WCZ:
            cycles = op_BITC();
            break;
        default:
            Q_ASSERT_X(false, "TESTB ANDC|ANDZ, BITC {WCZ}", "inst9 error");
        }
        break;
    case p2_TESTBN_AND_BITNC:
        switch (IR.op9.inst) {
        case p2_TESTBN_ANDZ:
        case p2_TESTBN_ANDC:
            cycles = op_TESTBN_AND();
            break;
        case p2_BITNC:
        case p2_BITNC_WCZ:
            cycles = op_BITNC();
            break;
        default:
            Q_ASSERT_X(false, "TESTBN ANDC|ANDZ, BITNC {WCZ}", "inst9 error");
        }
        break;
    case p2_TESTB_OR_BITZ:
        switch (IR.op9.inst) {
        case p2_TESTB_ORC:
        case p2_TESTB_ORZ:
            cycles = op_TESTB_OR();
            break;
        case p2_BITZ:
        case p2_BITZ_WCZ:
            cycles = op_BITZ();
            break;
        default:
            Q_ASSERT_X(false, "TESTB ORC|ORZ, BITZ {WCZ}", "inst9 error");
        }
        break;
    case p2_TESTBN_OR_BITNZ:
        switch (IR.op9.inst) {
        case p2_TESTBN_ORC:
        case p2_TESTBN_ORZ:
            cycles = op_TESTBN_OR();
            break;
        case p2_BITNZ:
        case p2_BITNZ_WCZ:
            cycles = op_BITNZ();
            break;
        default:
            Q_ASSERT_X(false, "TESTBN ORC|ORZ, BITNZ {WCZ}", "inst9 error");
        }
        break;

    case p2_TESTB_XOR_BITRND:
        switch (IR.op9.inst) {
        case p2_TESTB_XORC:
        case p2_TESTB_XORZ:
            cycles = op_TESTB_XOR();
            break;
        case p2_BITRND:
        case p2_BITRND_WCZ:
            cycles = op_BITRND();
            break;
        default:
            Q_ASSERT_X(false, "TESTB XORC|XORZ, BITRND {WCZ}", "inst9 error");
        }
        break;

    case p2_TESTBN_XOR_BITNOT:
        switch (IR.op9.inst) {
        case p2_TESTBN_XORC:
        case p2_TESTBN_XORZ:
            cycles = op_TESTBN_XOR();
            break;
        case p2_BITNOT:
        case p2_BITNOT_WCZ:
            cycles = op_BITNOT();
            break;
        default:
            Q_ASSERT_X(false, "TESTBN XORC|XORZ, BITNOT {WCZ}", "inst9 error");
        }
        break;

    case p2_AND:
        cycles = op_AND();
        break;

    case p2_ANDN:
        cycles = op_ANDN();
        break;

    case p2_OR:
        cycles = op_OR();
        break;

    case p2_XOR:
        cycles = op_XOR();
        break;

    case p2_MUXC:
        cycles = op_MUXC();
        break;

    case p2_MUXNC:
        cycles = op_MUXNC();
        break;

    case p2_MUXZ:
        cycles = op_MUXZ();
        break;

    case p2_MUXNZ:
        cycles = op_MUXNZ();
        break;

    case p2_MOV:
        cycles = op_MOV();
        break;

    case p2_NOT:
        cycles = op_NOT();
        break;

    case p2_ABS:
        cycles = op_ABS();
        break;

    case p2_NEG:
        cycles = op_NEG();
        break;

    case p2_NEGC:
        cycles = op_NEGC();
        break;

    case p2_NEGNC:
        cycles = op_NEGNC();
        break;

    case p2_NEGZ:
        cycles = op_NEGZ();
        break;

    case p2_NEGNZ:
        cycles = op_NEGNZ();
        break;

    case p2_INCMOD:
        cycles = op_INCMOD();
        break;

    case p2_DECMOD:
        cycles = op_DECMOD();
        break;

    case p2_ZEROX:
        cycles = op_ZEROX();
        break;

    case p2_SIGNX:
        cycles = op_SIGNX();
        break;

    case p2_ENCOD:
        cycles = op_ENCOD();
        break;

    case p2_ONES:
        cycles = op_ONES();
        break;

    case p2_TEST:
        cycles = op_TEST();
        break;

    case p2_TESTN:
        cycles = op_TESTN();
        break;

    case p2_SETNIB_0:
    case p2_SETNIB_1:
        cycles = op_SETNIB();
        break;

    case p2_GETNIB_0:
    case p2_GETNIB_1:
        cycles = op_GETNIB();
        break;

    case p2_ROLNIB_0:
    case p2_ROLNIB_1:
        cycles = op_ROLNIB();
        break;

    case p2_SETBYTE:
        cycles = op_SETBYTE();
        break;

    case p2_GETBYTE:
        cycles = op_GETBYTE();
        break;

    case p2_ROLBYTE:
        cycles = op_ROLBYTE();
        break;

    case p2_SETWORD_GETWORD:
        switch (IR.op9.inst) {
        case p2_SETWORD_ALTSW:
            cycles = op_SETWORD_ALTSW();
            break;
        case p2_SETWORD:
            cycles = op_SETWORD();
            break;
        case p2_GETWORD_ALTGW:
            cycles = op_GETWORD_ALTGW();
            break;
        case p2_GETWORD:
            cycles = op_GETWORD();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst9_e", "SETWORD/GETWORD");
        }
        break;

    case p2_ROLWORD_ALTSN_ALTGN:
        switch (IR.op9.inst) {
        case p2_ROLWORD_ALTGW:
            if (0 == IR.op7.src) {
                cycles = op_ROLWORD_ALTGW();
                break;
            }
            break;
        case p2_ROLWORD:
            cycles = op_ROLWORD();
            break;
        case p2_ALTSN:
            if (0 == IR.op7.src && true == IR.op7.im) {
                cycles = op_ALTSN_D();
                break;
            }
            cycles = op_ALTSN();
            break;
        case p2_ALTGN:
            if (0 == IR.op7.src && true == IR.op7.im) {
                cycles = op_ALTGN_D();
                break;
            }
            cycles = op_ALTGN();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst9_e", "ROLWORD/ALTSN/ALTGN");
        }
        break;

    case p2_ALTSB_ALTGB_ALTSW_ALTGW:
        switch (IR.op9.inst) {
        case p2_ALTSB:
            if (0 == IR.op7.src && true == IR.op7.im) {
                cycles = op_ALTSB_D();
                break;
            }
            cycles = op_ALTSB();
            break;
        case p2_ALTGB:
            if (0 == IR.op7.src && true == IR.op7.im) {
                cycles = op_ALTGB_D();
                break;
            }
            cycles = op_ALTGB();
            break;
        case p2_ALTSW:
            if (0 == IR.op7.src && true == IR.op7.im) {
                cycles = op_ALTSW_D();
                break;
            }
            cycles = op_ALTSW();
            break;
        case p2_ALTGW:
            if (0 == IR.op7.src && true == IR.op7.im) {
                cycles = op_ALTGW_D();
                break;
            }
            cycles = op_ALTGW();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst9_e", "ALTSB/ALTGB/ALTSW/ALTGW");
        }
        break;

    case p2_ALTR_ALTD_ALTS_ALTB:
        switch (IR.op9.inst) {
        case p2_ALTR:
            if (0 == IR.op7.src && true == IR.op7.im) {
                cycles = op_ALTR_D();
                break;
            }
            cycles = op_ALTR();
            break;
        case p2_ALTD:
            if (0 == IR.op7.src && true == IR.op7.im) {
                cycles = op_ALTD_D();
                break;
            }
            cycles = op_ALTD();
            break;
        case p2_ALTS:
            if (0 == IR.op7.src && true == IR.op7.im) {
                cycles = op_ALTS_D();
                break;
            }
            cycles = op_ALTS();
            break;
        case p2_ALTB:
            if (0 == IR.op7.src && true == IR.op7.im) {
                cycles = op_ALTB_D();
                break;
            }
            cycles = op_ALTB();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst9_e", "ALTR/ALTD/ALTS/ALTB");
        }
        break;

    case p2_ALTI_SETR_SETD_SETS:
        switch (IR.op9.inst) {
        case p2_ALTI:
            if (true == IR.op7.im && 0x164 == IR.op7.src /* 101100100 */) {
                cycles = op_ALTI_D();
                break;
            }
            cycles = op_ALTI();
            break;
        case p2_SETR:
            cycles = op_SETR();
            break;
        case p2_SETD:
            cycles = op_SETD();
            break;
        case p2_SETS:
            cycles = op_SETS();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst9_e", "ALTI/SETR/SETD/SETS");
        }
        break;

    case p2_DECOD_BMASK_CRCBIT_CRCNIB:
        switch (IR.op9.inst) {
        case p2_DECOD:
            if (false == IR.op7.im && IR.op7.src == IR.op7.dst) {
                cycles = op_DECOD_D();
                break;
            }
            cycles = op_DECOD();
            break;
        case p2_BMASK:
            if (false == IR.op7.im && IR.op7.src == IR.op7.dst) {
                cycles = op_BMASK_D();
                break;
            }
            cycles = op_BMASK();
            break;
        case p2_CRCBIT:
            cycles = op_CRCBIT();
            break;
        case p2_CRCNIB:
            cycles = op_CRCNIB();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst9_e", "DECOD/BMASK/CRCBIT/CRCNIB");
        }
        break;

    case p2_MUX_NITS_NIBS_Q_MOVBYTS:
        switch (IR.op9.inst) {
        case p2_MUXNITS:
            cycles = op_MUXNITS();
            break;
        case p2_MUXNIBS:
            cycles = op_MUXNIBS();
            break;
        case p2_MUXQ:
            cycles = op_MUXQ();
            break;
        case p2_MOVBYTS:
            cycles = op_MOVBYTS();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst9_e", "MUXNITS/MUXNIBS/MUXQ/MOVBYTS");
        }
        break;

    case p2_MUL_MULS:
        switch (IR.op8.inst) {
        case p2_MUL:
            cycles = op_MUL();
            break;
        case p2_MULS:
            cycles = op_MULS();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst8_e", "MUL/MULS");
        }
        break;

    case p2_SCA_SCAS:
        switch (IR.op8.inst) {
        case p2_SCA:
            cycles = op_SCA();
            break;
        case p2_SCAS:
            cycles = op_SCAS();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst8_e", "SCA/SCAS");
        }
        break;

    case p2_XXXPIX:
        switch (IR.op9.inst) {
        case p2_ADDPIX:
            cycles = op_ADDPIX();
            break;
        case p2_MULPIX:
            cycles = op_MULPIX();
            break;
        case p2_BLNPIX:
            cycles = op_BLNPIX();
            break;
        case p2_MIXPIX:
            cycles = op_MIXPIX();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst9_e", "ADDPIX/MULPIX/BLNPIX/MIXPIX");
        }
        break;

    case p2_WMLONG_ADDCTx:
        switch (IR.op9.inst) {
        case p2_ADDCT1:
            cycles = op_ADDCT1();
            break;
        case p2_ADDCT2:
            cycles = op_ADDCT2();
            break;
        case p2_ADDCT3:
            cycles = op_ADDCT3();
            break;
        case p2_WMLONG:
            cycles = op_WMLONG();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst9_e", "ADDCT1/ADDCT2/ADDCT3/WMLONG");
        }
        break;

    case p2_RQPIN_RDPIN:
        switch (IR.op8.inst) {
        case p2_RQPIN:
            cycles = op_RQPIN();
            break;
        case p2_RDPIN:
            cycles = op_RDPIN();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst8_e", "RQPIN/RDPIN");
        }
        break;

    case p2_RDLUT:
        cycles = op_RDLUT();
        break;

    case p2_RDBYTE:
        cycles = op_RDBYTE();
        break;

    case p2_RDWORD:
        cycles = op_RDWORD();
        break;

    case p2_RDLONG:
        cycles = op_RDLONG();
        break;

    case p2_CALLD:
        cycles = op_CALLD();
        break;

    case p2_CALLPA_CALLPB:
        switch (IR.op8.inst) {
        case p2_CALLPA:
            cycles = op_CALLPA();
            break;
        case p2_CALLPB:
            cycles = op_CALLPB();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst8_e", "CALLPA/CALLPB");
        }
        break;

    case p2_DJZ_DJNZ_DJF_DJNF:
        switch (IR.op9.inst) {
        case p2_DJZ:
            cycles = op_DJZ();
            break;
        case p2_DJNZ:
            cycles = op_DJNZ();
            break;
        case p2_DJF:
            cycles = op_DJF();
            break;
        case p2_DJNF:
            cycles = op_DJNF();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst9_e", "DJZ/DJNZ/DJF/DJNF");
        }
        break;

    case p2_IJZ_IJNZ_TJZ_TJNZ:
        switch (IR.op9.inst) {
        case p2_IJZ:
            cycles = op_IJZ();
            break;
        case p2_IJNZ:
            cycles = op_IJNZ();
            break;
        case p2_TJZ:
            cycles = op_TJZ();
            break;
        case p2_TJNZ:
            cycles = op_TJNZ();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst9_e", "IJZ/IJNZ/TJZ/TJNZ");
        }
        break;

    case p2_TJF_TJNF_TJS_TJNS:
        switch (IR.op9.inst) {
        case p2_TJF:
            cycles = op_TJF();
            break;
        case p2_TJNF:
            cycles = op_TJNF();
            break;
        case p2_TJS:
            cycles = op_TJS();
            break;
        case p2_TJNS:
            cycles = op_TJNS();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst9_e", "TJF/TJNF/TJS/TJNS");
        }
        break;

    case p2_TJV_OPDST_empty:
        if (false == IR.op7.wc) {
            if (false == IR.op7.wz) {
                cycles = op_TJV();
            } else {
                switch (IR.op7.dst) {
                case p2_OPDST_JINT:
                    cycles = op_JINT();
                    break;
                case p2_OPDST_JCT1:
                    cycles = op_JCT1();
                    break;
                case p2_OPDST_JCT2:
                    cycles = op_JCT2();
                    break;
                case p2_OPDST_JCT3:
                    cycles = op_JCT3();
                    break;
                case p2_OPDST_JSE1:
                    cycles = op_JSE1();
                    break;
                case p2_OPDST_JSE2:
                    cycles = op_JSE2();
                    break;
                case p2_OPDST_JSE3:
                    cycles = op_JSE3();
                    break;
                case p2_OPDST_JSE4:
                    cycles = op_JSE4();
                    break;
                case p2_OPDST_JPAT:
                    cycles = op_JPAT();
                    break;
                case p2_OPDST_JFBW:
                    cycles = op_JFBW();
                    break;
                case p2_OPDST_JXMT:
                    cycles = op_JXMT();
                    break;
                case p2_OPDST_JXFI:
                    cycles = op_JXFI();
                    break;
                case p2_OPDST_JXRO:
                    cycles = op_JXRO();
                    break;
                case p2_OPDST_JXRL:
                    cycles = op_JXRL();
                    break;
                case p2_OPDST_JATN:
                    cycles = op_JATN();
                    break;
                case p2_OPDST_JQMT:
                    cycles = op_JQMT();
                    break;
                case p2_OPDST_JNINT:
                    cycles = op_JNINT();
                    break;
                case p2_OPDST_JNCT1:
                    cycles = op_JNCT1();
                    break;
                case p2_OPDST_JNCT2:
                    cycles = op_JNCT2();
                    break;
                case p2_OPDST_JNCT3:
                    cycles = op_JNCT3();
                    break;
                case p2_OPDST_JNSE1:
                    cycles = op_JNSE1();
                    break;
                case p2_OPDST_JNSE2:
                    cycles = op_JNSE2();
                    break;
                case p2_OPDST_JNSE3:
                    cycles = op_JNSE3();
                    break;
                case p2_OPDST_JNSE4:
                    cycles = op_JNSE4();
                    break;
                case p2_OPDST_JNPAT:
                    cycles = op_JNPAT();
                    break;
                case p2_OPDST_JNFBW:
                    cycles = op_JNFBW();
                    break;
                case p2_OPDST_JNXMT:
                    cycles = op_JNXMT();
                    break;
                case p2_OPDST_JNXFI:
                    cycles = op_JNXFI();
                    break;
                case p2_OPDST_JNXRO:
                    cycles = op_JNXRO();
                    break;
                case p2_OPDST_JNXRL:
                    cycles = op_JNXRL();
                    break;
                case p2_OPDST_JNATN:
                    cycles = op_JNATN();
                    break;
                case p2_OPDST_JNQMT:
                    cycles = op_JNQMT();
                    break;
                default:
                    // TODO: invalid D value
                    Q_ASSERT_X(false, "p2_opdst_e", "missing enum value");
                    break;
                }
            }
        } else {
            cycles = op_1011110_1();
        }
        break;

    case p2_empty_SETPAT:
        switch (IR.op8.inst) {
        case p2_1011111_0:
            cycles = op_1011111_0();
            break;
        case p2_SETPAT:
            cycles = op_SETPAT();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst8_e", "1011111_0/SETPAT");
        }
        break;

    case p2_WRPIN_AKPIN_WXPIN:
        switch (IR.op8.inst) {
        case p2_WRPIN:
            if (IR.op7.wz == 1 && IR.op7.dst == 1) {
                cycles = op_AKPIN();
                break;
            }
            cycles = op_WRPIN();
            break;
        case p2_WXPIN:
            cycles = op_WXPIN();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst8_e", "WRPIN/AKPIN/WXPIN");
        }
        break;

    case p2_WYPIN_WRLUT:
        switch (IR.op8.inst) {
        case p2_WYPIN:
            cycles = op_WYPIN();
            break;
        case p2_WRLUT:
            cycles = op_WRLUT();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst8_e", "WYPIN/WRLUT");
        }
        break;

    case p2_WRBYTE_WRWORD:
        switch (IR.op8.inst) {
        case p2_WRBYTE:
            cycles = op_WRBYTE();
            break;
        case p2_WRWORD:
            cycles = op_WRWORD();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst8_e", "WRBYTE/WRWORD");
        }
        break;

    case p2_WRLONG_RDFAST:
        switch (IR.op8.inst) {
        case p2_WRLONG:
            cycles = op_WRLONG();
            break;
        case p2_RDFAST:
            cycles = op_RDFAST();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst8_e", "WRLONG/RDFAST");
        }
        break;

    case p2_WRFAST_FBLOCK:
        switch (IR.op8.inst) {
        case p2_WRFAST:
            cycles = op_WRFAST();
            break;
        case p2_FBLOCK:
            cycles = op_FBLOCK();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst8_e", "WRFAST/FBLOCK");
        }
        break;

    case p2_XINIT_XSTOP_XZERO:
        switch (IR.op8.inst) {
        case p2_XINIT:
            if (IR.op7.wz == 1 && true == IR.op7.im && 0 == IR.op7.src && 0 == IR.op7.dst) {
                cycles = op_XSTOP();
                break;
            }
            cycles = op_XINIT();
            break;
        case p2_XZERO:
            cycles = op_XZERO();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst8_e", "XINIT/XSTOP/XZERO");
        }
        break;

    case p2_XCONT_REP:
        switch (IR.op8.inst) {
        case p2_XCONT:
            cycles = op_XCONT();
            break;
        case p2_REP:
            cycles = op_REP();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst8_e", "XCONT/REP");
        }
        break;

    case p2_COGINIT:
        cycles = op_COGINIT();
        break;

    case p2_QMUL_QDIV:
        switch (IR.op8.inst) {
        case p2_QMUL:
            cycles = op_QMUL();
            break;
        case p2_QDIV:
            cycles = op_QDIV();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst8_e", "QMUL/QDIV");
        }
        break;

    case p2_QFRAC_QSQRT:
        switch (IR.op8.inst) {
        case p2_QFRAC:
            cycles = op_QFRAC();
            break;
        case p2_QSQRT:
            cycles = op_QSQRT();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst8_e", "QFRAC/QSQRT");
        }
        break;

    case p2_QROTATE_QVECTOR:
        switch (IR.op8.inst) {
        case p2_QROTATE:
            cycles = op_QROTATE();
            break;
        case p2_QVECTOR:
            cycles = op_QVECTOR();
            break;
        default:
            Q_ASSERT_X(false, "p2_inst8_e", "QROTATE/QVECTOR");
        }
        break;

    case p2_OPSRC:
        switch (IR.op7.src) {
        case p2_OPSRC_HUBSET:
            cycles = op_HUBSET();
            break;
        case p2_OPSRC_COGID:
            cycles = op_COGID();
            break;
        case p2_OPSRC_COGSTOP:
            cycles = op_COGSTOP();
            break;
        case p2_OPSRC_LOCKNEW:
            cycles = op_LOCKNEW();
            break;
        case p2_OPSRC_LOCKRET:
            cycles = op_LOCKRET();
            break;
        case p2_OPSRC_LOCKTRY:
            cycles = op_LOCKTRY();
            break;
        case p2_OPSRC_LOCKREL:
            cycles = op_LOCKREL();
            break;
        case p2_OPSRC_QLOG:
            cycles = op_QLOG();
            break;
        case p2_OPSRC_QEXP:
            cycles = op_QEXP();
            break;
        case p2_OPSRC_RFBYTE:
            cycles = op_RFBYTE();
            break;
        case p2_OPSRC_RFWORD:
            cycles = op_RFWORD();
            break;
        case p2_OPSRC_RFLONG:
            cycles = op_RFLONG();
            break;
        case p2_OPSRC_RFVAR:
            cycles = op_RFVAR();
            break;
        case p2_OPSRC_RFVARS:
            cycles = op_RFVARS();
            break;
        case p2_OPSRC_WFBYTE:
            cycles = op_WFBYTE();
            break;
        case p2_OPSRC_WFWORD:
            cycles = op_WFWORD();
            break;
        case p2_OPSRC_WFLONG:
            cycles = op_WFLONG();
            break;
        case p2_OPSRC_GETQX:
            cycles = op_GETQX();
            break;
        case p2_OPSRC_GETQY:
            cycles = op_GETQY();
            break;
        case p2_OPSRC_GETCT:
            cycles = op_GETCT();
            break;
        case p2_OPSRC_GETRND:
            if (0 == IR.op7.dst) {
                cycles = op_GETRND_CZ();
                break;
            }
            cycles = op_GETRND();
            break;
        case p2_OPSRC_SETDACS:
            cycles = op_SETDACS();
            break;
        case p2_OPSRC_SETXFRQ:
            cycles = op_SETXFRQ();
            break;
        case p2_OPSRC_GETXACC:
            cycles = op_GETACC();
            break;
        case p2_OPSRC_WAITX:
            cycles = op_WAITX();
            break;
        case p2_OPSRC_SETSE1:
            cycles = op_SETSE1();
            break;
        case p2_OPSRC_SETSE2:
            cycles = op_SETSE2();
            break;
        case p2_OPSRC_SETSE3:
            cycles = op_SETSE3();
            break;
        case p2_OPSRC_SETSE4:
            cycles = op_SETSE4();
            break;
        case p2_OPSRC_X24:
            switch (IR.op7.dst) {
            case p2_OPX24_POLLINT:
                cycles = op_POLLINT();
                break;
            case p2_OPX24_POLLCT1:
                cycles = op_POLLCT1();
                break;
            case p2_OPX24_POLLCT2:
                cycles = op_POLLCT2();
                break;
            case p2_OPX24_POLLCT3:
                cycles = op_POLLCT3();
                break;
            case p2_OPX24_POLLSE1:
                cycles = op_POLLSE1();
                break;
            case p2_OPX24_POLLSE2:
                cycles = op_POLLSE2();
                break;
            case p2_OPX24_POLLSE3:
                cycles = op_POLLSE3();
                break;
            case p2_OPX24_POLLSE4:
                cycles = op_POLLSE4();
                break;
            case p2_OPX24_POLLPAT:
                cycles = op_POLLPAT();
                break;
            case p2_OPX24_POLLFBW:
                cycles = op_POLLFBW();
                break;
            case p2_OPX24_POLLXMT:
                cycles = op_POLLXMT();
                break;
            case p2_OPX24_POLLXFI:
                cycles = op_POLLXFI();
                break;
            case p2_OPX24_POLLXRO:
                cycles = op_POLLXRO();
                break;
            case p2_OPX24_POLLXRL:
                cycles = op_POLLXRL();
                break;
            case p2_OPX24_POLLATN:
                cycles = op_POLLATN();
                break;
            case p2_OPX24_POLLQMT:
                cycles = op_POLLQMT();
                break;
            case p2_OPX24_WAITINT:
                cycles = op_WAITINT();
                break;
            case p2_OPX24_WAITCT1:
                cycles = op_WAITCT1();
                break;
            case p2_OPX24_WAITCT2:
                cycles = op_WAITCT2();
                break;
            case p2_OPX24_WAITCT3:
                cycles = op_WAITCT3();
                break;
            case p2_OPX24_WAITSE1:
                cycles = op_WAITSE1();
                break;
            case p2_OPX24_WAITSE2:
                cycles = op_WAITSE2();
                break;
            case p2_OPX24_WAITSE3:
                cycles = op_WAITSE3();
                break;
            case p2_OPX24_WAITSE4:
                cycles = op_WAITSE4();
                break;
            case p2_OPX24_WAITPAT:
                cycles = op_WAITPAT();
                break;
            case p2_OPX24_WAITFBW:
                cycles = op_WAITFBW();
                break;
            case p2_OPX24_WAITXMT:
                cycles = op_WAITXMT();
                break;
            case p2_OPX24_WAITXFI:
                cycles = op_WAITXFI();
                break;
            case p2_OPX24_WAITXRO:
                cycles = op_WAITXRO();
                break;
            case p2_OPX24_WAITXRL:
                cycles = op_WAITXRL();
                break;
            case p2_OPX24_WAITATN:
                cycles = op_WAITATN();
                break;
            case p2_OPX24_ALLOWI:
                cycles = op_ALLOWI();
                break;
            case p2_OPX24_STALLI:
                cycles = op_STALLI();
                break;
            case p2_OPX24_TRGINT1:
                cycles = op_TRGINT1();
                break;
            case p2_OPX24_TRGINT2:
                cycles = op_TRGINT2();
                break;
            case p2_OPX24_TRGINT3:
                cycles = op_TRGINT3();
                break;
            case p2_OPX24_NIXINT1:
                cycles = op_NIXINT1();
                break;
            case p2_OPX24_NIXINT2:
                cycles = op_NIXINT2();
                break;
            case p2_OPX24_NIXINT3:
                cycles = op_NIXINT3();
                break;
            }
            break;
        case p2_OPSRC_SETINT1:
            cycles = op_SETINT1();
            break;
        case p2_OPSRC_SETINT2:
            cycles = op_SETINT2();
            break;
        case p2_OPSRC_SETINT3:
            cycles = op_SETINT3();
            break;
        case p2_OPSRC_SETQ:
            cycles = op_SETQ();
            break;
        case p2_OPSRC_SETQ2:
            cycles = op_SETQ2();
            break;
        case p2_OPSRC_PUSH:
            cycles = op_PUSH();
            break;
        case p2_OPSRC_POP:
            cycles = op_POP();
            break;
        case p2_OPSRC_JMP:
            cycles = op_JMP();
            break;
        case p2_OPSRC_CALL_RET:
            if (false == IR.op7.im) {
                cycles = op_CALL();
                break;
            }
            cycles = op_RET();
            break;
        case p2_OPSRC_CALLA_RETA:
            if (false == IR.op7.im) {
                cycles = op_CALLA();
                break;
            }
            cycles = op_RETA();
            break;
        case p2_OPSRC_CALLB_RETB:
            if (false == IR.op7.im) {
                cycles = op_CALLB();
                break;
            }
            cycles = op_RETB();
            break;
        case p2_OPSRC_JMPREL:
            cycles = op_JMPREL();
            break;
        case p2_OPSRC_SKIP:
            cycles = op_SKIP();
            break;
        case p2_OPSRC_SKIPF:
            cycles = op_SKIPF();
            break;
        case p2_OPSRC_EXECF:
            cycles = op_EXECF();
            break;
        case p2_OPSRC_GETPTR:
            cycles = op_GETPTR();
            break;
        case p2_OPSRC_COGBRK:
            switch (IR.op9.inst) {
            case p2_COGBRK:
                cycles = op_COGBRK();
                break;
            case p2_GETBRK_WZ:
            case p2_GETBRK_WC:
            case p2_GETBRK_WCZ:
                cycles = op_GETBRK();
                break;
            default:
                Q_ASSERT_X(false, "p2_inst9_e", "COGBRK/GETBRK {WZ,WC,WCZ}");
            }
            break;
        case p2_OPSRC_BRK:
            cycles = op_BRK();
            break;
        case p2_OPSRC_SETLUTS:
            cycles = op_SETLUTS();
            break;
        case p2_OPSRC_SETCY:
            cycles = op_SETCY();
            break;
        case p2_OPSRC_SETCI:
            cycles = op_SETCI();
            break;
        case p2_OPSRC_SETCQ:
            cycles = op_SETCQ();
            break;
        case p2_OPSRC_SETCFRQ:
            cycles = op_SETCFRQ();
            break;
        case p2_OPSRC_SETCMOD:
            cycles = op_SETCMOD();
            break;
        case p2_OPSRC_SETPIV:
            cycles = op_SETPIV();
            break;
        case p2_OPSRC_SETPIX:
            cycles = op_SETPIX();
            break;
        case p2_OPSRC_COGATN:
            cycles = op_COGATN();
            break;
        case p2_OPSRC_TESTP_W_DIRL:
            cycles = (IR.op7.wc != IR.op7.wz) ? op_TESTP_W()
                                            : op_DIRL();
            break;
        case p2_OPSRC_TESTPN_W_DIRH:
            cycles = (IR.op7.wc != IR.op7.wz) ? op_TESTPN_W()
                                            : op_DIRH();
            break;
        case p2_OPSRC_TESTP_AND_DIRC:
            cycles = (IR.op7.wc != IR.op7.wz) ? op_TESTP_AND()
                                            : op_DIRC();
            break;
        case p2_OPSRC_TESTPN_AND_DIRNC:
            cycles = (IR.op7.wc != IR.op7.wz) ? op_TESTPN_AND()
                                            : op_DIRNC();
            break;
        case p2_OPSRC_TESTP_OR_DIRZ:
            cycles = (IR.op7.wc != IR.op7.wz) ? op_TESTP_OR()
                                            : op_DIRZ();
            break;
        case p2_OPSRC_TESTPN_OR_DIRNZ:
            cycles = (IR.op7.wc != IR.op7.wz) ? op_TESTPN_OR()
                                            : op_DIRNZ();
            break;
        case p2_OPSRC_TESTP_XOR_DIRRND:
            cycles = (IR.op7.wc != IR.op7.wz) ? op_TESTP_XOR()
                                            : op_DIRRND();
            break;
        case p2_OPSRC_TESTPN_XOR_DIRNOT:
            cycles = (IR.op7.wc != IR.op7.wz) ? op_TESTPN_XOR()
                                            : op_DIRNOT();
            break;

        case p2_OPSRC_OUTL:
            cycles = op_OUTL();
            break;
        case p2_OPSRC_OUTH:
            cycles = op_OUTH();
            break;
        case p2_OPSRC_OUTC:
            cycles = op_OUTC();
            break;
        case p2_OPSRC_OUTNC:
            cycles = op_OUTNC();
            break;
        case p2_OPSRC_OUTZ:
            cycles = op_OUTZ();
            break;
        case p2_OPSRC_OUTNZ:
            cycles = op_OUTNZ();
            break;
        case p2_OPSRC_OUTRND:
            cycles = op_OUTRND();
            break;
        case p2_OPSRC_OUTNOT:
            cycles = op_OUTNOT();
            break;

        case p2_OPSRC_FLTL:
            cycles = op_FLTL();
            break;
        case p2_OPSRC_FLTH:
            cycles = op_FLTH();
            break;
        case p2_OPSRC_FLTC:
            cycles = op_FLTC();
            break;
        case p2_OPSRC_FLTNC:
            cycles = op_FLTNC();
            break;
        case p2_OPSRC_FLTZ:
            cycles = op_FLTZ();
            break;
        case p2_OPSRC_FLTNZ:
            cycles = op_FLTNZ();
            break;
        case p2_OPSRC_FLTRND:
            cycles = op_FLTRND();
            break;
        case p2_OPSRC_FLTNOT:
            cycles = op_FLTNOT();
            break;

        case p2_OPSRC_DRVL:
            cycles = op_DRVL();
            break;
        case p2_OPSRC_DRVH:
            cycles = op_DRVH();
            break;
        case p2_OPSRC_DRVC:
            cycles = op_DRVC();
            break;
        case p2_OPSRC_DRVNC:
            cycles = op_DRVNC();
            break;
        case p2_OPSRC_DRVZ:
            cycles = op_DRVZ();
            break;
        case p2_OPSRC_DRVNZ:
            cycles = op_DRVNZ();
            break;
        case p2_OPSRC_DRVRND:
            cycles = op_DRVRND();
            break;
        case p2_OPSRC_DRVNOT:
            cycles = op_DRVNOT();
            break;

        case p2_OPSRC_SPLITB:
            cycles = op_SPLITB();
            break;
        case p2_OPSRC_MERGEB:
            cycles = op_MERGEB();
            break;
        case p2_OPSRC_SPLITW:
            cycles = op_SPLITW();
            break;
        case p2_OPSRC_MERGEW:
            cycles = op_MERGEW();
            break;
        case p2_OPSRC_SEUSSF:
            cycles = op_SEUSSF();
            break;
        case p2_OPSRC_SEUSSR:
            cycles = op_SEUSSR();
            break;
        case p2_OPSRC_RGBSQZ:
            cycles = op_RGBSQZ();
            break;
        case p2_OPSRC_RGBEXP:
            cycles = op_RGBEXP();
            break;
        case p2_OPSRC_XORO32:
            cycles = op_XORO32();
            break;
        case p2_OPSRC_REV:
            cycles = op_REV();
            break;
        case p2_OPSRC_RCZR:
            cycles = op_RCZR();
            break;
        case p2_OPSRC_RCZL:
            cycles = op_RCZL();
            break;
        case p2_OPSRC_WRC:
            cycles = op_WRC();
            break;
        case p2_OPSRC_WRNC:
            cycles = op_WRNC();
            break;
        case p2_OPSRC_WRZ:
            cycles = op_WRZ();
            break;
        case p2_OPSRC_WRNZ_MODCZ:
            cycles = (IR.op7.wc || IR.op7.wz) ? op_MODCZ()
                                            : op_WRNZ();
            break;
        case p2_OPSRC_SETSCP:
            cycles = op_SETSCP();
            break;
        case p2_OPSRC_GETSCP:
            cycles = op_GETSCP();
            break;
        }
        break;

    case p2_JMP_ABS:
        cycles = op_JMP_ABS();
        break;

    case p2_CALL_ABS:
        cycles = op_CALL_ABS();
        break;

    case p2_CALLA_ABS:
        cycles = op_CALLA_ABS();
        break;

    case p2_CALLB_ABS:
        cycles = op_CALLB_ABS();
        break;

    case p2_CALLD_ABS_PA:
        cycles = op_CALLD_ABS_PA();
        break;

    case p2_CALLD_ABS_PB:
        cycles = op_CALLD_ABS_PB();
        break;

    case p2_CALLD_ABS_PTRA:
        cycles = op_CALLD_ABS_PTRA();
        break;

    case p2_CALLD_ABS_PTRB:
        cycles = op_CALLD_ABS_PTRB();
        break;

    case p2_LOC_PA:
        cycles = op_LOC_PA();
        break;

    case p2_LOC_PB:
        cycles = op_LOC_PB();
        break;

    case p2_LOC_PTRA:
        cycles = op_LOC_PTRA();
        break;

    case p2_LOC_PTRB:
        cycles = op_LOC_PTRB();
        break;

    case p2_AUGS_00:
    case p2_AUGS_01:
    case p2_AUGS_10:
    case p2_AUGS_11:
        cycles = op_AUGS();
        break;

    case p2_AUGD_00:
    case p2_AUGD_01:
    case p2_AUGD_10:
    case p2_AUGD_11:
        cycles = op_AUGD();
        break;
    }

    // Handle REP instructions
    if (IR.op8.inst != p2_REP && REP_instr.isValid()) {
        p2_LONG instr = REP_instr.toUInt();
        // qDebug("%s: repeat %u instructions %u times", __func__, instr, REP_times);
        if (++REP_offset == instr) {
            if (REP_times == 0 || --REP_times > 0) {
                PC = PC - (PC < 0x400 ? instr : 4 * instr);
                REP_offset = 0;
            }

        }
    }

    return cycles;
}

/**
 * @brief No operation.
 *<pre>
 * 0000 0000000 000 000000000 000000000
 *
 * NOP
 *</pre>
 */
int P2Cog::op_NOP()
{
    return 2;
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
int P2Cog::op_ROR()
{
    if (0 == IR.opcode)
        return op_NOP();
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const p2_QUAD accu = U64(D) << 32 | U64(D);
    const p2_LONG result = U32L(accu >> shift);
    updateC((D & (LSB << shift)) != 0);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_ROL()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const p2_QUAD accu = U64(D) << 32 | U64(D);
    const p2_LONG result = U32H(accu << shift);
    updateC((D & (MSB >> shift)) != 0);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_SHR()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const p2_QUAD accu = U64(D);
    const p2_LONG result = U32L(accu >> shift);
    updateC((D & (LSB << shift)) != 0);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_SHL()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const p2_QUAD accu = U64(D) << 32;
    const p2_LONG result = U32H(accu << shift);
    updateC((D & (MSB >> shift)) != 0);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_RCR()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const p2_QUAD accu = U64(D) | C ? HMAX : 0;
    const p2_LONG result = U32L(accu >> shift);
    updateC((D & (LSB << shift)) != 0);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_RCL()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const p2_QUAD accu = U64(D) << 32 | C ? LMAX : 0;
    const p2_LONG result = U32H(accu << shift);
    updateC((D & (MSB >> shift)) != 0);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_SAR()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const p2_QUAD accu = U64(D) | (D & MSB) ? HMAX : 0;
    const p2_LONG result = U32L(accu >> shift);
    updateC((D & (LSB << shift)) != 0);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_SAL()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const p2_QUAD accu = U64(D) << 32 | (D & LSB) ? LMAX : 0;
    const p2_LONG result = U32H(accu << shift);
    updateC((D & (MSB >> shift)) != 0);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_ADD()
{
    augmentS(IR.op7.im);
    const p2_QUAD accu = U64(D) + U64(S);
    const p2_LONG result = U32L(accu);
    updateC((accu >> 32) & 1);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_ADDX()
{
    augmentS(IR.op7.im);
    const p2_QUAD accu = U64(D) + U64(S) + C;
    const p2_LONG result = U32L(accu);
    updateC((accu >> 32) & 1);
    updateZ(Z & (result == 0));
    updateD(result);
    return 2;
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
int P2Cog::op_ADDS()
{
    augmentS(IR.op7.im);
    const bool sign = (S32(D) ^ S32(S)) < 0;
    const qint64 accu = SX64(D) + SX64(S);
    const p2_LONG result = U32L(accu);
    updateC((accu < 0) ^ sign);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_ADDSX()
{
    augmentS(IR.op7.im);
    const uchar sign = (D ^ (S + C)) >> 31;
    const qint64 accu = SX64(D) + SX64(S) + C;
    const p2_LONG result = U32L(accu);
    updateC((accu < 0) ^ sign);
    updateZ(Z & (result == 0));
    updateD(result);
    return 2;
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
int P2Cog::op_SUB()
{
    augmentS(IR.op7.im);
    const p2_QUAD accu = U64(D) - U64(S);
    const p2_LONG result = U32L(accu);
    updateC((accu >> 32) & 1);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_SUBX()
{
    augmentS(IR.op7.im);
    const p2_QUAD accu = U64(D) - (U64(S) + C);
    const p2_LONG result = U32L(accu);
    updateC((accu >> 32) & 1);
    updateZ(Z & (result == 0));
    updateD(result);
    return 2;
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
int P2Cog::op_SUBS()
{
    augmentS(IR.op7.im);
    const bool sign = (S32(D) ^ S32(S)) < 0;
    const qint64 accu = SX64(D) - SX64(S);
    const p2_LONG result = U32L(accu);
    updateC((accu < 0) ^ sign);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_SUBSX()
{
    augmentS(IR.op7.im);
    const uchar sign = (D ^ (S + C)) >> 31;
    const qint64 accu = SX64(D) - (SX64(S) + C);
    const p2_LONG result = U32L(accu);
    updateC((accu < 0) ^ sign);
    updateZ(Z & (result == 0));
    updateD(result);
    return 2;
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
int P2Cog::op_CMP()
{
    augmentS(IR.op7.im);
    const p2_QUAD accu = U64(D) - U64(S);
    const p2_LONG result = U32L(accu);
    updateC((accu >> 32) & 1);
    updateZ(0 == result);
    return 2;
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
int P2Cog::op_CMPX()
{
    augmentS(IR.op7.im);
    const p2_QUAD accu = U64(D) - (U64(S) + C);
    const p2_LONG result = U32L(accu);
    updateC((accu >> 32) & 1);
    updateZ(Z & (result == 0));
    return 2;
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
int P2Cog::op_CMPS()
{
    augmentS(IR.op7.im);
    const bool sign = (S32(D) ^ S32(S)) < 0;
    const qint64 accu = SX64(D) - SX64(S);
    const p2_LONG result = U32L(accu);
    updateC((accu < 0) ^ sign);
    updateZ(0 == result);
    return 2;
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
int P2Cog::op_CMPSX()
{
    augmentS(IR.op7.im);
    const uchar sign = (D ^ (S + C)) >> 31;
    const qint64 accu = SX64(D) - (SX64(S) + C);
    const p2_LONG result = U32L(accu);
    updateC((accu < 0) ^ sign);
    updateZ(Z & (result == 0));
    return 2;
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
int P2Cog::op_CMPR()
{
    augmentS(IR.op7.im);
    const p2_QUAD accu = U64(S) - U64(D);
    const p2_LONG result = U32L(accu);
    updateC((accu >> 32) & 1);
    updateZ(0 == result);
    return 2;
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
int P2Cog::op_CMPM()
{
    augmentS(IR.op7.im);
    const p2_QUAD accu = U64(D) - U64(S);
    const p2_LONG result = U32L(accu);
    updateC((accu >> 31) & 1);
    updateZ(0 == result);
    return 2;
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
int P2Cog::op_SUBR()
{
    augmentS(IR.op7.im);
    const p2_QUAD accu = U64(S) - U64(D);
    const p2_LONG result = U32L(accu);
    updateC((accu >> 32) & 1);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_CMPSUB()
{
    augmentS(IR.op7.im);
    if (D < S) {
        // Do not change D
        const p2_LONG result = D;
        updateC(0);
        updateZ(0 == result);
    } else {
        // Do the subtract and set C = 1, if WC is set
        const p2_QUAD accu = U64(D) - U64(S);
        const p2_LONG result = U32L(accu);
        updateC(1);
        updateZ(0 == result);
        updateD(result);
    }
    return 2;
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
int P2Cog::op_FGE()
{
    augmentS(IR.op7.im);
    if (D < S) {
        const p2_LONG result = S;
        updateC(1);
        updateZ(0 == result);
        updateD(result);
    } else {
        const p2_LONG result = D;
        updateC(0);
        updateZ(0 == result);
    }
    return 2;
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
int P2Cog::op_FLE()
{
    augmentS(IR.op7.im);
    if (D > S) {
        const p2_LONG result = S;
        updateC(1);
        updateZ(0 == result);
        updateD(result);
    } else {
        const p2_LONG result = D;
        updateC(0);
        updateZ(0 == result);
    }
    return 2;
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
int P2Cog::op_FGES()
{
    augmentS(IR.op7.im);
    if (S32(D) < S32(S)) {
        const p2_LONG result = S;
        updateC(1);
        updateZ(0 == result);
        updateD(result);
    } else {
        const p2_LONG result = D;
        updateC(0);
        updateZ(0 == result);
    }
    return 2;
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
int P2Cog::op_FLES()
{
    augmentS(IR.op7.im);
    if (S32(D) > S32(S)) {
        const p2_LONG result = S;
        updateC(1);
        updateZ(0 == result);
        updateD(result);
    } else {
        const p2_LONG result = D;
        updateC(0);
        updateZ(0 == result);
    }
    return 2;
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
int P2Cog::op_SUMC()
{
    augmentS(IR.op7.im);
    const bool sign = (S32(D) ^ S32(S)) < 0;
    const p2_QUAD accu = C ? U64(D) - U64(S) : U64(D) + U64(S);
    const p2_LONG result = U32L(accu);
    updateC(((accu >> 32) & 1) ^ sign);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_SUMNC()
{
    augmentS(IR.op7.im);
    const bool sign = (S32(D) ^ S32(S)) < 0;
    const p2_QUAD accu = !C ? U64(D) - U64(S) : U64(D) + U64(S);
    const p2_LONG result = U32L(accu);
    updateC(((accu >> 32) & 1) ^ sign);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_SUMZ()
{
    augmentS(IR.op7.im);
    const bool sign = (S32(D) ^ S32(S)) < 0;
    const p2_QUAD accu = Z ? U64(D) - U64(S) : U64(D) + U64(S);
    const p2_LONG result = U32L(accu);
    updateC(((accu >> 32) & 1) ^ sign);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_SUMNZ()
{
    augmentS(IR.op7.im);
    const bool sign = (S32(D) ^ S32(S)) < 0;
    const p2_QUAD accu = !Z ? U64(D) - U64(S) : U64(D) + U64(S);
    const p2_LONG result = U32L(accu);
    updateC(((accu >> 32) & 1) ^ sign);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_TESTB_W()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const uchar bit = (D >> shift) & 1;
    updateC(bit);
    updateZ(bit);
    return 2;
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
int P2Cog::op_TESTBN_W()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const uchar bit = (~D >> shift) & 1;
    updateC(bit);
    updateZ(bit);
    return 2;
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
int P2Cog::op_TESTB_AND()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const uchar bit = (D >> shift) & 1;
    updateC(C & bit);
    updateZ(Z & bit);
    return 2;
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
int P2Cog::op_TESTBN_AND()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const uchar bit = (~D >> shift) & 1;
    updateC(C & bit);
    updateZ(Z & bit);
    return 2;
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
int P2Cog::op_TESTB_OR()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const uchar bit = (D >> shift) & 1;
    updateC(C | bit);
    updateZ(Z | bit);
    return 2;
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
int P2Cog::op_TESTBN_OR()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const uchar bit = (~D >> shift) & 1;
    updateC(C | bit);
    updateZ(Z | bit);
    return 2;
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
int P2Cog::op_TESTB_XOR()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const uchar bit = (D >> shift) & 1;
    updateC(C ^ bit);
    updateZ(Z ^ bit);
    return 2;
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
int P2Cog::op_TESTBN_XOR()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const uchar bit = (~D >> shift) & 1;
    updateC(C ^ bit);
    updateZ(Z ^ bit);
    return 2;
}

/**
 * @brief Bit S[4:0] of D = 0,    C,Z = D[S[4:0]].
 *<pre>
 * EEEE 0100000 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITL    D,{#}S         {WCZ}
 *</pre>
 */
int P2Cog::op_BITL()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const p2_LONG bit = LSB << shift;
    const p2_LONG result = D & ~bit;
    updateC((result >> shift) & 1);
    updateZ((result >> shift) & 1);
    return 2;
}

/**
 * @brief Bit S[4:0] of D = 1,    C,Z = D[S[4:0]].
 *<pre>
 * EEEE 0100001 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITH    D,{#}S         {WCZ}
 *</pre>
 */
int P2Cog::op_BITH()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const p2_LONG bit = LSB << shift;
    const p2_LONG result = D | bit;
    updateC((result >> shift) & 1);
    updateZ((result >> shift) & 1);
    return 2;
}

/**
 * @brief Bit S[4:0] of D = C,    C,Z = D[S[4:0]].
 *<pre>
 * EEEE 0100010 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITC    D,{#}S         {WCZ}
 *</pre>
 */
int P2Cog::op_BITC()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const p2_LONG bit = LSB << shift;
    const p2_LONG result = C ? D | bit : D & ~bit;
    updateC((result >> shift) & 1);
    updateZ((result >> shift) & 1);
    return 2;
}

/**
 * @brief Bit S[4:0] of D = !C,   C,Z = D[S[4:0]].
 *<pre>
 * EEEE 0100011 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNC   D,{#}S         {WCZ}
 *</pre>
 */
int P2Cog::op_BITNC()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const p2_LONG bit = LSB << shift;
    const p2_LONG result = !C ? D | bit : D & ~bit;
    updateC((result >> shift) & 1);
    updateZ((result >> shift) & 1);
    return 2;
}

/**
 * @brief Bit S[4:0] of D = Z,    C,Z = D[S[4:0]].
 *<pre>
 * EEEE 0100100 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITZ    D,{#}S         {WCZ}
 *</pre>
 */
int P2Cog::op_BITZ()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const p2_LONG bit = LSB << shift;
    const p2_LONG result = Z ? D | bit : D & ~bit;
    updateC((result >> shift) & 1);
    updateZ((result >> shift) & 1);
    return 2;
}

/**
 * @brief Bit S[4:0] of D = !Z,   C,Z = D[S[4:0]].
 *<pre>
 * EEEE 0100101 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNZ   D,{#}S         {WCZ}
 *</pre>
 */
int P2Cog::op_BITNZ()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const p2_LONG bit = LSB << shift;
    const p2_LONG result = !Z ? D | bit : D & ~bit;
    updateC((result >> shift) & 1);
    updateZ((result >> shift) & 1);
    return 2;
}

/**
 * @brief Bit S[4:0] of D = RND,  C,Z = D[S[4:0]].
 *<pre>
 * EEEE 0100110 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITRND  D,{#}S         {WCZ}
 *</pre>
 */
int P2Cog::op_BITRND()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const p2_LONG bit = LSB << shift;
    const p2_LONG result = (HUB->random(shift) & 1) ? D | bit : D & ~bit;
    updateC((result >> shift) & 1);
    updateZ((result >> shift) & 1);
    return 2;
}

/**
 * @brief Bit S[4:0] of D = !bit, C,Z = D[S[4:0]].
 *<pre>
 * EEEE 0100111 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNOT  D,{#}S         {WCZ}
 *</pre>
 */
int P2Cog::op_BITNOT()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const p2_LONG bit = LSB << shift;
    const p2_LONG result = D ^ bit;
    updateC((result >> shift) & 1);
    updateZ((result >> shift) & 1);
    return 2;
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
int P2Cog::op_AND()
{
    augmentS(IR.op7.im);
    const p2_LONG result = D & S;
    updateC(P2Util::parity(result));
    updateZ(0 == result);
    return 2;
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
int P2Cog::op_ANDN()
{
    augmentS(IR.op7.im);
    const p2_LONG result = D & ~S;
    updateC(P2Util::parity(result));
    updateZ(0 == result);
    return 2;
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
int P2Cog::op_OR()
{
    augmentS(IR.op7.im);
    const p2_LONG result = D | S;
    updateC(P2Util::parity(result));
    updateZ(0 == result);
    return 2;
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
int P2Cog::op_XOR()
{
    augmentS(IR.op7.im);
    const p2_LONG result = D ^ S;
    updateC(P2Util::parity(result));
    updateZ(0 == result);
    return 2;
}

/**
 * @brief Mux  C into each D bit that is '1' in S.
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
int P2Cog::op_MUXC()
{
    augmentS(IR.op7.im);
    const p2_LONG result = (D & ~S) | (C ? S : 0);
    updateC(P2Util::parity(result));
    updateZ(0 == result);
    return 2;
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
int P2Cog::op_MUXNC()
{
    augmentS(IR.op7.im);
    const p2_LONG result = (D & ~S) | (!C ? S : 0);
    updateC(P2Util::parity(result));
    updateZ(0 == result);
    return 2;
}

/**
 * @brief Mux  Z into each D bit that is '1' in S.
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
int P2Cog::op_MUXZ()
{
    augmentS(IR.op7.im);
    const p2_LONG result = (D & ~S) | (Z ? S : 0);
    updateC(P2Util::parity(result));
    updateZ(0 == result);
    return 2;
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
int P2Cog::op_MUXNZ()
{
    augmentS(IR.op7.im);
    const p2_LONG result = (D & ~S) | (!Z ? S : 0);
    updateC(P2Util::parity(result));
    updateZ(0 == result);
    return 2;
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
int P2Cog::op_MOV()
{
    augmentS(IR.op7.im);
    const p2_LONG result = S;
    updateC(result >> 31);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_NOT()
{
    augmentS(IR.op7.im);
    const p2_LONG result = ~S;
    updateC(result >> 31);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_ABS()
{
    augmentS(IR.op7.im);
    const qint32 result = qAbs(S32(S));
    updateC(S >> 31);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_NEG()
{
    augmentS(IR.op7.im);
    const qint32 result = 0 - S32(S);
    updateC(result >> 31);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_NEGC()
{
    augmentS(IR.op7.im);
    const qint32 result = C ? 0 - S32(S) : S32(32);
    updateC(result >> 31);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_NEGNC()
{
    augmentS(IR.op7.im);
    const qint32 result = !C ? 0 - S32(S) : S32(32);
    updateC(result >> 31);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_NEGZ()
{
    augmentS(IR.op7.im);
    const qint32 result = Z ? 0 - S32(S) : S32(32);
    updateC(result >> 31);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_NEGNZ()
{
    augmentS(IR.op7.im);
    const qint32 result = !Z ? 0 - S32(S) : S32(32);
    updateC(result >> 31);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_INCMOD()
{
    augmentS(IR.op7.im);
    const p2_LONG result = (D == S) ? 0 : D + 1;
    updateC(result == 0);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_DECMOD()
{
    augmentS(IR.op7.im);
    const p2_LONG result = (D == 0) ? S : D - 1;
    updateC(result == S);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_ZEROX()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const p2_LONG msb = (D >> (shift - 1)) & 1;
    const p2_LONG mask = 0xffffffffu << shift;
    const p2_LONG result = D & ~mask;
    updateC(msb);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_SIGNX()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const p2_LONG msb = (D >> (shift - 1)) & 1;
    const p2_LONG mask = FULL << shift;
    const p2_LONG result = msb ? D | mask : D & ~mask;
    updateC(msb);
    updateZ(0 == result);
    updateD(result);
    return 2;
}

/**
 * @brief Get bit position of top-most '1' in S into D.
 *<pre>
 * EEEE 0111100 CZI DDDDDDDDD SSSSSSSSS
 *
 * ENCOD   D,{#}S   {WC/WZ/WCZ}
 *
 * D = position of top '1' in S (0 … 31).
 * C = (S != 0).
 * Z = (result == 0).
 *</pre>
 */
int P2Cog::op_ENCOD()
{
    augmentS(IR.op7.im);
    const p2_LONG result = static_cast<p2_LONG>(P2Util::encode(S));
    updateC(S != 0);
    updateZ(0 == result);
    updateD(result);
    return 2;
}

/**
 * @brief Get number of '1's in S into D.
 *<pre>
 * EEEE 0111101 CZI DDDDDDDDD SSSSSSSSS
 *
 * ONES    D,{#}S   {WC/WZ/WCZ}
 *
 * D = number of '1's in S (0 … 32).
 * C = LSB of result.
 * Z = (result == 0).
 *</pre>
 */
int P2Cog::op_ONES()
{
    augmentS(IR.op7.im);
    const p2_LONG result = P2Util::ones(S);
    updateC(result & 1);
    updateZ(0 == result);
    return 2;
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
int P2Cog::op_TEST()
{
    augmentS(IR.op7.im);
    const p2_LONG result = D & S;
    updateC(P2Util::parity(result));
    updateZ(0 == result);
    return 2;
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
int P2Cog::op_TESTN()
{
    augmentS(IR.op7.im);
    const p2_LONG result = D & ~S;
    updateC(P2Util::parity(result));
    updateZ(0 == result);
    return 2;
}

/**
 * @brief Set S[3:0] into nibble N in D, keeping rest of D same.
 *<pre>
 * EEEE 100000N NNI DDDDDDDDD SSSSSSSSS
 *
 * SETNIB  D,{#}S,#N
 *</pre>
 */
int P2Cog::op_SETNIB()
{
    augmentS(IR.op7.im);
    const uchar shift = static_cast<uchar>((IR.opcode >> 19) & 7) * 4;
    const p2_LONG mask = LNIBBLE << shift;
    const p2_LONG result = (D & ~mask) | ((S << shift) & mask);
    updateD(result);
    return 2;
}

/**
 * @brief Set S[3:0] into nibble established by prior ALTSN instruction.
 *<pre>
 * EEEE 1000000 00I 000000000 SSSSSSSSS
 *
 * SETNIB  {#}S
 *</pre>
 */
int P2Cog::op_SETNIB_ALTSN()
{
    augmentS(IR.op7.im);
    return 2;
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
int P2Cog::op_GETNIB()
{
    augmentS(IR.op7.im);
    const uchar shift = static_cast<uchar>((IR.opcode >> 19) & 7) * 4;
    const p2_LONG result = (S >> shift) & LNIBBLE;
    updateD(result);
    return 2;
}

/**
 * @brief Get nibble established by prior ALTGN instruction into D.
 *<pre>
 * EEEE 1000010 000 DDDDDDDDD 000000000
 *
 * GETNIB  D
 *</pre>
 */
int P2Cog::op_GETNIB_ALTGN()
{
    augmentS(IR.op7.im);
    return 2;
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
int P2Cog::op_ROLNIB()
{
    augmentS(IR.op7.im);
    const uchar shift = static_cast<uchar>((IR.opcode >> 19) & 7) * 4;
    const p2_LONG result = (D << 4) | ((S >> shift) & LNIBBLE);
    updateD(result);
    return 2;
}

/**
 * @brief Rotate-left nibble established by prior ALTGN instruction into D.
 *<pre>
 * EEEE 1000100 000 DDDDDDDDD 000000000
 *
 * ROLNIB  D
 *</pre>
 */
int P2Cog::op_ROLNIB_ALTGN()
{
    augmentS(IR.op7.im);
    return 2;
}

/**
 * @brief Set S[7:0] into byte N in D, keeping rest of D same.
 *<pre>
 * EEEE 1000110 NNI DDDDDDDDD SSSSSSSSS
 *
 * SETBYTE D,{#}S,#N
 *</pre>
 */
int P2Cog::op_SETBYTE()
{
    augmentS(IR.op7.im);
    const uchar shift = static_cast<uchar>((IR.opcode >> 19) & 3) * 8;
    const p2_LONG mask = LBYTE << shift;
    const p2_LONG result = (D & ~mask) | ((S << shift) & mask);
    updateD(result);
    return 2;
}

/**
 * @brief Set S[7:0] into byte established by prior ALTSB instruction.
 *<pre>
 * EEEE 1000110 00I 000000000 SSSSSSSSS
 *
 * SETBYTE {#}S
 *</pre>
 */
int P2Cog::op_SETBYTE_ALTSB()
{
    augmentS(IR.op7.im);
    return 2;
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
int P2Cog::op_GETBYTE()
{
    augmentS(IR.op7.im);
    const uchar shift = static_cast<uchar>((IR.opcode >> 19) & 3) * 8;
    const p2_LONG result = (S >> shift) & LBYTE;
    updateD(result);
    return 2;
}

/**
 * @brief Get byte established by prior ALTGB instruction into D.
 *<pre>
 * EEEE 1000111 000 DDDDDDDDD 000000000
 *
 * GETBYTE D
 *</pre>
 */
int P2Cog::op_GETBYTE_ALTGB()
{
    augmentS(IR.op7.im);
    return 2;
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
int P2Cog::op_ROLBYTE()
{
    augmentS(IR.op7.im);
    const uchar shift = static_cast<uchar>((IR.opcode >> 19) & 3) * 8;
    const p2_LONG result = (D << 8) | ((S >> shift) & LBYTE);
    updateD(result);
    return 2;
}

/**
 * @brief Rotate-left byte established by prior ALTGB instruction into D.
 *<pre>
 * EEEE 1001000 000 DDDDDDDDD 000000000
 *
 * ROLBYTE D
 *</pre>
 */
int P2Cog::op_ROLBYTE_ALTGB()
{
    augmentS(IR.op7.im);
    return 2;
}

/**
 * @brief Set S[15:0] into word N in D, keeping rest of D same.
 *<pre>
 * EEEE 1001001 0NI DDDDDDDDD SSSSSSSSS
 *
 * SETWORD D,{#}S,#N
 *</pre>
 */
int P2Cog::op_SETWORD()
{
    augmentS(IR.op7.im);
    const uchar shift = IR.op7.wz ? 16 : 0;
    const p2_LONG mask = LWORD << shift;
    const p2_LONG result = (D & ~mask) | ((S >> shift) & mask);
    updateD(result);
    return 2;
}

/**
 * @brief Set S[15:0] into word established by prior ALTSW instruction.
 *<pre>
 * EEEE 1001001 00I 000000000 SSSSSSSSS
 *
 * SETWORD {#}S
 *</pre>
 */
int P2Cog::op_SETWORD_ALTSW()
{
    augmentS(IR.op7.im);
    return 2;
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
int P2Cog::op_GETWORD()
{
    augmentS(IR.op7.im);
    const uchar shift = IR.op7.wz * 16;
    const p2_LONG result = (S >> shift) & LWORD;
    updateD(result);
    return 2;
}

/**
 * @brief Get word established by prior ALTGW instruction into D.
 *<pre>
 * EEEE 1001001 100 DDDDDDDDD 000000000
 *
 * GETWORD D
 *</pre>
 */
int P2Cog::op_GETWORD_ALTGW()
{
    return 2;
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
int P2Cog::op_ROLWORD()
{
    augmentS(IR.op7.im);
    const uchar shift = IR.op7.wz * 16;
    const p2_LONG result = (D << 16) & ((S >> shift) & LWORD);
    updateD(result);
    return 2;
}

/**
 * @brief Rotate-left word established by prior ALTGW instruction into D.
 *<pre>
 * EEEE 1001010 000 DDDDDDDDD 000000000
 *
 * ROLWORD D
 *</pre>
 */
int P2Cog::op_ROLWORD_ALTGW()
{
    return 2;
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
int P2Cog::op_ALTSN()
{
    augmentS(IR.op7.im);
    return 2;
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
int P2Cog::op_ALTSN_D()
{
    return 2;
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
int P2Cog::op_ALTGN()
{
    augmentS(IR.op7.im);
    return 2;
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
int P2Cog::op_ALTGN_D()
{
    return 2;
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
int P2Cog::op_ALTSB()
{
    augmentS(IR.op7.im);
    return 2;
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
int P2Cog::op_ALTSB_D()
{
    return 2;
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
int P2Cog::op_ALTGB()
{
    augmentS(IR.op7.im);
    return 2;
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
int P2Cog::op_ALTGB_D()
{
    return 2;
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
int P2Cog::op_ALTSW()
{
    augmentS(IR.op7.im);
    return 2;
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
int P2Cog::op_ALTSW_D()
{
    return 2;
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
int P2Cog::op_ALTGW()
{
    augmentS(IR.op7.im);
    return 2;
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
int P2Cog::op_ALTGW_D()
{
    return 2;
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
int P2Cog::op_ALTR()
{
    augmentS(IR.op7.im);
    return 2;
}

/**
 * @brief Alter result register address (normally D field) of next instruction to D[8:0].
 *<pre>
 * EEEE 1001100 001 DDDDDDDDD 000000000
 *
 * ALTR    D
 *</pre>
 */
int P2Cog::op_ALTR_D()
{
    return 2;
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
int P2Cog::op_ALTD()
{
    augmentS(IR.op7.im);
    return 2;
}

/**
 * @brief Alter D field of next instruction to D[8:0].
 *<pre>
 * EEEE 1001100 011 DDDDDDDDD 000000000
 *
 * ALTD    D
 *</pre>
 */
int P2Cog::op_ALTD_D()
{
    return 2;
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
int P2Cog::op_ALTS()
{
    augmentS(IR.op7.im);
    return 2;
}

/**
 * @brief Alter S field of next instruction to D[8:0].
 *<pre>
 * EEEE 1001100 101 DDDDDDDDD 000000000
 *
 * ALTS    D
 *</pre>
 */
int P2Cog::op_ALTS_D()
{
    return 2;
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
int P2Cog::op_ALTB()
{
    augmentS(IR.op7.im);
    return 2;
}

/**
 * @brief Alter D field of next instruction to D[13:5].
 *<pre>
 * EEEE 1001100 111 DDDDDDDDD 000000000
 *
 * ALTB    D
 *</pre>
 */
int P2Cog::op_ALTB_D()
{
    return 2;
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
int P2Cog::op_ALTI()
{
    augmentS(IR.op7.im);
    return 2;
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
int P2Cog::op_ALTI_D()
{
    return 2;
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
int P2Cog::op_SETR()
{
    augmentS(IR.op7.im);
    return 2;
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
int P2Cog::op_SETD()
{
    augmentS(IR.op7.im);
    return 2;
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
int P2Cog::op_SETS()
{
    augmentS(IR.op7.im);
    return 2;
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
int P2Cog::op_DECOD()
{
    augmentS(IR.op7.im);
    const uchar shift = S & 31;
    const p2_LONG result = LSB << shift;
    updateD(result);
    return 2;
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
int P2Cog::op_DECOD_D()
{
    return 2;
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
int P2Cog::op_BMASK()
{
    const uchar shift = S & 31;
    const p2_LONG result = U32L((Q_UINT64_C(2) << shift) - 1);
    updateD(result);
    augmentS(IR.op7.im);
    return 2;
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
int P2Cog::op_BMASK_D()
{
    const uchar shift = D & 31;
    const p2_LONG result = U32L((Q_UINT64_C(2) << shift) - 1);
    updateD(result);
    return 2;
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
int P2Cog::op_CRCBIT()
{
    augmentS(IR.op7.im);
    const p2_LONG result = (D >> 1) ^ (S & SXn<p2_LONG,1>(C ^ D));
    updateC(D & 1);
    updateD(result);
    return 2;
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
int P2Cog::op_CRCNIB()
{
    augmentS(IR.op7.im);
    const p2_LONG q0 = Q >> 28;
    const p2_LONG c0 = C;
    const p2_LONG q1 = (q0 >> 1) ^ (S & SXn<p2_LONG,1>(c0 ^ q0));
    const p2_LONG c1 = q0 & 1;
    const p2_LONG q2 = (q1 >> 1) ^ (S & SXn<p2_LONG,1>(c1 ^ q1));
    const p2_LONG c2 = q1 & 1;
    const p2_LONG q3 = (q2 >> 1) ^ (S & SXn<p2_LONG,1>(c2 ^ q2));
    const p2_LONG c3 = q1 & 1;
    const p2_LONG q4 = (q3 >> 1) ^ (S & SXn<p2_LONG,1>(c3 ^ q3));
    updateQ(Q << 4);
    updateD(D ^ q4);
    updateC(c3);
    return 2;
}

/**
 * @brief For each non-zero bit pair in S, copy that bit pair into the corresponding D bits, else leave that D bit pair the same.
 *<pre>
 * EEEE 1001111 00I DDDDDDDDD SSSSSSSSS
 *
 * MUXNITS D,{#}S
 *</pre>
 */
int P2Cog::op_MUXNITS()
{
    augmentS(IR.op7.im);
    const p2_LONG mask = S | ((S & 0xaaaaaaaa) >> 1) | ((S & 0x55555555) << 1);
    const p2_LONG result = (D & ~mask) | (S & mask);
    updateD(result);
    return 2;
}

/**
 * @brief For each non-zero nibble in S, copy that nibble into the corresponding D nibble, else leave that D nibble the same.
 *<pre>
 * EEEE 1001111 01I DDDDDDDDD SSSSSSSSS
 *
 * MUXNIBS D,{#}S
 *</pre>
 */
int P2Cog::op_MUXNIBS()
{
    augmentS(IR.op7.im);
    const p2_LONG mask0 = S | ((S & 0xaaaaaaaa) >> 1) | ((S & 0x55555555) << 1);
    const p2_LONG mask1 = ((mask0 & 0xcccccccc) >> 2) | ((mask0 & 0x33333333) << 2);
    const p2_LONG result = (D & ~mask1) | (S & mask1);
    updateD(result);
    return 2;
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
int P2Cog::op_MUXQ()
{
    augmentS(IR.op7.im);
    const p2_LONG result = (D & ~Q) | (S & Q);
    updateD(result);
    return 2;
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
int P2Cog::op_MOVBYTS()
{
    augmentS(IR.op7.im);
    union {
        p2_LONG l;
        p2_BYTE b[4];
    }   src, dst;
    src.l = D;
    dst.b[0] = src.b[(S >> 0) & 3];
    dst.b[1] = src.b[(S >> 2) & 3];
    dst.b[2] = src.b[(S >> 4) & 3];
    dst.b[3] = src.b[(S >> 6) & 3];
    updateD(dst.l);
    return 2;
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
int P2Cog::op_MUL()
{
    augmentS(IR.op7.im);
    const p2_LONG result = U16(D) * U16(S);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_MULS()
{
    augmentS(IR.op7.im);
    const p2_LONG result = static_cast<p2_LONG>(S16(D) * S16(S));
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_SCA()
{
    augmentS(IR.op7.im);
    const p2_LONG result = (U16(D) * U16(S)) >> 16;
    updateZ(0 == result);
    S_next = result;
    return 2;
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
int P2Cog::op_SCAS()
{
    augmentS(IR.op7.im);
    const p2_LONG result = static_cast<p2_LONG>((S16(D) * S16(S)) >> 14);
    updateZ(0 == result);
    S_next = result;
    return 2;
}

/**
 * @brief Add bytes of S into bytes of D, with $FF saturation.
 *<pre>
 * EEEE 1010010 00I DDDDDDDDD SSSSSSSSS
 *
 * ADDPIX  D,{#}S
 *</pre>
 */
int P2Cog::op_ADDPIX()
{
    augmentS(IR.op7.im);
    union {
        p2_LONG l;
        p2_BYTE b[4];
    }   dst, src;
    dst.l = D;
    src.l = S;
    dst.b[0] = qMin<p2_BYTE>(dst.b[0] + src.b[0], 0xff);
    dst.b[1] = qMin<p2_BYTE>(dst.b[1] + src.b[1], 0xff);
    dst.b[2] = qMin<p2_BYTE>(dst.b[2] + src.b[2], 0xff);
    dst.b[3] = qMin<p2_BYTE>(dst.b[3] + src.b[3], 0xff);
    updateD(dst.l);
    return 2;
}

/**
 * @brief Multiply bytes of S into bytes of D, where $FF = 1.
 *<pre>
 * EEEE 1010010 01I DDDDDDDDD SSSSSSSSS
 *
 * MULPIX  D,{#}S
 *</pre>
 */
int P2Cog::op_MULPIX()
{
    augmentS(IR.op7.im);
    return 2;
}

/**
 * @brief Alpha-blend bytes of S into bytes of D, using SETPIV value.
 *<pre>
 * EEEE 1010010 10I DDDDDDDDD SSSSSSSSS
 *
 * BLNPIX  D,{#}S
 *</pre>
 */
int P2Cog::op_BLNPIX()
{
    augmentS(IR.op7.im);
    return 2;
}

/**
 * @brief Mix bytes of S into bytes of D, using SETPIX and SETPIV values.
 *<pre>
 * EEEE 1010010 11I DDDDDDDDD SSSSSSSSS
 *
 * MIXPIX  D,{#}S
 *</pre>
 */
int P2Cog::op_MIXPIX()
{
    augmentS(IR.op7.im);
    return 2;
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
int P2Cog::op_ADDCT1()
{
    augmentS(IR.op7.im);
    return 2;
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
int P2Cog::op_ADDCT2()
{
    augmentS(IR.op7.im);
    return 2;
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
int P2Cog::op_ADDCT3()
{
    augmentS(IR.op7.im);
    return 2;
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
int P2Cog::op_WMLONG()
{
    augmentS(IR.op7.im);
    return 2;
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
int P2Cog::op_RQPIN()
{
    augmentS(IR.op7.im);
    return 2;
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
int P2Cog::op_RDPIN()
{
    augmentS(IR.op7.im);
    return 2;
}

/**
 * @brief Read LUT data from address S[8:0] into D.
 *<pre>
 * EEEE 1010101 CZI DDDDDDDDD SSSSSSSSS
 *
 * RDLUT   D,{#}S/P   {WC/WZ/WCZ}
 *
 * C = MSB of data.
 * Z = (result == 0).
 *</pre>
 */
int P2Cog::op_RDLUT()
{
    augmentS(IR.op7.im);
    p2_LONG result;
    if (S == offs_PTRA) {
        result = HUB->rd_LONG(COG.REG.PTRA);
    } else if (S == offs_PTRB) {
        result = HUB->rd_LONG(COG.REG.PTRB);
    } else {
        result = LUT.RAM[S & 0x1ff];
    }
    updateC((result >> 31) & 1);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_RDBYTE()
{
    augmentS(IR.op7.im);
    p2_BYTE result;
    if (S == offs_PTRA) {
        result = HUB->rd_BYTE(COG.REG.PTRA);
    } else if (S == offs_PTRB) {
        result = HUB->rd_BYTE(COG.REG.PTRB);
    } else {
        result = HUB->rd_BYTE(S);
    }
    updateC((result >> 7) & 1);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_RDWORD()
{
    augmentS(IR.op7.im);
    p2_WORD result;
    if (S == offs_PTRA) {
        result = HUB->rd_WORD(COG.REG.PTRA);
    } else if (S == offs_PTRB) {
        result = HUB->rd_WORD(COG.REG.PTRB);
    } else {
        result = HUB->rd_WORD(S);
    }
    updateC((result >> 15) & 1);
    updateZ(0 == result);
    updateD(result);
    return 2;
}

/**
 * @brief Read long from hub address {#}S/PTRx into D.
 *<pre>
 * EEEE 1011000 CZI DDDDDDDDD SSSSSSSSS
 *
 * RDLONG  D,{#}S/P {WC/WZ/WCZ}
 *
 * C = MSB of long.
 * rior SETQ/SETQ2 invokes cog/LUT block transfer.
 *</pre>
 */
int P2Cog::op_RDLONG()
{
    augmentS(IR.op7.im);
    p2_LONG result;
    if (S == offs_PTRA) {
        result = HUB->rd_LONG(COG.REG.PTRA);
    } else if (S == offs_PTRB) {
        result = HUB->rd_LONG(COG.REG.PTRB);
    } else {
        result = HUB->rd_LONG(S);
    }
    updateC((result >> 15) & 1);
    updateZ(0 == result);
    updateD(result);
    return 2;
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
int P2Cog::op_POPA()
{
    return 2;
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
int P2Cog::op_POPB()
{
    return 2;
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
int P2Cog::op_CALLD()
{
    augmentS(IR.op7.im);
    if (IR.op7.wc && IR.op7.wz) {
        if (IR.op7.dst == offs_IJMP3 && IR.op7.src == offs_IRET3)
            return op_RESI3();
        if (IR.op7.dst == offs_IJMP2 && IR.op7.src == offs_IRET2)
            return op_RESI2();
        if (IR.op7.dst == offs_IJMP1 && IR.op7.src == offs_IRET1)
            return op_RESI1();
        if (IR.op7.dst == offs_INA && IR.op7.src == offs_INB)
            return op_RESI0();
        if (IR.op7.dst == offs_INB && IR.op7.src == offs_IRET3)
            return op_RETI3();
        if (IR.op7.dst == offs_INB && IR.op7.src == offs_IRET2)
            return op_RETI2();
        if (IR.op7.dst == offs_INB && IR.op7.src == offs_IRET1)
            return op_RETI1();
        if (IR.op7.dst == offs_INB && IR.op7.src == offs_INB)
            return op_RETI0();
    }
    const p2_LONG result = (U32(C) << 31) | (U32(Z) << 30) | PC;
    updateC((S >> 31) & 1);
    updateZ((S >> 30) & 1);
    updateD(result);
    updatePC(S & A20MASK);
    return 2;
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
int P2Cog::op_RESI3()
{
    return 2;
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
int P2Cog::op_RESI2()
{
    return 2;
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
int P2Cog::op_RESI1()
{
    return 2;
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
int P2Cog::op_RESI0()
{
    return 2;
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
int P2Cog::op_RETI3()
{
    return 2;
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
int P2Cog::op_RETI2()
{
    return 2;
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
int P2Cog::op_RETI1()
{
    return 2;
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
int P2Cog::op_RETI0()
{
    return 2;
}

/**
 * @brief Call to S** by pushing {C, Z, 10'b0, PC[19:0]} onto stack, copy D to PA.
 *<pre>
 * EEEE 1011010 0LI DDDDDDDDD SSSSSSSSS
 *
 * CALLPA  {#}D,{#}S
 *
 *</pre>
 */
int P2Cog::op_CALLPA()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    const p2_LONG stack = (C << 31) | (Z << 30) | PC;
    const p2_LONG address = S;
    const p2_LONG result = D;
    pushK(stack);
    updatePA(result);
    updatePC(address);
    return 2;
}

/**
 * @brief Call to S** by pushing {C, Z, 10'b0, PC[19:0]} onto stack, copy D to PB.
 *<pre>
 * EEEE 1011010 1LI DDDDDDDDD SSSSSSSSS
 *
 * CALLPB  {#}D,{#}S
 *
 *</pre>
 */
int P2Cog::op_CALLPB()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    const p2_LONG stack = (C << 31) | (Z << 30) | PC;
    const p2_LONG address = S;
    const p2_LONG result = D;
    pushK(stack);
    updatePB(result);
    updatePC(address);
    return 2;
}

/**
 * @brief Decrement D and jump to S** if result is zero.
 *<pre>
 * EEEE 1011011 00I DDDDDDDDD SSSSSSSSS
 *
 * DJZ     D,{#}S
 *
 *</pre>
 */
int P2Cog::op_DJZ()
{
    augmentS(IR.op7.im);
    const p2_LONG result = D - 1;
    const p2_LONG address = S;
    if (result == ZERO)
        updatePC(address);
    return 2;
}

/**
 * @brief Decrement D and jump to S** if result is not zero.
 *<pre>
 * EEEE 1011011 01I DDDDDDDDD SSSSSSSSS
 *
 * DJNZ    D,{#}S
 *
 *</pre>
 */
int P2Cog::op_DJNZ()
{
    augmentS(IR.op7.im);
    const p2_LONG result = D - 1;
    const p2_LONG address = S;
    if (result != ZERO)
        updatePC(address);
    return 2;
}

/**
 * @brief Decrement D and jump to S** if result is $FFFF_FFFF.
 *<pre>
 * EEEE 1011011 10I DDDDDDDDD SSSSSSSSS
 *
 * DJF     D,{#}S
 *
 *</pre>
 */
int P2Cog::op_DJF()
{
    augmentS(IR.op7.im);
    const p2_LONG result = D - 1;
    const p2_LONG address = S;
    if (result == FULL)
        updatePC(address);
    return 2;
}

/**
 * @brief Decrement D and jump to S** if result is not $FFFF_FFFF.
 *<pre>
 * EEEE 1011011 11I DDDDDDDDD SSSSSSSSS
 *
 * DJNF    D,{#}S
 *
 *</pre>
 */
int P2Cog::op_DJNF()
{
    augmentS(IR.op7.im);
    const p2_LONG result = D - 1;
    const p2_LONG address = S;
    if (result != FULL)
        updatePC(address);
    return 2;
}

/**
 * @brief Increment D and jump to S** if result is zero.
 *<pre>
 * EEEE 1011100 00I DDDDDDDDD SSSSSSSSS
 *
 * IJZ     D,{#}S
 *
 *</pre>
 */
int P2Cog::op_IJZ()
{
    augmentS(IR.op7.im);
    const p2_LONG result = D + 1;
    const p2_LONG address = S;
    if (result == ZERO)
        updatePC(address);
    return 2;
}

/**
 * @brief Increment D and jump to S** if result is not zero.
 *<pre>
 * EEEE 1011100 01I DDDDDDDDD SSSSSSSSS
 *
 * IJNZ    D,{#}S
 *
 *</pre>
 */
int P2Cog::op_IJNZ()
{
    augmentS(IR.op7.im);
    const p2_LONG result = D + 1;
    const p2_LONG address = S;
    if (result != ZERO)
        updatePC(address);
    return 2;
}

/**
 * @brief Test D and jump to S** if D is zero.
 *<pre>
 * EEEE 1011100 10I DDDDDDDDD SSSSSSSSS
 *
 * TJZ     D,{#}S
 *
 *</pre>
 */
int P2Cog::op_TJZ()
{
    augmentS(IR.op7.im);
    const p2_LONG result = D;
    const p2_LONG address = S;
    if (result == ZERO)
        updatePC(address);
    return 2;
}

/**
 * @brief Test D and jump to S** if D is not zero.
 *<pre>
 * EEEE 1011100 11I DDDDDDDDD SSSSSSSSS
 *
 * TJNZ    D,{#}S
 *
 *</pre>
 */
int P2Cog::op_TJNZ()
{
    augmentS(IR.op7.im);
    const p2_LONG result = D;
    const p2_LONG address = S;
    if (result != ZERO)
        updatePC(address);
    return 2;
}

/**
 * @brief Test D and jump to S** if D is full (D = $FFFF_FFFF).
 *<pre>
 * EEEE 1011101 00I DDDDDDDDD SSSSSSSSS
 *
 * TJF     D,{#}S
 *
 *</pre>
 */
int P2Cog::op_TJF()
{
    augmentS(IR.op7.im);
    const p2_LONG result = D;
    const p2_LONG address = S;
    if (result == FULL)
        updatePC(address);
    return 2;
}

/**
 * @brief Test D and jump to S** if D is not full (D != $FFFF_FFFF).
 *<pre>
 * EEEE 1011101 01I DDDDDDDDD SSSSSSSSS
 *
 * TJNF    D,{#}S
 *
 *</pre>
 */
int P2Cog::op_TJNF()
{
    augmentS(IR.op7.im);
    const p2_LONG result = D;
    const p2_LONG address = S;
    if (result != FULL)
        updatePC(address);
    return 2;
}

/**
 * @brief Test D and jump to S** if D is signed (D[31] = 1).
 *<pre>
 * EEEE 1011101 10I DDDDDDDDD SSSSSSSSS
 *
 * TJS     D,{#}S
 *
 *</pre>
 */
int P2Cog::op_TJS()
{
    augmentS(IR.op7.im);
    const p2_LONG result = D & MSB;
    const p2_LONG address = S;
    if (result)
        updatePC(address);
    return 2;
}

/**
 * @brief Test D and jump to S** if D is not signed (D[31] = 0).
 *<pre>
 * EEEE 1011101 11I DDDDDDDDD SSSSSSSSS
 *
 * TJNS    D,{#}S
 *
 *</pre>
 */
int P2Cog::op_TJNS()
{
    augmentS(IR.op7.im);
    const p2_LONG result = D & MSB;
    const p2_LONG address = S;
    if (!result)
        updatePC(address);
    return 2;
}

/**
 * @brief Test D and jump to S** if D overflowed (D[31] != C, C = 'correct sign' from last addition/subtraction).
 *<pre>
 * EEEE 1011110 00I DDDDDDDDD SSSSSSSSS
 *
 * TJV     D,{#}S
 *
 *</pre>
 */
int P2Cog::op_TJV()
{
    augmentS(IR.op7.im);
    const p2_LONG result = (D >> 31) ^ C;
    const p2_LONG address = S;
    if (result)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if INT event flag is set.
 *<pre>
 * EEEE 1011110 01I 000000000 SSSSSSSSS
 *
 * JINT    {#}S
 *
 *</pre>
 */
int P2Cog::op_JINT()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (FLAGS.f_INT)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if CT1 event flag is set.
 *<pre>
 * EEEE 1011110 01I 000000001 SSSSSSSSS
 *
 * JCT1    {#}S
 *
 *</pre>
 */
int P2Cog::op_JCT1()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (FLAGS.f_CT1)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if CT2 event flag is set.
 *<pre>
 * EEEE 1011110 01I 000000010 SSSSSSSSS
 *
 * JCT2    {#}S
 *
 *</pre>
 */
int P2Cog::op_JCT2()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (FLAGS.f_CT2)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if CT3 event flag is set.
 *<pre>
 * EEEE 1011110 01I 000000011 SSSSSSSSS
 *
 * JCT3    {#}S
 *
 *</pre>
 */
int P2Cog::op_JCT3()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (FLAGS.f_CT3)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if SE1 event flag is set.
 *<pre>
 * EEEE 1011110 01I 000000100 SSSSSSSSS
 *
 * JSE1    {#}S
 *
 *</pre>
 */
int P2Cog::op_JSE1()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (FLAGS.f_SE1)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if SE2 event flag is set.
 *<pre>
 * EEEE 1011110 01I 000000101 SSSSSSSSS
 *
 * JSE2    {#}S
 *
 *</pre>
 */
int P2Cog::op_JSE2()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (FLAGS.f_SE2)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if SE3 event flag is set.
 *<pre>
 * EEEE 1011110 01I 000000110 SSSSSSSSS
 *
 * JSE3    {#}S
 *
 *</pre>
 */
int P2Cog::op_JSE3()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (FLAGS.f_SE3)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if SE4 event flag is set.
 *<pre>
 * EEEE 1011110 01I 000000111 SSSSSSSSS
 *
 * JSE4    {#}S
 *
 *</pre>
 */
int P2Cog::op_JSE4()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (FLAGS.f_SE4)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if PAT event flag is set.
 *<pre>
 * EEEE 1011110 01I 000001000 SSSSSSSSS
 *
 * JPAT    {#}S
 *
 *</pre>
 */
int P2Cog::op_JPAT()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (FLAGS.f_PAT)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if FBW event flag is set.
 *<pre>
 * EEEE 1011110 01I 000001001 SSSSSSSSS
 *
 * JFBW    {#}S
 *
 *</pre>
 */
int P2Cog::op_JFBW()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (FLAGS.f_FBW)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if XMT event flag is set.
 *<pre>
 * EEEE 1011110 01I 000001010 SSSSSSSSS
 *
 * JXMT    {#}S
 *
 *</pre>
 */
int P2Cog::op_JXMT()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (FLAGS.f_XMT)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if XFI event flag is set.
 *<pre>
 * EEEE 1011110 01I 000001011 SSSSSSSSS
 *
 * JXFI    {#}S
 *
 *</pre>
 */
int P2Cog::op_JXFI()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (FLAGS.f_XFI)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if XRO event flag is set.
 *<pre>
 * EEEE 1011110 01I 000001100 SSSSSSSSS
 *
 * JXRO    {#}S
 *
 *</pre>
 */
int P2Cog::op_JXRO()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (FLAGS.f_XRO)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if XRL event flag is set.
 *<pre>
 * EEEE 1011110 01I 000001101 SSSSSSSSS
 *
 * JXRL    {#}S
 *
 *</pre>
 */
int P2Cog::op_JXRL()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (FLAGS.f_XRL)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if ATN event flag is set.
 *<pre>
 * EEEE 1011110 01I 000001110 SSSSSSSSS
 *
 * JATN    {#}S
 *
 *</pre>
 */
int P2Cog::op_JATN()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (FLAGS.f_ATN)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if QMT event flag is set.
 *<pre>
 * EEEE 1011110 01I 000001111 SSSSSSSSS
 *
 * JQMT    {#}S
 *
 *</pre>
 */
int P2Cog::op_JQMT()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (FLAGS.f_QMT)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if INT event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000010000 SSSSSSSSS
 *
 * JNINT   {#}S
 *
 *</pre>
 */
int P2Cog::op_JNINT()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (!FLAGS.f_INT)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if CT1 event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000010001 SSSSSSSSS
 *
 * JNCT1   {#}S
 *
 *</pre>
 */
int P2Cog::op_JNCT1()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (!FLAGS.f_CT1)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if CT2 event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000010010 SSSSSSSSS
 *
 * JNCT2   {#}S
 *
 *</pre>
 */
int P2Cog::op_JNCT2()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (!FLAGS.f_CT2)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if CT3 event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000010011 SSSSSSSSS
 *
 * JNCT3   {#}S
 *
 *</pre>
 */
int P2Cog::op_JNCT3()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (!FLAGS.f_CT3)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if SE1 event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000010100 SSSSSSSSS
 *
 * JNSE1   {#}S
 *
 *</pre>
 */
int P2Cog::op_JNSE1()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (!FLAGS.f_SE1)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if SE2 event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000010101 SSSSSSSSS
 *
 * JNSE2   {#}S
 *
 *</pre>
 */
int P2Cog::op_JNSE2()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (!FLAGS.f_SE2)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if SE3 event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000010110 SSSSSSSSS
 *
 * JNSE3   {#}S
 *
 *</pre>
 */
int P2Cog::op_JNSE3()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (!FLAGS.f_SE3)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if SE4 event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000010111 SSSSSSSSS
 *
 * JNSE4   {#}S
 *
 *</pre>
 */
int P2Cog::op_JNSE4()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (!FLAGS.f_SE4)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if PAT event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000011000 SSSSSSSSS
 *
 * JNPAT   {#}S
 *
 *</pre>
 */
int P2Cog::op_JNPAT()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (!FLAGS.f_PAT)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if FBW event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000011001 SSSSSSSSS
 *
 * JNFBW   {#}S
 *
 *</pre>
 */
int P2Cog::op_JNFBW()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (!FLAGS.f_FBW)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if XMT event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000011010 SSSSSSSSS
 *
 * JNXMT   {#}S
 *
 *</pre>
 */
int P2Cog::op_JNXMT()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (!FLAGS.f_XMT)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if XFI event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000011011 SSSSSSSSS
 *
 * JNXFI   {#}S
 *
 *</pre>
 */
int P2Cog::op_JNXFI()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (!FLAGS.f_XFI)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if XRO event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000011100 SSSSSSSSS
 *
 * JNXRO   {#}S
 *
 *</pre>
 */
int P2Cog::op_JNXRO()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (!FLAGS.f_XRO)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if XRL event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000011101 SSSSSSSSS
 *
 * JNXRL   {#}S
 *
 *</pre>
 */
int P2Cog::op_JNXRL()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (!FLAGS.f_XRL)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if ATN event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000011110 SSSSSSSSS
 *
 * JNATN   {#}S
 *
 *</pre>
 */
int P2Cog::op_JNATN()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (!FLAGS.f_ATN)
        updatePC(address);
    return 2;
}

/**
 * @brief Jump to S** if QMT event flag is clear.
 *<pre>
 * EEEE 1011110 01I 000011111 SSSSSSSSS
 *
 * JNQMT   {#}S
 *
 *</pre>
 */
int P2Cog::op_JNQMT()
{
    augmentS(IR.op7.im);
    const p2_LONG address = S;
    if (!FLAGS.f_QMT)
        updatePC(address);
    return 2;
}

/**
 * @brief <empty>.
 *<pre>
 * EEEE 1011110 1LI DDDDDDDDD SSSSSSSSS
 *
 * <empty> {#}D,{#}S
 *
 *</pre>
 */
int P2Cog::op_1011110_1()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    return 2;
}

/**
 * @brief <empty>.
 *<pre>
 * EEEE 1011111 0LI DDDDDDDDD SSSSSSSSS
 *
 * <empty> {#}D,{#}S
 *
 *</pre>
 */
int P2Cog::op_1011111_0()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    return 2;
}

/**
 * @brief Set pin pattern for PAT event.
 *<pre>
 * EEEE 1011111 BEI DDDDDDDDD SSSSSSSSS
 *
 * SETPAT  {#}D,{#}S
 *
 * C selects INA/INB, Z selects =/!=, D provides mask value, S provides match value.
 *</pre>
 */
int P2Cog::op_SETPAT()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.im);
    PAT.mode = IR.op7.wc ? (IR.op7.wz ? p2_PAT_PB_EQ : p2_PAT_PB_NE)
                        : (IR.op7.wz ? p2_PAT_PA_EQ : p2_PAT_PA_NE);
    PAT.mask = D;
    PAT.match = S;
    return 2;
}

/**
 * @brief Write D to mode register of smart pin S[5:0], acknowledge smart pin.
 *<pre>
 * EEEE 1100000 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRPIN   {#}D,{#}S
 *
 *</pre>
 */
int P2Cog::op_WRPIN()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    return 2;
}

/**
 * @brief Acknowledge smart pin S[5:0].
 *<pre>
 * EEEE 1100000 01I 000000001 SSSSSSSSS
 *
 * AKPIN   {#}S
 *
 *</pre>
 */
int P2Cog::op_AKPIN()
{
    augmentS(IR.op7.im);
    return 2;
}

/**
 * @brief Write D to parameter "X" of smart pin S[5:0], acknowledge smart pin.
 *<pre>
 * EEEE 1100000 1LI DDDDDDDDD SSSSSSSSS
 *
 * WXPIN   {#}D,{#}S
 *
 *</pre>
 */
int P2Cog::op_WXPIN()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    return 2;
}

/**
 * @brief Write D to parameter "Y" of smart pin S[5:0], acknowledge smart pin.
 *<pre>
 * EEEE 1100001 0LI DDDDDDDDD SSSSSSSSS
 *
 * WYPIN   {#}D,{#}S
 *
 *</pre>
 */
int P2Cog::op_WYPIN()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    return 2;
}

/**
 * @brief Write D to LUT address S[8:0].
 *<pre>
 * EEEE 1100001 1LI DDDDDDDDD SSSSSSSSS
 *
 * WRLUT   {#}D,{#}S
 *
 *</pre>
 */
int P2Cog::op_WRLUT()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    const p2_LONG address = D;
    const p2_LONG result = S;
    updateLUT(address, result);
    return 2;
}

/**
 * @brief Write byte in D[7:0] to hub address {#}S/PTRx.
 *<pre>
 * EEEE 1100010 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRBYTE  {#}D,{#}S/P
 *
 *</pre>
 */
int P2Cog::op_WRBYTE()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    p2_LONG address = get_pointer(S, 2);
    p2_BYTE result = static_cast<p2_BYTE>(D);
    HUB->wr_BYTE(address, result);
    return 2;
}

/**
 * @brief Write word in D[15:0] to hub address {#}S/PTRx.
 *<pre>
 * EEEE 1100010 1LI DDDDDDDDD SSSSSSSSS
 *
 * WRWORD  {#}D,{#}S/P
 *
 *</pre>
 */
int P2Cog::op_WRWORD()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    p2_LONG address = get_pointer(S, 1);
    p2_WORD result = static_cast<p2_WORD>(D);
    HUB->wr_WORD(address, result);
    return 2;
}

/**
 * @brief Write long in D[31:0] to hub address {#}S/PTRx.
 *<pre>
 * EEEE 1100011 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRLONG  {#}D,{#}S/P
 *
 * Prior SETQ/SETQ2 invokes cog/LUT block transfer.
 *</pre>
 */
int P2Cog::op_WRLONG()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    p2_LONG address = get_pointer(S, 0);
    p2_LONG result = D;
    HUB->wr_BYTE(address, result);
    return 2;
}

/**
 * @brief Write long in D[31:0] to hub address PTRA++.
 *<pre>
 * EEEE 1100011 0L1 DDDDDDDDD 101100001
 *
 * PUSHA   {#}D
 *
 *</pre>
 */
int P2Cog::op_PUSHA()
{
    augmentD(IR.op7.wz);
    p2_LONG result = D;
    pushPTRA(result);
    return 2;
}

/**
 * @brief Write long in D[31:0] to hub address PTRB++.
 *<pre>
 * EEEE 1100011 0L1 DDDDDDDDD 111100001
 *
 * PUSHB   {#}D
 *
 *</pre>
 */
int P2Cog::op_PUSHB()
{
    augmentD(IR.op7.wz);
    p2_LONG result = D;
    pushPTRB(result);
    return 2;
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
int P2Cog::op_RDFAST()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    return 2;
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
int P2Cog::op_WRFAST()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    return 2;
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
int P2Cog::op_FBLOCK()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    return 2;
}

/**
 * @brief Issue streamer command immediately, zeroing phase.
 *<pre>
 * EEEE 1100101 0LI DDDDDDDDD SSSSSSSSS
 *
 * XINIT   {#}D,{#}S
 *
 *</pre>
 */
int P2Cog::op_XINIT()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    return 2;
}

/**
 * @brief Stop streamer immediately.
 *<pre>
 * EEEE 1100101 011 000000000 000000000
 *
 * XSTOP
 *
 *</pre>
 */
int P2Cog::op_XSTOP()
{
    return 2;
}

/**
 * @brief Buffer new streamer command to be issued on final NCO rollover of current command, zeroing phase.
 *<pre>
 * EEEE 1100101 1LI DDDDDDDDD SSSSSSSSS
 *
 * XZERO   {#}D,{#}S
 *
 *</pre>
 */
int P2Cog::op_XZERO()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    return 2;
}

/**
 * @brief Buffer new streamer command to be issued on final NCO rollover of current command, continuing phase.
 *<pre>
 * EEEE 1100110 0LI DDDDDDDDD SSSSSSSSS
 *
 * XCONT   {#}D,{#}S
 *
 *</pre>
 */
int P2Cog::op_XCONT()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    return 2;
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
int P2Cog::op_REP()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    updateREP(D, S);
    return 2;
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
int P2Cog::op_COGINIT()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    Q_ASSERT(HUB);
    HUB->coginit(D, S, Q);
    return 2;
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
int P2Cog::op_QMUL()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    return 2;
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
int P2Cog::op_QDIV()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    return 2;
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
int P2Cog::op_QFRAC()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    return 2;
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
int P2Cog::op_QSQRT()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    return 2;
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
int P2Cog::op_QROTATE()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    return 2;
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
int P2Cog::op_QVECTOR()
{
    augmentS(IR.op7.im);
    augmentD(IR.op7.wz);
    return 2;
}

/**
 * @brief Set hub configuration to D.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000000000
 *
 * HUBSET  {#}D
 *
 *</pre>
 */
int P2Cog::op_HUBSET()
{
    augmentD(IR.op7.im);
    return 2;
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
int P2Cog::op_COGID()
{
    augmentD(IR.op7.im);
    return 2;
}

/**
 * @brief Stop cog D[3:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000000011
 *
 * COGSTOP {#}D
 *
 *</pre>
 */
int P2Cog::op_COGSTOP()
{
    augmentD(IR.op7.im);
    return 2;
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
int P2Cog::op_LOCKNEW()
{
    return 2;
}

/**
 * @brief Return LOCK D[3:0] for reallocation.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000000101
 *
 * LOCKRET {#}D
 *
 *</pre>
 */
int P2Cog::op_LOCKRET()
{
    augmentD(IR.op7.im);
    return 2;
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
int P2Cog::op_LOCKTRY()
{
    augmentD(IR.op7.im);
    return 2;
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
int P2Cog::op_LOCKREL()
{
    augmentD(IR.op7.im);
    return 2;
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
int P2Cog::op_QLOG()
{
    augmentD(IR.op7.im);
    return 2;
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
int P2Cog::op_QEXP()
{
    augmentD(IR.op7.im);
    return 2;
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
int P2Cog::op_RFBYTE()
{
    return 2;
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
int P2Cog::op_RFWORD()
{
    return 2;
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
int P2Cog::op_RFLONG()
{
    return 2;
}

/**
 * @brief Read zero-extended 1 … 4-byte value from FIFO into D. Used after RDFAST.
 *<pre>
 * EEEE 1101011 CZ0 DDDDDDDDD 000010011
 *
 * RFVAR   D        {WC/WZ/WCZ}
 *
 * C = 0.
 * Z = (result == 0).
 *</pre>
 */
int P2Cog::op_RFVAR()
{
    return 2;
}

/**
 * @brief Read sign-extended 1 … 4-byte value from FIFO into D. Used after RDFAST.
 *<pre>
 * EEEE 1101011 CZ0 DDDDDDDDD 000010100
 *
 * RFVARS  D        {WC/WZ/WCZ}
 *
 * C = MSB of value.
 * Z = (result == 0).
 *</pre>
 */
int P2Cog::op_RFVARS()
{
    return 2;
}

/**
 * @brief Write byte in D[7:0] into FIFO. Used after WRFAST.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000010101
 *
 * WFBYTE  {#}D
 *
 *</pre>
 */
int P2Cog::op_WFBYTE()
{
    return 2;
}

/**
 * @brief Write word in D[15:0] into FIFO. Used after WRFAST.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000010110
 *
 * WFWORD  {#}D
 *
 *</pre>
 */
int P2Cog::op_WFWORD()
{
    return 2;
}

/**
 * @brief Write long in D[31:0] into FIFO. Used after WRFAST.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000010111
 *
 * WFLONG  {#}D
 *
 *</pre>
 */
int P2Cog::op_WFLONG()
{
    return 2;
}

/**
 * @brief Retrieve CORDIC result X into D.
 *<pre>
 * EEEE 1101011 CZ0 DDDDDDDDD 000011000
 *
 * GETQX   D        {WC/WZ/WCZ}
 *
 * Waits, in case result:
        op_not ready.();
        return;
 * C = X[31].
 * Z = (result == 0).
 *</pre>
 */
int P2Cog::op_GETQX()
{
    return 2;
}

/**
 * @brief Retrieve CORDIC result Y into D.
 *<pre>
 * EEEE 1101011 CZ0 DDDDDDDDD 000011001
 *
 * GETQY   D        {WC/WZ/WCZ}
 *
 * Waits, in case result:
        op_not ready.();
        return;
 * C = Y[31].
 * Z = (result == 0).
 *</pre>
 */
int P2Cog::op_GETQY()
{
    return 2;
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
int P2Cog::op_GETCT()
{
    return 2;
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
int P2Cog::op_GETRND()
{
    p2_LONG result = HUB->random(2*ID);
    updateC((result >> 31) & 1);
    updateZ((result >> 30) & 1);
    updateD(result);
    return 2;
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
int P2Cog::op_GETRND_CZ()
{
    p2_LONG result = HUB->random(2*ID);
    updateC((result >> 31) & 1);
    updateZ((result >> 30) & 1);
    return 2;
}

/**
 * @brief DAC3 = D[31:24], DAC2 = D[23:16], DAC1 = D[15:8], DAC0 = D[7:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000011100
 *
 * SETDACS {#}D
 *
 *</pre>
 */
int P2Cog::op_SETDACS()
{
    augmentD(IR.op7.im);
    return 2;
}

/**
 * @brief Set streamer NCO frequency to D.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000011101
 *
 * SETXFRQ {#}D
 *
 *</pre>
 */
int P2Cog::op_SETXFRQ()
{
    augmentD(IR.op7.im);
    return 2;
}

/**
 * @brief Get the streamer's Goertzel X accumulator into D and the Y accumulator into the next instruction's S, clear accumulators.
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 000011110
 *
 * GETXACC D
 *
 *</pre>
 */
int P2Cog::op_GETACC()
{
    return 2;
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
int P2Cog::op_WAITX()
{
    augmentD(IR.op7.im);
    return 2;
}

/**
 * @brief Set SE1 event configuration to D[8:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000100000
 *
 * SETSE1  {#}D
 *
 *</pre>
 */
int P2Cog::op_SETSE1()
{
    augmentD(IR.op7.im);
    return 2;
}

/**
 * @brief Set SE2 event configuration to D[8:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000100001
 *
 * SETSE2  {#}D
 *
 *</pre>
 */
int P2Cog::op_SETSE2()
{
    augmentD(IR.op7.im);
    return 2;
}

/**
 * @brief Set SE3 event configuration to D[8:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000100010
 *
 * SETSE3  {#}D
 *
 *</pre>
 */
int P2Cog::op_SETSE3()
{
    augmentD(IR.op7.im);
    return 2;
}

/**
 * @brief Set SE4 event configuration to D[8:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000100011
 *
 * SETSE4  {#}D
 *
 *</pre>
 */
int P2Cog::op_SETSE4()
{
    augmentD(IR.op7.im);
    return 2;
}

/**
 * @brief Get INT event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000000000 000100100
 *
 * POLLINT          {WC/WZ/WCZ}
 *
 *</pre>
 */
int P2Cog::op_POLLINT()
{
    return 2;
}

/**
 * @brief Get CT1 event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000000001 000100100
 *
 * POLLCT1          {WC/WZ/WCZ}
 *
 *</pre>
 */
int P2Cog::op_POLLCT1()
{
    return 2;
}

/**
 * @brief Get CT2 event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000000010 000100100
 *
 * POLLCT2          {WC/WZ/WCZ}
 *
 *</pre>
 */
int P2Cog::op_POLLCT2()
{
    return 2;
}

/**
 * @brief Get CT3 event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000000011 000100100
 *
 * POLLCT3          {WC/WZ/WCZ}
 *
 *</pre>
 */
int P2Cog::op_POLLCT3()
{
    return 2;
}

/**
 * @brief Get SE1 event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000000100 000100100
 *
 * POLLSE1          {WC/WZ/WCZ}
 *
 *</pre>
 */
int P2Cog::op_POLLSE1()
{
    return 2;
}

/**
 * @brief Get SE2 event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000000101 000100100
 *
 * POLLSE2          {WC/WZ/WCZ}
 *
 *</pre>
 */
int P2Cog::op_POLLSE2()
{
    return 2;
}

/**
 * @brief Get SE3 event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000000110 000100100
 *
 * POLLSE3          {WC/WZ/WCZ}
 *
 *</pre>
 */
int P2Cog::op_POLLSE3()
{
    return 2;
}

/**
 * @brief Get SE4 event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000000111 000100100
 *
 * POLLSE4          {WC/WZ/WCZ}
 *
 *</pre>
 */
int P2Cog::op_POLLSE4()
{
    return 2;
}

/**
 * @brief Get PAT event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000001000 000100100
 *
 * POLLPAT          {WC/WZ/WCZ}
 *
 *</pre>
 */
int P2Cog::op_POLLPAT()
{
    return 2;
}

/**
 * @brief Get FBW event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000001001 000100100
 *
 * POLLFBW          {WC/WZ/WCZ}
 *
 *</pre>
 */
int P2Cog::op_POLLFBW()
{
    return 2;
}

/**
 * @brief Get XMT event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000001010 000100100
 *
 * POLLXMT          {WC/WZ/WCZ}
 *
 *</pre>
 */
int P2Cog::op_POLLXMT()
{
    return 2;
}

/**
 * @brief Get XFI event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000001011 000100100
 *
 * POLLXFI          {WC/WZ/WCZ}
 *
 *</pre>
 */
int P2Cog::op_POLLXFI()
{
    return 2;
}

/**
 * @brief Get XRO event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000001100 000100100
 *
 * POLLXRO          {WC/WZ/WCZ}
 *
 *</pre>
 */
int P2Cog::op_POLLXRO()
{
    return 2;
}

/**
 * @brief Get XRL event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000001101 000100100
 *
 * POLLXRL          {WC/WZ/WCZ}
 *
 *</pre>
 */
int P2Cog::op_POLLXRL()
{
    return 2;
}

/**
 * @brief Get ATN event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000001110 000100100
 *
 * POLLATN          {WC/WZ/WCZ}
 *
 *</pre>
 */
int P2Cog::op_POLLATN()
{
    return 2;
}

/**
 * @brief Get QMT event flag into C/Z, then clear it.
 *<pre>
 * EEEE 1101011 CZ0 000001111 000100100
 *
 * POLLQMT          {WC/WZ/WCZ}
 *
 *</pre>
 */
int P2Cog::op_POLLQMT()
{
    return 2;
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
int P2Cog::op_WAITINT()
{
    return 2;
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
int P2Cog::op_WAITCT1()
{
    return 2;
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
int P2Cog::op_WAITCT2()
{
    return 2;
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
int P2Cog::op_WAITCT3()
{
    return 2;
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
int P2Cog::op_WAITSE1()
{
    return 2;
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
int P2Cog::op_WAITSE2()
{
    return 2;
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
int P2Cog::op_WAITSE3()
{
    return 2;
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
int P2Cog::op_WAITSE4()
{
    return 2;
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
int P2Cog::op_WAITPAT()
{
    return 2;
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
int P2Cog::op_WAITFBW()
{
    return 2;
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
int P2Cog::op_WAITXMT()
{
    return 2;
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
int P2Cog::op_WAITXFI()
{
    return 2;
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
int P2Cog::op_WAITXRO()
{
    return 2;
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
int P2Cog::op_WAITXRL()
{
    return 2;
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
int P2Cog::op_WAITATN()
{
    return 2;
}

/**
 * @brief Allow interrupts (default).
 *<pre>
 * EEEE 1101011 000 000100000 000100100
 *
 * ALLOWI
 *
 *</pre>
 */
int P2Cog::op_ALLOWI()
{
    return 2;
}

/**
 * @brief Stall Interrupts.
 *<pre>
 * EEEE 1101011 000 000100001 000100100
 *
 * STALLI
 *
 *</pre>
 */
int P2Cog::op_STALLI()
{
    return 2;
}

/**
 * @brief Trigger INT1, regardless of STALLI mode.
 *<pre>
 * EEEE 1101011 000 000100010 000100100
 *
 * TRGINT1
 *
 *</pre>
 */
int P2Cog::op_TRGINT1()
{
    return 2;
}

/**
 * @brief Trigger INT2, regardless of STALLI mode.
 *<pre>
 * EEEE 1101011 000 000100011 000100100
 *
 * TRGINT2
 *
 *</pre>
 */
int P2Cog::op_TRGINT2()
{
    return 2;
}

/**
 * @brief Trigger INT3, regardless of STALLI mode.
 *<pre>
 * EEEE 1101011 000 000100100 000100100
 *
 * TRGINT3
 *
 *</pre>
 */
int P2Cog::op_TRGINT3()
{
    return 2;
}

/**
 * @brief Cancel INT1.
 *<pre>
 * EEEE 1101011 000 000100101 000100100
 *
 * NIXINT1
 *
 *</pre>
 */
int P2Cog::op_NIXINT1()
{
    return 2;
}

/**
 * @brief Cancel INT2.
 *<pre>
 * EEEE 1101011 000 000100110 000100100
 *
 * NIXINT2
 *
 *</pre>
 */
int P2Cog::op_NIXINT2()
{
    return 2;
}

/**
 * @brief Cancel INT3.
 *<pre>
 * EEEE 1101011 000 000100111 000100100
 *
 * NIXINT3
 *
 *</pre>
 */
int P2Cog::op_NIXINT3()
{
    return 2;
}

/**
 * @brief Set INT1 source to D[3:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000100101
 *
 * SETINT1 {#}D
 *
 *</pre>
 */
int P2Cog::op_SETINT1()
{
    augmentD(IR.op7.im);
    return 2;
}

/**
 * @brief Set INT2 source to D[3:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000100110
 *
 * SETINT2 {#}D
 *
 *</pre>
 */
int P2Cog::op_SETINT2()
{
    augmentD(IR.op7.im);
    return 2;
}

/**
 * @brief Set INT3 source to D[3:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000100111
 *
 * SETINT3 {#}D
 *
 *</pre>
 */
int P2Cog::op_SETINT3()
{
    augmentD(IR.op7.im);
    return 2;
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
int P2Cog::op_SETQ()
{
    augmentD(IR.op7.im);
    return 2;
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
int P2Cog::op_SETQ2()
{
    augmentD(IR.op7.im);
    return 2;
}

/**
 * @brief Push D onto stack.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000101010
 *
 * PUSH    {#}D
 *
 *</pre>
 */
int P2Cog::op_PUSH()
{
    augmentD(IR.op7.im);
    return 2;
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
int P2Cog::op_POP()
{
    return 2;
}

/**
 * @brief Jump to D.
 *<pre>
 * EEEE 1101011 CZ0 DDDDDDDDD 000101100
 *
 * JMP     D        {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 *</pre>
 */
int P2Cog::op_JMP()
{
    updateC((D >> 31) & 1);
    updateZ((D >> 30) & 1);
    updatePC(D);
    return 2;
}

/**
 * @brief Call to D by pushing {C, Z, 10'b0, PC[19:0]} onto stack.
 *<pre>
 * EEEE 1101011 CZ0 DDDDDDDDD 000101101
 *
 * CALL    D        {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 *</pre>
 */
int P2Cog::op_CALL()
{
    const p2_LONG stack = (C << 31) | (Z << 30) | PC;
    const p2_LONG result = D;
    pushK(stack);
    updateC((result >> 31) & 1);
    updateZ((result >> 30) & 1);
    updatePC(result);
    return 2;
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
int P2Cog::op_RET()
{
    p2_LONG result = popK();
    updateC((result >> 31) & 1);
    updateZ((result >> 30) & 1);
    updatePC(result);
    return 2;
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
int P2Cog::op_CALLA()
{
    const p2_LONG stack = (C << 31) | (Z << 30) | PC;
    const p2_LONG result = D;
    pushPTRA(stack);
    updateC((result >> 31) & 1);
    updateZ((result >> 30) & 1);
    updatePC(result);
    return 2;
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
int P2Cog::op_RETA()
{
    p2_LONG result = popPTRA();
    updateC((result >> 31) & 1);
    updateZ((result >> 30) & 1);
    updatePC(result);
    return 2;
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
int P2Cog::op_CALLB()
{
    const p2_LONG stack = (C << 31) | (Z << 30) | PC;
    const p2_LONG result = D;
    pushPTRB(stack);
    updateC((result >> 31) & 1);
    updateZ((result >> 30) & 1);
    updatePC(result);
    return 2;
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
int P2Cog::op_RETB()
{
    p2_LONG result = popPTRB();
    updateC((result >> 31) & 1);
    updateZ((result >> 30) & 1);
    updatePC(result);
    return 2;
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
int P2Cog::op_JMPREL()
{
    augmentD(IR.op7.im);
    const p2_LONG result = PC < 0x400 ? PC + D : PC + D * 4;
    updatePC(result);
    return 2;
}

/**
 * @brief Skip instructions per D.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000110001
 *
 * SKIP    {#}D
 *
 * Subsequent instructions 0 … 31 get cancelled for each '1' bit in D[0] … D[31].
 *</pre>
 */
int P2Cog::op_SKIP()
{
    augmentD(IR.op7.im);
    const p2_LONG result = D;
    updateSKIP(result);
    return 2;
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
int P2Cog::op_SKIPF()
{
    augmentD(IR.op7.im);
    return 2;
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
int P2Cog::op_EXECF()
{
    augmentD(IR.op7.im);
    return 2;
}

/**
 * @brief Get current FIFO hub pointer into D.
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 000110100
 *
 * GETPTR  D
 *
 *</pre>
 */
int P2Cog::op_GETPTR()
{
    const p2_LONG result = FIFO.head_addr;
    updateD(result);
    return 2;
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
int P2Cog::op_GETBRK()
{
    return 2;
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
int P2Cog::op_COGBRK()
{
    augmentD(IR.op7.im);
    return 2;
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
int P2Cog::op_BRK()
{
    augmentD(IR.op7.im);
    return 2;
}

/**
 * @brief If D[0] = 1 then enable LUT sharing, where LUT writes within the adjacent odd/even companion cog are copied to this LUT.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000110111
 *
 * SETLUTS {#}D
 *
 *</pre>
 */
int P2Cog::op_SETLUTS()
{
    augmentD(IR.op7.im);
    return 2;
}

/**
 * @brief Set the colorspace converter "CY" parameter to D[31:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000111000
 *
 * SETCY   {#}D
 *
 *</pre>
 */
int P2Cog::op_SETCY()
{
    augmentD(IR.op7.im);
    return 2;
}

/**
 * @brief Set the colorspace converter "CI" parameter to D[31:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000111001
 *
 * SETCI   {#}D
 *
 *</pre>
 */
int P2Cog::op_SETCI()
{
    augmentD(IR.op7.im);
    return 2;
}

/**
 * @brief Set the colorspace converter "CQ" parameter to D[31:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000111010
 *
 * SETCQ   {#}D
 *
 *</pre>
 */
int P2Cog::op_SETCQ()
{
    augmentD(IR.op7.im);
    return 2;
}

/**
 * @brief Set the colorspace converter "CFRQ" parameter to D[31:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000111011
 *
 * SETCFRQ {#}D
 *
 *</pre>
 */
int P2Cog::op_SETCFRQ()
{
    augmentD(IR.op7.im);
    return 2;
}

/**
 * @brief Set the colorspace converter "CMOD" parameter to D[6:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000111100
 *
 * SETCMOD {#}D
 *
 *</pre>
 */
int P2Cog::op_SETCMOD()
{
    augmentD(IR.op7.im);
    return 2;
}

/**
 * @brief Set BLNPIX/MIXPIX blend factor to D[7:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000111101
 *
 * SETPIV  {#}D
 *
 *</pre>
 */
int P2Cog::op_SETPIV()
{
    augmentD(IR.op7.im);
    return 2;
}

/**
 * @brief Set MIXPIX mode to D[5:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000111110
 *
 * SETPIX  {#}D
 *
 *</pre>
 */
int P2Cog::op_SETPIX()
{
    augmentD(IR.op7.im);
    return 2;
}

/**
 * @brief Strobe "attention" of all cogs whose corresponging bits are high in D[15:0].
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 000111111
 *
 * COGATN  {#}D
 *
 *</pre>
 */
int P2Cog::op_COGATN()
{
    augmentD(IR.op7.im);
    return 2;
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
int P2Cog::op_TESTP_W()
{
    augmentD(IR.op7.im);
    return 2;
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
int P2Cog::op_TESTPN_W()
{
    augmentD(IR.op7.im);
    return 2;
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
int P2Cog::op_TESTP_AND()
{
    augmentD(IR.op7.im);
    return 2;
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
int P2Cog::op_TESTPN_AND()
{
    augmentD(IR.op7.im);
    return 2;
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
int P2Cog::op_TESTP_OR()
{
    augmentD(IR.op7.im);
    return 2;
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
int P2Cog::op_TESTPN_OR()
{
    augmentD(IR.op7.im);
    return 2;
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
int P2Cog::op_TESTP_XOR()
{
    augmentD(IR.op7.im);
    return 2;
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
int P2Cog::op_TESTPN_XOR()
{
    augmentD(IR.op7.im);
    return 2;
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
int P2Cog::op_DIRL()
{
    augmentD(IR.op7.im);
    const p2_LONG result = 0;
    updateC(result);
    updateZ(result);
    updateDIR(D, result);
    return 2;
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
int P2Cog::op_DIRH()
{
    augmentD(IR.op7.im);
    const p2_LONG result = 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, result);
    return 2;
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
int P2Cog::op_DIRC()
{
    augmentD(IR.op7.im);
    const p2_LONG result = C;
    updateC(result);
    updateZ(result);
    updateDIR(D, result);
    return 2;
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
int P2Cog::op_DIRNC()
{
    augmentD(IR.op7.im);
    const p2_LONG result = C ^ 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, result);
    return 2;
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
int P2Cog::op_DIRZ()
{
    augmentD(IR.op7.im);
    const p2_LONG result = Z;
    updateC(result);
    updateZ(result);
    updateDIR(D, result);
    return 2;
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
int P2Cog::op_DIRNZ()
{
    augmentD(IR.op7.im);
    const p2_LONG result = Z ^ 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, result);
    return 2;
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
int P2Cog::op_DIRRND()
{
    augmentD(IR.op7.im);
    const p2_LONG result = HUB->random(2*ID) & 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, result);
    return 2;
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
int P2Cog::op_DIRNOT()
{
    augmentD(IR.op7.im);
    const p2_LONG result = HUB->rd_DIR(D) ^ 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, result);
    return 2;
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
int P2Cog::op_OUTL()
{
    augmentD(IR.op7.im);
    const p2_LONG result = 0;
    updateC(result);
    updateZ(result);
    updateOUT(D, result);
    return 2;
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
int P2Cog::op_OUTH()
{
    augmentD(IR.op7.im);
    const p2_LONG result = 1;
    updateC(result);
    updateZ(result);
    updateOUT(D, result);
    return 2;
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
int P2Cog::op_OUTC()
{
    augmentD(IR.op7.im);
    const p2_LONG result = C;
    updateC(result);
    updateZ(result);
    updateOUT(D, result);
    return 2;
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
int P2Cog::op_OUTNC()
{
    augmentD(IR.op7.im);
    const p2_LONG result = C ^ 1;
    updateC(result);
    updateZ(result);
    updateOUT(D, result);
    return 2;
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
int P2Cog::op_OUTZ()
{
    augmentD(IR.op7.im);
    const p2_LONG result = Z;
    updateC(result);
    updateZ(result);
    updateOUT(D, result);
    return 2;
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
int P2Cog::op_OUTNZ()
{
    augmentD(IR.op7.im);
    const p2_LONG result = Z ^ 1;
    updateC(result);
    updateZ(result);
    updateOUT(D, result);
    return 2;
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
int P2Cog::op_OUTRND()
{
    augmentD(IR.op7.im);
    const p2_LONG result = HUB->random(2*ID) & 1;
    updateC(result);
    updateZ(result);
    updateOUT(D, result);
    return 2;
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
int P2Cog::op_OUTNOT()
{
    augmentD(IR.op7.im);
    const p2_LONG result = HUB->rd_OUT(D) ^ 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, result);
    return 2;
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
int P2Cog::op_FLTL()
{
    augmentD(IR.op7.im);
    const p2_LONG result = 0;
    updateC(result);
    updateZ(result);
    updateDIR(D, 0);
    updateOUT(D, result);
    return 2;
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
int P2Cog::op_FLTH()
{
    augmentD(IR.op7.im);
    const p2_LONG result = 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, 0);
    updateOUT(D, result);
    return 2;
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
int P2Cog::op_FLTC()
{
    augmentD(IR.op7.im);
    const p2_LONG result = C;
    updateC(result);
    updateZ(result);
    updateDIR(D, 0);
    updateOUT(D, result);
    return 2;
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
int P2Cog::op_FLTNC()
{
    augmentD(IR.op7.im);
    const p2_LONG result = C ^ 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, 0);
    updateOUT(D, result);
    return 2;
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
int P2Cog::op_FLTZ()
{
    augmentD(IR.op7.im);
    const p2_LONG result = Z;
    updateC(result);
    updateZ(result);
    updateDIR(D, 0);
    updateOUT(D, result);
    return 2;
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
int P2Cog::op_FLTNZ()
{
    augmentD(IR.op7.im);
    const p2_LONG result = Z ^ 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, 0);
    updateOUT(D, result);
    return 2;
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
int P2Cog::op_FLTRND()
{
    augmentD(IR.op7.im);
    const p2_LONG result = HUB->random(2*ID) & 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, 0);
    updateOUT(D, result);
    return 2;
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
int P2Cog::op_FLTNOT()
{
    augmentD(IR.op7.im);
    const p2_LONG result = HUB->rd_OUT(D);
    updateC(result);
    updateZ(result);
    updateDIR(D, 0);
    updateOUT(D, result);
    return 2;
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
int P2Cog::op_DRVL()
{
    augmentD(IR.op7.im);
    const p2_LONG result = 0;
    updateC(result);
    updateZ(result);
    updateDIR(D, 1);
    updateOUT(D, result);
    return 2;
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
int P2Cog::op_DRVH()
{
    augmentD(IR.op7.im);
    const p2_LONG result = 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, 1);
    updateOUT(D, result);
    return 2;
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
int P2Cog::op_DRVC()
{
    augmentD(IR.op7.im);
    const p2_LONG result = C;
    updateC(result);
    updateZ(result);
    updateDIR(D, 1);
    updateOUT(D, result);
    return 2;
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
int P2Cog::op_DRVNC()
{
    augmentD(IR.op7.im);
    const p2_LONG result = C ^ 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, 1);
    updateOUT(D, result);
    return 2;
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
int P2Cog::op_DRVZ()
{
    augmentD(IR.op7.im);
    const p2_LONG result = Z;
    updateC(result);
    updateZ(result);
    updateDIR(D, 1);
    updateOUT(D, result);
    return 2;
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
int P2Cog::op_DRVNZ()
{
    augmentD(IR.op7.im);
    const p2_LONG result = Z ^ 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, 1);
    updateOUT(D, result);
    return 2;
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
int P2Cog::op_DRVRND()
{
    augmentD(IR.op7.im);
    const p2_LONG result = HUB->random(2*ID) & 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, 1);
    updateOUT(D, result);
    return 2;
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
int P2Cog::op_DRVNOT()
{
    augmentD(IR.op7.im);
    const p2_LONG result = HUB->rd_OUT(D) ^ 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, 1);
    updateOUT(D, result);
    return 2;
}

/**
 * @brief Split every 4th bit of S into bytes of D.
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 001100000
 *
 * SPLITB  D
 *
 * D = {S[31], S[27], S[23], S[19], … S[12], S[8], S[4], S[0]}.
 *</pre>
 */
int P2Cog::op_SPLITB()
{
    const quint32 result =
            (((S >> 31) & 1) << 31) | (((S >> 27) & 1) << 30) | (((S >> 23) & 1) << 29) | (((S >> 19) & 1) << 28) |
            (((S >> 15) & 1) << 27) | (((S >> 11) & 1) << 26) | (((S >>  7) & 1) << 25) | (((S >>  3) & 1) << 24) |
            (((S >> 30) & 1) << 23) | (((S >> 26) & 1) << 22) | (((S >> 22) & 1) << 21) | (((S >> 18) & 1) << 20) |
            (((S >> 14) & 1) << 19) | (((S >> 10) & 1) << 18) | (((S >>  6) & 1) << 17) | (((S >>  2) & 1) << 16) |
            (((S >> 29) & 1) << 15) | (((S >> 25) & 1) << 14) | (((S >> 21) & 1) << 13) | (((S >> 17) & 1) << 12) |
            (((S >> 13) & 1) << 11) | (((S >>  9) & 1) << 10) | (((S >>  5) & 1) <<  9) | (((S >>  1) & 1) <<  8) |
            (((S >> 28) & 1) <<  7) | (((S >> 24) & 1) <<  6) | (((S >> 20) & 1) <<  5) | (((S >> 16) & 1) <<  4) |
            (((S >> 12) & 1) <<  3) | (((S >>  8) & 1) <<  2) | (((S >>  4) & 1) <<  1) | (((S >>  0) & 1) <<  0);
    updateD(result);
    return 2;
}

/**
 * @brief Merge bits of bytes in S into D.
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 001100001
 *
 * MERGEB  D
 *
 * D = {S[31], S[23], S[15], S[7], … S[24], S[16], S[8], S[0]}.
 *</pre>
 */
int P2Cog::op_MERGEB()
{
    const quint32 result =
            (((S >> 31) & 1) << 31) | (((S >> 23) & 1) << 30) | (((S >> 15) & 1) << 29) | (((S >>  7) & 1) << 28) |
            (((S >> 30) & 1) << 27) | (((S >> 22) & 1) << 26) | (((S >> 14) & 1) << 25) | (((S >>  6) & 1) << 24) |
            (((S >> 29) & 1) << 23) | (((S >> 21) & 1) << 22) | (((S >> 13) & 1) << 21) | (((S >>  5) & 1) << 20) |
            (((S >> 28) & 1) << 19) | (((S >> 20) & 1) << 18) | (((S >> 12) & 1) << 17) | (((S >>  4) & 1) << 16) |
            (((S >> 27) & 1) << 15) | (((S >> 19) & 1) << 14) | (((S >> 11) & 1) << 13) | (((S >>  3) & 1) << 12) |
            (((S >> 26) & 1) << 11) | (((S >> 18) & 1) << 10) | (((S >> 10) & 1) <<  9) | (((S >>  2) & 1) <<  8) |
            (((S >> 25) & 1) <<  7) | (((S >> 17) & 1) <<  6) | (((S >>  9) & 1) <<  5) | (((S >>  1) & 1) <<  4) |
            (((S >> 24) & 1) <<  3) | (((S >> 16) & 1) <<  2) | (((S >>  8) & 1) <<  1) | (((S >>  0) & 1) <<  0);
    updateD(result);
    return 2;
}

/**
 * @brief Split bits of S into words of D.
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 001100010
 *
 * SPLITW  D
 *
 * D = {S[31], S[29], S[27], S[25], … S[6], S[4], S[2], S[0]}.
 *</pre>
 */
int P2Cog::op_SPLITW()
{
    const quint32 result =
            (((S >> 31) & 1) << 31) | (((S >> 29) & 1) << 30) | (((S >> 27) & 1) << 29) | (((S >> 25) & 1) << 28) |
            (((S >> 23) & 1) << 27) | (((S >> 21) & 1) << 26) | (((S >> 19) & 1) << 25) | (((S >> 17) & 1) << 24) |
            (((S >> 15) & 1) << 23) | (((S >> 13) & 1) << 22) | (((S >> 11) & 1) << 21) | (((S >>  9) & 1) << 20) |
            (((S >>  7) & 1) << 19) | (((S >>  5) & 1) << 18) | (((S >>  3) & 1) << 17) | (((S >>  1) & 1) << 16) |
            (((S >> 30) & 1) << 15) | (((S >> 28) & 1) << 14) | (((S >> 26) & 1) << 13) | (((S >> 24) & 1) << 12) |
            (((S >> 22) & 1) << 11) | (((S >> 20) & 1) << 10) | (((S >> 18) & 1) <<  9) | (((S >> 16) & 1) <<  8) |
            (((S >> 14) & 1) <<  7) | (((S >> 12) & 1) <<  6) | (((S >> 10) & 1) <<  5) | (((S >>  8) & 1) <<  4) |
            (((S >>  6) & 1) <<  3) | (((S >>  4) & 1) <<  2) | (((S >>  2) & 1) <<  1) | (((S >>  0) & 1) <<  0);
    updateD(result);
    return 2;
}

/**
 * @brief Merge bits of words in S into D.
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 001100011
 *
 * MERGEW  D
 *
 * D = {S[31], S[15], S[30], S[14], … S[17], S[1], S[16], S[0]}.
 *</pre>
 */
int P2Cog::op_MERGEW()
{
    const quint32 result =
            (((S >> 31) & 1) << 31) | (((S >> 15) & 1) << 30) |
            (((S >> 30) & 1) << 29) | (((S >> 14) & 1) << 28) |
            (((S >> 29) & 1) << 27) | (((S >> 13) & 1) << 26) |
            (((S >> 28) & 1) << 25) | (((S >> 12) & 1) << 24) |
            (((S >> 27) & 1) << 23) | (((S >> 11) & 1) << 22) |
            (((S >> 26) & 1) << 21) | (((S >> 10) & 1) << 20) |
            (((S >> 25) & 1) << 19) | (((S >>  9) & 1) << 18) |
            (((S >> 24) & 1) << 17) | (((S >>  8) & 1) << 16) |
            (((S >> 23) & 1) << 15) | (((S >>  7) & 1) << 14) |
            (((S >> 22) & 1) << 13) | (((S >>  6) & 1) << 12) |
            (((S >> 21) & 1) << 11) | (((S >>  5) & 1) << 10) |
            (((S >> 20) & 1) <<  9) | (((S >>  4) & 1) <<  8) |
            (((S >> 19) & 1) <<  7) | (((S >>  3) & 1) <<  6) |
            (((S >> 18) & 1) <<  5) | (((S >>  2) & 1) <<  4) |
            (((S >> 17) & 1) <<  3) | (((S >>  1) & 1) <<  2) |
            (((S >> 16) & 1) <<  1) | (((S >>  0) & 1) <<  0);
    updateD(result);
    return 2;
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
int P2Cog::op_SEUSSF()
{
    const p2_LONG result = P2Util::seuss(S, true);
    updateD(result);
    return 2;
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
int P2Cog::op_SEUSSR()
{
    const p2_LONG result = P2Util::seuss(S, false);
    updateD(result);
    return 2;
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
int P2Cog::op_RGBSQZ()
{
    const p2_LONG result =
            (((S >> 27) & 0x1f) << 11) |
            (((S >> 18) & 0x3f) <<  5) |
            (((S >> 11) & 0x1f) <<  0);
    updateD(result);
    return 2;
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
int P2Cog::op_RGBEXP()
{
    const p2_LONG result =
            (((S >> 11) & 0x1f) << 27) |
            (((S >> 13) & 0x07) << 24) |
            (((S >>  5) & 0x3f) << 18) |
            (((S >>  9) & 0x03) << 16) |
            (((S >>  0) & 0x1f) << 11) |
            (((S >>  2) & 0x07) <<  8);
    updateD(result);
    return 2;
}

/**
 * @brief Iterate D with xoroshiro32+ PRNG algorithm and put PRNG result into next instruction's S.
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 001101000
 *
 * XORO32  D
 *
 *</pre>
 */
int P2Cog::op_XORO32()
{
    return 2;
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
int P2Cog::op_REV()
{
    p2_LONG result = P2Util::reverse(D);
    updateD(result);
    return 2;
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
int P2Cog::op_RCZR()
{
    const p2_LONG result = (C << 31) | (Z << 30) | (D >> 2);
    updateC((D >> 1) & 1);
    updateZ((D >> 0) & 1);
    updateD(result);
    return 2;
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
int P2Cog::op_RCZL()
{
    const p2_LONG result = (D << 2) | (C << 1) | (Z << 0);
    updateC((D >> 31) & 1);
    updateZ((D >> 30) & 1);
    updateD(result);
    return 2;
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
int P2Cog::op_WRC()
{
    const p2_LONG result = C;
    updateD(result);
    return 2;
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
int P2Cog::op_WRNC()
{
    const p2_LONG result = C ^ 1;
    updateD(result);
    return 2;
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
int P2Cog::op_WRZ()
{
    const p2_LONG result = Z;
    updateD(result);
    return 2;
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
int P2Cog::op_WRNZ()
{
    const p2_LONG result = Z ^ 1;
    updateD(result);
    return 2;
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
int P2Cog::op_MODCZ()
{
    const p2_cond_e cccc = static_cast<p2_cond_e>((IR.op7.dst >> 4) & 15);
    const p2_cond_e zzzz = static_cast<p2_cond_e>((IR.op7.dst >> 0) & 15);
    updateC(conditional(cccc));
    updateZ(conditional(zzzz));
    return 2;
}

/**
 * @brief Set scope mode.
 *<pre>
 * EEEE 1101011 00L DDDDDDDDD 001110000
 *
 * SETSCP  {#}D
 *
 *
 * SETSCP points the scope mux to a set of four pins starting
 * at (D[5:0] AND $3C), with D[6]=1 to enable scope operation.
 *
 * Set pins D[5:2] (0, 4, 8, 12, …, 60)
 * Enable if D[6]=1.
 *</pre>
 */
int P2Cog::op_SETSCP()
{
    augmentD(IR.op7.im);
    HUB->wr_SCP(D);
    return 2;
}

/**
 * @brief Get scope values.
 *<pre>
 * EEEE 1101011 000 DDDDDDDDD 001110001
 *
 * Any time GETSCP is executed, the lower bytes of those four pins' RDPIN values are returned in D.
 *
 * GETSCP  D
 *
 * Pins D[5:2].
 *</pre>
 */
int P2Cog::op_GETSCP()
{
    const p2_LONG result = HUB->rd_SCP();
    updateD(result);
    return 2;
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
int P2Cog::op_JMP_ABS()
{
    const p2_LONG result = (IR.op5.address + (PC & SXn<p2_LONG,1>(IR.op5.rel))) & A20MASK;
    updatePC(result);
    return 2;
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
int P2Cog::op_CALL_ABS()
{
    const p2_LONG stack = (C << 31) | (Z << 30) | PC;
    const p2_LONG result = (IR.op5.address + (PC & SXn<p2_LONG,1>(IR.op5.rel))) & A20MASK;
    pushK(stack);
    updatePC(result);
    return 2;
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
int P2Cog::op_CALLA_ABS()
{
    const p2_LONG stack = (C << 31) | (Z << 30) | PC;
    const p2_LONG result = (IR.op5.address + (PC & SXn<p2_LONG,1>(IR.op5.rel))) & A20MASK;
    pushPTRA(stack);
    updatePC(result);
    return 2;
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
int P2Cog::op_CALLB_ABS()
{
    const p2_LONG stack = (C << 31) | (Z << 30) | PC;
    const p2_LONG result = (IR.op5.address + (PC & SXn<p2_LONG,1>(IR.op5.rel))) & A20MASK;
    pushPTRB(stack);
    updatePC(result);
    return 2;
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PA (per W).
 *<pre>
 * EEEE 1110000 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PA,#A
 *
 * If R = 1, PC += A, else PC = A.
 *</pre>
 */
int P2Cog::op_CALLD_ABS_PA()
{
    const p2_LONG stack = (C << 31) | (Z << 30) | PC;
    const p2_LONG result = (IR.op5.address + (PC & SXn<p2_LONG,1>(IR.op5.rel))) & A20MASK;
    pushPA(stack);
    updatePC(result);
    return 2;
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PB (per W).
 *<pre>
 * EEEE 1110001 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PB,#A
 *
 * If R = 1, PC += A, else PC = A.
 *</pre>
 */
int P2Cog::op_CALLD_ABS_PB()
{
    const p2_LONG stack = (C << 31) | (Z << 30) | PC;
    const p2_LONG result = (IR.op5.address + (PC & SXn<p2_LONG,1>(IR.op5.rel))) & A20MASK;
    pushPB(stack);
    updatePC(result);
    return 2;
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
int P2Cog::op_CALLD_ABS_PTRA()
{
    const p2_LONG stack = (C << 31) | (Z << 30) | PC;
    const p2_LONG result = (IR.op5.address + (PC & SXn<p2_LONG,1>(IR.op5.rel))) & A20MASK;
    pushPTRA(stack);
    updatePC(result);
    return 2;
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
int P2Cog::op_CALLD_ABS_PTRB()
{
    const p2_LONG stack = (C << 31) | (Z << 30) | PC;
    const p2_LONG result = (IR.op5.address + (PC & SXn<p2_LONG,1>(IR.op5.rel))) & A20MASK;
    pushPTRB(stack);
    updatePC(result);
    return 2;
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
int P2Cog::op_LOC_PA()
{
    const p2_LONG result = (IR.op5.address + (PC & SXn<p2_LONG,1>(IR.op5.rel))) & A20MASK;
    updatePA(result);
    return 2;
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
int P2Cog::op_LOC_PB()
{
    const p2_LONG result = (IR.op5.address + (PC & SXn<p2_LONG,1>(IR.op5.rel))) & A20MASK;
    updatePB(result);
    return 2;
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
int P2Cog::op_LOC_PTRA()
{
    const p2_LONG result = (IR.op5.address + (PC & SXn<p2_LONG,1>(IR.op5.rel))) & A20MASK;
    updatePTRA(result);
    return 2;
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
int P2Cog::op_LOC_PTRB()
{
    const p2_LONG result = (IR.op5.address + (PC & SXn<p2_LONG,1>(IR.op5.rel))) & A20MASK;
    updatePTRB(result);
    return 2;
}

/**
 * @brief Queue #N[31:9] to be used as upper 23 bits for next #S occurrence, so that the next 9-bit #S will be augmented to 32 bits.
 *<pre>
 * EEEE 11110NN NNN NNNNNNNNN NNNNNNNNN
 *
 * AUGS    #N
 *
 *</pre>
 */
int P2Cog::op_AUGS()
{
    S_aug = (IR.opcode << AUG_SHIFT) & AUG_MASK;
    return 2;
}

/**
 * @brief Queue #N[31:9] to be used as upper 23 bits for next #D occurrence, so that the next 9-bit #D will be augmented to 32 bits.
 *<pre>
 * EEEE 11111NN NNN NNNNNNNNN NNNNNNNNN
 *
 * AUGD    #N
 *
 *</pre>
 */
int P2Cog::op_AUGD()
{
    D_aug = (IR.opcode << AUG_SHIFT) & AUG_MASK;
    return 2;
}
