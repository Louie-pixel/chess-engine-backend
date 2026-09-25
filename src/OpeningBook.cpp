#include "OpeningBook.h"
#include "Transposition.h"
#include "MoveGen.h"

using namespace Chess;

std::unordered_map<uint64_t, std::string> OpeningBook::book;

void OpeningBook::addLine(const std::vector<std::string>& moves) {
    Board board;
    board.initialize(); // Start from standard position

    for (const std::string& moveStr : moves) {
        uint64_t hash = TranspositionTable::generateHash(board);
        
        // If we haven't mapped this position yet, map it to the next move in the line
        if (book.find(hash) == book.end()) {
            book[hash] = moveStr;
        }

        // Physically execute the move on our invisible board to advance the state
        int fromSq = (moveStr[1] - '1') * 8 + (moveStr[0] - 'a');
        int toSq   = (moveStr[3] - '1') * 8 + (moveStr[2] - 'a');
        
        Piece pMoved = EMPTY, pCaptured = EMPTY;
        for (int p = WHITE_PAWN; p <= BLACK_KING; p++) {
            if (board.getBit(board.getPieceBoard(static_cast<Piece>(p)), fromSq)) pMoved = static_cast<Piece>(p);
            if (board.getBit(board.getPieceBoard(static_cast<Piece>(p)), toSq)) pCaptured = static_cast<Piece>(p);
        }
        
        board.makeMove(fromSq, toSq, pMoved, pCaptured);
    }
}

void OpeningBook::initialize() {
    book.clear();

    // 1. The Italian Game
    addLine({"e2e4", "e7e5", "g1f3", "b8c6", "f1c4", "f8c5"});
    
    // 2. The Sicilian Defense
    addLine({"e2e4", "c7c5", "g1f3", "d7d6", "d2d4", "c5d4", "f3d4", "g8f6", "b1c3"});
    
    // 3. The Queen's Gambit
    addLine({"d2d4", "d7d5", "c2c4", "e7e6", "b1c3", "g8f6"});
    
    // 4. The French Defense
    addLine({"e2e4", "e7e6", "d2d4", "d7d5", "b1c3", "g8f6", "c1g5"});
}

bool OpeningBook::hasMove(uint64_t hash) {
    return book.find(hash) != book.end();
}

std::string OpeningBook::getMove(uint64_t hash) {
    return book[hash];
}