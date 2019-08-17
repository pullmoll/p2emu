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
    P2BYTE* mem();
    P2LONG memsize() const;

    quint64 count() const;
    P2LONG hubslots() const;
    P2LONG cogindex() const;
    int lockstate(int id) const;

    P2LONG random(uint index = 0);

    P2BYTE rd_BYTE(P2LONG addr) const;
    void wr_BYTE(P2LONG addr, P2BYTE val);

    P2WORD rd_WORD(P2LONG addr) const;
    void wr_WORD(P2LONG addr, P2WORD val);

    P2LONG rd_LONG(P2LONG addr) const;
    void wr_LONG(P2LONG addr, P2LONG val);

    P2LONG rd_cog(int cog, P2LONG offs) const;
    void wr_cog(int cog, P2LONG offs, P2LONG val);

    P2LONG rd_lut(int cog, P2LONG offs) const;
    void wr_lut(int cog, P2LONG offs, P2LONG val);

    P2LONG rd_mem(int cog, P2LONG addr) const;
    void wr_mem(int cog, P2LONG addr, P2LONG val);

    P2LONG rd_PA();
    void wr_PA(P2LONG val);

    P2LONG rd_PB();
    void wr_PB(P2LONG val);

    P2LONG rd_DIR(P2LONG port);
    void wr_DIR(P2LONG port, P2LONG val);

    P2LONG rd_OUT(P2LONG port);
    void wr_OUT(P2LONG port, P2LONG val);

    P2LONG rd_SCP();
    void wr_SCP(P2LONG n);

    bool rd_PIN(P2LONG n);
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
    P2LONG MUX;            //!< scope input MUX (TODO: how is it defined?)
    QVector<P2Cog*> COGS;   //!< vector of available COGs
    int nCOGS;              //!< number of available COGs (1 … 16)
    P2LONG mCOGS;        //!< COG mask
    P2LONG LOCK;           //!< lock state
    QVector<P2LONG> pin_mode;
    QVector<P2LONG> pin_X;
    QVector<P2LONG> pin_Y;
    P2LONG scope_pin0;
    P2LONG scope_enable;
    union {
        P2BYTE B[MEM_SIZE];
        P2WORD W[MEM_SIZE/2];
        P2LONG L[MEM_SIZE/4];
    } MEM;
};
