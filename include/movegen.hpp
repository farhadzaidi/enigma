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

// Expose this function since it's used in board.cpp as well
template <Piece P>
inline Bitboard generate_sliding_attack_mask(const Board& b, Square from) {
    // Assign constants based on sliding piece type
    constexpr auto& attack_table = P == BISHOP ? BISHOP_ATTACK_TABLE : ROOK_ATTACK_TABLE; 
    constexpr auto& blocker_map  = P == BISHOP ? BISHOP_BLOCKER_MAP : ROOK_BLOCKER_MAP;
    constexpr auto& magic        = P == BISHOP ? BISHOP_MAGIC : ROOK_MAGIC;
    constexpr auto& offset       = P == BISHOP ? BISHOP_OFFSET : ROOK_OFFSET;
        
    // Look up sliding piece attacks from attack table based on blocker pattern
    Bitboard blocker_mask = blocker_map[from];
    Bitboard blockers = b.occupied & blocker_mask;
    size_t index = get_attack_table_index(blockers, blocker_mask, magic[from]);
    return attack_table[offset[from] + index];
}

MoveList generate_moves(Board& b);