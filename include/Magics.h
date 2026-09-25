#pragma once
#include <cstdint>
#include "Types.h"

namespace Chess {
    class Magics {
    private:
        // The lookup tables: [square][hash_index]
        static uint64_t rookAttacks[64][4096];
        static uint64_t bishopAttacks[64][512];

        // The pre-calculated mathematical constants
        static const uint64_t rookMagics[64];
        static const uint64_t bishopMagics[64];
        static const uint64_t rookMasks[64];
        static const uint64_t bishopMasks[64];
        static const int rookShifts[64];
        static const int bishopShifts[64];

        // Helpers to initialize the tables at startup
        static uint64_t setOccupancy(int index, int bitsInMask, uint64_t attackMask);
        static uint64_t generateRookAttacksClassical(int square, uint64_t block);
        static uint64_t generateBishopAttacksClassical(int square, uint64_t block);

    public:
        static void initialize();
        
        // The O(1) constant-time lookup functions!
        static inline uint64_t getRookAttacks(int sq, uint64_t occ) {
            occ &= rookMasks[sq];
            occ *= rookMagics[sq];
            occ >>= rookShifts[sq];
            return rookAttacks[sq][occ];
        }

        static inline uint64_t getBishopAttacks(int sq, uint64_t occ) {
            occ &= bishopMasks[sq];
            occ *= bishopMagics[sq];
            occ >>= bishopShifts[sq];
            return bishopAttacks[sq][occ];
        }
    };
}