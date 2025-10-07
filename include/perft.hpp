#pragma once

#include <iostream>

#include "board.hpp"
#include "utils.hpp"
#include "move.hpp"
#include "movegen.hpp"

template<bool Root>
inline uint64_t perft(Board &b, int depth) {
    uint64_t nodes = 0;
    uint64_t total_nodes = 0;
    MoveList moves = generate_moves(b);


    // No need to make moves, just return the count
    if (depth == 1) {
        return moves.size;
    }

    for (Move move: moves) {
        b.make_move(move);
        nodes = perft<false>(b, depth - 1);
        total_nodes += nodes;
        b.unmake_move(move);

        if (Root) {
            std::clog << decode_move_to_uci(move) << ": " << nodes << "\n";
        }

    }

    if (Root) {
        std::clog << "\nNodes searched: " << total_nodes << "\n";
    }

    return total_nodes;
}