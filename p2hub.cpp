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
#include "p2hub.h"
#include "p2cog.h"

P2Hub::P2Hub()
    : COGS()
    , PA(0)
    , PB(0)
    , MEM()
{
    COGS[ 0] = new P2Cog( 0, this);
#if NUM_COGS > 1
    COGS[ 1] = new P2Cog( 1, this);
#if NUM_COGS > 2
    COGS[ 2] = new P2Cog( 2, this);
    COGS[ 3] = new P2Cog( 3, this);
#if NUM_COGS > 4
    COGS[ 4] = new P2Cog( 4, this);
    COGS[ 5] = new P2Cog( 5, this);
    COGS[ 6] = new P2Cog( 6, this);
    COGS[ 7] = new P2Cog( 7, this);
#if NUM_COGS > 8
    cogs[ 8] = new P2Cog( 8, this);
    cogs[ 9] = new P2Cog( 9, this);
    cogs[10] = new P2Cog(10, this);
    cogs[11] = new P2Cog(11, this);
    cogs[12] = new P2Cog(12, this);
    cogs[13] = new P2Cog(13, this);
    cogs[14] = new P2Cog(14, this);
    cogs[15] = new P2Cog(15, this);
#endif
#endif
#endif
#endif
}

/**
 * @brief Return a pointer to the HUB memory
 * @return pointer to MEM
 */
uchar* P2Hub::mem()
{
    return MEM.B;
}

/**
 * @brief Return the size of the HUB memory in bytes
 * @return size of MEM
 */
quint32 P2Hub::memsize() const
{
    return sizeof(MEM);
}

/**
 * @brief Read byte at address %addr from HUB memory
 * @param addr address
 * @return
 */
quint8 P2Hub::rd_BYTE(quint32 addr) const
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
void P2Hub::wr_BYTE(quint32 addr, quint8 val)
{
    if (addr < sizeof(MEM))
        MEM.B[addr] = val;
}

/**
 * @brief Read word at address %addr from HUB memory
 * @param addr address (bit 0 is ignored, i.e. word aligned)
 * @return
 */
quint16 P2Hub::rd_WORD(quint32 addr) const
{
    addr &= ~1u;
    if (addr*2 < sizeof(MEM))
        return MEM.W[addr/2];
    return 0x0000;
}

/**
 * @brief Write word to address %addr in HUB memory
 * @param addr address (bit 0 is ignored, i.e. word aligned)
 * @param val byte value
 */
void P2Hub::wr_WORD(quint32 addr, quint16 val)
{
    addr &= ~1u;
    if (addr*2 < sizeof(MEM))
        MEM.W[addr/2] = val;
}

/**
 * @brief Read byte at address %addr from HUB memory
 * @param addr address (bits 0+1 are ignored, i.e. long aligned)
 * @return
 */
quint32 P2Hub::rd_LONG(quint32 addr) const
{
    addr &= ~3u;
    if (addr*4 < sizeof(MEM))
        return MEM.L[addr/4];
    return 0x00000000;
}

/**
 * @brief Write byte to address %addr in HUB memory
 * @param addr address (bits 0+1 are ignored, i.e. long aligned)
 * @param val byte value
 */
void P2Hub::wr_LONG(quint32 addr, quint32 val)
{
    addr &= ~3u;
    if (addr*4 < sizeof(MEM))
        MEM.L[addr/4] = val;
}

/**
 * @brief Return the current status of port A
 * @return bits of port A
 */
quint32 P2Hub::rd_PA()
{
    return PA;
}

/**
 * @brief Modify the current status of port A
 * @param val new value for port A
 */
void P2Hub::wr_PA(quint32 val)
{
    PA = val;
}

/**
 * @brief Return the current status of port B
 * @return bits of port B
 */
quint32 P2Hub::rd_PB()
{
    return PB;
}

/**
 * @brief Modify the current status of port B
 * @param val new value for port B
 */
void P2Hub::wr_PB(quint32 val)
{
    PB = val;
}
