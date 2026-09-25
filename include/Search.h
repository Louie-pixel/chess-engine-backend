#pragma once
#include "Board.h"
#include "Types.h"
#include "MoveGen.h"
#include "Evaluation.h"
#include <chrono>

namespace Chess {
    class Search {
    private:
        // Timing control variables
        static std::chrono::time_point<std::chrono::steady_clock> startTime;
        static int timeLimitMs;
        static bool abortFlag;
        static int nodesSearched;

        // Internal recursive search loops
        static int quiescence(Board board, int alpha, int beta);
        static int negamax(Board board, int depth, int alpha, int beta);
        static void orderMoves(MoveList& list);
        
        // Helper to check if the engine has run out of allocated time
        static void checkTime();

    public:
        // Changed interface: The engine now takes a maximum time budget in milliseconds
        static Move getBestMove(Board board, int allocatedTimeMs);
    };
}