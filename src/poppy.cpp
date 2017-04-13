//
// Created by foudrer on 12/4/2017.
//

#include "poppy.h"

Poppy::Poppy(uint64_t * const bits, const uint64_t num_bits) {
    bits_ = bits;
    num_bits_ = num_bits;
    num_counts_ = 0;

    l1EntryCount_ = std::max(num_bits_ >> 32, (uint64_t) 1);
    l2EntryCount_ = num_bits_ >> 11;
    basicBlockCount_ = num_bits_ / kBasicBlockSize;

    assert(posix_memalign((void **) &l1Entries_, kCacheLineSize, l1EntryCount_ * sizeof(uint64_t)) >= 0);
    assert(posix_memalign((void **) &l2Entries_, kCacheLineSize, l2EntryCount_ * sizeof(uint64_t)) >= 0);

    uint64_t l2Id = 0;
    uint64_t basicBlockId = 0;

    memset(locCount_, 0, sizeof(locCount_));

    for (uint64_t i = 0; i < l1EntryCount_; i++) {
        l1Entries_[i] = num_counts_;

        uint32_t cum = 0;
        for (int k = 0; k < kL2EntryCountPerL1Entry; k++) {
            l2Entries_[l2Id] = cum;

            for (int offset = 0; offset < 30; offset += 10) {
                int c = popcountLinear(bits_,
                                       basicBlockId * kWordCountPerBasicBlock,
                                       kBasicBlockSize);
                cum += c;
                basicBlockId++;
                l2Entries_[l2Id] |= (uint64_t) c << (32 + offset);
            }
            cum += popcountLinear(bits_, basicBlockId * kWordCountPerBasicBlock, kBasicBlockSize);
            basicBlockId++;

            if (++l2Id >= l2EntryCount_) break;
        }

        locCount_[i] = (cum + kLocFreq - 1) / kLocFreq;
        num_counts_ += cum;
    }

    basicBlockId = 0;

    for (uint64_t i = 0; i < l1EntryCount_; i++) {
        loc_[i] = new uint32_t[locCount_[i]];
        locCount_[i] = 0;

        uint32_t oneCount = 0;

        for (uint32_t k = 0; k < kBasicBlockCountPerL1Entry; k++) {
            uint64_t woff = basicBlockId * kWordCountPerBasicBlock;
            for (int widx = 0; widx < kWordCountPerBasicBlock; widx++)
                for (int bit = 0; bit < kWordSize; bit++)
                    if (bits_[woff + widx] & (1ULL << (63 - bit))) {
                        oneCount++;
                        if ((oneCount & kLocFreqMask) == 1) {
                            loc_[i][locCount_[i]] = k * kBasicBlockSize + widx * kWordSize + bit;
                            locCount_[i]++;
                        }
                    }

            basicBlockId++;
            if (basicBlockId >= basicBlockCount_) break;
        }
    }
}

uint64_t Poppy::rank(uint64_t pos) {
    assert(pos <= num_bits_);
    //--pos;

    uint64_t l1Id = pos >> 32;
    uint64_t l2Id = pos >> 11;
    uint64_t x = l2Entries_[l2Id];

    uint64_t res = l1Entries_[l1Id] + (x & 0xFFFFFFFFULL);
    x >>= 32;

    int groupId = (pos & 2047) / 512;
    for (int i = 0; i < groupId; i++) {
        res += x & 1023;
        x >>= 10;
    }
    res += popcountLinear(bits_, (l2Id * 4 + groupId) * kWordCountPerBasicBlock, (pos & 511));

    return res;
}

uint64_t Poppy::select(uint64_t rank) {
    assert(rank <= num_counts_);

    uint64_t l1Id;
    uint64_t zero = 0;
    for (l1Id = l1EntryCount_ - 1; l1Id >= zero; l1Id--) {
        if (l1Entries_[l1Id] < rank) {
            rank -= l1Entries_[l1Id];
            break;
        }
    }

    uint32_t offset = l1Id * kL2EntryCountPerL1Entry;
    uint32_t maxL2Id = kL2EntryCountPerL1Entry;
    if (l1Id == l1EntryCount_ - 1)
        maxL2Id = l2EntryCount_ - offset;

    uint32_t pos = loc_[l1Id][(rank - 1) / kLocFreq];
    uint32_t l2Id = pos >> 11;

    while (l2Id + 1 < maxL2Id && (l2Entries_[l2Id + 1] & 0xFFFFFFFFULL) < rank)
        l2Id++;
    rank -= l2Entries_[l2Id] & 0xFFFFFFFFULL;

    uint32_t x = l2Entries_[l2Id] >> 32;
    int groupId;

    for (groupId = 0; groupId < 3; groupId++) {
        int k = x & 1023;
        if (rank > k)
            rank -= k;
        else
            break;
        x >>= 10;
    }

    return (l1Id << 32) + (l2Id << 11) + (groupId << 9) +
           select512(bits_, ((offset + l2Id) * 4 + groupId) * kWordCountPerBasicBlock, rank);
}

uint64_t Poppy::bit_count() {
    return num_counts_;
}

void Poppy::print_counts() {

}

Poppy::~Poppy() {

}
