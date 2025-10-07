#include <algorithm>
#include <chrono>

#include "types.hpp"
#include "search.hpp"
#include "board.hpp"
#include "movegen.hpp"
#include "evaluate.hpp"

// Deadline after which we stop the search
std::chrono::_V2::steady_clock::time_point deadline;

// Keep track of how many nodes have been traversed.
// Useful for periodically checking stop condition (only check every X nodes)
uint64_t nodes;

static inline bool should_stop_search() {
    // Stop when atomic flag is set
    if (stop_requested) return true;

    // Check if the search has exceeded its time limit
    return std::chrono::steady_clock::now() >= deadline;
}

static inline int negamax(Board& b, int depth) {
    nodes++;

    // Stop search early if needed
    // Returning minimum score here to indicate that we couldn't find a move
    // Guarantees that we don't select this move
    if (should_stop_search()) {
        return MIN_SCORE;
    }

    if (depth == 0) {
        return evaluate(b);
    }

    int best_score = MIN_SCORE;
    MoveList moves = generate_moves(b);

    // Side to move has no remaining moves
    if (moves.size == 0) {
        if (b.in_check()) {
            // If we're in check with no moves, then that is a checkmate
            // Add ply to the score to incentivize drawing out the game for the
            // losing side or ending the game quicker for the winning side
            return -CHECKMATE_SCORE + b.ply;
        } else {
            // If we're not in check with no moves, then that is a stalemate
            return STALEMATE_SCORE;
        }
    }

    for (Move move : moves) {
        b.make_move(move);
        int score = -negamax(b, depth - 1);
        best_score = std::max(best_score, score);
        b.unmake_move(move);
    }
    
    return best_score;
}

// Iteratively calls negamax search with increasing depth
Move search(Board& b, int time_limit_ms) {
    // Calculate search deadline based on time limit
    deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(time_limit_ms);

    nodes = 0;
    int depth = 1;
    Move best_move;

    // Ensure at least one iteration to make sure we don't return a null move
    while (depth == 1 || !should_stop_search()) {
        Move best_move_at_depth;
        int best_score = MIN_SCORE;

        MoveList moves = generate_moves(b);
        for (Move move : moves) {
            b.make_move(move);
            int score = -negamax(b, depth - 1);
            if (score > best_score) {
                best_score = score;
                best_move_at_depth = move;
            }
            b.unmake_move(move);
        }

        // Once search is complete at this depth, we can replace the best move
        // from the search at the previous depth with the best move from the
        // search at this depth
        best_move = best_move_at_depth;
        depth++;
    }

    return best_move;
}
