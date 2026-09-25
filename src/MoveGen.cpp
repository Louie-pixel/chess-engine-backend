#include "MoveGen.h"

using namespace Chess;

// --- KNIGHT MOVES ---
Bitboard MoveGen::getKnightAttacks(int square) {
    Bitboard knight = 1ULL << square;
    Bitboard attacks = 0ULL;

    attacks |= (knight << 15) & NOT_H_FILE;
    attacks |= (knight << 17) & NOT_A_FILE;
    attacks |= (knight >> 15) & NOT_A_FILE;
    attacks |= (knight >> 17) & NOT_H_FILE;
    attacks |= (knight << 6) & NOT_GH_FILE;
    attacks |= (knight >> 10) & NOT_GH_FILE;
    attacks |= (knight << 10) & NOT_AB_FILE;
    attacks |= (knight >> 6) & NOT_AB_FILE;

    return attacks;
}

bool MoveGen::isSquareAttacked(int sq, Color attacker, const Board& board) {
    Bitboard occ = board.getOccupancy(BOTH);
    
    if (getKnightAttacks(sq) & board.getPieceBoard(attacker == WHITE ? WHITE_KNIGHT : BLACK_KNIGHT)) return true;
    if (getKingAttacks(sq) & board.getPieceBoard(attacker == WHITE ? WHITE_KING : BLACK_KING)) return true;
    
    Bitboard p = 1ULL << sq;
    if (attacker == WHITE) {
        if (((p >> 7) & NOT_A_FILE) & board.getPieceBoard(WHITE_PAWN)) return true;
        if (((p >> 9) & NOT_H_FILE) & board.getPieceBoard(WHITE_PAWN)) return true;
    } else {
        if (((p << 7) & NOT_H_FILE) & board.getPieceBoard(BLACK_PAWN)) return true;
        if (((p << 9) & NOT_A_FILE) & board.getPieceBoard(BLACK_PAWN)) return true;
    }
    
    Bitboard diagonal = getBishopAttacks(sq, occ);
    if (diagonal & board.getPieceBoard(attacker == WHITE ? WHITE_BISHOP : BLACK_BISHOP)) return true;
    if (diagonal & board.getPieceBoard(attacker == WHITE ? WHITE_QUEEN : BLACK_QUEEN)) return true;
    
    Bitboard straight = getRookAttacks(sq, occ);
    if (straight & board.getPieceBoard(attacker == WHITE ? WHITE_ROOK : BLACK_ROOK)) return true;
    if (straight & board.getPieceBoard(attacker == WHITE ? WHITE_QUEEN : BLACK_QUEEN)) return true;
    
    return false;
}

// --- WHITE PAWN MOVES ---
Bitboard MoveGen::getWhitePawnPushes(const Board& board) {
    Bitboard pawns = board.getPieceBoard(WHITE_PAWN);
    Bitboard emptySquares = ~board.getOccupancy(BOTH);

    Bitboard singlePushes = (pawns << 8) & emptySquares;
    constexpr Bitboard RANK_4 = 0x00000000FF000000ULL;
    Bitboard doublePushes = (singlePushes << 8) & emptySquares & RANK_4;

    return singlePushes | doublePushes;
}

Bitboard MoveGen::getWhitePawnAttacks(const Board& board) {
    Bitboard pawns = board.getPieceBoard(WHITE_PAWN);
    Bitboard enemies = board.getOccupancy(BLACK);

    Bitboard leftAttacks = (pawns << 7) & NOT_H_FILE & enemies;
    Bitboard rightAttacks = (pawns << 9) & NOT_A_FILE & enemies;

    return leftAttacks | rightAttacks;
}

// --- BLACK PAWN MOVES ---
Bitboard MoveGen::getBlackPawnPushes(const Board& board) {
    Bitboard pawns = board.getPieceBoard(BLACK_PAWN);
    Bitboard emptySquares = ~board.getOccupancy(BOTH);

    Bitboard singlePushes = (pawns >> 8) & emptySquares;
    constexpr Bitboard RANK_5 = 0x000000FF00000000ULL;
    Bitboard doublePushes = (singlePushes >> 8) & emptySquares & RANK_5;

    return singlePushes | doublePushes;
}

Bitboard MoveGen::getBlackPawnAttacks(const Board& board) {
    Bitboard pawns = board.getPieceBoard(BLACK_PAWN);
    Bitboard enemies = board.getOccupancy(WHITE);

    Bitboard leftAttacks = (pawns >> 9) & NOT_H_FILE & enemies;
    Bitboard rightAttacks = (pawns >> 7) & NOT_A_FILE & enemies;

    return leftAttacks | rightAttacks;
}

// --- SLIDER MOVES (Bulletproof Ray-Casting) ---
Bitboard MoveGen::getRookAttacks(int square, Bitboard blockers) {
    Bitboard attacks = 0ULL;
    int r = square / 8, f = square % 8;
    
    for (int i = r + 1; i <= 7; i++) { attacks |= (1ULL << (i * 8 + f)); if (blockers & (1ULL << (i * 8 + f))) break; }
    for (int i = r - 1; i >= 0; i--) { attacks |= (1ULL << (i * 8 + f)); if (blockers & (1ULL << (i * 8 + f))) break; }
    for (int i = f + 1; i <= 7; i++) { attacks |= (1ULL << (r * 8 + i)); if (blockers & (1ULL << (r * 8 + i))) break; }
    for (int i = f - 1; i >= 0; i--) { attacks |= (1ULL << (r * 8 + i)); if (blockers & (1ULL << (r * 8 + i))) break; }
    
    return attacks;
}

Bitboard MoveGen::getBishopAttacks(int square, Bitboard blockers) {
    Bitboard attacks = 0ULL;
    int r = square / 8, f = square % 8;
    
    for (int i = r + 1, j = f + 1; i <= 7 && j <= 7; i++, j++) { attacks |= (1ULL << (i * 8 + j)); if (blockers & (1ULL << (i * 8 + j))) break; }
    for (int i = r - 1, j = f + 1; i >= 0 && j <= 7; i--, j++) { attacks |= (1ULL << (i * 8 + j)); if (blockers & (1ULL << (i * 8 + j))) break; }
    for (int i = r + 1, j = f - 1; i <= 7 && j >= 0; i++, j--) { attacks |= (1ULL << (i * 8 + j)); if (blockers & (1ULL << (i * 8 + j))) break; }
    for (int i = r - 1, j = f - 1; i >= 0 && j >= 0; i--, j--) { attacks |= (1ULL << (i * 8 + j)); if (blockers & (1ULL << (i * 8 + j))) break; }
    
    return attacks;
}

Bitboard MoveGen::getQueenAttacks(int square, Bitboard blockers) {
    return getRookAttacks(square, blockers) | getBishopAttacks(square, blockers);
}

// --- KING MOVES ---
Bitboard MoveGen::getKingAttacks(int square) {
    Bitboard king = 1ULL << square;
    Bitboard attacks = 0ULL;
    
    attacks |= (king << 8) | (king >> 8); 
    attacks |= ((king << 1) | (king >> 7) | (king << 9)) & NOT_A_FILE; 
    attacks |= ((king >> 1) | (king << 7) | (king >> 9)) & NOT_H_FILE; 
    
    return attacks;
}

void MoveGen::generateAllMoves(const Board& board, MoveList& moveList) {
    moveList.clear();
    Color us = board.getSideToMove();

    auto getCapturedPiece = [&](int toSquare, Color enemyColor) -> Piece {
        int start = (enemyColor == WHITE) ? WHITE_PAWN : BLACK_PAWN;
        int end = (enemyColor == WHITE) ? WHITE_KING : BLACK_KING;
        for (int p = start; p <= end; p++) {
            if (board.getBit(board.getPieceBoard(static_cast<Piece>(p)), toSquare)) {
                return static_cast<Piece>(p);
            }
        }
        return EMPTY;
    };

    Bitboard emptySquares = ~board.getOccupancy(BOTH);

    if (us == WHITE) {
        Bitboard enemies = board.getOccupancy(BLACK);
        Bitboard whitePieces = board.getOccupancy(WHITE);

        Bitboard pawns = board.getPieceBoard(WHITE_PAWN);
        Bitboard singlePushes = (pawns << 8) & emptySquares;
        for (int to = 0; to < 64; to++) {
            if (singlePushes & (1ULL << to)) moveList.addMove({to - 8, to, WHITE_PAWN, EMPTY, EMPTY, false, false});
        }

        constexpr Bitboard RANK_4 = 0x00000000FF000000ULL;
        Bitboard doublePushes = (singlePushes << 8) & emptySquares & RANK_4;
        for (int to = 0; to < 64; to++) {
            if (doublePushes & (1ULL << to)) moveList.addMove({to - 16, to, WHITE_PAWN, EMPTY, EMPTY, false, false});
        }

        Bitboard leftAttacks = (pawns << 7) & NOT_H_FILE & enemies;
        for (int to = 0; to < 64; to++) {
            if (leftAttacks & (1ULL << to)) moveList.addMove({to - 7, to, WHITE_PAWN, getCapturedPiece(to, BLACK), EMPTY, false, false});
        }

        Bitboard rightAttacks = (pawns << 9) & NOT_A_FILE & enemies;
        for (int to = 0; to < 64; to++) {
            if (rightAttacks & (1ULL << to)) moveList.addMove({to - 9, to, WHITE_PAWN, getCapturedPiece(to, BLACK), EMPTY, false, false});
        }

        int ep = board.getState().enPassantSquare;
        if (ep != -1) {
            if ((pawns << 7) & NOT_H_FILE & (1ULL << ep)) moveList.addMove({ep - 7, ep, WHITE_PAWN, BLACK_PAWN, EMPTY, true, false});
            if ((pawns << 9) & NOT_A_FILE & (1ULL << ep)) moveList.addMove({ep - 9, ep, WHITE_PAWN, BLACK_PAWN, EMPTY, true, false});
        }

        Bitboard knights = board.getPieceBoard(WHITE_KNIGHT);
        for (int from = 0; from < 64; from++) {
            if (knights & (1ULL << from)) {
                Bitboard attacks = getKnightAttacks(from) & ~whitePieces;
                for (int to = 0; to < 64; to++) {
                    if (attacks & (1ULL << to)) moveList.addMove({from, to, WHITE_KNIGHT, getCapturedPiece(to, BLACK), EMPTY, false, false});
                }
            }
        }

        Bitboard bishops = board.getPieceBoard(WHITE_BISHOP);
        for (int from = 0; from < 64; from++) {
            if (bishops & (1ULL << from)) {
                Bitboard attacks = getBishopAttacks(from, board.getOccupancy(BOTH)) & ~whitePieces;
                for (int to = 0; to < 64; to++) {
                    if (attacks & (1ULL << to)) moveList.addMove({from, to, WHITE_BISHOP, getCapturedPiece(to, BLACK), EMPTY, false, false});
                }
            }
        }

        Bitboard rooks = board.getPieceBoard(WHITE_ROOK);
        for (int from = 0; from < 64; from++) {
            if (rooks & (1ULL << from)) {
                Bitboard attacks = getRookAttacks(from, board.getOccupancy(BOTH)) & ~whitePieces;
                for (int to = 0; to < 64; to++) {
                    if (attacks & (1ULL << to)) moveList.addMove({from, to, WHITE_ROOK, getCapturedPiece(to, BLACK), EMPTY, false, false});
                }
            }
        }

        Bitboard queens = board.getPieceBoard(WHITE_QUEEN);
        for (int from = 0; from < 64; from++) {
            if (queens & (1ULL << from)) {
                Bitboard attacks = getQueenAttacks(from, board.getOccupancy(BOTH)) & ~whitePieces;
                for (int to = 0; to < 64; to++) {
                    if (attacks & (1ULL << to)) moveList.addMove({from, to, WHITE_QUEEN, getCapturedPiece(to, BLACK), EMPTY, false, false});
                }
            }
        }

        Bitboard king = board.getPieceBoard(WHITE_KING);
        for (int from = 0; from < 64; from++) {
            if (king & (1ULL << from)) {
                Bitboard attacks = getKingAttacks(from) & ~whitePieces;
                for (int to = 0; to < 64; to++) {
                    if (attacks & (1ULL << to)) moveList.addMove({from, to, WHITE_KING, getCapturedPiece(to, BLACK), EMPTY, false, false});
                }
            }
        }

        if (board.getState().whiteCanCastleKingside) {
            if (board.getBit(emptySquares, 5) && board.getBit(emptySquares, 6)) { 
                if (!isSquareAttacked(4, BLACK, board) && !isSquareAttacked(5, BLACK, board) && !isSquareAttacked(6, BLACK, board)) {
                    moveList.addMove({4, 6, WHITE_KING, EMPTY, EMPTY, false, true});
                }
            }
        }
        if (board.getState().whiteCanCastleQueenside) {
            if (board.getBit(emptySquares, 1) && board.getBit(emptySquares, 2) && board.getBit(emptySquares, 3)) { 
                if (!isSquareAttacked(4, BLACK, board) && !isSquareAttacked(3, BLACK, board) && !isSquareAttacked(2, BLACK, board)) {
                    moveList.addMove({4, 2, WHITE_KING, EMPTY, EMPTY, false, true});
                }
            }
        }
    } 
    else {
        Bitboard enemies = board.getOccupancy(WHITE);
        Bitboard blackPieces = board.getOccupancy(BLACK);

        Bitboard pawns = board.getPieceBoard(BLACK_PAWN);
        Bitboard singlePushes = (pawns >> 8) & emptySquares;
        for (int to = 0; to < 64; to++) {
            if (singlePushes & (1ULL << to)) moveList.addMove({to + 8, to, BLACK_PAWN, EMPTY, EMPTY, false, false});
        }

        constexpr Bitboard RANK_5 = 0x000000FF00000000ULL;
        Bitboard doublePushes = (singlePushes >> 8) & emptySquares & RANK_5;
        for (int to = 0; to < 64; to++) {
            if (doublePushes & (1ULL << to)) moveList.addMove({to + 16, to, BLACK_PAWN, EMPTY, EMPTY, false, false});
        }

        Bitboard leftAttacks = (pawns >> 9) & NOT_H_FILE & enemies;
        for (int to = 0; to < 64; to++) {
            if (leftAttacks & (1ULL << to)) moveList.addMove({to + 9, to, BLACK_PAWN, getCapturedPiece(to, WHITE), EMPTY, false, false});
        }

        Bitboard rightAttacks = (pawns >> 7) & NOT_A_FILE & enemies;
        for (int to = 0; to < 64; to++) {
            if (rightAttacks & (1ULL << to)) moveList.addMove({to + 7, to, BLACK_PAWN, getCapturedPiece(to, WHITE), EMPTY, false, false});
        }

        int ep = board.getState().enPassantSquare;
        if (ep != -1) {
            if ((pawns >> 9) & NOT_H_FILE & (1ULL << ep)) moveList.addMove({ep + 9, ep, BLACK_PAWN, WHITE_PAWN, EMPTY, true, false});
            if ((pawns >> 7) & NOT_A_FILE & (1ULL << ep)) moveList.addMove({ep + 7, ep, BLACK_PAWN, WHITE_PAWN, EMPTY, true, false});
        }

        Bitboard knights = board.getPieceBoard(BLACK_KNIGHT);
        for (int from = 0; from < 64; from++) {
            if (knights & (1ULL << from)) {
                Bitboard attacks = getKnightAttacks(from) & ~blackPieces;
                for (int to = 0; to < 64; to++) {
                    if (attacks & (1ULL << to)) moveList.addMove({from, to, BLACK_KNIGHT, getCapturedPiece(to, WHITE), EMPTY, false, false});
                }
            }
        }

        Bitboard bishops = board.getPieceBoard(BLACK_BISHOP);
        for (int from = 0; from < 64; from++) {
            if (bishops & (1ULL << from)) {
                Bitboard attacks = getBishopAttacks(from, board.getOccupancy(BOTH)) & ~blackPieces;
                for (int to = 0; to < 64; to++) {
                    if (attacks & (1ULL << to)) moveList.addMove({from, to, BLACK_BISHOP, getCapturedPiece(to, WHITE), EMPTY, false, false});
                }
            }
        }

        Bitboard rooks = board.getPieceBoard(BLACK_ROOK);
        for (int from = 0; from < 64; from++) {
            if (rooks & (1ULL << from)) {
                Bitboard attacks = getRookAttacks(from, board.getOccupancy(BOTH)) & ~blackPieces;
                for (int to = 0; to < 64; to++) {
                    if (attacks & (1ULL << to)) moveList.addMove({from, to, BLACK_ROOK, getCapturedPiece(to, WHITE), EMPTY, false, false});
                }
            }
        }

        Bitboard queens = board.getPieceBoard(BLACK_QUEEN);
        for (int from = 0; from < 64; from++) {
            if (queens & (1ULL << from)) {
                Bitboard attacks = getQueenAttacks(from, board.getOccupancy(BOTH)) & ~blackPieces;
                for (int to = 0; to < 64; to++) {
                    if (attacks & (1ULL << to)) moveList.addMove({from, to, BLACK_QUEEN, getCapturedPiece(to, WHITE), EMPTY, false, false});
                }
            }
        }

        Bitboard king = board.getPieceBoard(BLACK_KING);
        for (int from = 0; from < 64; from++) {
            if (king & (1ULL << from)) {
                Bitboard attacks = getKingAttacks(from) & ~blackPieces;
                for (int to = 0; to < 64; to++) {
                    if (attacks & (1ULL << to)) moveList.addMove({from, to, BLACK_KING, getCapturedPiece(to, WHITE), EMPTY, false, false});
                }
            }
        }

        if (board.getState().blackCanCastleKingside) {
            if (board.getBit(emptySquares, 61) && board.getBit(emptySquares, 62)) { 
                if (!isSquareAttacked(60, WHITE, board) && !isSquareAttacked(61, WHITE, board) && !isSquareAttacked(62, WHITE, board)) {
                    moveList.addMove({60, 62, BLACK_KING, EMPTY, EMPTY, false, true});
                }
            }
        }
        if (board.getState().blackCanCastleQueenside) {
            if (board.getBit(emptySquares, 57) && board.getBit(emptySquares, 58) && board.getBit(emptySquares, 59)) { 
                if (!isSquareAttacked(60, WHITE, board) && !isSquareAttacked(59, WHITE, board) && !isSquareAttacked(58, WHITE, board)) {
                    moveList.addMove({60, 58, BLACK_KING, EMPTY, EMPTY, false, true});
                }
            }
        }
    }
}