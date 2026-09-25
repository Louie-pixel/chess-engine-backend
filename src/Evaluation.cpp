#include "Evaluation.h"

using namespace Chess;

// --- PIECE-SQUARE TABLES (White's Perspective) ---
// Index 0 is A1, Index 63 is H8.

const int pawnPST[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     5, 10, 10,-20,-20, 10, 10,  5,
     5, -5,-10,  0,  0,-10, -5,  5,
     0,  0,  0, 20, 20,  0,  0,  0,
     5,  5, 10, 25, 25, 10,  5,  5,
    10, 10, 20, 30, 30, 20, 10, 10,
    50, 50, 50, 50, 50, 50, 50, 50,
     0,  0,  0,  0,  0,  0,  0,  0
};

const int knightPST[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

const int bishopPST[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

// --- HELPER FUNCTION: BIT EXTRACTOR ---
// Scores a specific piece type by popping bits one by one
int scorePieces(Bitboard b, int pieceValue, const int* pst, Color color) {
    int score = 0;
    while (b) {
        // 1. Find the index of the first '1' bit
        int square = __builtin_ctzll(b);
        
        // 2. Add the base material value
        score += pieceValue;
        
        // 3. Add the positional PST bonus (Flip the board if Black)
        if (pst != nullptr) {
            int pstIndex = (color == WHITE) ? square : (square ^ 56);
            score += pst[pstIndex];
        }
        
        // 4. Erase that '1' bit and repeat until the bitboard is empty
        b &= b - 1;
    }
    return score;
}

// --- MAIN EVALUATION ---
int Evaluator::evaluate(const Board& board) {
    int whiteScore = 0;
    int blackScore = 0;

    // --- WHITE SCORING ---
    whiteScore += scorePieces(board.getPieceBoard(WHITE_PAWN), PAWN_VALUE, pawnPST, WHITE);
    whiteScore += scorePieces(board.getPieceBoard(WHITE_KNIGHT), KNIGHT_VALUE, knightPST, WHITE);
    whiteScore += scorePieces(board.getPieceBoard(WHITE_BISHOP), BISHOP_VALUE, bishopPST, WHITE);
    whiteScore += scorePieces(board.getPieceBoard(WHITE_ROOK), ROOK_VALUE, nullptr, WHITE);
    whiteScore += scorePieces(board.getPieceBoard(WHITE_QUEEN), QUEEN_VALUE, nullptr, WHITE);

    // --- BLACK SCORING ---
    blackScore += scorePieces(board.getPieceBoard(BLACK_PAWN), PAWN_VALUE, pawnPST, BLACK);
    blackScore += scorePieces(board.getPieceBoard(BLACK_KNIGHT), KNIGHT_VALUE, knightPST, BLACK);
    blackScore += scorePieces(board.getPieceBoard(BLACK_BISHOP), BISHOP_VALUE, bishopPST, BLACK);
    blackScore += scorePieces(board.getPieceBoard(BLACK_ROOK), ROOK_VALUE, nullptr, BLACK);
    blackScore += scorePieces(board.getPieceBoard(BLACK_QUEEN), QUEEN_VALUE, nullptr, BLACK);

    // Absolute difference
    int finalScore = whiteScore - blackScore;

    // Negamax perspective flip
    if (board.getSideToMove() == BLACK) {
        return -finalScore;
    }

    return finalScore;
}