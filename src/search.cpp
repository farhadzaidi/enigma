#include <algorithm>
#include <chrono>

#include "types.hpp"
#include "search.hpp"
#include "board.hpp"
#include "movegen.hpp"
#include "evaluate.hpp"

static SearchState ss;

template <SearchMode SM>
static inline bool should_stop_search() {
    // Stop when the search interrupted flag is set or if stop is requested via UCI
    if (ss.search_interrupted || stop_requested) {
        return true;
    }

    if constexpr (SM == TIME) {
        // Check if the search has exceeded its time limit (if search mode is TIME)
        return std::chrono::steady_clock::now() >= ss.deadline;
    } else if constexpr (SM == NODES) {
        // Check if search has exceeded the number of nodes to search (if search mode is NODE)
        return ss.nodes >= ss.limits.nodes;
    } else {
        // In all other cases, we shouldn't stop the search
        // INFINITE = keep going forever (or until stop flag)
        // DEPTH is handled in the iterative search loop
        return false;
    }
}

template <SearchMode SM>
static inline int negamax(Board& b, int depth, int alpha, int beta) {
    if (should_stop_search<SM>()) {
        ss.search_interrupted = true;
        return SEARCH_INTERRUPTED; // Dummy value (for semantics) - will not be used
    }

    ss.nodes++;

    if (depth == 0) {
        return evaluate(b);
    }

    MoveList moves = generate_moves<ALL>(b);

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
        int score = -negamax<SM>(b, depth - 1, -beta, -alpha);
        b.unmake_move(move);

        // Discard the score and return early if the search has been interrupted
        if (ss.search_interrupted) {
            return SEARCH_INTERRUPTED;
        }

        // Update lower bound and determine if we need to prune this branch
        alpha = std::max(alpha, score);
        if (alpha >= beta) {
            break;
        }
    }
    
    return alpha;
}

// Searches all root moves at a given depth and returns the best move
template <SearchMode SM>
static Move search_at_depth(Board& b, int depth) {
    Move best_move;

    // Alpha will serve as our lower bound (best score so far at this depth)
    int alpha = MIN_SCORE;

    // Beta will serve as our upper bound - if we find a move better than beta
    // then that move is too good and our opponent won't allow it (it's worse
    // for them than their lower bound)
    int beta = MAX_SCORE;

    MoveList moves = generate_moves<ALL>(b);
    for (Move move : moves) {
        b.make_move(move);
        int score = -negamax<SM>(b, depth - 1, -beta, -alpha);
        b.unmake_move(move);

        // Same here - return early if the search is interrutpted, otherwise negate
        // the score to process it for the parent
        if (ss.search_interrupted) {
            return NULL_MOVE;
        }

        // If we found a move better than the current best move at this depth,
        // update the best score (alpha) and the best move at this depth
        if (score > alpha) {
            alpha = score;
            best_move = move;
        }

        // If the move we found is too good and our opponent will not allow it (because
        // they found a better move elsewhere), we can break out of the loop and return
        // early, effectively pruning the branch (aka beta cutoff)
        // In other words, the move we found is worse for the opponent than their current
        // lower bound and so we'll never be allowed to play this move
        if (alpha >= beta) {
            break;
        }
    }

    return best_move;
}

// Initializes search globals and performs iterative deepening search
template <SearchMode SM>
Move search(Board& b, const SearchLimits& limits) {
    ss.limits = limits;
    ss.nodes = 0;
    ss.search_interrupted = false;

    // Calculate search deadline based on time limit if search mode is TIME
    if constexpr (SM == TIME) {
        ss.deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(limits.time);
    }

    int depth = 1;
    Move best_move;

    // Iterative search loop
    while (!should_stop_search<SM>()) {
        // Check if we've hit the max depth if search mode is DEPTH
        if constexpr (SM == DEPTH) {
            if (depth > ss.limits.depth) break;
        }

        Move best_move_at_depth = search_at_depth<SM>(b, depth);
        if (best_move_at_depth != NULL_MOVE) {
            best_move = best_move_at_depth;
        }

        if (ss.search_interrupted) break;

        depth++;
    }

    return best_move;
}

// Explicit template instantiations
template Move search<TIME>(Board& b, const SearchLimits& limits);
template Move search<NODES>(Board& b, const SearchLimits& limits);
template Move search<DEPTH>(Board& b, const SearchLimits& limits);
template Move search<INFINITE>(Board& b, const SearchLimits& limits);
