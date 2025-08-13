#pragma once

#include <cstdint>

#include "types.hpp"

// This struct contains important board state information that is otherwise not
// contained in the move encoding or able to be extracted from pieces

struct State {
    Square en_passant_target;
    CastlingRights castling_rights;
    uint8_t halfmoves; // Truncating from int to U8 to save space
    Piece captured_piece;

    State(Square ep, CastlingRights cr, uint8_t hm, Piece cp) :
        en_passant_target(ep),
        castling_rights(cr),
        halfmoves(hm),
        captured_piece(cp) {}
};