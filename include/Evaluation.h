#pragma once
#include "Board.h"
#include "Types.h"

namespace Chess {
    class Evaluator {
    public:
        // Standard piece values in "Centipawns"
        static constexpr int PAWN_VALUE = 100;
        static constexpr int KNIGHT_VALUE = 300;
        static constexpr int BISHOP_VALUE = 300;
        static constexpr int ROOK_VALUE = 500;
        static constexpr int QUEEN_VALUE = 900;

        // Returns a score for the current board
        static int evaluate(const Board& board);
    };
}