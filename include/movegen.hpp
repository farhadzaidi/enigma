#pragma once

#include <array>

#include "types.hpp"
#include "board.hpp"
#include "move.hpp"

struct MoveList {
    std::array<Move, MAX_MOVES> moves;
    int size = 0;

    void add_move(Move move) {
        moves[size] = move;
        size++;
    }

    // Iterator support for range-based for loops
    Move* begin() { return moves.data(); }
    Move* end() { return moves.data() + size; }
    const Move* begin() const { return moves.data(); }
    const Move* end() const { return moves.data() + size; }
};

MoveList generate_moves(Board& b);