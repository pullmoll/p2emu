#include "p2util.h"

P2Util::P2Util()
{
}

/**
 * @brief Find the most significant 1 bit in value %val
 * @param val value
 * @return position of top most 1 bit
 */
p2_BYTE P2Util::msb(p2_LONG val)
{
    val |= (val >> 1);
    val |= (val >> 2);
    val |= (val >> 4);
    val |= (val >> 8);
    val |= (val >> 16);
    return static_cast<p2_BYTE>(val & ~(val >> 1));
}

/**
 * @brief Find the most significant 1 bit in value %val
 * @param val value
 * @return position of top most 1 bit
 */
p2_BYTE P2Util::msb(p2_WORD val)
{
    val |= (val >> 1);
    val |= (val >> 2);
    val |= (val >> 4);
    val |= (val >> 8);
    return static_cast<p2_BYTE>(val & ~(val >> 1));
}

/**
 * @brief Find the most significant 1 bit in value %val
 * @param val value
 * @return position of top most 1 bit
 */
p2_BYTE P2Util::msb(p2_BYTE val)
{
    val |= (val >> 1);
    val |= (val >> 2);
    val |= (val >> 4);
    return static_cast<p2_BYTE>(val & ~(val >> 1));
}

/**
 * @brief Count the number of leading zero bits
 * @param val value to inspect
 * @return the number of leading zero bits
 */
p2_BYTE P2Util::lcz(p2_LONG val)
{
    val |= (val >> 1);
    val |= (val >> 2);
    val |= (val >> 4);
    val |= (val >> 8);
    val |= (val >> 16);
    return 32 - ones(val);
}

/**
 * @brief Return the number of ones (1) in a 32 bit value
 * @param val 32 bit value
 * @return number of 1 bits
 */
p2_BYTE P2Util::ones(p2_LONG val)
{
    val = val - ((val >> 1) & 0x55555555);
    val = (val & 0x33333333) + ((val >> 2) & 0x33333333);
    val = (((val + (val >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
    return static_cast<p2_BYTE>(val);
}

/**
 * @brief Return the number of ones (1) in a 16 bit value
 * @param val 16 bit value
 * @return number of 1 bits
 */
p2_BYTE P2Util::ones(p2_WORD val)
{
    val = val - ((val >> 1) & 0x5555);
    val = (val & 0x3333) + ((val >> 2) & 0x3333);
    val = (((val + (val >> 4)) & 0x0F0F) * 0x0101) >> 12;
    return static_cast<p2_BYTE>(val);
}

/**
 * @brief Return the number of ones (1) in a 8 bit value
 * @param val 8 bit value
 * @return number of 1 bits
 */
p2_BYTE P2Util::ones(p2_BYTE val)
{
    val = val - ((val >> 1) & 0x55);
    val = (val & 0x33) + ((val >> 2) & 0x33);
    val = (val + (val >> 4)) & 0x0F;
    return static_cast<p2_BYTE>(val);
}

/**
 * @brief Return the parity of a 32 bit value
 * @param val 32 bit value
 * @return 1 for odd parity, 0 for even parity
 */
p2_BYTE P2Util::parity(p2_LONG val)
{
    val ^= val >> 16;
    val ^= val >> 8;
    val ^= val >> 4;
    val &= 15;
    return (0x6996 >> val) & 1;
}

/**
 * @brief Return the parity of a 16 bit value
 * @param val 16 bit value
 * @return 1 for odd parity, 0 for even parity
 */
p2_BYTE P2Util::parity(p2_WORD val)
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
p2_BYTE P2Util::parity(p2_BYTE val)
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
    const uchar bits[32] = {
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
