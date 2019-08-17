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

P2Cog::P2Cog(int cog_id, P2Hub* hub, QObject* parent)
    : QObject(parent)
    , HUB(hub)
    , ID(static_cast<P2LONG>(cog_id))
    , PC(0)
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
    , S_next()
    , S_aug()
    , D_aug()
    , R_aug()
    , COG()
    , LUT()
    , MEM(hub->mem())
    , MEMSIZE(hub->memsize())
{
}

P2LONG P2Cog::rd_cog(P2LONG addr) const
{
    return COG.RAM[addr & 0x1ff];
}

void P2Cog::wr_cog(P2LONG addr, P2LONG val)
{
    COG.RAM[addr & 0x1ff] = val;
}

P2LONG P2Cog::rd_lut(P2LONG addr) const
{
    return LUT.RAM[addr & 0x1ff];
}

void P2Cog::wr_lut(P2LONG addr, P2LONG val)
{
    LUT.RAM[addr & 0x1ff] = val;
}

P2LONG P2Cog::rd_mem(P2LONG addr) const
{
    if (addr < 0x200*4)
        return rd_cog(addr / 4);
    if (addr < 0x400*4)
        return rd_lut(addr / 4 - 0x200);
    Q_ASSERT(HUB);
    return HUB->rd_LONG(addr);
}

void P2Cog::wr_mem(P2LONG addr, P2LONG val)
{
    if (addr < 0x200*4) {
        wr_cog(addr / 4, val);
        return;
    }
    if (addr < 0x400*4) {
        wr_lut(addr / 4 - 0x200, val);
        return;
    }
    Q_ASSERT(HUB);
    HUB->wr_LONG(addr, val);
}

/**
 * @brief return conditional execution status for condition %cond
 * @param cond condition
 * @return true if met, false otherwise
 */
bool P2Cog::conditional(p2_cond_e cond)
{
    switch (cond) {
    case cond__ret_:        // execute always
        return true;
    case cond_nc_and_nz:    // execute if C = 0 and Z = 0
        return C == 0 && Z == 0;
    case cond_nc_and_z:     // execute if C = 0 and Z = 1
        return C == 0 && Z == 1;
    case cond_nc:           // execute if C = 0
        return C == 0;
    case cond_c_and_nz:     // execute if C = 1 and Z = 0
        return C == 1 && Z == 0;
    case cond_nz:           // execute if Z = 0
        return Z == 0;
    case cond_c_ne_z:       // execute if C != Z
        return C != Z;
    case cond_nc_or_nz:     // execute if C = 0 or Z = 0
        return C == 0 || Z == 0;
    case cond_c_and_z:      // execute if C = 1 and Z = 1
        return C == 1 && Z == 1;
    case cond_c_eq_z:       // execute if C = Z
        return C == Z;
    case cond_z:            // execute if Z = 1
        return Z == 1;
    case cond_nc_or_z:      // execute if C = 0 or Z = 1
        return C == 0 || Z == 1;
    case cond_c:            // execute if C = 1
        return C == 1;
    case cond_c_or_nz:      // execute if C = 1 or Z = 0
        return C == 1 || Z == 0;
    case cond_c_or_z:       // execute if C = 1 or Z = 1
        return C == 1 || Z == 1;
    case cond_always:
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
 * @brief Find the most significant 1 bit in value %val
 * @param val value
 * @return position of top most 1 bit
 */
P2BYTE P2Cog::msbit(P2LONG val)
{
    if (val == 0)
        return 0;
    P2BYTE pos;
    for (pos = 31; pos > 0; pos--, val <<= 1) {
        if (val & MSB)
            return pos;
    }
    return pos;
}

/**
 * @brief Return the number of ones (1) in a 32 bit value
 * @param val 32 bit value
 * @return number of 1 bits
 */
P2BYTE P2Cog::ones(P2LONG val)
{
    val = val - ((val >> 1) & 0x55555555);
    val = (val & 0x33333333) + ((val >> 2) & 0x33333333);
    val = (((val + (val >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
    return static_cast<uchar>(val);
}

/**
 * @brief Return the parity of a 32 bit value
 * @param val 32 bit value
 * @return 1 for odd parity, 0 for even parity
 */
P2BYTE P2Cog::parity(P2LONG val)
{
    val ^= val >> 16;
    val ^= val >> 8;
    val ^= val >> 4;
    val &= 15;
    return (0x6996 >> val) & 1;
}

/**
 * @brief Calculate the seuss function for val, forward or reverse
 * @param val value to calulcate seuss for
 * @param forward apply forward if true, otherwise reverse
 * @return seuss value
 */
P2LONG P2Cog::seuss(P2LONG val, bool forward)
{
    const uchar bits[32] = {
        11,  5, 18, 24, 27, 19, 20, 30, 28, 26, 21, 25,  3,  8,  7, 23,
        13, 12, 16,  2, 15,  1,  9, 31,  0, 29, 17, 10, 14,  4,  6, 22
    };
    P2LONG result;

    if (forward) {
        result = 0x354dae51;
        for (int i = 0; i < 32; i++) {
            if (val & (1u << i))
                result ^= (1u << bits[i]);
        }
    } else {
        result = 0xeb55032d;
        for (int i = 0; i < 32; i++) {
            if (val & (1u << bits[i]))
                result ^= (1u << i);
        }
    }
    return result;
}

/**
 * @brief Reverse 32 bits in val
 * @param val value to reverse
 * @return bit reversed value
 */
P2LONG P2Cog::reverse(P2LONG val)
{
    val = (((val & 0xaaaaaaaau) >> 1) | ((val & 0x55555555u) << 1));
    val = (((val & 0xccccccccu) >> 2) | ((val & 0x33333333u) << 2));
    val = (((val & 0xf0f0f0f0u) >> 4) | ((val & 0x0f0f0f0fu) << 4));
    val = (((val & 0xff00ff00u) >> 8) | ((val & 0x00ff00ffu) << 8));
    return (val >> 16) | (val << 16);
}

/**
 * @brief Return the current FIFO level
 * @return FIFO level 0 … 15
 */
P2LONG P2Cog::fifo_level()
{
    return (FIFO.windex - FIFO.rindex) & 15;
}

void P2Cog::check_interrupt_flags()
{

    P2LONG count = U32L(HUB->count());

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
        P2LONG bitmask = 1u << INT.flags.INT1_source;
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
        P2LONG bitmask = 1u << INT.flags.INT2_source;
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
        P2LONG bitmask = 1u << INT.flags.INT3_source;
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
P2LONG P2Cog::check_wait_flag(p2_opword_t IR, P2LONG value1, P2LONG value2, bool streamflag)
{
    P2LONG hubcycles;
    const p2_inst_e inst1 = static_cast<p2_inst_e>(IR.word & p2_INSTR_MASK1);
    const p2_inst_e inst2 = static_cast<p2_inst_e>(IR.word & p2_INSTR_MASK2);
    const p2_inst_e opcode = static_cast<p2_inst_e>(IR.op.inst);

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

        if (0x02024 == (IR.word & 0x3ffff))
            check_wait_int_state();

        switch (IR.op.dst) {
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
               (opcode == p2_OPCODE_WRLONG && IR.op.wc) ||
               (opcode == p2_OPCODE_WMLONG && IR.op.wc && IR.op.wz)) {

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
P2LONG P2Cog::get_pointer(P2LONG inst, P2LONG size)
{
    P2LONG address = 0;
    P2LONG offset = static_cast<P2LONG>((static_cast<qint32>(inst) << 27) >> (27 - size));

    switch ((inst >> 5) & 7) {
    case 0: // PTRA[offset]
        address = LUT.REG.PTRA + offset;
        break;
    case 1: // PTRA
        address = LUT.REG.PTRA;
        break;
    case 2: // PTRA[++offset]
        address = LUT.REG.PTRA + offset;
        PTRA0 = address;
        break;
    case 3: // PTRA[offset++]
        address = LUT.REG.PTRA;
        PTRA0 = LUT.REG.PTRA + offset;
        break;
    case 4: // PTRB[offset]
        address = LUT.REG.PTRB + offset;
        break;
    case 5: // PTRB
        address = LUT.REG.PTRB;
        break;
    case 6: // PTRB[++offset]
        address = LUT.REG.PTRB + offset;
        PTRB0 = address;
        break;
    case 7: // PTRB[offset++]
        address = LUT.REG.PTRB;
        PTRB0 = LUT.REG.PTRB + offset;
        break;
    }
    return address & A20MASK;
}

/**
 * @brief Save pointer registers PTRA/PTRB
 */
void P2Cog::save_regs()
{
    PTRA0 = LUT.REG.PTRA;
    PTRB0 = LUT.REG.PTRB;
}

/**
 * @brief Update pointer registers PTRA/PTRB
 */
void P2Cog::update_regs()
{
    LUT.REG.PTRA = PTRA0;
    LUT.REG.PTRB = PTRB0;
}

/**
 * @brief Read and decode the next Propeller2 opcode
 * @return number of cycles
 */
int P2Cog::decode()
{
    int cycles = 2;

    check_interrupt_flags();

    switch (PC & 0xffe00) {
    case 0x00000:   // cogexec
        IR.word = COG.RAM[PC];
        PC++;       // increment PC
        break;
    case 0x00200:   // lutexec
        IR.word = LUT.RAM[PC - 0x200];
        PC++;       // increment PC
        break;
    default:        // hubexec
        IR.word = HUB->rd_LONG(PC);
        PC += 4;    // increment PC by 4
    }
    S = COG.RAM[IR.op.src]; // set S to COG[src]
    D = COG.RAM[IR.op.dst]; // set D to COG[dst]

    // check for the condition
    if (!conditional(IR.op.cond))
        return cycles;

    // Dispatch to op_xxx() functions
    switch (IR.op.inst) {
    case p2_ROR:
        cycles = op_ror();
        break;

    case p2_ROL:
        cycles = op_rol();
        break;

    case p2_SHR:
        cycles = op_shr();
        break;

    case p2_SHL:
        cycles = op_shl();
        break;

    case p2_RCR:
        cycles = op_rcr();
        break;

    case p2_RCL:
        cycles = op_rcl();
        break;

    case p2_SAR:
        cycles = op_sar();
        break;

    case p2_SAL:
        cycles = op_sal();
        break;

    case p2_ADD:
        cycles = op_add();
        break;

    case p2_ADDX:
        cycles = op_addx();
        break;

    case p2_ADDS:
        cycles = op_adds();
        break;

    case p2_ADDSX:
        cycles = op_addsx();
        break;

    case p2_SUB:
        cycles = op_sub();
        break;

    case p2_SUBX:
        cycles = op_subx();
        break;

    case p2_SUBS:
        cycles = op_subs();
        break;

    case p2_SUBSX:
        cycles = op_subsx();
        break;

    case p2_CMP:
        cycles = op_cmp();
        break;

    case p2_CMPX:
        cycles = op_cmpx();
        break;

    case p2_CMPS:
        cycles = op_cmps();
        break;

    case p2_CMPSX:
        cycles = op_cmpsx();
        break;

    case p2_CMPR:
        cycles = op_cmpr();
        break;

    case p2_CMPM:
        cycles = op_cmpm();
        break;

    case p2_SUBR:
        cycles = op_subr();
        break;

    case p2_CMPSUB:
        cycles = op_cmpsub();
        break;

    case p2_FGE:
        cycles = op_fge();
        break;

    case p2_FLE:
        cycles = op_fle();
        break;

    case p2_FGES:
        cycles = op_fges();
        break;

    case p2_FLES:
        cycles = op_fles();
        break;

    case p2_SUMC:
        cycles = op_sumc();
        break;

    case p2_SUMNC:
        cycles = op_sumnc();
        break;

    case p2_SUMZ:
        cycles = op_sumz();
        break;

    case p2_SUMNZ:
        cycles = op_sumnz();
        break;

    case p2_TESTB_W_BITL: // case p2_bitl:
        cycles = (IR.op.wc != IR.op.wz) ? op_testb_w()
                                        : op_bitl();
        break;

    case p2_TESTBN_W_BITH: // case p2_bith:
        cycles = (IR.op.wc != IR.op.wz) ? op_testbn_w()
                                        : op_bith();
        break;

    case p2_TESTB_AND_BITC: // case p2_bitc:
        cycles = (IR.op.wc != IR.op.wz) ? op_testb_and()
                                        : op_bitc();
        break;

    case p2_TESTBN_AND_BITNC: // case p2_bitnc:
        cycles = (IR.op.wc != IR.op.wz) ? op_testbn_and()
                                        : op_bitnc();
        break;

    case p2_TESTB_OR_BITZ: // case p2_bitz:
        cycles = (IR.op.wc != IR.op.wz) ? op_testb_or()
                                        : op_bitz();
        break;

    case p2_TESTBN_OR_BITNZ: // case p2_bitnz:
        cycles = (IR.op.wc != IR.op.wz) ? op_testbn_or()
                                        : op_bitnz();
        break;

    case p2_TESTB_XOR_BITRND: // case p2_bitrnd:
        cycles = (IR.op.wc != IR.op.wz) ? op_testb_xor()
                                        : op_bitrnd();
        break;

    case p2_TESTBN_XOR_BITNOT: // case p2_bitnot:
        cycles = (IR.op.wc != IR.op.wz) ? op_testbn_xor()
                                        : op_bitnot();
        break;

    case p2_AND:
        cycles = op_and();
        break;

    case p2_ANDN:
        cycles = op_andn();
        break;

    case p2_OR:
        cycles = op_or();
        break;

    case p2_XOR:
        cycles = op_xor();
        break;

    case p2_MUXC:
        cycles = op_muxc();
        break;

    case p2_MUXNC:
        cycles = op_muxnc();
        break;

    case p2_MUXZ:
        cycles = op_muxz();
        break;

    case p2_MUXNZ:
        cycles = op_muxnz();
        break;

    case p2_MOV:
        cycles = op_mov();
        break;

    case p2_NOT:
        cycles = op_not();
        break;

    case p2_ABS:
        cycles = op_abs();
        break;

    case p2_NEG:
        cycles = op_neg();
        break;

    case p2_NEGC:
        cycles = op_negc();
        break;

    case p2_NEGNC:
        cycles = op_negnc();
        break;

    case p2_NEGZ:
        cycles = op_negz();
        break;

    case p2_NEGNZ:
        cycles = op_negnz();
        break;

    case p2_INCMOD:
        cycles = op_incmod();
        break;

    case p2_DECMOD:
        cycles = op_decmod();
        break;

    case p2_ZEROX:
        cycles = op_zerox();
        break;

    case p2_SIGNX:
        cycles = op_signx();
        break;

    case p2_ENCOD:
        cycles = op_encod();
        break;

    case p2_ONES:
        cycles = op_ones();
        break;

    case p2_TEST:
        cycles = op_test();
        break;

    case p2_TESTN:
        cycles = op_testn();
        break;

    case p2_SETNIB_0:
    case p2_SETNIB_1:
        cycles = op_setnib();
        break;

    case p2_GETNIB_0:
    case p2_GETNIB_1:
        cycles = op_getnib();
        break;

    case p2_ROLNIB_0:
    case p2_ROLNIB_1:
        cycles = op_rolnib();
        break;

    case p2_SETBYTE:
        cycles = op_setbyte();
        break;

    case p2_GETBYTE:
        cycles = op_getbyte();
        break;

    case p2_ROLBYTE:
        cycles = op_rolbyte();
        break;

    case p2_SETWORD_GETWORD:
        if (IR.op.wc == 0) {
            cycles = (IR.op.dst == 0 && IR.op.wz == 0) ? op_setword_altsw()
                                                       : op_setword();
        } else {
            cycles = (IR.op.src == 0 && IR.op.wz == 0) ? op_getword_altgw()
                                                       : op_getword();
        }
        break;

    case p2_1001010:
        if (IR.op.wc == 0) {
            cycles = (IR.op.src == 0 && IR.op.wz == 0) ? op_rolword_altgw()
                                                       : op_rolword();
        } else {
            if (IR.op.wz == 0) {
                cycles = (IR.op.src == 0 && IR.op.imm == 1) ? op_altsn_d()
                                                            : op_altsn();
            } else {
                cycles = (IR.op.src == 0 && IR.op.imm == 1) ? op_altgn_d()
                                                            : op_altgn();
            }
        }
        break;

    case p2_1001011:
        if (IR.op.wc == 0) {
            if (IR.op.wz == 0) {
                cycles = (IR.op.src == 0 && IR.op.imm == 1) ? op_altsb_d()
                                                            : op_altsb();
            } else {
                cycles = (IR.op.src == 0 && IR.op.imm == 1) ? op_altgb_d()
                                                            : op_altgb();
            }
        } else {
            if (IR.op.wz == 0) {
                cycles = (IR.op.src == 0 && IR.op.imm == 1) ? op_altsw_d()
                                                            : op_altsw();
            } else {
                cycles = (IR.op.src == 0 && IR.op.imm == 1) ? op_altgw_d()
                                                            : op_altgw();
            }
        }
        break;

    case p2_1001100:
        if (IR.op.wc == 0) {
            if (IR.op.wz == 0) {
                cycles = (IR.op.src == 0 && IR.op.imm == 1) ? op_altr_d()
                                                            : op_altr();
            } else {
                cycles = (IR.op.src == 0 && IR.op.imm == 1) ? op_altd_d()
                                                            : op_altd();
            }
        } else {
            if (IR.op.wz == 0) {
                cycles = (IR.op.src == 0 && IR.op.imm == 1) ? op_alts_d()
                                                            : op_alts();
            } else {
                cycles = (IR.op.src == 0 && IR.op.imm == 1) ? op_altb_d()
                                                            : op_altb();
            }
        }
        break;

    case p2_1001101:
        if (IR.op.wc == 0) {
            if (IR.op.wz == 0) {
                cycles = (IR.op.imm == 1 && IR.op.src == 0x164 /* 101100100 */) ? op_alti_d()
                                                                                : op_alti();
            } else {
                cycles = op_setr();
            }
        } else {
            cycles = (IR.op.wz == 0) ? op_setd()
                                     : op_sets();
        }
        break;

    case p2_1001110:
        if (IR.op.wc == 0) {
            if (IR.op.wz == 0) {
                cycles = (IR.op.imm == 0 && IR.op.src == IR.op.dst) ? op_decod_d()
                                                                    : op_decod();
            } else {
                cycles = (IR.op.imm == 0 && IR.op.src == IR.op.dst) ? op_bmask_d()
                                                                    : op_bmask();
            }
        } else {
            if (IR.op.wz == 0) {
                cycles = op_crcbit();
            } else {
                cycles = op_crcnib();
            }
        }
        break;

    case p2_MUXXXX:
        if (IR.op.wc == 0) {
            if (IR.op.wz == 0) {
                cycles = op_muxnits();
            } else {
                cycles = op_muxnibs();
            }
        } else {
            if (IR.op.wz == 0) {
                cycles = op_muxq();
            } else {
                cycles = op_movbyts();
            }
        }
        break;

    case p2_MUL_MULS:
        if (IR.op.wc == 0) {
            cycles = op_mul();
        } else {
            cycles = op_muls();
        }
        break;

    case p2_SCA_SCAS:
        if (IR.op.wc == 0) {
            cycles = op_sca();
        } else {
            cycles = op_scas();
        }
        break;

    case p2_1010010:
        if (IR.op.wc == 0) {
            if (IR.op.wz == 0) {
                cycles = op_addpix();
            } else {
                cycles = op_mulpix();
            }
        } else {
            if (IR.op.wz == 0) {
                cycles = op_blnpix();
            } else {
                cycles = op_mixpix();
            }
        }
        break;

    case p2_WMLONG_ADDCTx:
        if (IR.op.wc == 0) {
            if (IR.op.wz == 0) {
                cycles = op_addct1();
            } else {
                cycles = op_addct2();
            }
        } else {
            if (IR.op.wz == 0) {
                cycles = op_addct3();
            } else {
                cycles = op_wmlong();
            }
        }
        break;

    case p2_RQPIND_RDPIN:
        if (IR.op.wz == 0) {
            cycles = op_rqpin();
        } else {
            cycles = op_rdpin();
        }
        break;

    case p2_RDLUT:
        cycles = op_rdlut();
        break;

    case p2_RDBYTE:
        cycles = op_rdbyte();
        break;

    case p2_RDWORD:
        cycles = op_rdword();
        break;

    case p2_RDLONG:
        cycles = op_rdlong();
        break;

    case p2_CALLD:
        cycles = op_calld();
        break;

    case p2_CALLP:
        cycles = (IR.op.wc == 0) ? op_callpa() : op_callpb();
        break;

    case p2_1011011:
        if (IR.op.wc == 0) {
            if (IR.op.wz == 0) {
                cycles = op_djz();
            } else {
                cycles = op_djnz();
            }
        } else {
            if (IR.op.wz == 0) {
                cycles = op_djf();
            } else {
                cycles = op_djnf();
            }
        }
        break;

    case p2_1011100:
        if (IR.op.wc == 0) {
            if (IR.op.wz == 0) {
                cycles = op_ijz();
            } else {
                cycles = op_ijnz();
            }
        } else {
            if (IR.op.wz == 0) {
                cycles = op_tjz();
            } else {
                cycles = op_tjnz();
            }
        }
        break;

    case p2_1011101:
        if (IR.op.wc == 0) {
            if (IR.op.wz == 0) {
                cycles = op_tjf();
            } else {
                cycles = op_tjnf();
            }
        } else {
            if (IR.op.wz == 0) {
                cycles = op_tjs();
            } else {
                cycles = op_tjns();
            }
        }
        break;

    case p2_1011110:
        if (IR.op.wc == 0) {
            if (IR.op.wz == 0) {
                cycles = op_tjv();
            } else {
                switch (IR.op.dst) {
                case p2_OPDST_JINT:
                    cycles = op_jint();
                    break;
                case p2_OPDST_JCT1:
                    cycles = op_jct1();
                    break;
                case p2_OPDST_JCT2:
                    cycles = op_jct2();
                    break;
                case p2_OPDST_JCT3:
                    cycles = op_jct3();
                    break;
                case p2_OPDST_JSE1:
                    cycles = op_jse1();
                    break;
                case p2_OPDST_JSE2:
                    cycles = op_jse2();
                    break;
                case p2_OPDST_JSE3:
                    cycles = op_jse3();
                    break;
                case p2_OPDST_JSE4:
                    cycles = op_jse4();
                    break;
                case p2_OPDST_JPAT:
                    cycles = op_jpat();
                    break;
                case p2_OPDST_JFBW:
                    cycles = op_jfbw();
                    break;
                case p2_OPDST_JXMT:
                    cycles = op_jxmt();
                    break;
                case p2_OPDST_JXFI:
                    cycles = op_jxfi();
                    break;
                case p2_OPDST_JXRO:
                    cycles = op_jxro();
                    break;
                case p2_OPDST_JXRL:
                    cycles = op_jxrl();
                    break;
                case p2_OPDST_JATN:
                    cycles = op_jatn();
                    break;
                case p2_OPDST_JQMT:
                    cycles = op_jqmt();
                    break;
                case p2_OPDST_JNINT:
                    cycles = op_jnint();
                    break;
                case p2_OPDST_JNCT1:
                    cycles = op_jnct1();
                    break;
                case p2_OPDST_JNCT2:
                    cycles = op_jnct2();
                    break;
                case p2_OPDST_JNCT3:
                    cycles = op_jnct3();
                    break;
                case p2_OPDST_JNSE1:
                    cycles = op_jnse1();
                    break;
                case p2_OPDST_JNSE2:
                    cycles = op_jnse2();
                    break;
                case p2_OPDST_JNSE3:
                    cycles = op_jnse3();
                    break;
                case p2_OPDST_JNSE4:
                    cycles = op_jnse4();
                    break;
                case p2_OPDST_JNPAT:
                    cycles = op_jnpat();
                    break;
                case p2_OPDST_JNFBW:
                    cycles = op_jnfbw();
                    break;
                case p2_OPDST_JNXMT:
                    cycles = op_jnxmt();
                    break;
                case p2_OPDST_JNXFI:
                    cycles = op_jnxfi();
                    break;
                case p2_OPDST_JNXRO:
                    cycles = op_jnxro();
                    break;
                case p2_OPDST_JNXRL:
                    cycles = op_jnxrl();
                    break;
                case p2_OPDST_JNATN:
                    cycles = op_jnatn();
                    break;
                case p2_OPDST_JNQMT:
                    cycles = op_jnqmt();
                    break;
                default:
                    // TODO: invalid D value
                    break;
                }
            }
        } else {
            cycles = op_1011110_1();
        }
        break;

    case p2_1011111:
        if (IR.op.wc == 0) {
            cycles = op_1011111_0();
        } else {
            cycles = op_setpat();
        }
        break;

    case p2_1100000:
        if (IR.op.wc == 0) {
            cycles = (IR.op.wz == 1 && IR.op.dst == 1) ? op_akpin()
                                                       : op_wrpin();
        } else {
            cycles = op_wxpin();
        }
        break;

    case p2_1100001:
        if (IR.op.wc == 0) {
            cycles = op_wypin();
        } else {
            cycles = op_wrlut();
        }
        break;

    case p2_1100010:
        if (IR.op.wc == 0) {
            cycles = op_wrbyte();
        } else {
            cycles = op_wrword();
        }
        break;

    case p2_1100011:
        if (IR.op.wc == 0) {
            cycles = op_wrlong();
        } else {
            cycles = op_rdfast();
        }
        break;

    case p2_1100100:
        if (IR.op.wc == 0) {
            cycles = op_wrfast();
        } else {
            cycles = op_fblock();
        }
        break;

    case p2_1100101:
        if (IR.op.wc == 0) {
            if (IR.op.wz == 1 && IR.op.imm == 1 && IR.op.src == 0 && IR.op.dst == 0) {
                cycles = op_xstop();
            } else {
                cycles = op_xinit();
            }
        } else {
            cycles = op_xzero();
        }
        break;

    case p2_1100110:
        if (IR.op.wc == 0) {
            cycles = op_xcont();
        } else {
            cycles = op_rep();
        }
        break;

    case p2_COGINIT:
        cycles = op_coginit();
        break;

    case p2_1101000:
        if (IR.op.wc == 0) {
            cycles = op_qmul();
        } else {
            cycles = op_qdiv();
        }
        break;

    case p2_1101001:
        if (IR.op.wc == 0) {
            cycles = op_qfrac();
        } else {
            cycles = op_qsqrt();
        }
        break;

    case p2_1101010:
        if (IR.op.wc == 0) {
            cycles = op_qrotate();
        } else {
            cycles = op_qvector();
        }
        break;

    case p2_1101011:
        switch (IR.op.src) {
        case p2_OPSRC_HUBSET:
            cycles = op_hubset();
            break;
        case p2_OPSRC_COGID:
            cycles = op_cogid();
            break;
        case p2_OPSRC_COGSTOP:
            cycles = op_cogstop();
            break;
        case p2_OPSRC_LOCKNEW:
            cycles = op_locknew();
            break;
        case p2_OPSRC_LOCKRET:
            cycles = op_lockret();
            break;
        case p2_OPSRC_LOCKTRY:
            cycles = op_locktry();
            break;
        case p2_OPSRC_LOCKREL:
            cycles = op_lockrel();
            break;
        case p2_OPSRC_QLOG:
            cycles = op_qlog();
            break;
        case p2_OPSRC_QEXP:
            cycles = op_qexp();
            break;
        case p2_OPSRC_RFBYTE:
            cycles = op_rfbyte();
            break;
        case p2_OPSRC_RFWORD:
            cycles = op_rfword();
            break;
        case p2_OPSRC_RFLONG:
            cycles = op_rflong();
            break;
        case p2_OPSRC_RFVAR:
            cycles = op_rfvar();
            break;
        case p2_OPSRC_RFVARS:
            cycles = op_rfvars();
            break;
        case p2_OPSRC_WFBYTE:
            cycles = op_wfbyte();
            break;
        case p2_OPSRC_WFWORD:
            cycles = op_wfword();
            break;
        case p2_OPSRC_WFLONG:
            cycles = op_wflong();
            break;
        case p2_OPSRC_GETQX:
            cycles = op_getqx();
            break;
        case p2_OPSRC_GETQY:
            cycles = op_getqy();
            break;
        case p2_OPSRC_GETCT:
            cycles = op_getct();
            break;
        case p2_OPSRC_GETRND:
            cycles = (IR.op.dst == 0) ? op_getrnd_cz()
                                      : op_getrnd();
            break;
        case p2_OPSRC_SETDACS:
            cycles = op_setdacs();
            break;
        case p2_OPSRC_SETXFRQ:
            cycles = op_setxfrq();
            break;
        case p2_OPSRC_GETXACC:
            cycles = op_getxacc();
            break;
        case p2_OPSRC_WAITX:
            cycles = op_waitx();
            break;
        case p2_OPSRC_SETSE1:
            cycles = op_setse1();
            break;
        case p2_OPSRC_SETSE2:
            cycles = op_setse2();
            break;
        case p2_OPSRC_SETSE3:
            cycles = op_setse3();
            break;
        case p2_OPSRC_SETSE4:
            cycles = op_setse4();
            break;
        case p2_OPSRC_X24:
            switch (IR.op.dst) {
            case p2_OPX24_POLLINT:
                cycles = op_pollint();
                break;
            case p2_OPX24_POLLCT1:
                cycles = op_pollct1();
                break;
            case p2_OPX24_POLLCT2:
                cycles = op_pollct2();
                break;
            case p2_OPX24_POLLCT3:
                cycles = op_pollct3();
                break;
            case p2_OPX24_POLLSE1:
                cycles = op_pollse1();
                break;
            case p2_OPX24_POLLSE2:
                cycles = op_pollse2();
                break;
            case p2_OPX24_POLLSE3:
                cycles = op_pollse3();
                break;
            case p2_OPX24_POLLSE4:
                cycles = op_pollse4();
                break;
            case p2_OPX24_POLLPAT:
                cycles = op_pollpat();
                break;
            case p2_OPX24_POLLFBW:
                cycles = op_pollfbw();
                break;
            case p2_OPX24_POLLXMT:
                cycles = op_pollxmt();
                break;
            case p2_OPX24_POLLXFI:
                cycles = op_pollxfi();
                break;
            case p2_OPX24_POLLXRO:
                cycles = op_pollxro();
                break;
            case p2_OPX24_POLLXRL:
                cycles = op_pollxrl();
                break;
            case p2_OPX24_POLLATN:
                cycles = op_pollatn();
                break;
            case p2_OPX24_POLLQMT:
                cycles = op_pollqmt();
                break;
            case p2_OPX24_WAITINT:
                cycles = op_waitint();
                break;
            case p2_OPX24_WAITCT1:
                cycles = op_waitct1();
                break;
            case p2_OPX24_WAITCT2:
                cycles = op_waitct2();
                break;
            case p2_OPX24_WAITCT3:
                cycles = op_waitct3();
                break;
            case p2_OPX24_WAITSE1:
                cycles = op_waitse1();
                break;
            case p2_OPX24_WAITSE2:
                cycles = op_waitse2();
                break;
            case p2_OPX24_WAITSE3:
                cycles = op_waitse3();
                break;
            case p2_OPX24_WAITSE4:
                cycles = op_waitse4();
                break;
            case p2_OPX24_WAITPAT:
                cycles = op_waitpat();
                break;
            case p2_OPX24_WAITFBW:
                cycles = op_waitfbw();
                break;
            case p2_OPX24_WAITXMT:
                cycles = op_waitxmt();
                break;
            case p2_OPX24_WAITXFI:
                cycles = op_waitxfi();
                break;
            case p2_OPX24_WAITXRO:
                cycles = op_waitxro();
                break;
            case p2_OPX24_WAITXRL:
                cycles = op_waitxrl();
                break;
            case p2_OPX24_WAITATN:
                cycles = op_waitatn();
                break;
            case p2_OPX24_ALLOWI:
                cycles = op_allowi();
                break;
            case p2_OPX24_STALLI:
                cycles = op_stalli();
                break;
            case p2_OPX24_TRGINT1:
                cycles = op_trgint1();
                break;
            case p2_OPX24_TRGINT2:
                cycles = op_trgint2();
                break;
            case p2_OPX24_TRGINT3:
                cycles = op_trgint3();
                break;
            case p2_OPX24_NIXINT1:
                cycles = op_nixint1();
                break;
            case p2_OPX24_NIXINT2:
                cycles = op_nixint2();
                break;
            case p2_OPX24_NIXINT3:
                cycles = op_nixint3();
                break;
            }
            break;
        case p2_OPSRC_SETINT1:
            cycles = op_setint1();
            break;
        case p2_OPSRC_SETINT2:
            cycles = op_setint2();
            break;
        case p2_OPSRC_SETINT3:
            cycles = op_setint3();
            break;
        case p2_OPSRC_SETQ:
            cycles = op_setq();
            break;
        case p2_OPSRC_SETQ2:
            cycles = op_setq2();
            break;
        case p2_OPSRC_PUSH:
            cycles = op_push();
            break;
        case p2_OPSRC_POP:
            cycles = op_pop();
            break;
        case p2_OPSRC_JMP:
            cycles = op_jmp();
            break;
        case p2_OPSRC_CALL_RET:
            cycles = (IR.op.imm == 0) ? op_call()
                                      : op_ret();
            break;
        case p2_OPSRC_CALLA_RETA:
            cycles = (IR.op.imm == 0) ? op_calla()
                                      : op_reta();
            break;
        case p2_OPSRC_CALLB_RETB:
            cycles = (IR.op.imm == 0) ? op_callb()
                                      : op_retb();
            break;
        case p2_OPSRC_JMPREL:
            cycles = op_jmprel();
            break;
        case p2_OPSRC_SKIP:
            cycles = op_skip();
            break;
        case p2_OPSRC_SKIPF:
            cycles = op_skipf();
            break;
        case p2_OPSRC_EXECF:
            cycles = op_execf();
            break;
        case p2_OPSRC_GETPTR:
            cycles = op_getptr();
            break;
        case p2_OPSRC_COGBRK:
            cycles = (IR.op.wc == 0 && IR.op.wz == 0) ? op_cogbrk()
                                                      : op_getbrk();
            break;
        case p2_OPSRC_BRK:
            cycles = op_brk();
            break;
        case p2_OPSRC_SETLUTS:
            cycles = op_setluts();
            break;
        case p2_OPSRC_SETCY:
            cycles = op_setcy();
            break;
        case p2_OPSRC_SETCI:
            cycles = op_setci();
            break;
        case p2_OPSRC_SETCQ:
            cycles = op_setcq();
            break;
        case p2_OPSRC_SETCFRQ:
            cycles = op_setcfrq();
            break;
        case p2_OPSRC_SETCMOD:
            cycles = op_setcmod();
            break;
        case p2_OPSRC_SETPIV:
            cycles = op_setpiv();
            break;
        case p2_OPSRC_SETPIX:
            cycles = op_setpix();
            break;
        case p2_OPSRC_COGATN:
            cycles = op_cogatn();
            break;
        case p2_OPSRC_TESTP_W_DIRL:
            cycles = (IR.op.wc == IR.op.wz) ? op_testp_w()
                                            : op_dirl();
            break;
        case p2_OPSRC_TESTPN_W_DIRH:
            cycles = (IR.op.wc == IR.op.wz) ? op_testpn_w()
                                            : op_dirh();
            break;
        case p2_OPSRC_TESTP_AND_DIRC:
            cycles = (IR.op.wc == IR.op.wz) ? op_testp_and()
                                            : op_dirc();
            break;
        case p2_OPSRC_TESTPN_AND_DIRNC:
            cycles = (IR.op.wc == IR.op.wz) ? op_testpn_and()
                                            : op_dirnc();
            break;
        case p2_OPSRC_TESTP_OR_DIRZ:
            cycles = (IR.op.wc == IR.op.wz) ? op_testp_or()
                                            : op_dirz();
            break;
        case p2_OPSRC_TESTPN_OR_DIRNZ:
            cycles = (IR.op.wc == IR.op.wz) ? op_testpn_or()
                                            : op_dirnz();
            break;
        case p2_OPSRC_TESTP_XOR_DIRRND:
            cycles = (IR.op.wc == IR.op.wz) ? op_testp_xor()
                                            : op_dirrnd();
            break;
        case p2_OPSRC_TESTPN_XOR_DIRNOT:
            cycles = (IR.op.wc == IR.op.wz) ? op_testpn_xor()
                                            : op_dirnot();
            break;

        case p2_OPSRC_OUTL:
            cycles = op_outl();
            break;
        case p2_OPSRC_OUTH:
            cycles = op_outh();
            break;
        case p2_OPSRC_OUTC:
            cycles = op_outc();
            break;
        case p2_OPSRC_OUTNC:
            cycles = op_outnc();
            break;
        case p2_OPSRC_OUTZ:
            cycles = op_outz();
            break;
        case p2_OPSRC_OUTNZ:
            cycles = op_outnz();
            break;
        case p2_OPSRC_OUTRND:
            cycles = op_outrnd();
            break;
        case p2_OPSRC_OUTNOT:
            cycles = op_outnot();
            break;

        case p2_OPSRC_FLTL:
            cycles = op_fltl();
            break;
        case p2_OPSRC_FLTH:
            cycles = op_flth();
            break;
        case p2_OPSRC_FLTC:
            cycles = op_fltc();
            break;
        case p2_OPSRC_FLTNC:
            cycles = op_fltnc();
            break;
        case p2_OPSRC_FLTZ:
            cycles = op_fltz();
            break;
        case p2_OPSRC_FLTNZ:
            cycles = op_fltnz();
            break;
        case p2_OPSRC_FLTRND:
            cycles = op_fltrnd();
            break;
        case p2_OPSRC_FLTNOT:
            cycles = op_fltnot();
            break;

        case p2_OPSRC_DRVL:
            cycles = op_drvl();
            break;
        case p2_OPSRC_DRVH:
            cycles = op_drvh();
            break;
        case p2_OPSRC_DRVC:
            cycles = op_drvc();
            break;
        case p2_OPSRC_DRVNC:
            cycles = op_drvnc();
            break;
        case p2_OPSRC_DRVZ:
            cycles = op_drvz();
            break;
        case p2_OPSRC_DRVNZ:
            cycles = op_drvnz();
            break;
        case p2_OPSRC_DRVRND:
            cycles = op_drvrnd();
            break;
        case p2_OPSRC_DRVNOT:
            cycles = op_drvnot();
            break;

        case p2_OPSRC_SPLITB:
            cycles = op_splitb();
            break;
        case p2_OPSRC_MERGEB:
            cycles = op_mergeb();
            break;
        case p2_OPSRC_SPLITW:
            cycles = op_splitw();
            break;
        case p2_OPSRC_MERGEW:
            cycles = op_mergew();
            break;
        case p2_OPSRC_SEUSSF:
            cycles = op_seussf();
            break;
        case p2_OPSRC_SEUSSR:
            cycles = op_seussr();
            break;
        case p2_OPSRC_RGBSQZ:
            cycles = op_rgbsqz();
            break;
        case p2_OPSRC_RGBEXP:
            cycles = op_rgbexp();
            break;
        case p2_OPSRC_XORO32:
            cycles = op_xoro32();
            break;
        case p2_OPSRC_REV:
            cycles = op_rev();
            break;
        case p2_OPSRC_RCZR:
            cycles = op_rczr();
            break;
        case p2_OPSRC_RCZL:
            cycles = op_rczl();
            break;
        case p2_OPSRC_WRC:
            cycles = op_wrc();
            break;
        case p2_OPSRC_WRNC:
            cycles = op_wrnc();
            break;
        case p2_OPSRC_WRZ:
            cycles = op_wrz();
            break;
        case p2_OPSRC_WRNZ_MODCZ:
            cycles = (IR.op.wc || IR.op.wz) ? op_modcz()
                                            : op_wrnz();
            break;
        case p2_OPSRC_SETSCP:
            cycles = op_setscp();
            break;
        case p2_OPSRC_GETSCP:
            cycles = op_getscp();
            break;
        }
        break;

    case p2_JMP_ABS:
        cycles = op_jmp_abs();
        break;

    case p2_CALL_ABS:
        cycles = op_call_abs();
        break;

    case p2_CALLA_ABS:
        cycles = op_calla_abs();
        break;

    case p2_CALLB_ABS:
        cycles = op_callb_abs();
        break;

    case p2_CALLD_PA_ABS:
        cycles = op_calld_pa_abs();
        break;

    case p2_CALLD_PB_ABS:
        cycles = op_calld_pb_abs();
        break;

    case p2_CALLD_PTRA_ABS:
        cycles = op_calld_ptra_abs();
        break;

    case p2_CALLD_PTRB_ABS:
        cycles = op_calld_ptrb_abs();
        break;

    case p2_LOC_PA:
        cycles = op_loc_pa();
        break;

    case p2_LOC_PB:
        cycles = op_loc_pb();
        break;

    case p2_LOC_PTRA:
        cycles = op_loc_ptra();
        break;

    case p2_LOC_PTRB:
        cycles = op_loc_ptrb();
        break;

    case p2_AUGS_00:
    case p2_AUGS_01:
    case p2_AUGS_10:
    case p2_AUGS_11:
        cycles = op_augs();
        break;

    case p2_AUGD_00:
    case p2_AUGD_01:
    case p2_AUGD_10:
    case p2_AUDG_11:
        cycles = op_augd();
        break;
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
int P2Cog::op_nop()
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
int P2Cog::op_ror()
{
    if (0 == IR.word)
        return op_nop();
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint64 accu = U64(D) << 32 | U64(D);
    const P2LONG result = U32L(accu >> shift);
    updateC((D & (LSB << shift)) != 0);
    updateZ(result == 0);
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
int P2Cog::op_rol()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint64 accu = U64(D) << 32 | U64(D);
    const P2LONG result = U32H(accu << shift);
    updateC((D & (MSB >> shift)) != 0);
    updateZ(result == 0);
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
int P2Cog::op_shr()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint64 accu = U64(D);
    const P2LONG result = U32L(accu >> shift);
    updateC((D & (LSB << shift)) != 0);
    updateZ(result == 0);
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
int P2Cog::op_shl()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint64 accu = U64(D) << 32;
    const P2LONG result = U32H(accu << shift);
    updateC((D & (MSB >> shift)) != 0);
    updateZ(result == 0);
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
int P2Cog::op_rcr()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint64 accu = U64(D) | C ? HMAX : 0;
    const P2LONG result = U32L(accu >> shift);
    updateC((D & (LSB << shift)) != 0);
    updateZ(result == 0);
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
int P2Cog::op_rcl()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint64 accu = U64(D) << 32 | C ? LMAX : 0;
    const P2LONG result = U32H(accu << shift);
    updateC((D & (MSB >> shift)) != 0);
    updateZ(result == 0);
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
int P2Cog::op_sar()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint64 accu = U64(D) | (D & MSB) ? HMAX : 0;
    const P2LONG result = U32L(accu >> shift);
    updateC((D & (LSB << shift)) != 0);
    updateZ(result == 0);
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
int P2Cog::op_sal()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint64 accu = U64(D) << 32 | (D & LSB) ? LMAX : 0;
    const P2LONG result = U32H(accu << shift);
    updateC((D & (MSB >> shift)) != 0);
    updateZ(result == 0);
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
int P2Cog::op_add()
{
    augmentS(IR.op.imm);
    const quint64 accu = U64(D) + U64(S);
    const P2LONG result = U32L(accu);
    updateC((accu >> 32) & 1);
    updateZ(result == 0);
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
int P2Cog::op_addx()
{
    augmentS(IR.op.imm);
    const quint64 accu = U64(D) + U64(S) + C;
    const P2LONG result = U32L(accu);
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
int P2Cog::op_adds()
{
    augmentS(IR.op.imm);
    const bool sign = (S32(D) ^ S32(S)) < 0;
    const qint64 accu = SX64(D) + SX64(S);
    const P2LONG result = U32L(accu);
    updateC((accu < 0) ^ sign);
    updateZ(result == 0);
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
int P2Cog::op_addsx()
{
    augmentS(IR.op.imm);
    const uchar sign = (D ^ (S + C)) >> 31;
    const qint64 accu = SX64(D) + SX64(S) + C;
    const P2LONG result = U32L(accu);
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
int P2Cog::op_sub()
{
    augmentS(IR.op.imm);
    const quint64 accu = U64(D) - U64(S);
    const P2LONG result = U32L(accu);
    updateC((accu >> 32) & 1);
    updateZ(result == 0);
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
int P2Cog::op_subx()
{
    augmentS(IR.op.imm);
    const quint64 accu = U64(D) - (U64(S) + C);
    const P2LONG result = U32L(accu);
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
int P2Cog::op_subs()
{
    augmentS(IR.op.imm);
    const bool sign = (S32(D) ^ S32(S)) < 0;
    const qint64 accu = SX64(D) - SX64(S);
    const P2LONG result = U32L(accu);
    updateC((accu < 0) ^ sign);
    updateZ(result == 0);
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
int P2Cog::op_subsx()
{
    augmentS(IR.op.imm);
    const uchar sign = (D ^ (S + C)) >> 31;
    const qint64 accu = SX64(D) - (SX64(S) + C);
    const P2LONG result = U32L(accu);
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
int P2Cog::op_cmp()
{
    augmentS(IR.op.imm);
    const quint64 accu = U64(D) - U64(S);
    const P2LONG result = U32L(accu);
    updateC((accu >> 32) & 1);
    updateZ(result == 0);
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
int P2Cog::op_cmpx()
{
    augmentS(IR.op.imm);
    const quint64 accu = U64(D) - (U64(S) + C);
    const P2LONG result = U32L(accu);
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
int P2Cog::op_cmps()
{
    augmentS(IR.op.imm);
    const bool sign = (S32(D) ^ S32(S)) < 0;
    const qint64 accu = SX64(D) - SX64(S);
    const P2LONG result = U32L(accu);
    updateC((accu < 0) ^ sign);
    updateZ(result == 0);
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
int P2Cog::op_cmpsx()
{
    augmentS(IR.op.imm);
    const uchar sign = (D ^ (S + C)) >> 31;
    const qint64 accu = SX64(D) - (SX64(S) + C);
    const P2LONG result = U32L(accu);
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
int P2Cog::op_cmpr()
{
    augmentS(IR.op.imm);
    const quint64 accu = U64(S) - U64(D);
    const P2LONG result = U32L(accu);
    updateC((accu >> 32) & 1);
    updateZ(result == 0);
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
int P2Cog::op_cmpm()
{
    augmentS(IR.op.imm);
    const quint64 accu = U64(D) - U64(S);
    const P2LONG result = U32L(accu);
    updateC((accu >> 31) & 1);
    updateZ(result == 0);
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
int P2Cog::op_subr()
{
    augmentS(IR.op.imm);
    const quint64 accu = U64(S) - U64(D);
    const P2LONG result = U32L(accu);
    updateC((accu >> 32) & 1);
    updateZ(result == 0);
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
int P2Cog::op_cmpsub()
{
    augmentS(IR.op.imm);
    if (D < S) {
        // Do not change D
        const P2LONG result = D;
        updateC(0);
        updateZ(result == 0);
    } else {
        // Do the subtract and set C = 1, if WC is set
        const quint64 accu = U64(D) - U64(S);
        const P2LONG result = U32L(accu);
        updateC(1);
        updateZ(result == 0);
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
int P2Cog::op_fge()
{
    augmentS(IR.op.imm);
    if (D < S) {
        const P2LONG result = S;
        updateC(1);
        updateZ(result == 0);
        updateD(result);
    } else {
        const P2LONG result = D;
        updateC(0);
        updateZ(result == 0);
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
int P2Cog::op_fle()
{
    augmentS(IR.op.imm);
    if (D > S) {
        const P2LONG result = S;
        updateC(1);
        updateZ(result == 0);
        updateD(result);
    } else {
        const P2LONG result = D;
        updateC(0);
        updateZ(result == 0);
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
int P2Cog::op_fges()
{
    augmentS(IR.op.imm);
    if (S32(D) < S32(S)) {
        const P2LONG result = S;
        updateC(1);
        updateZ(result == 0);
        updateD(result);
    } else {
        const P2LONG result = D;
        updateC(0);
        updateZ(result == 0);
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
int P2Cog::op_fles()
{
    augmentS(IR.op.imm);
    if (S32(D) > S32(S)) {
        const P2LONG result = S;
        updateC(1);
        updateZ(result == 0);
        updateD(result);
    } else {
        const P2LONG result = D;
        updateC(0);
        updateZ(result == 0);
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
int P2Cog::op_sumc()
{
    augmentS(IR.op.imm);
    const bool sign = (S32(D) ^ S32(S)) < 0;
    const quint64 accu = C ? U64(D) - U64(S) : U64(D) + U64(S);
    const P2LONG result = U32L(accu);
    updateC(((accu >> 32) & 1) ^ sign);
    updateZ(result == 0);
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
int P2Cog::op_sumnc()
{
    augmentS(IR.op.imm);
    const bool sign = (S32(D) ^ S32(S)) < 0;
    const quint64 accu = !C ? U64(D) - U64(S) : U64(D) + U64(S);
    const P2LONG result = U32L(accu);
    updateC(((accu >> 32) & 1) ^ sign);
    updateZ(result == 0);
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
int P2Cog::op_sumz()
{
    augmentS(IR.op.imm);
    const bool sign = (S32(D) ^ S32(S)) < 0;
    const quint64 accu = Z ? U64(D) - U64(S) : U64(D) + U64(S);
    const P2LONG result = U32L(accu);
    updateC(((accu >> 32) & 1) ^ sign);
    updateZ(result == 0);
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
int P2Cog::op_sumnz()
{
    augmentS(IR.op.imm);
    const bool sign = (S32(D) ^ S32(S)) < 0;
    const quint64 accu = !Z ? U64(D) - U64(S) : U64(D) + U64(S);
    const P2LONG result = U32L(accu);
    updateC(((accu >> 32) & 1) ^ sign);
    updateZ(result == 0);
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
int P2Cog::op_testb_w()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_testbn_w()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_testb_and()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_testbn_and()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_testb_or()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_testbn_or()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_testb_xor()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_testbn_xor()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_bitl()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const P2LONG bit = LSB << shift;
    const P2LONG result = D & ~bit;
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
int P2Cog::op_bith()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const P2LONG bit = LSB << shift;
    const P2LONG result = D | bit;
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
int P2Cog::op_bitc()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const P2LONG bit = LSB << shift;
    const P2LONG result = C ? D | bit : D & ~bit;
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
int P2Cog::op_bitnc()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const P2LONG bit = LSB << shift;
    const P2LONG result = !C ? D | bit : D & ~bit;
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
int P2Cog::op_bitz()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const P2LONG bit = LSB << shift;
    const P2LONG result = Z ? D | bit : D & ~bit;
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
int P2Cog::op_bitnz()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const P2LONG bit = LSB << shift;
    const P2LONG result = !Z ? D | bit : D & ~bit;
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
int P2Cog::op_bitrnd()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const P2LONG bit = LSB << shift;
    const P2LONG result = (HUB->random(shift) & 1) ? D | bit : D & ~bit;
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
int P2Cog::op_bitnot()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const P2LONG bit = LSB << shift;
    const P2LONG result = D ^ bit;
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
int P2Cog::op_and()
{
    augmentS(IR.op.imm);
    const P2LONG result = D & S;
    updateC(parity(result));
    updateZ(result == 0);
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
int P2Cog::op_andn()
{
    augmentS(IR.op.imm);
    const P2LONG result = D & ~S;
    updateC(parity(result));
    updateZ(result == 0);
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
int P2Cog::op_or()
{
    augmentS(IR.op.imm);
    const P2LONG result = D | S;
    updateC(parity(result));
    updateZ(result == 0);
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
int P2Cog::op_xor()
{
    augmentS(IR.op.imm);
    const P2LONG result = D ^ S;
    updateC(parity(result));
    updateZ(result == 0);
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
int P2Cog::op_muxc()
{
    augmentS(IR.op.imm);
    const P2LONG result = (D & ~S) | (C ? S : 0);
    updateC(parity(result));
    updateZ(result == 0);
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
int P2Cog::op_muxnc()
{
    augmentS(IR.op.imm);
    const P2LONG result = (D & ~S) | (!C ? S : 0);
    updateC(parity(result));
    updateZ(result == 0);
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
int P2Cog::op_muxz()
{
    augmentS(IR.op.imm);
    const P2LONG result = (D & ~S) | (Z ? S : 0);
    updateC(parity(result));
    updateZ(result == 0);
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
int P2Cog::op_muxnz()
{
    augmentS(IR.op.imm);
    const P2LONG result = (D & ~S) | (!Z ? S : 0);
    updateC(parity(result));
    updateZ(result == 0);
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
int P2Cog::op_mov()
{
    augmentS(IR.op.imm);
    const P2LONG result = S;
    updateC(result >> 31);
    updateZ(result == 0);
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
int P2Cog::op_not()
{
    augmentS(IR.op.imm);
    const P2LONG result = ~S;
    updateC(result >> 31);
    updateZ(result == 0);
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
int P2Cog::op_abs()
{
    augmentS(IR.op.imm);
    const qint32 result = qAbs(S32(S));
    updateC(S >> 31);
    updateZ(result == 0);
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
int P2Cog::op_neg()
{
    augmentS(IR.op.imm);
    const qint32 result = 0 - S32(S);
    updateC(result >> 31);
    updateZ(result == 0);
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
int P2Cog::op_negc()
{
    augmentS(IR.op.imm);
    const qint32 result = C ? 0 - S32(S) : S32(32);
    updateC(result >> 31);
    updateZ(result == 0);
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
int P2Cog::op_negnc()
{
    augmentS(IR.op.imm);
    const qint32 result = !C ? 0 - S32(S) : S32(32);
    updateC(result >> 31);
    updateZ(result == 0);
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
int P2Cog::op_negz()
{
    augmentS(IR.op.imm);
    const qint32 result = Z ? 0 - S32(S) : S32(32);
    updateC(result >> 31);
    updateZ(result == 0);
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
int P2Cog::op_negnz()
{
    augmentS(IR.op.imm);
    const qint32 result = !Z ? 0 - S32(S) : S32(32);
    updateC(result >> 31);
    updateZ(result == 0);
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
int P2Cog::op_incmod()
{
    augmentS(IR.op.imm);
    const P2LONG result = (D == S) ? 0 : D + 1;
    updateC(result == 0);
    updateZ(result == 0);
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
int P2Cog::op_decmod()
{
    augmentS(IR.op.imm);
    const P2LONG result = (D == 0) ? S : D - 1;
    updateC(result == S);
    updateZ(result == 0);
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
int P2Cog::op_zerox()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const P2LONG msb = (D >> (shift - 1)) & 1;
    const P2LONG mask = 0xffffffffu << shift;
    const P2LONG result = D & ~mask;
    updateC(msb);
    updateZ(result == 0);
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
int P2Cog::op_signx()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const P2LONG msb = (D >> (shift - 1)) & 1;
    const P2LONG mask = UMAX << shift;
    const P2LONG result = msb ? D | mask : D & ~mask;
    updateC(msb);
    updateZ(result == 0);
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
int P2Cog::op_encod()
{
    augmentS(IR.op.imm);
    const P2LONG result = msbit(S);
    updateC(S != 0);
    updateZ(result == 0);
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
int P2Cog::op_ones()
{
    augmentS(IR.op.imm);
    const P2LONG result = ones(S);
    updateC(result & 1);
    updateZ(result == 0);
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
int P2Cog::op_test()
{
    augmentS(IR.op.imm);
    const P2LONG result = D & S;
    updateC(parity(result));
    updateZ(result == 0);
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
int P2Cog::op_testn()
{
    augmentS(IR.op.imm);
    const P2LONG result = D & ~S;
    updateC(parity(result));
    updateZ(result == 0);
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
int P2Cog::op_setnib()
{
    augmentS(IR.op.imm);
    const uchar shift = static_cast<uchar>((IR.word >> 19) & 7) * 4;
    const P2LONG mask = LNIBBLE << shift;
    const P2LONG result = (D & ~mask) | ((S << shift) & mask);
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
int P2Cog::op_setnib_altsn()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_getnib()
{
    augmentS(IR.op.imm);
    const uchar shift = static_cast<uchar>((IR.word >> 19) & 7) * 4;
    const P2LONG result = (S >> shift) & LNIBBLE;
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
int P2Cog::op_getnib_altgn()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_rolnib()
{
    augmentS(IR.op.imm);
    const uchar shift = static_cast<uchar>((IR.word >> 19) & 7) * 4;
    const P2LONG result = (D << 4) | ((S >> shift) & LNIBBLE);
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
int P2Cog::op_rolnib_altgn()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_setbyte()
{
    augmentS(IR.op.imm);
    const uchar shift = static_cast<uchar>((IR.word >> 19) & 3) * 8;
    const P2LONG mask = LBYTE << shift;
    const P2LONG result = (D & ~mask) | ((S << shift) & mask);
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
int P2Cog::op_setbyte_altsb()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_getbyte()
{
    augmentS(IR.op.imm);
    const uchar shift = static_cast<uchar>((IR.word >> 19) & 3) * 8;
    const P2LONG result = (S >> shift) & LBYTE;
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
int P2Cog::op_getbyte_altgb()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_rolbyte()
{
    augmentS(IR.op.imm);
    const uchar shift = static_cast<uchar>((IR.word >> 19) & 3) * 8;
    const P2LONG result = (D << 8) | ((S >> shift) & LBYTE);
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
int P2Cog::op_rolbyte_altgb()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_setword()
{
    augmentS(IR.op.imm);
    const uchar shift = IR.op.wz ? 16 : 0;
    const P2LONG mask = LWORD << shift;
    const P2LONG result = (D & ~mask) | ((S >> shift) & mask);
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
int P2Cog::op_setword_altsw()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_getword()
{
    augmentS(IR.op.imm);
    const uchar shift = IR.op.wz * 16;
    const P2LONG result = (S >> shift) & LWORD;
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
int P2Cog::op_getword_altgw()
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
int P2Cog::op_rolword()
{
    augmentS(IR.op.imm);
    const uchar shift = IR.op.wz * 16;
    const P2LONG result = (D << 16) & ((S >> shift) & LWORD);
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
int P2Cog::op_rolword_altgw()
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
int P2Cog::op_altsn()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_altsn_d()
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
int P2Cog::op_altgn()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_altgn_d()
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
int P2Cog::op_altsb()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_altsb_d()
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
int P2Cog::op_altgb()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_altgb_d()
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
int P2Cog::op_altsw()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_altsw_d()
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
int P2Cog::op_altgw()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_altgw_d()
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
int P2Cog::op_altr()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_altr_d()
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
int P2Cog::op_altd()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_altd_d()
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
int P2Cog::op_alts()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_alts_d()
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
int P2Cog::op_altb()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_altb_d()
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
int P2Cog::op_alti()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_alti_d()
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
int P2Cog::op_setr()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_setd()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_sets()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_decod()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const P2LONG result = LSB << shift;
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
int P2Cog::op_decod_d()
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
int P2Cog::op_bmask()
{
    const uchar shift = S & 31;
    const P2LONG result = U32L((Q_UINT64_C(2) << shift) - 1);
    updateD(result);
    augmentS(IR.op.imm);
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
int P2Cog::op_bmask_d()
{
    const uchar shift = D & 31;
    const P2LONG result = U32L((Q_UINT64_C(2) << shift) - 1);
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
int P2Cog::op_crcbit()
{
    augmentS(IR.op.imm);
    const P2LONG result = ((C ^ D) & 1) ? (D >> 1) ^ S : (D >> 1);
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
int P2Cog::op_crcnib()
{
    augmentS(IR.op.imm);
    // FIXME: Huh?
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
int P2Cog::op_muxnits()
{
    augmentS(IR.op.imm);
    const P2LONG mask = S | ((S & 0xaaaaaaaa) >> 1) | ((S & 0x55555555) << 1);
    const P2LONG result = (D & ~mask) | (S & mask);
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
int P2Cog::op_muxnibs()
{
    augmentS(IR.op.imm);
    const P2LONG mask0 = S | ((S & 0xaaaaaaaa) >> 1) | ((S & 0x55555555) << 1);
    const P2LONG mask1 = ((mask0 & 0xcccccccc) >> 2) | ((mask0 & 0x33333333) << 2);
    const P2LONG result = (D & ~mask1) | (S & mask1);
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
int P2Cog::op_muxq()
{
    augmentS(IR.op.imm);
    const P2LONG result = (D & ~Q) | (S & Q);
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
int P2Cog::op_movbyts()
{
    augmentS(IR.op.imm);
    union {
        P2LONG l;
        P2BYTE b[4];
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
int P2Cog::op_mul()
{
    augmentS(IR.op.imm);
    const P2LONG result = U16(D) * U16(S);
    updateZ(result == 0);
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
int P2Cog::op_muls()
{
    augmentS(IR.op.imm);
    const qint32 result = S16(D) * S16(S);
    updateZ(result == 0);
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
int P2Cog::op_sca()
{
    augmentS(IR.op.imm);
    const P2LONG result = (U16(D) * U16(S)) >> 16;
    updateZ(result == 0);
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
int P2Cog::op_scas()
{
    augmentS(IR.op.imm);
    const qint32 result = (S16(D) * S16(S)) >> 14;
    updateZ(result == 0);
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
int P2Cog::op_addpix()
{
    augmentS(IR.op.imm);
    union {
        P2LONG l;
        P2BYTE b[4];
    }   dst, src;
    dst.l = D;
    src.l = S;
    dst.b[0] = qMin<P2BYTE>(dst.b[0] + src.b[0], 0xff);
    dst.b[1] = qMin<P2BYTE>(dst.b[1] + src.b[1], 0xff);
    dst.b[2] = qMin<P2BYTE>(dst.b[2] + src.b[2], 0xff);
    dst.b[3] = qMin<P2BYTE>(dst.b[3] + src.b[3], 0xff);
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
int P2Cog::op_mulpix()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_blnpix()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_mixpix()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_addct1()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_addct2()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_addct3()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_wmlong()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_rqpin()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_rdpin()
{
    augmentS(IR.op.imm);
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
int P2Cog::op_rdlut()
{
    augmentS(IR.op.imm);
    P2LONG result;
    if (S == offs_PTRA) {
        result = HUB->rd_LONG(LUT.REG.PTRA);
    } else if (S == offs_PTRB) {
        result = HUB->rd_LONG(LUT.REG.PTRB);
    } else {
        result = LUT.RAM[S & 0x1ff];
    }
    updateC((result >> 31) & 1);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Read zero-extended byte from hub address {#}S/PTRx into D.
 *
 * EEEE 1010110 CZI DDDDDDDDD SSSSSSSSS
 *
 * RDBYTE  D,{#}S/P {WC/WZ/WCZ}
 *
 * C = MSB of byte.
 * Z = (result == 0).
 */
int P2Cog::op_rdbyte()
{
    augmentS(IR.op.imm);
    P2BYTE result;
    if (S == offs_PTRA) {
        result = HUB->rd_BYTE(LUT.REG.PTRA);
    } else if (S == offs_PTRB) {
        result = HUB->rd_BYTE(LUT.REG.PTRB);
    } else {
        result = HUB->rd_BYTE(S);
    }
    updateC((result >> 7) & 1);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Read zero-extended word from hub address {#}S/PTRx into D.
 *
 * EEEE 1010111 CZI DDDDDDDDD SSSSSSSSS
 *
 * RDWORD  D,{#}S/P {WC/WZ/WCZ}
 *
 * C = MSB of word.
 * Z = (result == 0).
 */
int P2Cog::op_rdword()
{
    augmentS(IR.op.imm);
    P2WORD result;
    if (S == offs_PTRA) {
        result = HUB->rd_WORD(LUT.REG.PTRA);
    } else if (S == offs_PTRB) {
        result = HUB->rd_WORD(LUT.REG.PTRB);
    } else {
        result = HUB->rd_WORD(S);
    }
    updateC((result >> 15) & 1);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Read long from hub address {#}S/PTRx into D.
 *
 * EEEE 1011000 CZI DDDDDDDDD SSSSSSSSS
 *
 * RDLONG  D,{#}S/P {WC/WZ/WCZ}
 *
 * C = MSB of long.
 * *   Prior SETQ/SETQ2 invokes cog/LUT block transfer.
 */
int P2Cog::op_rdlong()
{
    augmentS(IR.op.imm);
    P2LONG result;
    if (S == offs_PTRA) {
        result = HUB->rd_LONG(LUT.REG.PTRA);
    } else if (S == offs_PTRB) {
        result = HUB->rd_LONG(LUT.REG.PTRB);
    } else {
        result = HUB->rd_LONG(S);
    }
    updateC((result >> 15) & 1);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Read long from hub address --PTRA into D.
 *
 * EEEE 1011000 CZ1 DDDDDDDDD 101011111
 *
 * POPA    D        {WC/WZ/WCZ}
 *
 * C = MSB of long.
 * Z = (result == 0).
 */
int P2Cog::op_popa()
{
    return 2;
}

/**
 * @brief Read long from hub address --PTRB into D.
 *
 * EEEE 1011000 CZ1 DDDDDDDDD 111011111
 *
 * POPB    D        {WC/WZ/WCZ}
 *
 * C = MSB of long.
 * Z = (result == 0).
 */
int P2Cog::op_popb()
{
    return 2;
}

/**
 * @brief Call to S** by writing {C, Z, 10'b0, PC[19:0]} to D.
 *
 * EEEE 1011001 CZI DDDDDDDDD SSSSSSSSS
 *
 * CALLD   D,{#}S   {WC/WZ/WCZ}
 *
 * C = S[31], Z = S[30].
 */
int P2Cog::op_calld()
{
    augmentS(IR.op.imm);
    if (IR.op.wc && IR.op.wz) {
        if (IR.op.dst == offs_IJMP3 && IR.op.src == offs_IRET3)
            return op_resi3();
        if (IR.op.dst == offs_IJMP2 && IR.op.src == offs_IRET2)
            return op_resi2();
        if (IR.op.dst == offs_IJMP1 && IR.op.src == offs_IRET1)
            return op_resi1();
        if (IR.op.dst == offs_INA && IR.op.src == offs_INB)
            return op_resi0();
        if (IR.op.dst == offs_INB && IR.op.src == offs_IRET3)
            return op_reti3();
        if (IR.op.dst == offs_INB && IR.op.src == offs_IRET2)
            return op_reti2();
        if (IR.op.dst == offs_INB && IR.op.src == offs_IRET1)
            return op_reti1();
        if (IR.op.dst == offs_INB && IR.op.src == offs_INB)
            return op_reti0();
    }
    const P2LONG result = (U32(C) << 31) | (U32(Z) << 30) | PC;
    updateC((S >> 31) & 1);
    updateZ((S >> 30) & 1);
    updateD(result);
    updatePC(S & A20MASK);
    return 2;
}

/**
 * @brief Resume from INT3.
 *
 * EEEE 1011001 110 111110000 111110001
 *
 * RESI3
 *
 * (CALLD $1F0,$1F1 WC,WZ).
 */
int P2Cog::op_resi3()
{
    return 2;
}

/**
 * @brief Resume from INT2.
 *
 * EEEE 1011001 110 111110010 111110011
 *
 * RESI2
 *
 * (CALLD $1F2,$1F3 WC,WZ).
 */
int P2Cog::op_resi2()
{
    return 2;
}

/**
 * @brief Resume from INT1.
 *
 * EEEE 1011001 110 111110100 111110101
 *
 * RESI1
 *
 * (CALLD $1F4,$1F5 WC,WZ).
 */
int P2Cog::op_resi1()
{
    return 2;
}

/**
 * @brief Resume from INT0.
 *
 * EEEE 1011001 110 111111110 111111111
 *
 * RESI0
 *
 * (CALLD $1FE,$1FF WC,WZ).
 */
int P2Cog::op_resi0()
{
    return 2;
}

/**
 * @brief Return from INT3.
 *
 * EEEE 1011001 110 111111111 111110001
 *
 * RETI3
 *
 * (CALLD $1FF,$1F1 WC,WZ).
 */
int P2Cog::op_reti3()
{
    return 2;
}

/**
 * @brief Return from INT2.
 *
 * EEEE 1011001 110 111111111 111110011
 *
 * RETI2
 *
 * (CALLD $1FF,$1F3 WC,WZ).
 */
int P2Cog::op_reti2()
{
    return 2;
}

/**
 * @brief Return from INT1.
 *
 * EEEE 1011001 110 111111111 111110101
 *
 * RETI1
 *
 * (CALLD $1FF,$1F5 WC,WZ).
 */
int P2Cog::op_reti1()
{
    return 2;
}

/**
 * @brief Return from INT0.
 *
 * EEEE 1011001 110 111111111 111111111
 *
 * RETI0
 *
 * (CALLD $1FF,$1FF WC,WZ).
 */
int P2Cog::op_reti0()
{
    return 2;
}

/**
 * @brief Call to S** by pushing {C, Z, 10'b0, PC[19:0]} onto stack, copy D to PA.
 *
 * EEEE 1011010 0LI DDDDDDDDD SSSSSSSSS
 *
 * CALLPA  {#}D,{#}S
 *
 */
int P2Cog::op_callpa()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Call to S** by pushing {C, Z, 10'b0, PC[19:0]} onto stack, copy D to PB.
 *
 * EEEE 1011010 1LI DDDDDDDDD SSSSSSSSS
 *
 * CALLPB  {#}D,{#}S
 *
 */
int P2Cog::op_callpb()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Decrement D and jump to S** if result is zero.
 *
 * EEEE 1011011 00I DDDDDDDDD SSSSSSSSS
 *
 * DJZ     D,{#}S
 *
 */
int P2Cog::op_djz()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Decrement D and jump to S** if result is not zero.
 *
 * EEEE 1011011 01I DDDDDDDDD SSSSSSSSS
 *
 * DJNZ    D,{#}S
 *
 */
int P2Cog::op_djnz()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Decrement D and jump to S** if result is $FFFF_FFFF.
 *
 * EEEE 1011011 10I DDDDDDDDD SSSSSSSSS
 *
 * DJF     D,{#}S
 *
 */
int P2Cog::op_djf()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Decrement D and jump to S** if result is not $FFFF_FFFF.
 *
 * EEEE 1011011 11I DDDDDDDDD SSSSSSSSS
 *
 * DJNF    D,{#}S
 *
 */
int P2Cog::op_djnf()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Increment D and jump to S** if result is zero.
 *
 * EEEE 1011100 00I DDDDDDDDD SSSSSSSSS
 *
 * IJZ     D,{#}S
 *
 */
int P2Cog::op_ijz()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Increment D and jump to S** if result is not zero.
 *
 * EEEE 1011100 01I DDDDDDDDD SSSSSSSSS
 *
 * IJNZ    D,{#}S
 *
 */
int P2Cog::op_ijnz()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Test D and jump to S** if D is zero.
 *
 * EEEE 1011100 10I DDDDDDDDD SSSSSSSSS
 *
 * TJZ     D,{#}S
 *
 */
int P2Cog::op_tjz()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Test D and jump to S** if D is not zero.
 *
 * EEEE 1011100 11I DDDDDDDDD SSSSSSSSS
 *
 * TJNZ    D,{#}S
 *
 */
int P2Cog::op_tjnz()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Test D and jump to S** if D is full (D = $FFFF_FFFF).
 *
 * EEEE 1011101 00I DDDDDDDDD SSSSSSSSS
 *
 * TJF     D,{#}S
 *
 */
int P2Cog::op_tjf()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Test D and jump to S** if D is not full (D != $FFFF_FFFF).
 *
 * EEEE 1011101 01I DDDDDDDDD SSSSSSSSS
 *
 * TJNF    D,{#}S
 *
 */
int P2Cog::op_tjnf()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Test D and jump to S** if D is signed (D[31] = 1).
 *
 * EEEE 1011101 10I DDDDDDDDD SSSSSSSSS
 *
 * TJS     D,{#}S
 *
 */
int P2Cog::op_tjs()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Test D and jump to S** if D is not signed (D[31] = 0).
 *
 * EEEE 1011101 11I DDDDDDDDD SSSSSSSSS
 *
 * TJNS    D,{#}S
 *
 */
int P2Cog::op_tjns()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Test D and jump to S** if D overflowed (D[31] != C, C = 'correct sign' from last addition/subtraction).
 *
 * EEEE 1011110 00I DDDDDDDDD SSSSSSSSS
 *
 * TJV     D,{#}S
 *
 */
int P2Cog::op_tjv()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Jump to S** if INT event flag is set.
 *
 * EEEE 1011110 01I 000000000 SSSSSSSSS
 *
 * JINT    {#}S
 *
 */
int P2Cog::op_jint()
{
    augmentS(IR.op.imm);
    if (FLAGS.f_INT)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if CT1 event flag is set.
 *
 * EEEE 1011110 01I 000000001 SSSSSSSSS
 *
 * JCT1    {#}S
 *
 */
int P2Cog::op_jct1()
{
    augmentS(IR.op.imm);
    if (FLAGS.f_CT1)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if CT2 event flag is set.
 *
 * EEEE 1011110 01I 000000010 SSSSSSSSS
 *
 * JCT2    {#}S
 *
 */
int P2Cog::op_jct2()
{
    augmentS(IR.op.imm);
    if (FLAGS.f_CT2)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if CT3 event flag is set.
 *
 * EEEE 1011110 01I 000000011 SSSSSSSSS
 *
 * JCT3    {#}S
 *
 */
int P2Cog::op_jct3()
{
    augmentS(IR.op.imm);
    if (FLAGS.f_CT3)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if SE1 event flag is set.
 *
 * EEEE 1011110 01I 000000100 SSSSSSSSS
 *
 * JSE1    {#}S
 *
 */
int P2Cog::op_jse1()
{
    augmentS(IR.op.imm);
    if (FLAGS.f_SE1)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if SE2 event flag is set.
 *
 * EEEE 1011110 01I 000000101 SSSSSSSSS
 *
 * JSE2    {#}S
 *
 */
int P2Cog::op_jse2()
{
    augmentS(IR.op.imm);
    if (FLAGS.f_SE2)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if SE3 event flag is set.
 *
 * EEEE 1011110 01I 000000110 SSSSSSSSS
 *
 * JSE3    {#}S
 *
 */
int P2Cog::op_jse3()
{
    augmentS(IR.op.imm);
    if (FLAGS.f_SE3)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if SE4 event flag is set.
 *
 * EEEE 1011110 01I 000000111 SSSSSSSSS
 *
 * JSE4    {#}S
 *
 */
int P2Cog::op_jse4()
{
    augmentS(IR.op.imm);
    if (FLAGS.f_SE4)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if PAT event flag is set.
 *
 * EEEE 1011110 01I 000001000 SSSSSSSSS
 *
 * JPAT    {#}S
 *
 */
int P2Cog::op_jpat()
{
    augmentS(IR.op.imm);
    if (FLAGS.f_PAT)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if FBW event flag is set.
 *
 * EEEE 1011110 01I 000001001 SSSSSSSSS
 *
 * JFBW    {#}S
 *
 */
int P2Cog::op_jfbw()
{
    augmentS(IR.op.imm);
    if (FLAGS.f_FBW)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if XMT event flag is set.
 *
 * EEEE 1011110 01I 000001010 SSSSSSSSS
 *
 * JXMT    {#}S
 *
 */
int P2Cog::op_jxmt()
{
    augmentS(IR.op.imm);
    if (FLAGS.f_XMT)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if XFI event flag is set.
 *
 * EEEE 1011110 01I 000001011 SSSSSSSSS
 *
 * JXFI    {#}S
 *
 */
int P2Cog::op_jxfi()
{
    augmentS(IR.op.imm);
    if (FLAGS.f_XFI)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if XRO event flag is set.
 *
 * EEEE 1011110 01I 000001100 SSSSSSSSS
 *
 * JXRO    {#}S
 *
 */
int P2Cog::op_jxro()
{
    augmentS(IR.op.imm);
    if (FLAGS.f_XRO)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if XRL event flag is set.
 *
 * EEEE 1011110 01I 000001101 SSSSSSSSS
 *
 * JXRL    {#}S
 *
 */
int P2Cog::op_jxrl()
{
    augmentS(IR.op.imm);
    if (FLAGS.f_XRL)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if ATN event flag is set.
 *
 * EEEE 1011110 01I 000001110 SSSSSSSSS
 *
 * JATN    {#}S
 *
 */
int P2Cog::op_jatn()
{
    augmentS(IR.op.imm);
    if (FLAGS.f_ATN)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if QMT event flag is set.
 *
 * EEEE 1011110 01I 000001111 SSSSSSSSS
 *
 * JQMT    {#}S
 *
 */
int P2Cog::op_jqmt()
{
    augmentS(IR.op.imm);
    if (FLAGS.f_QMT)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if INT event flag is clear.
 *
 * EEEE 1011110 01I 000010000 SSSSSSSSS
 *
 * JNINT   {#}S
 *
 */
int P2Cog::op_jnint()
{
    augmentS(IR.op.imm);
    if (!FLAGS.f_INT)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if CT1 event flag is clear.
 *
 * EEEE 1011110 01I 000010001 SSSSSSSSS
 *
 * JNCT1   {#}S
 *
 */
int P2Cog::op_jnct1()
{
    augmentS(IR.op.imm);
    if (!FLAGS.f_CT1)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if CT2 event flag is clear.
 *
 * EEEE 1011110 01I 000010010 SSSSSSSSS
 *
 * JNCT2   {#}S
 *
 */
int P2Cog::op_jnct2()
{
    augmentS(IR.op.imm);
    if (!FLAGS.f_CT2)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if CT3 event flag is clear.
 *
 * EEEE 1011110 01I 000010011 SSSSSSSSS
 *
 * JNCT3   {#}S
 *
 */
int P2Cog::op_jnct3()
{
    augmentS(IR.op.imm);
    if (!FLAGS.f_CT3)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if SE1 event flag is clear.
 *
 * EEEE 1011110 01I 000010100 SSSSSSSSS
 *
 * JNSE1   {#}S
 *
 */
int P2Cog::op_jnse1()
{
    augmentS(IR.op.imm);
    if (!FLAGS.f_SE1)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if SE2 event flag is clear.
 *
 * EEEE 1011110 01I 000010101 SSSSSSSSS
 *
 * JNSE2   {#}S
 *
 */
int P2Cog::op_jnse2()
{
    augmentS(IR.op.imm);
    if (!FLAGS.f_SE2)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if SE3 event flag is clear.
 *
 * EEEE 1011110 01I 000010110 SSSSSSSSS
 *
 * JNSE3   {#}S
 *
 */
int P2Cog::op_jnse3()
{
    augmentS(IR.op.imm);
    if (!FLAGS.f_SE3)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if SE4 event flag is clear.
 *
 * EEEE 1011110 01I 000010111 SSSSSSSSS
 *
 * JNSE4   {#}S
 *
 */
int P2Cog::op_jnse4()
{
    augmentS(IR.op.imm);
    if (!FLAGS.f_SE4)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if PAT event flag is clear.
 *
 * EEEE 1011110 01I 000011000 SSSSSSSSS
 *
 * JNPAT   {#}S
 *
 */
int P2Cog::op_jnpat()
{
    augmentS(IR.op.imm);
    if (!FLAGS.f_PAT)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if FBW event flag is clear.
 *
 * EEEE 1011110 01I 000011001 SSSSSSSSS
 *
 * JNFBW   {#}S
 *
 */
int P2Cog::op_jnfbw()
{
    augmentS(IR.op.imm);
    if (!FLAGS.f_FBW)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if XMT event flag is clear.
 *
 * EEEE 1011110 01I 000011010 SSSSSSSSS
 *
 * JNXMT   {#}S
 *
 */
int P2Cog::op_jnxmt()
{
    augmentS(IR.op.imm);
    if (!FLAGS.f_XMT)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if XFI event flag is clear.
 *
 * EEEE 1011110 01I 000011011 SSSSSSSSS
 *
 * JNXFI   {#}S
 *
 */
int P2Cog::op_jnxfi()
{
    augmentS(IR.op.imm);
    if (!FLAGS.f_XFI)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if XRO event flag is clear.
 *
 * EEEE 1011110 01I 000011100 SSSSSSSSS
 *
 * JNXRO   {#}S
 *
 */
int P2Cog::op_jnxro()
{
    augmentS(IR.op.imm);
    if (!FLAGS.f_XRO)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if XRL event flag is clear.
 *
 * EEEE 1011110 01I 000011101 SSSSSSSSS
 *
 * JNXRL   {#}S
 *
 */
int P2Cog::op_jnxrl()
{
    augmentS(IR.op.imm);
    if (!FLAGS.f_XRL)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if ATN event flag is clear.
 *
 * EEEE 1011110 01I 000011110 SSSSSSSSS
 *
 * JNATN   {#}S
 *
 */
int P2Cog::op_jnatn()
{
    augmentS(IR.op.imm);
    if (!FLAGS.f_ATN)
        updatePC(S);
    return 2;
}

/**
 * @brief Jump to S** if QMT event flag is clear.
 *
 * EEEE 1011110 01I 000011111 SSSSSSSSS
 *
 * JNQMT   {#}S
 *
 */
int P2Cog::op_jnqmt()
{
    augmentS(IR.op.imm);
    if (!FLAGS.f_QMT)
        updatePC(S);
    return 2;
}

/**
 * @brief <empty>.
 *
 * EEEE 1011110 1LI DDDDDDDDD SSSSSSSSS
 *
 * <empty> {#}D,{#}S
 *
 */
int P2Cog::op_1011110_1()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief <empty>.
 *
 * EEEE 1011111 0LI DDDDDDDDD SSSSSSSSS
 *
 * <empty> {#}D,{#}S
 *
 */
int P2Cog::op_1011111_0()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Set pin pattern for PAT event.
 *
 * EEEE 1011111 BEI DDDDDDDDD SSSSSSSSS
 *
 * SETPAT  {#}D,{#}S
 *
 * C selects INA/INB, Z selects =/!=, D provides mask value, S provides match value.
 */
int P2Cog::op_setpat()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.imm);
    PAT.mode = IR.op.wc ? (IR.op.wz ? p2_PAT_PB_EQ : p2_PAT_PB_NE)
                        : (IR.op.wz ? p2_PAT_PA_EQ : p2_PAT_PA_NE);
    PAT.mask = D;
    PAT.match = S;
    return 2;
}

/**
 * @brief Write D to mode register of smart pin S[5:0], acknowledge smart pin.
 *
 * EEEE 1100000 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRPIN   {#}D,{#}S
 *
 */
int P2Cog::op_wrpin()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Acknowledge smart pin S[5:0].
 *
 * EEEE 1100000 01I 000000001 SSSSSSSSS
 *
 * AKPIN   {#}S
 *
 */
int P2Cog::op_akpin()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Write D to parameter "X" of smart pin S[5:0], acknowledge smart pin.
 *
 * EEEE 1100000 1LI DDDDDDDDD SSSSSSSSS
 *
 * WXPIN   {#}D,{#}S
 *
 */
int P2Cog::op_wxpin()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Write D to parameter "Y" of smart pin S[5:0], acknowledge smart pin.
 *
 * EEEE 1100001 0LI DDDDDDDDD SSSSSSSSS
 *
 * WYPIN   {#}D,{#}S
 *
 */
int P2Cog::op_wypin()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Write D to LUT address S[8:0].
 *
 * EEEE 1100001 1LI DDDDDDDDD SSSSSSSSS
 *
 * WRLUT   {#}D,{#}S
 *
 */
int P2Cog::op_wrlut()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    updateLUT(D, S);
    return 2;
}

/**
 * @brief Write byte in D[7:0] to hub address {#}S/PTRx.
 *
 * EEEE 1100010 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRBYTE  {#}D,{#}S/P
 *
 */
int P2Cog::op_wrbyte()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Write word in D[15:0] to hub address {#}S/PTRx.
 *
 * EEEE 1100010 1LI DDDDDDDDD SSSSSSSSS
 *
 * WRWORD  {#}D,{#}S/P
 *
 */
int P2Cog::op_wrword()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Write long in D[31:0] to hub address {#}S/PTRx.
 *
 * EEEE 1100011 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRLONG  {#}D,{#}S/P
 *
 * Prior SETQ/SETQ2 invokes cog/LUT block transfer.
 */
int P2Cog::op_wrlong()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Write long in D[31:0] to hub address PTRA++.
 *
 * EEEE 1100011 0L1 DDDDDDDDD 101100001
 *
 * PUSHA   {#}D
 *
 */
int P2Cog::op_pusha()
{
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Write long in D[31:0] to hub address PTRB++.
 *
 * EEEE 1100011 0L1 DDDDDDDDD 111100001
 *
 * PUSHB   {#}D
 *
 */
int P2Cog::op_pushb()
{
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Begin new fast hub read via FIFO.
 *
 * EEEE 1100011 1LI DDDDDDDDD SSSSSSSSS
 *
 * RDFAST  {#}D,{#}S
 *
 * D[31] = no wait, D[13:0] = block size in 64-byte units (0 = max), S[19:0] = block start address.
 */
int P2Cog::op_rdfast()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Begin new fast hub write via FIFO.
 *
 * EEEE 1100100 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRFAST  {#}D,{#}S
 *
 * D[31] = no wait, D[13:0] = block size in 64-byte units (0 = max), S[19:0] = block start address.
 */
int P2Cog::op_wrfast()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Set next block for when block wraps.
 *
 * EEEE 1100100 1LI DDDDDDDDD SSSSSSSSS
 *
 * FBLOCK  {#}D,{#}S
 *
 * D[13:0] = block size in 64-byte units (0 = max), S[19:0] = block start address.
 */
int P2Cog::op_fblock()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Issue streamer command immediately, zeroing phase.
 *
 * EEEE 1100101 0LI DDDDDDDDD SSSSSSSSS
 *
 * XINIT   {#}D,{#}S
 *
 */
int P2Cog::op_xinit()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Stop streamer immediately.
 *
 * EEEE 1100101 011 000000000 000000000
 *
 * XSTOP
 *
 */
int P2Cog::op_xstop()
{
    return 2;
}

/**
 * @brief Buffer new streamer command to be issued on final NCO rollover of current command, zeroing phase.
 *
 * EEEE 1100101 1LI DDDDDDDDD SSSSSSSSS
 *
 * XZERO   {#}D,{#}S
 *
 */
int P2Cog::op_xzero()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Buffer new streamer command to be issued on final NCO rollover of current command, continuing phase.
 *
 * EEEE 1100110 0LI DDDDDDDDD SSSSSSSSS
 *
 * XCONT   {#}D,{#}S
 *
 */
int P2Cog::op_xcont()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Execute next D[8:0] instructions S times.
 *
 * EEEE 1100110 1LI DDDDDDDDD SSSSSSSSS
 *
 * REP     {#}D,{#}S
 *
 * If S = 0, repeat infinitely.
 * If D[8:0] = 0, nothing repeats.
 */
int P2Cog::op_rep()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Start cog selected by D.
 *
 * EEEE 1100111 CLI DDDDDDDDD SSSSSSSSS
 *
 * COGINIT {#}D,{#}S       {WC}
 *
 * S[19:0] sets hub startup address and PTRB of cog.
 * Prior SETQ sets PTRA of cog.
 */
int P2Cog::op_coginit()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Begin CORDIC unsigned multiplication of D * S.
 *
 * EEEE 1101000 0LI DDDDDDDDD SSSSSSSSS
 *
 * QMUL    {#}D,{#}S
 *
 * GETQX/GETQY retrieves lower/upper product.
 */
int P2Cog::op_qmul()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Begin CORDIC unsigned division of {SETQ value or 32'b0, D} / S.
 *
 * EEEE 1101000 1LI DDDDDDDDD SSSSSSSSS
 *
 * QDIV    {#}D,{#}S
 *
 * GETQX/GETQY retrieves quotient/remainder.
 */
int P2Cog::op_qdiv()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Begin CORDIC unsigned division of {D, SETQ value or 32'b0} / S.
 *
 * EEEE 1101001 0LI DDDDDDDDD SSSSSSSSS
 *
 * QFRAC   {#}D,{#}S
 *
 * GETQX/GETQY retrieves quotient/remainder.
 */
int P2Cog::op_qfrac()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Begin CORDIC square root of {S, D}.
 *
 * EEEE 1101001 1LI DDDDDDDDD SSSSSSSSS
 *
 * QSQRT   {#}D,{#}S
 *
 * GETQX retrieves root.
 */
int P2Cog::op_qsqrt()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Begin CORDIC rotation of point (D, SETQ value or 32'b0) by angle S.
 *
 * EEEE 1101010 0LI DDDDDDDDD SSSSSSSSS
 *
 * QROTATE {#}D,{#}S
 *
 * GETQX/GETQY retrieves X/Y.
 */
int P2Cog::op_qrotate()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Begin CORDIC vectoring of point (D, S).
 *
 * EEEE 1101010 1LI DDDDDDDDD SSSSSSSSS
 *
 * QVECTOR {#}D,{#}S
 *
 * GETQX/GETQY retrieves length/angle.
 */
int P2Cog::op_qvector()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.wz);
    return 2;
}

/**
 * @brief Set hub configuration to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000000000
 *
 * HUBSET  {#}D
 *
 */
int P2Cog::op_hubset()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief If D is register and no WC, get cog ID (0 to 15) into D.
 *
 * EEEE 1101011 C0L DDDDDDDDD 000000001
 *
 * COGID   {#}D            {WC}
 *
 * If WC, check status of cog D[3:0], C = 1 if on.
 */
int P2Cog::op_cogid()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Stop cog D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000000011
 *
 * COGSTOP {#}D
 *
 */
int P2Cog::op_cogstop()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Request a LOCK.
 *
 * EEEE 1101011 C00 DDDDDDDDD 000000100
 *
 * LOCKNEW D               {WC}
 *
 * D will be written with the LOCK number (0 to 15).
 * C = 1 if no LOCK available.
 */
int P2Cog::op_locknew()
{
    return 2;
}

/**
 * @brief Return LOCK D[3:0] for reallocation.
 *
 * EEEE 1101011 00L DDDDDDDDD 000000101
 *
 * LOCKRET {#}D
 *
 */
int P2Cog::op_lockret()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Try to get LOCK D[3:0].
 *
 * EEEE 1101011 C0L DDDDDDDDD 000000110
 *
 * LOCKTRY {#}D            {WC}
 *
 * C = 1 if got LOCK.
 * LOCKREL releases LOCK.
 * LOCK is also released if owner cog stops or restarts.
 */
int P2Cog::op_locktry()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Release LOCK D[3:0].
 *
 * EEEE 1101011 C0L DDDDDDDDD 000000111
 *
 * LOCKREL {#}D            {WC}
 *
 * If D is a register and WC, get current/last cog id of LOCK owner into D and LOCK status into C.
 */
int P2Cog::op_lockrel()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Begin CORDIC number-to-logarithm conversion of D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000001110
 *
 * QLOG    {#}D
 *
 * GETQX retrieves log {5'whole_exponent, 27'fractional_exponent}.
 */
int P2Cog::op_qlog()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Begin CORDIC logarithm-to-number conversion of D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000001111
 *
 * QEXP    {#}D
 *
 * GETQX retrieves number.
 */
int P2Cog::op_qexp()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Read zero-extended byte from FIFO into D. Used after RDFAST.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000010000
 *
 * RFBYTE  D        {WC/WZ/WCZ}
 *
 * C = MSB of byte.
 * Z = (result == 0).
 */
int P2Cog::op_rfbyte()
{
    return 2;
}

/**
 * @brief Read zero-extended word from FIFO into D. Used after RDFAST.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000010001
 *
 * RFWORD  D        {WC/WZ/WCZ}
 *
 * C = MSB of word.
 * Z = (result == 0).
 */
int P2Cog::op_rfword()
{
    return 2;
}

/**
 * @brief Read long from FIFO into D. Used after RDFAST.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000010010
 *
 * RFLONG  D        {WC/WZ/WCZ}
 *
 * C = MSB of long.
 * Z = (result == 0).
 */
int P2Cog::op_rflong()
{
    return 2;
}

/**
 * @brief Read zero-extended 1 … 4-byte value from FIFO into D. Used after RDFAST.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000010011
 *
 * RFVAR   D        {WC/WZ/WCZ}
 *
 * C = 0.
 * Z = (result == 0).
 */
int P2Cog::op_rfvar()
{
    return 2;
}

/**
 * @brief Read sign-extended 1 … 4-byte value from FIFO into D. Used after RDFAST.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000010100
 *
 * RFVARS  D        {WC/WZ/WCZ}
 *
 * C = MSB of value.
 * Z = (result == 0).
 */
int P2Cog::op_rfvars()
{
    return 2;
}

/**
 * @brief Write byte in D[7:0] into FIFO. Used after WRFAST.
 *
 * EEEE 1101011 00L DDDDDDDDD 000010101
 *
 * WFBYTE  {#}D
 *
 */
int P2Cog::op_wfbyte()
{
    return 2;
}

/**
 * @brief Write word in D[15:0] into FIFO. Used after WRFAST.
 *
 * EEEE 1101011 00L DDDDDDDDD 000010110
 *
 * WFWORD  {#}D
 *
 */
int P2Cog::op_wfword()
{
    return 2;
}

/**
 * @brief Write long in D[31:0] into FIFO. Used after WRFAST.
 *
 * EEEE 1101011 00L DDDDDDDDD 000010111
 *
 * WFLONG  {#}D
 *
 */
int P2Cog::op_wflong()
{
    return 2;
}

/**
 * @brief Retrieve CORDIC result X into D.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000011000
 *
 * GETQX   D        {WC/WZ/WCZ}
 *
 * Waits, in case result:
        op_not ready.();
        return;
 * C = X[31].
 * Z = (result == 0).
 */
int P2Cog::op_getqx()
{
    return 2;
}

/**
 * @brief Retrieve CORDIC result Y into D.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000011001
 *
 * GETQY   D        {WC/WZ/WCZ}
 *
 * Waits, in case result:
        op_not ready.();
        return;
 * C = Y[31].
 * Z = (result == 0).
 */
int P2Cog::op_getqy()
{
    return 2;
}

/**
 * @brief Get CT into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 000011010
 *
 * GETCT   D
 *
 * CT is the free-running 32-bit system counter that increments on every clock.
 */
int P2Cog::op_getct()
{
    return 2;
}

/**
 * @brief Get RND into D/C/Z.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000011011
 *
 * GETRND  D        {WC/WZ/WCZ}
 *
 * RND is the PRNG that updates on every clock.
 * D = RND[31:0], C = RND[31], Z = RND[30], unique per cog.
 */
int P2Cog::op_getrnd()
{
    P2LONG result = HUB->random(2*ID);
    updateC((result >> 31) & 1);
    updateZ((result >> 30) & 1);
    updateD(result);
    return 2;
}

/**
 * @brief Get RND into C/Z.
 *
 * EEEE 1101011 CZ1 000000000 000011011
 *
 * GETRND            WC/WZ/WCZ
 *
 * C = RND[31], Z = RND[30], unique per cog.
 */
int P2Cog::op_getrnd_cz()
{
    P2LONG result = HUB->random(2*ID);
    updateC((result >> 31) & 1);
    updateZ((result >> 30) & 1);
    return 2;
}

/**
 * @brief DAC3 = D[31:24], DAC2 = D[23:16], DAC1 = D[15:8], DAC0 = D[7:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000011100
 *
 * SETDACS {#}D
 *
 */
int P2Cog::op_setdacs()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Set streamer NCO frequency to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000011101
 *
 * SETXFRQ {#}D
 *
 */
int P2Cog::op_setxfrq()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Get the streamer's Goertzel X accumulator into D and the Y accumulator into the next instruction's S, clear accumulators.
 *
 * EEEE 1101011 000 DDDDDDDDD 000011110
 *
 * GETXACC D
 *
 */
int P2Cog::op_getxacc()
{
    return 2;
}

/**
 * @brief Wait 2 + D clocks if no WC/WZ/WCZ.
 *
 * EEEE 1101011 00L DDDDDDDDD 000011111
 *
 * WAITX   {#}D     {WC/WZ/WCZ}
 *
 * If WC/WZ/WCZ, wait 2 + (D & RND) clocks.
 * C/Z = 0.
 */
int P2Cog::op_waitx()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Set SE1 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100000
 *
 * SETSE1  {#}D
 *
 */
int P2Cog::op_setse1()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Set SE2 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100001
 *
 * SETSE2  {#}D
 *
 */
int P2Cog::op_setse2()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Set SE3 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100010
 *
 * SETSE3  {#}D
 *
 */
int P2Cog::op_setse3()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Set SE4 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100011
 *
 * SETSE4  {#}D
 *
 */
int P2Cog::op_setse4()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Get INT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000000 000100100
 *
 * POLLINT          {WC/WZ/WCZ}
 *
 */
int P2Cog::op_pollint()
{
    return 2;
}

/**
 * @brief Get CT1 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000001 000100100
 *
 * POLLCT1          {WC/WZ/WCZ}
 *
 */
int P2Cog::op_pollct1()
{
    return 2;
}

/**
 * @brief Get CT2 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000010 000100100
 *
 * POLLCT2          {WC/WZ/WCZ}
 *
 */
int P2Cog::op_pollct2()
{
    return 2;
}

/**
 * @brief Get CT3 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000011 000100100
 *
 * POLLCT3          {WC/WZ/WCZ}
 *
 */
int P2Cog::op_pollct3()
{
    return 2;
}

/**
 * @brief Get SE1 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000100 000100100
 *
 * POLLSE1          {WC/WZ/WCZ}
 *
 */
int P2Cog::op_pollse1()
{
    return 2;
}

/**
 * @brief Get SE2 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000101 000100100
 *
 * POLLSE2          {WC/WZ/WCZ}
 *
 */
int P2Cog::op_pollse2()
{
    return 2;
}

/**
 * @brief Get SE3 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000110 000100100
 *
 * POLLSE3          {WC/WZ/WCZ}
 *
 */
int P2Cog::op_pollse3()
{
    return 2;
}

/**
 * @brief Get SE4 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000111 000100100
 *
 * POLLSE4          {WC/WZ/WCZ}
 *
 */
int P2Cog::op_pollse4()
{
    return 2;
}

/**
 * @brief Get PAT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001000 000100100
 *
 * POLLPAT          {WC/WZ/WCZ}
 *
 */
int P2Cog::op_pollpat()
{
    return 2;
}

/**
 * @brief Get FBW event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001001 000100100
 *
 * POLLFBW          {WC/WZ/WCZ}
 *
 */
int P2Cog::op_pollfbw()
{
    return 2;
}

/**
 * @brief Get XMT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001010 000100100
 *
 * POLLXMT          {WC/WZ/WCZ}
 *
 */
int P2Cog::op_pollxmt()
{
    return 2;
}

/**
 * @brief Get XFI event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001011 000100100
 *
 * POLLXFI          {WC/WZ/WCZ}
 *
 */
int P2Cog::op_pollxfi()
{
    return 2;
}

/**
 * @brief Get XRO event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001100 000100100
 *
 * POLLXRO          {WC/WZ/WCZ}
 *
 */
int P2Cog::op_pollxro()
{
    return 2;
}

/**
 * @brief Get XRL event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001101 000100100
 *
 * POLLXRL          {WC/WZ/WCZ}
 *
 */
int P2Cog::op_pollxrl()
{
    return 2;
}

/**
 * @brief Get ATN event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001110 000100100
 *
 * POLLATN          {WC/WZ/WCZ}
 *
 */
int P2Cog::op_pollatn()
{
    return 2;
}

/**
 * @brief Get QMT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001111 000100100
 *
 * POLLQMT          {WC/WZ/WCZ}
 *
 */
int P2Cog::op_pollqmt()
{
    return 2;
}

/**
 * @brief Wait for INT event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000010000 000100100
 *
 * WAITINT          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 */
int P2Cog::op_waitint()
{
    return 2;
}

/**
 * @brief Wait for CT1 event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000010001 000100100
 *
 * WAITCT1          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 */
int P2Cog::op_waitct1()
{
    return 2;
}

/**
 * @brief Wait for CT2 event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000010010 000100100
 *
 * WAITCT2          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 */
int P2Cog::op_waitct2()
{
    return 2;
}

/**
 * @brief Wait for CT3 event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000010011 000100100
 *
 * WAITCT3          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 */
int P2Cog::op_waitct3()
{
    return 2;
}

/**
 * @brief Wait for SE1 event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000010100 000100100
 *
 * WAITSE1          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 */
int P2Cog::op_waitse1()
{
    return 2;
}

/**
 * @brief Wait for SE2 event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000010101 000100100
 *
 * WAITSE2          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 */
int P2Cog::op_waitse2()
{
    return 2;
}

/**
 * @brief Wait for SE3 event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000010110 000100100
 *
 * WAITSE3          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 */
int P2Cog::op_waitse3()
{
    return 2;
}

/**
 * @brief Wait for SE4 event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000010111 000100100
 *
 * WAITSE4          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 */
int P2Cog::op_waitse4()
{
    return 2;
}

/**
 * @brief Wait for PAT event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000011000 000100100
 *
 * WAITPAT          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 */
int P2Cog::op_waitpat()
{
    return 2;
}

/**
 * @brief Wait for FBW event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000011001 000100100
 *
 * WAITFBW          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 */
int P2Cog::op_waitfbw()
{
    return 2;
}

/**
 * @brief Wait for XMT event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000011010 000100100
 *
 * WAITXMT          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 */
int P2Cog::op_waitxmt()
{
    return 2;
}

/**
 * @brief Wait for XFI event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000011011 000100100
 *
 * WAITXFI          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 */
int P2Cog::op_waitxfi()
{
    return 2;
}

/**
 * @brief Wait for XRO event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000011100 000100100
 *
 * WAITXRO          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 */
int P2Cog::op_waitxro()
{
    return 2;
}

/**
 * @brief Wait for XRL event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000011101 000100100
 *
 * WAITXRL          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 */
int P2Cog::op_waitxrl()
{
    return 2;
}

/**
 * @brief Wait for ATN event flag, then clear it.
 *
 * EEEE 1101011 CZ0 000011110 000100100
 *
 * WAITATN          {WC/WZ/WCZ}
 *
 * Prior SETQ sets optional CT timeout value.
 * C/Z = timeout.
 */
int P2Cog::op_waitatn()
{
    return 2;
}

/**
 * @brief Allow interrupts (default).
 *
 * EEEE 1101011 000 000100000 000100100
 *
 * ALLOWI
 *
 */
int P2Cog::op_allowi()
{
    return 2;
}

/**
 * @brief Stall Interrupts.
 *
 * EEEE 1101011 000 000100001 000100100
 *
 * STALLI
 *
 */
int P2Cog::op_stalli()
{
    return 2;
}

/**
 * @brief Trigger INT1, regardless of STALLI mode.
 *
 * EEEE 1101011 000 000100010 000100100
 *
 * TRGINT1
 *
 */
int P2Cog::op_trgint1()
{
    return 2;
}

/**
 * @brief Trigger INT2, regardless of STALLI mode.
 *
 * EEEE 1101011 000 000100011 000100100
 *
 * TRGINT2
 *
 */
int P2Cog::op_trgint2()
{
    return 2;
}

/**
 * @brief Trigger INT3, regardless of STALLI mode.
 *
 * EEEE 1101011 000 000100100 000100100
 *
 * TRGINT3
 *
 */
int P2Cog::op_trgint3()
{
    return 2;
}

/**
 * @brief Cancel INT1.
 *
 * EEEE 1101011 000 000100101 000100100
 *
 * NIXINT1
 *
 */
int P2Cog::op_nixint1()
{
    return 2;
}

/**
 * @brief Cancel INT2.
 *
 * EEEE 1101011 000 000100110 000100100
 *
 * NIXINT2
 *
 */
int P2Cog::op_nixint2()
{
    return 2;
}

/**
 * @brief Cancel INT3.
 *
 * EEEE 1101011 000 000100111 000100100
 *
 * NIXINT3
 *
 */
int P2Cog::op_nixint3()
{
    return 2;
}

/**
 * @brief Set INT1 source to D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100101
 *
 * SETINT1 {#}D
 *
 */
int P2Cog::op_setint1()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Set INT2 source to D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100110
 *
 * SETINT2 {#}D
 *
 */
int P2Cog::op_setint2()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Set INT3 source to D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100111
 *
 * SETINT3 {#}D
 *
 */
int P2Cog::op_setint3()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Set Q to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000101000
 *
 * SETQ    {#}D
 *
 * Use before RDLONG/WRLONG/WMLONG to set block transfer.
 * Also used before MUXQ/COGINIT/QDIV/QFRAC/QROTATE/WAITxxx.
 */
int P2Cog::op_setq()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Set Q to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000101001
 *
 * SETQ2   {#}D
 *
 * Use before RDLONG/WRLONG/WMLONG to set LUT block transfer.
 */
int P2Cog::op_setq2()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Push D onto stack.
 *
 * EEEE 1101011 00L DDDDDDDDD 000101010
 *
 * PUSH    {#}D
 *
 */
int P2Cog::op_push()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Pop stack (K).
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101011
 *
 * POP     D        {WC/WZ/WCZ}
 *
 * C = K[31], Z = K[30], D = K.
 */
int P2Cog::op_pop()
{
    return 2;
}

/**
 * @brief Jump to D.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101100
 *
 * JMP     D        {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 */
int P2Cog::op_jmp()
{
    updateC((D >> 31) & 1);
    updateZ((D >> 30) & 1);
    updatePC(D);
    return 2;
}

/**
 * @brief Call to D by pushing {C, Z, 10'b0, PC[19:0]} onto stack.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101101
 *
 * CALL    D        {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 */
int P2Cog::op_call()
{
    // FIXME: which stack?
    updateC((D >> 31) & 1);
    updateZ((D >> 30) & 1);
    updatePC(D);
    return 2;
}

/**
 * @brief Return by popping stack (K).
 *
 * EEEE 1101011 CZ1 000000000 000101101
 *
 * RET              {WC/WZ/WCZ}
 *
 * C = K[31], Z = K[30], PC = K[19:0].
 */
int P2Cog::op_ret()
{
    // FIXME: which stack?
    P2LONG K = HUB->rd_LONG(LUT.REG.PB);
    updateC((K >> 31) & 1);
    updateZ((K >> 30) & 1);
    updatePC(K);
    return 2;
}

/**
 * @brief Call to D by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRA++.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101110
 *
 * CALLA   D        {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 */
int P2Cog::op_calla()
{
    return 2;
}

/**
 * @brief Return by reading hub long (L) at --PTRA.
 *
 * EEEE 1101011 CZ1 000000000 000101110
 *
 * RETA             {WC/WZ/WCZ}
 *
 * C = L[31], Z = L[30], PC = L[19:0].
 */
int P2Cog::op_reta()
{
    return 2;
}

/**
 * @brief Call to D by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRB++.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101111
 *
 * CALLB   D        {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 */
int P2Cog::op_callb()
{
    return 2;
}

/**
 * @brief Return by reading hub long (L) at --PTRB.
 *
 * EEEE 1101011 CZ1 000000000 000101111
 *
 * RETB             {WC/WZ/WCZ}
 *
 * C = L[31], Z = L[30], PC = L[19:0].
 */
int P2Cog::op_retb()
{
    return 2;
}

/**
 * @brief Jump ahead/back by D instructions.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110000
 *
 * JMPREL  {#}D
 *
 * For cogex, PC += D[19:0].
 * For hubex, PC += D[17:0] << 2.
 */
int P2Cog::op_jmprel()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Skip instructions per D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110001
 *
 * SKIP    {#}D
 *
 * Subsequent instructions 0.
 * 31 get cancelled for each '1' bit in D[0].
 * D[31].
 */
int P2Cog::op_skip()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Skip cog/LUT instructions fast per D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110010
 *
 * SKIPF   {#}D
 *
 * Like SKIP, but instead of cancelling instructions, the PC leaps over them.
 */
int P2Cog::op_skipf()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Jump to D[9:0] in cog/LUT and set SKIPF pattern to D[31:10].
 *
 * EEEE 1101011 00L DDDDDDDDD 000110011
 *
 * EXECF   {#}D
 *
 * PC = {10'b0, D[9:0]}.
 */
int P2Cog::op_execf()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Get current FIFO hub pointer into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 000110100
 *
 * GETPTR  D
 *
 */
int P2Cog::op_getptr()
{
    return 2;
}

/**
 * @brief Get breakpoint status into D according to WC/WZ/WCZ.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000110101
 *
 * GETBRK  D          WC/WZ/WCZ
 *
 * C = 0.
 * Z = 0.
 */
int P2Cog::op_getbrk()
{
    return 2;
}

/**
 * @brief If in debug ISR, trigger asynchronous breakpoint in cog D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000110101
 *
 * COGBRK  {#}D
 *
 * Cog D[3:0] must have asynchronous breakpoint enabled.
 */
int P2Cog::op_cogbrk()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief If in debug ISR, set next break condition to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110110
 *
 * BRK     {#}D
 *
 * Else, trigger break if enabled, conditionally write break code to D[7:0].
 */
int P2Cog::op_brk()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief If D[0] = 1 then enable LUT sharing, where LUT writes within the adjacent odd/even companion cog are copied to this LUT.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110111
 *
 * SETLUTS {#}D
 *
 */
int P2Cog::op_setluts()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Set the colorspace converter "CY" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111000
 *
 * SETCY   {#}D
 *
 */
int P2Cog::op_setcy()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Set the colorspace converter "CI" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111001
 *
 * SETCI   {#}D
 *
 */
int P2Cog::op_setci()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Set the colorspace converter "CQ" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111010
 *
 * SETCQ   {#}D
 *
 */
int P2Cog::op_setcq()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Set the colorspace converter "CFRQ" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111011
 *
 * SETCFRQ {#}D
 *
 */
int P2Cog::op_setcfrq()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Set the colorspace converter "CMOD" parameter to D[6:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111100
 *
 * SETCMOD {#}D
 *
 */
int P2Cog::op_setcmod()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Set BLNPIX/MIXPIX blend factor to D[7:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111101
 *
 * SETPIV  {#}D
 *
 */
int P2Cog::op_setpiv()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Set MIXPIX mode to D[5:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111110
 *
 * SETPIX  {#}D
 *
 */
int P2Cog::op_setpix()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Strobe "attention" of all cogs whose corresponging bits are high in D[15:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111111
 *
 * COGATN  {#}D
 *
 */
int P2Cog::op_cogatn()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Test  IN bit of pin D[5:0], write to C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000000
 *
 * TESTP   {#}D           WC/WZ
 *
 * C/Z =          IN[D[5:0]].
 */
int P2Cog::op_testp_w()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Test !IN bit of pin D[5:0], write to C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000001
 *
 * TESTPN  {#}D           WC/WZ
 *
 * C/Z =         !IN[D[5:0]].
 */
int P2Cog::op_testpn_w()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Test  IN bit of pin D[5:0], AND into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000010
 *
 * TESTP   {#}D       ANDC/ANDZ
 *
 * C/Z = C/Z AND  IN[D[5:0]].
 */
int P2Cog::op_testp_and()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Test !IN bit of pin D[5:0], AND into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000011
 *
 * TESTPN  {#}D       ANDC/ANDZ
 *
 * C/Z = C/Z AND !IN[D[5:0]].
 */
int P2Cog::op_testpn_and()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Test  IN bit of pin D[5:0], OR  into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000100
 *
 * TESTP   {#}D         ORC/ORZ
 *
 * C/Z = C/Z OR   IN[D[5:0]].
 */
int P2Cog::op_testp_or()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Test !IN bit of pin D[5:0], OR  into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000101
 *
 * TESTPN  {#}D         ORC/ORZ
 *
 * C/Z = C/Z OR  !IN[D[5:0]].
 */
int P2Cog::op_testpn_or()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Test  IN bit of pin D[5:0], XOR into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000110
 *
 * TESTP   {#}D       XORC/XORZ
 *
 * C/Z = C/Z XOR  IN[D[5:0]].
 */
int P2Cog::op_testp_xor()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Test !IN bit of pin D[5:0], XOR into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000111
 *
 * TESTPN  {#}D       XORC/XORZ
 *
 * C/Z = C/Z XOR !IN[D[5:0]].
 */
int P2Cog::op_testpn_xor()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief DIR bit of pin D[5:0] = 0.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000000
 *
 * DIRL    {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 */
int P2Cog::op_dirl()
{
    augmentD(IR.op.imm);
    const P2LONG result = 0;
    updateC(result);
    updateZ(result);
    updateDIR(D, result);
    return 2;
}

/**
 * @brief DIR bit of pin D[5:0] = 1.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000001
 *
 * DIRH    {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 */
int P2Cog::op_dirh()
{
    augmentD(IR.op.imm);
    const P2LONG result = 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, result);
    return 2;
}

/**
 * @brief DIR bit of pin D[5:0] = C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000010
 *
 * DIRC    {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 */
int P2Cog::op_dirc()
{
    augmentD(IR.op.imm);
    const P2LONG result = C;
    updateC(result);
    updateZ(result);
    updateDIR(D, result);
    return 2;
}

/**
 * @brief DIR bit of pin D[5:0] = !C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000011
 *
 * DIRNC   {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 */
int P2Cog::op_dirnc()
{
    augmentD(IR.op.imm);
    const P2LONG result = C ^ 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, result);
    return 2;
}

/**
 * @brief DIR bit of pin D[5:0] = Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000100
 *
 * DIRZ    {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 */
int P2Cog::op_dirz()
{
    augmentD(IR.op.imm);
    const P2LONG result = Z;
    updateC(result);
    updateZ(result);
    updateDIR(D, result);
    return 2;
}

/**
 * @brief DIR bit of pin D[5:0] = !Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000101
 *
 * DIRNZ   {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 */
int P2Cog::op_dirnz()
{
    augmentD(IR.op.imm);
    const P2LONG result = Z ^ 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, result);
    return 2;
}

/**
 * @brief DIR bit of pin D[5:0] = RND.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000110
 *
 * DIRRND  {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 */
int P2Cog::op_dirrnd()
{
    augmentD(IR.op.imm);
    const P2LONG result = HUB->random(2*ID) & 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, result);
    return 2;
}

/**
 * @brief DIR bit of pin D[5:0] = !bit.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000111
 *
 * DIRNOT  {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 */
int P2Cog::op_dirnot()
{
    augmentD(IR.op.imm);
    const P2LONG result = HUB->rd_DIR(D) ^ 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, result);
    return 2;
}

/**
 * @brief OUT bit of pin D[5:0] = 0.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001000
 *
 * OUTL    {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 */
int P2Cog::op_outl()
{
    augmentD(IR.op.imm);
    const P2LONG result = 0;
    updateC(result);
    updateZ(result);
    updateOUT(D, result);
    return 2;
}

/**
 * @brief OUT bit of pin D[5:0] = 1.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001001
 *
 * OUTH    {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 */
int P2Cog::op_outh()
{
    augmentD(IR.op.imm);
    const P2LONG result = 1;
    updateC(result);
    updateZ(result);
    updateOUT(D, result);
    return 2;
}

/**
 * @brief OUT bit of pin D[5:0] = C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001010
 *
 * OUTC    {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 */
int P2Cog::op_outc()
{
    augmentD(IR.op.imm);
    const P2LONG result = C;
    updateC(result);
    updateZ(result);
    updateOUT(D, result);
    return 2;
}

/**
 * @brief OUT bit of pin D[5:0] = !C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001011
 *
 * OUTNC   {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 */
int P2Cog::op_outnc()
{
    augmentD(IR.op.imm);
    const P2LONG result = C ^ 1;
    updateC(result);
    updateZ(result);
    updateOUT(D, result);
    return 2;
}

/**
 * @brief OUT bit of pin D[5:0] = Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001100
 *
 * OUTZ    {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 */
int P2Cog::op_outz()
{
    augmentD(IR.op.imm);
    const P2LONG result = Z;
    updateC(result);
    updateZ(result);
    updateOUT(D, result);
    return 2;
}

/**
 * @brief OUT bit of pin D[5:0] = !Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001101
 *
 * OUTNZ   {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 */
int P2Cog::op_outnz()
{
    augmentD(IR.op.imm);
    const P2LONG result = Z ^ 1;
    updateC(result);
    updateZ(result);
    updateOUT(D, result);
    return 2;
}

/**
 * @brief OUT bit of pin D[5:0] = RND.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001110
 *
 * OUTRND  {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 */
int P2Cog::op_outrnd()
{
    augmentD(IR.op.imm);
    const P2LONG result = HUB->random(2*ID) & 1;
    updateC(result);
    updateZ(result);
    updateOUT(D, result);
    return 2;
}

/**
 * @brief OUT bit of pin D[5:0] = !bit.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001111
 *
 * OUTNOT  {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 */
int P2Cog::op_outnot()
{
    augmentD(IR.op.imm);
    const P2LONG result = HUB->rd_OUT(D) ^ 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, result);
    return 2;
}

/**
 * @brief OUT bit of pin D[5:0] = 0.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001010000
 *
 * FLTL    {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 */
int P2Cog::op_fltl()
{
    augmentD(IR.op.imm);
    const P2LONG result = 0;
    updateC(result);
    updateZ(result);
    updateDIR(D, 0);
    updateOUT(D, result);
    return 2;
}

/**
 * @brief OUT bit of pin D[5:0] = 1.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001010001
 *
 * FLTH    {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 */
int P2Cog::op_flth()
{
    augmentD(IR.op.imm);
    const P2LONG result = 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, 0);
    updateOUT(D, result);
    return 2;
}

/**
 * @brief OUT bit of pin D[5:0] = C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001010010
 *
 * FLTC    {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 */
int P2Cog::op_fltc()
{
    augmentD(IR.op.imm);
    const P2LONG result = C;
    updateC(result);
    updateZ(result);
    updateDIR(D, 0);
    updateOUT(D, result);
    return 2;
}

/**
 * @brief OUT bit of pin D[5:0] = !C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001010011
 *
 * FLTNC   {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 */
int P2Cog::op_fltnc()
{
    augmentD(IR.op.imm);
    const P2LONG result = C ^ 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, 0);
    updateOUT(D, result);
    return 2;
}

/**
 * @brief OUT bit of pin D[5:0] = Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001010100
 *
 * FLTZ    {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 */
int P2Cog::op_fltz()
{
    augmentD(IR.op.imm);
    const P2LONG result = Z;
    updateC(result);
    updateZ(result);
    updateDIR(D, 0);
    updateOUT(D, result);
    return 2;
}

/**
 * @brief OUT bit of pin D[5:0] = !Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001010101
 *
 * FLTNZ   {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 */
int P2Cog::op_fltnz()
{
    augmentD(IR.op.imm);
    const P2LONG result = Z ^ 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, 0);
    updateOUT(D, result);
    return 2;
}

/**
 * @brief OUT bit of pin D[5:0] = RND.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001010110
 *
 * FLTRND  {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 */
int P2Cog::op_fltrnd()
{
    augmentD(IR.op.imm);
    const P2LONG result = HUB->random(2*ID) & 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, 0);
    updateOUT(D, result);
    return 2;
}

/**
 * @brief OUT bit of pin D[5:0] = !bit.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001010111
 *
 * FLTNOT  {#}D           {WCZ}
 *
 * DIR bit = 0.
 * C,Z = OUT bit.
 */
int P2Cog::op_fltnot()
{
    augmentD(IR.op.imm);
    const P2LONG result = HUB->rd_OUT(D);
    updateC(result);
    updateZ(result);
    updateDIR(D, 0);
    updateOUT(D, result);
    return 2;
}

/**
 * @brief OUT bit of pin D[5:0] = 0.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001011000
 *
 * DRVL    {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 */
int P2Cog::op_drvl()
{
    augmentD(IR.op.imm);
    const P2LONG result = 0;
    updateC(result);
    updateZ(result);
    updateDIR(D, 1);
    updateOUT(D, result);
    return 2;
}

/**
 * @brief OUT bit of pin D[5:0] = 1.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001011001
 *
 * DRVH    {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 */
int P2Cog::op_drvh()
{
    augmentD(IR.op.imm);
    const P2LONG result = 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, 1);
    updateOUT(D, result);
    return 2;
}

/**
 * @brief OUT bit of pin D[5:0] = C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001011010
 *
 * DRVC    {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 */
int P2Cog::op_drvc()
{
    augmentD(IR.op.imm);
    const P2LONG result = C;
    updateC(result);
    updateZ(result);
    updateDIR(D, 1);
    updateOUT(D, result);
    return 2;
}

/**
 * @brief OUT bit of pin D[5:0] = !C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001011011
 *
 * DRVNC   {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 */
int P2Cog::op_drvnc()
{
    augmentD(IR.op.imm);
    const P2LONG result = C ^ 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, 1);
    updateOUT(D, result);
    return 2;
}

/**
 * @brief OUT bit of pin D[5:0] = Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001011100
 *
 * DRVZ    {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 */
int P2Cog::op_drvz()
{
    augmentD(IR.op.imm);
    const P2LONG result = Z;
    updateC(result);
    updateZ(result);
    updateDIR(D, 1);
    updateOUT(D, result);
    return 2;
}

/**
 * @brief OUT bit of pin D[5:0] = !Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001011101
 *
 * DRVNZ   {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 */
int P2Cog::op_drvnz()
{
    augmentD(IR.op.imm);
    const P2LONG result = Z ^ 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, 1);
    updateOUT(D, result);
    return 2;
}

/**
 * @brief OUT bit of pin D[5:0] = RND.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001011110
 *
 * DRVRND  {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 */
int P2Cog::op_drvrnd()
{
    augmentD(IR.op.imm);
    const P2LONG result = HUB->random(2*ID) & 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, 1);
    updateOUT(D, result);
    return 2;
}

/**
 * @brief OUT bit of pin D[5:0] = !bit.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001011111
 *
 * DRVNOT  {#}D           {WCZ}
 *
 * DIR bit = 1.
 * C,Z = OUT bit.
 */
int P2Cog::op_drvnot()
{
    augmentD(IR.op.imm);
    const P2LONG result = HUB->rd_OUT(D) ^ 1;
    updateC(result);
    updateZ(result);
    updateDIR(D, 1);
    updateOUT(D, result);
    return 2;
}

/**
 * @brief Split every 4th bit of S into bytes of D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100000
 *
 * SPLITB  D
 *
 * D = {S[31], S[27], S[23], S[19], … S[12], S[8], S[4], S[0]}.
 */
int P2Cog::op_splitb()
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
 *
 * EEEE 1101011 000 DDDDDDDDD 001100001
 *
 * MERGEB  D
 *
 * D = {S[31], S[23], S[15], S[7], … S[24], S[16], S[8], S[0]}.
 */
int P2Cog::op_mergeb()
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
 *
 * EEEE 1101011 000 DDDDDDDDD 001100010
 *
 * SPLITW  D
 *
 * D = {S[31], S[29], S[27], S[25], … S[6], S[4], S[2], S[0]}.
 */
int P2Cog::op_splitw()
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
 *
 * EEEE 1101011 000 DDDDDDDDD 001100011
 *
 * MERGEW  D
 *
 * D = {S[31], S[15], S[30], S[14], … S[17], S[1], S[16], S[0]}.
 */
int P2Cog::op_mergew()
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
 *
 * EEEE 1101011 000 DDDDDDDDD 001100100
 *
 * SEUSSF  D
 *
 * Returns to original value on 32nd iteration.
 * Forward pattern.
 */
int P2Cog::op_seussf()
{
    const P2LONG result = seuss(S, true);
    updateD(result);
    return 2;
}

/**
 * @brief Relocate and periodically invert bits from S into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100101
 *
 * SEUSSR  D
 *
 * Returns to original value on 32nd iteration.
 * Reverse pattern.
 */
int P2Cog::op_seussr()
{
    const P2LONG result = seuss(S, false);
    updateD(result);
    return 2;
}

/**
 * @brief Squeeze 8:8:8 RGB value in S[31:8] into 5:6:5 value in D[15:0].
 *
 * EEEE 1101011 000 DDDDDDDDD 001100110
 *
 * RGBSQZ  D
 *
 * D = {15'b0, S[31:27], S[23:18], S[15:11]}.
 */
int P2Cog::op_rgbsqz()
{
    const P2LONG result =
            (((S >> 27) & 0x1f) << 11) |
            (((S >> 18) & 0x3f) <<  5) |
            (((S >> 11) & 0x1f) <<  0);
    updateD(result);
    return 2;
}

/**
 * @brief Expand 5:6:5 RGB value in S[15:0] into 8:8:8 value in D[31:8].
 *
 * EEEE 1101011 000 DDDDDDDDD 001100111
 *
 * RGBEXP  D
 *
 * D = {S[15:11,15:13], S[10:5,10:9], S[4:0,4:2], 8'b0}.
 */
int P2Cog::op_rgbexp()
{
    const P2LONG result =
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
 *
 * EEEE 1101011 000 DDDDDDDDD 001101000
 *
 * XORO32  D
 *
 */
int P2Cog::op_xoro32()
{
    return 2;
}

/**
 * @brief Reverse D bits.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101001
 *
 * REV     D
 *
 * D = D[0:31].
 */
int P2Cog::op_rev()
{
    P2LONG result = reverse(D);
    updateD(result);
    return 2;
}

/**
 * @brief Rotate C,Z right through D.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 001101010
 *
 * RCZR    D        {WC/WZ/WCZ}
 *
 * D = {C, Z, D[31:2]}.
 * C = D[1],  Z = D[0].
 */
int P2Cog::op_rczr()
{
    const P2LONG result = (C << 31) | (Z << 30) | (D >> 2);
    updateC((D >> 1) & 1);
    updateZ((D >> 0) & 1);
    updateD(result);
    return 2;
}

/**
 * @brief Rotate C,Z left through D.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 001101011
 *
 * RCZL    D        {WC/WZ/WCZ}
 *
 * D = {D[29:0], C, Z}.
 * C = D[31], Z = D[30].
 */
int P2Cog::op_rczl()
{
    const P2LONG result = (D << 2) | (C << 1) | (Z << 0);
    updateC((D >> 31) & 1);
    updateZ((D >> 30) & 1);
    updateD(result);
    return 2;
}

/**
 * @brief Write 0 or 1 to D, according to  C.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101100
 *
 * WRC     D
 *
 * D = {31'b0,  C).
 */
int P2Cog::op_wrc()
{
    const P2LONG result = C;
    updateD(result);
    return 2;
}

/**
 * @brief Write 0 or 1 to D, according to !C.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101101
 *
 * WRNC    D
 *
 * D = {31'b0, !C).
 */
int P2Cog::op_wrnc()
{
    const P2LONG result = C ^ 1;
    updateD(result);
    return 2;
}

/**
 * @brief Write 0 or 1 to D, according to  Z.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101110
 *
 * WRZ     D
 *
 * D = {31'b0,  Z).
 */
int P2Cog::op_wrz()
{
    const P2LONG result = Z;
    updateD(result);
    return 2;
}

/**
 * @brief Write 0 or 1 to D, according to !Z.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101111
 *
 * WRNZ    D
 *
 * D = {31'b0, !Z).
 */
int P2Cog::op_wrnz()
{
    const P2LONG result = Z ^ 1;
    updateD(result);
    return 2;
}

/**
 * @brief Modify C and Z according to cccc and zzzz.
 *
 * EEEE 1101011 CZ1 0cccczzzz 001101111
 *
 * MODCZ   c,z      {WC/WZ/WCZ}
 *
 * C = cccc[{C,Z}], Z = zzzz[{C,Z}].
 */
int P2Cog::op_modcz()
{
    const p2_cond_e cccc = static_cast<p2_cond_e>((IR.op.dst >> 4) & 15);
    const p2_cond_e zzzz = static_cast<p2_cond_e>((IR.op.dst >> 0) & 15);
    updateC(conditional(cccc));
    updateZ(conditional(zzzz));
    return 2;
}

/**
 * @brief Set scope mode.
 *
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
 */
int P2Cog::op_setscp()
{
    augmentD(IR.op.imm);
    HUB->wr_SCP(D);
    return 2;
}

/**
 * @brief Get scope values.
 *
 * EEEE 1101011 000 DDDDDDDDD 001110001
 *
 * Any time GETSCP is executed, the lower bytes of those four pins' RDPIN values are returned in D.
 *
 * GETSCP  D
 *
 * Pins D[5:2].
 */
int P2Cog::op_getscp()
{
    const P2LONG result = HUB->rd_SCP();
    updateD(result);
    return 2;
}

/**
 * @brief Jump to A.
 *
 * EEEE 1101100 RAA AAAAAAAAA AAAAAAAAA
 *
 * JMP     #A
 *
 * If R = 1, PC += A, else PC = A.
 */
int P2Cog::op_jmp_abs()
{
    const P2LONG result = (IR.op.wc ? (PC + IR.word) : IR.word) & A20MASK;
    updatePC(result);
    return 2;
}

/**
 * @brief Call to A by pushing {C, Z, 10'b0, PC[19:0]} onto stack.
 *
 * EEEE 1101101 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALL    #A
 *
 * If R = 1, PC += A, else PC = A.
 */
int P2Cog::op_call_abs()
{
    const P2LONG stack = (C << 31) | (Z << 30) | PC;
    const P2LONG result = (IR.op.wc ? (PC + IR.word) : IR.word) & A20MASK;
    pushK(stack);
    updatePC(result);
    return 2;
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRA++.
 *
 * EEEE 1101110 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLA   #A
 *
 * If R = 1, PC += A, else PC = A.
 */
int P2Cog::op_calla_abs()
{
    const P2LONG stack = (C << 31) | (Z << 30) | PC;
    const P2LONG result = (IR.op.wc ? (PC + IR.word) : IR.word) & A20MASK;
    pushPTRA(stack);
    updatePC(result);
    return 2;
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRB++.
 *
 * EEEE 1101111 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLB   #A
 *
 * If R = 1, PC += A, else PC = A.
 */
int P2Cog::op_callb_abs()
{
    const P2LONG stack = (C << 31) | (Z << 30) | PC;
    const P2LONG result = (IR.op.wc ? (PC + IR.word) : IR.word) & A20MASK;
    pushPTRB(stack);
    updatePC(result);
    return 2;
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 1110000 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PA,#A
 *
 * If R = 1, PC += A, else PC = A.
 */
int P2Cog::op_calld_pa_abs()
{
    const P2LONG stack = (C << 31) | (Z << 30) | PC;
    const P2LONG result = (IR.op.wc ? (PC + IR.word) : IR.word) & A20MASK;
    pushPA(stack);
    updatePC(result);
    return 2;
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 1110001 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PB,#A
 *
 * If R = 1, PC += A, else PC = A.
 */
int P2Cog::op_calld_pb_abs()
{
    const P2LONG stack = (C << 31) | (Z << 30) | PC;
    const P2LONG result = (IR.op.wc ? (PC + IR.word) : IR.word) & A20MASK;
    pushPB(stack);
    updatePC(result);
    return 2;
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 1110010 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PTRA,#A
 *
 * If R = 1, PC += A, else PC = A.
 */
int P2Cog::op_calld_ptra_abs()
{
    const P2LONG stack = (C << 31) | (Z << 30) | PC;
    const P2LONG result = (IR.op.wc ? (PC + IR.word) : IR.word) & A20MASK;
    pushPTRA(stack);
    updatePC(result);
    return 2;
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 1110011 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PTRB,#A
 *
 * If R = 1, PC += A, else PC = A.
 */
int P2Cog::op_calld_ptrb_abs()
{
    const P2LONG stack = (C << 31) | (Z << 30) | PC;
    const P2LONG result = (IR.op.wc ? (PC + IR.word) : IR.word) & A20MASK;
    pushPTRB(stack);
    updatePC(result);
    return 2;
}

/**
 * @brief Get {12'b0, address[19:0]} into PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 11101WW RAA AAAAAAAAA AAAAAAAAA
 *
 * LOC     PA/PB/PTRA/PTRB,#A
 *
 * If R = 1, address = PC + A, else address = A.
 */
int P2Cog::op_loc_pa()
{
    const P2LONG aaaa = IR.word & A20MASK;
    const P2LONG address = IR.op.wc ? PC + aaaa : aaaa;
    updatePA(address);
    return 2;
}

/**
 * @brief Get {12'b0, address[19:0]} into PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 11101WW RAA AAAAAAAAA AAAAAAAAA
 *
 * LOC     PA/PB/PTRA/PTRB,#A
 *
 * If R = 1, address = PC + A, else address = A.
 */
int P2Cog::op_loc_pb()
{
    const P2LONG aaaa = IR.word & A20MASK;
    const P2LONG address = IR.op.wc ? PC + aaaa : aaaa;
    updatePB(address);
    return 2;
}

/**
 * @brief Get {12'b0, address[19:0]} into PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 11101WW RAA AAAAAAAAA AAAAAAAAA
 *
 * LOC     PA/PB/PTRA/PTRB,#A
 *
 * If R = 1, address = PC + A, else address = A.
 */
int P2Cog::op_loc_ptra()
{
    const P2LONG aaaa = IR.word & A20MASK;
    const P2LONG address = IR.op.wc ? PC + aaaa : aaaa;
    updatePTRA(address);
    return 2;
}

/**
 * @brief Get {12'b0, address[19:0]} into PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 11101WW RAA AAAAAAAAA AAAAAAAAA
 *
 * LOC     PA/PB/PTRA/PTRB,#A
 *
 * If R = 1, address = PC + A, else address = A.
 */
int P2Cog::op_loc_ptrb()
{
    const P2LONG aaaa = IR.word & A20MASK;
    const P2LONG address = IR.op.wc ? PC + aaaa : aaaa;
    updatePTRB(address);
    return 2;
}

/**
 * @brief Queue #N[31:9] to be used as upper 23 bits for next #S occurrence, so that the next 9-bit #S will be augmented to 32 bits.
 *
 * EEEE 11110NN NNN NNNNNNNNN NNNNNNNNN
 *
 * AUGS    #N
 *
 */
int P2Cog::op_augs()
{
    S_aug = (IR.word << 9) & AUG;
    return 2;
}

/**
 * @brief Queue #N[31:9] to be used as upper 23 bits for next #D occurrence, so that the next 9-bit #D will be augmented to 32 bits.
 *
 * EEEE 11111NN NNN NNNNNNNNN NNNNNNNNN
 *
 * AUGD    #N
 *
 */
int P2Cog::op_augd()
{
    D_aug = (IR.word << 9) & AUG;
    return 2;
}
