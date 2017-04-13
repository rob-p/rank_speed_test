//
// Created by foudrer on 12/4/2017.
//

#ifndef RANKSELECT_COMPARISON_POPPY_H
#define RANKSELECT_COMPARISON_POPPY_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>

#include "popcount.h"

const int kWordSize = 64;
const int kBasicBlockSize = 512;
const int kBasicBlockBits = 9;
const int kBasicBlockMask = kBasicBlockSize - 1;
const int kWordCountPerBasicBlock = kBasicBlockSize / kWordSize;
const int kCacheLineSize = 64;

class Poppy {
public:
    Poppy(uint64_t * const bits, const uint64_t num_bits);
    ~Poppy();

    uint64_t rank(uint64_t pos);
    uint64_t select(uint64_t rank);

    // just for analysis
    void print_counts();
    uint64_t bit_count();

private:
    uint64_t *bits_;
    uint64_t num_bits_;
    uint64_t num_counts_;

    uint64_t *l2Entries_;
    uint64_t l2EntryCount_;
    uint64_t *l1Entries_;
    uint64_t l1EntryCount_;
    uint64_t basicBlockCount_;

    uint32_t *loc_[1 << 16];
    uint32_t locCount_[1 << 16];

    static const int kLocFreq = 8192;
    static const int kLocFreqMask = 8191;
    static const int kL2EntryCountPerL1Entry = 1 << 21;
    static const int kBasicBlockCountPerL1Entry = 1 << 23;
};

#endif //RANKSELECT_COMPARISON_POPPY_H
