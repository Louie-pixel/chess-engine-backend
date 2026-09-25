#pragma once
#include "Board.h"
#include "Types.h"
#include <cstdint>

namespace Chess {
    
    // Flags to tell us what kind of score we saved
    enum TTFlag { 
        HASH_EXACT, // We fully calculated this node
        HASH_ALPHA, // This move was too bad to finish calculating
        HASH_BETA   // This move was so good the opponent will avoid it
    };

    struct TTEntry {
        uint64_t key;
        int depth;
        int score;
        TTFlag flag;
    };

    // ~1 Million entries takes about 16MB of RAM. Perfect for our engine.
    constexpr int TT_SIZE = 1048576; 

    class TranspositionTable {
    private:
        static TTEntry table[TT_SIZE];
        static uint64_t pieceKeys[12][64];
        static uint64_t sideKey; // XORed if it is Black's turn

    public:
        // Initializes the random 64-bit numbers (Run once at startup)
        static void initZobrist();

        // Generates the unique 64-bit fingerprint for a given board
        static uint64_t generateHash(const Board& board);

        // Clears the memory before a new game
        static void clear();

        // Read and Write to the memory
        static int readHash(uint64_t key, int depth, int alpha, int beta);
        static void writeHash(uint64_t key, int depth, int score, TTFlag flag);
    };
}