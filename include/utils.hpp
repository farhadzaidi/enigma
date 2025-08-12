#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <cstdint>

#include "constants.hpp"

const uint64_t NOT_A_FILE = 0xfefefefefefefefe;
const uint64_t NOT_H_FILE = 0x7f7f7f7f7f7f7f7f;

int get_square_index(int rank, int file);
int algebraic_to_index(const std::string& square);
std::string index_to_algebraic(int index);
uint16_t encode_move_from_uci(const std::string& uci_str);
std::string decode_move_to_uci(uint16_t move);


// Shifting bitboards
inline uint64_t shift_north(uint64_t b) {return b << 8;}
inline uint64_t shift_south(uint64_t b) {return b >> 8;}
inline uint64_t shift_east(uint64_t b) {return (b << 1) & NOT_A_FILE;}
inline uint64_t shift_northeast(uint64_t b) {return (b << 9) & NOT_A_FILE;}
inline uint64_t shift_southeast(uint64_t b) {return (b >> 7) & NOT_A_FILE;}
inline uint64_t shift_west(uint64_t b) {return (b >> 1) & NOT_H_FILE;}
inline uint64_t shift_northwest(uint64_t b) {return (b << 7) & NOT_H_FILE;}
inline uint64_t shift_southwest(uint64_t b) {return (b >> 9) & NOT_H_FILE;}

inline uint64_t get_mask(int square) { return uint64_t{1} << square;}
inline int get_rank(int square) { return square / BOARD_SIZE;}
int inline get_file(int square) { return square % BOARD_SIZE;}


#endif