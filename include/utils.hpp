#pragma once

#include <string>
#include <cstdint>
#include <bit>

#include "types.hpp"
#include "board.hpp"

inline constexpr Bitboard NOT_A_FILE = uint64_t{0xfefefefefefefefe};
inline constexpr Bitboard NOT_H_FILE = uint64_t{0x7f7f7f7f7f7f7f7f};

// Parsing
Square get_square(int rank, int file);
Square uci_to_index(const std::string& square);
std::string index_to_uci(Square square);
Move encode_move_from_uci(const Board& b, const std::string& uci_move);
std::string decode_move_to_uci(Move move);

// Shifting bitboards
constexpr Bitboard shift_north(Bitboard bb) {return bb << 8;}
constexpr Bitboard shift_south(Bitboard bb) {return bb >> 8;}
constexpr Bitboard shift_east(Bitboard bb) {return (bb << 1) & NOT_A_FILE;}
constexpr Bitboard shift_northeast(Bitboard bb) {return (bb << 9) & NOT_A_FILE;}
constexpr Bitboard shift_southeast(Bitboard bb) {return (bb >> 7) & NOT_A_FILE;}
constexpr Bitboard shift_west(Bitboard bb) {return (bb >> 1) & NOT_H_FILE;}
constexpr Bitboard shift_northwest(Bitboard bb) {return (bb << 7) & NOT_H_FILE;}
constexpr Bitboard shift_southwest(Bitboard bb) {return (bb >> 9) & NOT_H_FILE;}

constexpr Bitboard get_mask(Square square) { return uint64_t{1} << square;}
constexpr int get_rank(Square square) { return square / BOARD_SIZE;}
constexpr int get_file(Square square) { return square % BOARD_SIZE;}

// Bit manipulation
inline Square pop_lsb(Bitboard& bb) {
    Square sq = std::countr_zero(bb);
    bb &= bb - 1; // pop
    return sq;
}
