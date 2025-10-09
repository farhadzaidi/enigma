#pragma once

#include <cstdint>

#include "types.hpp"
#include "board.hpp"
#include "move.hpp"

struct SearchLimits {
    int time;
    uint64_t nodes;
    int depth;
};

struct SearchGlobals {
    SearchLimits limits;
    std::chrono::steady_clock::time_point deadline;
    uint64_t nodes = 0;
    bool search_interrupted = false;
};

template <SearchMode SM>
Move search(Board& b, const SearchLimits& limits);

inline Move search_time(Board& b, int time) {
    return search<TIME>(b, {.time = time});
}

inline Move search_nodes(Board& b, uint64_t nodes) {
    return search<NODES>(b, {.nodes = nodes});
}

inline Move search_depth(Board& b, int depth) {
    return search<DEPTH>(b, {.depth = depth});
}

inline Move search_infinite(Board& b) {
    return search<INFINITE>(b, {});
}