#include "UCI.h"
#include "Search.h"
#include "Transposition.h"
#include "OpeningBook.h"
#include <iostream>
#include <sstream>
#include <vector>

using namespace Chess;

void UCI::loop() {
    Board board;
    board.initialize();
    
    std::string line;
    
    // Listen infinitely for GUI commands
    while (std::getline(std::cin, line)) {
        
        // CRITICAL FIX: Strip Windows carriage returns (\r) from WSL pipe
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line == "quit") {
            break;
        } 
        else if (line == "uci") {
            std::cout << "id name C++ Minimax Engine" << std::endl;
            std::cout << "id author You" << std::endl;
            std::cout << "uciok" << std::endl;
        } 
        else if (line == "isready") {
            std::cout << "readyok" << std::endl;
        } 
        else if (line == "ucinewgame") {
            TranspositionTable::clear();
            board.initialize();
        } 
        else if (line.find("position") == 0) {
            parsePosition(line, board);
        } 
        else if (line.find("go") == 0) {
            parseGo(line, board);
        } 
    }
}

void UCI::parsePosition(const std::string& command, Board& board) {
    std::istringstream ss(command);
    std::string token;
    ss >> token; 

    ss >> token;
    if (token == "startpos") {
        board.initialize();
        ss >> token; 
    } else if (token == "fen") {
        std::string fen = "";
        while (ss >> token && token != "moves") {
            fen += token + " ";
        }
        board.loadFEN(fen);
    }

    if (token == "moves" || ss >> token) {
        while (ss >> token) {
            int fromSq = (token[1] - '1') * 8 + (token[0] - 'a');
            int toSq   = (token[3] - '1') * 8 + (token[2] - 'a');
            
            Piece pMoved = EMPTY, pCaptured = EMPTY;
            for (int p = WHITE_PAWN; p <= BLACK_KING; p++) {
                if (board.getBit(board.getPieceBoard(static_cast<Piece>(p)), fromSq)) pMoved = static_cast<Piece>(p);
                if (board.getBit(board.getPieceBoard(static_cast<Piece>(p)), toSq)) pCaptured = static_cast<Piece>(p);
            }
            
            if (pMoved == WHITE_PAWN && toSq == board.getState().enPassantSquare) pCaptured = BLACK_PAWN;
            if (pMoved == BLACK_PAWN && toSq == board.getState().enPassantSquare) pCaptured = WHITE_PAWN;
            
            if (pMoved != EMPTY) {
                board.makeMove(fromSq, toSq, pMoved, pCaptured);
            }
        }
    }
}

void UCI::parseGo(const std::string& command, Board& board) {
    int allocatedTimeMs = 1000; 
    
    std::istringstream ss(command);
    std::string token;
    
    while (ss >> token) {
        if (token == "wtime" && board.getSideToMove() == WHITE) {
            ss >> allocatedTimeMs;
            allocatedTimeMs /= 20; 
        } else if (token == "btime" && board.getSideToMove() == BLACK) {
            ss >> allocatedTimeMs;
            allocatedTimeMs /= 20;
        } else if (token == "movetime") {
            ss >> allocatedTimeMs; 
        }
    }

    if (allocatedTimeMs < 50) allocatedTimeMs = 50;
    if (allocatedTimeMs > 5000) allocatedTimeMs = 5000;

    Move bestMove = Search::getBestMove(board, allocatedTimeMs);
    
    auto squareToAlgebraic = [](int sq) -> std::string {
        return std::string() + (char)('a' + (sq % 8)) + (char)('1' + (sq / 8));
    };
    
    // Final strictly formatted response
    std::cout << "bestmove " << squareToAlgebraic(bestMove.fromSquare) << squareToAlgebraic(bestMove.toSquare) << std::endl;
}