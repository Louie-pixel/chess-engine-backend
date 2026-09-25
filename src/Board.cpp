#include "Board.h"
#include <cmath>
#include <cctype>

using namespace Chess;

Board::Board() {
    for (int i = 0; i < 12; i++) pieceBoards[i] = 0ULL;
    for (int j = 0; j < 3; j++) occupancyBoards[j] = 0ULL;
    sideToMove = WHITE;
    
    state.whiteCanCastleKingside = true;
    state.whiteCanCastleQueenside = true;
    state.blackCanCastleKingside = true;
    state.blackCanCastleQueenside = true;
    state.enPassantSquare = -1;
}

void Board::setBit(Bitboard& b, int square) { b |= (1ULL << square); }
void Board::clearBit(Bitboard& b, int square) { b &= ~(1ULL << square); }
bool Board::getBit(Bitboard b, int square) const { return (b & (1ULL << square)) != 0; }

void Board::initialize() {
    // Standard chess starting position FEN
    loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

// --- FEN PARSER ---
void Board::loadFEN(const std::string& fen) {
    // 1. Wipe the board completely clean
    for (int i = 0; i < 12; i++) pieceBoards[i] = 0ULL;
    for (int j = 0; j < 3; j++) occupancyBoards[j] = 0ULL;
    
    state.whiteCanCastleKingside = false;
    state.whiteCanCastleQueenside = false;
    state.blackCanCastleKingside = false;
    state.blackCanCastleQueenside = false;
    state.enPassantSquare = -1;

    int rank = 7, file = 0;
    size_t i = 0;

    // 2. Parse Piece Placement
    for (; i < fen.length() && fen[i] != ' '; i++) {
        char c = fen[i];
        if (c == '/') {
            rank--;
            file = 0;
        } else if (isdigit(c)) {
            file += (c - '0'); // Skip empty squares
        } else {
            int square = rank * 8 + file;
            switch (c) {
                case 'P': setBit(pieceBoards[WHITE_PAWN], square); break;
                case 'N': setBit(pieceBoards[WHITE_KNIGHT], square); break;
                case 'B': setBit(pieceBoards[WHITE_BISHOP], square); break;
                case 'R': setBit(pieceBoards[WHITE_ROOK], square); break;
                case 'Q': setBit(pieceBoards[WHITE_QUEEN], square); break;
                case 'K': setBit(pieceBoards[WHITE_KING], square); break;
                case 'p': setBit(pieceBoards[BLACK_PAWN], square); break;
                case 'n': setBit(pieceBoards[BLACK_KNIGHT], square); break;
                case 'b': setBit(pieceBoards[BLACK_BISHOP], square); break;
                case 'r': setBit(pieceBoards[BLACK_ROOK], square); break;
                case 'q': setBit(pieceBoards[BLACK_QUEEN], square); break;
                case 'k': setBit(pieceBoards[BLACK_KING], square); break;
            }
            file++;
        }
    }

    // 3. Parse Side to Move
    i++; 
    if (i < fen.length()) {
        sideToMove = (fen[i] == 'w') ? WHITE : BLACK;
        i += 2; 
    }

    // 4. Parse Castling Rights
    if (i < fen.length() && fen[i] != '-') {
        while (i < fen.length() && fen[i] != ' ') {
            switch (fen[i]) {
                case 'K': state.whiteCanCastleKingside = true; break;
                case 'Q': state.whiteCanCastleQueenside = true; break;
                case 'k': state.blackCanCastleKingside = true; break;
                case 'q': state.blackCanCastleQueenside = true; break;
            }
            i++;
        }
    } else {
        i++; 
    }
    i++; 

    // 5. Parse En Passant Target
    if (i < fen.length() && fen[i] != '-') {
        int epFile = fen[i] - 'a';
        int epRank = fen[i+1] - '1';
        state.enPassantSquare = epRank * 8 + epFile;
    } else {
        state.enPassantSquare = -1;
    }

    // 6. Rebuild Occupancies
    for (int p = WHITE_PAWN; p <= WHITE_KING; p++) occupancyBoards[WHITE] |= pieceBoards[p];
    for (int p = BLACK_PAWN; p <= BLACK_KING; p++) occupancyBoards[BLACK] |= pieceBoards[p];
    occupancyBoards[BOTH] = occupancyBoards[WHITE] | occupancyBoards[BLACK];
}

void Board::printBoard() const {
    std::cout << "\n  a b c d e f g h\n";
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << rank + 1 << " ";
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            char pieceChar = '.';
            for (int p = 0; p < 12; p++) {
                if (getBit(pieceBoards[p], square)) {
                    const char* chars = "PNBRQKpnbrqk";
                    pieceChar = chars[p];
                    break;
                }
            }
            std::cout << pieceChar << " ";
        }
        std::cout << rank + 1 << "\n";
    }
    std::cout << "  a b c d e f g h\n\n";
}

Bitboard Board::getOccupancy(Color c) const { return occupancyBoards[c]; }

void Board::makeMove(int from, int to, Piece pieceMoved, Piece pieceCaptured) {
    if (pieceMoved == WHITE_PAWN && to == state.enPassantSquare) {
        clearBit(pieceBoards[BLACK_PAWN], to - 8); 
    } else if (pieceMoved == BLACK_PAWN && to == state.enPassantSquare) {
        clearBit(pieceBoards[WHITE_PAWN], to + 8); 
    } else if (pieceCaptured != EMPTY) {
        clearBit(pieceBoards[pieceCaptured], to);
    }

    clearBit(pieceBoards[pieceMoved], from);
    setBit(pieceBoards[pieceMoved], to);

    if (pieceMoved == WHITE_KING) {
        if (from == E1 && to == G1) { clearBit(pieceBoards[WHITE_ROOK], H1); setBit(pieceBoards[WHITE_ROOK], F1); } 
        if (from == E1 && to == C1) { clearBit(pieceBoards[WHITE_ROOK], A1); setBit(pieceBoards[WHITE_ROOK], D1); } 
    } 
    else if (pieceMoved == BLACK_KING) {
        if (from == E8 && to == G8) { clearBit(pieceBoards[BLACK_ROOK], H8); setBit(pieceBoards[BLACK_ROOK], F8); } 
        if (from == E8 && to == C8) { clearBit(pieceBoards[BLACK_ROOK], A8); setBit(pieceBoards[BLACK_ROOK], D8); } 
    }

    if (pieceMoved == WHITE_KING) { state.whiteCanCastleKingside = false; state.whiteCanCastleQueenside = false; }
    if (pieceMoved == BLACK_KING) { state.blackCanCastleKingside = false; state.blackCanCastleQueenside = false; }
    
    if (from == A1 || to == A1) state.whiteCanCastleQueenside = false;
    if (from == H1 || to == H1) state.whiteCanCastleKingside = false;
    if (from == A8 || to == A8) state.blackCanCastleQueenside = false;
    if (from == H8 || to == H8) state.blackCanCastleKingside = false;

    state.enPassantSquare = -1; 
    if (pieceMoved == WHITE_PAWN && (to - from == 16)) state.enPassantSquare = from + 8;
    else if (pieceMoved == BLACK_PAWN && (from - to == 16)) state.enPassantSquare = from - 8;

    occupancyBoards[WHITE] = 0ULL;
    occupancyBoards[BLACK] = 0ULL;

    for (int i = WHITE_PAWN; i <= WHITE_KING; i++) occupancyBoards[WHITE] |= pieceBoards[i];
    for (int i = BLACK_PAWN; i <= BLACK_KING; i++) occupancyBoards[BLACK] |= pieceBoards[i];
    occupancyBoards[BOTH] = occupancyBoards[WHITE] | occupancyBoards[BLACK];

    sideToMove = (sideToMove == WHITE) ? BLACK : WHITE;
}