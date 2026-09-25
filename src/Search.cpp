#include "Search.h"
#include "Transposition.h"
#include "OpeningBook.h" 

using namespace Chess;

std::chrono::time_point<std::chrono::steady_clock> Search::startTime;
int Search::timeLimitMs = 0;
bool Search::abortFlag = false;
int Search::nodesSearched = 0;

constexpr int UNKNOWN_SCORE = -1000000;

void Search::checkTime() {
    if ((nodesSearched & 2047) == 0) {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
        if (elapsed >= timeLimitMs) {
            abortFlag = true;
        }
    }
}

void Search::orderMoves(MoveList& list) {
    for (int i = 0; i < list.count - 1; i++) {
        for (int j = i + 1; j < list.count; j++) {
            if (list.moves[j].pieceCaptured != EMPTY && list.moves[i].pieceCaptured == EMPTY) {
                Move temp = list.moves[i];
                list.moves[i] = list.moves[j];
                list.moves[j] = temp;
            }
        }
    }
}

int Search::quiescence(Board board, int alpha, int beta) {
    nodesSearched++;
    checkTime();
    if (abortFlag) return 0;

    int stand_pat = Evaluator::evaluate(board);
    if (stand_pat >= beta) return beta;
    if (alpha < stand_pat) alpha = stand_pat;

    MoveList list;
    MoveGen::generateAllMoves(board, list);
    orderMoves(list);

    Color us = board.getSideToMove();
    Color enemy = (us == WHITE) ? BLACK : WHITE;

    for (int i = 0; i < list.count; i++) {
        Move m = list.moves[i];
        if (m.pieceCaptured == EMPTY) continue;

        Board copy = board;
        copy.makeMove(m.fromSquare, m.toSquare, m.pieceMoved, m.pieceCaptured);

        Bitboard ourKing = copy.getPieceBoard(us == WHITE ? WHITE_KING : BLACK_KING);
        if (ourKing == 0) continue;
        int kingSq = __builtin_ctzll(ourKing);
        if (MoveGen::isSquareAttacked(kingSq, enemy, copy)) continue;

        int score = -quiescence(copy, -beta, -alpha);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

int Search::negamax(Board board, int depth, int alpha, int beta) {
    nodesSearched++;
    checkTime();
    if (abortFlag) return 0; 

    uint64_t boardHash = TranspositionTable::generateHash(board);
    int cachedScore = TranspositionTable::readHash(boardHash, depth, alpha, beta);
    if (cachedScore != UNKNOWN_SCORE) return cachedScore;

    if (depth == 0) return quiescence(board, alpha, beta);

    MoveList list;
    MoveGen::generateAllMoves(board, list);
    orderMoves(list);
    
    Color us = board.getSideToMove();
    Color enemy = (us == WHITE) ? BLACK : WHITE;

    int maxScore = -100000;
    int originalAlpha = alpha;
    bool hasLegalMoves = false;

    for (int i = 0; i < list.count; i++) {
        Board copy = board; 
        Move m = list.moves[i];
        
        copy.makeMove(m.fromSquare, m.toSquare, m.pieceMoved, m.pieceCaptured);

        Bitboard ourKing = copy.getPieceBoard(us == WHITE ? WHITE_KING : BLACK_KING);
        if (ourKing == 0) continue; 
        int kingSq = __builtin_ctzll(ourKing);
        if (MoveGen::isSquareAttacked(kingSq, enemy, copy)) continue;

        hasLegalMoves = true;

        int score = -negamax(copy, depth - 1, -beta, -alpha);

        if (score > maxScore) maxScore = score;
        if (maxScore > alpha) alpha = maxScore;
        if (alpha >= beta) break; 
    }

    if (!hasLegalMoves) {
        Bitboard ourKing = board.getPieceBoard(us == WHITE ? WHITE_KING : BLACK_KING);
        if (ourKing == 0) return 0; // CRITICAL FIX: Failsafe against undefined behavior segfault
        int kingSq = __builtin_ctzll(ourKing);
        if (MoveGen::isSquareAttacked(kingSq, enemy, board)) {
            return -50000 - depth; 
        } else {
            return 0; 
        }
    }

    TTFlag flag;
    if (maxScore <= originalAlpha) flag = HASH_ALPHA;
    else if (maxScore >= beta) flag = HASH_BETA;
    else flag = HASH_EXACT;
    
    TranspositionTable::writeHash(boardHash, depth, maxScore, flag);

    return maxScore;
}

Move Search::getBestMove(Board board, int allocatedTimeMs) {
    startTime = std::chrono::steady_clock::now();
    timeLimitMs = allocatedTimeMs;
    abortFlag = false;
    nodesSearched = 0;

    MoveList list;
    MoveGen::generateAllMoves(board, list);
    orderMoves(list);

    uint64_t currentHash = TranspositionTable::generateHash(board);
    if (OpeningBook::hasMove(currentHash)) {
        std::string bookMoveStr = OpeningBook::getMove(currentHash);
        
        int fromSq = (bookMoveStr[1] - '1') * 8 + (bookMoveStr[0] - 'a');
        int toSq   = (bookMoveStr[3] - '1') * 8 + (bookMoveStr[2] - 'a');
        
        for (int i = 0; i < list.count; i++) {
            if (list.moves[i].fromSquare == fromSq && list.moves[i].toSquare == toSq) {
                // Sent as a valid UCI info string instead of illegal raw text
                std::cout << "info string Book move found" << std::endl;
                return list.moves[i];
            }
        }
    }

    Move overallBestMove = list.moves[0]; 
    Color us = board.getSideToMove();
    Color enemy = (us == WHITE) ? BLACK : WHITE;

    int currentDepth = 1;
    constexpr int MAX_ALLOWED_DEPTH = 64;

    auto squareToAlg = [](int sq) -> std::string {
        return std::string() + (char)('a' + (sq % 8)) + (char)('1' + (sq / 8));
    };

    while (currentDepth <= MAX_ALLOWED_DEPTH) {
        Move bestMoveThisDepth = overallBestMove;
        int maxScoreThisDepth = -100000;
        int alpha = -100000;
        int beta = 100000;
        bool foundValidMoveThisDepth = false;

        for (int i = 0; i < list.count; i++) {
            Board copy = board;
            Move m = list.moves[i];
            
            copy.makeMove(m.fromSquare, m.toSquare, m.pieceMoved, m.pieceCaptured);

            Bitboard ourKing = copy.getPieceBoard(us == WHITE ? WHITE_KING : BLACK_KING);
            if (ourKing == 0) continue; 
            int kingSq = __builtin_ctzll(ourKing);
            if (MoveGen::isSquareAttacked(kingSq, enemy, copy)) continue;

            foundValidMoveThisDepth = true;
            int score = -negamax(copy, currentDepth - 1, -beta, -alpha);

            if (abortFlag) break;

            if (score > maxScoreThisDepth) {
                maxScoreThisDepth = score;
                bestMoveThisDepth = m;
            }
        }

        if (abortFlag) break; 

        if (foundValidMoveThisDepth) {
            overallBestMove = bestMoveThisDepth;
            // CRITICAL FIX: Standard UCI output stream. CuteChess will use this to populate its Evaluation Table.
            std::cout << "info depth " << currentDepth 
                      << " score cp " << maxScoreThisDepth 
                      << " nodes " << nodesSearched 
                      << " pv " << squareToAlg(overallBestMove.fromSquare) << squareToAlg(overallBestMove.toSquare) 
                      << std::endl;
        }

        currentDepth++;
    }

    return overallBestMove; 
}