# C++ High-Performance Chess Engine

A tournament-ready, high-performance chess engine built completely from scratch in modern C++.

This project was engineered to explore low-level hardware optimization, advanced search algorithms, and dual-boot application architecture. It features a custom Bitboard physics engine, a Minimax AI with Alpha-Beta pruning, and seamlessly dual-boots into either a local HTTP Web API or the Universal Chess Interface (UCI) protocol for professional tournament play.

## 🧠 Core Architecture

### 1. Board Representation: Bitboards
The board state is managed using **64-bit integers (Bitboards)**. Instead of using a 2D array, the location of every piece is mapped to the bits of a `uint64_t`.
* **Bitwise Physics:** Move generation is calculated using extremely fast bitwise operators (`AND`, `OR`, `XOR`, `Bitshifts`).
* **Bulletproof Sliders:** Rook, Bishop, and Queen moves are calculated using highly optimized ray-casting loops to guarantee mathematically flawless move generation, avoiding hash-collision vulnerabilities.

### 2. The Search Algorithm
The engine's "brain" is built on the **Negamax** framework, heavily optimized to traverse millions of nodes per second:
* **Alpha-Beta Pruning:** Eliminates branches of the search tree that are mathematically proven to be worse than previously evaluated lines, massively increasing search depth.
* **Quiescence Search:** Prevents the "Horizon Effect" by continuing to search volatile positions (captures) beyond the hard depth limit until a quiet position is reached.
* **Iterative Deepening & Time Management:** The engine searches depth 1, then depth 2, etc., dynamically interrupting the search tree and returning the best safe move exactly when the allocated time limit (e.g., 1000ms) expires.

### 3. Memory & Optimization
* **Transposition Tables (Zobrist Hashing):** Every board state is assigned a unique 64-bit Zobrist Hash. Evaluated nodes are stored in a massive Hash Map. If the engine encounters the same board state via a different move order, it instantly retrieves the cached score in `O(1)` time.
* **Grandmaster Opening Book:** The engine checks its hash against a pre-compiled library of famous chess openings (Sicilian Defense, Queen's Gambit, etc.), allowing it to play the first several moves instantly with theoretical perfection.

### 4. Advanced Evaluation Heuristics
The evaluation function goes far beyond simple material counting:
* **Piece-Square Tables (PSTs):** Heavily incentivizes positional play (e.g., centralizing knights, tucking kings into corners behind pawns).
* **Pawn Structure Analysis:** Mathematically penalizes doubled and isolated pawns while rewarding rooks placed on open files.

## 🚀 Dual-Boot Architecture

The engine is built to run in two completely distinct environments:

### Mode A: Full-Stack Web App (HTTP Server)
Running the executable directly boots a custom **C++ TCP/IP Socket Server** on `localhost:8080`. The repository includes a modern, responsive HTML/JS/CSS frontend dashboard.
* Live move validation and algebraic notation history.
* FEN String parsing for loading custom puzzles directly into the engine.
* Material advantage tracking and Checkmate detection.

### Mode B: Tournament Protocol (UCI)
Running the executable with the `uci` flag strips away the web server and boots the engine as a standard terminal process communicating via standard input/output (`stdin`/`stdout`).
* Fully compliant with the Universal Chess Interface (UCI).
* Plugs directly into professional GUIs like CuteChess, Arena, or ChessBase.
* Ready to be deployed online as an automated bot on Lichess.org.

## 🛠️ Build & Installation

This project uses CMake for cross-platform compilation.

**Prerequisites:**
* C++ Compiler (GCC, Clang, or MSVC)
* CMake (v3.10+)

**Build Steps:**
Clone the repository:
```bash
git clone https://github.com/yourusername/cpp-chess-engine.git 
cd cpp-chess-engine
```

Create build directory and compile:
```bash
mkdir build 
cd build 
cmake .. 
cmake --build .
```

## 🎮 Usage

### Playing via the Web UI
1. Run the compiled executable:
   ```bash
   ./chess_server
   ```
2. Open the `index.html` file in any modern web browser.
3. Make your first move as White.

### Playing via a UCI GUI (e.g., CuteChess)
1. Install a UCI-compatible GUI like [CuteChess](https://cutechess.com/).
2. Add a new engine in the GUI settings.
3. Point the executable path to your compiled `chess_server`.
4. **Crucial:** Add `uci` to the command-line arguments in the GUI settings.
5. Start a CPU vs. Human game!

## 📁 Directory Structure
```text
├── include/           # C++ Header files (.h)
│   ├── Board.h        # Bitboard state & physics
│   ├── Evaluation.h   # Heuristics & PSTs
│   ├── Magics.h       # Magic Bitboard configurations
│   ├── MoveGen.h      # Legal move generation
│   ├── OpeningBook.h  # Opening book logic
│   ├── Search.h       # Minimax & Alpha-Beta algorithm
│   ├── Transposition.h# Zobrist Hashing memory
│   ├── Types.h        # Enums and global types
│   └── UCI.h          # Universal Chess Interface protocol
├── src/               # C++ Source files (.cpp)
│   ├── Board.cpp
│   ├── Evaluation.cpp
│   ├── Magics.cpp
│   ├── main.cpp       # Entry point & HTTP Server Router
│   ├── MoveGen.cpp
│   ├── OpeningBook.cpp
│   ├── Search.cpp
│   ├── Transposition.cpp
│   └── UCI.cpp
├── index.html         # Frontend Dashboard UI
└── CMakeLists.txt     # Build configuration
