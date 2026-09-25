#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <cstdint>
#include "Board.h"

namespace Chess {
    class OpeningBook {
    private:
        // Maps a Zobrist Hash to a specific move string (e.g., "e2e4")
        static std::unordered_map<uint64_t, std::string> book;
        
        // Internal helper to simulate moves and calculate their hashes dynamically
        static void addLine(const std::vector<std::string>& moves);

    public:
        // Run once at startup to compile the book
        static void initialize();
        
        static bool hasMove(uint64_t hash);
        static std::string getMove(uint64_t hash);
    };
}