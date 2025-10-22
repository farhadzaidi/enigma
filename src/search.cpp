#include <algorithm>
#include <chrono>

#include "types.hpp"
#include "search.hpp"
#include "board.hpp"
#include "movegen.hpp"
#include "evaluate.hpp"

/*
Search
4. TT + zobrist hashing
5. Killer moves
6. Principal variation search + aspiration windows
7. Enhanced move ordering/move selector (MVV-LVA, history heuristic, phases, SEE)
8. stackalloc instead of regular array in movelist to speed up movegen?
9. null move pruning
10. late move reductions
11. opening book/endgame tablebase
12. internal iterative deepening when no TT move

Evaluation
1. piece square tables
2. king/pawn endgame tables
3. check extensions
4. pawn structure
    - passed pawn bonus
    - isolated pawn penalty
    - stack pawn penalty
    - pawn hash structure?
5. king safety (pawn shield, open files, attackers near king)
6. rooks on open files
7. bishop pairs
*/

static SearchState ss;

constexpr uint64_t TIME_CHECK_PERIOD_MASK = 2047;

template <SearchMode SM>
static inline bool should_stop_search() {
    // Stop when the search interrupted flag is set or if stop is requested via UCI
    if (ss.search_interrupted || stop_requested) {
        return true;
    }

    if constexpr (SM == TIME) {
        // Check if the search has exceeded its time limit (if search mode is TIME)
        // Only check every N nodes (where N = TIME_CHECK_PERIOD_MASK + 1)
        return (
            (ss.nodes & TIME_CHECK_PERIOD_MASK) == 0
            && std::chrono::steady_clock::now() >= ss.deadline
        );
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

static inline int score_move(Move m) {
    return m.is_promotion() ? 3 : m.type() == CAPTURE ? 2 : 1;
}

template <bool UsePrevBestMove>
static inline void order_moves(MoveList& moves, Move prev_best_move = NULL_MOVE) {
    // Prioritize promotions, captures, then quiet moves (in that order)
    std::sort(moves.begin(), moves.end(), [prev_best_move](const Move& m1, const Move& m2) {
        // Always try the previous best move first if we have it
        if constexpr (UsePrevBestMove) {
            if (m1 == prev_best_move) return true;
            if (m2 == prev_best_move) return false;
        }

        return score_move(m1) > score_move(m2);
    });
}

template <SearchMode SM>
static inline int quiescence_search(Board& b, int alpha, int beta) {
    ss.nodes++;

    if (should_stop_search<SM>()) {
        ss.search_interrupted = true;
        return SEARCH_INTERRUPTED;
    }

    bool in_check = b.in_check();

    // First, we get a static evaluation of the position without any captures
    // This serves as our baseline score to prevent forcing captures (in a position where all captures are bad)
    // Additionally, we can stop the search early if the static evaluation is higher than the beta cutoff
    // This can only be done if we're not in check - otherwise we MUST make a move
    if (!in_check) {
        int no_capture_score = evaluate(b);
        alpha = std::max(alpha, no_capture_score);
        if (alpha >= beta) {
            return beta;
        }
    }

    // If we're not in check, we can only search captures. Otherwise, we must search all moves (evasions)
    MoveList moves = in_check ? generate_moves<ALL>(b) : generate_moves<CAPTURE_ONLY>(b);
    if (moves.is_empty()) {
        if (in_check) {
            // In check + no legal moves - checkmate
            return -CHECKMATE_SCORE + b.ply;
        }

        // Searched all captures already, we can return now
        return alpha;
    }

    for (Move move : moves) {
        b.make_move(move);
        int score = -quiescence_search<SM>(b, -beta, -alpha);
        b.unmake_move(move);

        if (ss.search_interrupted) {
            return SEARCH_INTERRUPTED;
        }

        alpha = std::max(alpha, score);
        if (alpha >= beta) {
            break;
        }
    }

    return alpha;
}

template <SearchMode SM>
static inline int negamax(Board& b, int depth, int alpha, int beta) {
    ss.nodes++;

    if (should_stop_search<SM>()) {
        ss.search_interrupted = true;
        return SEARCH_INTERRUPTED; // Dummy value (for semantics) - will not be used
    }

    if (depth == 0) {
        return quiescence_search<SM>(b, alpha, beta);
    }

    MoveList moves = generate_moves<ALL>(b);
    order_moves<false>(moves);

    // Side to move has no remaining moves
    if (moves.is_empty()) {
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
static Move search_at_depth(Board& b, int depth, Move prev_best_move) {
    Move best_move;

    // Alpha will serve as our lower bound (best score so far at this depth)
    int alpha = MIN_SCORE;

    // Beta will serve as our upper bound - if we find a move better than beta
    // then that move is too good and our opponent won't allow it (it's worse
    // for them than their lower bound)
    int beta = MAX_SCORE;

    MoveList moves = generate_moves<ALL>(b);
    order_moves<true>(moves, prev_best_move);

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

        Move best_move_at_depth = search_at_depth<SM>(b, depth, best_move);
        if (best_move_at_depth != NULL_MOVE) {
            best_move = best_move_at_depth;
        }

        if (ss.search_interrupted) break;

        depth++;
    }

    // In the rare case where we have legal moves at this position, but we weren't able
    // to complete our first search (depth = 1), we return an arbitrary move
    MoveList moves = generate_moves<ALL>(b);
    return best_move == NULL_MOVE && !moves.is_empty() ? moves[0] : best_move;
}

// Explicit template instantiations
template Move search<TIME>(Board& b, const SearchLimits& limits);
template Move search<NODES>(Board& b, const SearchLimits& limits);
template Move search<DEPTH>(Board& b, const SearchLimits& limits);
template Move search<INFINITE>(Board& b, const SearchLimits& limits);
