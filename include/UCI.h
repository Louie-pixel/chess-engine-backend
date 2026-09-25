#pragma once
#include "Board.h"
#include <string>

namespace Chess {
    class UCI {
    private:
        static void parsePosition(const std::string& command, Board& board);
        static void parseGo(const std::string& command, Board& board);
        
    public:
        static void loop();
    };
}