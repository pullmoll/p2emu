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
#include "p2token.h"
#include "p2doc.h"
#include "p2cog.h"
#include "p2dasm.h"

P2Dasm::P2Dasm(const P2Cog* cog, QObject* parent)
    : QObject(parent)
    , COG(cog)
    , m_lowercase(false)
    , pad_opcode(40)
    , pad_inst(-10)
    , pad_wcz(24)
    , IR()
    , S(0)
    , D(0)
{
}

p2_token_e P2Dasm::conditional(p2_cond_e cond)
{
    switch (cond) {
    case cc__ret_:        // execute always
        return t__RET_;
    case cc_nc_and_nz:    // execute if C = 0 and Z = 0
        return t_IF_NC_AND_NZ;
    case cc_nc_and_z:     // execute if C = 0 and Z = 1
        return t_IF_NC_AND_Z;
    case cc_nc:           // execute if C = 0
        return t_IF_NC;
    case cc_c_and_nz:     // execute if C = 1 and Z = 0
        return t_IF_C_AND_NZ;
    case cc_nz:           // execute if Z = 0
        return t_IF_NZ;
    case cc_c_ne_z:       // execute if C != Z
        return t_IF_C_NE_Z;
    case cc_nc_or_nz:     // execute if C = 0 or Z = 0
        return t_IF_NC_OR_NZ;
    case cc_c_and_z:      // execute if C = 1 and Z = 1
        return t_IF_C_AND_Z;
    case cc_c_eq_z:       // execute if C = Z
        return t_IF_C_EQ_Z;
    case cc_z:            // execute if Z = 1
        return t_IF_Z;
    case cc_nc_or_z:      // execute if C = 0 or Z = 1
        return t_IF_NC_OR_Z;
    case cc_c:            // execute if C = 1
        return t_IF_C;
    case cc_c_or_nz:      // execute if C = 1 or Z = 0
        return t_IF_C_OR_NZ;
    case cc_c_or_z:       // execute if C = 1 or Z = 1
        return t_IF_C_OR_Z;
    case cc_always:
        // return t_IF_ALWAYS;
        return t_none;
    }
    return t_invalid;
}

p2_token_e P2Dasm::conditional(unsigned cond)
{
    return conditional(static_cast<p2_cond_e>(cond));
}

bool P2Dasm::dasm(p2_LONG addr, QString* opcode, QString* instruction, QString* brief)
{

    IR.u.opcode = COG->rd_mem(addr*4);
    PC = addr * 4 < HUB_ADDR0 ? addr * 4 : addr;
    S = COG->rd_cog(IR.u.op.src);
    D = COG->rd_cog(IR.u.op.dst);

    // check for the condition
    QString cond;
    if (IR.u.opcode)
        cond = Token.string(conditional(IR.u.op.cond), m_lowercase);
    cond.resize(14, QChar::Space);

    if (opcode) {
        // format opcode from bit fields
        *opcode = QString("%1_%2_%3%4%5_%6_%7")
                  .arg(format_bin(IR.u.op.cond, 4))
                  .arg(format_bin(IR.u.op.inst, 7))
                  .arg(format_bin(IR.wc(), 1))
                  .arg(format_bin(IR.wz(), 1))
                  .arg(format_bin(IR.im(), 1))
                  .arg(format_bin(IR.u.op.dst, 9))
                  .arg(format_bin(IR.u.op.src, 9));
        opcode->resize(pad_opcode, QChar::Space);
    }

    if (instruction) {
        // Dispatch to dasm_xxx() functions
        switch (IR.inst7()) {
        case p2_ROR:           dasm_ror(instruction); break;
        case p2_ROL:           dasm_rol(instruction); break;
        case p2_SHR:           dasm_shr(instruction); break;
        case p2_SHL:           dasm_shl(instruction); break;
        case p2_RCR:           dasm_rcr(instruction); break;
        case p2_RCL:           dasm_rcl(instruction); break;
        case p2_SAR:           dasm_sar(instruction); break;
        case p2_SAL:           dasm_sal(instruction); break;
        case p2_ADD:           dasm_add(instruction); break;
        case p2_ADDX:          dasm_addx(instruction); break;
        case p2_ADDS:          dasm_adds(instruction); break;
        case p2_ADDSX:         dasm_addsx(instruction); break;
        case p2_SUB:           dasm_sub(instruction); break;
        case p2_SUBX:          dasm_subx(instruction); break;
        case p2_SUBS:          dasm_subs(instruction); break;
        case p2_SUBSX:         dasm_subsx(instruction); break;
        case p2_CMP:           dasm_cmp(instruction); break;
        case p2_CMPX:          dasm_cmpx(instruction); break;
        case p2_CMPS:          dasm_cmps(instruction); break;
        case p2_CMPSX:         dasm_cmpsx(instruction); break;
        case p2_CMPR:          dasm_cmpr(instruction); break;
        case p2_CMPM:          dasm_cmpm(instruction); break;
        case p2_SUBR:          dasm_subr(instruction); break;
        case p2_CMPSUB:        dasm_cmpsub(instruction); break;
        case p2_FGE:           dasm_fge(instruction); break;
        case p2_FLE:           dasm_fle(instruction); break;
        case p2_FGES:          dasm_fges(instruction); break;
        case p2_FLES:          dasm_fles(instruction); break;
        case p2_SUMC:          dasm_sumc(instruction); break;
        case p2_SUMNC:         dasm_sumnc(instruction); break;
        case p2_SUMZ:          dasm_sumz(instruction); break;
        case p2_SUMNZ:         dasm_sumnz(instruction); break;
        case p2_TESTB_W_BITL:
            switch (IR.inst9()) {
            case p2_BITL_eol:
                dasm_bitl(instruction);
                break;
            case p2_TESTB_WZ:
                dasm_testb_w(instruction);
                break;
            case p2_TESTB_WC:
                dasm_testb_w(instruction);
                break;
            case p2_BITL_WCZ:
                dasm_bitl(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst9_e", "TESTB WC/WZ or BITL {WCZ}");
            }
            break;

        case p2_TESTBN_W_BITH:
            switch (IR.inst9()) {
            case p2_BITH_eol:
                dasm_bith(instruction);
                break;
            case p2_TESTBN_WZ:
                dasm_testbn_w(instruction);
                break;
            case p2_TESTBN_WC:
                dasm_testbn_w(instruction);
                break;
            case p2_BITH_WCZ:
                dasm_bith(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst9_e", "TESTBN WC/WZ or BITH {WCZ}");
            }
            break;

        case p2_TESTB_AND_BITC:
            switch (IR.inst9()) {
            case p2_BITC_eol:
                dasm_bitc(instruction);
                break;
            case p2_TESTB_ANDZ:
                dasm_testb_and(instruction);
                break;
            case p2_TESTB_ANDC:
                dasm_testb_and(instruction);
                break;
            case p2_BITC_WCZ:
                dasm_bitc(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst9_e", "TESTB ANDC/ANDZ or BITC {WCZ}");
            }
            break;

        case p2_TESTBN_AND_BITNC:
            switch (IR.inst9()) {
            case p2_BITNC_eol:
                dasm_bitnc(instruction);
                break;
            case p2_TESTBN_ANDZ:
                dasm_testbn_and(instruction);
                break;
            case p2_TESTBN_ANDC:
                dasm_testbn_and(instruction);
                break;
            case p2_BITNC_WCZ:
                dasm_bitnc(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst9_e", "TESTBN ANDC/ANDZ or BITNC {WCZ}");
            }
            break;

        case p2_TESTB_OR_BITZ:
            switch (IR.inst9()) {
            case p2_BITZ_eol:
                dasm_bitz(instruction);
                break;
            case p2_TESTB_ORZ:
                dasm_testb_or(instruction);
                break;
            case p2_TESTB_ORC:
                dasm_testb_or(instruction);
                break;
            case p2_BITZ_WCZ:
                dasm_bitz(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst9_e", "TESTB ORC/ORZ or BITZ {WCZ}");
            }
            break;

        case p2_TESTBN_OR_BITNZ:
            switch (IR.inst9()) {
            case p2_BITNZ_eol:
                dasm_bitnz(instruction);
                break;
            case p2_TESTBN_ORZ:
                dasm_testbn_or(instruction);
                break;
            case p2_TESTBN_ORC:
                dasm_testbn_or(instruction);
                break;
            case p2_BITNZ_WCZ:
                dasm_bitnz(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst9_e", "TESTBN ORC/ORZ or BITNZ {WCZ}");
            }
            break;

        case p2_TESTB_XOR_BITRND:
            switch (IR.inst9()) {
            case p2_BITRND_eol:
                dasm_bitl(instruction);
                break;
            case p2_TESTB_XORZ:
                dasm_testb_xor(instruction);
                break;
            case p2_TESTB_XORC:
                dasm_testb_xor(instruction);
                break;
            case p2_BITRND_WCZ:
                dasm_bitl(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst9_e", "TESTB XORC/XORZ or BITRND {WCZ}");
            }
            break;

        case p2_TESTBN_XOR_BITNOT:
            switch (IR.inst9()) {
            case p2_BITRND_eol:
                dasm_bitnot(instruction);
                break;
            case p2_TESTB_XORZ:
                dasm_testbn_xor(instruction);
                break;
            case p2_TESTB_XORC:
                dasm_testbn_xor(instruction);
                break;
            case p2_BITRND_WCZ:
                dasm_bitnot(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst9_e", "TESTB XORC/XORZ or BITRND {WCZ}");
            }
            break;

        case p2_AND:
            dasm_and(instruction);
            break;

        case p2_ANDN:
            dasm_andn(instruction);
            break;

        case p2_OR:
            dasm_or(instruction);
            break;

        case p2_XOR:
            dasm_xor(instruction);
            break;

        case p2_MUXC:
            dasm_muxc(instruction);
            break;

        case p2_MUXNC:
            dasm_muxnc(instruction);
            break;

        case p2_MUXZ:
            dasm_muxz(instruction);
            break;

        case p2_MUXNZ:
            dasm_muxnz(instruction);
            break;

        case p2_MOV:
            dasm_mov(instruction);
            break;

        case p2_NOT:
            dasm_not(instruction);
            break;

        case p2_ABS:
            dasm_abs(instruction);
            break;

        case p2_NEG:
            dasm_neg(instruction);
            break;

        case p2_NEGC:
            dasm_negc(instruction);
            break;

        case p2_NEGNC:
            dasm_negnc(instruction);
            break;

        case p2_NEGZ:
            dasm_negz(instruction);
            break;

        case p2_NEGNZ:
            dasm_negnz(instruction);
            break;

        case p2_INCMOD:
            dasm_incmod(instruction);
            break;

        case p2_DECMOD:
            dasm_decmod(instruction);
            break;

        case p2_ZEROX:
            dasm_zerox(instruction);
            break;

        case p2_SIGNX:
            dasm_signx(instruction);
            break;

        case p2_ENCOD:
            dasm_encod(instruction);
            break;

        case p2_ONES:
            dasm_ones(instruction);
            break;

        case p2_TEST:
            dasm_test(instruction);
            break;

        case p2_TESTN:
            dasm_testn(instruction);
            break;

        case p2_SETNIB_0:
        case p2_SETNIB_1:
            dasm_setnib(instruction);
            break;

        case p2_GETNIB_0:
        case p2_GETNIB_1:
            dasm_getnib(instruction);
            break;

        case p2_ROLNIB_0:
        case p2_ROLNIB_1:
            dasm_rolnib(instruction);
            break;

        case p2_SETBYTE:
            dasm_setbyte(instruction);
            break;

        case p2_GETBYTE:
            dasm_getbyte(instruction);
            break;

        case p2_ROLBYTE:
            dasm_rolbyte(instruction);
            break;

        case p2_SETWORD_GETWORD:
            switch (IR.inst9()) {
            case p2_SETWORD_ALTSW:
                dasm_setword_altsw(instruction);
                break;
            case p2_SETWORD:
                dasm_setword(instruction);
                break;
            case p2_GETWORD_ALTGW:
                dasm_getword_altgw(instruction);
                break;
            case p2_GETWORD:
                dasm_getword(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst9_e", "SETWORD/GETWORD");
            }
            break;

        case p2_ROLWORD_ALTSN_ALTGN:
            switch (IR.inst9()) {
            case p2_ROLWORD_ALTGW:
                if (IR.src() == 0) {
                    dasm_rolword_altgw(instruction);
                    break;
                }
                dasm_rolword(instruction);
                break;
            case p2_ROLWORD:
                dasm_rolword(instruction);
                break;
            case p2_ALTSN:
                if (IR.src() == 0 && IR.im() == 1) {
                    dasm_altsn_d(instruction);
                    break;
                }
                dasm_altsn(instruction);
                break;
            case p2_ALTGN:
                if (IR.src() == 0 && IR.im() == 1) {
                    dasm_altgn_d(instruction);
                    break;
                }
                dasm_altgn(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst9_e", "ROLWORD/ALTSN/ALTGN");
            }
            break;

        case p2_ALTSB_ALTGB_ALTSW_ALTGW:
            switch (IR.inst9()) {
            case p2_ALTSB:
                if (IR.src() == 0 && IR.im() == 1) {
                    dasm_altsb_d(instruction);
                    break;
                }
                dasm_altsb(instruction);
                break;
            case p2_ALTGB:
                if (IR.src() == 0 && IR.im() == 1) {
                    dasm_altgb_d(instruction);
                    break;
                }
                dasm_altgb(instruction);
                break;
            case p2_ALTSW:
                if (IR.src() == 0 && IR.im() == 1) {
                    dasm_altsw_d(instruction);
                    break;
                }
                dasm_altsw(instruction);
                break;
            case p2_ALTGW:
                if (IR.src() == 0 && IR.im() == 1) {
                    dasm_altgw_d(instruction);
                    break;
                }
                dasm_altgw(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst9_e", "ALTSB/ALTGB/ALTSW/ALTGW");
            }
            break;

        case p2_ALTR_ALTD_ALTS_ALTB:
            switch (IR.inst9()) {
            case p2_ALTR:
                if (IR.src() == 0 && IR.im() == 1) {
                    dasm_altr_d(instruction);
                    break;
                }
                dasm_altr(instruction);
                break;
            case p2_ALTD:
                if (IR.src() == 0 && IR.im() == 1) {
                    dasm_altd_d(instruction);
                    break;
                }
                dasm_altd(instruction);
                break;
            case p2_ALTS:
                if (IR.src() == 0 && IR.im() == 1) {
                    dasm_alts_d(instruction);
                    break;
                }
                dasm_alts(instruction);
                break;
            case p2_ALTB:
                if (IR.src() == 0 && IR.im() == 1) {
                    dasm_altb_d(instruction);
                    break;
                }
                dasm_altb(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst9_e", "ALTR/ALTD/ALTS/ALTB");
            }
            break;

        case p2_ALTI_SETR_SETD_SETS:
            switch (IR.inst9()) {
            case p2_ALTI:
                if (IR.im() == 1 && IR.src() == 0x164 /* 101100100 */) {
                    dasm_alti_d(instruction);
                    break;
                }
                dasm_alti(instruction);
                break;
            case p2_SETR:
                dasm_setr(instruction);
                break;
            case p2_SETD:
                dasm_setd(instruction);
                break;
            case p2_SETS:
                dasm_sets(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst9_e", "ALTI/SETR/SETD/SETS");
            }
            break;

        case p2_DECOD_BMASK_CRCBIT_CRCNIB:
            switch (IR.inst9()) {
            case p2_DECOD:
                if (IR.im() == 0 && IR.src() == IR.u.op.dst) {
                    dasm_decod_d(instruction);
                    break;
                }
                dasm_decod(instruction);
                break;
            case p2_BMASK:
                if (IR.im() == 0 && IR.src() == IR.u.op.dst) {
                    dasm_bmask_d(instruction);
                    break;
                }
                dasm_bmask(instruction);
                break;
            case p2_CRCBIT:
                dasm_crcbit(instruction);
                break;
            case p2_CRCNIB:
                dasm_crcnib(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst9_e", "DECOD/BMASK/CRCBIT/CRCNIB");
            }
            break;

        case p2_MUX_NITS_NIBS_Q_MOVBYTS:
            switch (IR.inst9()) {
            case p2_MUXNITS:
                dasm_muxnits(instruction);
                break;
            case p2_MUXNIBS:
                dasm_muxnibs(instruction);
                break;
            case p2_MUXQ:
                dasm_muxq(instruction);
                break;
            case p2_MOVBYTS:
                dasm_movbyts(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst9_e", "MUXNITS/MUXNIBS/MUXQ/MOVBYTS");
            }
            break;

        case p2_MUL_MULS:
            switch (IR.inst8()) {
            case p2_MUL:
                dasm_mul(instruction);
                break;
            case p2_MULS:
                dasm_muls(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst8_e", "MUL/MULS");
            }
            break;

        case p2_SCA_SCAS:
            switch (IR.inst8()) {
            case p2_SCA:
                dasm_sca(instruction);
                break;
            case p2_SCAS:
                dasm_scas(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst8_e", "SCA/SCAS");
            }
            break;

        case p2_XXXPIX:
            switch (IR.inst9()) {
            case p2_ADDPIX:
                dasm_addpix(instruction);
                break;
            case p2_MULPIX:
                dasm_mulpix(instruction);
                break;
            case p2_BLNPIX:
                dasm_blnpix(instruction);
                break;
            case p2_MIXPIX:
                dasm_mixpix(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst9_e", "ADDPIX/MULPIX/BLNPIX/MIXPIX");
            }
            break;

        case p2_WMLONG_ADDCTx:
            switch (IR.inst9()) {
            case p2_ADDCT1:
                dasm_addct1(instruction);
                break;
            case p2_ADDCT2:
                dasm_addct2(instruction);
                break;
            case p2_ADDCT3:
                dasm_addct3(instruction);
                break;
            case p2_WMLONG:
                dasm_wmlong(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst9_e", "ADDCT1/ADDCT2/ADDCT3/WMLONG");
            }
            break;

        case p2_RQPIN_RDPIN:
            switch (IR.inst8()) {
            case p2_RQPIN:
                dasm_rqpin(instruction);
                break;
            case p2_RDPIN:
                dasm_rdpin(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst8_e", "RQPIN/RDPIN");
            }
            break;

        case p2_RDLUT:
            dasm_rdlut(instruction);
            break;

        case p2_RDBYTE:
            dasm_rdbyte(instruction);
            break;

        case p2_RDWORD:
            dasm_rdword(instruction);
            break;

        case p2_RDLONG:
            dasm_rdlong(instruction);
            break;

        case p2_CALLD:
            dasm_calld(instruction);
            break;

        case p2_CALLPA_CALLPB:
            switch (IR.inst8()) {
            case p2_CALLPA:
                dasm_callpa(instruction);
                break;
            case p2_CALLPB:
                dasm_callpb(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst8_e", "CALLPA/CALLPA");
            }
            break;

        case p2_DJZ_DJNZ_DJF_DJNF:
            switch (IR.inst9()) {
            case p2_DJZ:
                dasm_djz(instruction);
                break;
            case p2_DJNZ:
                dasm_djnz(instruction);
                break;
            case p2_DJF:
                dasm_djf(instruction);
                break;
            case p2_DJNF:
                dasm_djnf(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst9_e", "DJZ/DJNZ/DJF/DJNF");
            }
            break;

        case p2_IJZ_IJNZ_TJZ_TJNZ:
            switch (IR.inst9()) {
            case p2_IJZ:
                dasm_ijz(instruction);
                break;
            case p2_IJNZ:
                dasm_ijnz(instruction);
                break;
            case p2_TJZ:
                dasm_tjz(instruction);
                break;
            case p2_TJNZ:
                dasm_tjnz(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst9_e", "IJZ/IJNZ/TJZ/TJNZ");
            }
            break;

        case p2_TJF_TJNF_TJS_TJNS:
            switch (IR.inst9()) {
            case p2_TJF:
                dasm_tjf(instruction);
                break;
            case p2_TJNF:
                dasm_tjnf(instruction);
                break;
            case p2_TJS:
                dasm_tjs(instruction);
                break;
            case p2_TJNS:
                dasm_tjns(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst9_e", "TJF/TJNF/TJS/TJNS");
            }
            break;

        case p2_TJV_OPDST_empty:
            switch (IR.inst8()) {
            case p2_TJV_OPDST:
                switch (IR.inst9()) {
                case p2_TJV:
                    dasm_tjv(instruction);
                    break;
                case p2_OPDST:
                    switch (IR.u.op.dst) {
                    case p2_OPDST_JINT:
                        dasm_jint(instruction);
                        break;
                    case p2_OPDST_JCT1:
                        dasm_jct1(instruction);
                        break;
                    case p2_OPDST_JCT2:
                        dasm_jct2(instruction);
                        break;
                    case p2_OPDST_JCT3:
                        dasm_jct3(instruction);
                        break;
                    case p2_OPDST_JSE1:
                        dasm_jse1(instruction);
                        break;
                    case p2_OPDST_JSE2:
                        dasm_jse2(instruction);
                        break;
                    case p2_OPDST_JSE3:
                        dasm_jse3(instruction);
                        break;
                    case p2_OPDST_JSE4:
                        dasm_jse4(instruction);
                        break;
                    case p2_OPDST_JPAT:
                        dasm_jpat(instruction);
                        break;
                    case p2_OPDST_JFBW:
                        dasm_jfbw(instruction);
                        break;
                    case p2_OPDST_JXMT:
                        dasm_jxmt(instruction);
                        break;
                    case p2_OPDST_JXFI:
                        dasm_jxfi(instruction);
                        break;
                    case p2_OPDST_JXRO:
                        dasm_jxro(instruction);
                        break;
                    case p2_OPDST_JXRL:
                        dasm_jxrl(instruction);
                        break;
                    case p2_OPDST_JATN:
                        dasm_jatn(instruction);
                        break;
                    case p2_OPDST_JQMT:
                        dasm_jqmt(instruction);
                        break;
                    case p2_OPDST_JNINT:
                        dasm_jnint(instruction);
                        break;
                    case p2_OPDST_JNCT1:
                        dasm_jnct1(instruction);
                        break;
                    case p2_OPDST_JNCT2:
                        dasm_jnct2(instruction);
                        break;
                    case p2_OPDST_JNCT3:
                        dasm_jnct3(instruction);
                        break;
                    case p2_OPDST_JNSE1:
                        dasm_jnse1(instruction);
                        break;
                    case p2_OPDST_JNSE2:
                        dasm_jnse2(instruction);
                        break;
                    case p2_OPDST_JNSE3:
                        dasm_jnse3(instruction);
                        break;
                    case p2_OPDST_JNSE4:
                        dasm_jnse4(instruction);
                        break;
                    case p2_OPDST_JNPAT:
                        dasm_jnpat(instruction);
                        break;
                    case p2_OPDST_JNFBW:
                        dasm_jnfbw(instruction);
                        break;
                    case p2_OPDST_JNXMT:
                        dasm_jnxmt(instruction);
                        break;
                    case p2_OPDST_JNXFI:
                        dasm_jnxfi(instruction);
                        break;
                    case p2_OPDST_JNXRO:
                        dasm_jnxro(instruction);
                        break;
                    case p2_OPDST_JNXRL:
                        dasm_jnxrl(instruction);
                        break;
                    case p2_OPDST_JNATN:
                        dasm_jnatn(instruction);
                        break;
                    case p2_OPDST_JNQMT:
                        dasm_jnqmt(instruction);
                        break;
                    default:
                        // TODO: invalid D value
                        break;
                    }
                    break;
                default:
                    Q_ASSERT_X(false, "p2_inst9_e", "TJV/OPDST/1011110_10/1011110_11");
                }
                break;
            case p2_1011110_1:
                switch (IR.inst9()) {
                case p2_1011110_10:
                case p2_1011110_11:
                    dasm_1011110_1(instruction);
                    break;
                default:
                    Q_ASSERT_X(false, "p2_inst9_e", "TJV/OPDST/1011110_10/1011110_11");
                }
                break;
            default:
                break;
            }
            break;

        case p2_empty_SETPAT:
            switch (IR.inst8()) {
            case p2_1011111_0:
                dasm_1011111_0(instruction);
                break;
            case p2_SETPAT:
                dasm_setpat(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst8_e", "1011111_0/SETPAT");
            }
            break;

        case p2_WRPIN_AKPIN_WXPIN:
            switch (IR.inst8()) {
            case p2_WRPIN:
                if (IR.wz() == 1 && IR.dst() == 1) {
                    dasm_akpin(instruction);
                    break;
                }
                dasm_wrpin(instruction);
                break;
            case p2_WXPIN:
                dasm_wxpin(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst8_e", "WRPIN/AKPIN/WXPIN");
            }
            break;

        case p2_WYPIN_WRLUT:
            switch (IR.inst8()) {
            case p2_WYPIN:
                dasm_wypin(instruction);
                break;
            case p2_WRLUT:
                dasm_wrlut(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst8_e", "WYPIN/WRLUT");
            }
            break;

        case p2_WRBYTE_WRWORD:
            switch (IR.inst8()) {
            case p2_WRBYTE:
                dasm_wrbyte(instruction);
                break;
            case p2_WRWORD:
                dasm_wrword(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst8_e", "WRBYTE/WRWORD");
            }
            break;

        case p2_WRLONG_RDFAST:
            switch (IR.inst8()) {
            case p2_WRLONG:
                dasm_wrlong(instruction);
                break;
            case p2_RDFAST:
                dasm_rdfast(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst8_e", "WRLONG/RDFAST");
            }
            break;

        case p2_WRFAST_FBLOCK:
            switch (IR.inst8()) {
            case p2_WRFAST:
                dasm_wrfast(instruction);
                break;
            case p2_FBLOCK:
                dasm_fblock(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst8_e", "WRFAST/FBLOCK");
            }
            break;

        case p2_XINIT_XSTOP_XZERO:
            switch (IR.inst8()) {
            case p2_XINIT:
                if (IR.wz() == 1 && IR.im() == 1 && IR.src() == 0 && IR.dst() == 0) {
                    dasm_xstop(instruction);
                    break;
                }
                dasm_xinit(instruction);
                break;
            case p2_XZERO:
                dasm_xzero(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst8_e", "XINIT/XSTOP/XZERO");
            }
            break;

        case p2_XCONT_REP:
            switch (IR.inst8()) {
            case p2_XCONT:
                dasm_xcont(instruction);
                break;
            case p2_REP:
                dasm_rep(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst8_e", "XCONT/REP");
            }
            break;

        case p2_COGINIT:
            dasm_coginit(instruction);
            break;

        case p2_QMUL_QDIV:
            switch (IR.inst8()) {
            case p2_QMUL:
                dasm_qmul(instruction);
                break;
            case p2_QDIV:
                dasm_qdiv(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst8_e", "QMUL/QDIV");
            }
            break;

        case p2_QFRAC_QSQRT:
            switch (IR.inst8()) {
            case p2_QFRAC:
                dasm_qfrac(instruction);
                break;
            case p2_QSQRT:
                dasm_qsqrt(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst8_e", "QFRAC/QSQRT");
            }
            break;

        case p2_QROTATE_QVECTOR:
            switch (IR.inst8()) {
            case p2_QROTATE:
                dasm_qrotate(instruction);
                break;
            case p2_QVECTOR:
                dasm_qvector(instruction);
                break;
            default:
                Q_ASSERT_X(false, "p2_inst8_e", "QROTATE/QVECTOR");
            }
            break;

        case p2_OPSRC:
            switch (IR.u.op.src) {
            case p2_OPSRC_HUBSET:
                dasm_hubset(instruction);
                break;
            case p2_OPSRC_COGID:
                dasm_cogid(instruction);
                break;
            case p2_OPSRC_COGSTOP:
                dasm_cogstop(instruction);
                break;
            case p2_OPSRC_LOCKNEW:
                dasm_locknew(instruction);
                break;
            case p2_OPSRC_LOCKRET:
                dasm_lockret(instruction);
                break;
            case p2_OPSRC_LOCKTRY:
                dasm_locktry(instruction);
                break;
            case p2_OPSRC_LOCKREL:
                dasm_lockrel(instruction);
                break;
            case p2_OPSRC_QLOG:
                dasm_qlog(instruction);
                break;
            case p2_OPSRC_QEXP:
                dasm_qexp(instruction);
                break;
            case p2_OPSRC_RFBYTE:
                dasm_rfbyte(instruction);
                break;
            case p2_OPSRC_RFWORD:
                dasm_rfword(instruction);
                break;
            case p2_OPSRC_RFLONG:
                dasm_rflong(instruction);
                break;
            case p2_OPSRC_RFVAR:
                dasm_rfvar(instruction);
                break;
            case p2_OPSRC_RFVARS:
                dasm_rfvars(instruction);
                break;
            case p2_OPSRC_WFBYTE:
                dasm_wfbyte(instruction);
                break;
            case p2_OPSRC_WFWORD:
                dasm_wfword(instruction);
                break;
            case p2_OPSRC_WFLONG:
                dasm_wflong(instruction);
                break;
            case p2_OPSRC_GETQX:
                dasm_getqx(instruction);
                break;
            case p2_OPSRC_GETQY:
                dasm_getqy(instruction);
                break;
            case p2_OPSRC_GETCT:
                dasm_getct(instruction);
                break;
            case p2_OPSRC_GETRND:
                (IR.dst() == 0) ? dasm_getrnd_cz(instruction)
                                   : dasm_getrnd(instruction);
                break;
            case p2_OPSRC_SETDACS:
                dasm_setdacs(instruction);
                break;
            case p2_OPSRC_SETXFRQ:
                dasm_setxfrq(instruction);
                break;
            case p2_OPSRC_GETXACC:
                dasm_getxacc(instruction);
                break;
            case p2_OPSRC_WAITX:
                dasm_waitx(instruction);
                break;
            case p2_OPSRC_SETSE1:
                dasm_setse1(instruction);
                break;
            case p2_OPSRC_SETSE2:
                dasm_setse2(instruction);
                break;
            case p2_OPSRC_SETSE3:
                dasm_setse3(instruction);
                break;
            case p2_OPSRC_SETSE4:
                dasm_setse4(instruction);
                break;
            case p2_OPSRC_X24:
                switch (IR.u.op.dst) {
                case p2_OPX24_POLLINT:
                    dasm_pollint(instruction);
                    break;
                case p2_OPX24_POLLCT1:
                    dasm_pollct1(instruction);
                    break;
                case p2_OPX24_POLLCT2:
                    dasm_pollct2(instruction);
                    break;
                case p2_OPX24_POLLCT3:
                    dasm_pollct3(instruction);
                    break;
                case p2_OPX24_POLLSE1:
                    dasm_pollse1(instruction);
                    break;
                case p2_OPX24_POLLSE2:
                    dasm_pollse2(instruction);
                    break;
                case p2_OPX24_POLLSE3:
                    dasm_pollse3(instruction);
                    break;
                case p2_OPX24_POLLSE4:
                    dasm_pollse4(instruction);
                    break;
                case p2_OPX24_POLLPAT:
                    dasm_pollpat(instruction);
                    break;
                case p2_OPX24_POLLFBW:
                    dasm_pollfbw(instruction);
                    break;
                case p2_OPX24_POLLXMT:
                    dasm_pollxmt(instruction);
                    break;
                case p2_OPX24_POLLXFI:
                    dasm_pollxfi(instruction);
                    break;
                case p2_OPX24_POLLXRO:
                    dasm_pollxro(instruction);
                    break;
                case p2_OPX24_POLLXRL:
                    dasm_pollxrl(instruction);
                    break;
                case p2_OPX24_POLLATN:
                    dasm_pollatn(instruction);
                    break;
                case p2_OPX24_POLLQMT:
                    dasm_pollqmt(instruction);
                    break;
                case p2_OPX24_WAITINT:
                    dasm_waitint(instruction);
                    break;
                case p2_OPX24_WAITCT1:
                    dasm_waitct1(instruction);
                    break;
                case p2_OPX24_WAITCT2:
                    dasm_waitct2(instruction);
                    break;
                case p2_OPX24_WAITCT3:
                    dasm_waitct3(instruction);
                    break;
                case p2_OPX24_WAITSE1:
                    dasm_waitse1(instruction);
                    break;
                case p2_OPX24_WAITSE2:
                    dasm_waitse2(instruction);
                    break;
                case p2_OPX24_WAITSE3:
                    dasm_waitse3(instruction);
                    break;
                case p2_OPX24_WAITSE4:
                    dasm_waitse4(instruction);
                    break;
                case p2_OPX24_WAITPAT:
                    dasm_waitpat(instruction);
                    break;
                case p2_OPX24_WAITFBW:
                    dasm_waitfbw(instruction);
                    break;
                case p2_OPX24_WAITXMT:
                    dasm_waitxmt(instruction);
                    break;
                case p2_OPX24_WAITXFI:
                    dasm_waitxfi(instruction);
                    break;
                case p2_OPX24_WAITXRO:
                    dasm_waitxro(instruction);
                    break;
                case p2_OPX24_WAITXRL:
                    dasm_waitxrl(instruction);
                    break;
                case p2_OPX24_WAITATN:
                    dasm_waitatn(instruction);
                    break;
                case p2_OPX24_ALLOWI:
                    dasm_allowi(instruction);
                    break;
                case p2_OPX24_STALLI:
                    dasm_stalli(instruction);
                    break;
                case p2_OPX24_TRGINT1:
                    dasm_trgint1(instruction);
                    break;
                case p2_OPX24_TRGINT2:
                    dasm_trgint2(instruction);
                    break;
                case p2_OPX24_TRGINT3:
                    dasm_trgint3(instruction);
                    break;
                case p2_OPX24_NIXINT1:
                    dasm_nixint1(instruction);
                    break;
                case p2_OPX24_NIXINT2:
                    dasm_nixint2(instruction);
                    break;
                case p2_OPX24_NIXINT3:
                    dasm_nixint3(instruction);
                    break;
                }
                break;
            case p2_OPSRC_SETINT1:
                dasm_setint1(instruction);
                break;
            case p2_OPSRC_SETINT2:
                dasm_setint2(instruction);
                break;
            case p2_OPSRC_SETINT3:
                dasm_setint3(instruction);
                break;
            case p2_OPSRC_SETQ:
                dasm_setq(instruction);
                break;
            case p2_OPSRC_SETQ2:
                dasm_setq2(instruction);
                break;
            case p2_OPSRC_PUSH:
                dasm_push(instruction);
                break;
            case p2_OPSRC_POP:
                dasm_pop(instruction);
                break;
            case p2_OPSRC_JMP:
                dasm_jmp(instruction);
                break;
            case p2_OPSRC_CALL_RET:
                (IR.im() == 0) ? dasm_call(instruction)
                               : dasm_ret(instruction);
                break;
            case p2_OPSRC_CALLA_RETA:
                (IR.im() == 0) ? dasm_calla(instruction)
                               : dasm_reta(instruction);
                break;
            case p2_OPSRC_CALLB_RETB:
                (IR.im() == 0) ? dasm_callb(instruction)
                               : dasm_retb(instruction);
                break;
            case p2_OPSRC_JMPREL:
                dasm_jmprel(instruction);
                break;
            case p2_OPSRC_SKIP:
                dasm_skip(instruction);
                break;
            case p2_OPSRC_SKIPF:
                dasm_skipf(instruction);
                break;
            case p2_OPSRC_EXECF:
                dasm_execf(instruction);
                break;
            case p2_OPSRC_GETPTR:
                dasm_getptr(instruction);
                break;
            case p2_OPSRC_COGBRK:
                (IR.wc() == 0 && IR.wz() == 0) ? dasm_cogbrk(instruction)
                                               : dasm_getbrk(instruction);
                break;
            case p2_OPSRC_BRK:
                dasm_brk(instruction);
                break;
            case p2_OPSRC_SETLUTS:
                dasm_setluts(instruction);
                break;
            case p2_OPSRC_SETCY:
                dasm_setcy(instruction);
                break;
            case p2_OPSRC_SETCI:
                dasm_setci(instruction);
                break;
            case p2_OPSRC_SETCQ:
                dasm_setcq(instruction);
                break;
            case p2_OPSRC_SETCFRQ:
                dasm_setcfrq(instruction);
                break;
            case p2_OPSRC_SETCMOD:
                dasm_setcmod(instruction);
                break;
            case p2_OPSRC_SETPIV:
                dasm_setpiv(instruction);
                break;
            case p2_OPSRC_SETPIX:
                dasm_setpix(instruction);
                break;
            case p2_OPSRC_COGATN:
                dasm_cogatn(instruction);
                break;
            case p2_OPSRC_TESTP_W_DIRL:
                (IR.wc() != IR.wz()) ? dasm_testp_w(instruction)
                                     : dasm_dirl(instruction);
                break;
            case p2_OPSRC_TESTPN_W_DIRH:
                (IR.wc() != IR.wz()) ? dasm_testpn_w(instruction)
                                     : dasm_dirh(instruction);
                break;
            case p2_OPSRC_TESTP_AND_DIRC:
                (IR.wc() != IR.wz()) ? dasm_testp_and(instruction)
                                     : dasm_dirc(instruction);
                break;
            case p2_OPSRC_TESTPN_AND_DIRNC:
                (IR.wc() != IR.wz()) ? dasm_testpn_and(instruction)
                                     : dasm_dirnc(instruction);
                break;
            case p2_OPSRC_TESTP_OR_DIRZ:
                (IR.wc() != IR.wz()) ? dasm_testp_or(instruction)
                                     : dasm_dirz(instruction);
                break;
            case p2_OPSRC_TESTPN_OR_DIRNZ:
                (IR.wc() != IR.wz()) ? dasm_testpn_or(instruction)
                                     : dasm_dirnz(instruction);
                break;
            case p2_OPSRC_TESTP_XOR_DIRRND:
                (IR.wc() != IR.wz()) ? dasm_testp_xor(instruction)
                                     : dasm_dirrnd(instruction);
                break;
            case p2_OPSRC_TESTPN_XOR_DIRNOT:
                (IR.wc() != IR.wz()) ? dasm_testpn_xor(instruction)
                                     : dasm_dirnot(instruction);
                break;
            case p2_OPSRC_OUTL:
                dasm_outl(instruction);
                break;
            case p2_OPSRC_OUTH:
                dasm_outh(instruction);
                break;
            case p2_OPSRC_OUTC:
                dasm_outc(instruction);
                break;
            case p2_OPSRC_OUTNC:
                dasm_outnc(instruction);
                break;
            case p2_OPSRC_OUTZ:
                dasm_outz(instruction);
                break;
            case p2_OPSRC_OUTNZ:
                dasm_outnz(instruction);
                break;
            case p2_OPSRC_OUTRND:
                dasm_outrnd(instruction);
                break;
            case p2_OPSRC_OUTNOT:
                dasm_outnot(instruction);
                break;
            case p2_OPSRC_FLTL:
                dasm_fltl(instruction);
                break;
            case p2_OPSRC_FLTH:
                dasm_flth(instruction);
                break;
            case p2_OPSRC_FLTC:
                dasm_fltc(instruction);
                break;
            case p2_OPSRC_FLTNC:
                dasm_fltnc(instruction);
                break;
            case p2_OPSRC_FLTZ:
                dasm_fltz(instruction);
                break;
            case p2_OPSRC_FLTNZ:
                dasm_fltnz(instruction);
                break;
            case p2_OPSRC_FLTRND:
                dasm_fltrnd(instruction);
                break;
            case p2_OPSRC_FLTNOT:
                dasm_fltnot(instruction);
                break;
            case p2_OPSRC_DRVL:
                dasm_drvl(instruction);
                break;
            case p2_OPSRC_DRVH:
                dasm_drvh(instruction);
                break;
            case p2_OPSRC_DRVC:
                dasm_drvc(instruction);
                break;
            case p2_OPSRC_DRVNC:
                dasm_drvnc(instruction);
                break;
            case p2_OPSRC_DRVZ:
                dasm_drvz(instruction);
                break;
            case p2_OPSRC_DRVNZ:
                dasm_drvnz(instruction);
                break;
            case p2_OPSRC_DRVRND:
                dasm_drvrnd(instruction);
                break;
            case p2_OPSRC_DRVNOT:
                dasm_drvnot(instruction);
                break;
            case p2_OPSRC_SPLITB:
                dasm_splitb(instruction);
                break;
            case p2_OPSRC_MERGEB:
                dasm_mergeb(instruction);
                break;
            case p2_OPSRC_SPLITW:
                dasm_splitw(instruction);
                break;
            case p2_OPSRC_MERGEW:
                dasm_mergew(instruction);
                break;
            case p2_OPSRC_SEUSSF:
                dasm_seussf(instruction);
                break;
            case p2_OPSRC_SEUSSR:
                dasm_seussr(instruction);
                break;
            case p2_OPSRC_RGBSQZ:
                dasm_rgbsqz(instruction);
                break;
            case p2_OPSRC_RGBEXP:
                dasm_rgbexp(instruction);
                break;
            case p2_OPSRC_XORO32:
                dasm_xoro32(instruction);
                break;
            case p2_OPSRC_REV:
                dasm_rev(instruction);
                break;
            case p2_OPSRC_RCZR:
                dasm_rczr(instruction);
                break;
            case p2_OPSRC_RCZL:
                dasm_rczl(instruction);
                break;
            case p2_OPSRC_WRC:
                dasm_wrc(instruction);
                break;
            case p2_OPSRC_WRNC:
                dasm_wrnc(instruction);
                break;
            case p2_OPSRC_WRZ:
                dasm_wrz(instruction);
                break;
            case p2_OPSRC_WRNZ_MODCZ:
                if (IR.wc() | IR.wz())
                    dasm_modcz(instruction);
                else
                    dasm_wrnz(instruction);
                break;
            case p2_OPSRC_SETSCP:
                dasm_setscp(instruction);
                break;
            case p2_OPSRC_GETSCP:
                dasm_getscp(instruction);
                break;
            }
            break;

        case p2_JMP_ABS:
            dasm_jmp_abs(instruction);
            break;

        case p2_CALL_ABS:
            dasm_call_abs(instruction);
            break;

        case p2_CALLA_ABS:
            dasm_calla_abs(instruction);
            break;

        case p2_CALLB_ABS:
            dasm_callb_abs(instruction);
            break;

        case p2_CALLD_ABS_PA:
            dasm_calld_abs_pa(instruction);
            break;

        case p2_CALLD_ABS_PB:
            dasm_calld_abs_pb(instruction);
            break;

        case p2_CALLD_ABS_PTRA:
            dasm_calld_abs_ptra(instruction);
            break;

        case p2_CALLD_ABS_PTRB:
            dasm_calld_abs_ptrb(instruction);
            break;

        case p2_LOC_PA:
            dasm_loc_pa(instruction);
            break;

        case p2_LOC_PB:
            dasm_loc_pb(instruction);
            break;

        case p2_LOC_PTRA:
            dasm_loc_ptra(instruction);
            break;

        case p2_LOC_PTRB:
            dasm_loc_ptrb(instruction);
            break;

        case p2_AUGS_00:
        case p2_AUGS_01:
        case p2_AUGS_10:
        case p2_AUGS_11:
            dasm_augs(instruction);
            break;

        case p2_AUGD_00:
        case p2_AUGD_01:
        case p2_AUGD_10:
        case p2_AUGD_11:
            dasm_augd(instruction);
            break;
        }
        instruction->insert(0, cond);
    }

    if (brief)
        *brief = Doc.brief(IR.u.opcode);

    // FIXME: return false for invalid instructions?
    return true;
}

p2_LONG P2Dasm::rd_mem(p2_LONG addr)
{
    return COG->rd_mem(addr);
}

/**
 * @brief Return the memory size in the HUB
 * @return memory size in bytes
 */
p2_LONG P2Dasm::memsize() const
{
    P2Hub* hub = qobject_cast<P2Hub *>(COG->parent());
    if (!hub)
        return MEM_SIZE;
    return hub->memsize();
}

/**
 * @brief Return the lowercase flag
 * @return true if lowercase mode, or false otherwise
 */
bool P2Dasm::lowercase() const
{
    return m_lowercase;
}

/**
 * @brief Set the lowercase flag
 * @param flag lowercase if true, uppercase if false
 */
void P2Dasm::set_lowercase(bool flag)
{
    m_lowercase = flag;
}

/**
 * @brief Print an immediate prefix (#) if im is true
 * @param im flag to test (either WZ or WC)
 * @return QString with immediate token, or empty
 */
QString P2Dasm::format_imm(bool im)
{
    return Token.string(im ? t_IMMEDIATE : t_none, m_lowercase);
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
 * @param instruction pointer to string where to store the result
 * @param inst instruction token (mnemonic)
 */
void P2Dasm::format_inst(QString* instruction, p2_token_e inst)
{
    Q_ASSERT(nullptr != instruction);
    *instruction = Token.string(inst, m_lowercase);
}

/**
 * @brief format string for: EEEE xxxxxxx CZx xxxxxxxxx xxxxxxxxx
 * @param instruction pointer to string where to store the result
 * @param with
 */
void P2Dasm::format_WCZ(QString* instruction, p2_token_e wcz)
{
    Q_ASSERT(nullptr != instruction);

    p2_token_e wc, wz;
    switch (wcz) {
    case t_ANDC:
    case t_ANDZ:
        wc = t_ANDC;
        wz = t_ANDZ;
        break;
    case t_ORC:
    case t_ORZ:
        wc = t_ORC;
        wz = t_ORZ;
        break;
    case t_XORC:
    case t_XORZ:
        wc = t_XORC;
        wz = t_XORZ;
        break;
    case t_WCZ:
    default:
        wc = t_WC;
        wz = t_WZ;
        break;
    }

    if (IR.wc() || IR.wz()) {
        instruction->resize(pad_wcz, QChar::Space);
        if (IR.wc() && IR.wz()) {
            instruction->append(Token.string(wcz, m_lowercase));
        } else if (IR.wc()) {
            instruction->append(Token.string(wc, m_lowercase));
        } else {
            instruction->append(Token.string(wz, m_lowercase));
        }
    }
}

void P2Dasm::format_WC(QString* instruction, p2_token_e wc)
{
    Q_ASSERT(nullptr != instruction);
    if (IR.wc()) {
        instruction->resize(pad_wcz, QChar::Space);
        instruction->append(Token.string(wc, m_lowercase));
    }
}

void P2Dasm::format_WZ(QString* instruction, p2_token_e wz)
{
    Q_ASSERT(nullptr != instruction);
    if (IR.wz()) {
        instruction->resize(pad_wcz, QChar::Space);
        instruction->append(Token.string(wz, m_lowercase));
    }
}

/**
 * @brief format string for: EEEE xxxxxxx CZI DDDDDDDDD SSSSSSSSS
 *
 *  "INSTR  D,{#}S  {WC,WZ,WCZ}"
 *
 * @param instruction pointer to string where to store the result
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 */
void P2Dasm::format_D_IM_S_WCZ(QString* instruction, p2_token_e inst, p2_token_e wcz)
{
    Q_ASSERT(nullptr != instruction);
    if (!IR.im() && IR.dst() == IR.u.op.src) {
        // short form
        *instruction = QString("%1%2")
                       .arg(Token.string(inst, m_lowercase), pad_inst)
                       .arg(format_num(IR.u.op.dst));
    } else {
        *instruction = QString("%1%2,%3%4")
                       .arg(Token.string(inst, m_lowercase), pad_inst)
                       .arg(format_num(IR.u.op.dst))
                       .arg(format_imm(IR.im()))
                       .arg(format_num(IR.u.op.src));
    }
    format_WCZ(instruction, wcz);
}

/**
 * @brief format string for: EEEE xxxxxxx C0I DDDDDDDDD SSSSSSSSS
 *
 *  "INSTR  D,{#}S  {WC}"
 *
 * @param instruction pointer to string where to store the result
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 */
void P2Dasm::format_D_IM_S_WC(QString* instruction, p2_token_e inst)
{
    Q_ASSERT(nullptr != instruction);
    if (!IR.im() && IR.dst() == IR.u.op.src) {
        // short form
        *instruction = QString("%1%2")
                       .arg(Token.string(inst, m_lowercase), pad_inst)
                       .arg(format_num(IR.u.op.dst));
    } else {
        *instruction = QString("%1%2,%3%4")
                       .arg(Token.string(inst, m_lowercase), pad_inst)
                       .arg(format_num(IR.u.op.dst))
                       .arg(format_imm(IR.im()))
                       .arg(format_num(IR.u.op.src));
    }
    format_WC(instruction, t_WC);
}

/**
 * @brief format string for: EEEE xxxxxxx 0ZI DDDDDDDDD SSSSSSSSS
 *
 *  "INSTR  D,{#}S  {WZ}"
 *
 * @param instruction pointer to string where to store the result
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 */
void P2Dasm::format_D_IM_S_WZ(QString* instruction, p2_token_e inst, p2_token_e with)
{
    Q_ASSERT(nullptr != instruction);
    if (!IR.im() && IR.dst() == IR.u.op.src) {
        // short form
        *instruction = QString("%1%2")
                       .arg(Token.string(inst, m_lowercase), pad_inst)
                       .arg(format_num(IR.u.op.dst));
    } else {
        *instruction = QString("%1%2,%3%4")
                       .arg(Token.string(inst, m_lowercase), pad_inst)
                       .arg(format_num(IR.u.op.dst))
                       .arg(format_imm(IR.im()))                // I
                       .arg(format_num(IR.u.op.src));
    }
    format_WZ(instruction, with);
}

/**
 * @brief format string for: EEEE xxxxxxx 0LI DDDDDDDDD SSSSSSSSS
 *
 *  "INSTR  {#}D,{#}S"
 *
 * @param instruction pointer to string where to store the result
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 */
void P2Dasm::format_WZ_D_IM_S(QString* instruction, p2_token_e inst)
{
    Q_ASSERT(nullptr != instruction);
    if (!IR.wz() && !IR.im() && IR.dst() == IR.u.op.src) {
        // short form
        *instruction = QString("%1%2")
                       .arg(Token.string(inst, m_lowercase), pad_inst)
                       .arg(format_num(IR.u.op.dst));
    } else {
        *instruction = QString("%1%2%3,%4%5")
                       .arg(Token.string(inst, m_lowercase), pad_inst)
                       .arg(format_imm(IR.wz()))              // L
                       .arg(format_num(IR.u.op.dst))
                       .arg(format_imm(IR.im()))              // I
                       .arg(format_num(IR.u.op.src));
    }
}

/**
 * @brief format string for: EEEE xxxxxxx 0LI DDDDDDDDD SSSSSSSSS
 *
 *  "INSTR  {#}D,{#}S    {WC}"
 *
 * @param instruction pointer to string where to store the result
 * @param inst instruction token (mnemonic)
 */
void P2Dasm::format_WZ_D_IM_S_WC(QString* instruction, p2_token_e inst)
{
    Q_ASSERT(nullptr != instruction);
    if (!IR.wz() && !IR.im() && IR.dst() == IR.u.op.src) {
        // short form
        *instruction = QString("%1%2")
                       .arg(Token.string(inst, m_lowercase), pad_inst)
                       .arg(format_num(IR.u.op.dst));
    } else {
        *instruction = QString("%1%2%3,%4%5")
                       .arg(Token.string(inst, m_lowercase), pad_inst)
                       .arg(format_imm(IR.wz()))              // L
                       .arg(format_num(IR.u.op.dst))
                       .arg(format_imm(IR.im()))              // I
                       .arg(format_num(IR.u.op.src));
    }
    format_WC(instruction, t_WC);
}

/**
 * @brief format string for: EEEE xxxxxxN NNI DDDDDDDDD SSSSSSSSS
 *
 *  "INSTR  D,{#}S,#N"
 *
 * @param instruction pointer to string where to store the result
 * @param inst instruction token (mnemonic)
 */
void P2Dasm::format_D_IM_S_NNN(QString* instruction, p2_token_e inst, int max)
{
    Q_ASSERT(nullptr != instruction);
    int nnn = static_cast<int>(IR.u.opcode >> 18) & max;
    *instruction = QString("%1%2,%3%4,#%5")
                   .arg(Token.string(inst, m_lowercase), pad_inst)
                   .arg(format_num(IR.u.op.dst))
                   .arg(format_imm(IR.im()))
                   .arg(format_num(IR.u.op.src))
                   .arg(nnn);
}

/**
 * @brief format string for: EEEE xxxxxxx xxI DDDDDDDDD SSSSSSSSS
 *
 *  "INSTR  D,{#}S"
 *
 * @param instruction pointer to string where to store the result
 * @param inst instruction token (mnemonic)
 */
void P2Dasm::format_D_IMM_S(QString* instruction, p2_token_e inst)
{
    Q_ASSERT(nullptr != instruction);
    *instruction = QString("%1%2,%3%4")
                   .arg(Token.string(inst, m_lowercase), pad_inst)
                   .arg(format_num(IR.u.op.dst))
                   .arg(format_imm(IR.im()))
                   .arg(format_num(IR.u.op.src));
}

/**
 * @brief format string for: EEEE xxxxxxx CZ0 DDDDDDDDD xxxxxxxxx
 *
 *  "INSTR  D       {WC,WZ,WCZ}"
 *
 * @param instruction pointer to string where to store the result
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 */
void P2Dasm::format_D_CZ(QString* instruction, p2_token_e inst, p2_token_e with)
{
    Q_ASSERT(nullptr != instruction);
    *instruction = QString("%1%2")
                   .arg(Token.string(inst, m_lowercase), pad_inst)
                   .arg(format_num(IR.u.op.dst));
    format_WCZ(instruction, with);
}

/**
 * @brief format string for: EEEE xxxxxxx CZx xxxxxxxxx xxxxxxxxx
 *
 *  "INSTR          {WC,WZ,WCZ}"
 *
 * @param instruction pointer to string where to store the result
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 */
void P2Dasm::format_CZ(QString* instruction, p2_token_e inst, p2_token_e with)
{
    Q_ASSERT(nullptr != instruction);
    *instruction = QString("%1")
                   .arg(Token.string(inst, m_lowercase), pad_inst);
    format_WCZ(instruction, with);
}


/**
 * @brief format string for: EEEE xxxxxxx CZx 0cccczzzz xxxxxxxxx
 *
 *  "INSTR  cccc,zzzz {WC,WZ,WCZ}"
 *
 * @param instruction pointer to string where to store the result
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 */
void P2Dasm::format_MODCZ_WCZ(QString* instruction, p2_token_e inst, p2_token_e with)
{
    Q_ASSERT(nullptr != instruction);
    const uint cccc = (IR.u.op.dst >> 4) & 15;
    const uint zzzz = (IR.u.op.dst >> 0) & 15;
    if (0 == cccc) {
        // short form 1
        *instruction = QString("%1%2")
                       .arg(Token.string(t_MODZ, m_lowercase), pad_inst)
                       .arg(conditional(zzzz));
    } else if (0 == zzzz) {
        // short form 2
        *instruction = QString("%1%2")
                       .arg(Token.string(t_MODC, m_lowercase), pad_inst)
                       .arg(conditional(cccc));
    } else {
        *instruction = QString("%1%2,%3")
                       .arg(Token.string(inst, m_lowercase), pad_inst)
                       .arg(conditional(cccc))
                       .arg(conditional(zzzz));
    }
    format_WCZ(instruction, with);
}

/**
 * @brief format string for: EEEE xxxxxxx xxx DDDDDDDDD xxxxxxxxx
 *
 *  "INSTR  D"
 *
 * @param instruction pointer to string where to store the result
 * @param inst instruction token (mnemonic)
 */
void P2Dasm::format_D(QString* instruction, p2_token_e inst)
{
    Q_ASSERT(nullptr != instruction);
    *instruction = QString("%1%2")
                   .arg(Token.string(inst, m_lowercase), pad_inst)
                   .arg(format_num(IR.u.op.dst));
}

/**
 * @brief format string for: EEEE xxxxxxx xLx DDDDDDDDD xxxxxxxxx
 *
 *  "INSTR  {#}D"
 *
 * @param instruction pointer to string where to store the result
 * @param inst instruction token (mnemonic)
 */
void P2Dasm::format_WZ_D(QString* instruction, p2_token_e inst)
{
    Q_ASSERT(nullptr != instruction);
    *instruction = QString("%1%2%3")
                   .arg(Token.string(inst, m_lowercase), pad_inst)
                   .arg(format_imm(IR.wz()))
                   .arg(format_num(IR.u.op.dst));
}

/**
 * @brief format string for: EEEE xxxxxxx xxL DDDDDDDDD xxxxxxxxx
 *
 *  "INSTR  {#}D"
 *
 * @param instruction pointer to string where to store the result
 * @param inst instruction token (mnemonic)
 */
void P2Dasm::format_IM_D(QString* instruction, p2_token_e inst)
{
    Q_ASSERT(nullptr != instruction);
    *instruction = QString("%1%2%3")
                   .arg(Token.string(inst, m_lowercase), pad_inst)
                   .arg(format_imm(IR.im()))
                   .arg(format_num(IR.u.op.dst));
}

/**
 * @brief format string for: EEEE xxxxxxx CZL DDDDDDDDD xxxxxxxxx
 *
 *  "INSTR  {#}D    {WC,WZ,WCZ}"
 *
 * @param instruction pointer to string where to store the result
 * @param inst instruction token (mnemonic)
 * @param with token before {C,Z,CZ} i.e. W, AND, OR, XOR
 */
void P2Dasm::format_IM_D_WCZ(QString* instruction, p2_token_e inst, p2_token_e with)
{
    Q_ASSERT(nullptr != instruction);
    *instruction = QString("%1%2%3")
                   .arg(Token.string(inst, m_lowercase), pad_inst)
                   .arg(format_imm(IR.im()))
                   .arg(format_num(IR.u.op.dst));
    format_WCZ(instruction, with);
}

/**
 * @brief format string for: EEEE xxxxxxx CxL DDDDDDDDD xxxxxxxxx
 *
 *  "INSTR  {#}D    {WC}"
 *
 * @param instruction pointer to string where to store the result
 * @param inst instruction token (mnemonic)
 */
void P2Dasm::format_IM_D_WC(QString* instruction, p2_token_e inst)
{
    Q_ASSERT(nullptr != instruction);
    *instruction = QString("%1%2%3")
                   .arg(Token.string(inst, m_lowercase), pad_inst)
                   .arg(format_imm(IR.im()))
                   .arg(format_num(IR.u.op.dst));
    format_WC(instruction, t_WC);
}

/**
 * @brief format string for: EEEE xxxxxxx xxI xxxxxxxxx SSSSSSSSS
 *
 *  "INSTR  {#}S"
 *
 * @param instruction pointer to string where to store the result
 * @param inst instruction token (mnemonic)
 */
void P2Dasm::format_IM_S(QString* instruction, p2_token_e inst)
{
    Q_ASSERT(nullptr != instruction);
    *instruction = QString("%1%2%3")
                   .arg(inst, pad_inst)
                   .arg(format_imm(IR.im()))
                   .arg(format_num(IR.u.op.src));
}

/**
 * @brief format string for: EEEE xxxxxxx CxI xxxxxxxxx SSSSSSSSS
 *
 *  "INSTR  {#}S    {WC}"
 *
 * @param instruction pointer to string where to store the result
 * @param inst instruction token (mnemonic)
 */
void P2Dasm::format_IM_S_WC(QString* instruction, p2_token_e inst)
{
    Q_ASSERT(nullptr != instruction);
    *instruction = QString("%1%2%3")
                   .arg(inst, pad_inst)
                   .arg(format_imm(IR.im()))
                   .arg(format_num(IR.u.op.src));
    format_WC(instruction, t_WC);
}

/**
 * @brief format string for: EEEE xxxxxxx RAA AAAAAAAAA AAAAAAAAA
 *
 *  "INSTR  {PC+}#$AAAAAA"
 *
 * @param instruction pointer to string where to store the result
 * @param inst instruction token (mnemonic)
 * @param dest destination token (PA, PB, PTRA, PTRB)
 */
void P2Dasm::format_PC_ABS(QString* instruction, p2_token_e inst, p2_token_e dest)
{
    Q_ASSERT(nullptr != instruction);
    const p2_LONG aaaa = IR.u.opcode & A20MASK;
    const p2_LONG addr = IR.wc() ? (PC + aaaa) & A20MASK : aaaa;
    *instruction = QString("%1%2%3%4$%5")
                   .arg(Token.string(inst, m_lowercase), pad_inst)
                   .arg(Token.string(dest, m_lowercase))
                   .arg(Token.string(dest != t_none ? t_COMMA : t_none, m_lowercase))
                   .arg(format_imm(IR.im()))
                   .arg(addr, 0, 16);
}

/**
 * @brief format string for: EEEE xxxxxNN NNN NNNNNNNNN NNNNNNNNN
 *
 *  "INSTR  #$NNNNNNNN"
 *
 * @param instruction pointer to string where to store the result
 * @param inst instruction token (mnemonic)
 */
void P2Dasm::format_IMM23(QString* instruction, p2_token_e inst)
{
    Q_ASSERT(nullptr != instruction);
    const p2_LONG nnnn = (IR.u.opcode << 9) & AUG_MASK;
    *instruction = QString("%1%2$%3")
                   .arg(Token.string(inst, m_lowercase), pad_inst)
                   .arg(format_imm(IR.im()))
                   .arg(nnnn << 9, 0, 16, QChar('0'));
}

/**
 * @brief No operation.
 *
 * 0000 0000000 000 000000000 000000000
 *
 * NOP
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_nop(QString* instruction)
{
    format_inst(instruction, t_NOP);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_ror(QString* instruction)
{
    if (0 == IR.u.opcode) {
        dasm_nop(instruction);
    } else {
        format_D_IM_S_WCZ(instruction, t_ROR);
    }
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rol(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_ROL);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_shr(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_SHR);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_shl(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_SHL);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rcr(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_RCR);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rcl(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_RCL);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_sar(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_SAR);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_sal(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_SAL);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_add(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_ADD);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_addx(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_ADDX);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_adds(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_ADDS);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_addsx(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_ADDSX);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_sub(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_SUB);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_subx(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_SUBX);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_subs(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_SUBS);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_subsx(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_SUBSX);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_cmp(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_CMP);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_cmpx(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_CMPX);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_cmps(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_CMPS);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_cmpsx(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_CMPSX);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_cmpr(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_CMPR);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_cmpm(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_CMPM);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_subr(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_SUBR);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_cmpsub(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_CMPSUB);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_fge(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_FGE);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_fle(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_FLE);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_fges(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_FGES);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_fles(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_FLES);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_sumc(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_SUMC);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_sumnc(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_SUMNC);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_sumz(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_SUMZ);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_sumnz(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_SUMNZ);
}

/**
 * @brief Test bit S[4:0] of  D, write to C/Z.
 *
 * EEEE 0100000 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTB   D,{#}S         WC/WZ
 *
 * C/Z =          D[S[4:0]].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_testb_w(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_TESTB);
}

/**
 * @brief Test bit S[4:0] of !D, write to C/Z.
 *
 * EEEE 0100001 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTBN  D,{#}S         WC/WZ
 *
 * C/Z =         !D[S[4:0]].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_testbn_w(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_TESTBN);
}

/**
 * @brief Test bit S[4:0] of  D, AND into C/Z.
 *
 * EEEE 0100010 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTB   D,{#}S     ANDC/ANDZ
 *
 * C/Z = C/Z AND  D[S[4:0]].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_testb_and(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_TESTB, t_AND);
}

/**
 * @brief Test bit S[4:0] of !D, AND into C/Z.
 *
 * EEEE 0100011 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTBN  D,{#}S     ANDC/ANDZ
 *
 * C/Z = C/Z AND !D[S[4:0]].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_testbn_and(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_TESTBN, t_AND);
}

/**
 * @brief Test bit S[4:0] of  D, OR  into C/Z.
 *
 * EEEE 0100100 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTB   D,{#}S       ORC/ORZ
 *
 * C/Z = C/Z OR   D[S[4:0]].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_testb_or(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_TESTB, t_OR);
}

/**
 * @brief Test bit S[4:0] of !D, OR  into C/Z.
 *
 * EEEE 0100101 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTBN  D,{#}S       ORC/ORZ
 *
 * C/Z = C/Z OR  !D[S[4:0]].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_testbn_or(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_TESTBN, t_OR);
}

/**
 * @brief Test bit S[4:0] of  D, XOR into C/Z.
 *
 * EEEE 0100110 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTB   D,{#}S     XORC/XORZ
 *
 * C/Z = C/Z XOR  D[S[4:0]].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_testb_xor(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_TESTB, t_XOR);
}

/**
 * @brief Test bit S[4:0] of !D, XOR into C/Z.
 *
 * EEEE 0100111 CZI DDDDDDDDD SSSSSSSSS
 *
 * TESTBN  D,{#}S     XORC/XORZ
 *
 * C/Z = C/Z XOR !D[S[4:0]].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_testbn_xor(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_TESTBN, t_XOR);
}

/**
 * @brief Bit S[4:0] of D = 0,    C,Z = D[S[4:0]].
 *
 * EEEE 0100000 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITL    D,{#}S         {WCZ}
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_bitl(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_BITL);
}

/**
 * @brief Bit S[4:0] of D = 1,    C,Z = D[S[4:0]].
 *
 * EEEE 0100001 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITH    D,{#}S         {WCZ}
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_bith(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_BITH);
}

/**
 * @brief Bit S[4:0] of D = C,    C,Z = D[S[4:0]].
 *
 * EEEE 0100010 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITC    D,{#}S         {WCZ}
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_bitc(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_BITC);
}

/**
 * @brief Bit S[4:0] of D = !C,   C,Z = D[S[4:0]].
 *
 * EEEE 0100011 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNC   D,{#}S         {WCZ}
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_bitnc(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_BITNC);
}

/**
 * @brief Bit S[4:0] of D = Z,    C,Z = D[S[4:0]].
 *
 * EEEE 0100100 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITZ    D,{#}S         {WCZ}
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_bitz(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_BITZ);
}

/**
 * @brief Bit S[4:0] of D = !Z,   C,Z = D[S[4:0]].
 *
 * EEEE 0100101 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNZ   D,{#}S         {WCZ}
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_bitnz(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_BITNZ);
}

/**
 * @brief Bit S[4:0] of D = RND,  C,Z = D[S[4:0]].
 *
 * EEEE 0100110 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITRND  D,{#}S         {WCZ}
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_bitrnd(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_BITRND);
}

/**
 * @brief Bit S[4:0] of D = !bit, C,Z = D[S[4:0]].
 *
 * EEEE 0100111 CZI DDDDDDDDD SSSSSSSSS
 *
 * BITNOT  D,{#}S         {WCZ}
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_bitnot(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_BITNOT);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_and(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_AND);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_andn(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_ANDN);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_or(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_OR);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_xor(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_XOR);
}

/**
 * @brief Mux C into each D bit that is '1' in S.
 *
 * EEEE 0101100 CZI DDDDDDDDD SSSSSSSSS
 *
 * MUXC    D,{#}S   {WC/WZ/WCZ}
 *
 * D = (!S & D ) | (S & {32{ C}}).
 * C = parity of result.
 * Z = (result == 0).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_muxc(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_MUXC);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_muxnc(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_MUXNC);
}

/**
 * @brief Mux Z into each D bit that is '1' in S.
 *
 * EEEE 0101110 CZI DDDDDDDDD SSSSSSSSS
 *
 * MUXZ    D,{#}S   {WC/WZ/WCZ}
 *
 * D = (!S & D ) | (S & {32{ Z}}).
 * C = parity of result.
 * Z = (result == 0).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_muxz(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_MUXZ);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_muxnz(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_MUXNZ);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_mov(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_MOV);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_not(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_NOT);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_abs(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_ABS);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_neg(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_NEG);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_negc(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_NEGC);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_negnc(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_NEGNC);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_negz(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_NEGZ);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_negnz(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_NEGNZ);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_incmod(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_INCMOD);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_decmod(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_DECMOD);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_zerox(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_ZEROX);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_signx(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_SIGNX);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_encod(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_ENCOD);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_ones(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_ONES);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_test(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_TEST);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_testn(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_TESTN);
}

/**
 * @brief Set S[3:0] into nibble N in D, keeping rest of D same.
 *
 * EEEE 100000N NNI DDDDDDDDD SSSSSSSSS
 *
 * SETNIB  D,{#}S,#N
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setnib(QString* instruction)
{
    format_D_IM_S_NNN(instruction, t_SETNIB);
}

/**
 * @brief Set S[3:0] into nibble established by prior ALTSN instruction.
 *
 * EEEE 1000000 00I 000000000 SSSSSSSSS
 *
 * SETNIB  {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setnib_altsn(QString* instruction)
{
    format_IM_S(instruction, t_SETNIB);
}

/**
 * @brief Get nibble N of S into D.
 *
 * EEEE 100001N NNI DDDDDDDDD SSSSSSSSS
 *
 * GETNIB  D,{#}S,#N
 *
 * D = {28'b0, S.NIBBLE[N]).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_getnib(QString* instruction)
{
    format_D_IM_S_NNN(instruction, t_GETNIB);
}

/**
 * @brief Get nibble established by prior ALTGN instruction into D.
 *
 * EEEE 1000010 000 DDDDDDDDD 000000000
 *
 * GETNIB  D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_getnib_altgn(QString* instruction)
{
    format_IM_S(instruction, t_GETNIB);
}

/**
 * @brief Rotate-left nibble N of S into D.
 *
 * EEEE 100010N NNI DDDDDDDDD SSSSSSSSS
 *
 * ROLNIB  D,{#}S,#N
 *
 * D = {D[27:0], S.NIBBLE[N]).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rolnib(QString* instruction)
{
    format_D_IM_S_NNN(instruction, t_ROLNIB);
}

/**
 * @brief Rotate-left nibble established by prior ALTGN instruction into D.
 *
 * EEEE 1000100 000 DDDDDDDDD 000000000
 *
 * ROLNIB  D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rolnib_altgn(QString* instruction)
{
    format_D(instruction, t_ROLNIB);
}

/**
 * @brief Set S[7:0] into byte N in D, keeping rest of D same.
 *
 * EEEE 1000110 NNI DDDDDDDDD SSSSSSSSS
 *
 * SETBYTE D,{#}S,#N
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setbyte(QString* instruction)
{
    format_D_IM_S_NNN(instruction, t_SETBYTE, 3);
}

/**
 * @brief Set S[7:0] into byte established by prior ALTSB instruction.
 *
 * EEEE 1000110 00I 000000000 SSSSSSSSS
 *
 * SETBYTE {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setbyte_altsb(QString* instruction)
{
    format_IM_S(instruction, t_SETBYTE);
}

/**
 * @brief Get byte N of S into D.
 *
 * EEEE 1000111 NNI DDDDDDDDD SSSSSSSSS
 *
 * GETBYTE D,{#}S,#N
 *
 * D = {24'b0, S.BYTE[N]).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_getbyte(QString* instruction)
{
    format_D_IM_S_NNN(instruction, t_GETBYTE, 3);
}

/**
 * @brief Get byte established by prior ALTGB instruction into D.
 *
 * EEEE 1000111 000 DDDDDDDDD 000000000
 *
 * GETBYTE D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_getbyte_altgb(QString* instruction)
{
    format_D(instruction, t_GETBYTE);
}

/**
 * @brief Rotate-left byte N of S into D.
 *
 * EEEE 1001000 NNI DDDDDDDDD SSSSSSSSS
 *
 * ROLBYTE D,{#}S,#N
 *
 * D = {D[23:0], S.BYTE[N]).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rolbyte(QString* instruction)
{
    format_D_IM_S_NNN(instruction, t_ROLBYTE);
}

/**
 * @brief Rotate-left byte established by prior ALTGB instruction into D.
 *
 * EEEE 1001000 000 DDDDDDDDD 000000000
 *
 * ROLBYTE D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rolbyte_altgb(QString* instruction)
{
    format_D(instruction, t_ROLBYTE);
}

/**
 * @brief Set S[15:0] into word N in D, keeping rest of D same.
 *
 * EEEE 1001001 0NI DDDDDDDDD SSSSSSSSS
 *
 * SETWORD D,{#}S,#N
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setword(QString* instruction)
{
    format_D_IM_S_NNN(instruction, t_SETWORD, 1);
}

/**
 * @brief Set S[15:0] into word established by prior ALTSW instruction.
 *
 * EEEE 1001001 00I 000000000 SSSSSSSSS
 *
 * SETWORD {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setword_altsw(QString* instruction)
{
    format_IM_S(instruction, t_SETWORD);
}

/**
 * @brief Get word N of S into D.
 *
 * EEEE 1001001 1NI DDDDDDDDD SSSSSSSSS
 *
 * GETWORD D,{#}S,#N
 *
 * D = {16'b0, S.WORD[N]).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_getword(QString* instruction)
{
    format_D_IM_S_NNN(instruction, t_GETWORD, 1);
}

/**
 * @brief Get word established by prior ALTGW instruction into D.
 *
 * EEEE 1001001 100 DDDDDDDDD 000000000
 *
 * GETWORD D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_getword_altgw(QString* instruction)
{
    format_IM_S(instruction, t_GETWORD);
}

/**
 * @brief Rotate-left word N of S into D.
 *
 * EEEE 1001010 0NI DDDDDDDDD SSSSSSSSS
 *
 * ROLWORD D,{#}S,#N
 *
 * D = {D[15:0], S.WORD[N]).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rolword(QString* instruction)
{
    format_D_IM_S_NNN(instruction, t_ROLWORD, 1);
}

/**
 * @brief Rotate-left word established by prior ALTGW instruction into D.
 *
 * EEEE 1001010 000 DDDDDDDDD 000000000
 *
 * ROLWORD D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rolword_altgw(QString* instruction)
{
    format_D(instruction, t_ROLWORD);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_altsn(QString* instruction)
{
    format_D_IMM_S(instruction, t_ALTSN);
}

/**
 * @brief Alter subsequent SETNIB instruction.
 *
 * EEEE 1001010 101 DDDDDDDDD 000000000
 *
 * ALTSN   D
 *
 * Next D field = D[11:3], N field = D[2:0].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_altsn_d(QString* instruction)
{
    format_D(instruction, t_ALTSN);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_altgn(QString* instruction)
{
    format_D_IMM_S(instruction, t_ALTGN);
}

/**
 * @brief Alter subsequent GETNIB/ROLNIB instruction.
 *
 * EEEE 1001010 111 DDDDDDDDD 000000000
 *
 * ALTGN   D
 *
 * Next S field = D[11:3], N field = D[2:0].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_altgn_d(QString* instruction)
{
    format_D(instruction, t_ALTGN);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_altsb(QString* instruction)
{
    format_D_IMM_S(instruction, t_ALTSB);
}

/**
 * @brief Alter subsequent SETBYTE instruction.
 *
 * EEEE 1001011 001 DDDDDDDDD 000000000
 *
 * ALTSB   D
 *
 * Next D field = D[10:2], N field = D[1:0].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_altsb_d(QString* instruction)
{
    format_D(instruction, t_ALTSB);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_altgb(QString* instruction)
{
    format_D_IMM_S(instruction, t_ALTGB);
}

/**
 * @brief Alter subsequent GETBYTE/ROLBYTE instruction.
 *
 * EEEE 1001011 011 DDDDDDDDD 000000000
 *
 * ALTGB   D
 *
 * Next S field = D[10:2], N field = D[1:0].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_altgb_d(QString* instruction)
{
    format_D(instruction, t_ALTGB);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_altsw(QString* instruction)
{
    format_D_IMM_S(instruction, t_ALTSW);
}

/**
 * @brief Alter subsequent SETWORD instruction.
 *
 * EEEE 1001011 101 DDDDDDDDD 000000000
 *
 * ALTSW   D
 *
 * Next D field = D[9:1], N field = D[0].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_altsw_d(QString* instruction)
{
    format_D(instruction, t_ALTSW);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_altgw(QString* instruction)
{
    format_D_IMM_S(instruction, t_ALTGW);
}

/**
 * @brief Alter subsequent GETWORD/ROLWORD instruction.
 *
 * EEEE 1001011 111 DDDDDDDDD 000000000
 *
 * ALTGW   D
 *
 * Next S field = D[9:1], N field = D[0].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_altgw_d(QString* instruction)
{
    format_D(instruction, t_ALTGW);
}

/**
 * @brief Alter result register address (normally D field) of next instruction to (D + S) & $1FF.
 *
 * EEEE 1001100 00I DDDDDDDDD SSSSSSSSS
 *
 * ALTR    D,{#}S
 *
 * D += sign-extended S[17:9].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_altr(QString* instruction)
{
    format_D_IMM_S(instruction, t_ALTR);
}

/**
 * @brief Alter result register address (normally D field) of next instruction to D[8:0].
 *
 * EEEE 1001100 001 DDDDDDDDD 000000000
 *
 * ALTR    D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_altr_d(QString* instruction)
{
    format_D(instruction, t_ALTD);
}

/**
 * @brief Alter D field of next instruction to (D + S) & $1FF.
 *
 * EEEE 1001100 01I DDDDDDDDD SSSSSSSSS
 *
 * ALTD    D,{#}S
 *
 * D += sign-extended S[17:9].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_altd(QString* instruction)
{
    format_D_IMM_S(instruction, t_ALTD);
}

/**
 * @brief Alter D field of next instruction to D[8:0].
 *
 * EEEE 1001100 011 DDDDDDDDD 000000000
 *
 * ALTD    D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_altd_d(QString* instruction)
{
    format_D(instruction, t_ALTD);
}

/**
 * @brief Alter S field of next instruction to (D + S) & $1FF.
 *
 * EEEE 1001100 10I DDDDDDDDD SSSSSSSSS
 *
 * ALTS    D,{#}S
 *
 * D += sign-extended S[17:9].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_alts(QString* instruction)
{
    format_D_IMM_S(instruction, t_ALTS);
}

/**
 * @brief Alter S field of next instruction to D[8:0].
 *
 * EEEE 1001100 101 DDDDDDDDD 000000000
 *
 * ALTS    D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_alts_d(QString* instruction)
{
    format_D(instruction, t_ALTS);
}

/**
 * @brief Alter D field of next instruction to (D[13:5] + S) & $1FF.
 *
 * EEEE 1001100 11I DDDDDDDDD SSSSSSSSS
 *
 * ALTB    D,{#}S
 *
 * D += sign-extended S[17:9].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_altb(QString* instruction)
{
    format_D_IMM_S(instruction, t_ALTB);
}

/**
 * @brief Alter D field of next instruction to D[13:5].
 *
 * EEEE 1001100 111 DDDDDDDDD 000000000
 *
 * ALTB    D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_altb_d(QString* instruction)
{
    format_D(instruction, t_ALTB);
}

/**
 * @brief Substitute next instruction's I/R/D/S fields with fields from D, per S.
 *
 * EEEE 1001101 00I DDDDDDDDD SSSSSSSSS
 *
 * ALTI    D,{#}S
 *
 * Modify D per S.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_alti(QString* instruction)
{
    format_D_IMM_S(instruction, t_ALTI);
}

/**
 * @brief Execute D in place of next instruction.
 *
 * EEEE 1001101 001 DDDDDDDDD 101100100
 *
 * ALTI    D
 *
 * D stays same.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_alti_d(QString* instruction)
{
    format_D(instruction, t_ALTI);
}

/**
 * @brief Set R field of D to S[8:0].
 *
 * EEEE 1001101 01I DDDDDDDDD SSSSSSSSS
 *
 * SETR    D,{#}S
 *
 * D = {D[31:28], S[8:0], D[18:0]}.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setr(QString* instruction)
{
    format_D_IMM_S(instruction, t_SETR);
}

/**
 * @brief Set D field of D to S[8:0].
 *
 * EEEE 1001101 10I DDDDDDDDD SSSSSSSSS
 *
 * SETD    D,{#}S
 *
 * D = {D[31:18], S[8:0], D[8:0]}.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setd(QString* instruction)
{
    format_D_IMM_S(instruction, t_SETD);
}

/**
 * @brief Set S field of D to S[8:0].
 *
 * EEEE 1001101 11I DDDDDDDDD SSSSSSSSS
 *
 * SETS    D,{#}S
 *
 * D = {D[31:9], S[8:0]}.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_sets(QString* instruction)
{
    format_D_IMM_S(instruction, t_SETS);
}

/**
 * @brief Decode S[4:0] into D.
 *
 * EEEE 1001110 00I DDDDDDDDD SSSSSSSSS
 *
 * DECOD   D,{#}S
 *
 * D = 1 << S[4:0].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_decod(QString* instruction)
{
    format_D_IMM_S(instruction, t_DECOD);
}

/**
 * @brief Decode D[4:0] into D.
 *
 * EEEE 1001110 000 DDDDDDDDD DDDDDDDDD
 *
 * DECOD   D
 *
 * D = 1 << D[4:0].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_decod_d(QString* instruction)
{
    format_D(instruction, t_DECOD);
}

/**
 * @brief Get LSB-justified bit mask of size (S[4:0] + 1) into D.
 *
 * EEEE 1001110 01I DDDDDDDDD SSSSSSSSS
 *
 * BMASK   D,{#}S
 *
 * D = ($0000_0002 << S[4:0]) - 1.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_bmask(QString* instruction)
{
    format_D_IMM_S(instruction, t_BMASK);
}

/**
 * @brief Get LSB-justified bit mask of size (D[4:0] + 1) into D.
 *
 * EEEE 1001110 010 DDDDDDDDD DDDDDDDDD
 *
 * BMASK   D
 *
 * D = ($0000_0002 << D[4:0]) - 1.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_bmask_d(QString* instruction)
{
    format_D(instruction, t_BMASK);
}

/**
 * @brief Iterate CRC value in D using C and polynomial in S.
 *
 * EEEE 1001110 10I DDDDDDDDD SSSSSSSSS
 *
 * CRCBIT  D,{#}S
 *
 * If (C XOR D[0]) then D = (D >> 1) XOR S, else D = (D >> 1).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_crcbit(QString* instruction)
{
    format_D_IMM_S(instruction, t_CRCBIT);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_crcnib(QString* instruction)
{
    format_D_IMM_S(instruction, t_CRCNIB);
}

/**
 * @brief For each non-zero bit pair in S, copy that bit pair into the corresponding D bits, else leave that D bit pair the same.
 *
 * EEEE 1001111 00I DDDDDDDDD SSSSSSSSS
 *
 * MUXNITS D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_muxnits(QString* instruction)
{
    format_D_IMM_S(instruction, t_MUXNITS);
}

/**
 * @brief For each non-zero nibble in S, copy that nibble into the corresponding D nibble, else leave that D nibble the same.
 *
 * EEEE 1001111 01I DDDDDDDDD SSSSSSSSS
 *
 * MUXNIBS D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_muxnibs(QString* instruction)
{
    format_D_IMM_S(instruction, t_MUXNIBS);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_muxq(QString* instruction)
{
    format_D_IMM_S(instruction, t_MUXQ);
}

/**
 * @brief Move bytes within D, per S.
 *
 * EEEE 1001111 11I DDDDDDDDD SSSSSSSSS
 *
 * MOVBYTS D,{#}S
 *
 * D = {D.BYTE[S[7:6]], D.BYTE[S[5:4]], D.BYTE[S[3:2]], D.BYTE[S[1:0]]}.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_movbyts(QString* instruction)
{
    format_D_IMM_S(instruction, t_MOVBYTS);
}

/**
 * @brief D = unsigned (D[15:0] * S[15:0]).
 *
 * EEEE 1010000 0ZI DDDDDDDDD SSSSSSSSS
 *
 * MUL     D,{#}S          {WZ}
 *
 * Z = (S == 0) | (D == 0).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_mul(QString* instruction)
{
    format_D_IM_S_WZ(instruction, t_MUL);
}

/**
 * @brief D = signed (D[15:0] * S[15:0]).
 *
 * EEEE 1010000 1ZI DDDDDDDDD SSSSSSSSS
 *
 * MULS    D,{#}S          {WZ}
 *
 * Z = (S == 0) | (D == 0).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_muls(QString* instruction)
{
    format_D_IM_S_WZ(instruction, t_MULS);
}

/**
 * @brief Next instruction's S value = unsigned (D[15:0] * S[15:0]) >> 16.
 *
 * EEEE 1010001 0ZI DDDDDDDDD SSSSSSSSS
 *
 * SCA     D,{#}S          {WZ}
 *
 * Z = (result == 0).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_sca(QString* instruction)
{
    format_D_IM_S_WZ(instruction, t_SCA);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_scas(QString* instruction)
{
    format_D_IM_S_WZ(instruction, t_SCAS);
}

/**
 * @brief Add bytes of S into bytes of D, with $FF saturation.
 *
 * EEEE 1010010 00I DDDDDDDDD SSSSSSSSS
 *
 * ADDPIX  D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_addpix(QString* instruction)
{
    format_D_IMM_S(instruction, t_ADDPIX);
}

/**
 * @brief Multiply bytes of S into bytes of D, where $FF = 1.
 *
 * EEEE 1010010 01I DDDDDDDDD SSSSSSSSS
 *
 * MULPIX  D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_mulpix(QString* instruction)
{
    format_D_IMM_S(instruction, t_MULPIX);
}

/**
 * @brief Alpha-blend bytes of S into bytes of D, using SETPIV value.
 *
 * EEEE 1010010 10I DDDDDDDDD SSSSSSSSS
 *
 * BLNPIX  D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_blnpix(QString* instruction)
{
    format_D_IMM_S(instruction, t_BLNPIX);
}

/**
 * @brief Mix bytes of S into bytes of D, using SETPIX and SETPIV values.
 *
 * EEEE 1010010 11I DDDDDDDDD SSSSSSSSS
 *
 * MIXPIX  D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_mixpix(QString* instruction)
{
    format_D_IMM_S(instruction, t_MIXPIX);
}

/**
 * @brief Set CT1 event to trigger on CT = D + S.
 *
 * EEEE 1010011 00I DDDDDDDDD SSSSSSSSS
 *
 * ADDCT1  D,{#}S
 *
 * Adds S into D.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_addct1(QString* instruction)
{
    format_D_IMM_S(instruction, t_ADDCT1);
}

/**
 * @brief Set CT2 event to trigger on CT = D + S.
 *
 * EEEE 1010011 01I DDDDDDDDD SSSSSSSSS
 *
 * ADDCT2  D,{#}S
 *
 * Adds S into D.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_addct2(QString* instruction)
{
    format_D_IMM_S(instruction, t_ADDCT2);
}

/**
 * @brief Set CT3 event to trigger on CT = D + S.
 *
 * EEEE 1010011 10I DDDDDDDDD SSSSSSSSS
 *
 * ADDCT3  D,{#}S
 *
 * Adds S into D.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_addct3(QString* instruction)
{
    format_D_IMM_S(instruction, t_ADDCT3);
}

/**
 * @brief Write only non-$00 bytes in D[31:0] to hub address {#}S/PTRx.
 *
 * EEEE 1010011 11I DDDDDDDDD SSSSSSSSS
 *
 * WMLONG  D,{#}S/P
 *
 * Prior SETQ/SETQ2 invokes cog/LUT block transfer.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_wmlong(QString* instruction)
{
    format_D_IMM_S(instruction, t_WMLONG);
}

/**
 * @brief Read smart pin S[5:0] result "Z" into D, don't acknowledge smart pin ("Q" in RQPIN means "quiet").
 *
 * EEEE 1010100 C0I DDDDDDDDD SSSSSSSSS
 *
 * RQPIN   D,{#}S          {WC}
 *
 * C = modal result.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rqpin(QString* instruction)
{
    format_D_IM_S_WC(instruction, t_RQPIN);
}

/**
 * @brief Read smart pin S[5:0] result "Z" into D, acknowledge smart pin.
 *
 * EEEE 1010100 C1I DDDDDDDDD SSSSSSSSS
 *
 * RDPIN   D,{#}S          {WC}
 *
 * C = modal result.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rdpin(QString* instruction)
{
    format_D_IM_S_WC(instruction, t_RDPIN);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rdlut(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_RDLUT);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rdbyte(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_RDBYTE);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rdword(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_RDWORD);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rdlong(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_RDLONG);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_popa(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_POPA);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_popb(QString* instruction)
{
    format_D_IM_S_WCZ(instruction, t_POPB);
}

/**
 * @brief Call to S** by writing {C, Z, 10'b0, PC[19:0]} to D.
 *
 * EEEE 1011001 CZI DDDDDDDDD SSSSSSSSS
 *
 * CALLD   D,{#}S   {WC/WZ/WCZ}
 *
 * C = S[31], Z = S[30].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_calld(QString* instruction)
{
    if (IR.wc() && IR.wz()) {
        if (IR.dst() == offs_IJMP3 && IR.src() == offs_IRET3) {
            dasm_resi3(instruction);
            return;
        }
        if (IR.dst() == offs_IJMP2 && IR.src() == offs_IRET2) {
            dasm_resi2(instruction);
            return;
        }
        if (IR.dst() == offs_IJMP1 && IR.src() == offs_IRET1) {
            dasm_resi1(instruction);
            return;
        }
        if (IR.dst() == offs_INA && IR.src() == offs_INB) {
            dasm_resi0(instruction);
            return;
        }
        if (IR.dst() == offs_INB && IR.src() == offs_IRET3) {
            dasm_reti3(instruction);
            return;
        }
        if (IR.dst() == offs_INB && IR.src() == offs_IRET2) {
            dasm_reti2(instruction);
            return;
        }
        if (IR.dst() == offs_INB && IR.src() == offs_IRET1) {
            dasm_reti1(instruction);
            return;
        }
        if (IR.dst() == offs_INB && IR.src() == offs_INB) {
            dasm_reti0(instruction);
            return;
        }
    }
    format_D_IM_S_WCZ(instruction, t_CALLD);
}

/**
 * @brief Resume from INT3.
 *
 * EEEE 1011001 110 111110000 111110001
 *
 * RESI3
 *
 * (CALLD $1F0,$1F1 WC,WZ).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_resi3(QString* instruction)
{
    format_inst(instruction, t_RESI3);
}

/**
 * @brief Resume from INT2.
 *
 * EEEE 1011001 110 111110010 111110011
 *
 * RESI2
 *
 * (CALLD $1F2,$1F3 WC,WZ).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_resi2(QString* instruction)
{
    format_inst(instruction, t_RESI2);
}

/**
 * @brief Resume from INT1.
 *
 * EEEE 1011001 110 111110100 111110101
 *
 * RESI1
 *
 * (CALLD $1F4,$1F5 WC,WZ).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_resi1(QString* instruction)
{
    format_inst(instruction, t_RESI1);
}

/**
 * @brief Resume from INT0.
 *
 * EEEE 1011001 110 111111110 111111111
 *
 * RESI0
 *
 * (CALLD $1FE,$1FF WC,WZ).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_resi0(QString* instruction)
{
    format_inst(instruction, t_RESI0);
}

/**
 * @brief Return from INT3.
 *
 * EEEE 1011001 110 111111111 111110001
 *
 * RETI3
 *
 * (CALLD $1FF,$1F1 WC,WZ).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_reti3(QString* instruction)
{
    format_inst(instruction, t_RETI3);
}

/**
 * @brief Return from INT2.
 *
 * EEEE 1011001 110 111111111 111110011
 *
 * RETI2
 *
 * (CALLD $1FF,$1F3 WC,WZ).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_reti2(QString* instruction)
{
    format_inst(instruction, t_RETI2);
}

/**
 * @brief Return from INT1.
 *
 * EEEE 1011001 110 111111111 111110101
 *
 * RETI1
 *
 * (CALLD $1FF,$1F5 WC,WZ).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_reti1(QString* instruction)
{
    format_inst(instruction, t_RETI1);
}

/**
 * @brief Return from INT0.
 *
 * EEEE 1011001 110 111111111 111111111
 *
 * RETI0
 *
 * (CALLD $1FF,$1FF WC,WZ).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_reti0(QString* instruction)
{
    format_inst(instruction, t_RETI0);
}

/**
 * @brief Call to S** by pushing {C, Z, 10'b0, PC[19:0]} onto stack, copy D to PA.
 *
 * EEEE 1011010 0LI DDDDDDDDD SSSSSSSSS
 *
 * CALLPA  {#}D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_callpa(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_CALLPA);
}

/**
 * @brief Call to S** by pushing {C, Z, 10'b0, PC[19:0]} onto stack, copy D to PB.
 *
 * EEEE 1011010 1LI DDDDDDDDD SSSSSSSSS
 *
 * CALLPB  {#}D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_callpb(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_CALLPB);
}

/**
 * @brief Decrement D and jump to S** if result is zero.
 *
 * EEEE 1011011 00I DDDDDDDDD SSSSSSSSS
 *
 * DJZ     D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_djz(QString* instruction)
{
    format_D_IMM_S(instruction, t_DJZ);
}

/**
 * @brief Decrement D and jump to S** if result is not zero.
 *
 * EEEE 1011011 01I DDDDDDDDD SSSSSSSSS
 *
 * DJNZ    D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_djnz(QString* instruction)
{
    format_D_IMM_S(instruction, t_DJNZ);
}

/**
 * @brief Decrement D and jump to S** if result is $FFFF_FFFF.
 *
 * EEEE 1011011 10I DDDDDDDDD SSSSSSSSS
 *
 * DJF     D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_djf(QString* instruction)
{
    format_D_IMM_S(instruction, t_DJF);
}

/**
 * @brief Decrement D and jump to S** if result is not $FFFF_FFFF.
 *
 * EEEE 1011011 11I DDDDDDDDD SSSSSSSSS
 *
 * DJNF    D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_djnf(QString* instruction)
{
    format_D_IMM_S(instruction, t_DJNF);
}

/**
 * @brief Increment D and jump to S** if result is zero.
 *
 * EEEE 1011100 00I DDDDDDDDD SSSSSSSSS
 *
 * IJZ     D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_ijz(QString* instruction)
{
    format_D_IMM_S(instruction, t_IJZ);
}

/**
 * @brief Increment D and jump to S** if result is not zero.
 *
 * EEEE 1011100 01I DDDDDDDDD SSSSSSSSS
 *
 * IJNZ    D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_ijnz(QString* instruction)
{
    format_D_IMM_S(instruction, t_IJNZ);
}

/**
 * @brief Test D and jump to S** if D is zero.
 *
 * EEEE 1011100 10I DDDDDDDDD SSSSSSSSS
 *
 * TJZ     D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_tjz(QString* instruction)
{
    format_D_IMM_S(instruction, t_TJZ);
}

/**
 * @brief Test D and jump to S** if D is not zero.
 *
 * EEEE 1011100 11I DDDDDDDDD SSSSSSSSS
 *
 * TJNZ    D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_tjnz(QString* instruction)
{
    format_D_IMM_S(instruction, t_TJNZ);
}

/**
 * @brief Test D and jump to S** if D is full (D = $FFFF_FFFF).
 *
 * EEEE 1011101 00I DDDDDDDDD SSSSSSSSS
 *
 * TJF     D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_tjf(QString* instruction)
{
    format_D_IMM_S(instruction, t_TJF);
}

/**
 * @brief Test D and jump to S** if D is not full (D != $FFFF_FFFF).
 *
 * EEEE 1011101 01I DDDDDDDDD SSSSSSSSS
 *
 * TJNF    D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_tjnf(QString* instruction)
{
    format_D_IMM_S(instruction, t_TJNF);
}

/**
 * @brief Test D and jump to S** if D is signed (D[31] = 1).
 *
 * EEEE 1011101 10I DDDDDDDDD SSSSSSSSS
 *
 * TJS     D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_tjs(QString* instruction)
{
    format_D_IMM_S(instruction, t_TJS);
}

/**
 * @brief Test D and jump to S** if D is not signed (D[31] = 0).
 *
 * EEEE 1011101 11I DDDDDDDDD SSSSSSSSS
 *
 * TJNS    D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_tjns(QString* instruction)
{
    format_D_IMM_S(instruction, t_TJNS);
}

/**
 * @brief Test D and jump to S** if D overflowed (D[31] != C, C = 'correct sign' from last addition/subtraction).
 *
 * EEEE 1011110 00I DDDDDDDDD SSSSSSSSS
 *
 * TJV     D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_tjv(QString* instruction)
{
    format_D_IMM_S(instruction, t_TJV);
}

/**
 * @brief Jump to S** if INT event flag is set.
 *
 * EEEE 1011110 01I 000000000 SSSSSSSSS
 *
 * JINT    {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jint(QString* instruction)
{
    format_IM_S(instruction, t_JINT);
}

/**
 * @brief Jump to S** if CT1 event flag is set.
 *
 * EEEE 1011110 01I 000000001 SSSSSSSSS
 *
 * JCT1    {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jct1(QString* instruction)
{
    format_IM_S(instruction, t_JCT1);
}

/**
 * @brief Jump to S** if CT2 event flag is set.
 *
 * EEEE 1011110 01I 000000010 SSSSSSSSS
 *
 * JCT2    {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jct2(QString* instruction)
{
    format_IM_S(instruction, t_JCT2);
}

/**
 * @brief Jump to S** if CT3 event flag is set.
 *
 * EEEE 1011110 01I 000000011 SSSSSSSSS
 *
 * JCT3    {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jct3(QString* instruction)
{
    format_IM_S(instruction, t_JCT3);
}

/**
 * @brief Jump to S** if SE1 event flag is set.
 *
 * EEEE 1011110 01I 000000100 SSSSSSSSS
 *
 * JSE1    {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jse1(QString* instruction)
{
    format_IM_S(instruction, t_JSE1);
}

/**
 * @brief Jump to S** if SE2 event flag is set.
 *
 * EEEE 1011110 01I 000000101 SSSSSSSSS
 *
 * JSE2    {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jse2(QString* instruction)
{
    format_IM_S(instruction, t_JSE2);
}

/**
 * @brief Jump to S** if SE3 event flag is set.
 *
 * EEEE 1011110 01I 000000110 SSSSSSSSS
 *
 * JSE3    {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jse3(QString* instruction)
{
    format_IM_S(instruction, t_JSE3);
}

/**
 * @brief Jump to S** if SE4 event flag is set.
 *
 * EEEE 1011110 01I 000000111 SSSSSSSSS
 *
 * JSE4    {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jse4(QString* instruction)
{
    format_IM_S(instruction, t_JSE4);
}

/**
 * @brief Jump to S** if PAT event flag is set.
 *
 * EEEE 1011110 01I 000001000 SSSSSSSSS
 *
 * JPAT    {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jpat(QString* instruction)
{
    format_IM_S(instruction, t_JPAT);
}

/**
 * @brief Jump to S** if FBW event flag is set.
 *
 * EEEE 1011110 01I 000001001 SSSSSSSSS
 *
 * JFBW    {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jfbw(QString* instruction)
{
    format_IM_S(instruction, t_JFBW);
}

/**
 * @brief Jump to S** if XMT event flag is set.
 *
 * EEEE 1011110 01I 000001010 SSSSSSSSS
 *
 * JXMT    {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jxmt(QString* instruction)
{
    format_IM_S(instruction, t_JXMT);
}

/**
 * @brief Jump to S** if XFI event flag is set.
 *
 * EEEE 1011110 01I 000001011 SSSSSSSSS
 *
 * JXFI    {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jxfi(QString* instruction)
{
    format_IM_S(instruction, t_JXFI);
}

/**
 * @brief Jump to S** if XRO event flag is set.
 *
 * EEEE 1011110 01I 000001100 SSSSSSSSS
 *
 * JXRO    {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jxro(QString* instruction)
{
    format_IM_S(instruction, t_JXRO);
}

/**
 * @brief Jump to S** if XRL event flag is set.
 *
 * EEEE 1011110 01I 000001101 SSSSSSSSS
 *
 * JXRL    {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jxrl(QString* instruction)
{
    format_IM_S(instruction, t_JXRL);
}

/**
 * @brief Jump to S** if ATN event flag is set.
 *
 * EEEE 1011110 01I 000001110 SSSSSSSSS
 *
 * JATN    {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jatn(QString* instruction)
{
    format_IM_S(instruction, t_JATN);
}

/**
 * @brief Jump to S** if QMT event flag is set.
 *
 * EEEE 1011110 01I 000001111 SSSSSSSSS
 *
 * JQMT    {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jqmt(QString* instruction)
{
    format_IM_S(instruction, t_JQMT);
}

/**
 * @brief Jump to S** if INT event flag is clear.
 *
 * EEEE 1011110 01I 000010000 SSSSSSSSS
 *
 * JNINT   {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jnint(QString* instruction)
{
    format_IM_S(instruction, t_JNINT);
}

/**
 * @brief Jump to S** if CT1 event flag is clear.
 *
 * EEEE 1011110 01I 000010001 SSSSSSSSS
 *
 * JNCT1   {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jnct1(QString* instruction)
{
    format_IM_S(instruction, t_JNCT1);
}

/**
 * @brief Jump to S** if CT2 event flag is clear.
 *
 * EEEE 1011110 01I 000010010 SSSSSSSSS
 *
 * JNCT2   {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jnct2(QString* instruction)
{
    format_IM_S(instruction, t_JNCT2);
}

/**
 * @brief Jump to S** if CT3 event flag is clear.
 *
 * EEEE 1011110 01I 000010011 SSSSSSSSS
 *
 * JNCT3   {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jnct3(QString* instruction)
{
    format_IM_S(instruction, t_JNCT3);
}

/**
 * @brief Jump to S** if SE1 event flag is clear.
 *
 * EEEE 1011110 01I 000010100 SSSSSSSSS
 *
 * JNSE1   {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jnse1(QString* instruction)
{
    format_IM_S(instruction, t_JNSE1);
}

/**
 * @brief Jump to S** if SE2 event flag is clear.
 *
 * EEEE 1011110 01I 000010101 SSSSSSSSS
 *
 * JNSE2   {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jnse2(QString* instruction)
{
    format_IM_S(instruction, t_JNSE2);
}

/**
 * @brief Jump to S** if SE3 event flag is clear.
 *
 * EEEE 1011110 01I 000010110 SSSSSSSSS
 *
 * JNSE3   {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jnse3(QString* instruction)
{
    format_IM_S(instruction, t_JNSE3);
}

/**
 * @brief Jump to S** if SE4 event flag is clear.
 *
 * EEEE 1011110 01I 000010111 SSSSSSSSS
 *
 * JNSE4   {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jnse4(QString* instruction)
{
    format_IM_S(instruction, t_JNSE4);
}

/**
 * @brief Jump to S** if PAT event flag is clear.
 *
 * EEEE 1011110 01I 000011000 SSSSSSSSS
 *
 * JNPAT   {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jnpat(QString* instruction)
{
    format_IM_S(instruction, t_JNPAT);
}

/**
 * @brief Jump to S** if FBW event flag is clear.
 *
 * EEEE 1011110 01I 000011001 SSSSSSSSS
 *
 * JNFBW   {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jnfbw(QString* instruction)
{
    format_IM_S(instruction, t_JNFBW);
}

/**
 * @brief Jump to S** if XMT event flag is clear.
 *
 * EEEE 1011110 01I 000011010 SSSSSSSSS
 *
 * JNXMT   {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jnxmt(QString* instruction)
{
    format_IM_S(instruction, t_JNXMT);
}

/**
 * @brief Jump to S** if XFI event flag is clear.
 *
 * EEEE 1011110 01I 000011011 SSSSSSSSS
 *
 * JNXFI   {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jnxfi(QString* instruction)
{
    format_IM_S(instruction, t_JNXFI);
}

/**
 * @brief Jump to S** if XRO event flag is clear.
 *
 * EEEE 1011110 01I 000011100 SSSSSSSSS
 *
 * JNXRO   {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jnxro(QString* instruction)
{
    format_IM_S(instruction, t_JNXRO);
}

/**
 * @brief Jump to S** if XRL event flag is clear.
 *
 * EEEE 1011110 01I 000011101 SSSSSSSSS
 *
 * JNXRL   {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jnxrl(QString* instruction)
{
    format_IM_S(instruction, t_JNXRL);
}

/**
 * @brief Jump to S** if ATN event flag is clear.
 *
 * EEEE 1011110 01I 000011110 SSSSSSSSS
 *
 * JNATN   {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jnatn(QString* instruction)
{
    format_IM_S(instruction, t_JNATN);
}

/**
 * @brief Jump to S** if QMT event flag is clear.
 *
 * EEEE 1011110 01I 000011111 SSSSSSSSS
 *
 * JNQMT   {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jnqmt(QString* instruction)
{
    format_IM_S(instruction, t_JNQMT);
}

/**
 * @brief <empty>.
 *
 * EEEE 1011110 1LI DDDDDDDDD SSSSSSSSS
 *
 * <empty> {#}D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_1011110_1(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_empty);
}

/**
 * @brief <empty>.
 *
 * EEEE 1011111 0LI DDDDDDDDD SSSSSSSSS
 *
 * <empty> {#}D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_1011111_0(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_empty);
}

/**
 * @brief Set pin pattern for PAT event.
 *
 * EEEE 1011111 1LI DDDDDDDDD SSSSSSSSS
 *
 * SETPAT  {#}D,{#}S
 *
 * C selects INA/INB, Z selects =/!=, D provides mask value, S provides match value.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setpat(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_SETPAT);
}

/**
 * @brief Write D to mode register of smart pin S[5:0], acknowledge smart pin.
 *
 * EEEE 1100000 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRPIN   {#}D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_wrpin(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_WRPIN);
}

/**
 * @brief Acknowledge smart pin S[5:0].
 *
 * EEEE 1100000 01I 000000001 SSSSSSSSS
 *
 * AKPIN   {#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_akpin(QString* instruction)
{
    format_IM_S(instruction, t_AKPIN);
}

/**
 * @brief Write D to parameter "X" of smart pin S[5:0], acknowledge smart pin.
 *
 * EEEE 1100000 1LI DDDDDDDDD SSSSSSSSS
 *
 * WXPIN   {#}D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_wxpin(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_WXPIN);
}

/**
 * @brief Write D to parameter "Y" of smart pin S[5:0], acknowledge smart pin.
 *
 * EEEE 1100001 0LI DDDDDDDDD SSSSSSSSS
 *
 * WYPIN   {#}D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_wypin(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_WYPIN);
}

/**
 * @brief Write D to LUT address S[8:0].
 *
 * EEEE 1100001 1LI DDDDDDDDD SSSSSSSSS
 *
 * WRLUT   {#}D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_wrlut(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_WRLUT);
}

/**
 * @brief Write byte in D[7:0] to hub address {#}S/PTRx.
 *
 * EEEE 1100010 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRBYTE  {#}D,{#}S/P
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_wrbyte(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_WRBYTE);
}

/**
 * @brief Write word in D[15:0] to hub address {#}S/PTRx.
 *
 * EEEE 1100010 1LI DDDDDDDDD SSSSSSSSS
 *
 * WRWORD  {#}D,{#}S/P
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_wrword(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_WRWORD);
}

/**
 * @brief Write long in D[31:0] to hub address {#}S/PTRx.
 *
 * EEEE 1100011 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRLONG  {#}D,{#}S/P
 *
 * Prior SETQ/SETQ2 invokes cog/LUT block transfer.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_wrlong(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_WRLONG);
}

/**
 * @brief Write long in D[31:0] to hub address PTRA++.
 *
 * EEEE 1100011 0L1 DDDDDDDDD 101100001
 *
 * PUSHA   {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_pusha(QString* instruction)
{
    format_WZ_D(instruction, t_PUSHA);
}

/**
 * @brief Write long in D[31:0] to hub address PTRB++.
 *
 * EEEE 1100011 0L1 DDDDDDDDD 111100001
 *
 * PUSHB   {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_pushb(QString* instruction)
{
    format_WZ_D(instruction, t_PUSHB);
}

/**
 * @brief Begin new fast hub read via FIFO.
 *
 * EEEE 1100011 1LI DDDDDDDDD SSSSSSSSS
 *
 * RDFAST  {#}D,{#}S
 *
 * D[31] = no wait, D[13:0] = block size in 64-byte units (0 = max), S[19:0] = block start address.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rdfast(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_RDFAST);
}

/**
 * @brief Begin new fast hub write via FIFO.
 *
 * EEEE 1100100 0LI DDDDDDDDD SSSSSSSSS
 *
 * WRFAST  {#}D,{#}S
 *
 * D[31] = no wait, D[13:0] = block size in 64-byte units (0 = max), S[19:0] = block start address.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_wrfast(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_WRFAST);
}

/**
 * @brief Set next block for when block wraps.
 *
 * EEEE 1100100 1LI DDDDDDDDD SSSSSSSSS
 *
 * FBLOCK  {#}D,{#}S
 *
 * D[13:0] = block size in 64-byte units (0 = max), S[19:0] = block start address.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_fblock(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_FBLOCK);
}

/**
 * @brief Issue streamer command immediately, zeroing phase.
 *
 * EEEE 1100101 0LI DDDDDDDDD SSSSSSSSS
 *
 * XINIT   {#}D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_xinit(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_XINIT);
}

/**
 * @brief Stop streamer immediately.
 *
 * EEEE 1100101 011 000000000 000000000
 *
 * XSTOP
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_xstop(QString* instruction)
{
    format_inst(instruction, t_XSTOP);
}

/**
 * @brief Buffer new streamer command to be issued on final NCO rollover of current command, zeroing phase.
 *
 * EEEE 1100101 1LI DDDDDDDDD SSSSSSSSS
 *
 * XZERO   {#}D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_xzero(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_XZERO);
}

/**
 * @brief Buffer new streamer command to be issued on final NCO rollover of current command, continuing phase.
 *
 * EEEE 1100110 0LI DDDDDDDDD SSSSSSSSS
 *
 * XCONT   {#}D,{#}S
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_xcont(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_XCONT);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rep(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_REP);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_coginit(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_COGINIT);
}

/**
 * @brief Begin CORDIC unsigned multiplication of D * S.
 *
 * EEEE 1101000 0LI DDDDDDDDD SSSSSSSSS
 *
 * QMUL    {#}D,{#}S
 *
 * GETQX/GETQY retrieves lower/upper product.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_qmul(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_QMUL);
}

/**
 * @brief Begin CORDIC unsigned division of {SETQ value or 32'b0, D} / S.
 *
 * EEEE 1101000 1LI DDDDDDDDD SSSSSSSSS
 *
 * QDIV    {#}D,{#}S
 *
 * GETQX/GETQY retrieves quotient/remainder.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_qdiv(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_QDIV);
}

/**
 * @brief Begin CORDIC unsigned division of {D, SETQ value or 32'b0} / S.
 *
 * EEEE 1101001 0LI DDDDDDDDD SSSSSSSSS
 *
 * QFRAC   {#}D,{#}S
 *
 * GETQX/GETQY retrieves quotient/remainder.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_qfrac(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_QFRAC);
}

/**
 * @brief Begin CORDIC square root of {S, D}.
 *
 * EEEE 1101001 1LI DDDDDDDDD SSSSSSSSS
 *
 * QSQRT   {#}D,{#}S
 *
 * GETQX retrieves root.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_qsqrt(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_QSQRT);
}

/**
 * @brief Begin CORDIC rotation of point (D, SETQ value or 32'b0) by angle S.
 *
 * EEEE 1101010 0LI DDDDDDDDD SSSSSSSSS
 *
 * QROTATE {#}D,{#}S
 *
 * GETQX/GETQY retrieves X/Y.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_qrotate(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_QROTATE);
}

/**
 * @brief Begin CORDIC vectoring of point (D, S).
 *
 * EEEE 1101010 1LI DDDDDDDDD SSSSSSSSS
 *
 * QVECTOR {#}D,{#}S
 *
 * GETQX/GETQY retrieves length/angle.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_qvector(QString* instruction)
{
    format_WZ_D_IM_S(instruction, t_QVECTOR);
}

/**
 * @brief Set hub configuration to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000000000
 *
 * HUBSET  {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_hubset(QString* instruction)
{
    format_IM_D(instruction, t_HUBSET);
}

/**
 * @brief If D is register and no WC, get cog ID (0 to 15) into D.
 *
 * EEEE 1101011 C0L DDDDDDDDD 000000001
 *
 * COGID   {#}D            {WC}
 *
 * If WC, check status of cog D[3:0], C = 1 if on.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_cogid(QString* instruction)
{
    format_IM_D_WC(instruction, t_COGID);
}

/**
 * @brief Stop cog D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000000011
 *
 * COGSTOP {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_cogstop(QString* instruction)
{
    format_IM_D(instruction, t_COGSTOP);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_locknew(QString* instruction)
{
    format_IM_D_WC(instruction, t_LOCKNEW);
}

/**
 * @brief Return LOCK D[3:0] for reallocation.
 *
 * EEEE 1101011 00L DDDDDDDDD 000000101
 *
 * LOCKRET {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_lockret(QString* instruction)
{
    format_IM_D(instruction, t_LOCKRET);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_locktry(QString* instruction)
{
    format_IM_D_WC(instruction, t_LOCKTRY);
}

/**
 * @brief Release LOCK D[3:0].
 *
 * EEEE 1101011 C0L DDDDDDDDD 000000111
 *
 * LOCKREL {#}D            {WC}
 *
 * If D is a register and WC, get current/last cog id of LOCK owner into D and LOCK status into C.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_lockrel(QString* instruction)
{
    format_IM_D_WC(instruction, t_LOCKREL);
}

/**
 * @brief Begin CORDIC number-to-logarithm conversion of D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000001110
 *
 * QLOG    {#}D
 *
 * GETQX retrieves log {5'whole_exponent, 27'fractional_exponent}.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_qlog(QString* instruction)
{
    format_IM_D(instruction, t_QLOG);
}

/**
 * @brief Begin CORDIC logarithm-to-number conversion of D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000001111
 *
 * QEXP    {#}D
 *
 * GETQX retrieves number.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_qexp(QString* instruction)
{
    format_IM_D(instruction, t_QEXP);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rfbyte(QString* instruction)
{
    format_D_CZ(instruction, t_RFBYTE);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rfword(QString* instruction)
{
    format_D_CZ(instruction, t_RFWORD);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rflong(QString* instruction)
{
    format_D_CZ(instruction, t_RFLONG);
}

/**
 * @brief Read zero-extended 1..4-byte value from FIFO into D. Used after RDFAST.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000010011
 *
 * RFVAR   D        {WC/WZ/WCZ}
 *
 * C = 0.
 * Z = (result == 0).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rfvar(QString* instruction)
{
    format_D_CZ(instruction, t_RFVAR);
}

/**
 * @brief Read sign-extended 1..4-byte value from FIFO into D. Used after RDFAST.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000010100
 *
 * RFVARS  D        {WC/WZ/WCZ}
 *
 * C = MSB of value.
 * Z = (result == 0).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rfvars(QString* instruction)
{

    format_D_CZ(instruction, t_RFVARS);
}

/**
 * @brief Write byte in D[7:0] into FIFO. Used after WRFAST.
 *
 * EEEE 1101011 00L DDDDDDDDD 000010101
 *
 * WFBYTE  {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_wfbyte(QString* instruction)
{
    format_IM_D(instruction, t_WFBYTE);
}

/**
 * @brief Write word in D[15:0] into FIFO. Used after WRFAST.
 *
 * EEEE 1101011 00L DDDDDDDDD 000010110
 *
 * WFWORD  {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_wfword(QString* instruction)
{
    format_IM_D(instruction, t_WFWORD);
}

/**
 * @brief Write long in D[31:0] into FIFO. Used after WRFAST.
 *
 * EEEE 1101011 00L DDDDDDDDD 000010111
 *
 * WFLONG  {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_wflong(QString* instruction)
{
    format_IM_D(instruction, t_WFLONG);
}

/**
 * @brief Retrieve CORDIC result X into D.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000011000
 *
 * GETQX   D        {WC/WZ/WCZ}
 *
 * Waits, in case result not ready.
 * C = X[31].
 * Z = (result == 0).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_getqx(QString* instruction)
{
    format_D_CZ(instruction, t_GETQX);
}

/**
 * @brief Retrieve CORDIC result Y into D.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000011001
 *
 * GETQY   D        {WC/WZ/WCZ}
 *
 * Waits, in case result no ready.
 * C = Y[31].
 * Z = (result == 0).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_getqy(QString* instruction)
{
    format_D_CZ(instruction, t_GETQY);
}

/**
 * @brief Get CT into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 000011010
 *
 * GETCT   D
 *
 * CT is the free-running 32-bit system counter that increments on every clock.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_getct(QString* instruction)
{
    format_D(instruction, t_GETCT);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_getrnd(QString* instruction)
{
    format_D_CZ(instruction, t_GETRND);
}

/**
 * @brief Get RND into C/Z.
 *
 * EEEE 1101011 CZ1 000000000 000011011
 *
 * GETRND            WC/WZ/WCZ
 *
 * C = RND[31], Z = RND[30], unique per cog.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_getrnd_cz(QString* instruction)
{
    format_CZ(instruction, t_GETRND);
}

/**
 * @brief DAC3 = D[31:24], DAC2 = D[23:16], DAC1 = D[15:8], DAC0 = D[7:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000011100
 *
 * SETDACS {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setdacs(QString* instruction)
{
    format_IM_D(instruction, t_SETDACS);
}

/**
 * @brief Set streamer NCO frequency to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000011101
 *
 * SETXFRQ {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setxfrq(QString* instruction)
{
    format_IM_D(instruction, t_SETXFRQ);
}

/**
 * @brief Get the streamer's Goertzel X accumulator into D and the Y accumulator into the next instruction's S, clear accumulators.
 *
 * EEEE 1101011 000 DDDDDDDDD 000011110
 *
 * GETXACC D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_getxacc(QString* instruction)
{
    format_D(instruction, t_GETXACC);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_waitx(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_WAITX);
}

/**
 * @brief Set SE1 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100000
 *
 * SETSE1  {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setse1(QString* instruction)
{
    format_IM_D(instruction, t_SETSE1);
}

/**
 * @brief Set SE2 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100001
 *
 * SETSE2  {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setse2(QString* instruction)
{
    format_IM_D(instruction, t_SETSE2);
}

/**
 * @brief Set SE3 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100010
 *
 * SETSE3  {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setse3(QString* instruction)
{
    format_IM_D(instruction, t_SETSE3);
}

/**
 * @brief Set SE4 event configuration to D[8:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100011
 *
 * SETSE4  {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setse4(QString* instruction)
{
    format_IM_D(instruction, t_SETSE4);
}

/**
 * @brief Get INT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000000 000100100
 *
 * POLLINT          {WC/WZ/WCZ}
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_pollint(QString* instruction)
{
    format_CZ(instruction, t_POLLINT);
}

/**
 * @brief Get CT1 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000001 000100100
 *
 * POLLCT1          {WC/WZ/WCZ}
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_pollct1(QString* instruction)
{
    format_CZ(instruction, t_POLLCT1);
}

/**
 * @brief Get CT2 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000010 000100100
 *
 * POLLCT2          {WC/WZ/WCZ}
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_pollct2(QString* instruction)
{
    format_CZ(instruction, t_POLLCT2);
}

/**
 * @brief Get CT3 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000011 000100100
 *
 * POLLCT3          {WC/WZ/WCZ}
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_pollct3(QString* instruction)
{
    format_CZ(instruction, t_POLLCT3);
}

/**
 * @brief Get SE1 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000100 000100100
 *
 * POLLSE1          {WC/WZ/WCZ}
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_pollse1(QString* instruction)
{
    format_CZ(instruction, t_POLLSE1);
}

/**
 * @brief Get SE2 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000101 000100100
 *
 * POLLSE2          {WC/WZ/WCZ}
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_pollse2(QString* instruction)
{
    format_CZ(instruction, t_POLLSE2);
}

/**
 * @brief Get SE3 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000110 000100100
 *
 * POLLSE3          {WC/WZ/WCZ}
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_pollse3(QString* instruction)
{
    format_CZ(instruction, t_POLLSE3);
}

/**
 * @brief Get SE4 event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000000111 000100100
 *
 * POLLSE4          {WC/WZ/WCZ}
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_pollse4(QString* instruction)
{
    format_CZ(instruction, t_POLLSE4);
}

/**
 * @brief Get PAT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001000 000100100
 *
 * POLLPAT          {WC/WZ/WCZ}
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_pollpat(QString* instruction)
{
    format_CZ(instruction, t_POLLPAT);
}

/**
 * @brief Get FBW event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001001 000100100
 *
 * POLLFBW          {WC/WZ/WCZ}
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_pollfbw(QString* instruction)
{
    format_CZ(instruction, t_POLLFBW);
}

/**
 * @brief Get XMT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001010 000100100
 *
 * POLLXMT          {WC/WZ/WCZ}
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_pollxmt(QString* instruction)
{
    format_CZ(instruction, t_POLLXMT);
}

/**
 * @brief Get XFI event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001011 000100100
 *
 * POLLXFI          {WC/WZ/WCZ}
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_pollxfi(QString* instruction)
{
    format_CZ(instruction, t_POLLXFI);
}

/**
 * @brief Get XRO event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001100 000100100
 *
 * POLLXRO          {WC/WZ/WCZ}
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_pollxro(QString* instruction)
{
    format_CZ(instruction, t_POLLXRO);
}

/**
 * @brief Get XRL event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001101 000100100
 *
 * POLLXRL          {WC/WZ/WCZ}
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_pollxrl(QString* instruction)
{
    format_CZ(instruction, t_POLLXRL);
}

/**
 * @brief Get ATN event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001110 000100100
 *
 * POLLATN          {WC/WZ/WCZ}
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_pollatn(QString* instruction)
{
    format_CZ(instruction, t_POLLATN);
}

/**
 * @brief Get QMT event flag into C/Z, then clear it.
 *
 * EEEE 1101011 CZ0 000001111 000100100
 *
 * POLLQMT          {WC/WZ/WCZ}
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_pollqmt(QString* instruction)
{
    format_CZ(instruction, t_POLLQMT);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_waitint(QString* instruction)
{
    format_CZ(instruction, t_WAITINT);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_waitct1(QString* instruction)
{
    format_CZ(instruction, t_WAITCT1);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_waitct2(QString* instruction)
{
    format_CZ(instruction, t_WAITCT2);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_waitct3(QString* instruction)
{
    format_CZ(instruction, t_WAITCT3);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_waitse1(QString* instruction)
{
    format_CZ(instruction, t_WAITSE1);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_waitse2(QString* instruction)
{
    format_CZ(instruction, t_WAITSE2);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_waitse3(QString* instruction)
{
    format_CZ(instruction, t_WAITSE3);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_waitse4(QString* instruction)
{
    format_CZ(instruction, t_WAITSE4);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_waitpat(QString* instruction)
{
    format_CZ(instruction, t_WAITPAT);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_waitfbw(QString* instruction)
{
    format_CZ(instruction, t_WAITFBW);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_waitxmt(QString* instruction)
{
    format_CZ(instruction, t_WAITXMT);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_waitxfi(QString* instruction)
{
    format_CZ(instruction, t_WAITXFI);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_waitxro(QString* instruction)
{
    format_CZ(instruction, t_WAITXRO);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_waitxrl(QString* instruction)
{
    format_CZ(instruction, t_WAITXRL);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_waitatn(QString* instruction)
{
    format_CZ(instruction, t_WAITATN);
}

/**
 * @brief Allow interrupts (default).
 *
 * EEEE 1101011 000 000100000 000100100
 *
 * ALLOWI
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_allowi(QString* instruction)
{
    format_inst(instruction, t_ALLOWI);
}

/**
 * @brief Stall Interrupts.
 *
 * EEEE 1101011 000 000100001 000100100
 *
 * STALLI
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_stalli(QString* instruction)
{
    format_inst(instruction, t_STALLI);
}

/**
 * @brief Trigger INT1, regardless of STALLI mode.
 *
 * EEEE 1101011 000 000100010 000100100
 *
 * TRGINT1
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_trgint1(QString* instruction)
{
    format_inst(instruction, t_TRGINT1);
}

/**
 * @brief Trigger INT2, regardless of STALLI mode.
 *
 * EEEE 1101011 000 000100011 000100100
 *
 * TRGINT2
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_trgint2(QString* instruction)
{
    format_inst(instruction, t_TRGINT2);
}

/**
 * @brief Trigger INT3, regardless of STALLI mode.
 *
 * EEEE 1101011 000 000100100 000100100
 *
 * TRGINT3
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_trgint3(QString* instruction)
{
    format_inst(instruction, t_TRGINT3);
}

/**
 * @brief Cancel INT1.
 *
 * EEEE 1101011 000 000100101 000100100
 *
 * NIXINT1
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_nixint1(QString* instruction)
{
    format_inst(instruction, t_NIXINT1);
}

/**
 * @brief Cancel INT2.
 *
 * EEEE 1101011 000 000100110 000100100
 *
 * NIXINT2
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_nixint2(QString* instruction)
{
    format_inst(instruction, t_NIXINT2);
}

/**
 * @brief Cancel INT3.
 *
 * EEEE 1101011 000 000100111 000100100
 *
 * NIXINT3
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_nixint3(QString* instruction)
{
    format_inst(instruction, t_NIXINT3);
}

/**
 * @brief Set INT1 source to D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100101
 *
 * SETINT1 {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setint1(QString* instruction)
{
    format_IM_D(instruction, t_SETINT1);
}

/**
 * @brief Set INT2 source to D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100110
 *
 * SETINT2 {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setint2(QString* instruction)
{
    format_IM_D(instruction, t_SETINT2);
}

/**
 * @brief Set INT3 source to D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000100111
 *
 * SETINT3 {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setint3(QString* instruction)
{
    format_IM_D(instruction, t_SETINT3);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setq(QString* instruction)
{
    format_IM_D(instruction, t_SETQ);
}

/**
 * @brief Set Q to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000101001
 *
 * SETQ2   {#}D
 *
 * Use before RDLONG/WRLONG/WMLONG to set LUT block transfer.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setq2(QString* instruction)
{
    format_IM_D(instruction, t_SETQ2);
}

/**
 * @brief Push D onto stack.
 *
 * EEEE 1101011 00L DDDDDDDDD 000101010
 *
 * PUSH    {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_push(QString* instruction)
{
    format_IM_D(instruction, t_PUSH);
}

/**
 * @brief Pop stack (K).
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101011
 *
 * POP     D        {WC/WZ/WCZ}
 *
 * C = K[31], Z = K[30], D = K.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_pop(QString* instruction)
{
    format_D_CZ(instruction, t_POP);
}

/**
 * @brief Jump to D.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101100
 *
 * JMP     D        {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jmp(QString* instruction)
{
    format_D_CZ(instruction, t_JMP);
}

/**
 * @brief Call to D by pushing {C, Z, 10'b0, PC[19:0]} onto stack.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101101
 *
 * CALL    D        {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_call(QString* instruction)
{
    format_D_CZ(instruction, t_CALL);
}

/**
 * @brief Return by popping stack (K).
 *
 * EEEE 1101011 CZ1 000000000 000101101
 *
 * RET              {WC/WZ/WCZ}
 *
 * C = K[31], Z = K[30], PC = K[19:0].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_ret(QString* instruction)
{
    format_CZ(instruction, t_RET);
}

/**
 * @brief Call to D by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRA++.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101110
 *
 * CALLA   D        {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_calla(QString* instruction)
{
    format_D_CZ(instruction, t_CALLA);
}

/**
 * @brief Return by reading hub long (L) at --PTRA.
 *
 * EEEE 1101011 CZ1 000000000 000101110
 *
 * RETA             {WC/WZ/WCZ}
 *
 * C = L[31], Z = L[30], PC = L[19:0].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_reta(QString* instruction)
{
    format_CZ(instruction, t_RETA);
}

/**
 * @brief Call to D by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRB++.
 *
 * EEEE 1101011 CZ0 DDDDDDDDD 000101111
 *
 * CALLB   D        {WC/WZ/WCZ}
 *
 * C = D[31], Z = D[30], PC = D[19:0].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_callb(QString* instruction)
{
    format_D_CZ(instruction, t_CALLB);
}

/**
 * @brief Return by reading hub long (L) at --PTRB.
 *
 * EEEE 1101011 CZ1 000000000 000101111
 *
 * RETB             {WC/WZ/WCZ}
 *
 * C = L[31], Z = L[30], PC = L[19:0].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_retb(QString* instruction)
{
    format_CZ(instruction, t_RETB);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jmprel(QString* instruction)
{
    format_IM_D(instruction, t_JMPREL);
}

/**
 * @brief Skip instructions per D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110001
 *
 * SKIP    {#}D
 *
 * Subsequent instructions 0..31 get cancelled for each '1' bit in D[0]..D[31].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_skip(QString* instruction)
{
    format_IM_D(instruction, t_SKIP);
}

/**
 * @brief Skip cog/LUT instructions fast per D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110010
 *
 * SKIPF   {#}D
 *
 * Like SKIP, but instead of cancelling instructions, the PC leaps over them.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_skipf(QString* instruction)
{
    format_IM_D(instruction, t_SKIPF);
}

/**
 * @brief Jump to D[9:0] in cog/LUT and set SKIPF pattern to D[31:10].
 *
 * EEEE 1101011 00L DDDDDDDDD 000110011
 *
 * EXECF   {#}D
 *
 * PC = {10'b0, D[9:0]}.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_execf(QString* instruction)
{
    format_IM_D(instruction, t_EXECF);
}

/**
 * @brief Get current FIFO hub pointer into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 000110100
 *
 * GETPTR  D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_getptr(QString* instruction)
{
    format_D(instruction, t_GETPTR);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_getbrk(QString* instruction)
{
    format_D_CZ(instruction, t_GETBRK);
}

/**
 * @brief If in debug ISR, trigger asynchronous breakpoint in cog D[3:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000110101
 *
 * COGBRK  {#}D
 *
 * Cog D[3:0] must have asynchronous breakpoint enabled.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_cogbrk(QString* instruction)
{
    format_IM_D(instruction, t_COGBRK);
}

/**
 * @brief If in debug ISR, set next break condition to D.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110110
 *
 * BRK     {#}D
 *
 * Else, trigger break if enabled, conditionally write break code to D[7:0].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_brk(QString* instruction)
{
    format_IM_D(instruction, t_BRK);
}

/**
 * @brief If D[0] = 1 then enable LUT sharing, where LUT writes within the adjacent odd/even companion cog are copied to this LUT.
 *
 * EEEE 1101011 00L DDDDDDDDD 000110111
 *
 * SETLUTS {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setluts(QString* instruction)
{
    format_IM_D(instruction, t_SETLUTS);
}

/**
 * @brief Set the colorspace converter "CY" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111000
 *
 * SETCY   {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setcy(QString* instruction)
{
    format_IM_D(instruction, t_SETCY);
}

/**
 * @brief Set the colorspace converter "CI" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111001
 *
 * SETCI   {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setci(QString* instruction)
{
    format_IM_D(instruction, t_SETCI);
}

/**
 * @brief Set the colorspace converter "CQ" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111010
 *
 * SETCQ   {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setcq(QString* instruction)
{
    format_IM_D(instruction, t_SETCQ);
}

/**
 * @brief Set the colorspace converter "CFRQ" parameter to D[31:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111011
 *
 * SETCFRQ {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setcfrq(QString* instruction)
{
    format_IM_D(instruction, t_SETCFRQ);
}

/**
 * @brief Set the colorspace converter "CMOD" parameter to D[6:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111100
 *
 * SETCMOD {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setcmod(QString* instruction)
{
    format_IM_D(instruction, t_SETCMOD);
}

/**
 * @brief Set BLNPIX/MIXPIX blend factor to D[7:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111101
 *
 * SETPIV  {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setpiv(QString* instruction)
{
    format_IM_D(instruction, t_SETPIV);
}

/**
 * @brief Set MIXPIX mode to D[5:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111110
 *
 * SETPIX  {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setpix(QString* instruction)
{
    format_IM_D(instruction, t_SETPIX);
}

/**
 * @brief Strobe "attention" of all cogs whose corresponging bits are high in D[15:0].
 *
 * EEEE 1101011 00L DDDDDDDDD 000111111
 *
 * COGATN  {#}D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_cogatn(QString* instruction)
{
    format_IM_D(instruction, t_COGATN);
}

/**
 * @brief Test  IN bit of pin D[5:0], write to C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000000
 *
 * TESTP   {#}D           WC/WZ
 *
 * C/Z =          IN[D[5:0]].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_testp_w(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_TESTP);
}

/**
 * @brief Test !IN bit of pin D[5:0], write to C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000001
 *
 * TESTPN  {#}D           WC/WZ
 *
 * C/Z =         !IN[D[5:0]].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_testpn_w(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_TESTPN);
}

/**
 * @brief Test  IN bit of pin D[5:0], AND into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000010
 *
 * TESTP   {#}D       ANDC/ANDZ
 *
 * C/Z = C/Z AND  IN[D[5:0]].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_testp_and(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_TESTP, t_AND);
}

/**
 * @brief Test !IN bit of pin D[5:0], AND into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000011
 *
 * TESTPN  {#}D       ANDC/ANDZ
 *
 * C/Z = C/Z AND !IN[D[5:0]].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_testpn_and(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_TESTPN, t_AND);
}

/**
 * @brief Test  IN bit of pin D[5:0], OR  into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000100
 *
 * TESTP   {#}D         ORC/ORZ
 *
 * C/Z = C/Z OR   IN[D[5:0]].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_testp_or(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_TESTP, t_OR);
}

/**
 * @brief Test !IN bit of pin D[5:0], OR  into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000101
 *
 * TESTPN  {#}D         ORC/ORZ
 *
 * C/Z = C/Z OR  !IN[D[5:0]].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_testpn_or(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_TESTPN, t_OR);
}

/**
 * @brief Test  IN bit of pin D[5:0], XOR into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000110
 *
 * TESTP   {#}D       XORC/XORZ
 *
 * C/Z = C/Z XOR  IN[D[5:0]].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_testp_xor(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_TESTP, t_XOR);
}

/**
 * @brief Test !IN bit of pin D[5:0], XOR into C/Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000111
 *
 * TESTPN  {#}D       XORC/XORZ
 *
 * C/Z = C/Z XOR !IN[D[5:0]].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_testpn_xor(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_TESTPN, t_XOR);
}

/**
 * @brief DIR bit of pin D[5:0] = 0.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000000
 *
 * DIRL    {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_dirl(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_DIRL);
}

/**
 * @brief DIR bit of pin D[5:0] = 1.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000001
 *
 * DIRH    {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_dirh(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_DIRH);
}

/**
 * @brief DIR bit of pin D[5:0] = C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000010
 *
 * DIRC    {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_dirc(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_DIRC);
}

/**
 * @brief DIR bit of pin D[5:0] = !C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000011
 *
 * DIRNC   {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_dirnc(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_DIRNC);
}

/**
 * @brief DIR bit of pin D[5:0] = Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000100
 *
 * DIRZ    {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_dirz(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_DIRZ);
}

/**
 * @brief DIR bit of pin D[5:0] = !Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000101
 *
 * DIRNZ   {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_dirnz(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_DIRNZ);
}

/**
 * @brief DIR bit of pin D[5:0] = RND.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000110
 *
 * DIRRND  {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_dirrnd(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_DIRRND);
}

/**
 * @brief DIR bit of pin D[5:0] = !bit.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001000111
 *
 * DIRNOT  {#}D           {WCZ}
 *
 * C,Z = DIR bit.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_dirnot(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_DIRNOT);
}

/**
 * @brief OUT bit of pin D[5:0] = 0.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001000
 *
 * OUTL    {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_outl(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_OUTL);
}

/**
 * @brief OUT bit of pin D[5:0] = 1.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001001
 *
 * OUTH    {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_outh(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_OUTH);
}

/**
 * @brief OUT bit of pin D[5:0] = C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001010
 *
 * OUTC    {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_outc(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_OUTC);
}

/**
 * @brief OUT bit of pin D[5:0] = !C.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001011
 *
 * OUTNC   {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_outnc(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_OUTNC);
}

/**
 * @brief OUT bit of pin D[5:0] = Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001100
 *
 * OUTZ    {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_outz(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_OUTZ);
}

/**
 * @brief OUT bit of pin D[5:0] = !Z.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001101
 *
 * OUTNZ   {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_outnz(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_OUTNZ);
}

/**
 * @brief OUT bit of pin D[5:0] = RND.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001110
 *
 * OUTRND  {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_outrnd(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_OUTRND);
}

/**
 * @brief OUT bit of pin D[5:0] = !bit.
 *
 * EEEE 1101011 CZL DDDDDDDDD 001001111
 *
 * OUTNOT  {#}D           {WCZ}
 *
 * C,Z = OUT bit.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_outnot(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_OUTNOT);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_fltl(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_FLTL);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_flth(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_FLTH);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_fltc(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_FLTC);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_fltnc(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_FLTNC);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_fltz(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_FLTZ);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_fltnz(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_FLTNZ);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_fltrnd(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_FLTRND);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_fltnot(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_FLTNOT);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_drvl(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_DRVL);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_drvh(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_DRVH);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_drvc(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_DRVC);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_drvnc(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_DRVNC);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_drvz(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_DRVZ);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_drvnz(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_DRVNZ);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_drvrnd(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_DRVRND);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_drvnot(QString* instruction)
{
    format_IM_D_WCZ(instruction, t_DRVNOT);
}

/**
 * @brief Split every 4th bit of S into bytes of D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100000
 *
 * SPLITB  D
 *
 * D = {S[31], S[27], S[23], S[19], ... S[12], S[8], S[4], S[0]}.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_splitb(QString* instruction)
{
    format_D(instruction, t_SPLITB);
}

/**
 * @brief Merge bits of bytes in S into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100001
 *
 * MERGEB  D
 *
 * D = {S[31], S[23], S[15], S[7], ... S[24], S[16], S[8], S[0]}.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_mergeb(QString* instruction)
{
    format_D(instruction, t_MERGEB);
}

/**
 * @brief Split bits of S into words of D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100010
 *
 * SPLITW  D
 *
 * D = {S[31], S[29], S[27], S[25], ... S[6], S[4], S[2], S[0]}.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_splitw(QString* instruction)
{
    format_D(instruction, t_SPLITW);
}

/**
 * @brief Merge bits of words in S into D.
 *
 * EEEE 1101011 000 DDDDDDDDD 001100011
 *
 * MERGEW  D
 *
 * D = {S[31], S[15], S[30], S[14], ... S[17], S[1], S[16], S[0]}.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_mergew(QString* instruction)
{
    format_D(instruction, t_MERGEW);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_seussf(QString* instruction)
{
    format_D(instruction, t_SEUSSF);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_seussr(QString* instruction)
{
    format_D(instruction, t_SEUSSR);
}

/**
 * @brief Squeeze 8:8:8 RGB value in S[31:8] into 5:6:5 value in D[15:0].
 *
 * EEEE 1101011 000 DDDDDDDDD 001100110
 *
 * RGBSQZ  D
 *
 * D = {15'b0, S[31:27], S[23:18], S[15:11]}.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rgbsqz(QString* instruction)
{
    format_D(instruction, t_RGBSQZ);
}

/**
 * @brief Expand 5:6:5 RGB value in S[15:0] into 8:8:8 value in D[31:8].
 *
 * EEEE 1101011 000 DDDDDDDDD 001100111
 *
 * RGBEXP  D
 *
 * D = {S[15:11,15:13], S[10:5,10:9], S[4:0,4:2], 8'b0}.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rgbexp(QString* instruction)
{
    format_D(instruction, t_RGBEXP);
}

/**
 * @brief Iterate D with xoroshiro32+ PRNG algorithm and put PRNG result into next instruction's S.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101000
 *
 * XORO32  D
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_xoro32(QString* instruction)
{
    format_D(instruction, t_XORO32);
}

/**
 * @brief Reverse D bits.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101001
 *
 * REV     D
 *
 * D = D[0:31].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rev(QString* instruction)
{
    format_D(instruction, t_REV);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rczr(QString* instruction)
{
    format_D_CZ(instruction, t_RCZR);
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
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_rczl(QString* instruction)
{
    format_D_CZ(instruction, t_RCZL);
}

/**
 * @brief Write 0 or 1 to D, according to  C.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101100
 *
 * WRC     D
 *
 * D = {31'b0,  C).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_wrc(QString* instruction)
{
    format_D(instruction, t_WRC);
}

/**
 * @brief Write 0 or 1 to D, according to !C.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101101
 *
 * WRNC    D
 *
 * D = {31'b0, !C).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_wrnc(QString* instruction)
{
    format_D(instruction, t_WRNC);
}

/**
 * @brief Write 0 or 1 to D, according to  Z.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101110
 *
 * WRZ     D
 *
 * D = {31'b0,  Z).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_wrz(QString* instruction)
{
    format_D(instruction, t_WRZ);
}

/**
 * @brief Write 0 or 1 to D, according to !Z.
 *
 * EEEE 1101011 000 DDDDDDDDD 001101111
 *
 * WRNZ    D
 *
 * D = {31'b0, !Z).
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_wrnz(QString* instruction)
{
    format_D(instruction, t_WRNZ);
}

/**
 * @brief Modify C and Z according to cccc and zzzz.
 *
 * EEEE 1101011 CZ1 0cccczzzz 001101111
 *
 * MODCZ   c,z      {WC/WZ/WCZ}
 *
 * C = cccc[{C,Z}], Z = zzzz[{C,Z}].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_modcz(QString* instruction)
{
    format_MODCZ_WCZ(instruction, t_MODCZ);
}

/**
 * @brief Set scope mode.
 * SETSCP points the scope mux to a set of four pins starting
 * at (D[5:0] AND $3C), with D[6]=1 to enable scope operation.
 *
 * EEEE 1101011 00L DDDDDDDDD 001110000
 *
 * SETSCP  {#}D
 *
 * Pins D[5:2], enable D[6].
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_setscp(QString* instruction)
{
    format_IM_D(instruction, t_SETSCP);
}

/**
 * @brief Get scope values.
 *
 * EEEE 1101011 000 DDDDDDDDD 001110001
 *
 * Any time GETSCP is executed, the lower bytes of those four pins' RDPIN values are returned in D.
 * This feature will mainly be useful on the next silicon, as the FPGAs don't have ADC-capable pins.
 *
 * GETSCP  D
 *
 * C = cccc[{C,Z}], Z = zzzz[{C,Z}].
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_getscp(QString* instruction)
{
    format_D(instruction, t_GETSCP);
}

/**
 * @brief Jump to A.
 *
 * EEEE 1101100 RAA AAAAAAAAA AAAAAAAAA
 *
 * JMP     #A
 *
 * If R = 1, PC += A, else PC = A.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_jmp_abs(QString* instruction)
{
    format_PC_ABS(instruction, t_JMP);
}

/**
 * @brief Call to A by pushing {C, Z, 10'b0, PC[19:0]} onto stack.
 *
 * EEEE 1101101 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALL    #A
 *
 * If R = 1, PC += A, else PC = A.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_call_abs(QString* instruction)
{
    format_PC_ABS(instruction, t_CALL);
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRA++.
 *
 * EEEE 1101110 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLA   #A
 *
 * If R = 1, PC += A, else PC = A.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_calla_abs(QString* instruction)
{
    format_PC_ABS(instruction, t_CALLA);
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to hub long at PTRB++.
 *
 * EEEE 1101111 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLB   #A
 *
 * If R = 1, PC += A, else PC = A.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_callb_abs(QString* instruction)
{
    format_PC_ABS(instruction, t_CALLB);
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 1110000 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PA,#A
 *
 * If R = 1, PC += A, else PC = A.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_calld_abs_pa(QString* instruction)
{
    format_PC_ABS(instruction, t_CALLD, t_PA);
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 1110001 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PB,#A
 *
 * If R = 1, PC += A, else PC = A.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_calld_abs_pb(QString* instruction)
{
    format_PC_ABS(instruction, t_CALLD, t_PB);
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PTRA (per W).
 *
 * EEEE 1110010 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PTRA,#A
 *
 * If R = 1, PC += A, else PC = A.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_calld_abs_ptra(QString* instruction)
{
    format_PC_ABS(instruction, t_CALLD, t_PTRA);
}

/**
 * @brief Call to A by writing {C, Z, 10'b0, PC[19:0]} to PTRB (per W).
 *
 * EEEE 1110011 RAA AAAAAAAAA AAAAAAAAA
 *
 * CALLD   PTRB,#A
 *
 * If R = 1, PC += A, else PC = A.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_calld_abs_ptrb(QString* instruction)
{
    format_PC_ABS(instruction, t_CALLD, t_PTRB);
}

/**
 * @brief Get {12'b0, address[19:0]} into PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 11101WW RAA AAAAAAAAA AAAAAAAAA
 *
 * LOC     PA/PB/PTRA/PTRB,#A
 *
 * If R = 1, address = PC + A, else address = A.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_loc_pa(QString* instruction)
{
    format_PC_ABS(instruction, t_LOC, t_PA);
}

/**
 * @brief Get {12'b0, address[19:0]} into PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 11101WW RAA AAAAAAAAA AAAAAAAAA
 *
 * LOC     PA/PB/PTRA/PTRB,#A
 *
 * If R = 1, address = PC + A, else address = A.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_loc_pb(QString* instruction)
{
    format_PC_ABS(instruction, t_LOC, t_PB);
}

/**
 * @brief Get {12'b0, address[19:0]} into PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 11101WW RAA AAAAAAAAA AAAAAAAAA
 *
 * LOC     PA/PB/PTRA/PTRB,#A
 *
 * If R = 1, address = PC + A, else address = A.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_loc_ptra(QString* instruction)
{
    format_PC_ABS(instruction, t_LOC, t_PTRA);
}

/**
 * @brief Get {12'b0, address[19:0]} into PA/PB/PTRA/PTRB (per W).
 *
 * EEEE 11101WW RAA AAAAAAAAA AAAAAAAAA
 *
 * LOC     PA/PB/PTRA/PTRB,#A
 *
 * If R = 1, address = PC + A, else address = A.
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_loc_ptrb(QString* instruction)
{
    format_PC_ABS(instruction, t_LOC, t_PTRB);
}

/**
 * @brief Queue #N[31:9] to be used as upper 23 bits for next #S occurrence, so that the next 9-bit #S will be augmented to 32 bits.
 *
 * EEEE 11110NN NNN NNNNNNNNN NNNNNNNNN
 *
 * AUGS    #N
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_augs(QString* instruction)
{
    format_IMM23(instruction, t_AUGS);
}

/**
 * @brief Queue #N[31:9] to be used as upper 23 bits for next #D occurrence, so that the next 9-bit #D will be augmented to 32 bits.
 *
 * EEEE 11111NN NNN NNNNNNNNN NNNNNNNNN
 *
 * AUGD    #N
 *
 *
 * @param instruction pointer to string where to store the result
 */
void P2Dasm::dasm_augd(QString* instruction)
{
    format_IMM23(instruction, t_AUGD);
}
