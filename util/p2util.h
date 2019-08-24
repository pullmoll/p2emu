#pragma once
#include "p2defs.h"

class P2Util
{
public:
    P2Util();

    static p2_BYTE msb(p2_LONG val);
    static p2_BYTE msb(p2_WORD val);
    static p2_BYTE msb(p2_BYTE val);
    static p2_BYTE lcz(p2_LONG val);
    static p2_BYTE ones(p2_LONG val);
    static p2_BYTE ones(p2_WORD val);
    static p2_BYTE ones(p2_BYTE val);
    static p2_BYTE parity(p2_LONG val);
    static p2_BYTE parity(p2_WORD val);
    static p2_BYTE parity(p2_BYTE val);
    static p2_LONG seuss(p2_LONG val, bool forward = true);
    static p2_LONG reverse(p2_LONG val);
};
