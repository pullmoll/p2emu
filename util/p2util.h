/****************************************************************************
 *
 * P2 emulator class of static utility and helper functions
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

class P2Util
{
public:
    P2Util();

    static bool skip_space(int& pos, const QString& str);

    static p2_QUAD msb(p2_QUAD val);
    static p2_LONG msb(p2_LONG val);
    static p2_WORD msb(p2_WORD val);
    static p2_BYTE msb(p2_BYTE val);
    static uchar encode(p2_QUAD val);
    static uchar encode(p2_LONG val);
    static uchar encode(p2_WORD val);
    static uchar encode(p2_BYTE val);
    static uchar lzc(p2_QUAD val);
    static uchar lzc(p2_LONG val);
    static uchar lzc(p2_WORD val);
    static uchar lzc(p2_BYTE val);
    static uchar ones(p2_QUAD val);
    static uchar ones(p2_LONG val);
    static uchar ones(p2_WORD val);
    static uchar ones(p2_BYTE val);
    static uchar parity(p2_LONG val);
    static uchar parity(p2_WORD val);
    static uchar parity(p2_BYTE val);
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
