#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <cstring>
#include "Board.h"
#include "MoveGen.h"
#include "Search.h"
#include "Transposition.h"
#include "OpeningBook.h"
#include "Types.h"
#include "UCI.h"
#include "Magics.h" // Includes the Magic Engine Initialization

using namespace Chess;

// --- Helper to decode URL characters from Web UI ---
std::string urlDecode(const std::string& src) {
    std::string ret;
    for (size_t i = 0; i < src.length(); i++) {
        if (src[i] == '%' && i + 2 < src.length()) {
            int hex = std::stoi(src.substr(i + 1, 2), nullptr, 16);
            ret += static_cast<char>(hex);
            i += 2;
        } else if (src[i] == '+') {
            ret += ' ';
        } else {
            ret += src[i];
        }
    }
    return ret;
}

int main(int argc, char* argv[]) {
    srand(time(0));
    
    // --- STARTUP INITIALIZATIONS ---
    Magics::initialize(); // Build the Magic Hash Arrays!
    TranspositionTable::initZobrist();
    TranspositionTable::clear();
    OpeningBook::initialize();

    // --- 1. DUAL-BOOT LOGIC (UCI MODE) ---
    if (argc > 1 && std::strcmp(argv[1], "uci") == 0) {
        UCI::loop();
        return 0; 
    }

    // --- 2. HTTP WEB SERVER MODE ---
    Board board;
    MoveList list;
    std::vector<Board> gameHistory;

    std::cout << "--- STARTING HTTP WEB SERVER ---" << std::endl;
    board.initialize(); 
    
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (serverSocket < 0) {
        std::cerr << "Error: Could not create socket." << std::endl;
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error: Could not bind to port 8080." << std::endl;
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Error: Listen failed." << std::endl;
        close(serverSocket);
        return 1;
    }
    std::cout << "Server successfully booted and listening on port 8080...\n" << std::endl;
        
    while (true) {
        int clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket < 0) continue; 

        char buffer[1024] = {0};
        read(clientSocket, buffer, 1024);
        
        std::string request(buffer);
        size_t firstSpace = request.find(' ');
        size_t secondSpace = request.find(' ', firstSpace + 1);
        
        if (firstSpace == std::string::npos || secondSpace == std::string::npos) {
            close(clientSocket);
            continue;
        }

        std::string path = request.substr(firstSpace + 1, secondSpace - firstSpace - 1);
        std::string response;
        
        // --- ROUTER ---
        if (path == "/") {
            response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>C++ Engine API</h1><p>Status: Online</p></body></html>";
        } 
        else if (path.find("/restart") == 0) {
            board.initialize();
            gameHistory.clear();
            TranspositionTable::clear(); 
            response = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n\r\n{\"status\": \"success\"}";
        }
        else if (path.find("/setfen") == 0) {
            size_t paramPos = path.find("?fen=");
            if (paramPos != std::string::npos) {
                std::string rawFen = path.substr(paramPos + 5);
                std::string decodedFen = urlDecode(rawFen);
                board.loadFEN(decodedFen);
                gameHistory.clear();
                TranspositionTable::clear();
                response = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n\r\n{\"status\": \"success\"}";
            }
        }
        else if (path.find("/undo") == 0) {
            if (!gameHistory.empty()) {
                board = gameHistory.back();
                gameHistory.pop_back();
                response = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n\r\n{\"status\": \"success\"}";
            } else {
                response = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n\r\n{\"status\": \"error\"}";
            }
        }
        else if (path.find("/move") == 0) {
            size_t paramPos = path.find("?player=");
            if (paramPos != std::string::npos) {
                gameHistory.push_back(board);

                std::string pMove = path.substr(paramPos + 8, 4); 
                int fromSq = (pMove[1] - '1') * 8 + (pMove[0] - 'a');
                int toSq   = (pMove[3] - '1') * 8 + (pMove[2] - 'a');
                
                Piece pMoved = EMPTY, pCaptured = EMPTY;
                for (int p = WHITE_PAWN; p <= BLACK_KING; p++) {
                    if (board.getBit(board.getPieceBoard(static_cast<Piece>(p)), fromSq)) pMoved = static_cast<Piece>(p);
                    if (board.getBit(board.getPieceBoard(static_cast<Piece>(p)), toSq)) pCaptured = static_cast<Piece>(p);
                }
                
                if (pMoved == WHITE_PAWN && toSq == board.getState().enPassantSquare) pCaptured = BLACK_PAWN;
                if (pMoved == BLACK_PAWN && toSq == board.getState().enPassantSquare) pCaptured = WHITE_PAWN;
                
                if (pMoved != EMPTY) {
                    board.makeMove(fromSq, toSq, pMoved, pCaptured);
                    std::cout << "\n>>> HUMAN PLAYED: " << pMove << " <<<" << std::endl;
                }
            }

            MoveList currentMoves;
            MoveGen::generateAllMoves(board, currentMoves);
            
            if (currentMoves.count == 0) {
                response = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n\r\n{\"status\": \"game_over\"}";
            } else {
                Move selectedMove = Search::getBestMove(board, 1000); 
                board.makeMove(selectedMove.fromSquare, selectedMove.toSquare, selectedMove.pieceMoved, selectedMove.pieceCaptured);
                
                auto squareToAlgebraic = [](int sq) -> std::string {
                    return std::string() + (char)('a' + (sq % 8)) + (char)('1' + (sq / 8));
                };
                std::string moveString = squareToAlgebraic(selectedMove.fromSquare) + squareToAlgebraic(selectedMove.toSquare);
                
                response = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n\r\n{\"status\": \"success\", \"move\": \"" + moveString + "\"}";
                
                std::cout << ">>> ENGINE PLAYED: " << moveString << " <<<" << std::endl;
                board.printBoard();
            }
        } 
        else {
            response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n404 - Invalid Route";
        }

        write(clientSocket, response.c_str(), response.length());
        close(clientSocket); 
    }

    close(serverSocket);
    return 0; 
}