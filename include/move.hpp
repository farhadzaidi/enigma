#pragma once

#include <cstdint>

#include "constants.hpp"

// Each move is represented by a 16 bit unsigned integer
// Bits 1-6 indicate the origin square (0-63)
// Bits 7-12 indicate the destination square (0-63)
// Bits 13 indicates whether the move type (quiet or capture)
// Bits 14-16 indicate the move flag (special move)

inline Move encode_move(
    MoveComponent from, 
    MoveComponent to, 
    MoveComponent mtype, 
    MoveComponent flag
) {
    to <<= 6;
	mtype <<= 12;
	flag <<= 13;
	return from | to | mtype | flag;
}
inline MoveComponent get_from(Move m) {return m & 63;}
inline MoveComponent get_to(Move m) {return (m >> 6) & 63;}
inline MoveComponent get_mtype(Move m) {return (m >> 12) & 1;}
inline MoveComponent get_flag(Move m) {return (m >> 13) & 7;}