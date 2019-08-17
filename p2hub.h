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
#include <QObject>
#include <QVector>
#include "p2defs.h"

//! Size of the HUB memory in bytes
#define MEM_SIZE    0x100000

class P2Cog;

class P2Hub : public QObject
{
    Q_OBJECT
public:
    P2Hub(int ncogs, QObject* parent = nullptr);

    void execute(int cycles);
    bool load(const QString& filename);

    P2Cog* cog(int id);
    p2_BYTE* mem();
    p2_LONG memsize() const;

    quint64 count() const;
    p2_LONG hubslots() const;
    p2_LONG cogindex() const;
    int lockstate(int id) const;

    p2_LONG random(uint index = 0);

    p2_BYTE rd_BYTE(p2_LONG addr) const;
    void wr_BYTE(p2_LONG addr, p2_BYTE val);

    p2_WORD rd_WORD(p2_LONG addr) const;
    void wr_WORD(p2_LONG addr, p2_WORD val);

    p2_LONG rd_LONG(p2_LONG addr) const;
    void wr_LONG(p2_LONG addr, p2_LONG val);

    p2_LONG rd_cog(int cog, p2_LONG offs) const;
    void wr_cog(int cog, p2_LONG offs, p2_LONG val);

    p2_LONG rd_lut(int cog, p2_LONG offs) const;
    void wr_lut(int cog, p2_LONG offs, p2_LONG val);

    p2_LONG rd_mem(int cog, p2_LONG addr) const;
    void wr_mem(int cog, p2_LONG addr, p2_LONG val);

    p2_LONG rd_PA();
    void wr_PA(p2_LONG val);

    p2_LONG rd_PB();
    void wr_PB(p2_LONG val);

    p2_LONG rd_DIR(p2_LONG port);
    void wr_DIR(p2_LONG port, p2_LONG val);

    p2_LONG rd_OUT(p2_LONG port);
    void wr_OUT(p2_LONG port, p2_LONG val);

    p2_LONG rd_SCP();
    void wr_SCP(p2_LONG n);

    bool rd_PIN(p2_LONG n);
private:
    quint64 rotl(quint64 val, uchar shift);
    void xoro128();

    quint64 XORO128_s0;     //!< Xoroshiro128 PRNG state[0]
    quint64 XORO128_s1;     //!< Xoroshiro128 PRNG state[1]
    quint64 CNT;            //!< cycle counter
    quint64 RND;            //!< pseudo random value
    quint64 PIN;            //!< 64 pins (0 … 31 on PA, 32 … 63 on PB)
    quint64 DIR;            //!< 64 direction bits (0 … 31 on PA, 32 … 63 on PB)
    quint64 OUT;            //!< 64 output bits (0 … 31 on PA, 32 … 63 on PB)
    p2_LONG MUX;            //!< scope input MUX (TODO: how is it defined?)
    QVector<P2Cog*> COGS;   //!< vector of available COGs
    int nCOGS;              //!< number of available COGs (1 … 16)
    p2_LONG mCOGS;        //!< COG mask
    p2_LONG LOCK;           //!< lock state
    QVector<p2_LONG> pin_mode;
    QVector<p2_LONG> pin_X;
    QVector<p2_LONG> pin_Y;
    p2_LONG scope_pin0;
    p2_LONG scope_enable;
    union {
        p2_BYTE B[MEM_SIZE];
        p2_WORD W[MEM_SIZE/2];
        p2_LONG L[MEM_SIZE/4];
    } MEM;
};
