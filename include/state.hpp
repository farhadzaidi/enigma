#ifndef STATE_HPP
#define STATE_HPP

#include <cstdint>

// This struct contains important board state information that is otherwise not
// contained in the move encoding or able to be extracted from pieces

struct State {
    uint8_t en_passant_target;
    uint8_t castling_rights;
    uint8_t halfmoves;
    uint8_t captured_piece;

    State(uint8_t ep, uint8_t cr, uint8_t hm, uint8_t cp) :
        en_passant_target(ep),
        castling_rights(cr),
        halfmoves(hm),
        captured_piece(cp) {}
};

#endif