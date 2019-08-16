/****************************************************************************
 *
 * Propeller2 disassembler
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
#include "p2defs.h"
#include "p2cog.h"
#include "p2dasm.h"

P2Dasm::P2Dasm(const P2Cog* cog, QObject* parent)
    : QObject(parent)
    , COG(cog)
    , Token(new P2Token())
    , pad_opcode(40)
    , pad_inst(-10)
    , pad_wcz(24)
    , m_opcode()
    , m_string()
    , IR()
    , S(0)
    , D(0)
{
}

P2Dasm::~P2Dasm()
{
    delete Token;
}

p2_token_e P2Dasm::conditional(p2_cond_e cond)
{
    switch (cond) {
    case cond__ret_:        // execute always
        return t__RET_;
    case cond_nc_and_nz:    // execute if C = 0 and Z = 0
        return t_IF_NC_AND_NZ;
    case cond_nc_and_z:     // execute if C = 0 and Z = 1
        return t_IF_NC_AND_Z;
    case cond_nc:           // execute if C = 0
        return t_IF_NC;
    case cond_c_and_nz:     // execute if C = 1 and Z = 0
        return t_IF_C_AND_NZ;
    case cond_nz:           // execute if Z = 0
        return t_IF_NZ;
    case cond_c_ne_z:       // execute if C != Z
        return t_IF_C_NE_Z;
    case cond_nc_or_nz:     // execute if C = 0 or Z = 0
        return t_IF_NC_OR_NZ;
    case cond_c_and_z:      // execute if C = 1 and Z = 1
        return t_IF_C_AND_Z;
    case cond_c_eq_z:       // execute if C = Z
        return t_IF_C_EQ_Z;
    case cond_z:            // execute if Z = 1
        return t_IF_Z;
    case cond_nc_or_z:      // execute if C = 0 or Z = 1
        return t_IF_NC_OR_Z;
    case cond_c:            // execute if C = 1
        return t_IF_C;
    case cond_c_or_nz:      // execute if C = 1 or Z = 0
        return t_IF_C_OR_NZ;
    case cond_c_or_z:       // execute if C = 1 or Z = 1
        return t_IF_C_OR_Z;
    case cond_always:
        return t_ALWAYS;
    }
    return t_invalid;
}

p2_token_e P2Dasm::conditional(unsigned cond)
{
    return conditional(static_cast<p2_cond_e>(cond));
}

bool P2Dasm::dasm(p2_LONG PC, QString& opcode, QString& instruction)
{
    QString cond;

    IR.word = COG->rd_mem(PC*4);
    PC++;               // increment PC
    S = COG->rd_cog(IR.op.src);
    D = COG->rd_cog(IR.op.dst);

    // check for the condition
    cond = Token->str(conditional(IR.op.cond));

    m_opcode = QString("%1_%2_%3%4%5_%6_%7")
               .arg(format_bin(IR.op.cond, 4))
               .arg(format_bin(IR.op.inst, 7))
               .arg(format_bin(IR.op.wc, 1))
               .arg(format_bin(IR.op.wz, 1))
               .arg(format_bin(IR.op.imm, 1))
               .arg(format_bin(IR.op.dst, 9))
               .arg(format_bin(IR.op.src, 9));
    m_opcode.resize(pad_opcode, QChar::Space);

    // Dispatch to dasm_xxx() functions
    switch (IR.op.inst) {
    case p2_ror:
        dasm_ror();
        break;

    case p2_rol:
        dasm_rol();
        break;

    case p2_shr:
        dasm_shr();
        break;

    case p2_shl:
        dasm_shl();
        break;

    case p2_rcr:
        dasm_rcr();
        break;

    case p2_rcl:
        dasm_rcl();
        break;

    case p2_sar:
        dasm_sar();
        break;

    case p2_sal:
        dasm_sal();
        break;

    case p2_add:
        dasm_add();
        break;

    case p2_addx:
        dasm_addx();
        break;

    case p2_adds:
        dasm_adds();
        break;

    case p2_addsx:
        dasm_addsx();
        break;

    case p2_sub:
        dasm_sub();
        break;

    case p2_subx:
        dasm_subx();
        break;

    case p2_subs:
        dasm_subs();
        break;

    case p2_subsx:
        dasm_subsx();
        break;

    case p2_cmp:
        dasm_cmp();
        break;

    case p2_cmpx:
        dasm_cmpx();
        break;

    case p2_cmps:
        dasm_cmps();
        break;

    case p2_cmpsx:
        dasm_cmpsx();
        break;

    case p2_cmpr:
        dasm_cmpr();
        break;

    case p2_cmpm:
        dasm_cmpm();
        break;

    case p2_subr:
        dasm_subr();
        break;

    case p2_cmpsub:
        dasm_cmpsub();
        break;

    case p2_fge:
        dasm_fge();
        break;

    case p2_fle:
        dasm_fle();
        break;

    case p2_fges:
        dasm_fges();
        break;

    case p2_fles:
        dasm_fles();
        break;

    case p2_sumc:
        dasm_sumc();
        break;

    case p2_sumnc:
        dasm_sumnc();
        break;

    case p2_sumz:
        dasm_sumz();
        break;

    case p2_sumnz:
        dasm_sumnz();
        break;

    case p2_testb_w:
        // case p2_bitl:
        (IR.op.wc != IR.op.wz) ? dasm_testb_w()
                                        : dasm_bitl();
        break;

    case p2_testbn_w:
        // case p2_bith:
        (IR.op.wc != IR.op.wz) ? dasm_testbn_w()
                                        : dasm_bith();
        break;

    case p2_testb_and:
        // case p2_bitc:
        (IR.op.wc != IR.op.wz) ? dasm_testb_and()
                                        : dasm_bitc();
        break;

    case p2_testbn_and:
        // case p2_bitnc:
        (IR.op.wc != IR.op.wz) ? dasm_testbn_and()
                                        : dasm_bitnc();
        break;

    case p2_testb_or:
        // case p2_bitz:
        (IR.op.wc != IR.op.wz) ? dasm_testb_or()
                                        : dasm_bitz();
        break;

    case p2_testbn_or:
        // case p2_bitnz:
        (IR.op.wc != IR.op.wz) ? dasm_testbn_or()
                                        : dasm_bitnz();
        break;

    case p2_testb_xor:
        // case p2_bitrnd:
        (IR.op.wc != IR.op.wz) ? dasm_testb_xor()
                                        : dasm_bitrnd();
        break;

    case p2_testbn_xor:
        // case p2_bitnot:
        (IR.op.wc != IR.op.wz) ? dasm_testbn_xor()
                                        : dasm_bitnot();
        break;

    case p2_and:
        dasm_and();
        break;

    case p2_andn:
        dasm_andn();
        break;

    case p2_or:
        dasm_or();
        break;

    case p2_xor:
        dasm_xor();
        break;

    case p2_muxc:
        dasm_muxc();
        break;

    case p2_muxnc:
        dasm_muxnc();
        break;

    case p2_muxz:
        dasm_muxz();
        break;

    case p2_muxnz:
        dasm_muxnz();
        break;

    case p2_mov:
        dasm_mov();
        break;

    case p2_not:
        dasm_not();
        break;

    case p2_abs:
        dasm_abs();
        break;

    case p2_neg:
        dasm_neg();
        break;

    case p2_negc:
        dasm_negc();
        break;

    case p2_negnc:
        dasm_negnc();
        break;

    case p2_negz:
        dasm_negz();
        break;

    case p2_negnz:
        dasm_negnz();
        break;

    case p2_incmod:
        dasm_incmod();
        break;

    case p2_decmod:
        dasm_decmod();
        break;

    case p2_zerox:
        dasm_zerox();
        break;

    case p2_signx:
        dasm_signx();
        break;

    case p2_encod:
        dasm_encod();
        break;

    case p2_ones:
        dasm_ones();
        break;

    case p2_test:
        dasm_test();
        break;

    case p2_testn:
        dasm_testn();
        break;

    case p2_setnib_0:
    case p2_setnib_1:
        dasm_setnib();
        break;

    case p2_getnib_0:
    case p2_getnib_1:
        dasm_getnib();
        break;

    case p2_rolnib_0:
    case p2_rolnib_1:
        dasm_rolnib();
        break;

    case p2_setbyte:
        dasm_setbyte();
        break;

    case p2_getbyte:
        dasm_getbyte();
        break;

    case p2_rolbyte:
        dasm_rolbyte();
        break;

    case p2_1001001:
        if (IR.op.wc == 0) {
            (IR.op.dst == 0 && IR.op.wz == 0) ? dasm_setword_altsw()
                                                       : dasm_setword();
        } else {
            (IR.op.src == 0 && IR.op.wz == 0) ? dasm_getword_altgw()
                                                       : dasm_getword();
        }
        break;

    case p2_1001010:
        if (IR.op.wc == 0) {
            (IR.op.src == 0 && IR.op.wz == 0) ? dasm_rolword_altgw()
                                                       : dasm_rolword();
        } else {
            if (IR.op.wz == 0) {
                (IR.op.src == 0 && IR.op.imm == 1) ? dasm_altsn_d()
                                                            : dasm_altsn();
            } else {
                (IR.op.src == 0 && IR.op.imm == 1) ? dasm_altgn_d()
                                                            : dasm_altgn();
            }
        }
        break;

    case p2_1001011:
        if (IR.op.wc == 0) {
            if (IR.op.wz == 0) {
                (IR.op.src == 0 && IR.op.imm == 1) ? dasm_altsb_d()
                                                            : dasm_altsb();
            } else {
                (IR.op.src == 0 && IR.op.imm == 1) ? dasm_altgb_d()
                                                            : dasm_altgb();
            }
        } else {
            if (IR.op.wz == 0) {
                (IR.op.src == 0 && IR.op.imm == 1) ? dasm_altsw_d()
                                                            : dasm_altsw();
            } else {
                (IR.op.src == 0 && IR.op.imm == 1) ? dasm_altgw_d()
                                                            : dasm_altgw();
            }
        }
        break;

    case p2_1001100:
        if (IR.op.wc == 0) {
            if (IR.op.wz == 0) {
                (IR.op.src == 0 && IR.op.imm == 1) ? dasm_altr_d()
                                                            : dasm_altr();
            } else {
                (IR.op.src == 0 && IR.op.imm == 1) ? dasm_altd_d()
                                                            : dasm_altd();
            }
        } else {
            if (IR.op.wz == 0) {
                (IR.op.src == 0 && IR.op.imm == 1) ? dasm_alts_d()
                                                            : dasm_alts();
            } else {
                (IR.op.src == 0 && IR.op.imm == 1) ? dasm_altb_d()
                                                            : dasm_altb();
            }
        }
        break;

    case p2_1001101:
        if (IR.op.wc == 0) {
            if (IR.op.wz == 0) {
                (IR.op.imm == 1 && IR.op.src == 0x164 /* 101100100 */) ? dasm_alti_d()
                                                                                : dasm_alti();
            } else {
                dasm_setr();
            }
        } else {
            (IR.op.wz == 0) ? dasm_setd()
                                     : dasm_sets();
        }
        break;

    case p2_1001110:
        if (IR.op.wc == 0) {
            if (IR.op.wz == 0) {
                (IR.op.imm == 0 && IR.op.src == IR.op.dst) ? dasm_decod_d()
                                                                    : dasm_decod();
            } else {
                (IR.op.imm == 0 && IR.op.src == IR.op.dst) ? dasm_bmask_d()
                                                                    : dasm_bmask();
            }
        } else {
            if (IR.op.wz == 0) {
                dasm_crcbit();
            } else {
                dasm_crcnib();
            }
        }
        break;

    case p2_1001111:
        if (IR.op.wc == 0) {
            if (IR.op.wz == 0) {
                dasm_muxnits();
            } else {
                dasm_muxnibs();
            }
        } else {
            if (IR.op.wz == 0) {
                dasm_muxq();
            } else {
                dasm_movbyts();
            }
        }
        break;

    case p2_1010000:
        if (IR.op.wc == 0) {
            dasm_mul();
        } else {
            dasm_muls();
        }
        break;

    case p2_1010001:
        if (IR.op.wc == 0) {
            dasm_sca();
        } else {
            dasm_scas();
        }
        break;

    case p2_1010010:
        if (IR.op.wc == 0) {
            if (IR.op.wz == 0) {
                dasm_addpix();
            } else {
                dasm_mulpix();
            }
        } else {
            if (IR.op.wz == 0) {
                dasm_blnpix();
            } else {
                dasm_mixpix();
            }
        }
        break;

    case p2_1010011:
        if (IR.op.wc == 0) {
            if (IR.op.wz == 0) {
                dasm_addct1();
            } else {
                dasm_addct2();
            }
        } else {
            if (IR.op.wz == 0) {
                dasm_addct3();
            } else {
                dasm_wmlong();
            }
        }
        break;

    case p2_1010100:
        if (IR.op.wz == 0) {
            dasm_rqpin();
        } else {

        }
        break;

    case p2_rdlut:
        dasm_rdlut();
        break;

    case p2_rdbyte:
        dasm_rdbyte();
        break;

    case p2_rdword:
        dasm_rdword();
        break;

    case p2_rdlong:
        dasm_rdlong();
        break;

    case p2_calld:
        dasm_calld();
        break;

    case p2_callp:
        (IR.op.wc == 0) ? dasm_callpa() : dasm_callpb();
        break;

    case p2_1011011:
        if (IR.op.wc == 0) {
            if (IR.op.wz == 0) {
                dasm_djz();
            } else {
                dasm_djnz();
            }
        } else {
            if (IR.op.wz == 0) {
                dasm_djf();
            } else {
                dasm_djnf();
            }
        }
        break;

    case p2_1011100:
        if (IR.op.wc == 0) {
            if (IR.op.wz == 0) {
                dasm_ijz();
            } else {
                dasm_ijnz();
            }
        } else {
            if (IR.op.wz == 0) {
                dasm_tjz();
            } else {
                dasm_tjnz();
            }
        }
        break;

    case p2_1011101:
        if (IR.op.wc == 0) {
            if (IR.op.wz == 0) {
                dasm_tjf();
            } else {
                dasm_tjnf();
            }
        } else {
            if (IR.op.wz == 0) {
                dasm_tjs();
            } else {
                dasm_tjns();
            }
        }
        break;

    case p2_1011110:
        if (IR.op.wc == 0) {
            if (IR.op.wz == 0) {
                dasm_tjv();
            } else {
                switch (IR.op.dst) {
                case 0x00:
                    dasm_jint();
                    break;
                case 0x01:
                    dasm_jct1();
                    break;
                case 0x02:
                    dasm_jct2();
                    break;
                case 0x03:
                    dasm_jct3();
                    break;
                case 0x04:
                    dasm_jse1();
                    break;
                case 0x05:
                    dasm_jse2();
                    break;
                case 0x06:
                    dasm_jse3();
                    break;
                case 0x07:
                    dasm_jse4();
                    break;
                case 0x08:
                    dasm_jpat();
                    break;
                case 0x09:
                    dasm_jfbw();
                    break;
                case 0x0a:
                    dasm_jxmt();
                    break;
                case 0x0b:
                    dasm_jxfi();
                    break;
                case 0x0c:
                    dasm_jxro();
                    break;
                case 0x0d:
                    dasm_jxrl();
                    break;
                case 0x0e:
                    dasm_jatn();
                    break;
                case 0x0f:
                    dasm_jqmt();
                    break;
                case 0x10:
                    dasm_jnint();
                    break;
                case 0x11:
                    dasm_jnct1();
                    break;
                case 0x12:
                    dasm_jnct2();
                    break;
                case 0x13:
                    dasm_jnct3();
                    break;
                case 0x14:
                    dasm_jnse1();
                    break;
                case 0x15:
                    dasm_jnse2();
                    break;
                case 0x16:
                    dasm_jnse3();
                    break;
                case 0x17:
                    dasm_jnse4();
                    break;
                case 0x18:
                    dasm_jnpat();
                    break;
                case 0x19:
                    dasm_jnfbw();
                    break;
                case 0x1a:
                    dasm_jnxmt();
                    break;
                case 0x1b:
                    dasm_jnxfi();
                    break;
                case 0x1c:
                    dasm_jnxro();
                    break;
                case 0x1d:
                    dasm_jnxrl();
                    break;
                case 0x1e:
                    dasm_jnatn();
                    break;
                case 0x1f:
                    dasm_jnqmt();
                    break;
                default:
                    // TODO: invalid D value
                    break;
                }
            }
        } else {
            dasm_1011110_1();
        }
        break;

    case p2_1011111:
        if (IR.op.wc == 0) {
            dasm_1011111_0();
        } else {
            dasm_setpat();
        }
        break;

    case p2_1100000:
        if (IR.op.wc == 0) {
            (IR.op.wz == 1 && IR.op.dst == 1) ? dasm_akpin()
                                                       : dasm_wrpin();
        } else {
            dasm_wxpin();
        }
        break;

    case p2_1100001:
        if (IR.op.wc == 0) {
            dasm_wypin();
        } else {
            dasm_wrlut();
        }
        break;

    case p2_1100010:
        if (IR.op.wc == 0) {
            dasm_wrbyte();
        } else {
            dasm_wrword();
        }
        break;

    case p2_1100011:
        if (IR.op.wc == 0) {
            dasm_wrlong();
        } else {
            dasm_rdfast();
        }
        break;

    case p2_1100100:
        if (IR.op.wc == 0) {
            dasm_wrfast();
        } else {
            dasm_fblock();
        }
        break;

    case p2_1100101:
        if (IR.op.wc == 0) {
            if (IR.op.wz == 1 && IR.op.imm == 1 && IR.op.src == 0 && IR.op.dst == 0) {
                dasm_xstop();
            } else {
                dasm_xinit();
            }
        } else {
            dasm_xzero();
        }
        break;

    case p2_1100110:
        if (IR.op.wc == 0) {
            dasm_xcont();
        } else {
            dasm_rep();
        }
        break;

    case p2_coginit:
        dasm_coginit();
        break;

    case p2_1101000:
        if (IR.op.wc == 0) {
            dasm_qmul();
        } else {
            dasm_qdiv();
        }
        break;

    case p2_1101001:
        if (IR.op.wc == 0) {
            dasm_qfrac();
        } else {
            dasm_qsqrt();
        }
        break;

    case p2_1101010:
        if (IR.op.wc == 0) {
            dasm_qrotate();
        } else {
            dasm_qvector();
        }
        break;

    case p2_1101011:
        switch (IR.op.src) {
        case 0x00:
            dasm_hubset();
            break;
        case 0x01:
            dasm_cogid();
            break;
        case 0x03:
            dasm_cogstop();
            break;
        case 0x04:
            dasm_locknew();
            break;
        case 0x05:
            dasm_lockret();
            break;
        case 0x06:
            dasm_locktry();
            break;
        case 0x07:
            dasm_lockrel();
            break;
        case 0x0e:
            dasm_qlog();
            break;
        case 0x0f:
            dasm_qexp();
            break;
        case 0x10:
            dasm_rfbyte();
            break;
        case 0x11:
            dasm_rfword();
            break;
        case 0x12:
            dasm_rflong();
            break;
        case 0x13:
            dasm_rfvar();
            break;
        case 0x14:
            dasm_rfvars();
            break;
        case 0x15:
            dasm_wfbyte();
            break;
        case 0x16:
            dasm_wfword();
            break;
        case 0x17:
            dasm_wflong();
            break;
        case 0x18:
            dasm_getqx();
            break;
        case 0x19:
            dasm_getqy();
            break;
        case 0x1a:
            dasm_getct();
            break;
        case 0x1b:
            (IR.op.dst == 0) ? dasm_getrnd_cz()
                                      : dasm_getrnd();
            break;
        case 0x1c:
            dasm_setdacs();
            break;
        case 0x1d:
            dasm_setxfrq();
            break;
        case 0x1e:
            dasm_getxacc();
            break;
        case 0x1f:
            dasm_waitx();
            break;
        case 0x20:
            dasm_setse1();
            break;
        case 0x21:
            dasm_setse2();
            break;
        case 0x22:
            dasm_setse3();
            break;
        case 0x23:
            dasm_setse4();
            break;
        case 0x24:
            switch (IR.op.dst) {
            case 0x00:
                dasm_pollint();
                break;
            case 0x01:
                dasm_pollct1();
                break;
            case 0x02:
                dasm_pollct2();
                break;
            case 0x03:
                dasm_pollct3();
                break;
            case 0x04:
                dasm_pollse1();
                break;
            case 0x05:
                dasm_pollse2();
                break;
            case 0x06:
                dasm_pollse3();
                break;
            case 0x07:
                dasm_pollse4();
                break;
            case 0x08:
                dasm_pollpat();
                break;
            case 0x09:
                dasm_pollfbw();
                break;
            case 0x0a:
                dasm_pollxmt();
                break;
            case 0x0b:
                dasm_pollxfi();
                break;
            case 0x0c:
                dasm_pollxro();
                break;
            case 0x0d:
                dasm_pollxrl();
                break;
            case 0x0e:
                dasm_pollatn();
                break;
            case 0x0f:
                dasm_pollqmt();
                break;
            case 0x10:
                dasm_waitint();
                break;
            case 0x11:
                dasm_waitct1();
                break;
            case 0x12:
                dasm_waitct2();
                break;
            case 0x13:
                dasm_waitct3();
                break;
            case 0x14:
                dasm_waitse1();
                break;
            case 0x15:
                dasm_waitse2();
                break;
            case 0x16:
                dasm_waitse3();
                break;
            case 0x17:
                dasm_waitse4();
                break;
            case 0x18:
                dasm_waitpat();
                break;
            case 0x19:
                dasm_waitfbw();
                break;
            case 0x1a:
                dasm_waitxmt();
                break;
            case 0x1b:
                dasm_waitxfi();
                break;
            case 0x1c:
                dasm_waitxro();
                break;
            case 0x1d:
                dasm_waitxrl();
                break;
            case 0x1e:
                dasm_waitatn();
                break;
            case 0x20:
                dasm_allowi();
                break;
            case 0x21:
                dasm_stalli();
                break;
            case 0x22:
                dasm_trgint1();
                break;
            case 0x23:
                dasm_trgint2();
                break;
            case 0x24:
                dasm_trgint3();
                break;
            case 0x25:
                dasm_nixint1();
                break;
            case 0x26:
                dasm_nixint2();
                break;
            case 0x27:
                dasm_nixint3();
                break;
            }
            break;
        case 0x25:
            dasm_setint1();
            break;
        case 0x26:
            dasm_setint2();
            break;
        case 0x27:
            dasm_setint3();
            break;
        case 0x28:
            dasm_setq();
            break;
        case 0x29:
            dasm_setq2();
            break;
        case 0x2a:
            dasm_push();
            break;
        case 0x2b:
            dasm_pop();
            break;
        case 0x2c:
            dasm_jmp();
            break;
        case 0x2d:
            (IR.op.imm == 0) ? dasm_call()
                                      : dasm_ret();
            break;
        case 0x2e:
            (IR.op.imm == 0) ? dasm_calla()
                                      : dasm_reta();
            break;
        case 0x2f:
            (IR.op.imm == 0) ? dasm_callb()
                                      : dasm_retb();
            break;
        case 0x30:
            dasm_jmprel();
            break;
        case 0x31:
            dasm_skip();
            break;
        case 0x32:
            dasm_skipf();
            break;
        case 0x33:
            dasm_execf();
            break;
        case 0x34:
            dasm_getptr();
            break;
        case 0x35:
            (IR.op.wc == 0 && IR.op.wz == 0) ? dasm_cogbrk()
                                                      : dasm_getbrk();
            break;
        case 0x36:
            dasm_brk();
            break;
        case 0x37:
            dasm_setluts();
            break;
        case 0x38:
            dasm_setcy();
            break;
        case 0x39:
            dasm_setci();
            break;
        case 0x3a:
            dasm_setcq();
            break;
        case 0x3b:
            dasm_setcfrq();
            break;
        case 0x3c:
            dasm_setcmod();
            break;
        case 0x3d:
            dasm_setpiv();
            break;
        case 0x3e:
            dasm_setpix();
            break;
        case 0x3f:
            dasm_cogatn();
            break;
        case 0x40:
            (IR.op.wc == IR.op.wz) ? dasm_testp_w()
                                            : dasm_dirl();
            break;
        case 0x41:
            (IR.op.wc == IR.op.wz) ? dasm_testpn_w()
                                            : dasm_dirh();
            break;
        case 0x42:
            (IR.op.wc == IR.op.wz) ? dasm_testp_and()
                                            : dasm_dirc();
            break;
        case 0x43:
            (IR.op.wc == IR.op.wz) ? dasm_testpn_and()
                                            : dasm_dirnc();
            break;
        case 0x44:
            (IR.op.wc == IR.op.wz) ? dasm_testp_or()
                                            : dasm_dirz();
            break;
        case 0x45:
            (IR.op.wc == IR.op.wz) ? dasm_testpn_or()
                                            : dasm_dirnz();
            break;
        case 0x46:
            (IR.op.wc == IR.op.wz) ? dasm_testp_xor()
                                            : dasm_dirrnd();
            break;
        case 0x47:
            (IR.op.wc == IR.op.wz) ? dasm_testpn_xor()
                                            : dasm_dirnot();
            break;
        case 0x48:
            dasm_outl();
            break;
        case 0x49:
            dasm_outh();
            break;
        case 0x4a:
            dasm_outc();
            break;
        case 0x4b:
            dasm_outnc();
            break;
        case 0x4c:
            dasm_outz();
            break;
        case 0x4d:
            dasm_outnz();
            break;
        case 0x4e:
            dasm_outrnd();
            break;
        case 0x4f:
            dasm_outnot();
            break;
        case 0x50:
            dasm_fltl();
            break;
        case 0x51:
            dasm_flth();
            break;
        case 0x52:
            dasm_fltc();
            break;
        case 0x53:
            dasm_fltnc();
            break;
        case 0x54:
            dasm_fltz();
            break;
        case 0x55:
            dasm_fltnz();
            break;
        case 0x56:
            dasm_fltrnd();
            break;
        case 0x57:
            dasm_fltnot();
            break;
        case 0x58:
            dasm_drvl();
            break;
        case 0x59:
            dasm_drvh();
            break;
        case 0x5a:
            dasm_drvc();
            break;
        case 0x5b:
            dasm_drvnc();
            break;
        case 0x5c:
            dasm_drvz();
            break;
        case 0x5d:
            dasm_drvnz();
            break;
        case 0x5e:
            dasm_drvrnd();
            break;
        case 0x5f:
            dasm_drvnot();
            break;
        case 0x60:
            dasm_splitb();
            break;
        case 0x61:
            dasm_mergeb();
            break;
        case 0x62:
            dasm_splitw();
            break;
        case 0x63:
            dasm_mergew();
            break;
        case 0x64:
            dasm_seussf();
            break;
        case 0x65:
            dasm_seussr();
            break;
        case 0x66:
            dasm_rgbsqz();
            break;
        case 0x67:
            dasm_rgbexp();
            break;
        case 0x68:
            dasm_xoro32();
            break;
        case 0x69:
            dasm_rev();
            break;
        case 0x6a:
            dasm_rczr();
            break;
        case 0x6b:
            dasm_rczl();
            break;
        case 0x6c:
            dasm_wrc();
            break;
        case 0x6d:
            dasm_wrnc();
            break;
        case 0x6e:
            dasm_wrz();
            break;
        case 0x6f:
            dasm_wrnz();
            break;
        case 0x7f:
            dasm_modcz();
            break;
        }
        break;

    case p2_jmp_abs:
        dasm_jmp_abs();
        break;

    case p2_call_abs:
        dasm_call_abs();
        break;

    case p2_calla_abs:
        dasm_calla_abs();
        break;

    case p2_callb_abs:
        dasm_callb_abs();
        break;

    case p2_calld_pa_abs:
    case p2_calld_pb_abs:
    case p2_calld_ptra_abs:
    case p2_calld_ptrb_abs:
        dasm_calld_abs();
        break;

    case p2_loc_pa:
        dasm_loc_pa();
        break;

    case p2_loc_pb:
        dasm_loc_pb();
        break;

    case p2_loc_ptra:
        dasm_loc_ptra();
        break;

    case p2_loc_ptrb:
        dasm_loc_ptrb();
        break;

    case p2_augs_00:
    case p2_augs_01:
    case p2_augs_10:
    case p2_augs_11:
        dasm_augs();
        break;

    case p2_augd_00:
    case p2_augd_01:
    case p2_augd_10:
    case p2_augd_11:
        dasm_augd();
        break;
    }

    instruction = QString("%1%2").arg(cond, -16).arg(m_string);
    opcode = m_opcode;

    // FIXME: return false for invalid instructions?
    return true;
}

p2_LONG P2Dasm::memsize() const
{
    P2Hub* hub = qobject_cast<P2Hub *>(COG->parent());
    if (!hub)
        return MEM_SIZE;
    return hub->memsize();
}

/**
 * @brief Format a number as either hex or binary
 * @param num number to format
 * @param binary if true, print binary constant
 * @return QString with $hex or %binary value
 */
QString P2Dasm::format_num(uint num, bool binary)
{
    return binary ? QString("%%%1").arg(num, 9, 2, QChar('0'))
                  : QString("$%1").arg(num, 3, 16, QChar('0'));
}

/**
 * @brief Format a number as binary with %digits left zero padded
 * @param num number to format
 * @param digits number of digits
 * @return QString with the binary representation
 */
QString P2Dasm::format_bin(uint num, int digits)
{
    return QString("%1").arg(num, digits, 2, QChar('0'));
}

/**
 * @brief format string for: EEEE xxxxxxx xxx xxxxxxxxx xxxxxxxxx
 *
 *  "INSTR"
 *
 * @param inst instruction token (mnemonic)
 */
void P2Dasm::format_inst(p2_token_e inst)
{
    m_string = Token->str(inst);
}

/**
 * @brief format string for: EEEE xxxxxxx CZI DDDDDDDDD SSSSSSSSS
 *
 *  "INSTR  D,{#}S  {WC,WZ,WCZ}"
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 */
void P2Dasm::format_d_imm_s_cz(p2_token_e inst, p2_token_e with)
{
    m_string = QString("%1%2,%3%4")
               .arg(Token->str(inst), pad_inst)
               .arg(format_num(IR.op.dst))
               .arg(IR.op.imm ? "#" : "")
               .arg(format_num(IR.op.src));

    if (IR.op.wc || IR.op.wz) {
        m_string.resize(pad_wcz, QChar::Space);
        if (IR.op.wc && IR.op.wz) {
            m_string += QString("%1%2")
                        .arg(Token->str(with))
                        .arg(Token->str(t_CZ));
        } else if (IR.op.wz) {
            m_string += QString("%1%2")
                        .arg(Token->str(with))
                        .arg(Token->str(t_Z));
        } else {
            m_string += QString("%1%2")
                        .arg(Token->str(with))
                        .arg(Token->str(t_C));
        }
    }
}

/**
 * @brief format string for: EEEE xxxxxxx C0I DDDDDDDDD SSSSSSSSS
 *
 *  "INSTR  D,{#}S  {WC}"
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 */
void P2Dasm::format_d_imm_s_c(p2_token_e inst, p2_token_e with)
{
    m_string = QString("%1%2,%3%4")
               .arg(Token->str(inst), pad_inst)
               .arg(format_num(IR.op.dst))
               .arg(IR.op.imm ? "#" : "")
               .arg(format_num(IR.op.src));

    if (IR.op.wc) {
        m_string.resize(pad_wcz);
        m_string += QString("%1%2")
                    .arg(Token->str(with))
                    .arg(Token->str(t_C));
    }
}

/**
 * @brief format string for: EEEE xxxxxxx 0ZI DDDDDDDDD SSSSSSSSS
 *
 *  "INSTR  D,{#}S  {WZ}"
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 */
void P2Dasm::format_d_imm_s_z(p2_token_e inst, p2_token_e with)
{
    m_string = QString("%1%2,%3%4")
               .arg(Token->str(inst), pad_inst)
               .arg(format_num(IR.op.dst))
               .arg(IR.op.imm ? "#" : "")                // I
               .arg(format_num(IR.op.src));

    if (IR.op.wz) {
        m_string.resize(pad_wcz, QChar::Space);
        m_string += QString("%1%2")
                    .arg(Token->str(with))
                    .arg(Token->str(t_Z));
    }
}

/**
 * @brief format string for: EEEE xxxxxxx 0LI DDDDDDDDD SSSSSSSSS
 *
 *  "INSTR  {#}D,{#}S"
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 */
void P2Dasm::format_wz_d_imm_s(p2_token_e inst)
{
    m_string = QString("%1%2%3,%4%5")
               .arg(Token->str(inst), pad_inst)
               .arg(IR.op.wz ? "#" : "")                   // L
               .arg(format_num(IR.op.dst))
               .arg(IR.op.imm ? "#" : "")                  // I
               .arg(format_num(IR.op.src));
}

/**
 * @brief format string for: EEEE xxxxxxN NNI DDDDDDDDD SSSSSSSSS
 *
 *  "INSTR  D,{#}S,#N"
 *
 * @param inst instruction token (mnemonic)
 */
void P2Dasm::format_d_imm_s_nnn(p2_token_e inst)
{
    m_string = QString("%1%2,%3%4,#%5")
               .arg(Token->str(inst), pad_inst)
               .arg(format_num(IR.op.dst))
               .arg(IR.op.imm ? "#" : "")
               .arg(format_num(IR.op.src))
               .arg((IR.op.inst & 1) * 4 + IR.op.wc * 2 + IR.op.wz);
}

/**
 * @brief format string for: EEEE xxxxxxx 0NI DDDDDDDDD SSSSSSSSS
 *
 *  "INSTR  D,{#}S,#N"
 *
 * @param inst instruction token (mnemonic)
 */
void P2Dasm::format_d_imm_s_n(p2_token_e inst)
{
    m_string = QString("%1%2,%3%4,#%5")
               .arg(Token->str(inst), pad_inst)
               .arg(format_num(IR.op.dst))
               .arg(IR.op.imm ? "#" : "")
               .arg(format_num(IR.op.src))
               .arg(IR.op.wz);
}

/**
 * @brief format string for: EEEE xxxxxxx xxI DDDDDDDDD SSSSSSSSS
 *
 *  "INSTR  D,{#}S"
 *
 * @param inst instruction token (mnemonic)
 */
void P2Dasm::format_d_imm_s(p2_token_e inst)
{
    m_string = QString("%1%2,%3%4")
               .arg(Token->str(inst), pad_inst)
               .arg(format_num(IR.op.dst))
               .arg(IR.op.imm ? "#" : "")
               .arg(format_num(IR.op.src));
}

/**
 * @brief format string for: EEEE xxxxxxx CZ0 DDDDDDDDD xxxxxxxxx
 *
 *  "INSTR  D       {WC,WZ,WCZ}"
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 */
void P2Dasm::format_d_cz(p2_token_e inst, p2_token_e with)
{
    m_string = QString("%1%2")
               .arg(Token->str(inst), pad_inst)
               .arg(format_num(IR.op.dst));

    if (IR.op.wc || IR.op.wz) {
        m_string.resize(pad_wcz, QChar::Space);
        if (IR.op.wc && IR.op.wz) {
            m_string += QString("%1%2")
                        .arg(Token->str(with))
                        .arg(Token->str(t_CZ));
        } else if (IR.op.wz) {
            m_string += QString("%1%2")
                        .arg(Token->str(with))
                        .arg(Token->str(t_Z));
        } else {
            m_string += QString("%1%2")
                        .arg(Token->str(with))
                        .arg(Token->str(t_C));
        }
    }
}

/**
 * @brief format string for: EEEE xxxxxxx CZx xxxxxxxxx xxxxxxxxx
 *
 *  "INSTR          {WC,WZ,WCZ}"
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 */
void P2Dasm::format_cz(p2_token_e inst, p2_token_e with)
{
    m_string = QString("%1")
               .arg(Token->str(inst), pad_inst);

    if (IR.op.wc || IR.op.wz) {
        m_string.resize(pad_wcz, QChar::Space);
        if (IR.op.wc && IR.op.wz) {
            m_string += QString("%1%2")
                        .arg(Token->str(with))
                        .arg(Token->str(t_CZ));
        } else if (IR.op.wz) {
            m_string += QString("%1%2")
                        .arg(Token->str(with))
                        .arg(Token->str(t_Z));
        } else {
            m_string += QString("%1%2")
                        .arg(Token->str(with))
                        .arg(Token->str(t_C));
        }
    }
}


/**
 * @brief format string for: EEEE xxxxxxx CZx 0cccczzzz xxxxxxxxx
 *
 *  "INSTR  cccc,zzzz {WC,WZ,WCZ}"
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 */
void P2Dasm::format_cz_cz(p2_token_e inst, p2_token_e with)
{
    const uint cccc = (IR.op.dst >> 4) & 15;
    const uint zzzz = (IR.op.dst >> 0) & 15;

    m_string = QString("%1%2,%3")
               .arg(Token->str(inst), pad_inst)
               .arg(conditional(cccc))
               .arg(conditional(zzzz));

    if (IR.op.wc || IR.op.wz) {
        m_string.resize(pad_wcz, QChar::Space);
        if (IR.op.wc && IR.op.wz) {
            m_string += QString("%1%2")
                        .arg(Token->str(with))
                        .arg(Token->str(t_CZ));
        } else if (IR.op.wz) {
            m_string += QString("%1%2")
                        .arg(Token->str(with))
                        .arg(Token->str(t_Z));
        } else {
            m_string += QString("%1%2")
                        .arg(Token->str(with))
                        .arg(Token->str(t_C));
        }
    }
}

/**
 * @brief format string for: EEEE xxxxxxx xxx DDDDDDDDD xxxxxxxxx
 *
 *  "INSTR  D"
 *
 * @param inst instruction token (mnemonic)
 */
void P2Dasm::format_d(p2_token_e inst)
{
    m_string = QString("%1%2%3")
               .arg(Token->str(inst), pad_inst)
               .arg(format_num(IR.op.dst));
}

/**
 * @brief format string for: EEEE xxxxxxx xLx DDDDDDDDD xxxxxxxxx
 *
 *  "INSTR  {#}D"
 *
 * @param inst instruction token (mnemonic)
 */
void P2Dasm::format_wz_d(p2_token_e inst)
{
    m_string = QString("%1%2%3")
               .arg(Token->str(inst), pad_inst)
               .arg(IR.op.wz ? "#" : "")
               .arg(format_num(IR.op.dst));
}

/**
 * @brief format string for: EEEE xxxxxxx xxL DDDDDDDDD xxxxxxxxx
 *
 *  "INSTR  {#}D"
 *
 * @param inst instruction token (mnemonic)
 */
void P2Dasm::format_imm_d(p2_token_e inst)
{
    m_string = QString("%1%2%3")
               .arg(Token->str(inst), pad_inst)
               .arg(IR.op.imm ? "#" : "")
               .arg(format_num(IR.op.dst));
}

/**
 * @brief format string for: EEEE xxxxxxx CZL DDDDDDDDD xxxxxxxxx
 *
 *  "INSTR  {#}D    {WC,WZ,WCZ}"
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 */
void P2Dasm::format_imm_d_cz(p2_token_e inst, p2_token_e with)
{
    m_string = QString("%1%2%3")
               .arg(Token->str(inst), pad_inst)
               .arg(IR.op.imm ? "#" : "")
               .arg(format_num(IR.op.dst));

    if (IR.op.wc || IR.op.wz) {
        m_string.resize(pad_wcz, QChar::Space);
        if (IR.op.wc && IR.op.wz) {
            m_string += QString("%1%2")
                        .arg(Token->str(with))
                        .arg(Token->str(t_CZ));
        } else if (IR.op.wz) {
            m_string += QString("%1%2")
                        .arg(Token->str(with))
                        .arg(Token->str(t_Z));
        } else {
            m_string += QString("%1%2")
                        .arg(Token->str(with))
                        .arg(Token->str(t_C));
        }
    }
}

/**
 * @brief format string for: EEEE xxxxxxx CxL DDDDDDDDD xxxxxxxxx
 *
 *  "INSTR  {#}D    {WC}"
 *
 * @param inst instruction token (mnemonic)
 */
void P2Dasm::format_imm_d_c(p2_token_e inst)
{
    m_string = QString("%1%2%3")
               .arg(Token->str(inst), pad_inst)
               .arg(IR.op.imm ? "#" : "")
               .arg(format_num(IR.op.dst));

    if (IR.op.wc) {
        m_string.resize(pad_wcz, QChar::Space);
        m_string += Token->str(t_W);
        m_string += Token->str(t_C);
    }
}

/**
 * @brief format string for: EEEE xxxxxxx xxI xxxxxxxxx SSSSSSSSS
 *
 *  "INSTR  {#}S"
 *
 * @param inst instruction token (mnemonic)
 */
void P2Dasm::format_imm_s(p2_token_e inst)
{
    m_string = QString("%1%2%3")
               .arg(inst, pad_inst)
               .arg(IR.op.imm ? "#" : "")
               .arg(format_num(IR.op.src));
}

/**
 * @brief format string for: EEEE xxxxxxx RAA AAAAAAAAA AAAAAAAAA
 *
 *  "INSTR  {PC+}#$AAAAAA"
 *
 * @param inst instruction token (mnemonic)
 * @param dest destination token (PA, PB, PTRA, PTRB)
 */
void P2Dasm::format_pc_abs(p2_token_e inst, p2_token_e dest)
{
    const quint32 addr = IR.word & ((1u << 20) - 1);

    m_string = QString("%1%2%3%4$%5")
               .arg(Token->str(inst), pad_inst)
               .arg(Token->str(dest))
               .arg(dest != t_nothing ? "," : "")
               .arg(IR.op.wc ? "PC+" : "")
               .arg(addr, 0, 16);
}

/**
 * @brief format string for: EEEE xxxxxNN NNN NNNNNNNNN NNNNNNNNN
 *
 *  "INSTR  #$NNNNNNNN"
 *
 * @param inst instruction token (mnemonic)
 */
void P2Dasm::format_imm23(p2_token_e inst)
{
    const quint32 nnnn = IR.word & ((1u << 23) - 1);

    m_string = QString("%1#$%2")
               .arg(Token->str(inst), pad_inst)
               .arg(nnnn << 9, 0, 16, QChar('0'));
}

/**
 * @brief No operation.
 *
 * 0000 0000000 000 000000000 000000000
 *
 * NOP
 *
 */
void P2Dasm::dasm_nop()
{
    format_inst(t_NOP);
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
void P2Dasm::dasm_ror()
{
    if (0 == IR.word)
        dasm_nop();
    else
        format_d_imm_s_cz(t_ROR);
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
void P2Dasm::dasm_rol()
{
    format_d_imm_s_cz(t_ROL);
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
void P2Dasm::dasm_shr()
{
    format_d_imm_s_cz(t_SHR);
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
void P2Dasm::dasm_shl()
{
    format_d_imm_s_cz(t_SHL);
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
void P2Dasm::dasm_rcr()
{
    format_d_imm_s_cz(t_RCR);
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
void P2Dasm::dasm_rcl()
{
    format_d_imm_s_cz(t_RCL);
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
void P2Dasm::dasm_sar()
{
    format_d_imm_s_cz(t_SAR);
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
void P2Dasm::dasm_sal()
{
    format_d_imm_s_cz(t_SAL);
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
void P2Dasm::dasm_add()
{
    format_d_imm_s_cz(t_ADD);
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
void P2Dasm::dasm_addx()
{
    format_d_imm_s_cz(t_ADDX);
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
void P2Dasm::dasm_adds()
{
    format_d_imm_s_cz(t_ADDS);
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
void P2Dasm::dasm_addsx()
{
    format_d_imm_s_cz(t_ADDSX);
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
void P2Dasm::dasm_sub()
{
    format_d_imm_s_cz(t_SUB);
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
void P2Dasm::dasm_subx()
{
    format_d_imm_s_cz(t_SUBX);
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
void P2Dasm::dasm_subs()
{
    format_d_imm_s_cz(t_SUBS);
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
void P2Dasm::dasm_subsx()
{
    format_d_imm_s_cz(t_SUBSX);
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
void P2Dasm::dasm_cmp()
{
    format_d_imm_s_cz(t_CMP);
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
void P2Dasm::dasm_cmpx()
{
    format_d_imm_s_cz(t_CMPX);
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
void P2Dasm::dasm_cmps()
{
    format_d_imm_s_cz(t_CMPS);
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
void P2Dasm::dasm_cmpsx()
{
    format_d_imm_s_cz(t_CMPSX);
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
void P2Dasm::dasm_cmpr()
{
    format_d_imm_s_cz(t_CMPR);
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
void P2Dasm::dasm_cmpm()
{
    format_d_imm_s_cz(t_CMPM);
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
void P2Dasm::dasm_subr()
{
    format_d_imm_s_cz(t_SUBR);
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
void P2Dasm::dasm_cmpsub()
{
    format_d_imm_s_cz(t_CMPSUB);
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
void P2Dasm::dasm_fge()
{
    format_d_imm_s_cz(t_FGE);
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
void P2Dasm::dasm_fle()
{
    format_d_imm_s_cz(t_FLE);
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
void P2Dasm::dasm_fges()
{
    format_d_imm_s_cz(t_FGES);
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
void P2Dasm::dasm_fles()
{
    format_d_imm_s_cz(t_FLES);
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
void P2Dasm::dasm_sumc()
{
    format_d_imm_s_cz(t_SUMC);
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
void P2Dasm::dasm_sumnc()
{
    format_d_imm_s_cz(t_SUMNC);
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
void P2Dasm::dasm_sumz()
{
    format_d_imm_s_cz(t_SUMZ);
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
void P2Dasm::dasm_sumnz()
{
    format_d_imm_s_cz(t_SUMNZ);
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
void P2Dasm::dasm_testb_w()
{
    format_d_imm_s_cz(t_TESTB);
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
void P2Dasm::dasm_testbn_w()
{
    format_d_imm_s_cz(t_TESTBN);
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
void P2Dasm::dasm_testb_and()
{
    format_d_imm_s_cz(t_TESTB, t_AND);
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
void P2Dasm::dasm_testbn_and()
{
    format_d_imm_s_cz(t_TESTNB, t_AND);
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
void P2Dasm::dasm_testb_or()
{
    format_d_imm_s_cz(t_TESTB, t_OR);
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
void P2Dasm::dasm_testbn_or()
{
    format_d_imm_s_cz(t_TESTNB, t_OR);
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
void P2Dasm::dasm_testb_xor()
{
    format_d_imm_s_cz(t_TESTB, t_XOR);
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
void P2Dasm::dasm_testbn_xor()
{
    format_d_imm_s_cz(t_TESTBN, t_XOR);
}

/**
 * @brief Bit S[4:0] of D = 0,    C,Z = D[S[4:0]].
 *
 * EEEE 0100000 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITL    D,{#}S         {WCZ}
 *
 */
void P2Dasm::dasm_bitl()
{
    format_d_imm_s_cz(t_BITL);
}

/**
 * @brief Bit S[4:0] of D = 1,    C,Z = D[S[4:0]].
 *
 * EEEE 0100001 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITH    D,{#}S         {WCZ}
 *
 */
void P2Dasm::dasm_bith()
{
    format_d_imm_s_cz(t_BITH);
}

/**
 * @brief Bit S[4:0] of D = C,    C,Z = D[S[4:0]].
 *
 * EEEE 0100010 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITC    D,{#}S         {WCZ}
 *
 */
void P2Dasm::dasm_bitc()
{
    format_d_imm_s_cz(t_BITC);
}

/**
 * @brief Bit S[4:0] of D = !C,   C,Z = D[S[4:0]].
 *
 * EEEE 0100011 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNC   D,{#}S         {WCZ}
 *
 */
void P2Dasm::dasm_bitnc()
{
    format_d_imm_s_cz(t_BITNC);
}

/**
 * @brief Bit S[4:0] of D = Z,    C,Z = D[S[4:0]].
 *
 * EEEE 0100100 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITZ    D,{#}S         {WCZ}
 *
 */
void P2Dasm::dasm_bitz()
{
    format_d_imm_s_cz(t_BITZ);
}

/**
 * @brief Bit S[4:0] of D = !Z,   C,Z = D[S[4:0]].
 *
 * EEEE 0100101 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNZ   D,{#}S         {WCZ}
 *
 */
void P2Dasm::dasm_bitnz()
{
    format_d_imm_s_cz(t_BITNZ);
}

/**
 * @brief Bit S[4:0] of D = RND,  C,Z = D[S[4:0]].
 *
 * EEEE 0100110 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITRND  D,{#}S         {WCZ}
 *
 */
void P2Dasm::dasm_bitrnd()
{
    format_d_imm_s_cz(t_BITRND);
}

/**
 * @brief Bit S[4:0] of D = !bit, C,Z = D[S[4:0]].
 *
 * EEEE 0100111 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNOT  D,{#}S         {WCZ}
 *
 */
void P2Dasm::dasm_bitnot()
{
    format_d_imm_s_cz(t_BITNOT);
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
void P2Dasm::dasm_and()
{
    format_d_imm_s_cz(t_AND);
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
void P2Dasm::dasm_andn()
{
    format_d_imm_s_cz(t_ANDN);
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
void P2Dasm::dasm_or()
{
    format_d_imm_s_cz(t_OR);
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
void P2Dasm::dasm_xor()
{
    format_d_imm_s_cz(t_XOR);
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
void P2Dasm::dasm_muxc()
{
    format_d_imm_s_cz(t_MUXC);
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
void P2Dasm::dasm_muxnc()
{
    format_d_imm_s_cz(t_MUXNC);
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
void P2Dasm::dasm_muxz()
{
    format_d_imm_s_cz(t_MUXZ);
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
void P2Dasm::dasm_muxnz()
{
    format_d_imm_s_cz(t_MUXNZ);
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
void P2Dasm::dasm_mov()
{
    format_d_imm_s_cz(t_MOV);
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
void P2Dasm::dasm_not()
{
    format_d_imm_s_cz(t_NOT);
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
void P2Dasm::dasm_abs()
{
    format_d_imm_s_cz(t_ABS);
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
void P2Dasm::dasm_neg()
{
    format_d_imm_s_cz(t_NEG);
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
void P2Dasm::dasm_negc()
{
    format_d_imm_s_cz(t_NEGC);
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
void P2Dasm::dasm_negnc()
{
    format_d_imm_s_cz(t_NEGNC);
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
void P2Dasm::dasm_negz()
{
    format_d_imm_s_cz(t_NEGZ);
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
void P2Dasm::dasm_negnz()
{
    format_d_imm_s_cz(t_NEGNZ);
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
void P2Dasm::dasm_incmod()
{
    format_d_imm_s_cz(t_INCMOD);
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
void P2Dasm::dasm_decmod()
{
    format_d_imm_s_cz(t_DECMOD);
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
void P2Dasm::dasm_zerox()
{
    format_d_imm_s_cz(t_ZEROX);
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
void P2Dasm::dasm_signx()
{
    format_d_imm_s_cz(t_SIGNX);
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
void P2Dasm::dasm_encod()
{
    format_d_imm_s_cz(t_ENCOD);
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
void P2Dasm::dasm_ones()
{
    format_d_imm_s_cz(t_ONES);
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
void P2Dasm::dasm_test()
{
    format_d_imm_s_cz(t_TEST);
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
void P2Dasm::dasm_testn()
{
    format_d_imm_s_cz(t_TESTN);
}

/**
 * @brief Set S[3:0] into nibble N in D, keeping rest of D same.
 *
 * EEEE 100000N NNI DDDDDDDDD SSSSSSSSS
 *
 * SETNIB  D,{#}S,#N
 *
 */
void P2Dasm::dasm_setnib()
{
    format_d_imm_s_nnn(t_SETNIB);
}

/**
 * @brief Set S[3:0] into nibble established by prior ALTSN instruction.
 *
 * EEEE 1000000 00I 000000000 SSSSSSSSS
 *
 * SETNIB  {#}S
 *
 */
void P2Dasm::dasm_setnib_altsn()
{
    format_imm_s(t_SETNIB);
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
void P2Dasm::dasm_getnib()
{
    format_d_imm_s_nnn(t_GETNIB);
}

/**
 * @brief Get nibble established by prior ALTGN instruction into D.
 *
 * EEEE 1000010 000 DDDDDDDDD 000000000
 *
 * GETNIB  D
 *
 */
void P2Dasm::dasm_getnib_altgn()
{
    format_imm_s(t_GETNIB);
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
void P2Dasm::dasm_rolnib()
{
    format_d_imm_s_nnn(t_ROLNIB);
}

/**
 * @brief Rotate-left nibble established by prior ALTGN instruction into D.
 *
 * EEEE 1000100 000 DDDDDDDDD 000000000
 *
 * ROLNIB  D
 *
 */
void P2Dasm::dasm_rolnib_altgn()
{
    format_d(t_ROLNIB);
}

/**
 * @brief Set S[7:0] into byte N in D, keeping rest of D same.
 *
 * EEEE 1000110 NNI DDDDDDDDD SSSSSSSSS
 *
 * SETBYTE D,{#}S,#N
 *
 */
void P2Dasm::dasm_setbyte()
{
    format_d_imm_s_nnn(t_SETBYTE);
}

/**
 * @brief Set S[7:0] into byte established by prior ALTSB instruction.
 *
 * EEEE 1000110 00I 000000000 SSSSSSSSS
 *
 * SETBYTE {#}S
 *
 */
void P2Dasm::dasm_setbyte_altsb()
{
    format_imm_s(t_SETBYTE);
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
void P2Dasm::dasm_getbyte()
{
    format_d_imm_s_nnn(t_GETBYTE);
}

/**
 * @brief Get byte established by prior ALTGB instruction into D.
 *
 * EEEE 1000111 000 DDDDDDDDD 000000000
 *
 * GETBYTE D
 *
 */
void P2Dasm::dasm_getbyte_altgb()
{
    format_d(t_GETBYTE);
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
void P2Dasm::dasm_rolbyte()
{
    format_d_imm_s_nnn(t_ROLBYTE);
}

/**
 * @brief Rotate-left byte established by prior ALTGB instruction into D.
 *
 * EEEE 1001000 000 DDDDDDDDD 000000000
 *
 * ROLBYTE D
 *
 */
void P2Dasm::dasm_rolbyte_altgb()
{
    format_d(t_ROLBYTE);
}

/**
 * @brief Set S[15:0] into word N in D, keeping rest of D same.
 *
 * EEEE 1001001 0NI DDDDDDDDD SSSSSSSSS
 *
 * SETWORD D,{#}S,#N
 *
 */
void P2Dasm::dasm_setword()
{
    format_d_imm_s_n(t_SETWORD);
}

/**
 * @brief Set S[15:0] into word established by prior ALTSW instruction.
 *
 * EEEE 1001001 00I 000000000 SSSSSSSSS
 *
 * SETWORD {#}S
 *
 */
void P2Dasm::dasm_setword_altsw()
{
    format_imm_s(t_SETWORD);
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
void P2Dasm::dasm_getword()
{
    format_d_imm_s_n(t_GETWORD);
}

/**
 * @brief Get word established by prior ALTGW instruction into D.
 *
 * EEEE 1001001 100 DDDDDDDDD 000000000
 *
 * GETWORD D
 *
 */
void P2Dasm::dasm_getword_altgw()
{
    format_imm_s(t_GETWORD);
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
void P2Dasm::dasm_rolword()
{
    format_d_imm_s_n(t_ROLWORD);
}

/**
 * @brief Rotate-left word established by prior ALTGW instruction into D.
 *
 * EEEE 1001010 000 DDDDDDDDD 000000000
 *
 * ROLWORD D
 *
 */
void P2Dasm::dasm_rolword_altgw()
{
    format_d(t_ROLWORD);
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
void P2Dasm::dasm_altsn()
{
    format_d_imm_s(t_ALTSN);
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
void P2Dasm::dasm_altsn_d()
{
    format_d(t_ALTSN);
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
void P2Dasm::dasm_altgn()
{
    format_d_imm_s(t_ALTGN);
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
void P2Dasm::dasm_altgn_d()
{
    format_d(t_ALTGN);
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
void P2Dasm::dasm_altsb()
{
    format_d_imm_s(t_ALTSB);
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
void P2Dasm::dasm_altsb_d()
{
    format_d(t_ALTSB);
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
void P2Dasm::dasm_altgb()
{
    format_d_imm_s(t_ALTGB);
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
void P2Dasm::dasm_altgb_d()
{
    format_d(t_ALTGB);
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
void P2Dasm::dasm_altsw()
{
    format_d_imm_s(t_ALTSW);
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
void P2Dasm::dasm_altsw_d()
{
    format_d(t_ALTSW);
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
void P2Dasm::dasm_altgw()
{
    format_d_imm_s(t_ALTGW);
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
void P2Dasm::dasm_altgw_d()
{
    format_d(t_ALTGW);
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
void P2Dasm::dasm_altr()
{
    format_d_imm_s(t_ALTR);
}

/**
 * @brief Alter result register address (normally D field) of next instruction to D[8:0].
 *
 * EEEE 1001100 001 DDDDDDDDD 000000000
 *
 * ALTR    D
 *
 */
void P2Dasm::dasm_altr_d()
{
    format_d(t_ALTD);
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
void P2Dasm::dasm_altd()
{
    format_d_imm_s(t_ALTD);
}

/**
 * @brief Alter D field of next instruction to D[8:0].
 *
 * EEEE 1001100 011 DDDDDDDDD 000000000
 *
 * ALTD    D
 *
 */
void P2Dasm::dasm_altd_d()
{
    format_d(t_ALTD);
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
void P2Dasm::dasm_alts()
{
    format_d_imm_s(t_ALTS);
}

/**
 * @brief Alter S field of next instruction to D[8:0].
 *
 * EEEE 1001100 101 DDDDDDDDD 000000000
 *
 * ALTS    D
 *
 */
void P2Dasm::dasm_alts_d()
{
    format_d(t_ALTS);
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
void P2Dasm::dasm_altb()
{
    format_d_imm_s(t_ALTB);
}

/**
 * @brief Alter D field of next instruction to D[13:5].
 *
 * EEEE 1001100 111 DDDDDDDDD 000000000
 *
 * ALTB    D
 *
 */
void P2Dasm::dasm_altb_d()
{
    format_d(t_ALTB);
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
void P2Dasm::dasm_alti()
{
    format_d_imm_s(t_ALTI);
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
void P2Dasm::dasm_alti_d()
{
    format_d(t_ALTI);
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
void P2Dasm::dasm_setr()
{
    format_d_imm_s(t_SETR);
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
void P2Dasm::dasm_setd()
{
    format_d_imm_s(t_SETD);
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
void P2Dasm::dasm_sets()
{
    format_d_imm_s(t_SETS);
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
void P2Dasm::dasm_decod()
{
    format_d_imm_s(t_DECOD);
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
void P2Dasm::dasm_decod_d()
{
    format_d(t_DECOD);
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
void P2Dasm::dasm_bmask()
{
    format_d_imm_s(t_BMASK);
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
void P2Dasm::dasm_bmask_d()
{
    format_d(t_BMASK);
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
void P2Dasm::dasm_crcbit()
{
    format_d_imm_s(t_CRCBIT);
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
void P2Dasm::dasm_crcnib()
{
    format_d_imm_s(t_CRCNIB);
}

/**
 * @brief For each non-zero bit pair in S, copy that bit pair into the corresponding D bits, else leave that D bit pair the same.
 *
 * EEEE 1001111 00I DDDDDDDDD SSSSSSSSS
 *
 * MUXNITS D,{#}S
 *
 */
void P2Dasm::dasm_muxnits()
{
    format_d_imm_s(t_MUXNITS);
}

/**
 * @brief For each non-zero nibble in S, copy that nibble into the corresponding D nibble, else leave that D nibble the same.
 *
 * EEEE 1001111 01I DDDDDDDDD SSSSSSSSS
 *
 * MUXNIBS D,{#}S
 *
 */
void P2Dasm::dasm_muxnibs()
{
    format_d_imm_s(t_MUXNIBS);
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
void P2Dasm::dasm_muxq()
{
    format_d_imm_s(t_MUXQ);
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
void P2Dasm::dasm_movbyts()
{
    format_d_imm_s(t_MOVBYTS);
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
void P2Dasm::dasm_mul()
{
    format_d_imm_s_z(t_MUL);
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
void P2Dasm::dasm_muls()
{
    format_d_imm_s_z(t_MULS);
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
void P2Dasm::dasm_sca()
{
    format_d_imm_s_z(t_SCA);
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
void P2Dasm::dasm_scas()
{
    format_d_imm_s_z(t_SCAS);
}

/**
 * @brief Add bytes of S into bytes of D, with $FF saturation.
 *
 * EEEE 1010010 00I DDDDDDDDD SSSSSSSSS
 *
 * ADDPIX  D,{#}S
 *
 */
void P2Dasm::dasm_addpix()
{
    format_d_imm_s(t_ADDPIX);
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
void P2Dasm::dasm_mulpix()
{
    format_d_imm_s(t_MULPIX);
}

/**
 * @brief Alpha-blend bytes of S into bytes of D, using SETPIV value.
 *
 * EEEE 1010010 10I DDDDDDDDD SSSSSSSSS
 *
 * BLNPIX  D,{#}S
 *
 */
void P2Dasm::dasm_blnpix()
{
    format_d_imm_s(t_BLNPIX);
}

/**
 * @brief Mix bytes of S into bytes of D, using SETPIX and SETPIV values.
 *
 * EEEE 1010010 11I DDDDDDDDD SSSSSSSSS
 *
 * MIXPIX  D,{#}S
 *
 */
void P2Dasm::dasm_mixpix()
{
    format_d_imm_s(t_MIXPIX);
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
void P2Dasm::dasm_addct1()
{
    format_d_imm_s(t_ADDCT1);
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
void P2Dasm::dasm_addct2()
{
    format_d_imm_s(t_ADDCT2);
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
void P2Dasm::dasm_addct3()
{
    format_d_imm_s(t_ADDCT3);
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
void P2Dasm::dasm_wmlong()
{
    format_d_imm_s(t_WMLONG);
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
void P2Dasm::dasm_rqpin()
{
    format_d_imm_s_c(t_RQPIN);
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
void P2Dasm::dasm_rdpin()
{
    format_d_imm_s_c(t_RDPIN);
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
void P2Dasm::dasm_rdlut()
{
    format_d_imm_s_cz(t_RDLUT);
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
void P2Dasm::dasm_rdbyte()
{
    format_d_imm_s_cz(t_RDBYTE);
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
void P2Dasm::dasm_rdword()
{
    format_d_imm_s_cz(t_RDWORD);
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
void P2Dasm::dasm_rdlong()
{
    format_d_imm_s_cz(t_RDLONG);
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
void P2Dasm::dasm_popa()
{
    format_d_imm_s_cz(t_POPA);
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
void P2Dasm::dasm_popb()
{
    format_d_imm_s_cz(t_POPB);
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
void P2Dasm::dasm_calld()
{
    if (IR.op.wc && IR.op.wz) {
        if (IR.op.dst == offs_IJMP3 && IR.op.src == offs_IRET3) {
            dasm_resi3();
            return;
        }
        if (IR.op.dst == offs_IJMP2 && IR.op.src == offs_IRET2) {
            dasm_resi2();
            return;
        }
        if (IR.op.dst == offs_IJMP1 && IR.op.src == offs_IRET1) {
            dasm_resi1();
            return;
        }
        if (IR.op.dst == offs_INA && IR.op.src == offs_INB) {
            dasm_resi0();
            return;
        }
        if (IR.op.dst == offs_INB && IR.op.src == offs_IRET3) {
            dasm_reti3();
            return;
        }
        if (IR.op.dst == offs_INB && IR.op.src == offs_IRET2) {
            dasm_reti2();
            return;
        }
        if (IR.op.dst == offs_INB && IR.op.src == offs_IRET1) {
            dasm_reti1();
            return;
        }
        if (IR.op.dst == offs_INB && IR.op.src == offs_INB) {
            dasm_reti0();
            return;
        }
    }
    format_d_imm_s_cz(t_CALLD);
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
void P2Dasm::dasm_resi3()
{
    format_inst(t_RESI3);
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
void P2Dasm::dasm_resi2()
{
    format_inst(t_RESI2);
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
void P2Dasm::dasm_resi1()
{
    format_inst(t_RESI1);
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
void P2Dasm::dasm_resi0()
{
    format_inst(t_RESI0);
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
void P2Dasm::dasm_reti3()
{
    format_inst(t_RETI3);
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
void P2Dasm::dasm_reti2()
{
    format_inst(t_RETI2);
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
void P2Dasm::dasm_reti1()
{
    format_inst(t_RETI1);
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
void P2Dasm::dasm_reti0()
{
    format_inst(t_RETI0);
}

/**
 * @brief Call to S** by pushing {C, Z, 10'b0, PC[19:0]} onto stack, copy D to PA.
 *
 * EEEE 1011010 0LI DDDDDDDDD SSSSSSSSS
 *
 * CALLPA  {#}D,{#}S
 *
 */
void P2Dasm::dasm_callpa()
{
    format_wz_d_imm_s(t_CALLPA);
}

/**
 * @brief Call to S** by pushing {C, Z, 10'b0, PC[19:0]} onto stack, copy D to PB.
 *
 * EEEE 1011010 1LI DDDDDDDDD SSSSSSSSS
 *
 * CALLPB  {#}D,{#}S
 *
 */
void P2Dasm::dasm_callpb()
{
    format_wz_d_imm_s(t_CALLPB);
}

/**
 * @brief Decrement D and jump to S** if result is zero.
 *
 * EEEE 1011011 00I DDDDDDDDD SSSSSSSSS
 *
 * DJZ     D,{#}S
 *
 */
void P2Dasm::dasm_djz()
{
    format_d_imm_s(t_DJZ);
}

/**
 * @brief Decrement D and jump to S** if result is not zero.
 *
 * EEEE 1011011 01I DDDDDDDDD SSSSSSSSS
 *
 * DJNZ    D,{#}S
 *
 */
void P2Dasm::dasm_djnz()
{
    format_d_imm_s(t_DJNZ);
}

/**
 * @brief Decrement D and jump to S** if result is $FFFF_FFFF.
 *
 * EEEE 1011011 10I DDDDDDDDD SSSSSSSSS
 *
 * DJF     D,{#}S
 *
 */
void P2Dasm::dasm_djf()
{
    format_d_imm_s(t_DJF);
}

/**
 * @brief Decrement D and jump to S** if result is not $FFFF_FFFF.
 *
 * EEEE 1011011 11I DDDDDDDDD SSSSSSSSS
 *
 * DJNF    D,{#}S
 *
 */
void P2Dasm::dasm_djnf()
{
    format_d_imm_s(t_DJNF);
}

/**
 * @brief Increment D and jump to S** if result is zero.
 *
 * EEEE 1011100 00I DDDDDDDDD SSSSSSSSS
 *
 * IJZ     D,{#}S
 *
 */
void P2Dasm::dasm_ijz()
{
    format_d_imm_s(t_IJZ);
}

/**
 * @brief Increment D and jump to S** if result is not zero.
 *
 * EEEE 1011100 01I DDDDDDDDD SSSSSSSSS
 *
 * IJNZ    D,{#}S
 *
 */
void P2Dasm::dasm_ijnz()
{
    format_d_imm_s(t_IJNZ);
}

/**
 * @brief Test D and jump to S** if D is zero.
 *
 * EEEE 1011100 10I DDDDDDDDD SSSSSSSSS
 *
 * TJZ     D,{#}S
 *
 */
void P2Dasm::dasm_tjz()
{
    format_d_imm_s(t_TJZ);
}

/**
 * @brief Test D and jump to S** if D is not zero.
 *
 * EEEE 1011100 11I DDDDDDDDD SSSSSSSSS
 *
 * TJNZ    D,{#}S
 *
 */
void P2Dasm::dasm_tjnz()
{
    format_d_imm_s(t_TJNZ);
}

/**
 * @brief Test D and jump to S** if D is full (D = $FFFF_FFFF).
 *
 * EEEE 1011101 00I DDDDDDDDD SSSSSSSSS
 *
 * TJF     D,{#}S
 *
 */
void P2Dasm::dasm_tjf()
{
    format_d_imm_s(t_TJF);
}

/**
 * @brief Test D and jump to S** if D is not full (D != $FFFF_FFFF).
 *
 * EEEE 1011101 01I DDDDDDDDD SSSSSSSSS
 *
 * TJNF    D,{#}S
 *
 */
void P2Dasm::dasm_tjnf()
{
    format_d_imm_s(t_TJNF);
}

/**
 * @brief Test D and jump to S** if D is signed (D[31] = 1).
 *
 * EEEE 1011101 10I DDDDDDDDD SSSSSSSSS
 *
 * TJS     D,{#}S
 *
 */
void P2Dasm::dasm_tjs()
{
    format_d_imm_s(t_TJS);
}

/**
 * @brief Test D and jump to S** if D is not signed (D[31] = 0).
 *
 * EEEE 1011101 11I DDDDDDDDD SSSSSSSSS
 *
 * TJNS    D,{#}S
 *
 */
void P2Dasm::dasm_tjns()
{
    format_d_imm_s(t_TJNS);
}

/**
 * @brief Test D and jump to S** if D overflowed (D[31] != C, C = 'correct sign' from last addition/subtraction).
 *
 * EEEE 1011110 00I DDDDDDDDD SSSSSSSSS
 *
 * TJV     D,{#}S
 *
 */
void P2Dasm::dasm_tjv()
{
    format_d_imm_s(t_TJV);
}

/**
 * @brief Jump to S** if INT event flag is set.
 *
 * EEEE 1011110 01I 000000000 SSSSSSSSS
 *
 * JINT    {#}S
 *
 */
void P2Dasm::dasm_jint()
{
    format_imm_s(t_JINT);
}

/**
 * @brief Jump to S** if CT1 event flag is set.
 *
 * EEEE 1011110 01I 000000001 SSSSSSSSS
 *
 * JCT1    {#}S
 *
 */
void P2Dasm::dasm_jct1()
{
    format_imm_s(t_JCT1);
}

/**
 * @brief Jump to S** if CT2 event flag is set.
 *
 * EEEE 1011110 01I 000000010 SSSSSSSSS
 *
 * JCT2    {#}S
 *
 */
void P2Dasm::dasm_jct2()
{
    format_imm_s(t_JCT2);
}

/**
 * @brief Jump to S** if CT3 event flag is set.
 *
 * EEEE 1011110 01I 000000011 SSSSSSSSS
 *
 * JCT3    {#}S
 *
 */
void P2Dasm::dasm_jct3()
{
    format_imm_s(t_JCT3);
}

/**
 * @brief Jump to S** if SE1 event flag is set.
 *
 * EEEE 1011110 01I 000000100 SSSSSSSSS
 *
 * JSE1    {#}S
 *
 */
void P2Dasm::dasm_jse1()
{
    format_imm_s(t_JSE1);
}

/**
 * @brief Jump to S** if SE2 event flag is set.
 *
 * EEEE 1011110 01I 000000101 SSSSSSSSS
 *
 * JSE2    {#}S
 *
 */
void P2Dasm::dasm_jse2()
{
    format_imm_s(t_JSE2);
}

/**
 * @brief Jump to S** if SE3 event flag is set.
 *
 * EEEE 1011110 01I 000000110 SSSSSSSSS
 *
 * JSE3    {#}S
 *
 */
void P2Dasm::dasm_jse3()
{
    format_imm_s(t_JSE3);
}

/**
 * @brief Jump to S** if SE4 event flag is set.
 *
 * EEEE 1011110 01I 000000111 SSSSSSSSS
 *
 * JSE4    {#}S
 *
 */
void P2Dasm::dasm_jse4()
{
    format_imm_s(t_JSE4);
}

/**
 * @brief Jump to S** if PAT event flag is set.
 *
 * EEEE 1011110 01I 000001000 SSSSSSSSS
 *
 * JPAT    {#}S
 *
 */
void P2Dasm::dasm_jpat()
{
    format_imm_s(t_JPAT);
}

/**
 * @brief Jump to S** if FBW event flag is set.
 *
 * EEEE 1011110 01I 000001001 SSSSSSSSS
 *
 * JFBW    {#}S
 *
 */
void P2Dasm::dasm_jfbw()
{
    format_imm_s(t_JFBW);
}

/**
 * @brief Jump to S** if XMT event flag is set.
 *
 * EEEE 1011110 01I 000001010 SSSSSSSSS
 *
 * JXMT    {#}S
 *
 */
void P2Dasm::dasm_jxmt()
{
    format_imm_s(t_JXMT);
}

/**
 * @brief Jump to S** if XFI event flag is set.
 *
 * EEEE 1011110 01I 000001011 SSSSSSSSS
 *
 * JXFI    {#}S
 *
 */
void P2Dasm::dasm_jxfi()
{
    format_imm_s(t_JXFI);
}

/**
 * @brief Jump to S** if XRO event flag is set.
 *
 * EEEE 1011110 01I 000001100 SSSSSSSSS
 *
 * JXRO    {#}S
 *
 */
void P2Dasm::dasm_jxro()
{
    format_imm_s(t_JXRO);
}

/**
 * @brief Jump to S** if XRL event flag is set.
 *
 * EEEE 1011110 01I 000001101 SSSSSSSSS
 *
 * JXRL    {#}S
 *
 */
void P2Dasm::dasm_jxrl()
{
    format_imm_s(t_JXRL);
}

/**
 * @brief Jump to S** if ATN event flag is set.
 *
 * EEEE 1011110 01I 000001110 SSSSSSSSS
 *
 * JATN    {#}S
 *
 */
void P2Dasm::dasm_jatn()
{
    format_imm_s(t_JATN);
}

/**
 * @brief Jump to S** if QMT event flag is set.
 *
 * EEEE 1011110 01I 000001111 SSSSSSSSS
 *
 * JQMT    {#}S
 *
 */
void P2Dasm::dasm_jqmt()
{
    format_imm_s(t_JQMT);
}

/**
 * @brief Jump to S** if INT event flag is clear.
 *
 * EEEE 1011110 01I 000010000 SSSSSSSSS
 *
 * JNINT   {#}S
 *
 */
void P2Dasm::dasm_jnint()
{
    format_imm_s(t_JNINT);
}

/**
 * @brief Jump to S** if CT1 event flag is clear.
 *
 * EEEE 1011110 01I 000010001 SSSSSSSSS
 *
 * JNCT1   {#}S
 *
 */
void P2Dasm::dasm_jnct1()
{
    format_imm_s(t_JNCT1);
}

/**
 * @brief Jump to S** if CT2 event flag is clear.
 *
 * EEEE 1011110 01I 000010010 SSSSSSSSS
 *
 * JNCT2   {#}S
 *
 */
void P2Dasm::dasm_jnct2()
{
    format_imm_s(t_JNCT2);
}

/**
 * @brief Jump to S** if CT3 event flag is clear.
 *
 * EEEE 1011110 01I 000010011 SSSSSSSSS
 *
 * JNCT3   {#}S
 *
 */
void P2Dasm::dasm_jnct3()
{
    format_imm_s(t_JNCT3);
}

/**
 * @brief Jump to S** if SE1 event flag is clear.
 *
 * EEEE 1011110 01I 000010100 SSSSSSSSS
 *
 * JNSE1   {#}S
 *
 */
void P2Dasm::dasm_jnse1()
{
    format_imm_s(t_JNSE1);
}

/**
 * @brief Jump to S** if SE2 event flag is clear.
 *
 * EEEE 1011110 01I 000010101 SSSSSSSSS
 *
 * JNSE2   {#}S
 *
 */
void P2Dasm::dasm_jnse2()
{
    format_imm_s(t_JNSE2);
}

/**
 * @brief Jump to S** if SE3 event flag is clear.
 *
 * EEEE 1011110 01I 000010110 SSSSSSSSS
 *
 * JNSE3   {#}S
 *
 */
void P2Dasm::dasm_jnse3()
{
    format_imm_s(t_JNSE3);
}

/**
 * @brief Jump to S** if SE4 event flag is clear.
 *
 * EEEE 1011110 01I 000010111 SSSSSSSSS
 *
 * JNSE4   {#}S
 *
 */
void P2Dasm::dasm_jnse4()
{
    format_imm_s(t_JNSE4);
}

/**
 * @brief Jump to S** if PAT event flag is clear.
 *
 * EEEE 1011110 01I 000011000 SSSSSSSSS
 *
 * JNPAT   {#}S
 *
 */
void P2Dasm::dasm_jnpat()
{
    format_imm_s(t_JNPAT);
}

/**
 * @brief Jump to S** if FBW event flag is clear.
 *
 * EEEE 1011110 01I 000011001 SSSSSSSSS
 *
 * JNFBW   {#}S
 *
 */
void P2Dasm::dasm_jnfbw()
{
    format_imm_s(t_JNFBW);
}

/**
 * @brief Jump to S** if XMT event flag is clear.
 *
 * EEEE 1011110 01I 000011010 SSSSSSSSS
 *
 * JNXMT   {#}S
 *
 */
void P2Dasm::dasm_jnxmt()
{
    format_imm_s(t_JNXMT);
}

/**
 * @brief Jump to S** if XFI event flag is clear.
 *
 * EEEE 1011110 01I 000011011 SSSSSSSSS
 *
 * JNXFI   {#}S
 *
 */
void P2Dasm::dasm_jnxfi()
{
    format_imm_s(t_JNXFI);
}

/**
 * @brief Jump to S** if XRO event flag is clear.
 *
 * EEEE 1011110 01I 000011100 SSSSSSSSS
 *
 * JNXRO   {#}S
 *
 */
void P2Dasm::dasm_jnxro()
{
    format_imm_s(t_JNXRO);
}

/**
 * @brief Jump to S** if XRL event flag is clear.
 *
 * EEEE 1011110 01I 000011101 SSSSSSSSS
 *
 * JNXRL   {#}S
 *
 */
void P2Dasm::dasm_jnxrl()
{
    format_imm_s(t_JNXRL);
}

/**
 * @brief Jump to S** if ATN event flag is clear.
 *
 * EEEE 1011110 01I 000011110 SSSSSSSSS
 *
 * JNATN   {#}S
 *
 */
void P2Dasm::dasm_jnatn()
{
    format_imm_s(t_JNATN);
}

/**
 * @brief Jump to S** if QMT event flag is clear.
 *
 * EEEE 1011110 01I 000011111 SSSSSSSSS
 *
 * JNQMT   {#}S
 *
 */
void P2Dasm::dasm_jnqmt()
{
    format_imm_s(t_JNQMT);
}

/**
 * @brief <empty>.
 *
 * EEEE 1011110 1LI DDDDDDDDD SSSSSSSSS
 *
 * <empty> {#}D,{#}S
 *
 */
void P2Dasm::dasm_1011110_1()
{
    format_wz_d_imm_s(t_empty);
}

/**
 * @brief <empty>.
 *
 * EEEE 1011111 0LI DDDDDDDDD SSSSSSSSS
 *
 * <empty> {#}D,{#}S
 *
 */
void P2Dasm::dasm_1011111_0()
{
    format_wz_d_imm_s(t_empty);
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
void P2Dasm::dasm_setpat()
{
    format_wz_d_imm_s(t_SETPAT);
}

/**
 * @brief Write D to mode register of smart pin S[5:0], acknowledge smart pin.
 *
 * EEEE 1100000 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRPIN   {#}D,{#}S
 *
 */
void P2Dasm::dasm_wrpin()
{
    format_wz_d_imm_s(t_WRPIN);
}

/**
 * @brief Acknowledge smart pin S[5:0].
 *
 * EEEE 1100000 01I 000000001 SSSSSSSSS
 *
 * AKPIN   {#}S
 *
 */
void P2Dasm::dasm_akpin()
{
    format_imm_s(t_AKPIN);
}

/**
 * @brief Write D to parameter "X" of smart pin S[5:0], acknowledge smart pin.
 *
 * EEEE 1100000 1LI DDDDDDDDD SSSSSSSSS
 *
 * WXPIN   {#}D,{#}S
 *
 */
void P2Dasm::dasm_wxpin()
{
    format_wz_d_imm_s(t_WXPIN);
}

/**
 * @brief Write D to parameter "Y" of smart pin S[5:0], acknowledge smart pin.
 *
 * EEEE 1100001 0LI DDDDDDDDD SSSSSSSSS
 *
 * WYPIN   {#}D,{#}S
 *
 */
void P2Dasm::dasm_wypin()
{
    format_wz_d_imm_s(t_WYPIN);
}

/**
 * @brief Write D to LUT address S[8:0].
 *
 * EEEE 1100001 1LI DDDDDDDDD SSSSSSSSS
 *
 * WRLUT   {#}D,{#}S
 *
 */
void P2Dasm::dasm_wrlut()
{
    format_wz_d_imm_s(t_WRLUT);
}

/**
 * @brief Write byte in D[7:0] to hub address {#}S/PTRx.
 *
 * EEEE 1100010 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRBYTE  {#}D,{#}S/P
 *
 */
void P2Dasm::dasm_wrbyte()
{
    format_wz_d_imm_s(t_WRBYTE);
}

/**
 * @brief Write word in D[15:0] to hub address {#}S/PTRx.
 *
 * EEEE 1100010 1LI DDDDDDDDD SSSSSSSSS
 *
 * WRWORD  {#}D,{#}S/P
 *
 */
void P2Dasm::dasm_wrword()
{
    format_wz_d_imm_s(t_WRWORD);
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
void P2Dasm::dasm_wrlong()
{
    format_wz_d_imm_s(t_WRLONG);
}

/**
 * @brief Write long in D[31:0] to hub address PTRA++.
 *
 * EEEE 1100011 0L1 DDDDDDDDD 101100001
 *
 * PUSHA   {#}D
 *
 */
void P2Dasm::dasm_pusha()
{
    format_wz_d(t_PUSHA);
}

/**
 * @brief Write long in D[31:0] to hub address PTRB++.
 *
 * EEEE 1100011 0L1 DDDDDDDDD 111100001
 *
 * PUSHB   {#}D
 *
 */
void P2Dasm::dasm_pushb()
{
    format_wz_d(t_PUSHB);
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
void P2Dasm::dasm_rdfast()
{
    format_wz_d_imm_s(t_RDFAST);
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
void P2Dasm::dasm_wrfast()
{
    format_wz_d_imm_s(t_WRFAST);
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
void P2Dasm::dasm_fblock()
{
    format_wz_d_imm_s(t_FBLOCK);
}

/**
 * @brief Issue streamer command immediately, zeroing phase.
 *
 * EEEE 1100101 0LI DDDDDDDDD SSSSSSSSS
 *
 * XINIT   {#}D,{#}S
 *
 */
void P2Dasm::dasm_xinit()
{
    format_wz_d_imm_s(t_XINIT);
}

/**
 * @brief Stop streamer immediately.
 *
 * EEEE 1100101 011 000000000 000000000
 *
 * XSTOP
 *
 */
void P2Dasm::dasm_xstop()
{
    format_inst(t_XSTOP);
}

/**
 * @brief Buffer new streamer command to be issued on final NCO rollover of current command, zeroing phase.
 *
 * EEEE 1100101 1LI DDDDDDDDD SSSSSSSSS
 *
 * XZERO   {#}D,{#}S
 *
 */
void P2Dasm::dasm_xzero()
{
    format_wz_d_imm_s(t_XZERO);
}

/**
 * @brief Buffer new streamer command to be issued on final NCO rollover of current command, continuing phase.
 *
 * EEEE 1100110 0LI DDDDDDDDD SSSSSSSSS
 *
 * XCONT   {#}D,{#}S
 *
 */
void P2Dasm::dasm_xcont()
{
    format_wz_d_imm_s(t_XCONT);
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
void P2Dasm::dasm_rep()
{
    format_wz_d_imm_s(t_REP);
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
void P2Dasm::dasm_coginit()
{
    format_wz_d_imm_s(t_COGINIT);
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
void P2Dasm::dasm_qmul()
{
    format_wz_d_imm_s(t_QMUL);
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
void P2Dasm::dasm_qdiv()
{
    format_wz_d_imm_s(t_QDIV);
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
void P2Dasm::dasm_qfrac()
{
    format_wz_d_imm_s(t_QFRAC);
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
void P2Dasm::dasm_qsqrt()
{
    format_wz_d_imm_s(t_QSQRT);
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
void P2Dasm::dasm_qrotate()
{
    format_wz_d_imm_s(t_QROTATE);
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
void P2Dasm::dasm_qvector()
{
    format_wz_d_imm_s(t_QVECTOR);
}

/**
 * @brief Set hub configuration to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000000000
 *
 * HUBSET  {#}D
 *
 */
void P2Dasm::dasm_hubset()
{
    format_imm_d(t_HUBSET);
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
void P2Dasm::dasm_cogid()
{
    format_imm_d_c(t_COGID);
}

/**
 * @brief Stop cog D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000000011
 *
 * COGSTOP {#}D
 *
 */
void P2Dasm::dasm_cogstop()
{
    format_imm_d(t_COGSTOP);
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
void P2Dasm::dasm_locknew()
{
    format_imm_d_c(t_LOCKNEW);
}

/**
 * @brief Return LOCK D[3:0] for reallocation.
 *
 * EEEE 1101011 00L DDDDDDDDD 000000101
 *
 * LOCKRET {#}D
 *
 */
void P2Dasm::dasm_lockret()
{
    format_imm_d(t_LOCKRET);
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
void P2Dasm::dasm_locktry()
{
    format_imm_d_c(t_LOCKTRY);
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
void P2Dasm::dasm_lockrel()
{
    format_imm_d_c(t_LOCKREL);
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
void P2Dasm::dasm_qlog()
{
    format_imm_d(t_QLOG);
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
void P2Dasm::dasm_qexp()
{
    format_imm_d(t_QEXP);
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
void P2Dasm::dasm_rfbyte()
{
    format_d_cz(t_RFBYTE);
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
void P2Dasm::dasm_rfword()
{
    format_d_cz(t_RFWORD);
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
void P2Dasm::dasm_rflong()
{
    format_d_cz(t_RFLONG);
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
void P2Dasm::dasm_rfvar()
{
    format_d_cz(t_RFVAR);
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
void P2Dasm::dasm_rfvars()
{

    format_d_cz(t_RFVARS);
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
void P2Dasm::dasm_wfbyte()
{
    format_imm_d(t_WFBYTE);
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
void P2Dasm::dasm_wfword()
{
    format_imm_d(t_WFWORD);
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
void P2Dasm::dasm_wflong()
{
    format_imm_d(t_WFLONG);
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
void P2Dasm::dasm_getqx()
{
    format_d_cz(t_GETQX);
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
void P2Dasm::dasm_getqy()
{
    format_d_cz(t_GETQY);
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
void P2Dasm::dasm_getct()
{
    format_d(t_GETCT);
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
void P2Dasm::dasm_getrnd()
{
    format_d_cz(t_GETRND);
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
void P2Dasm::dasm_getrnd_cz()
{
    format_cz(t_GETRND);
}

/**
 * @brief DAC3 = D[31:24], DAC2 = D[23:16], DAC1 = D[15:8], DAC0 = D[7:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000011100
 *
 * SETDACS {#}D
 *
 */
void P2Dasm::dasm_setdacs()
{
    format_imm_d(t_SETDACS);
}

/**
 * @brief Set streamer NCO frequency to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000011101
 *
 * SETXFRQ {#}D
 *
 */
void P2Dasm::dasm_setxfrq()
{
    format_imm_d(t_SETXFRQ);
}

/**
 * @brief Get the streamer's Goertzel X accumulator into D and the Y accumulator into the next instruction's S, clear accumulators.
 *
 * EEEE 1101011 000 DDDDDDDDD 000011110
 *
 * GETXACC D
 *
 */
void P2Dasm::dasm_getxacc()
{
    format_d(t_GETXACC);
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
void P2Dasm::dasm_waitx()
{
    format_imm_d_cz(t_WAITX);
}

/**
 * @brief Set SE1 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100000
 *
 * SETSE1  {#}D
 *
 */
void P2Dasm::dasm_setse1()
{
    format_imm_d(t_SETSE1);
}

/**
 * @brief Set SE2 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100001
 *
 * SETSE2  {#}D
 *
 */
void P2Dasm::dasm_setse2()
{
    format_imm_d(t_SETSE2);
}

/**
 * @brief Set SE3 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100010
 *
 * SETSE3  {#}D
 *
 */
void P2Dasm::dasm_setse3()
{
    format_imm_d(t_SETSE3);
}

/**
 * @brief Set SE4 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100011
 *
 * SETSE4  {#}D
 *
 */
void P2Dasm::dasm_setse4()
{
    format_imm_d(t_SETSE4);
}

/**
 * @brief Get INT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000000 000100100
 *
 * POLLINT          {WC/WZ/WCZ}
 *
 */
void P2Dasm::dasm_pollint()
{
    format_cz(t_POLLINT);
}

/**
 * @brief Get CT1 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000001 000100100
 *
 * POLLCT1          {WC/WZ/WCZ}
 *
 */
void P2Dasm::dasm_pollct1()
{
    format_cz(t_POLLCT1);
}

/**
 * @brief Get CT2 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000010 000100100
 *
 * POLLCT2          {WC/WZ/WCZ}
 *
 */
void P2Dasm::dasm_pollct2()
{
    format_cz(t_POLLCT2);
}

/**
 * @brief Get CT3 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000011 000100100
 *
 * POLLCT3          {WC/WZ/WCZ}
 *
 */
void P2Dasm::dasm_pollct3()
{
    format_cz(t_POLLCT3);
}

/**
 * @brief Get SE1 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000100 000100100
 *
 * POLLSE1          {WC/WZ/WCZ}
 *
 */
void P2Dasm::dasm_pollse1()
{
    format_cz(t_POLLSE1);
}

/**
 * @brief Get SE2 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000101 000100100
 *
 * POLLSE2          {WC/WZ/WCZ}
 *
 */
void P2Dasm::dasm_pollse2()
{
    format_cz(t_POLLSE2);
}

/**
 * @brief Get SE3 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000110 000100100
 *
 * POLLSE3          {WC/WZ/WCZ}
 *
 */
void P2Dasm::dasm_pollse3()
{
    format_cz(t_POLLSE3);
}

/**
 * @brief Get SE4 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000111 000100100
 *
 * POLLSE4          {WC/WZ/WCZ}
 *
 */
void P2Dasm::dasm_pollse4()
{
    format_cz(t_POLLSE4);
}

/**
 * @brief Get PAT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001000 000100100
 *
 * POLLPAT          {WC/WZ/WCZ}
 *
 */
void P2Dasm::dasm_pollpat()
{
    format_cz(t_POLLPAT);
}

/**
 * @brief Get FBW event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001001 000100100
 *
 * POLLFBW          {WC/WZ/WCZ}
 *
 */
void P2Dasm::dasm_pollfbw()
{
    format_cz(t_POLLFBW);
}

/**
 * @brief Get XMT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001010 000100100
 *
 * POLLXMT          {WC/WZ/WCZ}
 *
 */
void P2Dasm::dasm_pollxmt()
{
    format_cz(t_POLLXMT);
}

/**
 * @brief Get XFI event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001011 000100100
 *
 * POLLXFI          {WC/WZ/WCZ}
 *
 */
void P2Dasm::dasm_pollxfi()
{
    format_cz(t_POLLXFI);
}

/**
 * @brief Get XRO event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001100 000100100
 *
 * POLLXRO          {WC/WZ/WCZ}
 *
 */
void P2Dasm::dasm_pollxro()
{
    format_cz(t_POLLXRO);
}

/**
 * @brief Get XRL event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001101 000100100
 *
 * POLLXRL          {WC/WZ/WCZ}
 *
 */
void P2Dasm::dasm_pollxrl()
{
    format_cz(t_POLLXRL);
}

/**
 * @brief Get ATN event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001110 000100100
 *
 * POLLATN          {WC/WZ/WCZ}
 *
 */
void P2Dasm::dasm_pollatn()
{
    format_cz(t_POLLATN);
}

/**
 * @brief Get QMT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001111 000100100
 *
 * POLLQMT          {WC/WZ/WCZ}
 *
 */
void P2Dasm::dasm_pollqmt()
{
    format_cz(t_POLLQMT);
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
void P2Dasm::dasm_waitint()
{
    format_cz(t_WAITINT);
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
void P2Dasm::dasm_waitct1()
{
    format_cz(t_WAITCT1);
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
void P2Dasm::dasm_waitct2()
{
    format_cz(t_WAITCT2);
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
void P2Dasm::dasm_waitct3()
{
    format_cz(t_WAITCT3);
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
void P2Dasm::dasm_waitse1()
{
    format_cz(t_WAITSE1);
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
void P2Dasm::dasm_waitse2()
{
    format_cz(t_WAITSE2);
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
void P2Dasm::dasm_waitse3()
{
    format_cz(t_WAITSE3);
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
void P2Dasm::dasm_waitse4()
{
    format_cz(t_WAITSE4);
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
void P2Dasm::dasm_waitpat()
{
    format_cz(t_WAITPAT);
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
void P2Dasm::dasm_waitfbw()
{
    format_cz(t_WAITFBW);
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
void P2Dasm::dasm_waitxmt()
{
    format_cz(t_WAITXMT);
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
void P2Dasm::dasm_waitxfi()
{
    format_cz(t_WAITXFI);
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
void P2Dasm::dasm_waitxro()
{
    format_cz(t_WAITXRO);
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
void P2Dasm::dasm_waitxrl()
{
    format_cz(t_WAITXRL);
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
void P2Dasm::dasm_waitatn()
{
    format_cz(t_WAITATN);
}

/**
 * @brief Allow interrupts (default).
 *
 * EEEE 1101011 000 000100000 000100100
 *
 * ALLOWI
 *
 */
void P2Dasm::dasm_allowi()
{
    format_inst(t_ALLOWI);
}

/**
 * @brief Stall Interrupts.
 *
 * EEEE 1101011 000 000100001 000100100
 *
 * STALLI
 *
 */
void P2Dasm::dasm_stalli()
{
    format_inst(t_STALLI);
}

/**
 * @brief Trigger INT1, regardless of STALLI mode.
 *
 * EEEE 1101011 000 000100010 000100100
 *
 * TRGINT1
 *
 */
void P2Dasm::dasm_trgint1()
{
    format_inst(t_TRGINT1);
}

/**
 * @brief Trigger INT2, regardless of STALLI mode.
 *
 * EEEE 1101011 000 000100011 000100100
 *
 * TRGINT2
 *
 */
void P2Dasm::dasm_trgint2()
{
    format_inst(t_TRGINT2);
}

/**
 * @brief Trigger INT3, regardless of STALLI mode.
 *
 * EEEE 1101011 000 000100100 000100100
 *
 * TRGINT3
 *
 */
void P2Dasm::dasm_trgint3()
{
    format_inst(t_TRGINT3);
}

/**
 * @brief Cancel INT1.
 *
 * EEEE 1101011 000 000100101 000100100
 *
 * NIXINT1
 *
 */
void P2Dasm::dasm_nixint1()
{
    format_inst(t_NIXINT1);
}

/**
 * @brief Cancel INT2.
 *
 * EEEE 1101011 000 000100110 000100100
 *
 * NIXINT2
 *
 */
void P2Dasm::dasm_nixint2()
{
    format_inst(t_NIXINT2);
}

/**
 * @brief Cancel INT3.
 *
 * EEEE 1101011 000 000100111 000100100
 *
 * NIXINT3
 *
 */
void P2Dasm::dasm_nixint3()
{
    format_inst(t_NIXINT3);
}

/**
 * @brief Set INT1 source to D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100101
 *
 * SETINT1 {#}D
 *
 */
void P2Dasm::dasm_setint1()
{
    format_imm_d(t_SETINT1);
}

/**
 * @brief Set INT2 source to D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100110
 *
 * SETINT2 {#}D
 *
 */
void P2Dasm::dasm_setint2()
{
    format_imm_d(t_SETINT2);
}

/**
 * @brief Set INT3 source to D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100111
 *
 * SETINT3 {#}D
 *
 */
void P2Dasm::dasm_setint3()
{
    format_imm_d(t_SETINT3);
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
void P2Dasm::dasm_setq()
{
    format_imm_d(t_SETQ);
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
void P2Dasm::dasm_setq2()
{
    format_imm_d(t_SETQ2);
}

/**
 * @brief Push D onto stack.
 *
 * EEEE 1101011 00L DDDDDDDDD 000101010
 *
 * PUSH    {#}D
 *
 */
void P2Dasm::dasm_push()
{
    format_imm_d(t_PUSH);
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
void P2Dasm::dasm_pop()
{
    format_d_cz(t_POP);
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
void P2Dasm::dasm_jmp()
{
    format_d_cz(t_JMP);
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
void P2Dasm::dasm_call()
{
    format_d_cz(t_CALL);
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
void P2Dasm::dasm_ret()
{
    format_cz(t_RET);
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
void P2Dasm::dasm_calla()
{
    format_d_cz(t_CALLA);
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
void P2Dasm::dasm_reta()
{
    format_cz(t_RETA);
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
void P2Dasm::dasm_callb()
{
    format_d_cz(t_CALLB);
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
void P2Dasm::dasm_retb()
{
    format_cz(t_RETB);
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
void P2Dasm::dasm_jmprel()
{
    format_imm_d(t_JMPREL);
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
void P2Dasm::dasm_skip()
{
    format_imm_d(t_SKIP);
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
void P2Dasm::dasm_skipf()
{
    format_imm_d(t_SKIPF);
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
void P2Dasm::dasm_execf()
{
    format_imm_d(t_EXECF);
}

/**
 * @brief Get current FIFO hub pointer into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 000110100
 *
 * GETPTR  D
 *
 */
void P2Dasm::dasm_getptr()
{
    format_d(t_GETPTR);
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
void P2Dasm::dasm_getbrk()
{
    format_d_cz(t_GETBRK);
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
void P2Dasm::dasm_cogbrk()
{
    format_imm_d(t_COGBRK);
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
void P2Dasm::dasm_brk()
{
    format_imm_d(t_BRK);
}

/**
 * @brief If D[0] = 1 then enable LUT sharing, where LUT writes within the adjacent odd/even companion cog are copied to this LUT.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110111
 *
 * SETLUTS {#}D
 *
 */
void P2Dasm::dasm_setluts()
{
    format_imm_d(t_SETLUTS);
}

/**
 * @brief Set the colorspace converter "CY" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111000
 *
 * SETCY   {#}D
 *
 */
void P2Dasm::dasm_setcy()
{
    format_imm_d(t_SETCY);
}

/**
 * @brief Set the colorspace converter "CI" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111001
 *
 * SETCI   {#}D
 *
 */
void P2Dasm::dasm_setci()
{
    format_imm_d(t_SETCI);
}

/**
 * @brief Set the colorspace converter "CQ" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111010
 *
 * SETCQ   {#}D
 *
 */
void P2Dasm::dasm_setcq()
{
    format_imm_d(t_SETCQ);
}

/**
 * @brief Set the colorspace converter "CFRQ" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111011
 *
 * SETCFRQ {#}D
 *
 */
void P2Dasm::dasm_setcfrq()
{
    format_imm_d(t_SETCFRQ);
}

/**
 * @brief Set the colorspace converter "CMOD" parameter to D[6:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111100
 *
 * SETCMOD {#}D
 *
 */
void P2Dasm::dasm_setcmod()
{
    format_imm_d(t_SETCMOD);
}

/**
 * @brief Set BLNPIX/MIXPIX blend factor to D[7:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111101
 *
 * SETPIV  {#}D
 *
 */
void P2Dasm::dasm_setpiv()
{
    format_imm_d(t_SETPIV);
}

/**
 * @brief Set MIXPIX mode to D[5:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111110
 *
 * SETPIX  {#}D
 *
 */
void P2Dasm::dasm_setpix()
{
    format_imm_d(t_SETPIX);
}

/**
 * @brief Strobe "attention" of all cogs whose corresponging bits are high in D[15:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111111
 *
 * COGATN  {#}D
 *
 */
void P2Dasm::dasm_cogatn()
{
    format_imm_d(t_COGATN);
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
void P2Dasm::dasm_testp_w()
{
    format_imm_d_cz(t_TESTP);
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
void P2Dasm::dasm_testpn_w()
{
    format_imm_d_cz(t_TESTPN);
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
void P2Dasm::dasm_testp_and()
{
    format_imm_d_cz(t_TESTP, t_AND);
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
void P2Dasm::dasm_testpn_and()
{
    format_imm_d_cz(t_TESTPN, t_AND);
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
void P2Dasm::dasm_testp_or()
{
    format_imm_d_cz(t_TESTP, t_OR);
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
void P2Dasm::dasm_testpn_or()
{
    format_imm_d_cz(t_TESTPN, t_OR);
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
void P2Dasm::dasm_testp_xor()
{
    format_imm_d_cz(t_TESTP, t_XOR);
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
void P2Dasm::dasm_testpn_xor()
{
    format_imm_d_cz(t_TESTPN, t_XOR);
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
void P2Dasm::dasm_dirl()
{
    format_imm_d_cz(t_DIRL);
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
void P2Dasm::dasm_dirh()
{
    format_imm_d_cz(t_DIRH);
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
void P2Dasm::dasm_dirc()
{
    format_imm_d_cz(t_DIRC);
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
void P2Dasm::dasm_dirnc()
{
    format_imm_d_cz(t_DIRNC);
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
void P2Dasm::dasm_dirz()
{
    format_imm_d_cz(t_DIRZ);
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
void P2Dasm::dasm_dirnz()
{
    format_imm_d_cz(t_DIRNZ);
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
void P2Dasm::dasm_dirrnd()
{
    format_imm_d_cz(t_DIRRND);
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
void P2Dasm::dasm_dirnot()
{
    format_imm_d_cz(t_DIRNOT);
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
void P2Dasm::dasm_outl()
{
    format_imm_d_cz(t_OUTL);
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
void P2Dasm::dasm_outh()
{
    format_imm_d_cz(t_OUTH);
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
void P2Dasm::dasm_outc()
{
    format_imm_d_cz(t_OUTC);
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
void P2Dasm::dasm_outnc()
{
    format_imm_d_cz(t_OUTNC);
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
void P2Dasm::dasm_outz()
{
    format_imm_d_cz(t_OUTZ);
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
void P2Dasm::dasm_outnz()
{
    format_imm_d_cz(t_OUTNZ);
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
void P2Dasm::dasm_outrnd()
{
    format_imm_d_cz(t_OUTRND);
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
void P2Dasm::dasm_outnot()
{
    format_imm_d_cz(t_OUTNOT);
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
void P2Dasm::dasm_fltl()
{
    format_imm_d_cz(t_FLTL);
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
void P2Dasm::dasm_flth()
{
    format_imm_d_cz(t_FLTH);
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
void P2Dasm::dasm_fltc()
{
    format_imm_d_cz(t_FLTC);
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
void P2Dasm::dasm_fltnc()
{
    format_imm_d_cz(t_FLTNC);
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
void P2Dasm::dasm_fltz()
{
    format_imm_d_cz(t_FLTZ);
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
void P2Dasm::dasm_fltnz()
{
    format_imm_d_cz(t_FLTNZ);
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
void P2Dasm::dasm_fltrnd()
{
    format_imm_d_cz(t_FLTRND);
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
void P2Dasm::dasm_fltnot()
{
    format_imm_d_cz(t_FLTNOT);
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
void P2Dasm::dasm_drvl()
{
    format_imm_d_cz(t_DRVL);
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
void P2Dasm::dasm_drvh()
{
    format_imm_d_cz(t_DRVH);
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
void P2Dasm::dasm_drvc()
{
    format_imm_d_cz(t_DRVC);
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
void P2Dasm::dasm_drvnc()
{
    format_imm_d_cz(t_DRVNC);
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
void P2Dasm::dasm_drvz()
{
    format_imm_d_cz(t_DRVZ);
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
void P2Dasm::dasm_drvnz()
{
    format_imm_d_cz(t_DRVNZ);
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
void P2Dasm::dasm_drvrnd()
{
    format_imm_d_cz(t_DRVRND);
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
void P2Dasm::dasm_drvnot()
{
    format_imm_d_cz(t_DRVNOT);
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
void P2Dasm::dasm_splitb()
{
    format_d(t_SPLITB);
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
void P2Dasm::dasm_mergeb()
{
    format_d(t_MERGEB);
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
void P2Dasm::dasm_splitw()
{
    format_d(t_SPLITW);
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
void P2Dasm::dasm_mergew()
{
    format_d(t_MERGEW);
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
void P2Dasm::dasm_seussf()
{
    format_d(t_SEUSSF);
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
void P2Dasm::dasm_seussr()
{
    format_d(t_SEUSSR);
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
void P2Dasm::dasm_rgbsqz()
{
    format_d(t_RGBSQZ);
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
void P2Dasm::dasm_rgbexp()
{
    format_d(t_RGBEXP);
}

/**
 * @brief Iterate D with xoroshiro32+ PRNG algorithm and put PRNG result into next instruction's S.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101000
 *
 * XORO32  D
 *
 */
void P2Dasm::dasm_xoro32()
{
    format_d(t_XORO32);
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
void P2Dasm::dasm_rev()
{
    format_d(t_REV);
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
void P2Dasm::dasm_rczr()
{
    format_d_cz(t_RCZR);
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
void P2Dasm::dasm_rczl()
{
    format_d_cz(t_RCZL);
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
void P2Dasm::dasm_wrc()
{
    format_d(t_WRC);
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
void P2Dasm::dasm_wrnc()
{
    format_d(t_WRNC);
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
void P2Dasm::dasm_wrz()
{
    format_d(t_WRZ);
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
void P2Dasm::dasm_wrnz()
{
    format_d(t_WRNZ);
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
void P2Dasm::dasm_modcz()
{
    format_cz_cz(t_MODCZ);
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
void P2Dasm::dasm_jmp_abs()
{
    format_pc_abs(t_JMP);
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
void P2Dasm::dasm_call_abs()
{
    format_pc_abs(t_CALL);
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
void P2Dasm::dasm_calla_abs()
{
    format_pc_abs(t_CALLA);
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
void P2Dasm::dasm_callb_abs()
{
    format_pc_abs(t_CALLB);
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
void P2Dasm::dasm_calld_abs()
{
    format_pc_abs(t_CALLD);
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
void P2Dasm::dasm_loc_pa()
{
    format_pc_abs(t_LOC, t_PA);
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
void P2Dasm::dasm_loc_pb()
{
    format_pc_abs(t_LOC, t_PB);
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
void P2Dasm::dasm_loc_ptra()
{
    format_pc_abs(t_LOC, t_PTRA);
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
void P2Dasm::dasm_loc_ptrb()
{
    format_pc_abs(t_LOC, t_PTRB);
}

/**
 * @brief Queue #N[31:9] to be used as upper 23 bits for next #S occurrence, so that the next 9-bit #S will be augmented to 32 bits.
 *
 * EEEE 11110NN NNN NNNNNNNNN NNNNNNNNN
 *
 * AUGS    #N
 *
 */
void P2Dasm::dasm_augs()
{
    format_imm23(t_AUGS);
}

/**
 * @brief Queue #N[31:9] to be used as upper 23 bits for next #D occurrence, so that the next 9-bit #D will be augmented to 32 bits.
 *
 * EEEE 11111NN NNN NNNNNNNNN NNNNNNNNN
 *
 * AUGD    #N
 *
 */
void P2Dasm::dasm_augd()
{
    format_imm23(t_AUGD);
}
