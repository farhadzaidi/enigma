#pragma once

#include <cstdint>

#include "types.hpp"

// Each move is represented by a 16 bit unsigned integer
// Bits 1-6 indicate the origin square (0-63)
// Bits 7-12 indicate the destination square (0-63)
// Bits 13 indicates whether the move type (quiet or capture)
// Bits 14-16 indicate the move flag (special move)

inline Move encode_move(
    uint16_t from, 
    uint16_t to, 
    MoveType mtype, 
    MoveFlag mflag
) {
    to <<= 6;
	mtype <<= 12;
	mflag <<= 13;
	return from | to | mtype | mflag;
}
inline Square get_from(Move m) {return m & 63;}
inline Square get_to(Move m) {return (m >> 6) & 63;}
inline MoveType get_mtype(Move m) {return (m >> 12) & 1;}
inline MoveFlag get_mflag(Move m) {return (m >> 13) & 7;}