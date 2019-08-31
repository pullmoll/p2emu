/****************************************************************************
 *
 * Propeller2 assembler opcode data class
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
#pragma once
#include "p2defs.h"
#include "p2atom.h"

//! A pair of p2_LONG where the first is the PC, the second the ORGH address
typedef QPair<p2_LONG,p2_LONG> p2_PC_ORGH_t;

/**
 * @brief The class P2Opcode is used to keep the data generated per line together.
 */
class P2Opcode
{
public:
    explicit P2Opcode(const p2_LONG opcode = 0, const p2_PC_ORGH_t& pc_orgh = p2_PC_ORGH_t(0,0));
    P2Opcode(const p2_inst7_e inst7, const p2_PC_ORGH_t& pc_orgh = p2_PC_ORGH_t(0,0));
    P2Opcode(const p2_inst8_e inst8, const p2_PC_ORGH_t& pc_orgh = p2_PC_ORGH_t(0,0));
    P2Opcode(const p2_inst9_e inst9, const p2_PC_ORGH_t& pc_orgh = p2_PC_ORGH_t(0,0));

    //! enumeration of possible targets for the immediate (#) prefix(es)
    enum ImmFlag {
        imm_none,                       //!< don't care about immediate
        imm_to_im,                      //!< if immediate mode, set the immediate (im) flag in IR
        imm_to_wz                       //!< if immediate mode, set the with-zero (wz) flag in IR
    };

    enum Error {
        none,
        dst_augd_none,  // DST constant larger than $1ff but no imediate mode
        dst_augd_im,    // DST constant larger than $1ff but im is not set for L
        dst_augd_wz,    // DST constant larger than $1ff but wz is not set for L
        src_augs_none,  // SRC constant larger than $1ff but no imediate mode
        src_augs_im,    // SRC constant larger than $1ff but im is not set for I
        src_augs_wz,    // SRC constant larger than $1ff but wz is not set for I
    };

    void clear(const p2_LONG opcode = 0, const p2_PC_ORGH_t& pc_orgh = p2_PC_ORGH_t(0,0));

    p2_cond_e cond() const;
    p2_inst7_e inst7() const;
    p2_inst8_e inst8() const;
    p2_inst9_e inst9() const;
    bool wc() const;
    bool wz() const;
    bool im() const;
    p2_LONG dst() const;
    p2_LONG src() const;

    void set_cond(const p2_cond_e cond);
    void set_inst7(const p2_inst7_e inst);
    void set_inst8(const p2_inst8_e inst);
    void set_inst9(const p2_inst9_e inst);
    void set_imm23(const p2_LONG addr);
    void set_wcz(bool on = true);
    void set_wc(bool on = true);
    void set_wz(bool on = true);
    void set_im(bool on = true);
    void set_to(bool on = true);
    bool set_dst(const P2Atom& value);
    bool set_src(const P2Atom& value);

    p2_opcode_u u;                  //!< instruction opcode or assignment value
    p2_PC_ORGH_t PC_ORGH;           //!< QPair of the instruction's PC and ORGH values
    ImmFlag dst_imm_flag;           //!< where to store destination (D) immediate flag
    ImmFlag src_imm_flag;           //!< where to store source (S) immediate flag
    bool imm_set;                   //!< if true, the expression started with a hash (#)
    bool imm_hub;                   //!< if true, the expression started with a double hash (##)
    bool as_IR;                     //!< if true, the p2_opcode_u contains an instruction
    bool as_EQU;                    //!< if true, the p2_opcode_u contains an assignment
    QVariant AUGD;                  //!< optional value in case an AUGD is required
    QVariant AUGS;                  //!< optional value in case an AUGS is required
    P2Atom DATA;                    //!< optional data generated from BYTE, WORD, LONG, FILE, etc.
    P2Atom EQU;                     //!< optional atom from an assignment (=)
    Error error;                    //!< error set when set_dst() or set_src() return false
};

Q_DECLARE_METATYPE(P2Opcode);
