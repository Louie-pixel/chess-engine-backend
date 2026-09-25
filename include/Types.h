#pragma once
#include <cstdint>

namespace Chess {
    // Standardizing the Bitboard as a 64-bit unsigned integer
    using Bitboard = std::uint64_t;

    enum Color { WHITE, BLACK, BOTH };

    enum Piece {
        WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
        BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING,
        EMPTY
    };

    enum Square {
        A1, B1, C1, D1, E1, F1, G1, H1,
        A2, B2, C2, D2, E2, F2, G2, H2,
        A3, B3, C3, D3, E3, F3, G3, H3,
        A4, B4, C4, D4, E4, F4, G4, H4,
        A5, B5, C5, D5, E5, F5, G5, H5,
        A6, B6, C6, D6, E6, F6, G6, H6,
        A7, B7, C7, D7, E7, F7, G7, H7,
        A8, B8, C8, D8, E8, F8, G8, H8
    };
    struct Move {
        int fromSquare;
        int toSquare;
        Piece pieceMoved;
        Piece pieceCaptured; // Set to EMPTY if no capture
        Piece promotedPiece; // Set to EMPTY if no promotion
        bool isEnPassant;
        bool isCastling;
    };

    // --- MOVE LIST ---
    // A highly efficient, pre-allocated array to store generated moves.
    class MoveList {
    public:
        Move moves[256]; // 256 is safely above the maximum possible moves in any chess position
        int count;

        // Constructor to ensure count starts at 0
        MoveList() : count(0) {}

        // Fast addition of a move
        void addMove(Move m) {
            moves[count] = m;
            count++;
        }
        
        // Reset the list for the next turn
        void clear() {
            count = 0;
        }
    };
}