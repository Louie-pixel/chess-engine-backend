#pragma once
#include <cstdint>
#include "Board.h"
#include "Types.h"

namespace Chess {
    
    // --- FILE MASKS ---
    // These prevent pieces from teleporting across the board when shifting bits.
    constexpr Bitboard FILE_A = 0x0101010101010101ULL;
    constexpr Bitboard FILE_B = 0x0202020202020202ULL;
    constexpr Bitboard FILE_G = 0x4040404040404040ULL;
    constexpr Bitboard FILE_H = 0x8080808080808080ULL;

    constexpr Bitboard NOT_A_FILE = ~FILE_A;
    constexpr Bitboard NOT_AB_FILE = ~(FILE_A | FILE_B);
    constexpr Bitboard NOT_H_FILE = ~FILE_H;
    constexpr Bitboard NOT_GH_FILE = ~(FILE_G | FILE_H);

    class MoveGen {
    public:
        // Knights
        static Bitboard getKnightAttacks(int square);
        
        // Pawns
        static Bitboard getWhitePawnPushes(const Board& board);
        static Bitboard getBlackPawnPushes(const Board& board);
        static Bitboard getWhitePawnAttacks(const Board& board);
        static Bitboard getBlackPawnAttacks(const Board& board);

        // Sliders
        static Bitboard getRookAttacks(int square, Bitboard blockers);
        static Bitboard getBishopAttacks(int square, Bitboard blockers);
        static Bitboard getQueenAttacks(int square, Bitboard blockers);
        // The Master Generator
        static void generateAllMoves(const Board& board, MoveList& moveList);
        static Bitboard getKingAttacks(int square);
        // Checks if a specific square is attacked by a specific color
        static bool isSquareAttacked(int square, Color attackerColor, const Board& board);
    };
}
