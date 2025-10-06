#include <algorithm>
#include <chrono>

#include "types.hpp"
#include "search.hpp"
#include "board.hpp"
#include "move.hpp"
#include "movegen.hpp"
#include "evaluate.hpp"

// Globals for tracking time
std::chrono::_V2::steady_clock::time_point g_start;
int g_time_limit_ms;


static bool should_stop_search() {
    // Stop when atomic flag is set
    if (stop_requested) return true;

    // Check if the search has exceeded its time limit
    auto elapsed = 
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - g_start
        ).count();
    
    return elapsed >= g_time_limit_ms;
}

static int negamax(Board& b, int depth) {
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
    // Set globals for tracking time
    g_start = std::chrono::steady_clock::now();
    g_time_limit_ms = time_limit_ms;

    Move best_move;
    for (int depth = 1;; depth++) {
        Move best_move_at_depth;
        int best_score = MIN_SCORE;

        MoveList moves = generate_moves(b);
        for (Move move : moves) {
            // Stop the search early if needed
            if (should_stop_search()) return best_move;

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
    }
}
