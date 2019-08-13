/****************************************************************************
 *
 * P2 emulator Hub class
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
#include <QtCore>

//! Number of COGs to emulate
#define NUM_COGS    2

//! Size of the HUB memory in bytes
#define MEM_SIZE    1024*1024

class P2Cog;

class P2Hub
{
public:
    P2Hub();

    P2Cog* cog(uint id);
    uchar* mem();
    quint32 memsize() const;

    quint8 rd_BYTE(quint32 addr) const;
    void wr_BYTE(quint32 addr, quint8 val);

    quint16 rd_WORD(quint32 addr) const;
    void wr_WORD(quint32 addr, quint16 val);

    quint32 rd_LONG(quint32 addr) const;
    void wr_LONG(quint32 addr, quint32 val);

    quint32 rd_PA();
    void wr_PA(quint32 val);

    quint32 rd_PB();
    void wr_PB(quint32 val);

private:
    P2Cog* COGS[NUM_COGS];
    quint32 PA;
    quint32 PB;
    union {
        quint8  B[MEM_SIZE];
        quint16 W[MEM_SIZE/2];
        quint32 L[MEM_SIZE/8];
    } MEM;
};
