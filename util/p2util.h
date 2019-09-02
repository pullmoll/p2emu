#pragma once
#include "p2defs.h"

class P2Util
{
public:
    P2Util();

    static bool skip_space(int& pos, const QString& str);

    static p2_QUAD msb(p2_QUAD val);
    static p2_LONG msb(p2_LONG val);
    static p2_WORD msb(p2_WORD val);
    static p2_BYTE msb(p2_BYTE val);
    static uint encode(p2_QUAD val);
    static uint encode(p2_LONG val);
    static uint encode(p2_WORD val);
    static uint encode(p2_BYTE val);
    static uint lzc(p2_QUAD val);
    static uint lzc(p2_LONG val);
    static uint lzc(p2_WORD val);
    static uint lzc(p2_BYTE val);
    static uint ones(p2_QUAD val);
    static uint ones(p2_LONG val);
    static uint ones(p2_WORD val);
    static uint ones(p2_BYTE val);
    static uint parity(p2_LONG val);
    static uint parity(p2_WORD val);
    static uint parity(p2_BYTE val);
    static p2_LONG seuss(p2_LONG val, bool forward = true);
    static p2_QUAD reverse(p2_QUAD val);
    static p2_LONG reverse(p2_LONG val);
    static p2_WORD reverse(p2_WORD val);
    static p2_BYTE reverse(p2_BYTE val);
    static p2_QUAD reverse(p2_QUAD val, uint bits);
    static p2_QUAD sqrt(p2_QUAD val, int fract_bits = 64, p2_QUAD* fraction = nullptr);

    static const QString esc(const QString& src);
};

extern P2Util Util;
