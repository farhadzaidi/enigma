#pragma once

#include <string>
#include <bit>

#include "types.hpp"
#include "board.hpp"

// Parsing

constexpr Square get_square(int rank, int file) { return rank * BOARD_SIZE + file; }
constexpr int get_rank(Square square) { return square / BOARD_SIZE;}
constexpr int get_file(Square square) { return square % BOARD_SIZE;}

Square uci_to_index(const std::string& square);
std::string index_to_uci(Square square);
Move encode_move_from_uci(const Board& b, const std::string& uci_move);
std::string decode_move_to_uci(Move move);

// Bitboards

static constexpr Bitboard NOT_A_FILE = ~A_FILE_MASK;
static constexpr Bitboard NOT_H_FILE = ~H_FILE_MASK;

template <Direction D>
constexpr Bitboard shift(Bitboard b) {
    switch(D) {
        case NORTH:         return b << 8;
        case SOUTH:         return b >> 8;
        case NORTH_NORTH:   return b << 16;
        case SOUTH_SOUTH:   return b >> 16;
        case EAST:          return (b << 1) & NOT_A_FILE;
        case WEST:          return (b >> 1) & NOT_H_FILE;
        case NORTHEAST:     return (b << 9) & NOT_A_FILE;
        case NORTHWEST:     return (b << 7) & NOT_H_FILE;
        case SOUTHEAST:     return (b >> 7) & NOT_A_FILE;
        case SOUTHWEST:     return (b >> 9) & NOT_H_FILE;
    }
}

constexpr Bitboard get_mask(Square square) { return uint64_t{1} << square;}

inline Square pop_lsb(Bitboard& b) {
    Square sq = std::countr_zero(b);
    b &= b - 1; // pop
    return sq;
}

