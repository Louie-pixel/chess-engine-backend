#include "Transposition.h"
#include <random>

using namespace Chess;

// Define the static variables so the linker doesn't crash
TTEntry TranspositionTable::table[TT_SIZE];
uint64_t TranspositionTable::pieceKeys[12][64];
uint64_t TranspositionTable::sideKey;

// Special constant to tell the engine "We don't have this in memory"
constexpr int UNKNOWN_SCORE = -1000000;

void TranspositionTable::initZobrist() {
    // Use a fixed seed (12345) so our engine behaves predictably for debugging
    std::mt19937_64 rng(12345);
    std::uniform_int_distribution<uint64_t> dist;

    for (int piece = WHITE_PAWN; piece <= BLACK_KING; piece++) {
        for (int square = 0; square < 64; square++) {
            pieceKeys[piece][square] = dist(rng);
        }
    }
    sideKey = dist(rng);
}

void TranspositionTable::clear() {
    for (int i = 0; i < TT_SIZE; i++) {
        table[i].key = 0;
        table[i].depth = 0;
        table[i].score = 0;
        table[i].flag = HASH_EXACT;
    }
}

uint64_t TranspositionTable::generateHash(const Board& board) {
    uint64_t finalHash = 0ULL;

    for (int p = WHITE_PAWN; p <= BLACK_KING; p++) {
        Bitboard b = board.getPieceBoard(static_cast<Piece>(p));
        while (b) {
            int sq = __builtin_ctzll(b);
            finalHash ^= pieceKeys[p][sq]; // XOR the random key into the hash
            b &= b - 1; // Clear the bit
        }
    }

    if (board.getSideToMove() == BLACK) {
        finalHash ^= sideKey;
    }

    return finalHash;
}

int TranspositionTable::readHash(uint64_t key, int depth, int alpha, int beta) {
    TTEntry* entry = &table[key % TT_SIZE]; // Use modulo to map the 64-bit key to our array size

    // If the fingerprint perfectly matches, and we searched at least as deep as we need to right now
    if (entry->key == key && entry->depth >= depth) {
        if (entry->flag == HASH_EXACT) {
            return entry->score;
        }
        if (entry->flag == HASH_ALPHA && entry->score <= alpha) {
            return alpha;
        }
        if (entry->flag == HASH_BETA && entry->score >= beta) {
            return beta;
        }
    }
    return UNKNOWN_SCORE;
}

void TranspositionTable::writeHash(uint64_t key, int depth, int score, TTFlag flag) {
    TTEntry* entry = &table[key % TT_SIZE];
    
    // Always overwrite (A common and effective TT replacement scheme)
    entry->key = key;
    entry->depth = depth;
    entry->score = score;
    entry->flag = flag;
}