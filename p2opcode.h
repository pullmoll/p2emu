#pragma once
#include "p2defs.h"
#include "p2atom.h"

//! enumeration of possible targets for the immediate (#) prefix(es)
typedef enum {
    immediate_none,                     //!< don't care about immediate
    immediate_im,                       //!< set the immediate (im) flag in IR
    immediate_wz,                       //!< set the with-zero (wz) flag in IR
    immediate_wc                        //!< set the with-carry (wc) flag in IR
}   imm_to_e;

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

    enum Error {
        none,
        dst_augd_none,  // DST constant larger than $1ff but no imediate mode
        dst_augd_im,    // DST constant larger than $1ff but im is not set for L
        dst_augd_wz,    // DST constant larger than $1ff but wz is not set for L
        dst_augd_wc,    // DST constant larger than $1ff but wc is not set for L
        src_augs_none,  // SRC constant larger than $1ff but no imediate mode
        src_augs_im,    // SRC constant larger than $1ff but im is not set for I
        src_augs_wz,    // SRC constant larger than $1ff but wz is not set for I
        src_augs_wc,    // SRC constant larger than $1ff but wc is not set for I
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
    bool set_dst(const P2Atom& value, imm_to_e imm_to = immediate_none);
    bool set_src(const P2Atom& value, imm_to_e imm_to = immediate_none);

    p2_opcode_u u;                  //!< instruction opcode or assignment value
    p2_PC_ORGH_t PC_ORGH;           //!< QPair of the instruction's PC and ORGH values
    bool as_IR;                     //!< if true, the p2_opcode_u contains an instruction
    bool as_EQU;                    //!< if true, the p2_opcode_u contains an assignment
    QVariant AUGD;                  //!< optional value in case an AUGD is required
    QVariant AUGS;                  //!< optional value in case an AUGS is required
    P2Atom DATA;                    //!< optional data generated from BYTE, WORD, LONG, FILE, etc.
    Error error;                    //!< error set when set_dst() or set_src() return false
};

Q_DECLARE_METATYPE(P2Opcode);
