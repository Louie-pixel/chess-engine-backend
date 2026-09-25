cmake_minimum_required(VERSION 3.10)
project(ChessEngine)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")

set(CMAKE_CXX_STANDARD 17)

# Tell CMake where the .h files are
include_directories(include)

# List all your .cpp files here
add_executable(chess_server 
src/main.cpp 
    src/Board.cpp 
    src/MoveGen.cpp 
    src/Evaluation.cpp 
    src/Search.cpp 
    src/Transposition.cpp 
    src/OpeningBook.cpp
    src/UCI.cpp
    src/Magics.cpp
)