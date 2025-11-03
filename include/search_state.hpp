#pragma once

#include <cstdint>

#include "types.hpp"
#include "move.hpp"

struct SearchLimits {
    int time;
    uint64_t nodes;
    SearchDepth depth;
};

struct SearchState {
    SearchLimits limits;
    std::chrono::steady_clock::time_point deadline; 
    uint64_t nodes = 0;
    bool search_interrupted = false;

    // Killer moves
    KillerMove killer_1;
    KillerMove killer_2;

    // History heuristic tables (for quiet moves)
    ColorPieceToHistory color_piece_to{};
    FromToHistory from_to{};
};