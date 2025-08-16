#pragma once

#include "types.hpp"

// Each move is represented by a 16 bit unsigned integer
// Bits 1-6 indicate the origin square (0-63)
// Bits 7-12 indicate the destination square (0-63)
// Bits 13 indicates whether the move type (quiet or capture)
// Bits 14-16 indicate the move flag (special move)

constexpr Move encode_move(
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
constexpr Square get_from(Move m) {return m & 63;}
constexpr Square get_to(Move m) {return (m >> 6) & 63;}
constexpr MoveType get_mtype(Move m) {return (m >> 12) & 1;}
constexpr MoveFlag get_mflag(Move m) {return (m >> 13) & 7;}
constexpr bool is_promotion(MoveFlag mf) {return mf >= 3;}