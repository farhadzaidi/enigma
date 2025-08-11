#ifndef MOVE_HPP
#define MOVE_HPP

#include <cstdint>

// Each move is represented by a 16 bit unsigned integer
// Bits 1-6 indicate the origin square (0-63)
// Bits 7-12 indicate the destination square (0-63)
// Bits 13 indicates whether the move type (quiet or capture)
// Bits 14-16 indicate the move flag (special move)

inline uint16_t new_move(uint16_t from, uint16_t to, uint16_t mtype, uint16_t flag) {
    to <<= 6;
	mtype <<= 12;
	flag <<= 13;
	return from | to | mtype | flag;
}
inline int get_from(uint16_t m) {return m & 63;}
inline int get_to(uint16_t m) {return (m >> 6) & 63;}
inline int get_mtype(uint16_t m) {return (m >> 12) & 1;}
inline int get_flag(uint16_t m) {return (m >> 13) & 7;}

#endif