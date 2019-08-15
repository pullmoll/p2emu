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

P2Cog::P2Cog(uchar cog_id, P2Hub* parent)
    : xoro_s()
    , HUB(parent)
    , ID(cog_id)
    , PC(0)
    , IR()
    , S(0)
    , D(0)
    , Q(0)
    , C(0)
    , Z(0)
    , S_next()
    , S_aug()
    , D_aug()
    , COG()
    , MEM(nullptr)
    , MEMSIZE(0)
{
    // Initialize the PRNG
    xoro_s[0] = 1;
    xoro_s[1] = 0;
    MEM = HUB->mem();
    MEMSIZE = HUB->memsize();
}

quint32 P2Cog::rd_cog(quint32 addr) const
{
    return COG[addr & 0x1ff];
}

void P2Cog::wr_cog(quint32 addr, quint32 val)
{
    COG[addr & 0x1ff] = val;
}

quint32 P2Cog::rd_lut(quint32 addr) const
{
    return LUT.RAM[addr & 0x1ff];
}

void P2Cog::wr_lut(quint32 addr, quint32 val)
{
    LUT.RAM[addr & 0x1ff] = val;
}

quint32 P2Cog::rd_mem(quint32 addr) const
{
    if (addr < 0x200)
        return rd_cog(addr);
    if (addr < 0x400)
        return rd_lut(addr - 0x200);
    if (HUB)
        return HUB->rd_LONG(addr * 4);
    return 0x00000000u;
}

void P2Cog::wr_mem(quint32 addr, quint32 val)
{
    if (HUB)
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
    case cond_never:
        return false;
    }
    return false;
}

/**
 * @brief Alias for conditional() with an unsigned parameter
 * @param cond condition (0 ... 15)
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
uchar P2Cog::msbit(quint32 val)
{
    if (val == 0)
        return 0;
    uchar pos;
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
uchar P2Cog::ones(quint32 val)
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
uchar P2Cog::parity(quint32 val)
{
    val ^= val >> 16;
    val ^= val >> 8;
    val ^= val >> 4;
    val &= 15;
    return (0x6996 >> val) & 1;
}

/**
 * @brief Rotate a 64 bit value %val left by %shift bits
 * @param val value to rotate
 * @param shift number of bits (0 .. 63)
 * @return rotated value
 */
quint64 P2Cog::rotl(quint64 val, uchar shift)
{
    return (val << shift) | (val >> (64 - shift));
}

/**
 * @brief Return the next PRNG value
 * @return pseudo random value
 */
quint64 P2Cog::rnd()
{
    const quint64 s0 = xoro_s[0];
    quint64 s1 = xoro_s[1];
    const quint64 result = s0 + s1;

    s1 ^= s0;
    xoro_s[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14); // a, b
    xoro_s[1] = rotl(s1, 36); // c
    return result;
}

/**
 * @brief Read and decode the next Propeller2 opcode
 * @return number of cycles
 */
quint32 P2Cog::decode()
{
    quint32 cycles = 2;

    if (PC < 0x0200) {
        // cogexec
        IR.word = COG[PC];
    } else if (PC < 0x0400) {
        // lutexec
        IR.word = LUT.RAM[PC - 0x200];
    } else if (MEM != nullptr && PC < MEMSIZE) {
        // hubexec
        // FIXME: use FIFO ?
        IR.word = HUB->rd_LONG(PC);
    } else {
        // no memory here
        IR.word = 0;
    }

    PC++;               // increment PC
    S = COG[IR.op.src]; // set S to COG[src]
    D = COG[IR.op.dst]; // set D to COG[dst]

    // check for the condition
    if (!conditional(IR.op.cond))
        return cycles;

    // Dispatch to op_xxx() functions
    switch (IR.op.inst) {
    case p2_ror:
        cycles = op_ror();
        break;

    case p2_rol:
        cycles = op_rol();
        break;

    case p2_shr:
        cycles = op_shr();
        break;

    case p2_shl:
        cycles = op_shl();
        break;

    case p2_rcr:
        cycles = op_rcr();
        break;

    case p2_rcl:
        cycles = op_rcl();
        break;

    case p2_sar:
        cycles = op_sar();
        break;

    case p2_sal:
        cycles = op_sal();
        break;

    case p2_add:
        cycles = op_add();
        break;

    case p2_addx:
        cycles = op_addx();
        break;

    case p2_adds:
        cycles = op_adds();
        break;

    case p2_addsx:
        cycles = op_addsx();
        break;

    case p2_sub:
        cycles = op_sub();
        break;

    case p2_subx:
        cycles = op_subx();
        break;

    case p2_subs:
        cycles = op_subs();
        break;

    case p2_subsx:
        cycles = op_subsx();
        break;

    case p2_cmp:
        cycles = op_cmp();
        break;

    case p2_cmpx:
        cycles = op_cmpx();
        break;

    case p2_cmps:
        cycles = op_cmps();
        break;

    case p2_cmpsx:
        cycles = op_cmpsx();
        break;

    case p2_cmpr:
        cycles = op_cmpr();
        break;

    case p2_cmpm:
        cycles = op_cmpm();
        break;

    case p2_subr:
        cycles = op_subr();
        break;

    case p2_cmpsub:
        cycles = op_cmpsub();
        break;

    case p2_fge:
        cycles = op_fge();
        break;

    case p2_fle:
        cycles = op_fle();
        break;

    case p2_fges:
        cycles = op_fges();
        break;

    case p2_fles:
        cycles = op_fles();
        break;

    case p2_sumc:
        cycles = op_sumc();
        break;

    case p2_sumnc:
        cycles = op_sumnc();
        break;

    case p2_sumz:
        cycles = op_sumz();
        break;

    case p2_sumnz:
        cycles = op_sumnz();
        break;

    case p2_testb_w:
    // case p2_bitl:
        cycles = (IR.op.uc != IR.op.uz) ? op_testb_w()
                                        : op_bitl();
        break;

    case p2_testbn_w:
    // case p2_bith:
        cycles = (IR.op.uc != IR.op.uz) ? op_testbn_w()
                                        : op_bith();
        break;

    case p2_testb_and:
    // case p2_bitc:
        cycles = (IR.op.uc != IR.op.uz) ? op_testb_and()
                                        : op_bitc();
        break;

    case p2_testbn_and:
    // case p2_bitnc:
        cycles = (IR.op.uc != IR.op.uz) ? op_testbn_and()
                                        : op_bitnc();
        break;

    case p2_testb_or:
    // case p2_bitz:
        cycles = (IR.op.uc != IR.op.uz) ? op_testb_or()
                                        : op_bitz();
        break;

    case p2_testbn_or:
    // case p2_bitnz:
        cycles = (IR.op.uc != IR.op.uz) ? op_testbn_or()
                                        : op_bitnz();
        break;

    case p2_testb_xor:
    // case p2_bitrnd:
        cycles = (IR.op.uc != IR.op.uz) ? op_testb_xor()
                                        : op_bitrnd();
        break;

    case p2_testbn_xor:
    // case p2_bitnot:
        cycles = (IR.op.uc != IR.op.uz) ? op_testbn_xor()
                                        : op_bitnot();
        break;

    case p2_and:
        cycles = op_and();
        break;

    case p2_andn:
        cycles = op_andn();
        break;

    case p2_or:
        cycles = op_or();
        break;

    case p2_xor:
        cycles = op_xor();
        break;

    case p2_muxc:
        cycles = op_muxc();
        break;

    case p2_muxnc:
        cycles = op_muxnc();
        break;

    case p2_muxz:
        cycles = op_muxz();
        break;

    case p2_muxnz:
        cycles = op_muxnz();
        break;

    case p2_mov:
        cycles = op_mov();
        break;

    case p2_not:
        cycles = op_not();
        break;

    case p2_abs:
        cycles = op_abs();
        break;

    case p2_neg:
        cycles = op_neg();
        break;

    case p2_negc:
        cycles = op_negc();
        break;

    case p2_negnc:
        cycles = op_negnc();
        break;

    case p2_negz:
        cycles = op_negz();
        break;

    case p2_negnz:
        cycles = op_negnz();
        break;

    case p2_incmod:
        cycles = op_incmod();
        break;

    case p2_decmod:
        cycles = op_decmod();
        break;

    case p2_zerox:
        cycles = op_zerox();
        break;

    case p2_signx:
        cycles = op_signx();
        break;

    case p2_encod:
        cycles = op_encod();
        break;

    case p2_ones:
        cycles = op_ones();
        break;

    case p2_test:
        cycles = op_test();
        break;

    case p2_testn:
        cycles = op_testn();
        break;

    case p2_setnib_0:
    case p2_setnib_1:
        cycles = op_setnib();
        break;

    case p2_getnib_0:
    case p2_getnib_1:
        cycles = op_getnib();
        break;

    case p2_rolnib_0:
    case p2_rolnib_1:
        cycles = op_rolnib();
        break;

    case p2_setbyte:
        cycles = op_setbyte();
        break;

    case p2_getbyte:
        cycles = op_getbyte();
        break;

    case p2_rolbyte:
        cycles = op_rolbyte();
        break;

    case p2_1001001:
        if (IR.op.uc == 0) {
            cycles = (IR.op.dst == 0 && IR.op.uz == 0) ? op_setword_altsw()
                                                       : op_setword();
        } else {
            cycles = (IR.op.src == 0 && IR.op.uz == 0) ? op_getword_altgw()
                                                       : op_getword();
        }
        break;

    case p2_1001010:
        if (IR.op.uc == 0) {
            cycles = (IR.op.src == 0 && IR.op.uz == 0) ? op_rolword_altgw()
                                                       : op_rolword();
        } else {
            if (IR.op.uz == 0) {
                cycles = (IR.op.src == 0 && IR.op.imm == 1) ? op_altsn_d()
                                                            : op_altsn();
            } else {
                cycles = (IR.op.src == 0 && IR.op.imm == 1) ? op_altgn_d()
                                                            : op_altgn();
            }
        }
        break;

    case p2_1001011:
        if (IR.op.uc == 0) {
            if (IR.op.uz == 0) {
                cycles = (IR.op.src == 0 && IR.op.imm == 1) ? op_altsb_d()
                                                            : op_altsb();
            } else {
                cycles = (IR.op.src == 0 && IR.op.imm == 1) ? op_altgb_d()
                                                            : op_altgb();
            }
        } else {
            if (IR.op.uz == 0) {
                cycles = (IR.op.src == 0 && IR.op.imm == 1) ? op_altsw_d()
                                                            : op_altsw();
            } else {
                cycles = (IR.op.src == 0 && IR.op.imm == 1) ? op_altgw_d()
                                                            : op_altgw();
            }
        }
        break;

    case p2_1001100:
        if (IR.op.uc == 0) {
            if (IR.op.uz == 0) {
                cycles = (IR.op.src == 0 && IR.op.imm == 1) ? op_altr_d()
                                                            : op_altr();
            } else {
                cycles = (IR.op.src == 0 && IR.op.imm == 1) ? op_altd_d()
                                                            : op_altd();
            }
        } else {
            if (IR.op.uz == 0) {
                cycles = (IR.op.src == 0 && IR.op.imm == 1) ? op_alts_d()
                                                            : op_alts();
            } else {
                cycles = (IR.op.src == 0 && IR.op.imm == 1) ? op_altb_d()
                                                            : op_altb();
            }
        }
        break;

    case p2_1001101:
        if (IR.op.uc == 0) {
            if (IR.op.uz == 0) {
                cycles = (IR.op.imm == 1 && IR.op.src == 0x164 /* 101100100 */) ? op_alti_d()
                                                                                : op_alti();
            } else {
                cycles = op_setr();
            }
        } else {
            cycles = (IR.op.uz == 0) ? op_setd()
                                     : op_sets();
        }
        break;

    case p2_1001110:
        if (IR.op.uc == 0) {
            if (IR.op.uz == 0) {
                cycles = (IR.op.imm == 0 && IR.op.src == IR.op.dst) ? op_decod_d()
                                                                    : op_decod();
            } else {
                cycles = (IR.op.imm == 0 && IR.op.src == IR.op.dst) ? op_bmask_d()
                                                                    : op_bmask();
            }
        } else {
            if (IR.op.uz == 0) {
                cycles = op_crcbit();
            } else {
                cycles = op_crcnib();
            }
        }
        break;

    case p2_1001111:
        if (IR.op.uc == 0) {
            if (IR.op.uz == 0) {
                cycles = op_muxnits();
            } else {
                cycles = op_muxnibs();
            }
        } else {
            if (IR.op.uz == 0) {
                cycles = op_muxq();
            } else {
                cycles = op_movbyts();
            }
        }
        break;

    case p2_1010000:
        if (IR.op.uc == 0) {
            cycles = op_mul();
        } else {
            cycles = op_muls();
        }
        break;

    case p2_1010001:
        if (IR.op.uc == 0) {
            cycles = op_sca();
        } else {
            cycles = op_scas();
        }
        break;

    case p2_1010010:
        if (IR.op.uc == 0) {
            if (IR.op.uz == 0) {
                cycles = op_addpix();
            } else {
                cycles = op_mulpix();
            }
        } else {
            if (IR.op.uz == 0) {
                cycles = op_blnpix();
            } else {
                cycles = op_mixpix();
            }
        }
        break;

    case p2_1010011:
        if (IR.op.uc == 0) {
            if (IR.op.uz == 0) {
                cycles = op_addct1();
            } else {
                cycles = op_addct2();
            }
        } else {
            if (IR.op.uz == 0) {
                cycles = op_addct3();
            } else {
                cycles = op_wmlong();
            }
        }
        break;

    case p2_1010100:
        if (IR.op.uz == 0) {
            cycles = op_rqpin();
        } else {

        }
        break;

    case p2_rdlut:
        cycles = op_rdlut();
        break;

    case p2_rdbyte:
        cycles = op_rdbyte();
        break;

    case p2_rdword:
        cycles = op_rdword();
        break;

    case p2_rdlong:
        cycles = op_rdlong();
        break;

    case p2_calld:
        cycles = op_calld();
        break;

    case p2_callp:
        cycles = (IR.op.uc == 0) ? op_callpa() : op_callpb();
        break;

    case p2_1011011:
        if (IR.op.uc == 0) {
            if (IR.op.uz == 0) {
                cycles = op_djz();
            } else {
                cycles = op_djnz();
            }
        } else {
            if (IR.op.uz == 0) {
                cycles = op_djf();
            } else {
                cycles = op_djnf();
            }
        }
        break;

    case p2_1011100:
        if (IR.op.uc == 0) {
            if (IR.op.uz == 0) {
                cycles = op_ijz();
            } else {
                cycles = op_ijnz();
            }
        } else {
            if (IR.op.uz == 0) {
                cycles = op_tjz();
            } else {
                cycles = op_tjnz();
            }
        }
        break;

    case p2_1011101:
        if (IR.op.uc == 0) {
            if (IR.op.uz == 0) {
                cycles = op_tjf();
            } else {
                cycles = op_tjnf();
            }
        } else {
            if (IR.op.uz == 0) {
                cycles = op_tjs();
            } else {
                cycles = op_tjns();
            }
        }
        break;

    case p2_1011110:
        if (IR.op.uc == 0) {
            if (IR.op.uz == 0) {
                cycles = op_tjv();
            } else {
                switch (IR.op.dst) {
                case 0x00:
                    cycles = op_jint();
                    break;
                case 0x01:
                    cycles = op_jct1();
                    break;
                case 0x02:
                    cycles = op_jct2();
                    break;
                case 0x03:
                    cycles = op_jct3();
                    break;
                case 0x04:
                    cycles = op_jse1();
                    break;
                case 0x05:
                    cycles = op_jse2();
                    break;
                case 0x06:
                    cycles = op_jse3();
                    break;
                case 0x07:
                    cycles = op_jse4();
                    break;
                case 0x08:
                    cycles = op_jpat();
                    break;
                case 0x09:
                    cycles = op_jfbw();
                    break;
                case 0x0a:
                    cycles = op_jxmt();
                    break;
                case 0x0b:
                    cycles = op_jxfi();
                    break;
                case 0x0c:
                    cycles = op_jxro();
                    break;
                case 0x0d:
                    cycles = op_jxrl();
                    break;
                case 0x0e:
                    cycles = op_jatn();
                    break;
                case 0x0f:
                    cycles = op_jqmt();
                    break;
                case 0x10:
                    cycles = op_jnint();
                    break;
                case 0x11:
                    cycles = op_jnct1();
                    break;
                case 0x12:
                    cycles = op_jnct2();
                    break;
                case 0x13:
                    cycles = op_jnct3();
                    break;
                case 0x14:
                    cycles = op_jnse1();
                    break;
                case 0x15:
                    cycles = op_jnse2();
                    break;
                case 0x16:
                    cycles = op_jnse3();
                    break;
                case 0x17:
                    cycles = op_jnse4();
                    break;
                case 0x18:
                    cycles = op_jnpat();
                    break;
                case 0x19:
                    cycles = op_jnfbw();
                    break;
                case 0x1a:
                    cycles = op_jnxmt();
                    break;
                case 0x1b:
                    cycles = op_jnxfi();
                    break;
                case 0x1c:
                    cycles = op_jnxro();
                    break;
                case 0x1d:
                    cycles = op_jnxrl();
                    break;
                case 0x1e:
                    cycles = op_jnatn();
                    break;
                case 0x1f:
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
        if (IR.op.uc == 0) {
            cycles = op_1011111_0();
        } else {
            cycles = op_setpat();
        }
        break;

    case p2_1100000:
        if (IR.op.uc == 0) {
            cycles = (IR.op.uz == 1 && IR.op.dst == 1) ? op_akpin()
                                                       : op_wrpin();
        } else {
            cycles = op_wxpin();
        }
        break;

    case p2_1100001:
        if (IR.op.uc == 0) {
            cycles = op_wypin();
        } else {
            cycles = op_wrlut();
        }
        break;

    case p2_1100010:
        if (IR.op.uc == 0) {
            cycles = op_wrbyte();
        } else {
            cycles = op_wrword();
        }
        break;

    case p2_1100011:
        if (IR.op.uc == 0) {
            cycles = op_wrlong();
        } else {
            cycles = op_rdfast();
        }
        break;

    case p2_1100100:
        if (IR.op.uc == 0) {
            cycles = op_wrfast();
        } else {
            cycles = op_fblock();
        }
        break;

    case p2_1100101:
        if (IR.op.uc == 0) {
            if (IR.op.uz == 1 && IR.op.imm == 1 && IR.op.src == 0 && IR.op.dst == 0) {
                cycles = op_xstop();
            } else {
                cycles = op_xinit();
            }
        } else {
            cycles = op_xzero();
        }
        break;

    case p2_1100110:
        if (IR.op.uc == 0) {
            cycles = op_xcont();
        } else {
            cycles = op_rep();
        }
        break;

    case p2_coginit:
        cycles = op_coginit();
        break;

    case p2_1101000:
        if (IR.op.uc == 0) {
            cycles = op_qmul();
        } else {
            cycles = op_qdiv();
        }
        break;

    case p2_1101001:
        if (IR.op.uc == 0) {
            cycles = op_qfrac();
        } else {
            cycles = op_qsqrt();
        }
        break;

    case p2_1101010:
        if (IR.op.uc == 0) {
            cycles = op_qrotate();
        } else {
            cycles = op_qvector();
        }
        break;

    case p2_1101011:
        switch (IR.op.src) {
        case 0x00:
            cycles = op_hubset();
            break;
        case 0x01:
            cycles = op_cogid();
            break;
        case 0x03:
            cycles = op_cogstop();
            break;
        case 0x04:
            cycles = op_locknew();
            break;
        case 0x05:
            cycles = op_lockret();
            break;
        case 0x06:
            cycles = op_locktry();
            break;
        case 0x07:
            cycles = op_lockrel();
            break;
        case 0x0e:
            cycles = op_qlog();
            break;
        case 0x0f:
            cycles = op_qexp();
            break;
        case 0x10:
            cycles = op_rfbyte();
            break;
        case 0x11:
            cycles = op_rfword();
            break;
        case 0x12:
            cycles = op_rflong();
            break;
        case 0x13:
            cycles = op_rfvar();
            break;
        case 0x14:
            cycles = op_rfvars();
            break;
        case 0x15:
            cycles = op_wfbyte();
            break;
        case 0x16:
            cycles = op_wfword();
            break;
        case 0x17:
            cycles = op_wflong();
            break;
        case 0x18:
            cycles = op_getqx();
            break;
        case 0x19:
            cycles = op_getqy();
            break;
        case 0x1a:
            cycles = op_getct();
            break;
        case 0x1b:
            cycles = (IR.op.dst == 0) ? op_getrnd_cz()
                                      : op_getrnd();
            break;
        case 0x1c:
            cycles = op_setdacs();
            break;
        case 0x1d:
            cycles = op_setxfrq();
            break;
        case 0x1e:
            cycles = op_getxacc();
            break;
        case 0x1f:
            cycles = op_waitx();
            break;
        case 0x20:
            cycles = op_setse1();
            break;
        case 0x21:
            cycles = op_setse2();
            break;
        case 0x22:
            cycles = op_setse3();
            break;
        case 0x23:
            cycles = op_setse4();
            break;
        case 0x24:
            switch (IR.op.dst) {
            case 0x00:
                cycles = op_pollint();
                break;
            case 0x01:
                cycles = op_pollct1();
                break;
            case 0x02:
                cycles = op_pollct2();
                break;
            case 0x03:
                cycles = op_pollct3();
                break;
            case 0x04:
                cycles = op_pollse1();
                break;
            case 0x05:
                cycles = op_pollse2();
                break;
            case 0x06:
                cycles = op_pollse3();
                break;
            case 0x07:
                cycles = op_pollse4();
                break;
            case 0x08:
                cycles = op_pollpat();
                break;
            case 0x09:
                cycles = op_pollfbw();
                break;
            case 0x0a:
                cycles = op_pollxmt();
                break;
            case 0x0b:
                cycles = op_pollxfi();
                break;
            case 0x0c:
                cycles = op_pollxro();
                break;
            case 0x0d:
                cycles = op_pollxrl();
                break;
            case 0x0e:
                cycles = op_pollatn();
                break;
            case 0x0f:
                cycles = op_pollqmt();
                break;
            case 0x10:
                cycles = op_waitint();
                break;
            case 0x11:
                cycles = op_waitct1();
                break;
            case 0x12:
                cycles = op_waitct2();
                break;
            case 0x13:
                cycles = op_waitct3();
                break;
            case 0x14:
                cycles = op_waitse1();
                break;
            case 0x15:
                cycles = op_waitse2();
                break;
            case 0x16:
                cycles = op_waitse3();
                break;
            case 0x17:
                cycles = op_waitse4();
                break;
            case 0x18:
                cycles = op_waitpat();
                break;
            case 0x19:
                cycles = op_waitfbw();
                break;
            case 0x1a:
                cycles = op_waitxmt();
                break;
            case 0x1b:
                cycles = op_waitxfi();
                break;
            case 0x1c:
                cycles = op_waitxro();
                break;
            case 0x1d:
                cycles = op_waitxrl();
                break;
            case 0x1e:
                cycles = op_waitatn();
                break;
            case 0x20:
                cycles = op_allowi();
                break;
            case 0x21:
                cycles = op_stalli();
                break;
            case 0x22:
                cycles = op_trgint1();
                break;
            case 0x23:
                cycles = op_trgint2();
                break;
            case 0x24:
                cycles = op_trgint3();
                break;
            case 0x25:
                cycles = op_nixint1();
                break;
            case 0x26:
                cycles = op_nixint2();
                break;
            case 0x27:
                cycles = op_nixint3();
                break;
            }
            break;
        case 0x25:
            cycles = op_setint1();
            break;
        case 0x26:
            cycles = op_setint2();
            break;
        case 0x27:
            cycles = op_setint3();
            break;
        case 0x28:
            cycles = op_setq();
            break;
        case 0x29:
            cycles = op_setq2();
            break;
        case 0x2a:
            cycles = op_push();
            break;
        case 0x2b:
            cycles = op_pop();
            break;
        case 0x2c:
            cycles = op_jmp();
            break;
        case 0x2d:
            cycles = (IR.op.imm == 0) ? op_call()
                                      : op_ret();
            break;
        case 0x2e:
            cycles = (IR.op.imm == 0) ? op_calla()
                                      : op_reta();
            break;
        case 0x2f:
            cycles = (IR.op.imm == 0) ? op_callb()
                                      : op_retb();
            break;
        case 0x30:
            cycles = op_jmprel();
            break;
        case 0x31:
            cycles = op_skip();
            break;
        case 0x32:
            cycles = op_skipf();
            break;
        case 0x33:
            cycles = op_execf();
            break;
        case 0x34:
            cycles = op_getptr();
            break;
        case 0x35:
            cycles = (IR.op.uc == 0 && IR.op.uz == 0) ? op_cogbrk()
                                                      : op_getbrk();
            break;
        case 0x36:
            cycles = op_brk();
            break;
        case 0x37:
            cycles = op_setluts();
            break;
        case 0x38:
            cycles = op_setcy();
            break;
        case 0x39:
            cycles = op_setci();
            break;
        case 0x3a:
            cycles = op_setcq();
            break;
        case 0x3b:
            cycles = op_setcfrq();
            break;
        case 0x3c:
            cycles = op_setcmod();
            break;
        case 0x3d:
            cycles = op_setpiv();
            break;
        case 0x3e:
            cycles = op_setpix();
            break;
        case 0x3f:
            cycles = op_cogatn();
            break;
        case 0x40:
            cycles = (IR.op.uc == IR.op.uz) ? op_testp_w()
                                            : op_dirl();
            break;
        case 0x41:
            cycles = (IR.op.uc == IR.op.uz) ? op_testpn_w()
                                            : op_dirh();
            break;
        case 0x42:
            cycles = (IR.op.uc == IR.op.uz) ? op_testp_and()
                                            : op_dirc();
            break;
        case 0x43:
            cycles = (IR.op.uc == IR.op.uz) ? op_testpn_and()
                                            : op_dirnc();
            break;
        case 0x44:
            cycles = (IR.op.uc == IR.op.uz) ? op_testp_or()
                                            : op_dirz();
            break;
        case 0x45:
            cycles = (IR.op.uc == IR.op.uz) ? op_testpn_or()
                                            : op_dirnz();
            break;
        case 0x46:
            cycles = (IR.op.uc == IR.op.uz) ? op_testp_xor()
                                            : op_dirrnd();
            break;
        case 0x47:
            cycles = (IR.op.uc == IR.op.uz) ? op_testpn_xor()
                                            : op_dirnot();
            break;
        case 0x48:
            cycles = op_outl();
            break;
        case 0x49:
            cycles = op_outh();
            break;
        case 0x4a:
            cycles = op_outc();
            break;
        case 0x4b:
            cycles = op_outnc();
            break;
        case 0x4c:
            cycles = op_outz();
            break;
        case 0x4d:
            cycles = op_outnz();
            break;
        case 0x4e:
            cycles = op_outrnd();
            break;
        case 0x4f:
            cycles = op_outnot();
            break;
        case 0x50:
            cycles = op_fltl();
            break;
        case 0x51:
            cycles = op_flth();
            break;
        case 0x52:
            cycles = op_fltc();
            break;
        case 0x53:
            cycles = op_fltnc();
            break;
        case 0x54:
            cycles = op_fltz();
            break;
        case 0x55:
            cycles = op_fltnz();
            break;
        case 0x56:
            cycles = op_fltrnd();
            break;
        case 0x57:
            cycles = op_fltnot();
            break;
        case 0x58:
            cycles = op_drvl();
            break;
        case 0x59:
            cycles = op_drvh();
            break;
        case 0x5a:
            cycles = op_drvc();
            break;
        case 0x5b:
            cycles = op_drvnc();
            break;
        case 0x5c:
            cycles = op_drvz();
            break;
        case 0x5d:
            cycles = op_drvnz();
            break;
        case 0x5e:
            cycles = op_drvrnd();
            break;
        case 0x5f:
            cycles = op_drvnot();
            break;
        case 0x60:
            cycles = op_splitb();
            break;
        case 0x61:
            cycles = op_mergeb();
            break;
        case 0x62:
            cycles = op_splitw();
            break;
        case 0x63:
            cycles = op_mergew();
            break;
        case 0x64:
            cycles = op_seussf();
            break;
        case 0x65:
            cycles = op_seussr();
            break;
        case 0x66:
            cycles = op_rgbsqz();
            break;
        case 0x67:
            cycles = op_rgbexp();
            break;
        case 0x68:
            cycles = op_xoro32();
            break;
        case 0x69:
            cycles = op_rev();
            break;
        case 0x6a:
            cycles = op_rczr();
            break;
        case 0x6b:
            cycles = op_rczl();
            break;
        case 0x6c:
            cycles = op_wrc();
            break;
        case 0x6d:
            cycles = op_wrnc();
            break;
        case 0x6e:
            cycles = op_wrz();
            break;
        case 0x6f:
            cycles = op_wrnz();
            break;
        case 0x7f:
            cycles = op_modcz();
            break;
        }
        break;

    case p2_jmp_abs:
        cycles = op_jmp_abs();
        break;

    case p2_call_abs:
        cycles = op_call_abs();
        break;

    case p2_calla_abs:
        cycles = op_calla_abs();
        break;

    case p2_callb_abs:
        cycles = op_callb_abs();
        break;

    case p2_calld_pa_abs:
    case p2_calld_pb_abs:
    case p2_calld_ptra_abs:
    case p2_calld_ptrb_abs:
        cycles = op_calld_abs();
        break;

    case p2_loc_pa:
        cycles = op_loc_pa();
        break;

    case p2_loc_pb:
        cycles = op_loc_pb();
        break;

    case p2_loc_ptra:
        cycles = op_loc_ptra();
        break;

    case p2_loc_ptrb:
        cycles = op_loc_ptrb();
        break;

    case p2_augs_00:
    case p2_augs_01:
    case p2_augs_10:
    case p2_augs_11:
        cycles = op_augs();
        break;

    case p2_augd_00:
    case p2_augd_01:
    case p2_augd_10:
    case p2_augd_11:
        cycles = op_augd();
        break;
    }

    return cycles;
}

/**
 * @brief No operation.
 *
 * 0000 0000000 000 000000000 000000000
 *
 * NOP
 *
 */
uint P2Cog::op_nop()
{
    return 2;
}

/**
 * @brief Rotate right.
 *
 * EEEE 0000000 CZI DDDDDDDDD SSSSSSSSS
 *
 * ROR     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [31:0]  of ({D[31:0], D[31:0]}     >> S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[0].
 * Z = (result == 0).
 */
uint P2Cog::op_ror()
{
    if (0 == IR.word)
        return op_nop();
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint64 accu = U64(D) << 32 | U64(D);
    const quint32 result = U32L(accu >> shift);
    updateC((D & (LSB << shift)) != 0);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Rotate left.
 *
 * EEEE 0000001 CZI DDDDDDDDD SSSSSSSSS
 *
 * ROL     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [63:32] of ({D[31:0], D[31:0]}     << S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[31].
 * Z = (result == 0).
 */
uint P2Cog::op_rol()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint64 accu = U64(D) << 32 | U64(D);
    const quint32 result = U32H(accu << shift);
    updateC((D & (MSB >> shift)) != 0);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Shift right.
 *
 * EEEE 0000010 CZI DDDDDDDDD SSSSSSSSS
 *
 * SHR     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [31:0]  of ({32'b0, D[31:0]}       >> S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[0].
 * Z = (result == 0).
 */
uint P2Cog::op_shr()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint64 accu = U64(D);
    const quint32 result = U32L(accu >> shift);
    updateC((D & (LSB << shift)) != 0);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Shift left.
 *
 * EEEE 0000011 CZI DDDDDDDDD SSSSSSSSS
 *
 * SHL     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [63:32] of ({D[31:0], 32'b0}       << S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[31].
 * Z = (result == 0).
 */
uint P2Cog::op_shl()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint64 accu = U64(D) << 32;
    const quint32 result = U32H(accu << shift);
    updateC((D & (MSB >> shift)) != 0);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Rotate carry right.
 *
 * EEEE 0000100 CZI DDDDDDDDD SSSSSSSSS
 *
 * RCR     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [31:0]  of ({{32{C}}, D[31:0]}     >> S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[0].
 * Z = (result == 0).
 */
uint P2Cog::op_rcr()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint64 accu = U64(D) | C ? HMAX : 0;
    const quint32 result = U32L(accu >> shift);
    updateC((D & (LSB << shift)) != 0);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Rotate carry left.
 *
 * EEEE 0000101 CZI DDDDDDDDD SSSSSSSSS
 *
 * RCL     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [63:32] of ({D[31:0], {32{C}}}     << S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[31].
 * Z = (result == 0).
 */
uint P2Cog::op_rcl()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint64 accu = U64(D) << 32 | C ? LMAX : 0;
    const quint32 result = U32H(accu << shift);
    updateC((D & (MSB >> shift)) != 0);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Shift arithmetic right.
 *
 * EEEE 0000110 CZI DDDDDDDDD SSSSSSSSS
 *
 * SAR     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [31:0]  of ({{32{D[31]}}, D[31:0]} >> S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[0].
 * Z = (result == 0).
 */
uint P2Cog::op_sar()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint64 accu = U64(D) | (D & MSB) ? HMAX : 0;
    const quint32 result = U32L(accu >> shift);
    updateC((D & (LSB << shift)) != 0);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Shift arithmetic left.
 *
 * EEEE 0000111 CZI DDDDDDDDD SSSSSSSSS
 *
 * SAL     D,{#}S   {WC/WZ/WCZ}
 *
 * D = [63:32] of ({D[31:0], {32{D[0]}}}  << S[4:0]).
 * C = last bit shifted out if S[4:0] > 0, else D[31].
 * Z = (result == 0).
 */
uint P2Cog::op_sal()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint64 accu = U64(D) << 32 | (D & LSB) ? LMAX : 0;
    const quint32 result = U32H(accu << shift);
    updateC((D & (MSB >> shift)) != 0);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Add S into D.
 *
 * EEEE 0001000 CZI DDDDDDDDD SSSSSSSSS
 *
 * ADD     D,{#}S   {WC/WZ/WCZ}
 *
 * D = D + S.
 * C = carry of (D + S).
 * Z = (result == 0).
 */
uint P2Cog::op_add()
{
    augmentS(IR.op.imm);
    const quint64 accu = U64(D) + U64(S);
    const quint32 result = U32L(accu);
    updateC((accu >> 32) & 1);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Add (S + C) into D, extended.
 *
 * EEEE 0001001 CZI DDDDDDDDD SSSSSSSSS
 *
 * ADDX    D,{#}S   {WC/WZ/WCZ}
 *
 * D = D + S + C.
 * C = carry of (D + S + C).
 * Z = Z AND (result == 0).
 */
uint P2Cog::op_addx()
{
    augmentS(IR.op.imm);
    const quint64 accu = U64(D) + U64(S) + C;
    const quint32 result = U32L(accu);
    updateC((accu >> 32) & 1);
    updateZ(Z & (result == 0));
    updateD(result);
    return 2;
}

/**
 * @brief Add S into D, signed.
 *
 * EEEE 0001010 CZI DDDDDDDDD SSSSSSSSS
 *
 * ADDS    D,{#}S   {WC/WZ/WCZ}
 *
 * D = D + S.
 * C = correct sign of (D + S).
 * Z = (result == 0).
 */
uint P2Cog::op_adds()
{
    augmentS(IR.op.imm);
    const bool sign = (S32(D) ^ S32(S)) < 0;
    const qint64 accu = SX64(D) + SX64(S);
    const quint32 result = U32L(accu);
    updateC((accu < 0) ^ sign);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Add (S + C) into D, signed and extended.
 *
 * EEEE 0001011 CZI DDDDDDDDD SSSSSSSSS
 *
 * ADDSX   D,{#}S   {WC/WZ/WCZ}
 *
 * D = D + S + C.
 * C = correct sign of (D + S + C).
 * Z = Z AND (result == 0).
 */
uint P2Cog::op_addsx()
{
    augmentS(IR.op.imm);
    const uchar sign = (D ^ (S + C)) >> 31;
    const qint64 accu = SX64(D) + SX64(S) + C;
    const quint32 result = U32L(accu);
    updateC((accu < 0) ^ sign);
    updateZ(Z & (result == 0));
    updateD(result);
    return 2;
}

/**
 * @brief Subtract S from D.
 *
 * EEEE 0001100 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUB     D,{#}S   {WC/WZ/WCZ}
 *
 * D = D - S.
 * C = borrow of (D - S).
 * Z = (result == 0).
 */
uint P2Cog::op_sub()
{
    augmentS(IR.op.imm);
    const quint64 accu = U64(D) - U64(S);
    const quint32 result = U32L(accu);
    updateC((accu >> 32) & 1);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Subtract (S + C) from D, extended.
 *
 * EEEE 0001101 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUBX    D,{#}S   {WC/WZ/WCZ}
 *
 * D = D - (S + C).
 * C = borrow of (D - (S + C)).
 * Z = Z AND (result == 0).
 */
uint P2Cog::op_subx()
{
    augmentS(IR.op.imm);
    const quint64 accu = U64(D) - (U64(S) + C);
    const quint32 result = U32L(accu);
    updateC((accu >> 32) & 1);
    updateZ(Z & (result == 0));
    updateD(result);
    return 2;
}

/**
 * @brief Subtract S from D, signed.
 *
 * EEEE 0001110 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUBS    D,{#}S   {WC/WZ/WCZ}
 *
 * D = D - S.
 * C = correct sign of (D - S).
 * Z = (result == 0).
 */
uint P2Cog::op_subs()
{
    augmentS(IR.op.imm);
    const bool sign = (S32(D) ^ S32(S)) < 0;
    const qint64 accu = SX64(D) - SX64(S);
    const quint32 result = U32L(accu);
    updateC((accu < 0) ^ sign);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Subtract (S + C) from D, signed and extended.
 *
 * EEEE 0001111 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUBSX   D,{#}S   {WC/WZ/WCZ}
 *
 * D = D - (S + C).
 * C = correct sign of (D - (S + C)).
 * Z = Z AND (result == 0).
 */
uint P2Cog::op_subsx()
{
    augmentS(IR.op.imm);
    const uchar sign = (D ^ (S + C)) >> 31;
    const qint64 accu = SX64(D) - (SX64(S) + C);
    const quint32 result = U32L(accu);
    updateC((accu < 0) ^ sign);
    updateZ(Z & (result == 0));
    updateD(result);
    return 2;
}

/**
 * @brief Compare D to S.
 *
 * EEEE 0010000 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMP     D,{#}S   {WC/WZ/WCZ}
 *
 * C = borrow of (D - S).
 * Z = (D == S).
 */
uint P2Cog::op_cmp()
{
    augmentS(IR.op.imm);
    const quint64 accu = U64(D) - U64(S);
    const quint32 result = U32L(accu);
    updateC((accu >> 32) & 1);
    updateZ(result == 0);
    return 2;
}

/**
 * @brief Compare D to (S + C), extended.
 *
 * EEEE 0010001 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMPX    D,{#}S   {WC/WZ/WCZ}
 *
 * C = borrow of (D - (S + C)).
 * Z = Z AND (D == S + C).
 */
uint P2Cog::op_cmpx()
{
    augmentS(IR.op.imm);
    const quint64 accu = U64(D) - (U64(S) + C);
    const quint32 result = U32L(accu);
    updateC((accu >> 32) & 1);
    updateZ(Z & (result == 0));
    return 2;
}

/**
 * @brief Compare D to S, signed.
 *
 * EEEE 0010010 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMPS    D,{#}S   {WC/WZ/WCZ}
 *
 * C = correct sign of (D - S).
 * Z = (D == S).
 */
uint P2Cog::op_cmps()
{
    augmentS(IR.op.imm);
    const bool sign = (S32(D) ^ S32(S)) < 0;
    const qint64 accu = SX64(D) - SX64(S);
    const quint32 result = U32L(accu);
    updateC((accu < 0) ^ sign);
    updateZ(result == 0);
    return 2;
}

/**
 * @brief Compare D to (S + C), signed and extended.
 *
 * EEEE 0010011 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMPSX   D,{#}S   {WC/WZ/WCZ}
 *
 * C = correct sign of (D - (S + C)).
 * Z = Z AND (D == S + C).
 */
uint P2Cog::op_cmpsx()
{
    augmentS(IR.op.imm);
    const uchar sign = (D ^ (S + C)) >> 31;
    const qint64 accu = SX64(D) - (SX64(S) + C);
    const quint32 result = U32L(accu);
    updateC((accu < 0) ^ sign);
    updateZ(Z & (result == 0));
    return 2;
}

/**
 * @brief Compare S to D (reverse).
 *
 * EEEE 0010100 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMPR    D,{#}S   {WC/WZ/WCZ}
 *
 * C = borrow of (S - D).
 * Z = (D == S).
 */
uint P2Cog::op_cmpr()
{
    augmentS(IR.op.imm);
    const quint64 accu = U64(S) - U64(D);
    const quint32 result = U32L(accu);
    updateC((accu >> 32) & 1);
    updateZ(result == 0);
    return 2;
}

/**
 * @brief Compare D to S, get MSB of difference into C.
 *
 * EEEE 0010101 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMPM    D,{#}S   {WC/WZ/WCZ}
 *
 * C = MSB of (D - S).
 * Z = (D == S).
 */
uint P2Cog::op_cmpm()
{
    augmentS(IR.op.imm);
    const quint64 accu = U64(D) - U64(S);
    const quint32 result = U32L(accu);
    updateC((accu >> 31) & 1);
    updateZ(result == 0);
    return 2;
}

/**
 * @brief Subtract D from S (reverse).
 *
 * EEEE 0010110 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUBR    D,{#}S   {WC/WZ/WCZ}
 *
 * D = S - D.
 * C = borrow of (S - D).
 * Z = (result == 0).
 */
uint P2Cog::op_subr()
{
    augmentS(IR.op.imm);
    const quint64 accu = U64(S) - U64(D);
    const quint32 result = U32L(accu);
    updateC((accu >> 32) & 1);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Compare and subtract S from D if D >= S.
 *
 * EEEE 0010111 CZI DDDDDDDDD SSSSSSSSS
 *
 * CMPSUB  D,{#}S   {WC/WZ/WCZ}
 *
 * If D => S then D = D - S and C = 1, else D same and C = 0.
 * Z = (result == 0).
 */
uint P2Cog::op_cmpsub()
{
    augmentS(IR.op.imm);
    if (D < S) {
        // Do not change D
        const quint32 result = D;
        updateC(0);
        updateZ(result == 0);
    } else {
        // Do the subtract and set C = 1, if WC is set
        const quint64 accu = U64(D) - U64(S);
        const quint32 result = U32L(accu);
        updateC(1);
        updateZ(result == 0);
        updateD(result);
    }
    return 2;
}

/**
 * @brief Force D >= S.
 *
 * EEEE 0011000 CZI DDDDDDDDD SSSSSSSSS
 *
 * FGE     D,{#}S   {WC/WZ/WCZ}
 *
 * If D < S then D = S and C = 1, else D same and C = 0.
 * Z = (result == 0).
 */
uint P2Cog::op_fge()
{
    augmentS(IR.op.imm);
    if (D < S) {
        const quint32 result = S;
        updateC(1);
        updateZ(result == 0);
        updateD(result);
    } else {
        const quint32 result = D;
        updateC(0);
        updateZ(result == 0);
    }
    return 2;
}

/**
 * @brief Force D <= S.
 *
 * EEEE 0011001 CZI DDDDDDDDD SSSSSSSSS
 *
 * FLE     D,{#}S   {WC/WZ/WCZ}
 *
 * If D > S then D = S and C = 1, else D same and C = 0.
 * Z = (result == 0).
 */
uint P2Cog::op_fle()
{
    augmentS(IR.op.imm);
    if (D > S) {
        const quint32 result = S;
        updateC(1);
        updateZ(result == 0);
        updateD(result);
    } else {
        const quint32 result = D;
        updateC(0);
        updateZ(result == 0);
    }
    return 2;
}

/**
 * @brief Force D >= S, signed.
 *
 * EEEE 0011010 CZI DDDDDDDDD SSSSSSSSS
 *
 * FGES    D,{#}S   {WC/WZ/WCZ}
 *
 * If D < S then D = S and C = 1, else D same and C = 0.
 * Z = (result == 0).
 */
uint P2Cog::op_fges()
{
    augmentS(IR.op.imm);
    if (S32(D) < S32(S)) {
        const quint32 result = S;
        updateC(1);
        updateZ(result == 0);
        updateD(result);
    } else {
        const quint32 result = D;
        updateC(0);
        updateZ(result == 0);
    }
    return 2;
}

/**
 * @brief Force D <= S, signed.
 *
 * EEEE 0011011 CZI DDDDDDDDD SSSSSSSSS
 *
 * FLES    D,{#}S   {WC/WZ/WCZ}
 *
 * If D > S then D = S and C = 1, else D same and C = 0.
 * Z = (result == 0).
 */
uint P2Cog::op_fles()
{
    augmentS(IR.op.imm);
    if (S32(D) > S32(S)) {
        const quint32 result = S;
        updateC(1);
        updateZ(result == 0);
        updateD(result);
    } else {
        const quint32 result = D;
        updateC(0);
        updateZ(result == 0);
    }
    return 2;
}

/**
 * @brief Sum +/-S into D by  C.
 *
 * EEEE 0011100 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUMC    D,{#}S   {WC/WZ/WCZ}
 *
 * If C = 1 then D = D - S, else D = D + S.
 * C = correct sign of (D +/- S).
 * Z = (result == 0).
 */
uint P2Cog::op_sumc()
{
    augmentS(IR.op.imm);
    const bool sign = (S32(D) ^ S32(S)) < 0;
    const quint64 accu = C ? U64(D) - U64(S) : U64(D) + U64(S);
    const quint32 result = U32L(accu);
    updateC(((accu >> 32) & 1) ^ sign);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Sum +/-S into D by !C.
 *
 * EEEE 0011101 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUMNC   D,{#}S   {WC/WZ/WCZ}
 *
 * If C = 0 then D = D - S, else D = D + S.
 * C = correct sign of (D +/- S).
 * Z = (result == 0).
 */
uint P2Cog::op_sumnc()
{
    augmentS(IR.op.imm);
    const bool sign = (S32(D) ^ S32(S)) < 0;
    const quint64 accu = !C ? U64(D) - U64(S) : U64(D) + U64(S);
    const quint32 result = U32L(accu);
    updateC(((accu >> 32) & 1) ^ sign);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Sum +/-S into D by  Z.
 *
 * EEEE 0011110 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUMZ    D,{#}S   {WC/WZ/WCZ}
 *
 * If Z = 1 then D = D - S, else D = D + S.
 * C = correct sign of (D +/- S).
 * Z = (result == 0).
 */
uint P2Cog::op_sumz()
{
    augmentS(IR.op.imm);
    const bool sign = (S32(D) ^ S32(S)) < 0;
    const quint64 accu = Z ? U64(D) - U64(S) : U64(D) + U64(S);
    const quint32 result = U32L(accu);
    updateC(((accu >> 32) & 1) ^ sign);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Sum +/-S into D by !Z.
 *
 * EEEE 0011111 CZI DDDDDDDDD SSSSSSSSS
 *
 * SUMNZ   D,{#}S   {WC/WZ/WCZ}
 *
 * If Z = 0 then D = D - S, else D = D + S.
 * C = correct sign of (D +/- S).
 * Z = (result == 0).
 */
uint P2Cog::op_sumnz()
{
    augmentS(IR.op.imm);
    const bool sign = (S32(D) ^ S32(S)) < 0;
    const quint64 accu = !Z ? U64(D) - U64(S) : U64(D) + U64(S);
    const quint32 result = U32L(accu);
    updateC(((accu >> 32) & 1) ^ sign);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Test bit S[4:0] of  D, write to C/Z.
 *
 * EEEE 0100000 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTB   D,{#}S         WC/WZ
 *
 * C/Z =          D[S[4:0]].
 */
uint P2Cog::op_testb_w()
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
 *
 * EEEE 0100001 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTBN  D,{#}S         WC/WZ
 *
 * C/Z =         !D[S[4:0]].
 */
uint P2Cog::op_testbn_w()
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
 *
 * EEEE 0100010 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTB   D,{#}S     ANDC/ANDZ
 *
 * C/Z = C/Z AND  D[S[4:0]].
 */
uint P2Cog::op_testb_and()
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
 *
 * EEEE 0100011 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTBN  D,{#}S     ANDC/ANDZ
 *
 * C/Z = C/Z AND !D[S[4:0]].
 */
uint P2Cog::op_testbn_and()
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
 *
 * EEEE 0100100 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTB   D,{#}S       ORC/ORZ
 *
 * C/Z = C/Z OR   D[S[4:0]].
 */
uint P2Cog::op_testb_or()
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
 *
 * EEEE 0100101 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTBN  D,{#}S       ORC/ORZ
 *
 * C/Z = C/Z OR  !D[S[4:0]].
 */
uint P2Cog::op_testbn_or()
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
 *
 * EEEE 0100110 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTB   D,{#}S     XORC/XORZ
 *
 * C/Z = C/Z XOR  D[S[4:0]].
 */
uint P2Cog::op_testb_xor()
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
 *
 * EEEE 0100111 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTBN  D,{#}S     XORC/XORZ
 *
 * C/Z = C/Z XOR !D[S[4:0]].
 */
uint P2Cog::op_testbn_xor()
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
 *
 * EEEE 0100000 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITL    D,{#}S         {WCZ}
 *
 */
uint P2Cog::op_bitl()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint32 bit = LSB << shift;
    const quint32 result = D & ~bit;
    updateC((result >> shift) & 1);
    updateZ((result >> shift) & 1);
    return 2;
}

/**
 * @brief Bit S[4:0] of D = 1,    C,Z = D[S[4:0]].
 *
 * EEEE 0100001 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITH    D,{#}S         {WCZ}
 *
 */
uint P2Cog::op_bith()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint32 bit = LSB << shift;
    const quint32 result = D | bit;
    updateC((result >> shift) & 1);
    updateZ((result >> shift) & 1);
    return 2;
}

/**
 * @brief Bit S[4:0] of D = C,    C,Z = D[S[4:0]].
 *
 * EEEE 0100010 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITC    D,{#}S         {WCZ}
 *
 */
uint P2Cog::op_bitc()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint32 bit = LSB << shift;
    const quint32 result = C ? D | bit : D & ~bit;
    updateC((result >> shift) & 1);
    updateZ((result >> shift) & 1);
    return 2;
}

/**
 * @brief Bit S[4:0] of D = !C,   C,Z = D[S[4:0]].
 *
 * EEEE 0100011 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNC   D,{#}S         {WCZ}
 *
 */
uint P2Cog::op_bitnc()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint32 bit = LSB << shift;
    const quint32 result = !C ? D | bit : D & ~bit;
    updateC((result >> shift) & 1);
    updateZ((result >> shift) & 1);
    return 2;
}

/**
 * @brief Bit S[4:0] of D = Z,    C,Z = D[S[4:0]].
 *
 * EEEE 0100100 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITZ    D,{#}S         {WCZ}
 *
 */
uint P2Cog::op_bitz()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint32 bit = LSB << shift;
    const quint32 result = Z ? D | bit : D & ~bit;
    updateC((result >> shift) & 1);
    updateZ((result >> shift) & 1);
    return 2;
}

/**
 * @brief Bit S[4:0] of D = !Z,   C,Z = D[S[4:0]].
 *
 * EEEE 0100101 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNZ   D,{#}S         {WCZ}
 *
 */
uint P2Cog::op_bitnz()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint32 bit = LSB << shift;
    const quint32 result = !Z ? D | bit : D & ~bit;
    updateC((result >> shift) & 1);
    updateZ((result >> shift) & 1);
    return 2;
}

/**
 * @brief Bit S[4:0] of D = RND,  C,Z = D[S[4:0]].
 *
 * EEEE 0100110 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITRND  D,{#}S         {WCZ}
 *
 */
uint P2Cog::op_bitrnd()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint32 bit = LSB << shift;
    const quint32 result = (rnd() & LSB) ? D | bit : D & ~bit;
    updateC((result >> shift) & 1);
    updateZ((result >> shift) & 1);
    return 2;
}

/**
 * @brief Bit S[4:0] of D = !bit, C,Z = D[S[4:0]].
 *
 * EEEE 0100111 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNOT  D,{#}S         {WCZ}
 *
 */
uint P2Cog::op_bitnot()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint32 bit = LSB << shift;
    const quint32 result = D ^ bit;
    updateC((result >> shift) & 1);
    updateZ((result >> shift) & 1);
    return 2;
}

/**
 * @brief AND S into D.
 *
 * EEEE 0101000 CZI DDDDDDDDD SSSSSSSSS
 *
 * AND     D,{#}S   {WC/WZ/WCZ}
 *
 * D = D & S.
 * C = parity of result.
 * Z = (result == 0).
 */
uint P2Cog::op_and()
{
    augmentS(IR.op.imm);
    const quint32 result = D & S;
    updateC(parity(result));
    updateZ(result == 0);
    return 2;
}

/**
 * @brief AND !S into D.
 *
 * EEEE 0101001 CZI DDDDDDDDD SSSSSSSSS
 *
 * ANDN    D,{#}S   {WC/WZ/WCZ}
 *
 * D = D & !S.
 * C = parity of result.
 * Z = (result == 0).
 */
uint P2Cog::op_andn()
{
    augmentS(IR.op.imm);
    const quint32 result = D & ~S;
    updateC(parity(result));
    updateZ(result == 0);
    return 2;
}

/**
 * @brief OR S into D.
 *
 * EEEE 0101010 CZI DDDDDDDDD SSSSSSSSS
 *
 * OR      D,{#}S   {WC/WZ/WCZ}
 *
 * D = D | S.
 * C = parity of result.
 * Z = (result == 0).
 */
uint P2Cog::op_or()
{
    augmentS(IR.op.imm);
    const quint32 result = D | S;
    updateC(parity(result));
    updateZ(result == 0);
    return 2;
}

/**
 * @brief XOR S into D.
 *
 * EEEE 0101011 CZI DDDDDDDDD SSSSSSSSS
 *
 * XOR     D,{#}S   {WC/WZ/WCZ}
 *
 * D = D ^ S.
 * C = parity of result.
 * Z = (result == 0).
 */
uint P2Cog::op_xor()
{
    augmentS(IR.op.imm);
    const quint32 result = D ^ S;
    updateC(parity(result));
    updateZ(result == 0);
    return 2;
}

/**
 * @brief Mux  C into each D bit that is '1' in S.
 *
 * EEEE 0101100 CZI DDDDDDDDD SSSSSSSSS
 *
 * MUXC    D,{#}S   {WC/WZ/WCZ}
 *
 * D = (!S & D ) | (S & {32{ C}}).
 * C = parity of result.
 * Z = (result == 0).
 */
uint P2Cog::op_muxc()
{
    augmentS(IR.op.imm);
    const quint32 result = (D & ~S) | (C ? S : 0);
    updateC(parity(result));
    updateZ(result == 0);
    return 2;
}

/**
 * @brief Mux !C into each D bit that is '1' in S.
 *
 * EEEE 0101101 CZI DDDDDDDDD SSSSSSSSS
 *
 * MUXNC   D,{#}S   {WC/WZ/WCZ}
 *
 * D = (!S & D ) | (S & {32{!C}}).
 * C = parity of result.
 * Z = (result == 0).
 */
uint P2Cog::op_muxnc()
{
    augmentS(IR.op.imm);
    const quint32 result = (D & ~S) | (!C ? S : 0);
    updateC(parity(result));
    updateZ(result == 0);
    return 2;
}

/**
 * @brief Mux  Z into each D bit that is '1' in S.
 *
 * EEEE 0101110 CZI DDDDDDDDD SSSSSSSSS
 *
 * MUXZ    D,{#}S   {WC/WZ/WCZ}
 *
 * D = (!S & D ) | (S & {32{ Z}}).
 * C = parity of result.
 * Z = (result == 0).
 */
uint P2Cog::op_muxz()
{
    augmentS(IR.op.imm);
    const quint32 result = (D & ~S) | (Z ? S : 0);
    updateC(parity(result));
    updateZ(result == 0);
    return 2;
}

/**
 * @brief Mux !Z into each D bit that is '1' in S.
 *
 * EEEE 0101111 CZI DDDDDDDDD SSSSSSSSS
 *
 * MUXNZ   D,{#}S   {WC/WZ/WCZ}
 *
 * D = (!S & D ) | (S & {32{!Z}}).
 * C = parity of result.
 * Z = (result == 0).
 */
uint P2Cog::op_muxnz()
{
    augmentS(IR.op.imm);
    const quint32 result = (D & ~S) | (!Z ? S : 0);
    updateC(parity(result));
    updateZ(result == 0);
    return 2;
}

/**
 * @brief Move S into D.
 *
 * EEEE 0110000 CZI DDDDDDDDD SSSSSSSSS
 *
 * MOV     D,{#}S   {WC/WZ/WCZ}
 *
 * D = S.
 * C = S[31].
 * Z = (result == 0).
 */
uint P2Cog::op_mov()
{
    augmentS(IR.op.imm);
    const quint32 result = S;
    updateC(result >> 31);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Get !S into D.
 *
 * EEEE 0110001 CZI DDDDDDDDD SSSSSSSSS
 *
 * NOT     D,{#}S   {WC/WZ/WCZ}
 *
 * D = !S.
 * C = !S[31].
 * Z = (result == 0).
 */
uint P2Cog::op_not()
{
    augmentS(IR.op.imm);
    const quint32 result = ~S;
    updateC(result >> 31);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Get absolute value of S into D.
 *
 * EEEE 0110010 CZI DDDDDDDDD SSSSSSSSS
 *
 * ABS     D,{#}S   {WC/WZ/WCZ}
 *
 * D = ABS(S).
 * C = S[31].
 * Z = (result == 0).
 */
uint P2Cog::op_abs()
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
 *
 * EEEE 0110011 CZI DDDDDDDDD SSSSSSSSS
 *
 * NEG     D,{#}S   {WC/WZ/WCZ}
 *
 * D = -S.
 * C = MSB of result.
 * Z = (result == 0).
 */
uint P2Cog::op_neg()
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
 *
 * EEEE 0110100 CZI DDDDDDDDD SSSSSSSSS
 *
 * NEGC    D,{#}S   {WC/WZ/WCZ}
 *
 * If C = 1 then D = -S, else D = S.
 * C = MSB of result.
 * Z = (result == 0).
 */
uint P2Cog::op_negc()
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
 *
 * EEEE 0110101 CZI DDDDDDDDD SSSSSSSSS
 *
 * NEGNC   D,{#}S   {WC/WZ/WCZ}
 *
 * If C = 0 then D = -S, else D = S.
 * C = MSB of result.
 * Z = (result == 0).
 */
uint P2Cog::op_negnc()
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
 *
 * EEEE 0110110 CZI DDDDDDDDD SSSSSSSSS
 *
 * NEGZ    D,{#}S   {WC/WZ/WCZ}
 *
 * If Z = 1 then D = -S, else D = S.
 * C = MSB of result.
 * Z = (result == 0).
 */
uint P2Cog::op_negz()
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
 *
 * EEEE 0110111 CZI DDDDDDDDD SSSSSSSSS
 *
 * NEGNZ   D,{#}S   {WC/WZ/WCZ}
 *
 * If Z = 0 then D = -S, else D = S.
 * C = MSB of result.
 * Z = (result == 0).
 */
uint P2Cog::op_negnz()
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
 *
 * EEEE 0111000 CZI DDDDDDDDD SSSSSSSSS
 *
 * INCMOD  D,{#}S   {WC/WZ/WCZ}
 *
 * If D = S then D = 0 and C = 1, else D = D + 1 and C = 0.
 * Z = (result == 0).
 */
uint P2Cog::op_incmod()
{
    augmentS(IR.op.imm);
    const quint32 result = (D == S) ? 0 : D + 1;
    updateC(result == 0);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Decrement with modulus.
 *
 * EEEE 0111001 CZI DDDDDDDDD SSSSSSSSS
 *
 * DECMOD  D,{#}S   {WC/WZ/WCZ}
 *
 * If D = 0 then D = S and C = 1, else D = D - 1 and C = 0.
 * Z = (result == 0).
 */
uint P2Cog::op_decmod()
{
    augmentS(IR.op.imm);
    const quint32 result = (D == 0) ? S : D - 1;
    updateC(result == S);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Zero-extend D above bit S[4:0].
 *
 * EEEE 0111010 CZI DDDDDDDDD SSSSSSSSS
 *
 * ZEROX   D,{#}S   {WC/WZ/WCZ}
 *
 * C = MSB of result.
 * Z = (result == 0).
 */
uint P2Cog::op_zerox()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint32 msb = (D >> (shift - 1)) & 1;
    const quint32 mask = 0xffffffffu << shift;
    const quint32 result = D & ~mask;
    updateC(msb);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Sign-extend D from bit S[4:0].
 *
 * EEEE 0111011 CZI DDDDDDDDD SSSSSSSSS
 *
 * SIGNX   D,{#}S   {WC/WZ/WCZ}
 *
 * C = MSB of result.
 * Z = (result == 0).
 */
uint P2Cog::op_signx()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint32 msb = (D >> (shift - 1)) & 1;
    const quint32 mask = 0xffffffffu << shift;
    const quint32 result = msb ? D | mask : D & ~mask;
    updateC(msb);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Get bit position of top-most '1' in S into D.
 *
 * EEEE 0111100 CZI DDDDDDDDD SSSSSSSSS
 *
 * ENCOD   D,{#}S   {WC/WZ/WCZ}
 *
 * D = position of top '1' in S (0..31).
 * C = (S != 0).
 * Z = (result == 0).
 */
uint P2Cog::op_encod()
{
    augmentS(IR.op.imm);
    const quint32 result = msbit(S);
    updateC(S != 0);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Get number of '1's in S into D.
 *
 * EEEE 0111101 CZI DDDDDDDDD SSSSSSSSS
 *
 * ONES    D,{#}S   {WC/WZ/WCZ}
 *
 * D = number of '1's in S (0..32).
 * C = LSB of result.
 * Z = (result == 0).
 */
uint P2Cog::op_ones()
{
    augmentS(IR.op.imm);
    const quint32 result = ones(S);
    updateC(result & 1);
    updateZ(result == 0);
    return 2;
}

/**
 * @brief Test D with S.
 *
 * EEEE 0111110 CZI DDDDDDDDD SSSSSSSSS
 *
 * TEST    D,{#}S   {WC/WZ/WCZ}
 *
 * C = parity of (D & S).
 * Z = ((D & S) == 0).
 */
uint P2Cog::op_test()
{
    augmentS(IR.op.imm);
    const quint32 result = D & S;
    updateC(parity(result));
    updateZ(result == 0);
    return 2;
}

/**
 * @brief Test D with !S.
 *
 * EEEE 0111111 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTN   D,{#}S   {WC/WZ/WCZ}
 *
 * C = parity of (D & !S).
 * Z = ((D & !S) == 0).
 */
uint P2Cog::op_testn()
{
    augmentS(IR.op.imm);
    const quint32 result = D & ~S;
    updateC(parity(result));
    updateZ(result == 0);
    return 2;
}

/**
 * @brief Set S[3:0] into nibble N in D, keeping rest of D same.
 *
 * EEEE 100000N NNI DDDDDDDDD SSSSSSSSS
 *
 * SETNIB  D,{#}S,#N
 *
 */
uint P2Cog::op_setnib()
{
    augmentS(IR.op.imm);
    const uchar shift = static_cast<uchar>((IR.word >> 19) & 7) * 4;
    const quint32 mask = LNIBBLE << shift;
    const quint32 result = (D & ~mask) | ((S << shift) & mask);
    updateD(result);
    return 2;
}

/**
 * @brief Set S[3:0] into nibble established by prior ALTSN instruction.
 *
 * EEEE 1000000 00I 000000000 SSSSSSSSS
 *
 * SETNIB  {#}S
 *
 */
uint P2Cog::op_setnib_altsn()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Get nibble N of S into D.
 *
 * EEEE 100001N NNI DDDDDDDDD SSSSSSSSS
 *
 * GETNIB  D,{#}S,#N
 *
 * D = {28'b0, S.
 * NIBBLE[N]).
 */
uint P2Cog::op_getnib()
{
    augmentS(IR.op.imm);
    const uchar shift = static_cast<uchar>((IR.word >> 19) & 7) * 4;
    const quint32 result = (S >> shift) & LNIBBLE;
    updateD(result);
    return 2;
}

/**
 * @brief Get nibble established by prior ALTGN instruction into D.
 *
 * EEEE 1000010 000 DDDDDDDDD 000000000
 *
 * GETNIB  D
 *
 */
uint P2Cog::op_getnib_altgn()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Rotate-left nibble N of S into D.
 *
 * EEEE 100010N NNI DDDDDDDDD SSSSSSSSS
 *
 * ROLNIB  D,{#}S,#N
 *
 * D = {D[27:0], S.NIBBLE[N]).
 */
uint P2Cog::op_rolnib()
{
    augmentS(IR.op.imm);
    const uchar shift = static_cast<uchar>((IR.word >> 19) & 7) * 4;
    const quint32 result = (D << 4) | ((S >> shift) & LNIBBLE);
    updateD(result);
    return 2;
}

/**
 * @brief Rotate-left nibble established by prior ALTGN instruction into D.
 *
 * EEEE 1000100 000 DDDDDDDDD 000000000
 *
 * ROLNIB  D
 *
 */
uint P2Cog::op_rolnib_altgn()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Set S[7:0] into byte N in D, keeping rest of D same.
 *
 * EEEE 1000110 NNI DDDDDDDDD SSSSSSSSS
 *
 * SETBYTE D,{#}S,#N
 *
 */
uint P2Cog::op_setbyte()
{
    augmentS(IR.op.imm);
    const uchar shift = static_cast<uchar>((IR.word >> 19) & 3) * 8;
    const quint32 mask = LBYTE << shift;
    const quint32 result = (D & ~mask) | ((S << shift) & mask);
    updateD(result);
    return 2;
}

/**
 * @brief Set S[7:0] into byte established by prior ALTSB instruction.
 *
 * EEEE 1000110 00I 000000000 SSSSSSSSS
 *
 * SETBYTE {#}S
 *
 */
uint P2Cog::op_setbyte_altsb()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Get byte N of S into D.
 *
 * EEEE 1000111 NNI DDDDDDDDD SSSSSSSSS
 *
 * GETBYTE D,{#}S,#N
 *
 * D = {24'b0, S.BYTE[N]).
 */
uint P2Cog::op_getbyte()
{
    augmentS(IR.op.imm);
    const uchar shift = static_cast<uchar>((IR.word >> 19) & 3) * 8;
    const quint32 result = (S >> shift) & LBYTE;
    updateD(result);
    return 2;
}

/**
 * @brief Get byte established by prior ALTGB instruction into D.
 *
 * EEEE 1000111 000 DDDDDDDDD 000000000
 *
 * GETBYTE D
 *
 */
uint P2Cog::op_getbyte_altgb()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Rotate-left byte N of S into D.
 *
 * EEEE 1001000 NNI DDDDDDDDD SSSSSSSSS
 *
 * ROLBYTE D,{#}S,#N
 *
 * D = {D[23:0], S.BYTE[N]).
 */
uint P2Cog::op_rolbyte()
{
    augmentS(IR.op.imm);
    const uchar shift = static_cast<uchar>((IR.word >> 19) & 3) * 8;
    const quint32 result = (D << 8) | ((S >> shift) & LBYTE);
    updateD(result);
    return 2;
}

/**
 * @brief Rotate-left byte established by prior ALTGB instruction into D.
 *
 * EEEE 1001000 000 DDDDDDDDD 000000000
 *
 * ROLBYTE D
 *
 */
uint P2Cog::op_rolbyte_altgb()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Set S[15:0] into word N in D, keeping rest of D same.
 *
 * EEEE 1001001 0NI DDDDDDDDD SSSSSSSSS
 *
 * SETWORD D,{#}S,#N
 *
 */
uint P2Cog::op_setword()
{
    augmentS(IR.op.imm);
    const uchar shift = IR.op.uz ? 16 : 0;
    const quint32 mask = LWORD << shift;
    const quint32 result = (D & ~mask) | ((S >> shift) & mask);
    updateD(result);
    return 2;
}

/**
 * @brief Set S[15:0] into word established by prior ALTSW instruction.
 *
 * EEEE 1001001 00I 000000000 SSSSSSSSS
 *
 * SETWORD {#}S
 *
 */
uint P2Cog::op_setword_altsw()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Get word N of S into D.
 *
 * EEEE 1001001 1NI DDDDDDDDD SSSSSSSSS
 *
 * GETWORD D,{#}S,#N
 *
 * D = {16'b0, S.WORD[N]).
 */
uint P2Cog::op_getword()
{
    augmentS(IR.op.imm);
    const uchar shift = IR.op.uz * 16;
    const quint32 result = (S >> shift) & LWORD;
    updateD(result);
    return 2;
}

/**
 * @brief Get word established by prior ALTGW instruction into D.
 *
 * EEEE 1001001 100 DDDDDDDDD 000000000
 *
 * GETWORD D
 *
 */
uint P2Cog::op_getword_altgw()
{
    return 2;
}

/**
 * @brief Rotate-left word N of S into D.
 *
 * EEEE 1001010 0NI DDDDDDDDD SSSSSSSSS
 *
 * ROLWORD D,{#}S,#N
 *
 * D = {D[15:0], S.WORD[N]).
 */
uint P2Cog::op_rolword()
{
    augmentS(IR.op.imm);
    const uchar shift = IR.op.uz * 16;
    const quint32 result = (D << 16) & ((S >> shift) & LWORD);
    updateD(result);
    return 2;
}

/**
 * @brief Rotate-left word established by prior ALTGW instruction into D.
 *
 * EEEE 1001010 000 DDDDDDDDD 000000000
 *
 * ROLWORD D
 *
 */
uint P2Cog::op_rolword_altgw()
{
    return 2;
}

/**
 * @brief Alter subsequent SETNIB instruction.
 *
 * EEEE 1001010 10I DDDDDDDDD SSSSSSSSS
 *
 * ALTSN   D,{#}S
 *
 * Next D field = (D[11:3] + S) & $1FF, N field = D[2:0].
 * D += sign-extended S[17:9].
 */
uint P2Cog::op_altsn()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Alter subsequent SETNIB instruction.
 *
 * EEEE 1001010 101 DDDDDDDDD 000000000
 *
 * ALTSN   D
 *
 * Next D field = D[11:3], N field = D[2:0].
 */
uint P2Cog::op_altsn_d()
{
    return 2;
}

/**
 * @brief Alter subsequent GETNIB/ROLNIB instruction.
 *
 * EEEE 1001010 11I DDDDDDDDD SSSSSSSSS
 *
 * ALTGN   D,{#}S
 *
 * Next S field = (D[11:3] + S) & $1FF, N field = D[2:0].
 * D += sign-extended S[17:9].
 */
uint P2Cog::op_altgn()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Alter subsequent GETNIB/ROLNIB instruction.
 *
 * EEEE 1001010 111 DDDDDDDDD 000000000
 *
 * ALTGN   D
 *
 * Next S field = D[11:3], N field = D[2:0].
 */
uint P2Cog::op_altgn_d()
{
    return 2;
}

/**
 * @brief Alter subsequent SETBYTE instruction.
 *
 * EEEE 1001011 00I DDDDDDDDD SSSSSSSSS
 *
 * ALTSB   D,{#}S
 *
 * Next D field = (D[10:2] + S) & $1FF, N field = D[1:0].
 * D += sign-extended S[17:9].
 */
uint P2Cog::op_altsb()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Alter subsequent SETBYTE instruction.
 *
 * EEEE 1001011 001 DDDDDDDDD 000000000
 *
 * ALTSB   D
 *
 * Next D field = D[10:2], N field = D[1:0].
 */
uint P2Cog::op_altsb_d()
{
    return 2;
}

/**
 * @brief Alter subsequent GETBYTE/ROLBYTE instruction.
 *
 * EEEE 1001011 01I DDDDDDDDD SSSSSSSSS
 *
 * ALTGB   D,{#}S
 *
 * Next S field = (D[10:2] + S) & $1FF, N field = D[1:0].
 * D += sign-extended S[17:9].
 */
uint P2Cog::op_altgb()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Alter subsequent GETBYTE/ROLBYTE instruction.
 *
 * EEEE 1001011 011 DDDDDDDDD 000000000
 *
 * ALTGB   D
 *
 * Next S field = D[10:2], N field = D[1:0].
 */
uint P2Cog::op_altgb_d()
{
    return 2;
}

/**
 * @brief Alter subsequent SETWORD instruction.
 *
 * EEEE 1001011 10I DDDDDDDDD SSSSSSSSS
 *
 * ALTSW   D,{#}S
 *
 * Next D field = (D[9:1] + S) & $1FF, N field = D[0].
 * D += sign-extended S[17:9].
 */
uint P2Cog::op_altsw()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Alter subsequent SETWORD instruction.
 *
 * EEEE 1001011 101 DDDDDDDDD 000000000
 *
 * ALTSW   D
 *
 * Next D field = D[9:1], N field = D[0].
 */
uint P2Cog::op_altsw_d()
{
    return 2;
}

/**
 * @brief Alter subsequent GETWORD/ROLWORD instruction.
 *
 * EEEE 1001011 11I DDDDDDDDD SSSSSSSSS
 *
 * ALTGW   D,{#}S
 *
 * Next S field = ((D[9:1] + S) & $1FF), N field = D[0].
 * D += sign-extended S[17:9].
 */
uint P2Cog::op_altgw()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Alter subsequent GETWORD/ROLWORD instruction.
 *
 * EEEE 1001011 111 DDDDDDDDD 000000000
 *
 * ALTGW   D
 *
 * Next S field = D[9:1], N field = D[0].
 */
uint P2Cog::op_altgw_d()
{
    return 2;
}

/**
 * @brief Alter result register address (normally D field) of next instruction to (D + S) & $1FF.
 *
 * EEEE 1001100 00I DDDDDDDDD SSSSSSSSS
 *
 * ALTR    D,{#}S
 *
 * D += sign-extended S[17:9].
 */
uint P2Cog::op_altr()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Alter result register address (normally D field) of next instruction to D[8:0].
 *
 * EEEE 1001100 001 DDDDDDDDD 000000000
 *
 * ALTR    D
 *
 */
uint P2Cog::op_altr_d()
{
    return 2;
}

/**
 * @brief Alter D field of next instruction to (D + S) & $1FF.
 *
 * EEEE 1001100 01I DDDDDDDDD SSSSSSSSS
 *
 * ALTD    D,{#}S
 *
 * D += sign-extended S[17:9].
 */
uint P2Cog::op_altd()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Alter D field of next instruction to D[8:0].
 *
 * EEEE 1001100 011 DDDDDDDDD 000000000
 *
 * ALTD    D
 *
 */
uint P2Cog::op_altd_d()
{
    return 2;
}

/**
 * @brief Alter S field of next instruction to (D + S) & $1FF.
 *
 * EEEE 1001100 10I DDDDDDDDD SSSSSSSSS
 *
 * ALTS    D,{#}S
 *
 * D += sign-extended S[17:9].
 */
uint P2Cog::op_alts()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Alter S field of next instruction to D[8:0].
 *
 * EEEE 1001100 101 DDDDDDDDD 000000000
 *
 * ALTS    D
 *
 */
uint P2Cog::op_alts_d()
{
    return 2;
}

/**
 * @brief Alter D field of next instruction to (D[13:5] + S) & $1FF.
 *
 * EEEE 1001100 11I DDDDDDDDD SSSSSSSSS
 *
 * ALTB    D,{#}S
 *
 * D += sign-extended S[17:9].
 */
uint P2Cog::op_altb()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Alter D field of next instruction to D[13:5].
 *
 * EEEE 1001100 111 DDDDDDDDD 000000000
 *
 * ALTB    D
 *
 */
uint P2Cog::op_altb_d()
{
    return 2;
}

/**
 * @brief Substitute next instruction's I/R/D/S fields with fields from D, per S.
 *
 * EEEE 1001101 00I DDDDDDDDD SSSSSSSSS
 *
 * ALTI    D,{#}S
 *
 * Modify D per S.
 */
uint P2Cog::op_alti()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Execute D in place of next instruction.
 *
 * EEEE 1001101 001 DDDDDDDDD 101100100
 *
 * ALTI    D
 *
 * D stays same.
 */
uint P2Cog::op_alti_d()
{
    return 2;
}

/**
 * @brief Set R field of D to S[8:0].
 *
 * EEEE 1001101 01I DDDDDDDDD SSSSSSSSS
 *
 * SETR    D,{#}S
 *
 * D = {D[31:28], S[8:0], D[18:0]}.
 */
uint P2Cog::op_setr()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Set D field of D to S[8:0].
 *
 * EEEE 1001101 10I DDDDDDDDD SSSSSSSSS
 *
 * SETD    D,{#}S
 *
 * D = {D[31:18], S[8:0], D[8:0]}.
 */
uint P2Cog::op_setd()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Set S field of D to S[8:0].
 *
 * EEEE 1001101 11I DDDDDDDDD SSSSSSSSS
 *
 * SETS    D,{#}S
 *
 * D = {D[31:9], S[8:0]}.
 */
uint P2Cog::op_sets()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Decode S[4:0] into D.
 *
 * EEEE 1001110 00I DDDDDDDDD SSSSSSSSS
 *
 * DECOD   D,{#}S
 *
 * D = 1 << S[4:0].
 */
uint P2Cog::op_decod()
{
    augmentS(IR.op.imm);
    const uchar shift = S & 31;
    const quint32 result = LSB << shift;
    updateD(result);
    return 2;
}

/**
 * @brief Decode D[4:0] into D.
 *
 * EEEE 1001110 000 DDDDDDDDD DDDDDDDDD
 *
 * DECOD   D
 *
 * D = 1 << D[4:0].
 */
uint P2Cog::op_decod_d()
{
    return 2;
}

/**
 * @brief Get LSB-justified bit mask of size (S[4:0] + 1) into D.
 *
 * EEEE 1001110 01I DDDDDDDDD SSSSSSSSS
 *
 * BMASK   D,{#}S
 *
 * D = ($0000_0002 << S[4:0]) - 1.
 */
uint P2Cog::op_bmask()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Get LSB-justified bit mask of size (D[4:0] + 1) into D.
 *
 * EEEE 1001110 010 DDDDDDDDD DDDDDDDDD
 *
 * BMASK   D
 *
 * D = ($0000_0002 << D[4:0]) - 1.
 */
uint P2Cog::op_bmask_d()
{
    return 2;
}

/**
 * @brief Iterate CRC value in D using C and polynomial in S.
 *
 * EEEE 1001110 10I DDDDDDDDD SSSSSSSSS
 *
 * CRCBIT  D,{#}S
 *
 * If (C XOR D[0]) then D = (D >> 1) XOR S, else D = (D >> 1).
 */
uint P2Cog::op_crcbit()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Iterate CRC value in D using Q[31:28] and polynomial in S.
 *
 * EEEE 1001110 11I DDDDDDDDD SSSSSSSSS
 *
 * CRCNIB  D,{#}S
 *
 * Like CRCBIT, but 4x.
 * Q = Q << 4.
 * Use SETQ+CRCNIB+CRCNIB+CRCNIB.
 */
uint P2Cog::op_crcnib()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief For each non-zero bit pair in S, copy that bit pair into the corresponding D bits, else leave that D bit pair the same.
 *
 * EEEE 1001111 00I DDDDDDDDD SSSSSSSSS
 *
 * MUXNITS D,{#}S
 *
 */
uint P2Cog::op_muxnits()
{
    augmentS(IR.op.imm);
    const quint32 mask = S | ((S & 0xaaaaaaaa) >> 1) | ((S & 0x55555555) << 1);
    const quint32 result = (D & ~mask) | (S & mask);
    updateD(result);
    return 2;
}

/**
 * @brief For each non-zero nibble in S, copy that nibble into the corresponding D nibble, else leave that D nibble the same.
 *
 * EEEE 1001111 01I DDDDDDDDD SSSSSSSSS
 *
 * MUXNIBS D,{#}S
 *
 */
uint P2Cog::op_muxnibs()
{
    augmentS(IR.op.imm);
    const quint32 mask0 = S | ((S & 0xaaaaaaaa) >> 1) | ((S & 0x55555555) << 1);
    const quint32 mask1 = ((mask0 & 0xcccccccc) >> 2) | ((mask0 & 0x33333333) << 2);
    const quint32 result = (D & ~mask1) | (S & mask1);
    updateD(result);
    return 2;
}

/**
 * @brief Used after SETQ.
 *
 * EEEE 1001111 10I DDDDDDDDD SSSSSSSSS
 *
 * MUXQ    D,{#}S
 *
 * For each '1' bit in Q, copy the corresponding bit in S into D.
 * D = (D & !Q) | (S & Q).
 */
uint P2Cog::op_muxq()
{
    augmentS(IR.op.imm);
    const quint32 result = (D & ~Q) | (S & Q);
    updateD(result);
    return 2;
}

/**
 * @brief Move bytes within D, per S.
 *
 * EEEE 1001111 11I DDDDDDDDD SSSSSSSSS
 *
 * MOVBYTS D,{#}S
 *
 * D = {D.BYTE[S[7:6]], D.BYTE[S[5:4]], D.BYTE[S[3:2]], D.BYTE[S[1:0]]}.
 */
uint P2Cog::op_movbyts()
{
    augmentS(IR.op.imm);
    union {
        quint32 word;
        quint8 bytes[4];
    }   s, d;
    s.word = D;
    d.bytes[0] = s.bytes[(S >> 0) & 3];
    d.bytes[1] = s.bytes[(S >> 2) & 3];
    d.bytes[2] = s.bytes[(S >> 4) & 3];
    d.bytes[3] = s.bytes[(S >> 6) & 3];
    updateD(d.word);
    return 2;
}

/**
 * @brief D = unsigned (D[15:0] * S[15:0]).
 *
 * EEEE 1010000 0ZI DDDDDDDDD SSSSSSSSS
 *
 * MUL     D,{#}S          {WZ}
 *
 * Z = (S == 0) | (D == 0).
 */
uint P2Cog::op_mul()
{
    augmentS(IR.op.imm);
    const quint32 result = U16(D) * U16(S);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief D = signed (D[15:0] * S[15:0]).
 *
 * EEEE 1010000 1ZI DDDDDDDDD SSSSSSSSS
 *
 * MULS    D,{#}S          {WZ}
 *
 * Z = (S == 0) | (D == 0).
 */
uint P2Cog::op_muls()
{
    augmentS(IR.op.imm);
    const qint32 result = S16(D) * S16(S);
    updateZ(result == 0);
    updateD(result);
    return 2;
}

/**
 * @brief Next instruction's S value = unsigned (D[15:0] * S[15:0]) >> 16.
 *
 * EEEE 1010001 0ZI DDDDDDDDD SSSSSSSSS
 *
 * SCA     D,{#}S          {WZ}
 *
 * Z = (result == 0).
 */
uint P2Cog::op_sca()
{
    augmentS(IR.op.imm);
    const quint32 result = (U16(D) * U16(S)) >> 16;
    updateZ(result == 0);
    S_next = result;
    return 2;
}

/**
 * @brief Next instruction's S value = signed (D[15:0] * S[15:0]) >> 14.
 *
 * EEEE 1010001 1ZI DDDDDDDDD SSSSSSSSS
 *
 * SCAS    D,{#}S          {WZ}
 *
 * In this scheme, $4000 = 1.0 and $C000 = -1.0.
 * Z = (result == 0).
 */
uint P2Cog::op_scas()
{
    augmentS(IR.op.imm);
    const qint32 result = (S16(D) * S16(S)) >> 14;
    updateZ(result == 0);
    S_next = result;
    return 2;
}

/**
 * @brief Add bytes of S into bytes of D, with $FF saturation.
 *
 * EEEE 1010010 00I DDDDDDDDD SSSSSSSSS
 *
 * ADDPIX  D,{#}S
 *
 */
uint P2Cog::op_addpix()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Multiply bytes of S into bytes of D, where $FF = 1.
 *
 * EEEE 1010010 01I DDDDDDDDD SSSSSSSSS
 *
 * MULPIX  D,{#}S
 *
 * 0.
 */
uint P2Cog::op_mulpix()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Alpha-blend bytes of S into bytes of D, using SETPIV value.
 *
 * EEEE 1010010 10I DDDDDDDDD SSSSSSSSS
 *
 * BLNPIX  D,{#}S
 *
 */
uint P2Cog::op_blnpix()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Mix bytes of S into bytes of D, using SETPIX and SETPIV values.
 *
 * EEEE 1010010 11I DDDDDDDDD SSSSSSSSS
 *
 * MIXPIX  D,{#}S
 *
 */
uint P2Cog::op_mixpix()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Set CT1 event to trigger on CT = D + S.
 *
 * EEEE 1010011 00I DDDDDDDDD SSSSSSSSS
 *
 * ADDCT1  D,{#}S
 *
 * Adds S into D.
 */
uint P2Cog::op_addct1()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Set CT2 event to trigger on CT = D + S.
 *
 * EEEE 1010011 01I DDDDDDDDD SSSSSSSSS
 *
 * ADDCT2  D,{#}S
 *
 * Adds S into D.
 */
uint P2Cog::op_addct2()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Set CT3 event to trigger on CT = D + S.
 *
 * EEEE 1010011 10I DDDDDDDDD SSSSSSSSS
 *
 * ADDCT3  D,{#}S
 *
 * Adds S into D.
 */
uint P2Cog::op_addct3()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Write only non-$00 bytes in D[31:0] to hub address {#}S/PTRx.
 *
 * EEEE 1010011 11I DDDDDDDDD SSSSSSSSS
 *
 * WMLONG  D,{#}S/P
 *
 * Prior SETQ/SETQ2 invokes cog/LUT block transfer.
 */
uint P2Cog::op_wmlong()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Read smart pin S[5:0] result "Z" into D, don't acknowledge smart pin ("Q" in RQPIN means "quiet").
 *
 * EEEE 1010100 C0I DDDDDDDDD SSSSSSSSS
 *
 * RQPIN   D,{#}S          {WC}
 *
 * C = modal result.
 */
uint P2Cog::op_rqpin()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Read smart pin S[5:0] result "Z" into D, acknowledge smart pin.
 *
 * EEEE 1010100 C1I DDDDDDDDD SSSSSSSSS
 *
 * RDPIN   D,{#}S          {WC}
 *
 * C = modal result.
 */
uint P2Cog::op_rdpin()
{
    augmentS(IR.op.imm);
    return 2;
}

/**
 * @brief Read LUT data from address S[8:0] into D.
 *
 * EEEE 1010101 CZI DDDDDDDDD SSSSSSSSS
 *
 * RDLUT   D,{#}S   {WC/WZ/WCZ}
 *
 * C = MSB of data.
 * Z = (result == 0).
 */
uint P2Cog::op_rdlut()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_rdbyte()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_rdword()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_rdlong()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_popa()
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
uint P2Cog::op_popb()
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
uint P2Cog::op_calld()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_resi3()
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
uint P2Cog::op_resi2()
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
uint P2Cog::op_resi1()
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
uint P2Cog::op_resi0()
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
uint P2Cog::op_reti3()
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
uint P2Cog::op_reti2()
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
uint P2Cog::op_reti1()
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
uint P2Cog::op_reti0()
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
uint P2Cog::op_callpa()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_callpb()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_djz()
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
uint P2Cog::op_djnz()
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
uint P2Cog::op_djf()
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
uint P2Cog::op_djnf()
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
uint P2Cog::op_ijz()
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
uint P2Cog::op_ijnz()
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
uint P2Cog::op_tjz()
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
uint P2Cog::op_tjnz()
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
uint P2Cog::op_tjf()
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
uint P2Cog::op_tjnf()
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
uint P2Cog::op_tjs()
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
uint P2Cog::op_tjns()
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
uint P2Cog::op_tjv()
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
uint P2Cog::op_jint()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jct1()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jct2()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jct3()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jse1()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jse2()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jse3()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jse4()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jpat()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jfbw()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jxmt()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jxfi()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jxro()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jxrl()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jatn()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jqmt()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jnint()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jnct1()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jnct2()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jnct3()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jnse1()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jnse2()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jnse3()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jnse4()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jnpat()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jnfbw()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jnxmt()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jnxfi()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jnxro()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jnxrl()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jnatn()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_jnqmt()
{
    augmentS(IR.op.imm);
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
uint P2Cog::op_1011110_1()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_1011111_0()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
    return 2;
}

/**
 * @brief Set pin pattern for PAT event.
 *
 * EEEE 1011111 1LI DDDDDDDDD SSSSSSSSS
 *
 * SETPAT  {#}D,{#}S
 *
 * C selects INA/INB, Z selects =/!=, D provides mask value, S provides match value.
 */
uint P2Cog::op_setpat()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_wrpin()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_akpin()
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
uint P2Cog::op_wxpin()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_wypin()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_wrlut()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_wrbyte()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_wrword()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_wrlong()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_pusha()
{
    augmentD(IR.op.uz);
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
uint P2Cog::op_pushb()
{
    augmentD(IR.op.uz);
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
uint P2Cog::op_rdfast()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_wrfast()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_fblock()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_xinit()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_xstop()
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
uint P2Cog::op_xzero()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_xcont()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_rep()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_coginit()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_qmul()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_qdiv()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_qfrac()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_qsqrt()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_qrotate()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_qvector()
{
    augmentS(IR.op.imm);
    augmentD(IR.op.uz);
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
uint P2Cog::op_hubset()
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
uint P2Cog::op_cogid()
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
uint P2Cog::op_cogstop()
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
uint P2Cog::op_locknew()
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
uint P2Cog::op_lockret()
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
uint P2Cog::op_locktry()
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
uint P2Cog::op_lockrel()
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
uint P2Cog::op_qlog()
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
uint P2Cog::op_qexp()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Used after RDFAST.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000010000
 *
 * RFBYTE  D        {WC/WZ/WCZ}
 *
 * Read zero-extended byte from FIFO into D.
 * C = MSB of byte.
 * Z = (result == 0).
 */
uint P2Cog::op_rfbyte()
{
    return 2;
}

/**
 * @brief Used after RDFAST.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000010001
 *
 * RFWORD  D        {WC/WZ/WCZ}
 *
 * Read zero-extended word from FIFO into D.
 * C = MSB of word.
 * Z = (result == 0).
 */
uint P2Cog::op_rfword()
{
    return 2;
}

/**
 * @brief Used after RDFAST.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000010010
 *
 * RFLONG  D        {WC/WZ/WCZ}
 *
 * Read long from FIFO into D.
 * C = MSB of long.
 * Z = (result == 0).
 */
uint P2Cog::op_rflong()
{
    return 2;
}

/**
 * @brief Used after RDFAST.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000010011
 *
 * RFVAR   D        {WC/WZ/WCZ}
 *
 * Read zero-extended 1.
 * 4-byte value from FIFO into D.
 * C = 0.
 * Z = (result == 0).
 */
uint P2Cog::op_rfvar()
{
    return 2;
}

/**
 * @brief Used after RDFAST.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000010100
 *
 * RFVARS  D        {WC/WZ/WCZ}
 *
 * Read sign-extended 1.
 * 4-byte value from FIFO into D.
 * C = MSB of value.
 * Z = (result == 0).
 */
uint P2Cog::op_rfvars()
{
    return 2;
}

/**
 * @brief Used after WRFAST.
 *
 * EEEE 1101011 00L DDDDDDDDD 000010101
 *
 * WFBYTE  {#}D
 *
 * Write byte in D[7:0] into FIFO.
 */
uint P2Cog::op_wfbyte()
{
    return 2;
}

/**
 * @brief Used after WRFAST.
 *
 * EEEE 1101011 00L DDDDDDDDD 000010110
 *
 * WFWORD  {#}D
 *
 * Write word in D[15:0] into FIFO.
 */
uint P2Cog::op_wfword()
{
    return 2;
}

/**
 * @brief Used after WRFAST.
 *
 * EEEE 1101011 00L DDDDDDDDD 000010111
 *
 * WFLONG  {#}D
 *
 * Write long in D[31:0] into FIFO.
 */
uint P2Cog::op_wflong()
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
uint P2Cog::op_getqx()
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
uint P2Cog::op_getqy()
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
uint P2Cog::op_getct()
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
uint P2Cog::op_getrnd()
{
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
uint P2Cog::op_getrnd_cz()
{
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
uint P2Cog::op_setdacs()
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
uint P2Cog::op_setxfrq()
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
uint P2Cog::op_getxacc()
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
uint P2Cog::op_waitx()
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
uint P2Cog::op_setse1()
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
uint P2Cog::op_setse2()
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
uint P2Cog::op_setse3()
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
uint P2Cog::op_setse4()
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
uint P2Cog::op_pollint()
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
uint P2Cog::op_pollct1()
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
uint P2Cog::op_pollct2()
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
uint P2Cog::op_pollct3()
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
uint P2Cog::op_pollse1()
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
uint P2Cog::op_pollse2()
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
uint P2Cog::op_pollse3()
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
uint P2Cog::op_pollse4()
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
uint P2Cog::op_pollpat()
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
uint P2Cog::op_pollfbw()
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
uint P2Cog::op_pollxmt()
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
uint P2Cog::op_pollxfi()
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
uint P2Cog::op_pollxro()
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
uint P2Cog::op_pollxrl()
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
uint P2Cog::op_pollatn()
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
uint P2Cog::op_pollqmt()
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
uint P2Cog::op_waitint()
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
uint P2Cog::op_waitct1()
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
uint P2Cog::op_waitct2()
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
uint P2Cog::op_waitct3()
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
uint P2Cog::op_waitse1()
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
uint P2Cog::op_waitse2()
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
uint P2Cog::op_waitse3()
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
uint P2Cog::op_waitse4()
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
uint P2Cog::op_waitpat()
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
uint P2Cog::op_waitfbw()
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
uint P2Cog::op_waitxmt()
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
uint P2Cog::op_waitxfi()
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
uint P2Cog::op_waitxro()
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
uint P2Cog::op_waitxrl()
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
uint P2Cog::op_waitatn()
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
uint P2Cog::op_allowi()
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
uint P2Cog::op_stalli()
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
uint P2Cog::op_trgint1()
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
uint P2Cog::op_trgint2()
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
uint P2Cog::op_trgint3()
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
uint P2Cog::op_nixint1()
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
uint P2Cog::op_nixint2()
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
uint P2Cog::op_nixint3()
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
uint P2Cog::op_setint1()
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
uint P2Cog::op_setint2()
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
uint P2Cog::op_setint3()
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
uint P2Cog::op_setq()
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
uint P2Cog::op_setq2()
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
uint P2Cog::op_push()
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
uint P2Cog::op_pop()
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
uint P2Cog::op_jmp()
{
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
uint P2Cog::op_call()
{
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
uint P2Cog::op_ret()
{
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
uint P2Cog::op_calla()
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
uint P2Cog::op_reta()
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
uint P2Cog::op_callb()
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
uint P2Cog::op_retb()
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
uint P2Cog::op_jmprel()
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
uint P2Cog::op_skip()
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
uint P2Cog::op_skipf()
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
uint P2Cog::op_execf()
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
uint P2Cog::op_getptr()
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
uint P2Cog::op_getbrk()
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
uint P2Cog::op_cogbrk()
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
uint P2Cog::op_brk()
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
uint P2Cog::op_setluts()
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
uint P2Cog::op_setcy()
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
uint P2Cog::op_setci()
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
uint P2Cog::op_setcq()
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
uint P2Cog::op_setcfrq()
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
uint P2Cog::op_setcmod()
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
uint P2Cog::op_setpiv()
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
uint P2Cog::op_setpix()
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
uint P2Cog::op_cogatn()
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
uint P2Cog::op_testp_w()
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
uint P2Cog::op_testpn_w()
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
uint P2Cog::op_testp_and()
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
uint P2Cog::op_testpn_and()
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
uint P2Cog::op_testp_or()
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
uint P2Cog::op_testpn_or()
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
uint P2Cog::op_testp_xor()
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
uint P2Cog::op_testpn_xor()
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
uint P2Cog::op_dirl()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_dirh()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_dirc()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_dirnc()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_dirz()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_dirnz()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_dirrnd()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_dirnot()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_outl()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_outh()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_outc()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_outnc()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_outz()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_outnz()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_outrnd()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_outnot()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_fltl()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_flth()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_fltc()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_fltnc()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_fltz()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_fltnz()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_fltrnd()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_fltnot()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_drvl()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_drvh()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_drvc()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_drvnc()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_drvz()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_drvnz()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_drvrnd()
{
    augmentD(IR.op.imm);
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
uint P2Cog::op_drvnot()
{
    augmentD(IR.op.imm);
    return 2;
}

/**
 * @brief Split every 4th bit of S into bytes of D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100000
 *
 * SPLITB  D
 *
 * D = {S[31], S[27], S[23], S[19], ... S[12], S[8], S[4], S[0]}.
 */
uint P2Cog::op_splitb()
{
    return 2;
}

/**
 * @brief Merge bits of bytes in S into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100001
 *
 * MERGEB  D
 *
 * D = {S[31], S[23], S[15], S[7], ... S[24], S[16], S[8], S[0]}.
 */
uint P2Cog::op_mergeb()
{
    return 2;
}

/**
 * @brief Split bits of S into words of D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100010
 *
 * SPLITW  D
 *
 * D = {S[31], S[29], S[27], S[25], ... S[6], S[4], S[2], S[0]}.
 */
uint P2Cog::op_splitw()
{
    return 2;
}

/**
 * @brief Merge bits of words in S into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100011
 *
 * MERGEW  D
 *
 * D = {S[31], S[15], S[30], S[14], ... S[17], S[1], S[16], S[0]}.
 */
uint P2Cog::op_mergew()
{
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
uint P2Cog::op_seussf()
{
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
uint P2Cog::op_seussr()
{
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
uint P2Cog::op_rgbsqz()
{
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
uint P2Cog::op_rgbexp()
{
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
uint P2Cog::op_xoro32()
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
uint P2Cog::op_rev()
{
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
uint P2Cog::op_rczr()
{
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
uint P2Cog::op_rczl()
{
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
uint P2Cog::op_wrc()
{
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
uint P2Cog::op_wrnc()
{
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
uint P2Cog::op_wrz()
{
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
uint P2Cog::op_wrnz()
{
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
uint P2Cog::op_modcz()
{
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
uint P2Cog::op_jmp_abs()
{
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
uint P2Cog::op_call_abs()
{
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
uint P2Cog::op_calla_abs()
{
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
uint P2Cog::op_callb_abs()
{
    return 2;
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 11100WW RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PA/PB/PTRA/PTRB,#A
 *
 * If R = 1, PC += A, else PC = A.
 */
uint P2Cog::op_calld_abs()
{
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
uint P2Cog::op_loc_pa()
{
    const quint32 A = IR.word & ADDR;
    const quint32 addr = IR.op.uc ? PC + A : A;
    updatePA(addr);
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
uint P2Cog::op_loc_pb()
{
    const quint32 A = IR.word & ADDR;
    const quint32 addr = IR.op.uc ? PC + A : A;
    updatePB(addr);
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
uint P2Cog::op_loc_ptra()
{
    const quint32 A = IR.word & ADDR;
    const quint32 addr = IR.op.uc ? PC + A : A;
    updatePTRA(addr);
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
uint P2Cog::op_loc_ptrb()
{
    const quint32 A = IR.word & ADDR;
    const quint32 addr = IR.op.uc ? PC + A : A;
    updatePTRB(addr);
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
uint P2Cog::op_augs()
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
uint P2Cog::op_augd()
{
    D_aug = (IR.word << 9) & AUG;
    return 2;
}
