#include <vector>
#include <iostream>

#include "perft.hpp"
#include "board.hpp"
#include "move.hpp"
#include "movegen.hpp"

int perft(Board &b, int depth) {
    if (depth == 0) {
        return 1;
    }

    int nodes = 0;
    std::vector<Move> moves = generate_moves(b);
    for (Move move: moves) {
        b.make_move(move);
        nodes += perft(b, depth - 1);
        b.unmake_move(move);
    }

    return nodes;
}