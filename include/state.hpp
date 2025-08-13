#pragma once

#include <cstdint>

#include "constants.hpp"

// This struct contains important board state information that is otherwise not
// contained in the move encoding or able to be extracted from pieces

struct State {
    Square en_passant_target;
    CastlingRights castling_rights;
    U8 halfmoves;
    Piece captured_piece;

    State(Square ep, CastlingRights cr, U8 hm, Piece cp) :
        en_passant_target(ep),
        castling_rights(cr),
        halfmoves(hm),
        captured_piece(cp) {}
};