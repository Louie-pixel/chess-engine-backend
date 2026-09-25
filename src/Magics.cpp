#include "Magics.h"

using namespace Chess;

uint64_t Magics::rookAttacks[64][4096];
uint64_t Magics::bishopAttacks[64][512];

// --- 1. THE MAGIC NUMBERS ---
// Found via brute-force supercomputing. They guarantee no two blocker 
// combinations on a specific square will ever result in the same array index.
const uint64_t Magics::rookMagics[64] = {
    0x8a80104000800020ULL, 0x140002000100040ULL, 0x2801880a0017001ULL, 0x100081001000420ULL, 0x200020010080420ULL, 0x3001c0002010008ULL, 0x8480008002000100ULL, 0x2080088004402900ULL,
    0x800098204000200ULL, 0x2024401000200040ULL, 0x100802000801000ULL, 0x120800800801000ULL, 0x208808088000400ULL, 0x280220080040084ULL, 0x2200800100020080ULL, 0x801000060821100ULL,
    0x80044006422000ULL, 0x100808020004000ULL, 0x12108a0010204200ULL, 0x140848010000802ULL, 0x481828014002800ULL, 0x8094004002004100ULL, 0x4010040010010802ULL, 0x200040006043200ULL,
    0x804040008008080ULL, 0x12000810020004ULL, 0x1000100200040208ULL, 0x430000a044020001ULL, 0x280008834000202ULL, 0x40006092804104ULL, 0x91024404000800ULL, 0x82226040020010ULL,
    0x401014102080400ULL, 0x8020200404000101ULL, 0x8100900404402ULL, 0x410020080800200ULL, 0x41600020080800ULL, 0x2400040100400ULL, 0x4004001014208000ULL, 0x8000404002044ULL,
    0x1000804080111ULL, 0x12200802000000ULL, 0x400403102000400ULL, 0x40000020100400ULL, 0x400002020084000ULL, 0x2040002000224ULL, 0x200080204005ULL, 0x40400000402000ULL,
    0x8180480000ULL, 0x100400000000ULL, 0x8010000000ULL, 0x8040000000ULL, 0x100400000000ULL, 0x100000000ULL, 0x8020000000ULL, 0x4010000000ULL,
    0x40400000ULL, 0x2000000ULL, 0x10040000ULL, 0x200000ULL, 0x40000ULL, 0x2000ULL, 0x200000ULL, 0x1000ULL
};

const uint64_t Magics::bishopMagics[64] = {
    0x40040844404084ULL, 0x2004208a004208ULL, 0x10190041080202ULL, 0x108060845042010ULL, 0x581104180800210ULL, 0x2112080446200010ULL, 0x1080820820060210ULL, 0x20080802102008ULL,
    0x40040010241080ULL, 0x4002400840108ULL, 0x10008060140100ULL, 0x2040420080001ULL, 0x8004104808000ULL, 0x80400010408080ULL, 0x8004028042080ULL, 0x1004044002001ULL,
    0x100400800040ULL, 0x820a0280400ULL, 0x20104084200ULL, 0x80084120ULL, 0x400088100ULL, 0x804422008ULL, 0x4208220ULL, 0x20840004ULL,
    0x40040ULL, 0x400100100ULL, 0x80ULL, 0x200ULL, 0x20ULL, 0x10080ULL, 0x2000040ULL, 0x1ULL,
    0x8008ULL, 0x4001000ULL, 0x2008000ULL, 0x400080000000ULL, 0x400008ULL, 0x200ULL, 0x800ULL, 0x8ULL,
    0x80ULL, 0x40ULL, 0x4ULL, 0x20080ULL, 0x8ULL, 0x80ULL, 0x1000ULL, 0x4ULL,
    0x4ULL, 0x2ULL, 0x10000ULL, 0x4ULL, 0x10ULL, 0x10ULL, 0x40ULL, 0x4ULL,
    0x4ULL, 0x2ULL, 0x8ULL, 0x10ULL, 0x2ULL, 0x2ULL, 0x2ULL, 0x2ULL
};

// Shift counts to pull the index down to the first 0-11 bits
const int Magics::rookShifts[64] = {
    52, 53, 53, 53, 53, 53, 53, 52, 53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53, 53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53, 53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53, 52, 53, 53, 53, 53, 53, 53, 52
};

const int Magics::bishopShifts[64] = {
    58, 59, 59, 59, 59, 59, 59, 58, 59, 59, 59, 59, 59, 59, 59, 59,
    59, 59, 57, 57, 57, 57, 59, 59, 59, 59, 57, 55, 55, 57, 59, 59,
    59, 59, 57, 55, 55, 57, 59, 59, 59, 59, 57, 57, 57, 57, 59, 59,
    59, 59, 59, 59, 59, 59, 59, 59, 58, 59, 59, 59, 59, 59, 59, 58
};

// Masks for blocking squares (excludes outer edges because outer edges don't block anything)
const uint64_t Magics::rookMasks[64] = {
    0x10101010101017e, 0x20202020202027c, 0x40404040404047a, 0x808080808080876, 0x101010101010106e, 0x202020202020205e, 0x404040404040403e, 0x808080808080807e,
    0x101010101017e00, 0x202020202027c00, 0x404040404047a00, 0x808080808087600, 0x1010101010106e00, 0x2020202020205e00, 0x4040404040403e00, 0x8080808080807e00,
    0x1010101017e0100, 0x2020202027c0200, 0x4040404047a0400, 0x808080808760800, 0x10101010106e1000, 0x20202020205e2000, 0x40404040403e4000, 0x80808080807e8000,
    0x10101017e010100, 0x20202027c020200, 0x40404047a040400, 0x808080876080800, 0x101010106e101000, 0x202020205e202000, 0x404040403e404000, 0x808080807e808000,
    0x101017e01010100, 0x202027c02020200, 0x404047a04040400, 0x808087608080800, 0x1010106e10101000, 0x2020205e20202000, 0x4040403e40404000, 0x8080807e80808000,
    0x1017e0101010100, 0x2027c0202020200, 0x4047a0404040400, 0x808760808080800, 0x10106e1010101000, 0x20205e2020202000, 0x40403e4040404000, 0x80807e8080808000,
    0x17e010101010100, 0x27c020202020200, 0x47a040404040400, 0x876080808080800, 0x106e101010101000, 0x205e202020202000, 0x403e404040404000, 0x807e808080808000,
    0x7e01010101010100, 0x7c02020202020200, 0x7a04040404040400, 0x7608080808080800, 0x6e10101010101000, 0x5e20202020202000, 0x3e40404040404000, 0x7e80808080808000
};

const uint64_t Magics::bishopMasks[64] = {
    0x40201008040200, 0x402010080400, 0x4020100800, 0x40201000, 0x402000, 0x4000, 0x0, 0x0,
    0x20100804020000, 0x40201008040000, 0x402010080000, 0x4020100000, 0x40200000, 0x400000, 0x0, 0x0,
    0x10080402000000, 0x20100804000000, 0x40201008000000, 0x402010000000, 0x4020000000, 0x40000000, 0x0, 0x0,
    0x8040200000000, 0x10080400000000, 0x20100800000000, 0x40201000000000, 0x402000000000, 0x4000000000, 0x0, 0x0,
    0x4020000000000, 0x8040000000000, 0x10080000000000, 0x20100000000000, 0x40200000000000, 0x400000000000, 0x0, 0x0,
    0x2000000000000, 0x4000000000000, 0x8000000000000, 0x10000000000000, 0x20000000000000, 0x40000000000000, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};

// Map bits from a sequential index onto the actual squares of the mask
uint64_t Magics::setOccupancy(int index, int bitsInMask, uint64_t attackMask) {
    uint64_t occupancy = 0ULL;
    for (int count = 0; count < bitsInMask; count++) {
        int square = __builtin_ctzll(attackMask);
        attackMask &= attackMask - 1; // Pop least significant bit
        if (index & (1 << count)) {
            occupancy |= (1ULL << square);
        }
    }
    return occupancy;
}

// Re-using your classical loops one final time to generate the truth tables
uint64_t Magics::generateRookAttacksClassical(int square, uint64_t block) {
    uint64_t attacks = 0ULL;
    int r = square / 8, f = square % 8;
    for (int i = r + 1; i <= 7; i++) { attacks |= (1ULL << (i * 8 + f)); if (block & (1ULL << (i * 8 + f))) break; }
    for (int i = r - 1; i >= 0; i--) { attacks |= (1ULL << (i * 8 + f)); if (block & (1ULL << (i * 8 + f))) break; }
    for (int i = f + 1; i <= 7; i++) { attacks |= (1ULL << (r * 8 + i)); if (block & (1ULL << (r * 8 + i))) break; }
    for (int i = f - 1; i >= 0; i--) { attacks |= (1ULL << (r * 8 + i)); if (block & (1ULL << (r * 8 + i))) break; }
    return attacks;
}

uint64_t Magics::generateBishopAttacksClassical(int square, uint64_t block) {
    uint64_t attacks = 0ULL;
    int r = square / 8, f = square % 8;
    for (int i = r + 1, j = f + 1; i <= 7 && j <= 7; i++, j++) { attacks |= (1ULL << (i * 8 + j)); if (block & (1ULL << (i * 8 + j))) break; }
    for (int i = r - 1, j = f + 1; i >= 0 && j <= 7; i--, j++) { attacks |= (1ULL << (i * 8 + j)); if (block & (1ULL << (i * 8 + j))) break; }
    for (int i = r + 1, j = f - 1; i <= 7 && j >= 0; i++, j--) { attacks |= (1ULL << (i * 8 + j)); if (block & (1ULL << (i * 8 + j))) break; }
    for (int i = r - 1, j = f - 1; i >= 0 && j >= 0; i--, j--) { attacks |= (1ULL << (i * 8 + j)); if (block & (1ULL << (i * 8 + j))) break; }
    return attacks;
}

// THIS RUNS ONCE AT STARTUP. It populates the massive arrays.
void Magics::initialize() {
    for (int sq = 0; sq < 64; sq++) {
        // Init Rooks
        int rookBits = 64 - rookShifts[sq];
        int rookIndices = (1 << rookBits);
        for (int i = 0; i < rookIndices; i++) {
            uint64_t occupancy = setOccupancy(i, rookBits, rookMasks[sq]);
            int magicIndex = (occupancy * rookMagics[sq]) >> rookShifts[sq];
            rookAttacks[sq][magicIndex] = generateRookAttacksClassical(sq, occupancy);
        }

        // Init Bishops
        int bishopBits = 64 - bishopShifts[sq];
        int bishopIndices = (1 << bishopBits);
        for (int i = 0; i < bishopIndices; i++) {
            uint64_t occupancy = setOccupancy(i, bishopBits, bishopMasks[sq]);
            int magicIndex = (occupancy * bishopMagics[sq]) >> bishopShifts[sq];
            bishopAttacks[sq][magicIndex] = generateBishopAttacksClassical(sq, occupancy);
        }
    }
}