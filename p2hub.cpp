/****************************************************************************
 *
 * P2 emulator Hub implementation
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
#include <QtEndian>
#include <QFile>
#include "p2hub.h"
#include "p2cog.h"

P2Hub::P2Hub(int ncogs, QObject* parent)
    : QObject(parent)
    , XORO128_s0(1)
    , XORO128_s1(0)
    , CNT(0)
    , PIN(0)
    , COGS()
    , nCOGS(ncogs)
    , mCOGS((1u << nCOGS) - 1)
    , pin_mode(64, 0)
    , pin_X(64, 0)
    , pin_Y(64, 0)
    , scope_pin0(0)
    , scope_enable(false)
    , MEM()
{
    Q_ASSERT(ncogs <= 16);
    for (int idx = 0; idx < ncogs; idx++)
        COGS += (new P2Cog(idx, this));
}

/**
 * @brief Execute COGs round robin for %run_cycles
 * @param run_cycles number of cycles to run COGs
 */
void P2Hub::execute(int run_cycles)
{
    while (run_cycles > 0) {
        xoro128();
        int id = static_cast<int>(CNT & mCOGS);
        P2Cog* cog = COGS[id];
        qDebug("%s: run COG #%x (%d cycles left)", __func__, id, run_cycles);
        int cycles = cog ? cog->decode() : 2;
        CNT++;
        run_cycles -= cycles;
    }
}

/**
 * @brief Load a file into HUB memory
 * @param filename name of the file or resource
 * @return true on success, or false on error
 */
bool P2Hub::load(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QByteArray bin = file.read(MEM_SIZE);
    qDebug("%s: file=%s size=0x%06x (%d)", __func__, qPrintable(filename), bin.size(), bin.size());
    p2_BWL* p = reinterpret_cast<p2_BWL *>(bin.data());
    for (int i = 0; i < bin.size(); i += 4, p++) {
        wr_mem(0, static_cast<P2LONG>(i), p->l);
    }

    return true;
}

P2Cog* P2Hub::cog(int id)
{
    return COGS.value(id, nullptr);
}

/**
 * @brief Return a pointer to the HUB memory
 * @return pointer to MEM
 */
P2BYTE* P2Hub::mem()
{
    return MEM.B;
}

/**
 * @brief Return the size of the HUB memory in bytes
 * @return size of MEM
 */
P2LONG P2Hub::memsize() const
{
    return sizeof(MEM);
}

/**
 * @brief Return the current free running counter value
 * @return
 */
quint64 P2Hub::count() const
{
    return CNT;
}

/**
 * @brief Return the number for free HUB slots
 * @return number of free HUB slots
 */
P2LONG P2Hub::hubslots() const
{
    return 0;
}

/**
 * @brief Return current COG index
 * @return
 */
P2LONG P2Hub::cogindex() const
{
    return CNT & mCOGS;
}

/**
 * @brief Return lock state for COG %id
 * @param id COG index
 * @return 1 if locked, 0 if unlocked
 */
int P2Hub::lockstate(int id) const
{
    return (LOCK >> id) & 1;
}

/**
 * @brief Return one pseudo random bit from the RND value
 * @param index bits index (0 … 63)
 * @return p2_LONG pseudo random value
 */
P2LONG P2Hub::random(uint index)
{
    Q_ASSERT(index < 64);
    return static_cast<P2LONG>(RND >> index);
}

/**
 * @brief Rotate a 64 bit value %val left by %shift bits
 * @param val value to rotate
 * @param shift number of bits (0 … 63)
 * @return rotated value
 */
quint64 P2Hub::rotl(quint64 val, uchar shift)
{
    return (val << shift) | (val >> (64 - shift));
}

/**
 * @brief Calculate the next PRNG value
 */
void P2Hub::xoro128()
{
    const quint64 s0 = XORO128_s0;
    const quint64 result = s0 + XORO128_s1;
    const quint64 s1 = XORO128_s1 ^ s0;
    XORO128_s0 = rotl(s0, 55) ^ s1 ^ (s1 << 14); // a, b
    XORO128_s1 = rotl(s1, 36); // c
    RND = result;
}

/**
 * @brief Read byte at address %addr from HUB memory
 * @param addr address
 * @return
 */
P2BYTE P2Hub::rd_BYTE(P2LONG addr) const
{
    if (addr < sizeof(MEM))
        return MEM.B[addr];
    return 0x00;
}

/**
 * @brief Write byte to address %addr in HUB memory
 * @param addr address
 * @param val byte value
 */
void P2Hub::wr_BYTE(P2LONG addr, P2BYTE val)
{
    if (addr < sizeof(MEM))
        MEM.B[addr] = val;
}

/**
 * @brief Read word at address %addr from HUB memory
 * @param addr address (bit 0 is ignored, i.e. word aligned)
 * @return
 */
P2WORD P2Hub::rd_WORD(P2LONG addr) const
{
    if (addr < sizeof(MEM))
        return MEM.W[addr/2];
    return 0x0000;
}

/**
 * @brief Write word to address %addr in HUB memory
 * @param addr address (bit 0 is ignored, i.e. word aligned)
 * @param val byte value
 */
void P2Hub::wr_WORD(P2LONG addr, P2WORD val)
{
    if (addr < sizeof(MEM))
        MEM.W[addr/2] = val;
}

/**
 * @brief Read byte at address %addr from HUB memory
 * @param addr address (bits 0+1 are ignored, i.e. long aligned)
 * @return
 */
P2LONG P2Hub::rd_LONG(P2LONG addr) const
{
    if (addr < sizeof(MEM))
        return MEM.L[addr/4];
    return 0x00000000;
}

/**
 * @brief Write byte to address %addr in HUB memory
 * @param addr address (bits 0+1 are ignored, i.e. long aligned)
 * @param val byte value
 */
void P2Hub::wr_LONG(P2LONG addr, P2LONG val)
{
    if (addr < sizeof(MEM))
        MEM.L[addr/4] = val;
}

/**
 * @brief Read long from COG %cog COG offset $000 <= %offs < $200
 * @param cog COG number (0 … 15)
 * @param offs COG offset (0 … 511)
 * @return long from that address
 */
P2LONG P2Hub::rd_cog(int cog, P2LONG offs) const
{
    if (cog >= nCOGS)
        return 0;
    Q_ASSERT(offs < 0x200);
    return COGS[cog]->rd_cog(offs);
}

/**
 * @brief Write long to COG %cog COG offset $000 <= %offs < $200
 * @param cog COG number (0 … 15)
 * @param offs COG offset (0 … 511)
 * @param val long value to write
 */
void P2Hub::wr_cog(int cog, P2LONG offs, P2LONG val)
{
    if (cog >= nCOGS)
        return;
    Q_ASSERT(offs < 0x200);
    COGS[cog]->wr_cog(offs, val);
}

/**
 * @brief Read long from COG %cog LUT offset $000 <= %offs < $200
 * @param cog COG number (0 … 15)
 * @param offs LUT offset (0 … 511)
 * @return long from that address
 */
P2LONG P2Hub::rd_lut(int cog, P2LONG offs) const
{
    if (cog >= nCOGS)
        return 0;
    Q_ASSERT(offs < 0x200);
    return COGS[cog]->rd_lut(offs);
}

/**
 * @brief Write long to COG %cog LUT offset $000 <= %offs < $200
 * @param cog COG number (0 … 15)
 * @param offs LUT offset (0 … 511)
 * @param val long value to write
 */
void P2Hub::wr_lut(int cog, P2LONG offs, P2LONG val)
{
    if (cog >= nCOGS)
        return;
    Q_ASSERT(offs < 0x200);
    COGS[cog]->wr_lut(offs, val);
}

/**
 * @brief Read long from COG %cog address $00000 <= %addr <= $fffff
 * @param cog COG number (0 … 15)
 * @param offs 20 bit address
 * @return long from that address
 */
P2LONG P2Hub::rd_mem(int cog, P2LONG addr) const
{
    P2LONG data = 0;
    if (cog >= nCOGS)
        return 0;
    Q_ASSERT(addr <= 0xfffffu);
    if (addr < 0x00800) {
        data = rd_cog(cog, addr/4);
    } else if (addr < 0x01000) {
        data = rd_lut(cog, addr/4 - 0x200);
    } else {
        data = MEM.L[addr/4];
    }
    return data;
}

/**
 * @brief Write long to COG %cog for LUT %addr < $200, or HUB memory otherwise
 * @param cog COG number (0 … 15)
 * @param offs 20 bit address
 * @param val long value to write
 */
void P2Hub::wr_mem(int cog, P2LONG addr, P2LONG val)
{
    if (cog >= nCOGS)
        return;
    if (addr < 0x00800) {
        wr_cog(cog, addr/4, val);
    } else if (addr < 0x01000) {
        wr_lut(cog, addr/4 - 0x200, val);
    } else {
        Q_ASSERT(addr < MEM_SIZE);
        MEM.L[addr/4] = val;
    }
}

/**
 * @brief Return the current status of port A
 * @return bits of port A
 */
P2LONG P2Hub::rd_PA()
{
    return static_cast<P2LONG>(PIN);
}

/**
 * @brief Modify the current status of port A
 * @param val new value for port A
 */
void P2Hub::wr_PA(P2LONG val)
{
    PIN = (PIN & Q_UINT64_C(0xffffffff00000000)) | val;
}

/**
 * @brief Return the current status of port B
 * @return bits of port B
 */
P2LONG P2Hub::rd_PB()
{
    return static_cast<P2LONG>(PIN >> 32);
}

/**
 * @brief Modify the current status of port B
 * @param val new value for port B
 */
void P2Hub::wr_PB(quint32 val)
{
    PIN = (PIN & Q_UINT64_C(0x00000000ffffffff)) | static_cast<quint64>(val) << 32;
}

/**
 * @brief Return the current direction of port PA/PB
 * @param port number 0 … 31 for PA, 32 … 63 for PB
 * @return 1 if input, 0 if output
 */
P2LONG P2Hub::rd_DIR(P2LONG port)
{
    Q_ASSERT(port < 64);
    return (DIR >> port) & 1;
}

/**
 * @brief Set the current direction of port PA/PB
 * @param port number 0 … 31 for PA, 32 … 63 for PB
 * @param val 1 for input, 0 for output
 */
void P2Hub::wr_DIR(P2LONG port, P2LONG val)
{
    Q_ASSERT(port < 64);
    const quint64 mask = Q_UINT64_C(1) << port;
    const quint64 bit = static_cast<quint64>(val & 1) << port;
    DIR = (DIR & ~mask) | bit;
}

/**
 * @brief Return the current output of port PA/PB
 * @param port number 0 … 31 for PA, 32 … 63 for PB
 * @return 1 if set, 0 if clear
 */
P2LONG P2Hub::rd_OUT(P2LONG port)
{
    Q_ASSERT(port < 64);
    return (OUT >> port) & 1;
}

/**
 * @brief Set the current output of port PA/PB
 * @param port number 0 … 31 for PA, 32 … 63 for PB
 * @param val 1 for set, 0 for clear
 */
void P2Hub::wr_OUT(P2LONG port, P2LONG val)
{
    Q_ASSERT(port < 64);
    const quint64 mask = Q_UINT64_C(1) << port;
    const quint64 bit = static_cast<quint64>(val & 1) << port;
    OUT = (OUT & ~mask) | bit;
}

/**
 * @brief Read scope MUX lower bytes.
 * @return four lower bytes of scope MUX.
 */
P2LONG P2Hub::rd_SCP()
{
    // TODO: implement scope MUX
    return scope_enable ? MUX : 0xffffffffu;
}

/**
 * @brief Write scope MUX pin0 and enable flag.
 * @param n pin number and enable flag
 */
void P2Hub::wr_SCP(P2LONG n)
{
    // TODO: implement scope MUX
    scope_pin0 = n & ~3u;
    scope_enable = (n >> 6) & 1;
}

/**
 * @brief Return true, if pin %n is high (0 … 31 on PA, 32 … 63 on PB)
 * @param n pin number
 * @return true if high, or false otherwise
 */
bool P2Hub::rd_PIN(P2LONG n)
{
    P2BYTE shift = n & 63;
    return ((PIN >> shift) & 1) != 0;
}
