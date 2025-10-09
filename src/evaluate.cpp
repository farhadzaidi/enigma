#include "types.hpp"
#include "evaluate.hpp"

int evaluate(Board& b) {
    Color us = b.to_move;
    Color them = b.to_move ^ 1;
    return b.material[us] - b.material[them];
}
