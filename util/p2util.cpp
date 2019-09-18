/****************************************************************************
 *
 * P2 emulator implementation of static utility and helper functions
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
#include "p2util.h"

P2Util Util;

P2Util::P2Util()
{
    Q_ASSERT(0x10000000u == msb(p2_LONG(0x11290023u)));
    Q_ASSERT(0x00020000u == msb(p2_LONG(0x0003f212u)));

    Q_ASSERT(29 == encode(p2_LONG(0x10000000u)));
    Q_ASSERT(32 == encode(p2_LONG(0x80000000u)));

    Q_ASSERT(16 == ones(p2_LONG(0xaaaa5555u)));
    Q_ASSERT(1 == ones(p2_LONG(0x00001000u)));

    Q_ASSERT(10000 == sqrt(10000*10000));
    Q_ASSERT(1000 == sqrt(1000*1000));
    Q_ASSERT(100 == sqrt(100*100));
    Q_ASSERT(10 == sqrt(10*10));
    Q_ASSERT(33121 == sqrt(33121*33121));
}

/**
 * @brief Skip over white-space
 * @param pos position where to start skipping
 * @param str const reference to a string
 * @return true on success, or false if pos >= length of string
 */
bool P2Util::skip_space(int& pos, const QString& str)
{
    if (pos >= str.length())
        return false;
    while (pos < str.length() && str.at(pos).isSpace())
        pos++;
    return true;
}

/**
 * @brief Leave just the most significant 1 bit in value %val
 * @param val value
 * @return position of top most 1 bit
 */
p2_QUAD P2Util::msb(p2_QUAD val)
{
    val |= (val >>  1);
    val |= (val >>  2);
    val |= (val >>  4);
    val |= (val >>  8);
    val |= (val >> 16);
    val |= (val >> 32);
    val &= ~(val >> 1);
    return val;
}

/**
 * @brief Leave just the most significant 1 bit in value %val
 * @param val value
 * @return position of top most 1 bit
 */
p2_LONG P2Util::msb(p2_LONG val)
{
    val |= (val >>  1);
    val |= (val >>  2);
    val |= (val >>  4);
    val |= (val >>  8);
    val |= (val >> 16);
    val &= ~(val >> 1);
    return val;
}

/**
 * @brief Leave just the most significant 1 bit in value %val
 * @param val value
 * @return position of top most 1 bit
 */
p2_WORD P2Util::msb(p2_WORD val)
{
    val |= (val >> 1);
    val |= (val >> 2);
    val |= (val >> 4);
    val |= (val >> 8);
    val &= ~(val >> 1);
    return val;
}

/**
 * @brief Leave just the most significant 1 bit in value %val
 * @param val value
 * @return position of top most 1 bit
 */
p2_BYTE P2Util::msb(p2_BYTE val)
{
    val |= (val >> 1);
    val |= (val >> 2);
    val |= (val >> 4);
    val &= ~(val >> 1);
    return val;
}

/**
 * @brief Find the most significant 1 bit position in value %val
 * @param val value
 * @return position of top most 1 bit
 */
uchar P2Util::encode(p2_QUAD val)
{
    val |= (val >> 1);
    val |= (val >> 2);
    val |= (val >> 4);
    val |= (val >> 8);
    val |= (val >> 16);
    val |= (val >> 32);
    return ones(val);
}

/**
 * @brief Find the most significant 1 bit position in value %val
 * @param val value
 * @return position of top most 1 bit
 */
uchar P2Util::encode(p2_LONG val)
{
    val |= (val >> 1);
    val |= (val >> 2);
    val |= (val >> 4);
    val |= (val >> 8);
    val |= (val >> 16);
    return ones(val);
}

/**
 * @brief Find the most significant 1 bit position in value %val
 * @param val value
 * @return position of top most 1 bit
 */
uchar P2Util::encode(p2_WORD val)
{
    val |= (val >> 1);
    val |= (val >> 2);
    val |= (val >> 4);
    val |= (val >> 8);
    return ones(val);
}

/**
 * @brief Find the most significant 1 bit position in value %val
 * @param val value
 * @return position of top most 1 bit
 */
uchar P2Util::encode(p2_BYTE val)
{
    val |= (val >> 1);
    val |= (val >> 2);
    val |= (val >> 4);
    return ones(val);
}

/**
 * @brief Count the number of leading zero bits in a 64 bit value
 * @param val value to inspect
 * @return the number of leading zero bits
 */
uchar P2Util::lzc(p2_QUAD val)
{
    val |= (val >> 1);
    val |= (val >> 2);
    val |= (val >> 4);
    val |= (val >> 8);
    val |= (val >> 16);
    val |= (val >> 32);
    return 64u - ones(val);
}

/**
 * @brief Count the number of leading zero bits in a 32 bit value
 * @param val value to inspect
 * @return the number of leading zero bits
 */
uchar P2Util::lzc(p2_LONG val)
{
    val |= (val >> 1);
    val |= (val >> 2);
    val |= (val >> 4);
    val |= (val >> 8);
    val |= (val >> 16);
    return 32u - ones(val);
}

/**
 * @brief Count the number of leading zero bits in a 16 bit value
 * @param val value to inspect
 * @return the number of leading zero bits
 */
uchar P2Util::lzc(p2_WORD val)
{
    val |= (val >> 1);
    val |= (val >> 2);
    val |= (val >> 4);
    val |= (val >> 8);
    return 16u - ones(val);
}

/**
 * @brief Count the number of leading zero bits in a 8 bit value
 * @param val value to inspect
 * @return the number of leading zero bits
 */
uchar P2Util::lzc(p2_BYTE val)
{
    val |= (val >> 1);
    val |= (val >> 2);
    val |= (val >> 4);
    return 8u - ones(val);
}

/**
 * @brief Return the number of ones (1) in a 64 bit value
 * @param val 64 bit value
 * @return number of 1 bits
 */
uchar P2Util::ones(p2_QUAD val)
{
    val = val - ((val >> 1) & Q_UINT64_C(0x5555555555555555));
    val = (val & Q_UINT64_C(0x3333333333333333)) + ((val >> 2) & Q_UINT64_C(0x3333333333333333));
    val = (((val + (val >> 4)) & Q_UINT64_C(0x0f0f0f0f0f0f0f0f)) * Q_UINT64_C(0x0101010101010101)) >> 56;
    return static_cast<uchar>(val);
}

/**
 * @brief Return the number of ones (1) in a 32 bit value
 * @param val 32 bit value
 * @return number of 1 bits
 */
uchar P2Util::ones(p2_LONG val)
{
    val = val - ((val >> 1) & 0x55555555);
    val = (val & 0x33333333) + ((val >> 2) & 0x33333333);
    val = (((val + (val >> 4)) & 0x0f0f0f0f) * 0x01010101) >> 24;
    return static_cast<uchar>(val);
}

/**
 * @brief Return the number of ones (1) in a 16 bit value
 * @param val 16 bit value
 * @return number of 1 bits
 */
uchar P2Util::ones(p2_WORD val)
{
    val = val - ((val >> 1) & 0x5555);
    val = (val & 0x3333) + ((val >> 2) & 0x3333);
    val = (((val + (val >> 4)) & 0x0f0f) * 0x0101) >> 12;
    return static_cast<uchar>(val);
}

/**
 * @brief Return the number of ones (1) in a 8 bit value
 * @param val 8 bit value
 * @return number of 1 bits
 */
uchar P2Util::ones(p2_BYTE val)
{
    val = val - ((val >> 1) & 0x55);
    val = (val & 0x33) + ((val >> 2) & 0x33);
    val = (val + (val >> 4)) & 0x0f;
    return val;
}

/**
 * @brief Return the parity of a 32 bit value
 * @param val 32 bit value
 * @return 1 for odd parity, 0 for even parity
 */
uchar P2Util::parity(p2_LONG val)
{
    val ^= val >> 16;
    val ^= val >> 8;
    val ^= val >> 4;
    val &= 15;
    return (0x6996 >> val) & 1u;
}

/**
 * @brief Return the parity of a 16 bit value
 * @param val 16 bit value
 * @return 1 for odd parity, 0 for even parity
 */
uchar P2Util::parity(p2_WORD val)
{
    val ^= val >> 8;
    val ^= val >> 4;
    val &= 15;
    return (0x6996 >> val) & 1;
}

/**
 * @brief Return the parity of a 8 bit value
 * @param val 8 bit value
 * @return 1 for odd parity, 0 for even parity
 */
uchar P2Util::parity(p2_BYTE val)
{
    val ^= val >> 4;
    val &= 15;
    return (0x6996 >> val) & 1;
}

/**
 * @brief Calculate the seuss function for val, forward or reverse
 * @param val value to calulcate seuss for
 * @param forward apply forward if true, otherwise reverse
 * @return seuss value
 */
p2_LONG P2Util::seuss(p2_LONG val, bool forward)
{
    static const uchar bits[32] = {
        11,  5, 18, 24, 27, 19, 20, 30, 28, 26, 21, 25,  3,  8,  7, 23,
        13, 12, 16,  2, 15,  1,  9, 31,  0, 29, 17, 10, 14,  4,  6, 22
    };
    p2_LONG result;

    if (forward) {
        result = 0x354dae51;
        for (int i = 0; i < 32; i++) {
            if (val & (1u << i))
                result ^= (1u << bits[i]);
        }
    } else {
        result = 0xeb55032d;
        for (int i = 0; i < 32; i++) {
            if (val & (1u << bits[i]))
                result ^= (1u << i);
        }
    }
    return result;
}

/**
 * @brief Reverse bits in 64 bit value
 * @param val value to reverse
 * @return bit reversed value
 */
p2_QUAD P2Util::reverse(p2_QUAD val)
{
    val = (((val & Q_UINT64_C(0xaaaaaaaaaaaaaaaa)) >>  1) | ((val & Q_UINT64_C(0x5555555555555555)) <<  1));
    val = (((val & Q_UINT64_C(0xcccccccccccccccc)) >>  2) | ((val & Q_UINT64_C(0x3333333333333333)) <<  2));
    val = (((val & Q_UINT64_C(0xf0f0f0f0f0f0f0f0)) >>  4) | ((val & Q_UINT64_C(0x0f0f0f0f0f0f0f0f)) <<  4));
    val = (((val & Q_UINT64_C(0xff00ff00ff00ff00)) >>  8) | ((val & Q_UINT64_C(0x00ff00ff00ff00ff)) <<  8));
    val = (((val & Q_UINT64_C(0xffff0000ffff0000)) >> 16) | ((val & Q_UINT64_C(0x0000ffff0000ffff)) << 16));
    return (val >> 32) | (val << 32);
}

/**
 * @brief Reverse bits in 32 bit value
 * @param val value to reverse
 * @return bit reversed value
 */
p2_LONG P2Util::reverse(p2_LONG val)
{
    val = (((val & 0xaaaaaaaa) >> 1) | ((val & 0x55555555) << 1));
    val = (((val & 0xcccccccc) >> 2) | ((val & 0x33333333) << 2));
    val = (((val & 0xf0f0f0f0) >> 4) | ((val & 0x0f0f0f0f) << 4));
    val = (((val & 0xff00ff00) >> 8) | ((val & 0x00ff00ff) << 8));
    return (val >> 16) | (val << 16);
}

/**
 * @brief Reverse bits in 16 bit value
 * @param val value to reverse
 * @return bit reversed value
 */
p2_WORD P2Util::reverse(p2_WORD val)
{
    val = static_cast<p2_WORD>(((val & 0xaaaa) >> 1) | ((val & 0x5555) << 1));
    val = static_cast<p2_WORD>(((val & 0xcccc) >> 2) | ((val & 0x3333) << 2));
    val = static_cast<p2_WORD>(((val & 0xf0f0) >> 4) | ((val & 0x0f0f) << 4));
    return static_cast<p2_WORD>((val >> 8) | (val << 8));
}

/**
 * @brief Reverse bits in 8 bit value
 * @param val value to reverse
 * @return bit reversed value
 */
p2_BYTE P2Util::reverse(p2_BYTE val)
{
    val = static_cast<p2_BYTE>(((val & 0xaa) >> 1) | ((val & 0x55) << 1));
    val = static_cast<p2_BYTE>(((val & 0xcc) >> 2) | ((val & 0x33) << 2));
    return static_cast<p2_BYTE>((val >> 4) | (val << 4));
}

/**
 * @brief Reverse %bits in bit value
 * @param val value to reverse
 * @param bits number of bits to reverse
 * @return bit reversed value
 */
p2_QUAD P2Util::reverse(p2_QUAD val, uint bits)
{
    p2_QUAD result = 0;
    switch (bits) {
    case 8:
        return reverse(static_cast<p2_BYTE>(val));
    case 16:
        return reverse(static_cast<p2_WORD>(val));
    case 32:
        return reverse(static_cast<p2_LONG>(val));
    case 64:
        return reverse(static_cast<p2_QUAD>(val));
    }
    for (uint i = 0; i < bits; i++) {
        result = (result << 1) | (val & 1);
        val >>= 1;
    }
    return result;
}

#define	SQRT_SHIFT_BITS     8       // 2, 4, 8, 16
#define	SQRT_SHIFT_HBITS    (SQRT_SHIFT_BITS>>1)
#define	SQRT_SHIFT_MASK     ((Q_UINT64_C(1)<<SQRT_SHIFT_BITS)-1)
/**
 * @brief Find the square root of a 64 bit value
 *
 * Optionally calcuate up to 64 bits of fraction in case
 * the value is not a perfect square.
 *
 * @param val value to find the square root for
 * @param fract_bits number of fraction bits to calcuate
 * @param fraction pointer to a 64 bit value to receive the fraction
 * @return integer part of the square root
 */
p2_QUAD P2Util::sqrt(p2_QUAD val, int fract_bits, p2_QUAD* fraction)
{
    p2_QUAD retval = 0;
    p2_QUAD accu = 0;
    p2_QUAD result = 0;
    p2_QUAD step = 1;
    int shift = 64 - SQRT_SHIFT_BITS;
    p2_LONG d;
    p2_LONG dig2;

    if (fraction)
        *fraction = 0;

    // find the shift that gives a non zero digit pair
    while (0 == (val >> shift))
        shift -= SQRT_SHIFT_BITS;
    dig2 = (val >> shift) & SQRT_SHIFT_MASK;

    for (;;) {
        // shift accu left the number of bits
        accu <<= SQRT_SHIFT_BITS;
        // add 2 digits in the least significant bits
        accu += dig2;

        // subtract consecutive odd numbers 'step' until overflow,
        // counting up the resulting digit in d
        for (d = 0; accu >= step; d++) {
            accu -= step;
            step += 2;
        }

        // shift result left for one digit
        // and add the digit to the result
        result = (result << SQRT_SHIFT_HBITS) + d;

        // next step = 2 * result * 2^hbits + 1
        step = (result << (SQRT_SHIFT_HBITS + 1)) + 1;
        // if shift is 0
        if (0 == shift)
            break;  // break out if no more bits
        // decrease shift to use next digit pair
        shift -= SQRT_SHIFT_BITS;
        dig2 = (val >> shift) & SQRT_SHIFT_MASK;
    }

    retval = result;

    if (accu > 0 && fract_bits > 0) {

        for (int calc = 0; calc < fract_bits; calc += SQRT_SHIFT_HBITS) {
            // shift accu left the number of bits
            accu <<= SQRT_SHIFT_HBITS;

            // subtract consecutive odd numbers 'step' until overflow,
            // counting up the resulting digit in d
            for (d = 0; accu >= step; d++) {
                accu -= step;
                step += 2;
            }

            // left shift result for one digit
            result <<= SQRT_SHIFT_HBITS;
            // add digit to result
            result += d;

            // next step = result * << 2^hbits + 1
            step = (result << (SQRT_SHIFT_HBITS + 1)) + 1;
        }
        *fraction = result;
    }
    return retval;
}


/**
 * @brief XML/HTML escape a string
 * @param src string to escape
 * @return escaped string
 */
const QString P2Util::esc(const QString& src)
{
    QString result = src;
    result.replace(QChar('&'), QStringLiteral("&amp;"));
    result.replace(QChar('<'), QStringLiteral("&lt;"));
    result.replace(QChar('>'), QStringLiteral("&gt;"));
    return result;
}
