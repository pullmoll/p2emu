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
    explicit P2Opcode(p2_LONG opcode = 0, const p2_PC_ORGH_t& pc_orgh = p2_PC_ORGH_t(0,0))
        : u()
        , PC_ORGH(pc_orgh)
        , as_IR(false)
        , AUGD()
        , AUGS()
        , data()
    {
        clear(opcode);
    }

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

    /**
     * @brief clear the members to their initial state
     * @param opcode optional opcode
     */
    void clear(p2_LONG opcode = 0)
    {
        as_IR = false;
        as_EQU = false;
        AUGD.clear();
        AUGS.clear();
        PC_ORGH = p2_PC_ORGH_t(0,0);
        u.opcode = opcode;
        data.clear();
    }

    void set_inst7(p2_inst7_e inst) { u.op.inst = static_cast<uint>(inst); }
    void set_inst8(p2_inst8_e inst) { u.op8.inst = static_cast<uint>(inst); }
    void set_inst9(p2_inst9_e inst) { u.op9.inst = static_cast<uint>(inst); }

    bool set_dst(const P2Atom& value, imm_to_e imm_to = immediate_none)
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

    bool set_src(const P2Atom& value, imm_to_e imm_to = immediate_none)
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

    p2_opcode_u u;                  //!< instruction opcode or assignment value
    p2_PC_ORGH_t PC_ORGH;           //!< QPair of the instruction's PC and ORGH values
    bool as_IR;                     //!< if true, the p2_opcode_u contains an instruction
    bool as_EQU;                    //!< if true, the p2_opcode_u contains an assignment
    QVariant AUGD;                  //!< optional value in case an AUGD is required
    QVariant AUGS;                  //!< optional value in case an AUGS is required
    P2Atom data;                    //!< optional data generated from BYTE, WORD, LONG, FILE, etc.
    Error error;                    //!< error set when set_dst() or set_src() return false
};

Q_DECLARE_METATYPE(P2Opcode);
