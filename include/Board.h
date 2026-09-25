#pragma once
#include <iostream>
#include <cstdint>
#include <string>
#include "Types.h"

namespace Chess {
    
    struct BoardState {
        bool whiteCanCastleKingside;
        bool whiteCanCastleQueenside;
        bool blackCanCastleKingside;
        bool blackCanCastleQueenside;
        int enPassantSquare; 
    };

    class Board {
    private:
        Bitboard pieceBoards[12];
        Bitboard occupancyBoards[3];
        Color sideToMove;
        BoardState state;

        void setBit(Bitboard& b, int square);
        void clearBit(Bitboard& b, int square);

    public:
        Board();
        
        // --- INITIALIZATION ---
        void initialize(); // Standard starting position
        void loadFEN(const std::string& fen); // Custom starting position
        
        void printBoard() const;
        
        // --- GETTERS ---
        bool getBit(Bitboard b, int square) const;
        Bitboard getOccupancy(Color c) const;
        Bitboard getPieceBoard(Piece p) const { return pieceBoards[p]; }
        Color getSideToMove() const { return sideToMove; }
        BoardState getState() const { return state; }

        // --- CORE PHYSICS ---
        void makeMove(int fromSquare, int toSquare, Piece pieceMoved, Piece pieceCaptured);
    };
}