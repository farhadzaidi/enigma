#include "types.hpp"
#include "evaluate.hpp"

int evaluate(Board& b) {
    // TODO: keep track of material incrementally
    int white_material = 0;
    int black_material = 0;
    for (Piece piece = PAWN; piece < NUM_PIECES - 1; piece++) {
        white_material += std::popcount(b.pieces[WHITE][piece]) * PIECE_VALUE[piece];
        black_material += std::popcount(b.pieces[BLACK][piece]) * PIECE_VALUE[piece];
    }

    int material = white_material - black_material;
    return b.to_move == WHITE ? material : -material;
}
