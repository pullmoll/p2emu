#include "p2_defs.h"
#include "p2cog.h"
#include "p2dasm.h"

P2Dasm::P2Dasm()
    : p2Token(new P2Token(false))
{
}

P2Dasm::~P2Dasm()
{
    delete p2Token;
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
    case cond_never:
        return t_NEVER;
    }
    return t_invalid;
}

p2_token_e P2Dasm::conditional(unsigned cond)
{
    return conditional(static_cast<p2_cond_e>(cond));
}

QString P2Dasm::dasm(P2Cog *COG, quint32 PC)
{
    QString str;

    if (PC < 0x0200) {
        // cogexec
        IR.word = COG->rd_cog(PC);
    } else if (PC < 0x0400) {
        // lutexec
        IR.word = COG->rd_lut(PC);
    } else {
        // hubexec
        IR.word = COG->rd_mem(PC);
    }

    PC++;               // increment PC
    S = COG->rd_cog(IR.op.src);
    D = COG->rd_cog(IR.op.dst);

    // check for the condition
    str = p2Token->str(conditional(IR.op.cond));

    // Dispatch to dasm_xxx() functions
    switch (IR.op.inst) {
    case p2_ror:
        str = dasm_ror();
        break;

    case p2_rol:
        str = dasm_rol();
        break;

    case p2_shr:
        str = dasm_shr();
        break;

    case p2_shl:
        str = dasm_shl();
        break;

    case p2_rcr:
        str = dasm_rcr();
        break;

    case p2_rcl:
        str = dasm_rcl();
        break;

    case p2_sar:
        str = dasm_sar();
        break;

    case p2_sal:
        str = dasm_sal();
        break;

    case p2_add:
        str = dasm_add();
        break;

    case p2_addx:
        str = dasm_addx();
        break;

    case p2_adds:
        str = dasm_adds();
        break;

    case p2_addsx:
        str = dasm_addsx();
        break;

    case p2_sub:
        str = dasm_sub();
        break;

    case p2_subx:
        str = dasm_subx();
        break;

    case p2_subs:
        str = dasm_subs();
        break;

    case p2_subsx:
        str = dasm_subsx();
        break;

    case p2_cmp:
        str = dasm_cmp();
        break;

    case p2_cmpx:
        str = dasm_cmpx();
        break;

    case p2_cmps:
        str = dasm_cmps();
        break;

    case p2_cmpsx:
        str = dasm_cmpsx();
        break;

    case p2_cmpr:
        str = dasm_cmpr();
        break;

    case p2_cmpm:
        str = dasm_cmpm();
        break;

    case p2_subr:
        str = dasm_subr();
        break;

    case p2_cmpsub:
        str = dasm_cmpsub();
        break;

    case p2_fge:
        str = dasm_fge();
        break;

    case p2_fle:
        str = dasm_fle();
        break;

    case p2_fges:
        str = dasm_fges();
        break;

    case p2_fles:
        str = dasm_fles();
        break;

    case p2_sumc:
        str = dasm_sumc();
        break;

    case p2_sumnc:
        str = dasm_sumnc();
        break;

    case p2_sumz:
        str = dasm_sumz();
        break;

    case p2_sumnz:
        str = dasm_sumnz();
        break;

    case p2_testb_w:
        // case p2_bitl:
        str = (IR.op.uc != IR.op.uz) ? dasm_testb_w()
                                        : dasm_bitl();
        break;

    case p2_testbn_w:
        // case p2_bith:
        str = (IR.op.uc != IR.op.uz) ? dasm_testbn_w()
                                        : dasm_bith();
        break;

    case p2_testb_and:
        // case p2_bitc:
        str = (IR.op.uc != IR.op.uz) ? dasm_testb_and()
                                        : dasm_bitc();
        break;

    case p2_testbn_and:
        // case p2_bitnc:
        str = (IR.op.uc != IR.op.uz) ? dasm_testbn_and()
                                        : dasm_bitnc();
        break;

    case p2_testb_or:
        // case p2_bitz:
        str = (IR.op.uc != IR.op.uz) ? dasm_testb_or()
                                        : dasm_bitz();
        break;

    case p2_testbn_or:
        // case p2_bitnz:
        str = (IR.op.uc != IR.op.uz) ? dasm_testbn_or()
                                        : dasm_bitnz();
        break;

    case p2_testb_xor:
        // case p2_bitrnd:
        str = (IR.op.uc != IR.op.uz) ? dasm_testb_xor()
                                        : dasm_bitrnd();
        break;

    case p2_testbn_xor:
        // case p2_bitnot:
        str = (IR.op.uc != IR.op.uz) ? dasm_testbn_xor()
                                        : dasm_bitnot();
        break;

    case p2_and:
        str = dasm_and();
        break;

    case p2_andn:
        str = dasm_andn();
        break;

    case p2_or:
        str = dasm_or();
        break;

    case p2_xor:
        str = dasm_xor();
        break;

    case p2_muxc:
        str = dasm_muxc();
        break;

    case p2_muxnc:
        str = dasm_muxnc();
        break;

    case p2_muxz:
        str = dasm_muxz();
        break;

    case p2_muxnz:
        str = dasm_muxnz();
        break;

    case p2_mov:
        str = dasm_mov();
        break;

    case p2_not:
        str = dasm_not();
        break;

    case p2_abs:
        str = dasm_abs();
        break;

    case p2_neg:
        str = dasm_neg();
        break;

    case p2_negc:
        str = dasm_negc();
        break;

    case p2_negnc:
        str = dasm_negnc();
        break;

    case p2_negz:
        str = dasm_negz();
        break;

    case p2_negnz:
        str = dasm_negnz();
        break;

    case p2_incmod:
        str = dasm_incmod();
        break;

    case p2_decmod:
        str = dasm_decmod();
        break;

    case p2_zerox:
        str = dasm_zerox();
        break;

    case p2_signx:
        str = dasm_signx();
        break;

    case p2_encod:
        str = dasm_encod();
        break;

    case p2_ones:
        str = dasm_ones();
        break;

    case p2_test:
        str = dasm_test();
        break;

    case p2_testn:
        str = dasm_testn();
        break;

    case p2_setnib_0:
    case p2_setnib_1:
        str = dasm_setnib();
        break;

    case p2_getnib_0:
    case p2_getnib_1:
        str = dasm_getnib();
        break;

    case p2_rolnib_0:
    case p2_rolnib_1:
        str = dasm_rolnib();
        break;

    case p2_setbyte:
        str = dasm_setbyte();
        break;

    case p2_getbyte:
        str = dasm_getbyte();
        break;

    case p2_rolbyte:
        str = dasm_rolbyte();
        break;

    case p2_1001001:
        if (IR.op.uc == 0) {
            str = (IR.op.dst == 0 && IR.op.uz == 0) ? dasm_setword_altsw()
                                                       : dasm_setword();
        } else {
            str = (IR.op.src == 0 && IR.op.uz == 0) ? dasm_getword_altgw()
                                                       : dasm_getword();
        }
        break;

    case p2_1001010:
        if (IR.op.uc == 0) {
            str = (IR.op.src == 0 && IR.op.uz == 0) ? dasm_rolword_altgw()
                                                       : dasm_rolword();
        } else {
            if (IR.op.uz == 0) {
                str = (IR.op.src == 0 && IR.op.imm == 1) ? dasm_altsn_d()
                                                            : dasm_altsn();
            } else {
                str = (IR.op.src == 0 && IR.op.imm == 1) ? dasm_altgn_d()
                                                            : dasm_altgn();
            }
        }
        break;

    case p2_1001011:
        if (IR.op.uc == 0) {
            if (IR.op.uz == 0) {
                str = (IR.op.src == 0 && IR.op.imm == 1) ? dasm_altsb_d()
                                                            : dasm_altsb();
            } else {
                str = (IR.op.src == 0 && IR.op.imm == 1) ? dasm_altgb_d()
                                                            : dasm_altgb();
            }
        } else {
            if (IR.op.uz == 0) {
                str = (IR.op.src == 0 && IR.op.imm == 1) ? dasm_altsw_d()
                                                            : dasm_altsw();
            } else {
                str = (IR.op.src == 0 && IR.op.imm == 1) ? dasm_altgw_d()
                                                            : dasm_altgw();
            }
        }
        break;

    case p2_1001100:
        if (IR.op.uc == 0) {
            if (IR.op.uz == 0) {
                str = (IR.op.src == 0 && IR.op.imm == 1) ? dasm_altr_d()
                                                            : dasm_altr();
            } else {
                str = (IR.op.src == 0 && IR.op.imm == 1) ? dasm_altd_d()
                                                            : dasm_altd();
            }
        } else {
            if (IR.op.uz == 0) {
                str = (IR.op.src == 0 && IR.op.imm == 1) ? dasm_alts_d()
                                                            : dasm_alts();
            } else {
                str = (IR.op.src == 0 && IR.op.imm == 1) ? dasm_altb_d()
                                                            : dasm_altb();
            }
        }
        break;

    case p2_1001101:
        if (IR.op.uc == 0) {
            if (IR.op.uz == 0) {
                str = (IR.op.imm == 1 && IR.op.src == 0x164 /* 101100100 */) ? dasm_alti_d()
                                                                                : dasm_alti();
            } else {
                str = dasm_setr();
            }
        } else {
            str = (IR.op.uz == 0) ? dasm_setd()
                                     : dasm_sets();
        }
        break;

    case p2_1001110:
        if (IR.op.uc == 0) {
            if (IR.op.uz == 0) {
                str = (IR.op.imm == 0 && IR.op.src == IR.op.dst) ? dasm_decod_d()
                                                                    : dasm_decod();
            } else {
                str = (IR.op.imm == 0 && IR.op.src == IR.op.dst) ? dasm_bmask_d()
                                                                    : dasm_bmask();
            }
        } else {
            if (IR.op.uz == 0) {
                str = dasm_crcbit();
            } else {
                str = dasm_crcnib();
            }
        }
        break;

    case p2_1001111:
        if (IR.op.uc == 0) {
            if (IR.op.uz == 0) {
                str = dasm_muxnits();
            } else {
                str = dasm_muxnibs();
            }
        } else {
            if (IR.op.uz == 0) {
                str = dasm_muxq();
            } else {
                str = dasm_movbyts();
            }
        }
        break;

    case p2_1010000:
        if (IR.op.uc == 0) {
            str = dasm_mul();
        } else {
            str = dasm_muls();
        }
        break;

    case p2_1010001:
        if (IR.op.uc == 0) {
            str = dasm_sca();
        } else {
            str = dasm_scas();
        }
        break;

    case p2_1010010:
        if (IR.op.uc == 0) {
            if (IR.op.uz == 0) {
                str = dasm_addpix();
            } else {
                str = dasm_mulpix();
            }
        } else {
            if (IR.op.uz == 0) {
                str = dasm_blnpix();
            } else {
                str = dasm_mixpix();
            }
        }
        break;

    case p2_1010011:
        if (IR.op.uc == 0) {
            if (IR.op.uz == 0) {
                str = dasm_addct1();
            } else {
                str = dasm_addct2();
            }
        } else {
            if (IR.op.uz == 0) {
                str = dasm_addct3();
            } else {
                str = dasm_wmlong();
            }
        }
        break;

    case p2_1010100:
        if (IR.op.uz == 0) {
            str = dasm_rqpin();
        } else {

        }
        break;

    case p2_rdlut:
        str = dasm_rdlut();
        break;

    case p2_rdbyte:
        str = dasm_rdbyte();
        break;

    case p2_rdword:
        str = dasm_rdword();
        break;

    case p2_rdlong:
        str = dasm_rdlong();
        break;

    case p2_calld:
        str = dasm_calld();
        break;

    case p2_callp:
        str = (IR.op.uc == 0) ? dasm_callpa() : dasm_callpb();
        break;

    case p2_1011011:
        if (IR.op.uc == 0) {
            if (IR.op.uz == 0) {
                str = dasm_djz();
            } else {
                str = dasm_djnz();
            }
        } else {
            if (IR.op.uz == 0) {
                str = dasm_djf();
            } else {
                str = dasm_djnf();
            }
        }
        break;

    case p2_1011100:
        if (IR.op.uc == 0) {
            if (IR.op.uz == 0) {
                str = dasm_ijz();
            } else {
                str = dasm_ijnz();
            }
        } else {
            if (IR.op.uz == 0) {
                str = dasm_tjz();
            } else {
                str = dasm_tjnz();
            }
        }
        break;

    case p2_1011101:
        if (IR.op.uc == 0) {
            if (IR.op.uz == 0) {
                str = dasm_tjf();
            } else {
                str = dasm_tjnf();
            }
        } else {
            if (IR.op.uz == 0) {
                str = dasm_tjs();
            } else {
                str = dasm_tjns();
            }
        }
        break;

    case p2_1011110:
        if (IR.op.uc == 0) {
            if (IR.op.uz == 0) {
                str = dasm_tjv();
            } else {
                switch (IR.op.dst) {
                case 0x00:
                    str = dasm_jint();
                    break;
                case 0x01:
                    str = dasm_jct1();
                    break;
                case 0x02:
                    str = dasm_jct2();
                    break;
                case 0x03:
                    str = dasm_jct3();
                    break;
                case 0x04:
                    str = dasm_jse1();
                    break;
                case 0x05:
                    str = dasm_jse2();
                    break;
                case 0x06:
                    str = dasm_jse3();
                    break;
                case 0x07:
                    str = dasm_jse4();
                    break;
                case 0x08:
                    str = dasm_jpat();
                    break;
                case 0x09:
                    str = dasm_jfbw();
                    break;
                case 0x0a:
                    str = dasm_jxmt();
                    break;
                case 0x0b:
                    str = dasm_jxfi();
                    break;
                case 0x0c:
                    str = dasm_jxro();
                    break;
                case 0x0d:
                    str = dasm_jxrl();
                    break;
                case 0x0e:
                    str = dasm_jatn();
                    break;
                case 0x0f:
                    str = dasm_jqmt();
                    break;
                case 0x10:
                    str = dasm_jnint();
                    break;
                case 0x11:
                    str = dasm_jnct1();
                    break;
                case 0x12:
                    str = dasm_jnct2();
                    break;
                case 0x13:
                    str = dasm_jnct3();
                    break;
                case 0x14:
                    str = dasm_jnse1();
                    break;
                case 0x15:
                    str = dasm_jnse2();
                    break;
                case 0x16:
                    str = dasm_jnse3();
                    break;
                case 0x17:
                    str = dasm_jnse4();
                    break;
                case 0x18:
                    str = dasm_jnpat();
                    break;
                case 0x19:
                    str = dasm_jnfbw();
                    break;
                case 0x1a:
                    str = dasm_jnxmt();
                    break;
                case 0x1b:
                    str = dasm_jnxfi();
                    break;
                case 0x1c:
                    str = dasm_jnxro();
                    break;
                case 0x1d:
                    str = dasm_jnxrl();
                    break;
                case 0x1e:
                    str = dasm_jnatn();
                    break;
                case 0x1f:
                    str = dasm_jnqmt();
                    break;
                default:
                    // TODO: invalid D value
                    break;
                }
            }
        } else {
            str = dasm_1011110_1();
        }
        break;

    case p2_1011111:
        if (IR.op.uc == 0) {
            str = dasm_1011111_0();
        } else {
            str = dasm_setpat();
        }
        break;

    case p2_1100000:
        if (IR.op.uc == 0) {
            str = (IR.op.uz == 1 && IR.op.dst == 1) ? dasm_akpin()
                                                       : dasm_wrpin();
        } else {
            str = dasm_wxpin();
        }
        break;

    case p2_1100001:
        if (IR.op.uc == 0) {
            str = dasm_wypin();
        } else {
            str = dasm_wrlut();
        }
        break;

    case p2_1100010:
        if (IR.op.uc == 0) {
            str = dasm_wrbyte();
        } else {
            str = dasm_wrword();
        }
        break;

    case p2_1100011:
        if (IR.op.uc == 0) {
            str = dasm_wrlong();
        } else {
            str = dasm_rdfast();
        }
        break;

    case p2_1100100:
        if (IR.op.uc == 0) {
            str = dasm_wrfast();
        } else {
            str = dasm_fblock();
        }
        break;

    case p2_1100101:
        if (IR.op.uc == 0) {
            if (IR.op.uz == 1 && IR.op.imm == 1 && IR.op.src == 0 && IR.op.dst == 0) {
                str = dasm_xstop();
            } else {
                str = dasm_xinit();
            }
        } else {
            str = dasm_xzero();
        }
        break;

    case p2_1100110:
        if (IR.op.uc == 0) {
            str = dasm_xcont();
        } else {
            str = dasm_rep();
        }
        break;

    case p2_coginit:
        str = dasm_coginit();
        break;

    case p2_1101000:
        if (IR.op.uc == 0) {
            str = dasm_qmul();
        } else {
            str = dasm_qdiv();
        }
        break;

    case p2_1101001:
        if (IR.op.uc == 0) {
            str = dasm_qfrac();
        } else {
            str = dasm_qsqrt();
        }
        break;

    case p2_1101010:
        if (IR.op.uc == 0) {
            str = dasm_qrotate();
        } else {
            str = dasm_qvector();
        }
        break;

    case p2_1101011:
        switch (IR.op.src) {
        case 0x00:
            str = dasm_hubset();
            break;
        case 0x01:
            str = dasm_cogid();
            break;
        case 0x03:
            str = dasm_cogstop();
            break;
        case 0x04:
            str = dasm_locknew();
            break;
        case 0x05:
            str = dasm_lockret();
            break;
        case 0x06:
            str = dasm_locktry();
            break;
        case 0x07:
            str = dasm_lockrel();
            break;
        case 0x0e:
            str = dasm_qlog();
            break;
        case 0x0f:
            str = dasm_qexp();
            break;
        case 0x10:
            str = dasm_rfbyte();
            break;
        case 0x11:
            str = dasm_rfword();
            break;
        case 0x12:
            str = dasm_rflong();
            break;
        case 0x13:
            str = dasm_rfvar();
            break;
        case 0x14:
            str = dasm_rfvars();
            break;
        case 0x15:
            str = dasm_wfbyte();
            break;
        case 0x16:
            str = dasm_wfword();
            break;
        case 0x17:
            str = dasm_wflong();
            break;
        case 0x18:
            str = dasm_getqx();
            break;
        case 0x19:
            str = dasm_getqy();
            break;
        case 0x1a:
            str = dasm_getct();
            break;
        case 0x1b:
            str = (IR.op.dst == 0) ? dasm_getrnd_cz()
                                      : dasm_getrnd();
            break;
        case 0x1c:
            str = dasm_setdacs();
            break;
        case 0x1d:
            str = dasm_setxfrq();
            break;
        case 0x1e:
            str = dasm_getxacc();
            break;
        case 0x1f:
            str = dasm_waitx();
            break;
        case 0x20:
            str = dasm_setse1();
            break;
        case 0x21:
            str = dasm_setse2();
            break;
        case 0x22:
            str = dasm_setse3();
            break;
        case 0x23:
            str = dasm_setse4();
            break;
        case 0x24:
            switch (IR.op.dst) {
            case 0x00:
                str = dasm_pollint();
                break;
            case 0x01:
                str = dasm_pollct1();
                break;
            case 0x02:
                str = dasm_pollct2();
                break;
            case 0x03:
                str = dasm_pollct3();
                break;
            case 0x04:
                str = dasm_pollse1();
                break;
            case 0x05:
                str = dasm_pollse2();
                break;
            case 0x06:
                str = dasm_pollse3();
                break;
            case 0x07:
                str = dasm_pollse4();
                break;
            case 0x08:
                str = dasm_pollpat();
                break;
            case 0x09:
                str = dasm_pollfbw();
                break;
            case 0x0a:
                str = dasm_pollxmt();
                break;
            case 0x0b:
                str = dasm_pollxfi();
                break;
            case 0x0c:
                str = dasm_pollxro();
                break;
            case 0x0d:
                str = dasm_pollxrl();
                break;
            case 0x0e:
                str = dasm_pollatn();
                break;
            case 0x0f:
                str = dasm_pollqmt();
                break;
            case 0x10:
                str = dasm_waitint();
                break;
            case 0x11:
                str = dasm_waitct1();
                break;
            case 0x12:
                str = dasm_waitct2();
                break;
            case 0x13:
                str = dasm_waitct3();
                break;
            case 0x14:
                str = dasm_waitse1();
                break;
            case 0x15:
                str = dasm_waitse2();
                break;
            case 0x16:
                str = dasm_waitse3();
                break;
            case 0x17:
                str = dasm_waitse4();
                break;
            case 0x18:
                str = dasm_waitpat();
                break;
            case 0x19:
                str = dasm_waitfbw();
                break;
            case 0x1a:
                str = dasm_waitxmt();
                break;
            case 0x1b:
                str = dasm_waitxfi();
                break;
            case 0x1c:
                str = dasm_waitxro();
                break;
            case 0x1d:
                str = dasm_waitxrl();
                break;
            case 0x1e:
                str = dasm_waitatn();
                break;
            case 0x20:
                str = dasm_allowi();
                break;
            case 0x21:
                str = dasm_stalli();
                break;
            case 0x22:
                str = dasm_trgint1();
                break;
            case 0x23:
                str = dasm_trgint2();
                break;
            case 0x24:
                str = dasm_trgint3();
                break;
            case 0x25:
                str = dasm_nixint1();
                break;
            case 0x26:
                str = dasm_nixint2();
                break;
            case 0x27:
                str = dasm_nixint3();
                break;
            }
            break;
        case 0x25:
            str = dasm_setint1();
            break;
        case 0x26:
            str = dasm_setint2();
            break;
        case 0x27:
            str = dasm_setint3();
            break;
        case 0x28:
            str = dasm_setq();
            break;
        case 0x29:
            str = dasm_setq2();
            break;
        case 0x2a:
            str = dasm_push();
            break;
        case 0x2b:
            str = dasm_pop();
            break;
        case 0x2c:
            str = dasm_jmp();
            break;
        case 0x2d:
            str = (IR.op.imm == 0) ? dasm_call()
                                      : dasm_ret();
            break;
        case 0x2e:
            str = (IR.op.imm == 0) ? dasm_calla()
                                      : dasm_reta();
            break;
        case 0x2f:
            str = (IR.op.imm == 0) ? dasm_callb()
                                      : dasm_retb();
            break;
        case 0x30:
            str = dasm_jmprel();
            break;
        case 0x31:
            str = dasm_skip();
            break;
        case 0x32:
            str = dasm_skipf();
            break;
        case 0x33:
            str = dasm_execf();
            break;
        case 0x34:
            str = dasm_getptr();
            break;
        case 0x35:
            str = (IR.op.uc == 0 && IR.op.uz == 0) ? dasm_cogbrk()
                                                      : dasm_getbrk();
            break;
        case 0x36:
            str = dasm_brk();
            break;
        case 0x37:
            str = dasm_setluts();
            break;
        case 0x38:
            str = dasm_setcy();
            break;
        case 0x39:
            str = dasm_setci();
            break;
        case 0x3a:
            str = dasm_setcq();
            break;
        case 0x3b:
            str = dasm_setcfrq();
            break;
        case 0x3c:
            str = dasm_setcmod();
            break;
        case 0x3d:
            str = dasm_setpiv();
            break;
        case 0x3e:
            str = dasm_setpix();
            break;
        case 0x3f:
            str = dasm_cogatn();
            break;
        case 0x40:
            str = (IR.op.uc == IR.op.uz) ? dasm_testp_w()
                                            : dasm_dirl();
            break;
        case 0x41:
            str = (IR.op.uc == IR.op.uz) ? dasm_testpn_w()
                                            : dasm_dirh();
            break;
        case 0x42:
            str = (IR.op.uc == IR.op.uz) ? dasm_testp_and()
                                            : dasm_dirc();
            break;
        case 0x43:
            str = (IR.op.uc == IR.op.uz) ? dasm_testpn_and()
                                            : dasm_dirnc();
            break;
        case 0x44:
            str = (IR.op.uc == IR.op.uz) ? dasm_testp_or()
                                            : dasm_dirz();
            break;
        case 0x45:
            str = (IR.op.uc == IR.op.uz) ? dasm_testpn_or()
                                            : dasm_dirnz();
            break;
        case 0x46:
            str = (IR.op.uc == IR.op.uz) ? dasm_testp_xor()
                                            : dasm_dirrnd();
            break;
        case 0x47:
            str = (IR.op.uc == IR.op.uz) ? dasm_testpn_xor()
                                            : dasm_dirnot();
            break;
        case 0x48:
            str = dasm_outl();
            break;
        case 0x49:
            str = dasm_outh();
            break;
        case 0x4a:
            str = dasm_outc();
            break;
        case 0x4b:
            str = dasm_outnc();
            break;
        case 0x4c:
            str = dasm_outz();
            break;
        case 0x4d:
            str = dasm_outnz();
            break;
        case 0x4e:
            str = dasm_outrnd();
            break;
        case 0x4f:
            str = dasm_outnot();
            break;
        case 0x50:
            str = dasm_fltl();
            break;
        case 0x51:
            str = dasm_flth();
            break;
        case 0x52:
            str = dasm_fltc();
            break;
        case 0x53:
            str = dasm_fltnc();
            break;
        case 0x54:
            str = dasm_fltz();
            break;
        case 0x55:
            str = dasm_fltnz();
            break;
        case 0x56:
            str = dasm_fltrnd();
            break;
        case 0x57:
            str = dasm_fltnot();
            break;
        case 0x58:
            str = dasm_drvl();
            break;
        case 0x59:
            str = dasm_drvh();
            break;
        case 0x5a:
            str = dasm_drvc();
            break;
        case 0x5b:
            str = dasm_drvnc();
            break;
        case 0x5c:
            str = dasm_drvz();
            break;
        case 0x5d:
            str = dasm_drvnz();
            break;
        case 0x5e:
            str = dasm_drvrnd();
            break;
        case 0x5f:
            str = dasm_drvnot();
            break;
        case 0x60:
            str = dasm_splitb();
            break;
        case 0x61:
            str = dasm_mergeb();
            break;
        case 0x62:
            str = dasm_splitw();
            break;
        case 0x63:
            str = dasm_mergew();
            break;
        case 0x64:
            str = dasm_seussf();
            break;
        case 0x65:
            str = dasm_seussr();
            break;
        case 0x66:
            str = dasm_rgbsqz();
            break;
        case 0x67:
            str = dasm_rgbexp();
            break;
        case 0x68:
            str = dasm_xoro32();
            break;
        case 0x69:
            str = dasm_rev();
            break;
        case 0x6a:
            str = dasm_rczr();
            break;
        case 0x6b:
            str = dasm_rczl();
            break;
        case 0x6c:
            str = dasm_wrc();
            break;
        case 0x6d:
            str = dasm_wrnc();
            break;
        case 0x6e:
            str = dasm_wrz();
            break;
        case 0x6f:
            str = dasm_wrnz();
            break;
        case 0x7f:
            str = dasm_modcz();
            break;
        }
        break;

    case p2_jmp_abs:
        str = dasm_jmp_abs();
        break;

    case p2_call_abs:
        str = dasm_call_abs();
        break;

    case p2_calla_abs:
        str = dasm_calla_abs();
        break;

    case p2_callb_abs:
        str = dasm_callb_abs();
        break;

    case p2_calld_pa_abs:
    case p2_calld_pb_abs:
    case p2_calld_ptra_abs:
    case p2_calld_ptrb_abs:
        str = dasm_calld_abs();
        break;

    case p2_loc_pa:
        str = dasm_loc_pa();
        break;

    case p2_loc_pb:
        str = dasm_loc_pb();
        break;

    case p2_loc_ptra:
        str = dasm_loc_ptra();
        break;

    case p2_loc_ptrb:
        str = dasm_loc_ptrb();
        break;

    case p2_augs_00:
    case p2_augs_01:
    case p2_augs_10:
    case p2_augs_11:
        str = dasm_augs();
        break;

    case p2_augd_00:
    case p2_augd_01:
    case p2_augd_10:
    case p2_augd_11:
        str = dasm_augd();
        break;
    }

    return str;
}

/**
 * @brief format string for: EEEE xxxxxxx CZI DDDDDDDDD SSSSSSSSS
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 * @return formatted string
 */
QString P2Dasm::format_d_imm_s_cz(p2_token_e inst, p2_token_e with)
{
    const int pad_inst = -16;
    const int pad_dst = -3;
    const int pad_src = (IR.op.uc || IR.op.uz) ? -8 : -3;

    QString str = QString("%1$%2, %3$%4")
                  .arg(p2Token->str(inst), pad_inst)
                  .arg(IR.op.dst, pad_dst, 16, QChar('0'))
                  .arg(IR.op.imm ? "#" : "")
                  .arg(IR.op.src, pad_src, 16, QChar('0'));

    if (IR.op.uc || IR.op.uz) {
        if (IR.op.uc && IR.op.uz) {
            str += QString("%1%2")
                   .arg(p2Token->str(with))
                   .arg(p2Token->str(t_CZ));
        } else if (IR.op.uz) {
            str += QString("%1%2")
                   .arg(p2Token->str(with))
                   .arg(p2Token->str(t_Z));
        } else {
            str += QString("%1%2")
                   .arg(p2Token->str(with))
                   .arg(p2Token->str(t_C));
        }
    }
    return str;
}

/**
 * @brief format string for: EEEE xxxxxxx C0I DDDDDDDDD SSSSSSSSS
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 * @return formatted string
 */
QString P2Dasm::format_d_imm_s_c(p2_token_e inst, p2_token_e with)
{
    const int pad_inst = -16;
    const int pad_dst = -3;
    const int pad_src = IR.op.uc ? -8 : -3;
    QString str = QString("%1$%2, %3$%4")
                  .arg(p2Token->str(inst), pad_inst)
                  .arg(IR.op.dst, pad_dst, 16, QChar('0'))
                  .arg(IR.op.imm ? "#" : "")
                  .arg(IR.op.src, pad_src, 16, QChar('0'));

    if (IR.op.uc) {
        str += QString(" %1%2")
               .arg(p2Token->str(with))
               .arg(p2Token->str(t_C));
    }
    return str;
}

/**
 * @brief format string for: EEEE xxxxxxx 0ZI DDDDDDDDD SSSSSSSSS
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 * @return formatted string
 */
QString P2Dasm::format_d_imm_s_z(p2_token_e inst, p2_token_e with)
{
    const int pad_inst = -16;
    const int pad_dst = -3;
    const int pad_src = IR.op.uz ? -8 : -3;
    QString str = QString("%1$%2, %3$%4")
                  .arg(p2Token->str(inst), pad_inst)
                  .arg(IR.op.dst, pad_dst, 16, QChar('0'))
                  .arg(IR.op.imm ? "#" : "")                // I
                  .arg(IR.op.src, pad_src, 16, QChar('0'));

    if (IR.op.uz) {
        str += QString(" %1%2")
               .arg(p2Token->str(with))
               .arg(p2Token->str(t_Z));
    }
    return str;
}

/**
 * @brief format string for: EEEE xxxxxxx 0LI DDDDDDDDD SSSSSSSSS
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 * @return formatted string
 */
QString P2Dasm::format_wz_d_imm_s(p2_token_e inst)
{
    const int pad_inst = -16;
    const int pad_dst = -3;
    const int pad_src = -3;
    QString str = QString("%1%2$%3, %4$%5")
                  .arg(p2Token->str(inst), pad_inst)
                  .arg(IR.op.uz ? "#" : "")                   // L
                  .arg(IR.op.dst, pad_dst, 16, QChar('0'))
                  .arg(IR.op.imm ? "#" : "")                  // I
                  .arg(IR.op.src, pad_src, 16, QChar('0'));
    return str;
}

/**
 * @brief format string for: EEEE xxxxxxN NNI DDDDDDDDD SSSSSSSSS
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 * @return formatted string
 */
QString P2Dasm::format_d_imm_s_nnn(p2_token_e inst)
{
    const int pad_inst = -16;
    const int pad_dst = -3;
    const int pad_src = -3;
    QString str = QString("%1$%2, %3$%4, #%5")
                  .arg(p2Token->str(inst), pad_inst)
                  .arg(IR.op.dst, pad_dst, 16, QChar('0'))
                  .arg(IR.op.imm ? "#" : "")
                  .arg(IR.op.src, pad_src, 16, QChar('0'))
                  .arg((IR.op.inst & 1) * 4 + IR.op.uc * 2 + IR.op.uz);
    return str;
}

/**
 * @brief format string for: EEEE xxxxxxx 0NI DDDDDDDDD SSSSSSSSS
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 * @return formatted string
 */
QString P2Dasm::format_d_imm_s_n(p2_token_e inst)
{
    const int pad_inst = -16;
    const int pad_dst = -3;
    const int pad_src = -3;
    QString str = QString("%1$%2, %3$%4, #%5")
                  .arg(p2Token->str(inst), pad_inst)
                  .arg(IR.op.dst, pad_dst, 16, QChar('0'))
                  .arg(IR.op.imm ? "#" : "")
                  .arg(IR.op.src, pad_src, 16, QChar('0'))
                  .arg(IR.op.uz);
    return str;
}

/**
 * @brief format string for: EEEE xxxxxxx xxI DDDDDDDDD SSSSSSSSS
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 * @return formatted string
 */
QString P2Dasm::format_d_imm_s(p2_token_e inst)
{
    const int pad_inst = -16;
    const int pad_dst = -3;
    const int pad_src = -3;
    QString str = QString("%1$%2, %3$%4")
                  .arg(p2Token->str(inst), pad_inst)
                  .arg(IR.op.dst, pad_dst, 16, QChar('0'))
                  .arg(IR.op.imm ? "#" : "")
                  .arg(IR.op.src, pad_src, 16, QChar('0'));
    return str;
}

/**
 * @brief format string for: EEEE xxxxxxx CZ0 DDDDDDDDD xxxxxxxxx
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 * @return formatted string
 */
QString P2Dasm::format_d_cz(p2_token_e inst, p2_token_e with)
{
    const int pad_inst = -16;
    const int pad_dst = (IR.op.uc || IR.op.uz) ? -8 : -3;
    QString str = QString("%1$%2, %3$%4")
                  .arg(p2Token->str(inst), pad_inst)
                  .arg(IR.op.dst, pad_dst, 16, QChar('0'));

    if (IR.op.uc || IR.op.uz) {
        if (IR.op.uc && IR.op.uz) {
            str += QString("%1%2")
                   .arg(p2Token->str(with))
                   .arg(p2Token->str(t_CZ));
        } else if (IR.op.uz) {
            str += QString("%1%2")
                   .arg(p2Token->str(with))
                   .arg(p2Token->str(t_Z));
        } else {
            str += QString("%1%2")
                   .arg(p2Token->str(with))
                   .arg(p2Token->str(t_C));
        }
    }
    return str;
}

/**
 * @brief format string for: EEEE xxxxxxx CZx xxxxxxxxx xxxxxxxxx
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 * @return formatted string
 */
QString P2Dasm::format_cz(p2_token_e inst, p2_token_e with)
{
    const int pad_inst = -16;
    QString str = QString("%1 ")
                  .arg(p2Token->str(inst), pad_inst);

    if (IR.op.uc || IR.op.uz) {
        if (IR.op.uc && IR.op.uz) {
            str += QString("%1%2")
                   .arg(p2Token->str(with))
                   .arg(p2Token->str(t_CZ));
        } else if (IR.op.uz) {
            str += QString("%1%2")
                   .arg(p2Token->str(with))
                   .arg(p2Token->str(t_Z));
        } else {
            str += QString("%1%2")
                   .arg(p2Token->str(with))
                   .arg(p2Token->str(t_C));
        }
    }
    return str;
}


/**
 * @brief format string for: EEEE xxxxxxx CZx 0cccczzzz xxxxxxxxx
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 * @return formatted string
 */
QString P2Dasm::format_cz_cz(p2_token_e inst, p2_token_e with)
{
    const int pad_inst = -16;
    const int pad_cmod = -8;
    const int pad_zmod = -8;

    QString str = QString("%1 %2,%3")
                  .arg(p2Token->str(inst), pad_inst)
                  .arg(conditional((IR.op.dst >> 4) & 15), pad_cmod)
                  .arg(conditional((IR.op.dst >> 0) & 15), pad_zmod);

    if (IR.op.uc || IR.op.uz) {
        if (IR.op.uc && IR.op.uz) {
            str += QString("%1%2")
                   .arg(p2Token->str(with))
                   .arg(p2Token->str(t_CZ));
        } else if (IR.op.uz) {
            str += QString("%1%2")
                   .arg(p2Token->str(with))
                   .arg(p2Token->str(t_Z));
        } else {
            str += QString("%1%2")
                   .arg(p2Token->str(with))
                   .arg(p2Token->str(t_C));
        }
    }
    return str;
}

/**
 * @brief format string for: EEEE xxxxxxx xxx DDDDDDDDD xxxxxxxxx
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 * @return formatted string
 */
QString P2Dasm::format_d(p2_token_e inst)
{
    const int pad_inst = -16;
    const int pad_dst = -3;
    QString str = QString("%1%2$%3")
                  .arg(p2Token->str(inst), pad_inst)
                  .arg(IR.op.dst, pad_dst, 16, QChar('0'));
    return str;
}

/**
 * @brief format string for: EEEE xxxxxxx xLx DDDDDDDDD xxxxxxxxx
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 * @return formatted string
 */
QString P2Dasm::format_wz_d(p2_token_e inst)
{
    const int pad_inst = -16;
    const int pad_dst = -3;
    QString str = QString("%1%2$%3")
                  .arg(p2Token->str(inst), pad_inst)
                  .arg(IR.op.uz ? "#" : "")
                  .arg(IR.op.dst, pad_dst, 16, QChar('0'));
    return str;
}

/**
 * @brief format string for: EEEE xxxxxxx xxL DDDDDDDDD xxxxxxxxx
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 * @return formatted string
 */
QString P2Dasm::format_imm_d(p2_token_e inst)
{
    const int pad_inst = -16;
    const int pad_dst = -3;
    QString str = QString("%1%2$%3")
                  .arg(p2Token->str(inst), pad_inst)
                  .arg(IR.op.imm ? "#" : "")
                  .arg(IR.op.dst, pad_dst, 16, QChar('0'));
    return str;
}

/**
 * @brief format string for: EEEE xxxxxxx CZL DDDDDDDDD xxxxxxxxx
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 * @return formatted string
 */
QString P2Dasm::format_imm_d_cz(p2_token_e inst, p2_token_e with)
{
    const int pad_inst = -16;
    const int pad_dst = (IR.op.uc || IR.op.uz) ? -8 : -3;
    QString str = QString("%1%2$%3")
                  .arg(p2Token->str(inst), pad_inst)
                  .arg(IR.op.imm ? "#" : "")
                  .arg(IR.op.dst, pad_dst, 16, QChar('0'));

    if (IR.op.uc || IR.op.uz) {
        if (IR.op.uc && IR.op.uz) {
            str += QString("%1%2")
                   .arg(p2Token->str(with))
                   .arg(p2Token->str(t_CZ));
        } else if (IR.op.uz) {
            str += QString("%1%2")
                   .arg(p2Token->str(with))
                   .arg(p2Token->str(t_Z));
        } else {
            str += QString("%1%2")
                   .arg(p2Token->str(with))
                   .arg(p2Token->str(t_C));
        }
    }
    return str;
}

/**
 * @brief format string for: EEEE xxxxxxx CxL DDDDDDDDD xxxxxxxxx
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 * @return formatted string
 */
QString P2Dasm::format_imm_d_c(p2_token_e inst)
{
    const int pad_inst = -16;
    const int pad_dst = IR.op.uc ? -8 : -3;
    QString str = QString("%1%2$%3")
                  .arg(p2Token->str(inst), pad_inst)
                  .arg(IR.op.imm ? "#" : "")
                  .arg(IR.op.dst, pad_dst, 16, QChar('0'));

    if (IR.op.uc) {
        str += p2Token->str(t_W);
        str += p2Token->str(t_C);
    }
    return str;
}

/**
 * @brief format string for: EEEE xxxxxxx xxL xxxxxxxxx SSSSSSSSS
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 * @return formatted string
 */
QString P2Dasm::format_imm_s(p2_token_e inst)
{
    const int pad_inst = -16;
    const int pad_src = -3;
    QString str = QString("%1%2$%3")
            .arg(inst, pad_inst)
            .arg(IR.op.imm ? "#" : "")
            .arg(IR.op.src, pad_src, 16, QChar('0'));
    return str;
}

/**
 * @brief format string for: EEEE xxxxxxx RAA AAAAAAAAA AAAAAAAAA
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 * @return formatted string
 */
QString P2Dasm::format_pc_abs(p2_token_e inst, p2_token_e dst)
{
    const int pad_inst = -16;
    const int pad_src = -14;
    const quint32 addr = IR.word & ((1u << 20) - 1);
    QString str = QString("%1%2%3$%4")
                  .arg(inst, pad_inst)
                  .arg(p2Token->str(dst))
                  .arg(IR.op.uc ? "PC+" : "")
                  .arg(addr, pad_src, 16, QChar('0'));
    return str;
}

/**
 * @brief format string for: EEEE xxxxxNN NNN NNNNNNNNN NNNNNNNNN
 *
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 * @return formatted string
 */
QString P2Dasm::format_imm23(p2_token_e inst)
{
    const int pad_inst = -16;
    const int pad_src = -5;
    const quint32 nnnn = IR.word & ((1u << 23) - 1);
    QString str = QString("%1#$%2")
                  .arg(inst, pad_inst)
                  .arg(nnnn, pad_src, 16, QChar('0'));
    return str;
}

/**
 * @brief No operation.
 *
 * 0000 0000000 000 000000000 000000000
 *
 * NOP
 *
 */
QString P2Dasm::dasm_nop()
{
    return p2Token->str(t_NOP);
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
QString P2Dasm::dasm_ror()
{
    if (0 == IR.word)
        return dasm_nop();
    return format_d_imm_s_cz(t_ROR);
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
QString P2Dasm::dasm_rol()
{
    return format_d_imm_s_cz(t_ROL);
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
QString P2Dasm::dasm_shr()
{
    return format_d_imm_s_cz(t_SHR);
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
QString P2Dasm::dasm_shl()
{
    return format_d_imm_s_cz(t_SHL);
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
QString P2Dasm::dasm_rcr()
{
    return format_d_imm_s_cz(t_RCR);
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
QString P2Dasm::dasm_rcl()
{
    return format_d_imm_s_cz(t_RCL);
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
QString P2Dasm::dasm_sar()
{
    return format_d_imm_s_cz(t_SAR);
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
QString P2Dasm::dasm_sal()
{
    return format_d_imm_s_cz(t_SAL);
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
QString P2Dasm::dasm_add()
{
    return format_d_imm_s_cz(t_ADD);
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
QString P2Dasm::dasm_addx()
{
    return format_d_imm_s_cz(t_ADDX);
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
QString P2Dasm::dasm_adds()
{
    return format_d_imm_s_cz(t_ADDS);
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
QString P2Dasm::dasm_addsx()
{
    return format_d_imm_s_cz(t_ADDSX);
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
QString P2Dasm::dasm_sub()
{
    return format_d_imm_s_cz(t_SUB);
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
QString P2Dasm::dasm_subx()
{
    return format_d_imm_s_cz(t_SUBX);
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
QString P2Dasm::dasm_subs()
{
    return format_d_imm_s_cz(t_SUBS);
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
QString P2Dasm::dasm_subsx()
{
    return format_d_imm_s_cz(t_SUBSX);
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
QString P2Dasm::dasm_cmp()
{
    return format_d_imm_s_cz(t_CMP);
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
QString P2Dasm::dasm_cmpx()
{
    return format_d_imm_s_cz(t_CMPX);
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
QString P2Dasm::dasm_cmps()
{
    return format_d_imm_s_cz(t_CMPS);
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
QString P2Dasm::dasm_cmpsx()
{
    return format_d_imm_s_cz(t_CMPSX);
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
QString P2Dasm::dasm_cmpr()
{
    return format_d_imm_s_cz(t_CMPR);
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
QString P2Dasm::dasm_cmpm()
{
    return format_d_imm_s_cz(t_CMPM);
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
QString P2Dasm::dasm_subr()
{
    return format_d_imm_s_cz(t_SUBR);
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
QString P2Dasm::dasm_cmpsub()
{
    return format_d_imm_s_cz(t_CMPSUB);
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
QString P2Dasm::dasm_fge()
{
    return format_d_imm_s_cz(t_FGE);
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
QString P2Dasm::dasm_fle()
{
    return format_d_imm_s_cz(t_FLE);
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
QString P2Dasm::dasm_fges()
{
    return format_d_imm_s_cz(t_FGES);
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
QString P2Dasm::dasm_fles()
{
    return format_d_imm_s_cz(t_FLES);
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
QString P2Dasm::dasm_sumc()
{
    return format_d_imm_s_cz(t_SUMC);
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
QString P2Dasm::dasm_sumnc()
{
    return format_d_imm_s_cz(t_SUMNC);
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
QString P2Dasm::dasm_sumz()
{
    return format_d_imm_s_cz(t_SUMZ);
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
QString P2Dasm::dasm_sumnz()
{
    return format_d_imm_s_cz(t_SUMNZ);
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
QString P2Dasm::dasm_testb_w()
{
    return format_d_imm_s_cz(t_TESTB);
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
QString P2Dasm::dasm_testbn_w()
{
    return format_d_imm_s_cz(t_TESTBN);
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
QString P2Dasm::dasm_testb_and()
{
    return format_d_imm_s_cz(t_TESTB, t_AND);
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
QString P2Dasm::dasm_testbn_and()
{
    return format_d_imm_s_cz(t_TESTNB, t_AND);
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
QString P2Dasm::dasm_testb_or()
{
    return format_d_imm_s_cz(t_TESTB, t_OR);
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
QString P2Dasm::dasm_testbn_or()
{
    return format_d_imm_s_cz(t_TESTNB, t_OR);
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
QString P2Dasm::dasm_testb_xor()
{
    return format_d_imm_s_cz(t_TESTB, t_XOR);
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
QString P2Dasm::dasm_testbn_xor()
{
    return format_d_imm_s_cz(t_TESTBN, t_XOR);
}

/**
 * @brief Bit S[4:0] of D = 0,    C,Z = D[S[4:0]].
 *
 * EEEE 0100000 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITL    D,{#}S         {WCZ}
 *
 */
QString P2Dasm::dasm_bitl()
{
    return format_d_imm_s_cz(t_BITL);
}

/**
 * @brief Bit S[4:0] of D = 1,    C,Z = D[S[4:0]].
 *
 * EEEE 0100001 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITH    D,{#}S         {WCZ}
 *
 */
QString P2Dasm::dasm_bith()
{
    return format_d_imm_s_cz(t_BITH);
}

/**
 * @brief Bit S[4:0] of D = C,    C,Z = D[S[4:0]].
 *
 * EEEE 0100010 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITC    D,{#}S         {WCZ}
 *
 */
QString P2Dasm::dasm_bitc()
{
    return format_d_imm_s_cz(t_BITC);
}

/**
 * @brief Bit S[4:0] of D = !C,   C,Z = D[S[4:0]].
 *
 * EEEE 0100011 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNC   D,{#}S         {WCZ}
 *
 */
QString P2Dasm::dasm_bitnc()
{
    return format_d_imm_s_cz(t_BITNC);
}

/**
 * @brief Bit S[4:0] of D = Z,    C,Z = D[S[4:0]].
 *
 * EEEE 0100100 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITZ    D,{#}S         {WCZ}
 *
 */
QString P2Dasm::dasm_bitz()
{
    return format_d_imm_s_cz(t_BITZ);
}

/**
 * @brief Bit S[4:0] of D = !Z,   C,Z = D[S[4:0]].
 *
 * EEEE 0100101 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNZ   D,{#}S         {WCZ}
 *
 */
QString P2Dasm::dasm_bitnz()
{
    return format_d_imm_s_cz(t_BITNZ);
}

/**
 * @brief Bit S[4:0] of D = RND,  C,Z = D[S[4:0]].
 *
 * EEEE 0100110 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITRND  D,{#}S         {WCZ}
 *
 */
QString P2Dasm::dasm_bitrnd()
{
    return format_d_imm_s_cz(t_BITRND);
}

/**
 * @brief Bit S[4:0] of D = !bit, C,Z = D[S[4:0]].
 *
 * EEEE 0100111 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNOT  D,{#}S         {WCZ}
 *
 */
QString P2Dasm::dasm_bitnot()
{
    return format_d_imm_s_cz(t_BITNOT);
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
QString P2Dasm::dasm_and()
{
    return format_d_imm_s_cz(t_AND);
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
QString P2Dasm::dasm_andn()
{
    return format_d_imm_s_cz(t_ANDN);
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
QString P2Dasm::dasm_or()
{
    return format_d_imm_s_cz(t_OR);
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
QString P2Dasm::dasm_xor()
{
    return format_d_imm_s_cz(t_XOR);
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
QString P2Dasm::dasm_muxc()
{
    return format_d_imm_s_cz(t_MUXC);
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
QString P2Dasm::dasm_muxnc()
{
    return format_d_imm_s_cz(t_MUXNC);
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
QString P2Dasm::dasm_muxz()
{
    return format_d_imm_s_cz(t_MUXZ);
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
QString P2Dasm::dasm_muxnz()
{
    return format_d_imm_s_cz(t_MUXNZ);
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
QString P2Dasm::dasm_mov()
{
    return format_d_imm_s_cz(t_MOV);
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
QString P2Dasm::dasm_not()
{
    return format_d_imm_s_cz(t_NOT);
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
QString P2Dasm::dasm_abs()
{
    return format_d_imm_s_cz(t_ABS);
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
QString P2Dasm::dasm_neg()
{
    return format_d_imm_s_cz(t_NEG);
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
QString P2Dasm::dasm_negc()
{
    return format_d_imm_s_cz(t_NEGC);
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
QString P2Dasm::dasm_negnc()
{
    return format_d_imm_s_cz(t_NEGNC);
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
QString P2Dasm::dasm_negz()
{
    return format_d_imm_s_cz(t_NEGZ);
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
QString P2Dasm::dasm_negnz()
{
    return format_d_imm_s_cz(t_NEGNZ);
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
QString P2Dasm::dasm_incmod()
{
    return format_d_imm_s_cz(t_INCMOD);
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
QString P2Dasm::dasm_decmod()
{
    return format_d_imm_s_cz(t_DECMOD);
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
QString P2Dasm::dasm_zerox()
{
    return format_d_imm_s_cz(t_ZEROX);
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
QString P2Dasm::dasm_signx()
{
    return format_d_imm_s_cz(t_SIGNX);
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
QString P2Dasm::dasm_encod()
{
    return format_d_imm_s_cz(t_ENCOD);
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
QString P2Dasm::dasm_ones()
{
    return format_d_imm_s_cz(t_ONES);
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
QString P2Dasm::dasm_test()
{
    return format_d_imm_s_cz(t_TEST);
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
QString P2Dasm::dasm_testn()
{
    return format_d_imm_s_cz(t_TESTN);
}

/**
 * @brief Set S[3:0] into nibble N in D, keeping rest of D same.
 *
 * EEEE 100000N NNI DDDDDDDDD SSSSSSSSS
 *
 * SETNIB  D,{#}S,#N
 *
 */
QString P2Dasm::dasm_setnib()
{
    return format_d_imm_s_nnn(t_SETNIB);
}

/**
 * @brief Set S[3:0] into nibble established by prior ALTSN instruction.
 *
 * EEEE 1000000 00I 000000000 SSSSSSSSS
 *
 * SETNIB  {#}S
 *
 */
QString P2Dasm::dasm_setnib_altsn()
{
    return format_imm_s(t_SETNIB);
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
QString P2Dasm::dasm_getnib()
{
    return format_d_imm_s_nnn(t_GETNIB);
}

/**
 * @brief Get nibble established by prior ALTGN instruction into D.
 *
 * EEEE 1000010 000 DDDDDDDDD 000000000
 *
 * GETNIB  D
 *
 */
QString P2Dasm::dasm_getnib_altgn()
{
    return format_imm_s(t_GETNIB);
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
QString P2Dasm::dasm_rolnib()
{
    return format_d_imm_s_nnn(t_ROLNIB);
}

/**
 * @brief Rotate-left nibble established by prior ALTGN instruction into D.
 *
 * EEEE 1000100 000 DDDDDDDDD 000000000
 *
 * ROLNIB  D
 *
 */
QString P2Dasm::dasm_rolnib_altgn()
{
    return format_d(t_ROLNIB);
}

/**
 * @brief Set S[7:0] into byte N in D, keeping rest of D same.
 *
 * EEEE 1000110 NNI DDDDDDDDD SSSSSSSSS
 *
 * SETBYTE D,{#}S,#N
 *
 */
QString P2Dasm::dasm_setbyte()
{
    return format_d_imm_s_nnn(t_SETBYTE);
}

/**
 * @brief Set S[7:0] into byte established by prior ALTSB instruction.
 *
 * EEEE 1000110 00I 000000000 SSSSSSSSS
 *
 * SETBYTE {#}S
 *
 */
QString P2Dasm::dasm_setbyte_altsb()
{
    return format_imm_s(t_SETBYTE);
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
QString P2Dasm::dasm_getbyte()
{
    return format_d_imm_s_nnn(t_GETBYTE);
}

/**
 * @brief Get byte established by prior ALTGB instruction into D.
 *
 * EEEE 1000111 000 DDDDDDDDD 000000000
 *
 * GETBYTE D
 *
 */
QString P2Dasm::dasm_getbyte_altgb()
{
    return format_d(t_GETBYTE);
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
QString P2Dasm::dasm_rolbyte()
{
    return format_d_imm_s_nnn(t_ROLBYTE);
}

/**
 * @brief Rotate-left byte established by prior ALTGB instruction into D.
 *
 * EEEE 1001000 000 DDDDDDDDD 000000000
 *
 * ROLBYTE D
 *
 */
QString P2Dasm::dasm_rolbyte_altgb()
{
    return format_d(t_ROLBYTE);
}

/**
 * @brief Set S[15:0] into word N in D, keeping rest of D same.
 *
 * EEEE 1001001 0NI DDDDDDDDD SSSSSSSSS
 *
 * SETWORD D,{#}S,#N
 *
 */
QString P2Dasm::dasm_setword()
{
    return format_d_imm_s_n(t_SETWORD);
}

/**
 * @brief Set S[15:0] into word established by prior ALTSW instruction.
 *
 * EEEE 1001001 00I 000000000 SSSSSSSSS
 *
 * SETWORD {#}S
 *
 */
QString P2Dasm::dasm_setword_altsw()
{
    return format_imm_s(t_SETWORD);
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
QString P2Dasm::dasm_getword()
{
    return format_d_imm_s_n(t_GETWORD);
}

/**
 * @brief Get word established by prior ALTGW instruction into D.
 *
 * EEEE 1001001 100 DDDDDDDDD 000000000
 *
 * GETWORD D
 *
 */
QString P2Dasm::dasm_getword_altgw()
{
    return format_imm_s(t_GETWORD);
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
QString P2Dasm::dasm_rolword()
{
    return format_d_imm_s_n(t_ROLWORD);
}

/**
 * @brief Rotate-left word established by prior ALTGW instruction into D.
 *
 * EEEE 1001010 000 DDDDDDDDD 000000000
 *
 * ROLWORD D
 *
 */
QString P2Dasm::dasm_rolword_altgw()
{
    return format_d(t_ROLWORD);
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
QString P2Dasm::dasm_altsn()
{
    return format_d_imm_s(t_ALTSN);
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
QString P2Dasm::dasm_altsn_d()
{
    return format_d(t_ALTSN);
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
QString P2Dasm::dasm_altgn()
{
    return format_d_imm_s(t_ALTGN);
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
QString P2Dasm::dasm_altgn_d()
{
    return format_d(t_ALTGN);
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
QString P2Dasm::dasm_altsb()
{
    return format_d_imm_s(t_ALTSB);
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
QString P2Dasm::dasm_altsb_d()
{
    return format_d(t_ALTSB);
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
QString P2Dasm::dasm_altgb()
{
    return format_d_imm_s(t_ALTGB);
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
QString P2Dasm::dasm_altgb_d()
{
    return format_d(t_ALTGB);
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
QString P2Dasm::dasm_altsw()
{
    return format_d_imm_s(t_ALTSW);
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
QString P2Dasm::dasm_altsw_d()
{
    return format_d(t_ALTSW);
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
QString P2Dasm::dasm_altgw()
{
    return format_d_imm_s(t_ALTGW);
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
QString P2Dasm::dasm_altgw_d()
{
    return format_d(t_ALTGW);
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
QString P2Dasm::dasm_altr()
{
    return format_d_imm_s(t_ALTR);
}

/**
 * @brief Alter result register address (normally D field) of next instruction to D[8:0].
 *
 * EEEE 1001100 001 DDDDDDDDD 000000000
 *
 * ALTR    D
 *
 */
QString P2Dasm::dasm_altr_d()
{
    return format_d(t_ALTD);
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
QString P2Dasm::dasm_altd()
{
    return format_d_imm_s(t_ALTD);
}

/**
 * @brief Alter D field of next instruction to D[8:0].
 *
 * EEEE 1001100 011 DDDDDDDDD 000000000
 *
 * ALTD    D
 *
 */
QString P2Dasm::dasm_altd_d()
{
    return format_d(t_ALTD);
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
QString P2Dasm::dasm_alts()
{
    return format_d_imm_s(t_ALTS);
}

/**
 * @brief Alter S field of next instruction to D[8:0].
 *
 * EEEE 1001100 101 DDDDDDDDD 000000000
 *
 * ALTS    D
 *
 */
QString P2Dasm::dasm_alts_d()
{
    return format_d(t_ALTS);
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
QString P2Dasm::dasm_altb()
{
    return format_d_imm_s(t_ALTB);
}

/**
 * @brief Alter D field of next instruction to D[13:5].
 *
 * EEEE 1001100 111 DDDDDDDDD 000000000
 *
 * ALTB    D
 *
 */
QString P2Dasm::dasm_altb_d()
{
    return format_d(t_ALTB);
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
QString P2Dasm::dasm_alti()
{
    return format_d_imm_s(t_ALTI);
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
QString P2Dasm::dasm_alti_d()
{
    return format_d(t_ALTI);
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
QString P2Dasm::dasm_setr()
{
    return format_d_imm_s(t_SETR);
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
QString P2Dasm::dasm_setd()
{
    return format_d_imm_s(t_SETD);
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
QString P2Dasm::dasm_sets()
{
    return format_d_imm_s(t_SETS);
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
QString P2Dasm::dasm_decod()
{
    return format_d_imm_s(t_DECOD);
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
QString P2Dasm::dasm_decod_d()
{
    return format_d(t_DECOD);
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
QString P2Dasm::dasm_bmask()
{
    return format_d_imm_s(t_BMASK);
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
QString P2Dasm::dasm_bmask_d()
{
    return format_d(t_BMASK);
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
QString P2Dasm::dasm_crcbit()
{
    return format_d_imm_s(t_CRCBIT);
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
QString P2Dasm::dasm_crcnib()
{
    return format_d_imm_s(t_CRCNIB);
}

/**
 * @brief For each non-zero bit pair in S, copy that bit pair into the corresponding D bits, else leave that D bit pair the same.
 *
 * EEEE 1001111 00I DDDDDDDDD SSSSSSSSS
 *
 * MUXNITS D,{#}S
 *
 */
QString P2Dasm::dasm_muxnits()
{
    return format_d_imm_s(t_MUXNITS);
}

/**
 * @brief For each non-zero nibble in S, copy that nibble into the corresponding D nibble, else leave that D nibble the same.
 *
 * EEEE 1001111 01I DDDDDDDDD SSSSSSSSS
 *
 * MUXNIBS D,{#}S
 *
 */
QString P2Dasm::dasm_muxnibs()
{
    return format_d_imm_s(t_MUXNIBS);
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
QString P2Dasm::dasm_muxq()
{
    return format_d_imm_s(t_MUXQ);
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
QString P2Dasm::dasm_movbyts()
{
    return format_d_imm_s(t_MOVBYTS);
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
QString P2Dasm::dasm_mul()
{
    return format_d_imm_s_z(t_MUL);
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
QString P2Dasm::dasm_muls()
{
    return format_d_imm_s_z(t_MULS);
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
QString P2Dasm::dasm_sca()
{
    return format_d_imm_s_z(t_SCA);
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
QString P2Dasm::dasm_scas()
{
    return format_d_imm_s_z(t_SCAS);
}

/**
 * @brief Add bytes of S into bytes of D, with $FF saturation.
 *
 * EEEE 1010010 00I DDDDDDDDD SSSSSSSSS
 *
 * ADDPIX  D,{#}S
 *
 */
QString P2Dasm::dasm_addpix()
{
    return format_d_imm_s(t_ADDPIX);
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
QString P2Dasm::dasm_mulpix()
{
    return format_d_imm_s(t_MULPIX);
}

/**
 * @brief Alpha-blend bytes of S into bytes of D, using SETPIV value.
 *
 * EEEE 1010010 10I DDDDDDDDD SSSSSSSSS
 *
 * BLNPIX  D,{#}S
 *
 */
QString P2Dasm::dasm_blnpix()
{
    return format_d_imm_s(t_BLNPIX);
}

/**
 * @brief Mix bytes of S into bytes of D, using SETPIX and SETPIV values.
 *
 * EEEE 1010010 11I DDDDDDDDD SSSSSSSSS
 *
 * MIXPIX  D,{#}S
 *
 */
QString P2Dasm::dasm_mixpix()
{
    return format_d_imm_s(t_MIXPIX);
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
QString P2Dasm::dasm_addct1()
{
    return format_d_imm_s(t_ADDCT1);
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
QString P2Dasm::dasm_addct2()
{
    return format_d_imm_s(t_ADDCT2);
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
QString P2Dasm::dasm_addct3()
{
    return format_d_imm_s(t_ADDCT3);
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
QString P2Dasm::dasm_wmlong()
{
    return format_d_imm_s(t_WMLONG);
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
QString P2Dasm::dasm_rqpin()
{
    return format_d_imm_s_c(t_RQPIN);
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
QString P2Dasm::dasm_rdpin()
{
    return format_d_imm_s_c(t_RDPIN);
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
QString P2Dasm::dasm_rdlut()
{
    return format_d_imm_s_cz(t_RDLUT);
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
QString P2Dasm::dasm_rdbyte()
{
    return format_d_imm_s_cz(t_RDBYTE);
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
QString P2Dasm::dasm_rdword()
{
    return format_d_imm_s_cz(t_RDWORD);
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
QString P2Dasm::dasm_rdlong()
{
    return format_d_imm_s_cz(t_RDLONG);
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
QString P2Dasm::dasm_popa()
{
    return format_d_imm_s_cz(t_POPA);
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
QString P2Dasm::dasm_popb()
{
    return format_d_imm_s_cz(t_POPB);
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
QString P2Dasm::dasm_calld()
{
    if (IR.op.dst == 0x1f0 && IR.op.src == 0x1f1 && IR.op.uc && IR.op.uz)
        return dasm_resi3();
    if (IR.op.dst == 0x1f2 && IR.op.src == 0x1f3 && IR.op.uc && IR.op.uz)
        return dasm_resi2();
    if (IR.op.dst == 0x1f4 && IR.op.src == 0x1f5 && IR.op.uc && IR.op.uz)
        return dasm_resi1();
    if (IR.op.dst == 0x1fe && IR.op.src == 0x1ff && IR.op.uc && IR.op.uz)
        return dasm_resi0();
    if (IR.op.dst == 0x1ff && IR.op.src == 0x1f1 && IR.op.uc && IR.op.uz)
        return dasm_reti3();
    if (IR.op.dst == 0x1ff && IR.op.src == 0x1f3 && IR.op.uc && IR.op.uz)
        return dasm_reti2();
    if (IR.op.dst == 0x1ff && IR.op.src == 0x1f5 && IR.op.uc && IR.op.uz)
        return dasm_reti1();
    if (IR.op.dst == 0x1ff && IR.op.src == 0x1ff && IR.op.uc && IR.op.uz)
        return dasm_reti0();
    return format_d_imm_s_cz(t_CALLD);
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
QString P2Dasm::dasm_resi3()
{
    return p2Token->str(t_RESI3);
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
QString P2Dasm::dasm_resi2()
{
    return p2Token->str(t_RESI2);
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
QString P2Dasm::dasm_resi1()
{
    return p2Token->str(t_RESI1);
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
QString P2Dasm::dasm_resi0()
{
    return p2Token->str(t_RESI0);
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
QString P2Dasm::dasm_reti3()
{
    return p2Token->str(t_RETI3);
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
QString P2Dasm::dasm_reti2()
{
    return p2Token->str(t_RETI2);
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
QString P2Dasm::dasm_reti1()
{
    return p2Token->str(t_RETI1);
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
QString P2Dasm::dasm_reti0()
{
    return p2Token->str(t_RETI0);
}

/**
 * @brief Call to S** by pushing {C, Z, 10'b0, PC[19:0]} onto stack, copy D to PA.
 *
 * EEEE 1011010 0LI DDDDDDDDD SSSSSSSSS
 *
 * CALLPA  {#}D,{#}S
 *
 */
QString P2Dasm::dasm_callpa()
{
    return format_wz_d_imm_s(t_CALLPA);
}

/**
 * @brief Call to S** by pushing {C, Z, 10'b0, PC[19:0]} onto stack, copy D to PB.
 *
 * EEEE 1011010 1LI DDDDDDDDD SSSSSSSSS
 *
 * CALLPB  {#}D,{#}S
 *
 */
QString P2Dasm::dasm_callpb()
{
    return format_wz_d_imm_s(t_CALLPB);
}

/**
 * @brief Decrement D and jump to S** if result is zero.
 *
 * EEEE 1011011 00I DDDDDDDDD SSSSSSSSS
 *
 * DJZ     D,{#}S
 *
 */
QString P2Dasm::dasm_djz()
{
    return format_d_imm_s(t_DJZ);
}

/**
 * @brief Decrement D and jump to S** if result is not zero.
 *
 * EEEE 1011011 01I DDDDDDDDD SSSSSSSSS
 *
 * DJNZ    D,{#}S
 *
 */
QString P2Dasm::dasm_djnz()
{
    return format_d_imm_s(t_DJNZ);
}

/**
 * @brief Decrement D and jump to S** if result is $FFFF_FFFF.
 *
 * EEEE 1011011 10I DDDDDDDDD SSSSSSSSS
 *
 * DJF     D,{#}S
 *
 */
QString P2Dasm::dasm_djf()
{
    return format_d_imm_s(t_DJF);
}

/**
 * @brief Decrement D and jump to S** if result is not $FFFF_FFFF.
 *
 * EEEE 1011011 11I DDDDDDDDD SSSSSSSSS
 *
 * DJNF    D,{#}S
 *
 */
QString P2Dasm::dasm_djnf()
{
    return format_d_imm_s(t_DJNF);
}

/**
 * @brief Increment D and jump to S** if result is zero.
 *
 * EEEE 1011100 00I DDDDDDDDD SSSSSSSSS
 *
 * IJZ     D,{#}S
 *
 */
QString P2Dasm::dasm_ijz()
{
    return format_d_imm_s(t_IJZ);
}

/**
 * @brief Increment D and jump to S** if result is not zero.
 *
 * EEEE 1011100 01I DDDDDDDDD SSSSSSSSS
 *
 * IJNZ    D,{#}S
 *
 */
QString P2Dasm::dasm_ijnz()
{
    return format_d_imm_s(t_IJNZ);
}

/**
 * @brief Test D and jump to S** if D is zero.
 *
 * EEEE 1011100 10I DDDDDDDDD SSSSSSSSS
 *
 * TJZ     D,{#}S
 *
 */
QString P2Dasm::dasm_tjz()
{
    return format_d_imm_s(t_TJZ);
}

/**
 * @brief Test D and jump to S** if D is not zero.
 *
 * EEEE 1011100 11I DDDDDDDDD SSSSSSSSS
 *
 * TJNZ    D,{#}S
 *
 */
QString P2Dasm::dasm_tjnz()
{
    return format_d_imm_s(t_TJNZ);
}

/**
 * @brief Test D and jump to S** if D is full (D = $FFFF_FFFF).
 *
 * EEEE 1011101 00I DDDDDDDDD SSSSSSSSS
 *
 * TJF     D,{#}S
 *
 */
QString P2Dasm::dasm_tjf()
{
    return format_d_imm_s(t_TJF);
}

/**
 * @brief Test D and jump to S** if D is not full (D != $FFFF_FFFF).
 *
 * EEEE 1011101 01I DDDDDDDDD SSSSSSSSS
 *
 * TJNF    D,{#}S
 *
 */
QString P2Dasm::dasm_tjnf()
{
    return format_d_imm_s(t_TJNF);
}

/**
 * @brief Test D and jump to S** if D is signed (D[31] = 1).
 *
 * EEEE 1011101 10I DDDDDDDDD SSSSSSSSS
 *
 * TJS     D,{#}S
 *
 */
QString P2Dasm::dasm_tjs()
{
    return format_d_imm_s(t_TJS);
}

/**
 * @brief Test D and jump to S** if D is not signed (D[31] = 0).
 *
 * EEEE 1011101 11I DDDDDDDDD SSSSSSSSS
 *
 * TJNS    D,{#}S
 *
 */
QString P2Dasm::dasm_tjns()
{
    return format_d_imm_s(t_TJNS);
}

/**
 * @brief Test D and jump to S** if D overflowed (D[31] != C, C = 'correct sign' from last addition/subtraction).
 *
 * EEEE 1011110 00I DDDDDDDDD SSSSSSSSS
 *
 * TJV     D,{#}S
 *
 */
QString P2Dasm::dasm_tjv()
{
    return format_d_imm_s(t_TJV);
}

/**
 * @brief Jump to S** if INT event flag is set.
 *
 * EEEE 1011110 01I 000000000 SSSSSSSSS
 *
 * JINT    {#}S
 *
 */
QString P2Dasm::dasm_jint()
{
    return format_imm_s(t_JINT);
}

/**
 * @brief Jump to S** if CT1 event flag is set.
 *
 * EEEE 1011110 01I 000000001 SSSSSSSSS
 *
 * JCT1    {#}S
 *
 */
QString P2Dasm::dasm_jct1()
{
    return format_imm_s(t_JCT1);
}

/**
 * @brief Jump to S** if CT2 event flag is set.
 *
 * EEEE 1011110 01I 000000010 SSSSSSSSS
 *
 * JCT2    {#}S
 *
 */
QString P2Dasm::dasm_jct2()
{
    return format_imm_s(t_JCT2);
}

/**
 * @brief Jump to S** if CT3 event flag is set.
 *
 * EEEE 1011110 01I 000000011 SSSSSSSSS
 *
 * JCT3    {#}S
 *
 */
QString P2Dasm::dasm_jct3()
{
    return format_imm_s(t_JCT3);
}

/**
 * @brief Jump to S** if SE1 event flag is set.
 *
 * EEEE 1011110 01I 000000100 SSSSSSSSS
 *
 * JSE1    {#}S
 *
 */
QString P2Dasm::dasm_jse1()
{
    return format_imm_s(t_JSE1);
}

/**
 * @brief Jump to S** if SE2 event flag is set.
 *
 * EEEE 1011110 01I 000000101 SSSSSSSSS
 *
 * JSE2    {#}S
 *
 */
QString P2Dasm::dasm_jse2()
{
    return format_imm_s(t_JSE2);
}

/**
 * @brief Jump to S** if SE3 event flag is set.
 *
 * EEEE 1011110 01I 000000110 SSSSSSSSS
 *
 * JSE3    {#}S
 *
 */
QString P2Dasm::dasm_jse3()
{
    return format_imm_s(t_JSE3);
}

/**
 * @brief Jump to S** if SE4 event flag is set.
 *
 * EEEE 1011110 01I 000000111 SSSSSSSSS
 *
 * JSE4    {#}S
 *
 */
QString P2Dasm::dasm_jse4()
{
    return format_imm_s(t_JSE4);
}

/**
 * @brief Jump to S** if PAT event flag is set.
 *
 * EEEE 1011110 01I 000001000 SSSSSSSSS
 *
 * JPAT    {#}S
 *
 */
QString P2Dasm::dasm_jpat()
{
    return format_imm_s(t_JPAT);
}

/**
 * @brief Jump to S** if FBW event flag is set.
 *
 * EEEE 1011110 01I 000001001 SSSSSSSSS
 *
 * JFBW    {#}S
 *
 */
QString P2Dasm::dasm_jfbw()
{
    return format_imm_s(t_JFBW);
}

/**
 * @brief Jump to S** if XMT event flag is set.
 *
 * EEEE 1011110 01I 000001010 SSSSSSSSS
 *
 * JXMT    {#}S
 *
 */
QString P2Dasm::dasm_jxmt()
{
    return format_imm_s(t_JXMT);
}

/**
 * @brief Jump to S** if XFI event flag is set.
 *
 * EEEE 1011110 01I 000001011 SSSSSSSSS
 *
 * JXFI    {#}S
 *
 */
QString P2Dasm::dasm_jxfi()
{
    return format_imm_s(t_JXFI);
}

/**
 * @brief Jump to S** if XRO event flag is set.
 *
 * EEEE 1011110 01I 000001100 SSSSSSSSS
 *
 * JXRO    {#}S
 *
 */
QString P2Dasm::dasm_jxro()
{
    return format_imm_s(t_JXRO);
}

/**
 * @brief Jump to S** if XRL event flag is set.
 *
 * EEEE 1011110 01I 000001101 SSSSSSSSS
 *
 * JXRL    {#}S
 *
 */
QString P2Dasm::dasm_jxrl()
{
    return format_imm_s(t_JXRL);
}

/**
 * @brief Jump to S** if ATN event flag is set.
 *
 * EEEE 1011110 01I 000001110 SSSSSSSSS
 *
 * JATN    {#}S
 *
 */
QString P2Dasm::dasm_jatn()
{
    return format_imm_s(t_JATN);
}

/**
 * @brief Jump to S** if QMT event flag is set.
 *
 * EEEE 1011110 01I 000001111 SSSSSSSSS
 *
 * JQMT    {#}S
 *
 */
QString P2Dasm::dasm_jqmt()
{
    return format_imm_s(t_JQMT);
}

/**
 * @brief Jump to S** if INT event flag is clear.
 *
 * EEEE 1011110 01I 000010000 SSSSSSSSS
 *
 * JNINT   {#}S
 *
 */
QString P2Dasm::dasm_jnint()
{
    return format_imm_s(t_JNINT);
}

/**
 * @brief Jump to S** if CT1 event flag is clear.
 *
 * EEEE 1011110 01I 000010001 SSSSSSSSS
 *
 * JNCT1   {#}S
 *
 */
QString P2Dasm::dasm_jnct1()
{
    return format_imm_s(t_JNCT1);
}

/**
 * @brief Jump to S** if CT2 event flag is clear.
 *
 * EEEE 1011110 01I 000010010 SSSSSSSSS
 *
 * JNCT2   {#}S
 *
 */
QString P2Dasm::dasm_jnct2()
{
    return format_imm_s(t_JNCT2);
}

/**
 * @brief Jump to S** if CT3 event flag is clear.
 *
 * EEEE 1011110 01I 000010011 SSSSSSSSS
 *
 * JNCT3   {#}S
 *
 */
QString P2Dasm::dasm_jnct3()
{
    return format_imm_s(t_JNCT3);
}

/**
 * @brief Jump to S** if SE1 event flag is clear.
 *
 * EEEE 1011110 01I 000010100 SSSSSSSSS
 *
 * JNSE1   {#}S
 *
 */
QString P2Dasm::dasm_jnse1()
{
    return format_imm_s(t_JNSE1);
}

/**
 * @brief Jump to S** if SE2 event flag is clear.
 *
 * EEEE 1011110 01I 000010101 SSSSSSSSS
 *
 * JNSE2   {#}S
 *
 */
QString P2Dasm::dasm_jnse2()
{
    return format_imm_s(t_JNSE2);
}

/**
 * @brief Jump to S** if SE3 event flag is clear.
 *
 * EEEE 1011110 01I 000010110 SSSSSSSSS
 *
 * JNSE3   {#}S
 *
 */
QString P2Dasm::dasm_jnse3()
{
    return format_imm_s(t_JNSE3);
}

/**
 * @brief Jump to S** if SE4 event flag is clear.
 *
 * EEEE 1011110 01I 000010111 SSSSSSSSS
 *
 * JNSE4   {#}S
 *
 */
QString P2Dasm::dasm_jnse4()
{
    return format_imm_s(t_JNSE4);
}

/**
 * @brief Jump to S** if PAT event flag is clear.
 *
 * EEEE 1011110 01I 000011000 SSSSSSSSS
 *
 * JNPAT   {#}S
 *
 */
QString P2Dasm::dasm_jnpat()
{
    return format_imm_s(t_JNPAT);
}

/**
 * @brief Jump to S** if FBW event flag is clear.
 *
 * EEEE 1011110 01I 000011001 SSSSSSSSS
 *
 * JNFBW   {#}S
 *
 */
QString P2Dasm::dasm_jnfbw()
{
    return format_imm_s(t_JNFBW);
}

/**
 * @brief Jump to S** if XMT event flag is clear.
 *
 * EEEE 1011110 01I 000011010 SSSSSSSSS
 *
 * JNXMT   {#}S
 *
 */
QString P2Dasm::dasm_jnxmt()
{
    return format_imm_s(t_JNXMT);
}

/**
 * @brief Jump to S** if XFI event flag is clear.
 *
 * EEEE 1011110 01I 000011011 SSSSSSSSS
 *
 * JNXFI   {#}S
 *
 */
QString P2Dasm::dasm_jnxfi()
{
    return format_imm_s(t_JNXFI);
}

/**
 * @brief Jump to S** if XRO event flag is clear.
 *
 * EEEE 1011110 01I 000011100 SSSSSSSSS
 *
 * JNXRO   {#}S
 *
 */
QString P2Dasm::dasm_jnxro()
{
    return format_imm_s(t_JNXRO);
}

/**
 * @brief Jump to S** if XRL event flag is clear.
 *
 * EEEE 1011110 01I 000011101 SSSSSSSSS
 *
 * JNXRL   {#}S
 *
 */
QString P2Dasm::dasm_jnxrl()
{
    return format_imm_s(t_JNXRL);
}

/**
 * @brief Jump to S** if ATN event flag is clear.
 *
 * EEEE 1011110 01I 000011110 SSSSSSSSS
 *
 * JNATN   {#}S
 *
 */
QString P2Dasm::dasm_jnatn()
{
    return format_imm_s(t_JNATN);
}

/**
 * @brief Jump to S** if QMT event flag is clear.
 *
 * EEEE 1011110 01I 000011111 SSSSSSSSS
 *
 * JNQMT   {#}S
 *
 */
QString P2Dasm::dasm_jnqmt()
{
    return format_imm_s(t_JNQMT);
}

/**
 * @brief <empty>.
 *
 * EEEE 1011110 1LI DDDDDDDDD SSSSSSSSS
 *
 * <empty> {#}D,{#}S
 *
 */
QString P2Dasm::dasm_1011110_1()
{
    return format_wz_d_imm_s(t_empty);
}

/**
 * @brief <empty>.
 *
 * EEEE 1011111 0LI DDDDDDDDD SSSSSSSSS
 *
 * <empty> {#}D,{#}S
 *
 */
QString P2Dasm::dasm_1011111_0()
{
    return format_wz_d_imm_s(t_empty);
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
QString P2Dasm::dasm_setpat()
{
    return format_wz_d_imm_s(t_SETPAT);
}

/**
 * @brief Write D to mode register of smart pin S[5:0], acknowledge smart pin.
 *
 * EEEE 1100000 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRPIN   {#}D,{#}S
 *
 */
QString P2Dasm::dasm_wrpin()
{
    return format_wz_d_imm_s(t_WRPIN);
}

/**
 * @brief Acknowledge smart pin S[5:0].
 *
 * EEEE 1100000 01I 000000001 SSSSSSSSS
 *
 * AKPIN   {#}S
 *
 */
QString P2Dasm::dasm_akpin()
{
    return format_imm_s(t_AKPIN);
}

/**
 * @brief Write D to parameter "X" of smart pin S[5:0], acknowledge smart pin.
 *
 * EEEE 1100000 1LI DDDDDDDDD SSSSSSSSS
 *
 * WXPIN   {#}D,{#}S
 *
 */
QString P2Dasm::dasm_wxpin()
{
    return format_wz_d_imm_s(t_WXPIN);
}

/**
 * @brief Write D to parameter "Y" of smart pin S[5:0], acknowledge smart pin.
 *
 * EEEE 1100001 0LI DDDDDDDDD SSSSSSSSS
 *
 * WYPIN   {#}D,{#}S
 *
 */
QString P2Dasm::dasm_wypin()
{
    return format_wz_d_imm_s(t_WYPIN);
}

/**
 * @brief Write D to LUT address S[8:0].
 *
 * EEEE 1100001 1LI DDDDDDDDD SSSSSSSSS
 *
 * WRLUT   {#}D,{#}S
 *
 */
QString P2Dasm::dasm_wrlut()
{
    return format_wz_d_imm_s(t_WRLUT);
}

/**
 * @brief Write byte in D[7:0] to hub address {#}S/PTRx.
 *
 * EEEE 1100010 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRBYTE  {#}D,{#}S/P
 *
 */
QString P2Dasm::dasm_wrbyte()
{
    return format_wz_d_imm_s(t_WRBYTE);
}

/**
 * @brief Write word in D[15:0] to hub address {#}S/PTRx.
 *
 * EEEE 1100010 1LI DDDDDDDDD SSSSSSSSS
 *
 * WRWORD  {#}D,{#}S/P
 *
 */
QString P2Dasm::dasm_wrword()
{
    return format_wz_d_imm_s(t_WRWORD);
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
QString P2Dasm::dasm_wrlong()
{
    return format_wz_d_imm_s(t_WRLONG);
}

/**
 * @brief Write long in D[31:0] to hub address PTRA++.
 *
 * EEEE 1100011 0L1 DDDDDDDDD 101100001
 *
 * PUSHA   {#}D
 *
 */
QString P2Dasm::dasm_pusha()
{
    return format_wz_d(t_PUSHA);
}

/**
 * @brief Write long in D[31:0] to hub address PTRB++.
 *
 * EEEE 1100011 0L1 DDDDDDDDD 111100001
 *
 * PUSHB   {#}D
 *
 */
QString P2Dasm::dasm_pushb()
{
    return format_wz_d(t_PUSHB);
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
QString P2Dasm::dasm_rdfast()
{
    return format_wz_d_imm_s(t_RDFAST);
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
QString P2Dasm::dasm_wrfast()
{
    return format_wz_d_imm_s(t_WRFAST);
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
QString P2Dasm::dasm_fblock()
{
    return format_wz_d_imm_s(t_FBLOCK);
}

/**
 * @brief Issue streamer command immediately, zeroing phase.
 *
 * EEEE 1100101 0LI DDDDDDDDD SSSSSSSSS
 *
 * XINIT   {#}D,{#}S
 *
 */
QString P2Dasm::dasm_xinit()
{
    return format_wz_d_imm_s(t_XINIT);
}

/**
 * @brief Stop streamer immediately.
 *
 * EEEE 1100101 011 000000000 000000000
 *
 * XSTOP
 *
 */
QString P2Dasm::dasm_xstop()
{
    return p2Token->str(t_XSTOP);
}

/**
 * @brief Buffer new streamer command to be issued on final NCO rollover of current command, zeroing phase.
 *
 * EEEE 1100101 1LI DDDDDDDDD SSSSSSSSS
 *
 * XZERO   {#}D,{#}S
 *
 */
QString P2Dasm::dasm_xzero()
{
    return format_wz_d_imm_s(t_XZERO);
}

/**
 * @brief Buffer new streamer command to be issued on final NCO rollover of current command, continuing phase.
 *
 * EEEE 1100110 0LI DDDDDDDDD SSSSSSSSS
 *
 * XCONT   {#}D,{#}S
 *
 */
QString P2Dasm::dasm_xcont()
{
    return format_wz_d_imm_s(t_XCONT);
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
QString P2Dasm::dasm_rep()
{
    return format_wz_d_imm_s(t_REP);
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
QString P2Dasm::dasm_coginit()
{
    return format_wz_d_imm_s(t_COGINIT);
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
QString P2Dasm::dasm_qmul()
{
    return format_wz_d_imm_s(t_QMUL);
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
QString P2Dasm::dasm_qdiv()
{
    return format_wz_d_imm_s(t_QDIV);
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
QString P2Dasm::dasm_qfrac()
{
    return format_wz_d_imm_s(t_QFRAC);
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
QString P2Dasm::dasm_qsqrt()
{
    return format_wz_d_imm_s(t_QSQRT);
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
QString P2Dasm::dasm_qrotate()
{
    return format_wz_d_imm_s(t_QROTATE);
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
QString P2Dasm::dasm_qvector()
{
    return format_wz_d_imm_s(t_QVECTOR);
}

/**
 * @brief Set hub configuration to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000000000
 *
 * HUBSET  {#}D
 *
 */
QString P2Dasm::dasm_hubset()
{
    return format_imm_d(t_HUBSET);
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
QString P2Dasm::dasm_cogid()
{
    return format_imm_d_c(t_COGID);
}

/**
 * @brief Stop cog D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000000011
 *
 * COGSTOP {#}D
 *
 */
QString P2Dasm::dasm_cogstop()
{
    return format_imm_d(t_COGSTOP);
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
QString P2Dasm::dasm_locknew()
{
    return format_imm_d_c(t_LOCKNEW);
}

/**
 * @brief Return LOCK D[3:0] for reallocation.
 *
 * EEEE 1101011 00L DDDDDDDDD 000000101
 *
 * LOCKRET {#}D
 *
 */
QString P2Dasm::dasm_lockret()
{
    return format_imm_d(t_LOCKRET);
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
QString P2Dasm::dasm_locktry()
{
    return format_imm_d_c(t_LOCKTRY);
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
QString P2Dasm::dasm_lockrel()
{
    return format_imm_d_c(t_LOCKREL);
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
QString P2Dasm::dasm_qlog()
{
    return format_imm_d(t_QLOG);
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
QString P2Dasm::dasm_qexp()
{
    return format_imm_d(t_QEXP);
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
QString P2Dasm::dasm_rfbyte()
{
    return format_d_cz(t_RFBYTE);
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
QString P2Dasm::dasm_rfword()
{
    return format_d_cz(t_RFWORD);
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
QString P2Dasm::dasm_rflong()
{
    return format_d_cz(t_RFLONG);
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
QString P2Dasm::dasm_rfvar()
{
    return format_d_cz(t_RFVAR);
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
QString P2Dasm::dasm_rfvars()
{

    return format_d_cz(t_RFVARS);
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
QString P2Dasm::dasm_wfbyte()
{
    return format_imm_d(t_WFBYTE);
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
QString P2Dasm::dasm_wfword()
{
    return format_imm_d(t_WFWORD);
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
QString P2Dasm::dasm_wflong()
{
    return format_imm_d(t_WFLONG);
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
QString P2Dasm::dasm_getqx()
{
    return format_d_cz(t_GETQX);
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
QString P2Dasm::dasm_getqy()
{
    return format_d_cz(t_GETQY);
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
QString P2Dasm::dasm_getct()
{
    return format_d(t_GETCT);
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
QString P2Dasm::dasm_getrnd()
{
    return format_d_cz(t_GETRND);
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
QString P2Dasm::dasm_getrnd_cz()
{
    return format_cz(t_GETRND);
}

/**
 * @brief DAC3 = D[31:24], DAC2 = D[23:16], DAC1 = D[15:8], DAC0 = D[7:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000011100
 *
 * SETDACS {#}D
 *
 */
QString P2Dasm::dasm_setdacs()
{
    return format_imm_d(t_SETDACS);
}

/**
 * @brief Set streamer NCO frequency to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000011101
 *
 * SETXFRQ {#}D
 *
 */
QString P2Dasm::dasm_setxfrq()
{
    return format_imm_d(t_SETXFRQ);
}

/**
 * @brief Get the streamer's Goertzel X accumulator into D and the Y accumulator into the next instruction's S, clear accumulators.
 *
 * EEEE 1101011 000 DDDDDDDDD 000011110
 *
 * GETXACC D
 *
 */
QString P2Dasm::dasm_getxacc()
{
    return format_d(t_GETXACC);
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
QString P2Dasm::dasm_waitx()
{
    return format_imm_d_cz(t_WAITX);
}

/**
 * @brief Set SE1 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100000
 *
 * SETSE1  {#}D
 *
 */
QString P2Dasm::dasm_setse1()
{
    return format_imm_d(t_SETSE1);
}

/**
 * @brief Set SE2 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100001
 *
 * SETSE2  {#}D
 *
 */
QString P2Dasm::dasm_setse2()
{
    return format_imm_d(t_SETSE2);
}

/**
 * @brief Set SE3 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100010
 *
 * SETSE3  {#}D
 *
 */
QString P2Dasm::dasm_setse3()
{
    return format_imm_d(t_SETSE3);
}

/**
 * @brief Set SE4 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100011
 *
 * SETSE4  {#}D
 *
 */
QString P2Dasm::dasm_setse4()
{
    return format_imm_d(t_SETSE4);
}

/**
 * @brief Get INT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000000 000100100
 *
 * POLLINT          {WC/WZ/WCZ}
 *
 */
QString P2Dasm::dasm_pollint()
{
    return format_cz(t_POLLINT);
}

/**
 * @brief Get CT1 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000001 000100100
 *
 * POLLCT1          {WC/WZ/WCZ}
 *
 */
QString P2Dasm::dasm_pollct1()
{
    return format_cz(t_POLLCT1);
}

/**
 * @brief Get CT2 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000010 000100100
 *
 * POLLCT2          {WC/WZ/WCZ}
 *
 */
QString P2Dasm::dasm_pollct2()
{
    return format_cz(t_POLLCT2);
}

/**
 * @brief Get CT3 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000011 000100100
 *
 * POLLCT3          {WC/WZ/WCZ}
 *
 */
QString P2Dasm::dasm_pollct3()
{
    return format_cz(t_POLLCT3);
}

/**
 * @brief Get SE1 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000100 000100100
 *
 * POLLSE1          {WC/WZ/WCZ}
 *
 */
QString P2Dasm::dasm_pollse1()
{
    return format_cz(t_POLLSE1);
}

/**
 * @brief Get SE2 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000101 000100100
 *
 * POLLSE2          {WC/WZ/WCZ}
 *
 */
QString P2Dasm::dasm_pollse2()
{
    return format_cz(t_POLLSE2);
}

/**
 * @brief Get SE3 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000110 000100100
 *
 * POLLSE3          {WC/WZ/WCZ}
 *
 */
QString P2Dasm::dasm_pollse3()
{
    return format_cz(t_POLLSE3);
}

/**
 * @brief Get SE4 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000111 000100100
 *
 * POLLSE4          {WC/WZ/WCZ}
 *
 */
QString P2Dasm::dasm_pollse4()
{
    return format_cz(t_POLLSE4);
}

/**
 * @brief Get PAT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001000 000100100
 *
 * POLLPAT          {WC/WZ/WCZ}
 *
 */
QString P2Dasm::dasm_pollpat()
{
    return format_cz(t_POLLPAT);
}

/**
 * @brief Get FBW event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001001 000100100
 *
 * POLLFBW          {WC/WZ/WCZ}
 *
 */
QString P2Dasm::dasm_pollfbw()
{
    return format_cz(t_POLLFBW);
}

/**
 * @brief Get XMT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001010 000100100
 *
 * POLLXMT          {WC/WZ/WCZ}
 *
 */
QString P2Dasm::dasm_pollxmt()
{
    return format_cz(t_POLLXMT);
}

/**
 * @brief Get XFI event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001011 000100100
 *
 * POLLXFI          {WC/WZ/WCZ}
 *
 */
QString P2Dasm::dasm_pollxfi()
{
    return format_cz(t_POLLXFI);
}

/**
 * @brief Get XRO event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001100 000100100
 *
 * POLLXRO          {WC/WZ/WCZ}
 *
 */
QString P2Dasm::dasm_pollxro()
{
    return format_cz(t_POLLXRO);
}

/**
 * @brief Get XRL event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001101 000100100
 *
 * POLLXRL          {WC/WZ/WCZ}
 *
 */
QString P2Dasm::dasm_pollxrl()
{
    return format_cz(t_POLLXRL);
}

/**
 * @brief Get ATN event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001110 000100100
 *
 * POLLATN          {WC/WZ/WCZ}
 *
 */
QString P2Dasm::dasm_pollatn()
{
    return format_cz(t_POLLATN);
}

/**
 * @brief Get QMT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001111 000100100
 *
 * POLLQMT          {WC/WZ/WCZ}
 *
 */
QString P2Dasm::dasm_pollqmt()
{
    return format_cz(t_POLLQMT);
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
QString P2Dasm::dasm_waitint()
{
    return format_cz(t_WAITINT);
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
QString P2Dasm::dasm_waitct1()
{
    return format_cz(t_WAITCT1);
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
QString P2Dasm::dasm_waitct2()
{
    return format_cz(t_WAITCT2);
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
QString P2Dasm::dasm_waitct3()
{
    return format_cz(t_WAITCT3);
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
QString P2Dasm::dasm_waitse1()
{
    return format_cz(t_WAITSE1);
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
QString P2Dasm::dasm_waitse2()
{
    return format_cz(t_WAITSE2);
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
QString P2Dasm::dasm_waitse3()
{
    return format_cz(t_WAITSE3);
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
QString P2Dasm::dasm_waitse4()
{
    return format_cz(t_WAITSE4);
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
QString P2Dasm::dasm_waitpat()
{
    return format_cz(t_WAITPAT);
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
QString P2Dasm::dasm_waitfbw()
{
    return format_cz(t_WAITFBW);
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
QString P2Dasm::dasm_waitxmt()
{
    return format_cz(t_WAITXMT);
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
QString P2Dasm::dasm_waitxfi()
{
    return format_cz(t_WAITXFI);
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
QString P2Dasm::dasm_waitxro()
{
    return format_cz(t_WAITXRO);
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
QString P2Dasm::dasm_waitxrl()
{
    return format_cz(t_WAITXRL);
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
QString P2Dasm::dasm_waitatn()
{
    return format_cz(t_WAITATN);
}

/**
 * @brief Allow interrupts (default).
 *
 * EEEE 1101011 000 000100000 000100100
 *
 * ALLOWI
 *
 */
QString P2Dasm::dasm_allowi()
{
    return p2Token->str(t_ALLOWI);
}

/**
 * @brief Stall Interrupts.
 *
 * EEEE 1101011 000 000100001 000100100
 *
 * STALLI
 *
 */
QString P2Dasm::dasm_stalli()
{
    return p2Token->str(t_STALLI);
}

/**
 * @brief Trigger INT1, regardless of STALLI mode.
 *
 * EEEE 1101011 000 000100010 000100100
 *
 * TRGINT1
 *
 */
QString P2Dasm::dasm_trgint1()
{
    return p2Token->str(t_TRGINT1);
}

/**
 * @brief Trigger INT2, regardless of STALLI mode.
 *
 * EEEE 1101011 000 000100011 000100100
 *
 * TRGINT2
 *
 */
QString P2Dasm::dasm_trgint2()
{
    return p2Token->str(t_TRGINT2);
}

/**
 * @brief Trigger INT3, regardless of STALLI mode.
 *
 * EEEE 1101011 000 000100100 000100100
 *
 * TRGINT3
 *
 */
QString P2Dasm::dasm_trgint3()
{
    return p2Token->str(t_TRGINT3);
}

/**
 * @brief Cancel INT1.
 *
 * EEEE 1101011 000 000100101 000100100
 *
 * NIXINT1
 *
 */
QString P2Dasm::dasm_nixint1()
{
    return p2Token->str(t_NIXINT1);
}

/**
 * @brief Cancel INT2.
 *
 * EEEE 1101011 000 000100110 000100100
 *
 * NIXINT2
 *
 */
QString P2Dasm::dasm_nixint2()
{
    return p2Token->str(t_NIXINT2);
}

/**
 * @brief Cancel INT3.
 *
 * EEEE 1101011 000 000100111 000100100
 *
 * NIXINT3
 *
 */
QString P2Dasm::dasm_nixint3()
{
    return p2Token->str(t_NIXINT3);
}

/**
 * @brief Set INT1 source to D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100101
 *
 * SETINT1 {#}D
 *
 */
QString P2Dasm::dasm_setint1()
{
    return format_imm_d(t_SETINT1);
}

/**
 * @brief Set INT2 source to D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100110
 *
 * SETINT2 {#}D
 *
 */
QString P2Dasm::dasm_setint2()
{
    return format_imm_d(t_SETINT2);
}

/**
 * @brief Set INT3 source to D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100111
 *
 * SETINT3 {#}D
 *
 */
QString P2Dasm::dasm_setint3()
{
    return format_imm_d(t_SETINT3);
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
QString P2Dasm::dasm_setq()
{
    return format_imm_d(t_SETQ);
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
QString P2Dasm::dasm_setq2()
{
    return format_imm_d(t_SETQ2);
}

/**
 * @brief Push D onto stack.
 *
 * EEEE 1101011 00L DDDDDDDDD 000101010
 *
 * PUSH    {#}D
 *
 */
QString P2Dasm::dasm_push()
{
    return format_imm_d(t_PUSH);
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
QString P2Dasm::dasm_pop()
{
    return format_d_cz(t_POP);
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
QString P2Dasm::dasm_jmp()
{
    return format_d_cz(t_JMP);
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
QString P2Dasm::dasm_call()
{
    return format_d_cz(t_CALL);
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
QString P2Dasm::dasm_ret()
{
    return format_cz(t_RET);
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
QString P2Dasm::dasm_calla()
{
    return format_d_cz(t_CALLA);
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
QString P2Dasm::dasm_reta()
{
    return format_cz(t_RETA);
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
QString P2Dasm::dasm_callb()
{
    return format_d_cz(t_CALLB);
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
QString P2Dasm::dasm_retb()
{
    return format_cz(t_RETB);
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
QString P2Dasm::dasm_jmprel()
{
    return format_imm_d(t_JMPREL);
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
QString P2Dasm::dasm_skip()
{
    return format_imm_d(t_SKIP);
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
QString P2Dasm::dasm_skipf()
{
    return format_imm_d(t_SKIPF);
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
QString P2Dasm::dasm_execf()
{
    return format_imm_d(t_EXECF);
}

/**
 * @brief Get current FIFO hub pointer into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 000110100
 *
 * GETPTR  D
 *
 */
QString P2Dasm::dasm_getptr()
{
    return format_d(t_GETPTR);
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
QString P2Dasm::dasm_getbrk()
{
    return format_d_cz(t_GETBRK);
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
QString P2Dasm::dasm_cogbrk()
{
    return format_imm_d(t_COGBRK);
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
QString P2Dasm::dasm_brk()
{
    return format_imm_d(t_BRK);
}

/**
 * @brief If D[0] = 1 then enable LUT sharing, where LUT writes within the adjacent odd/even companion cog are copied to this LUT.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110111
 *
 * SETLUTS {#}D
 *
 */
QString P2Dasm::dasm_setluts()
{
    return format_imm_d(t_SETLUTS);
}

/**
 * @brief Set the colorspace converter "CY" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111000
 *
 * SETCY   {#}D
 *
 */
QString P2Dasm::dasm_setcy()
{
    return format_imm_d(t_SETCY);
}

/**
 * @brief Set the colorspace converter "CI" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111001
 *
 * SETCI   {#}D
 *
 */
QString P2Dasm::dasm_setci()
{
    return format_imm_d(t_SETCI);
}

/**
 * @brief Set the colorspace converter "CQ" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111010
 *
 * SETCQ   {#}D
 *
 */
QString P2Dasm::dasm_setcq()
{
    return format_imm_d(t_SETCQ);
}

/**
 * @brief Set the colorspace converter "CFRQ" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111011
 *
 * SETCFRQ {#}D
 *
 */
QString P2Dasm::dasm_setcfrq()
{
    return format_imm_d(t_SETCFRQ);
}

/**
 * @brief Set the colorspace converter "CMOD" parameter to D[6:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111100
 *
 * SETCMOD {#}D
 *
 */
QString P2Dasm::dasm_setcmod()
{
    return format_imm_d(t_SETCMOD);
}

/**
 * @brief Set BLNPIX/MIXPIX blend factor to D[7:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111101
 *
 * SETPIV  {#}D
 *
 */
QString P2Dasm::dasm_setpiv()
{
    return format_imm_d(t_SETPIV);
}

/**
 * @brief Set MIXPIX mode to D[5:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111110
 *
 * SETPIX  {#}D
 *
 */
QString P2Dasm::dasm_setpix()
{
    return format_imm_d(t_SETPIX);
}

/**
 * @brief Strobe "attention" of all cogs whose corresponging bits are high in D[15:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111111
 *
 * COGATN  {#}D
 *
 */
QString P2Dasm::dasm_cogatn()
{
    return format_imm_d(t_COGATN);
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
QString P2Dasm::dasm_testp_w()
{
    return format_imm_d_cz(t_TESTP);
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
QString P2Dasm::dasm_testpn_w()
{
    return format_imm_d_cz(t_TESTPN);
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
QString P2Dasm::dasm_testp_and()
{
    return format_imm_d_cz(t_TESTP, t_AND);
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
QString P2Dasm::dasm_testpn_and()
{
    return format_imm_d_cz(t_TESTPN, t_AND);
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
QString P2Dasm::dasm_testp_or()
{
    return format_imm_d_cz(t_TESTP, t_OR);
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
QString P2Dasm::dasm_testpn_or()
{
    return format_imm_d_cz(t_TESTPN, t_OR);
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
QString P2Dasm::dasm_testp_xor()
{
    return format_imm_d_cz(t_TESTP, t_XOR);
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
QString P2Dasm::dasm_testpn_xor()
{
    return format_imm_d_cz(t_TESTPN, t_XOR);
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
QString P2Dasm::dasm_dirl()
{
    return format_imm_d_cz(t_DIRL);
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
QString P2Dasm::dasm_dirh()
{
    return format_imm_d_cz(t_DIRH);
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
QString P2Dasm::dasm_dirc()
{
    return format_imm_d_cz(t_DIRC);
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
QString P2Dasm::dasm_dirnc()
{
    return format_imm_d_cz(t_DIRNC);
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
QString P2Dasm::dasm_dirz()
{
    return format_imm_d_cz(t_DIRZ);
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
QString P2Dasm::dasm_dirnz()
{
    return format_imm_d_cz(t_DIRNZ);
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
QString P2Dasm::dasm_dirrnd()
{
    return format_imm_d_cz(t_DIRRND);
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
QString P2Dasm::dasm_dirnot()
{
    return format_imm_d_cz(t_DIRNOT);
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
QString P2Dasm::dasm_outl()
{
    return format_imm_d_cz(t_OUTL);
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
QString P2Dasm::dasm_outh()
{
    return format_imm_d_cz(t_OUTH);
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
QString P2Dasm::dasm_outc()
{
    return format_imm_d_cz(t_OUTC);
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
QString P2Dasm::dasm_outnc()
{
    return format_imm_d_cz(t_OUTNC);
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
QString P2Dasm::dasm_outz()
{
    return format_imm_d_cz(t_OUTZ);
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
QString P2Dasm::dasm_outnz()
{
    return format_imm_d_cz(t_OUTNZ);
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
QString P2Dasm::dasm_outrnd()
{
    return format_imm_d_cz(t_OUTRND);
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
QString P2Dasm::dasm_outnot()
{
    return format_imm_d_cz(t_OUTNOT);
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
QString P2Dasm::dasm_fltl()
{
    return format_imm_d_cz(t_FLTL);
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
QString P2Dasm::dasm_flth()
{
    return format_imm_d_cz(t_FLTH);
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
QString P2Dasm::dasm_fltc()
{
    return format_imm_d_cz(t_FLTC);
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
QString P2Dasm::dasm_fltnc()
{
    return format_imm_d_cz(t_FLTNC);
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
QString P2Dasm::dasm_fltz()
{
    return format_imm_d_cz(t_FLTZ);
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
QString P2Dasm::dasm_fltnz()
{
    return format_imm_d_cz(t_FLTNZ);
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
QString P2Dasm::dasm_fltrnd()
{
    return format_imm_d_cz(t_FLTRND);
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
QString P2Dasm::dasm_fltnot()
{
    return format_imm_d_cz(t_FLTNOT);
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
QString P2Dasm::dasm_drvl()
{
    return format_imm_d_cz(t_DRVL);
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
QString P2Dasm::dasm_drvh()
{
    return format_imm_d_cz(t_DRVH);
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
QString P2Dasm::dasm_drvc()
{
    return format_imm_d_cz(t_DRVC);
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
QString P2Dasm::dasm_drvnc()
{
    return format_imm_d_cz(t_DRVNC);
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
QString P2Dasm::dasm_drvz()
{
    return format_imm_d_cz(t_DRVZ);
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
QString P2Dasm::dasm_drvnz()
{
    return format_imm_d_cz(t_DRVNZ);
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
QString P2Dasm::dasm_drvrnd()
{
    return format_imm_d_cz(t_DRVRND);
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
QString P2Dasm::dasm_drvnot()
{
    return format_imm_d_cz(t_DRVNOT);
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
QString P2Dasm::dasm_splitb()
{
    return format_d(t_SPLITB);
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
QString P2Dasm::dasm_mergeb()
{
    return format_d(t_MERGEB);
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
QString P2Dasm::dasm_splitw()
{
    return format_d(t_SPLITW);
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
QString P2Dasm::dasm_mergew()
{
    return format_d(t_MERGEW);
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
QString P2Dasm::dasm_seussf()
{
    return format_d(t_SEUSSF);
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
QString P2Dasm::dasm_seussr()
{
    return format_d(t_SEUSSR);
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
QString P2Dasm::dasm_rgbsqz()
{
    return format_d(t_RGBSQZ);
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
QString P2Dasm::dasm_rgbexp()
{
    return format_d(t_RGBEXP);
}

/**
 * @brief Iterate D with xoroshiro32+ PRNG algorithm and put PRNG result into next instruction's S.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101000
 *
 * XORO32  D
 *
 */
QString P2Dasm::dasm_xoro32()
{
    return format_d(t_XORO32);
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
QString P2Dasm::dasm_rev()
{
    return format_d(t_REV);
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
QString P2Dasm::dasm_rczr()
{
    return format_d_cz(t_RCZR);
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
QString P2Dasm::dasm_rczl()
{
    return format_d_cz(t_RCZL);
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
QString P2Dasm::dasm_wrc()
{
    return format_d(t_WRC);
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
QString P2Dasm::dasm_wrnc()
{
    return format_d(t_WRNC);
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
QString P2Dasm::dasm_wrz()
{
    return format_d(t_WRZ);
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
QString P2Dasm::dasm_wrnz()
{
    return format_d(t_WRNZ);
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
QString P2Dasm::dasm_modcz()
{
    return format_cz_cz(t_MODCZ);
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
QString P2Dasm::dasm_jmp_abs()
{
    return format_pc_abs(t_JMP);
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
QString P2Dasm::dasm_call_abs()
{
    return format_pc_abs(t_CALL);
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
QString P2Dasm::dasm_calla_abs()
{
    return format_pc_abs(t_CALLA);
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
QString P2Dasm::dasm_callb_abs()
{
    return format_pc_abs(t_CALLB);
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
QString P2Dasm::dasm_calld_abs()
{
    return format_pc_abs(t_CALLD);
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
QString P2Dasm::dasm_loc_pa()
{
    return format_pc_abs(t_LOC, t_PA);
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
QString P2Dasm::dasm_loc_pb()
{
    return format_pc_abs(t_LOC, t_PB);
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
QString P2Dasm::dasm_loc_ptra()
{
    return format_pc_abs(t_LOC, t_PTRA);
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
QString P2Dasm::dasm_loc_ptrb()
{
    return format_pc_abs(t_LOC, t_PTRB);
}

/**
 * @brief Queue #N[31:9] to be used as upper 23 bits for next #S occurrence, so that the next 9-bit #S will be augmented to 32 bits.
 *
 * EEEE 11110NN NNN NNNNNNNNN NNNNNNNNN
 *
 * AUGS    #N
 *
 */
QString P2Dasm::dasm_augs()
{
    return format_imm23(t_AUGS);
}

/**
 * @brief Queue #N[31:9] to be used as upper 23 bits for next #D occurrence, so that the next 9-bit #D will be augmented to 32 bits.
 *
 * EEEE 11111NN NNN NNNNNNNNN NNNNNNNNN
 *
 * AUGD    #N
 *
 */
QString P2Dasm::dasm_augd()
{
    return format_imm23(t_AUGD);
}
