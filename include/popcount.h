//
// Created by foudrer on 12/4/2017.
//

#ifndef RANKSELECT_COMPARISON_POPCOUNT_H
#define RANKSELECT_COMPARISON_POPCOUNT_H

#include "macros.h"
#include <nmmintrin.h>

#define popcountsize 64ULL
#define popcountmask (popcountsize - 1)

inline uint64_t popcountLinear(uint64_t *bits, uint64_t x, uint64_t nbits) {
    if (nbits == 0)
        return 0;

    uint64_t lastword = (nbits - 1) / popcountsize;
    uint64_t p = 0;

    for (int i = 0; i < lastword; i++) {
        p += _mm_popcnt_u64(bits[x+i]);
    }

    uint64_t lastshifted = bits[x+lastword] >> (63 - ((nbits - 1) & popcountmask));
    p += _mm_popcnt_u64(lastshifted);
    return p;
}

inline int select64_naive(uint64_t x, int k) {
    int count = -1;
    for (int i = 63; i >= 0; i--) {
        count++;
        if (x & (1ULL << i)) {
            k--;
            if (k == 0) {
                return count;
            }
        }
    }
    return -1;
}

inline int select64_popcount_search(uint64_t x, int k) {
    int loc = -1;
    // if (k > popcount(x)) { return -1; }

    for (int testbits = 32; testbits > 0; testbits >>= 1) {
        int lcount = _mm_popcnt_u64(x >> testbits);
        if (k > lcount) {
            x &= ((1ULL << testbits)-1);
            loc += testbits;
            k -= lcount;
        } else {
            x >>= testbits;
        }
    }
    return loc+k;
}

inline int select64_broadword(uint64_t x, int k) {
    uint64_t word = x;
    int residual = k;
    uint64_t byte_sums;

    byte_sums = word - ( ( word & 0xa * ONES_STEP_4 ) >> 1 );
    byte_sums = ( byte_sums & 3 * ONES_STEP_4 ) + ( ( byte_sums >> 2 ) & 3 * ONES_STEP_4 );
    byte_sums = ( byte_sums + ( byte_sums >> 4 ) ) & 0x0f * ONES_STEP_8;
    byte_sums *= ONES_STEP_8;

    // Phase 2: compare each byte sum with the residual
    const uint64_t residual_step_8 = residual * ONES_STEP_8;
    const int place = ( LEQ_STEP_8( byte_sums, residual_step_8 ) * ONES_STEP_8 >> 53 ) & ~0x7;

    // Phase 3: Locate the relevant byte and make 8 copies with incremental masks
    const int byte_rank = residual - ( ( ( byte_sums << 8 ) >> place ) & 0xFF );

    const uint64_t spread_bits = ( word >> place & 0xFF ) * ONES_STEP_8 & INCR_STEP_8;
    const uint64_t bit_sums = ZCOMPARE_STEP_8( spread_bits ) * ONES_STEP_8;

    // Compute the inside-byte location and return the sum
    const uint64_t byte_rank_step_8 = byte_rank * ONES_STEP_8;

    return place + ( LEQ_STEP_8( bit_sums, byte_rank_step_8 ) * ONES_STEP_8 >> 56 );
}

inline int select64(uint64_t x, int k) {
    return select64_popcount_search(x, k);
}

// x is the starting offset of the 512 bits;
// k is the thing we're selecting for.
inline int select512(uint64_t *bits, int x, int k) {
    __asm__ __volatile__ (
    "prefetchnta (%0)\n"
    : : "r" (&bits[x]) );
    int i = 0;
    int pop = _mm_popcnt_u64(bits[x+i]);
    while (k > pop && i < 7) {
        k -= pop;
        i++;
        pop = _mm_popcnt_u64(bits[x+i]);
    }
    if (i == 7 && _mm_popcnt_u64(bits[x+i]) < k) {
        return -1;
    }
    // We're now certain that the bit we want is stored in bv[x+i]
    return i*64 + select64(bits[x+i], k);
}

// brute-force linear select
// x is the starting offset of the bits in bv;
// k is the thing we're selecting for (starting from bv[x]).
// bvlen is the total length of bv
inline int selectLinear(uint64_t *bits, uint64_t length, uint64_t x, uint64_t k) {
    if (k > (length - x) * 64)
        return -1;
    uint64_t i = 0;
    uint64_t pop = _mm_popcnt_u64(bits[x+i]);
    while (k > pop && i < (length - 1)) {
        k -= pop;
        i++;
        pop = _mm_popcnt_u64(bits[x+i]);
    }
    if ((i == length - 1) && (pop < k)) {
        return -1;
    }
    // We're now certain that the bit we want is stored in bits[x+i]
    return i*64 + select64(bits[x+i], k);
}

#endif //RANKSELECT_COMPARISON_POPCOUNT_H
