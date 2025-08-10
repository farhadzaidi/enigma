#include <iostream>
#include <cstring>

#include "constants.hpp"
#include "board.hpp"

inline void print_bitboard(uint64_t bitboard) {
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file <= 7; file++) {
            int square = rank * 8 + file;
            std::clog << (bitboard & (uint64_t{1} << square) ? "1" : "0");
        }
        std::clog << "\n";
    }
}

inline void print_chessboard(Board &b) {
    
}


