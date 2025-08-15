#include <vector>

#include "movegen.hpp"
#include "types.hpp"
#include "utils.hpp"

template<Color C>
inline Bitboard fwd(Bitboard bb) {
    if constexpr (C == WHITE) return shift_north(bb);
    else return shift_south(bb);
}

template<Color C>
inline Bitboard fwd_right(Bitboard bb) {
    if constexpr (C == WHITE) return shift_northeast(bb);
    else return shift_southwest(bb);
}

template<Color C>
inline Bitboard fwd_left(Bitboard bb) {
    if constexpr (C == WHITE) return shift_northwest(bb);
    else return shift_southeast(bb);
}

static inline void emit_pawn_moves(
    std::vector<Move>& moves,
    Bitmask move_mask,
    MoveType mtype,
    MoveFlag mflag,
    int delta
) {
    while (move_mask != 0) {
        Square to = pop_lsb(move_mask);
        Square from = to - delta;
        moves.push_back(encode_move(from, to, mtype, mflag));
    }
}

static inline void emit_pawn_promo_moves(
    std::vector<Move>& moves,
    Bitmask move_mask,
    MoveType mtype,
    MoveFlag mflag,
    int delta
) {
    while (move_mask != 0) {
        Square to = pop_lsb(move_mask);
        Square from = to - delta;
        moves.push_back(encode_move(from, to, mtype, PROMOTION_QUEEN));
        moves.push_back(encode_move(from, to, mtype, PROMOTION_ROOK));
        moves.push_back(encode_move(from, to, mtype, PROMOTION_BISHOP));
        moves.push_back(encode_move(from, to, mtype, PROMOTION_KNIGHT));
    }
}

template<Color C>
static void generate_pawn_moves(Board& b, std::vector<Move>& moves) {
    // Compile-time constants derived from template
    constexpr int fwd_delta = C == WHITE ? NORTH_DELTA : SOUTH_DELTA;
    constexpr int fwd_right_delta = C == WHITE ? NORTHEAST_DELTA : SOUTHWEST_DELTA;
    constexpr int fwd_left_delta = C == WHITE ? NORTHWEST_DELTA : SOUTHEAST_DELTA;
    constexpr Bitmask promo_rank = C == WHITE ? RANK_7_MASK : RANK_2_MASK;
    constexpr Bitmask double_push_rank = C == WHITE ? RANK_4_MASK : RANK_5_MASK;

    Bitmask empty = ~b.occupied;

    Bitboard enemy_pieces = b.colors[C ^ 1];
    Bitboard pawns = b.pieces[C][PAWN];
    Bitboard promo_pawns = pawns & promo_rank;
    Bitboard non_promo_pawns = pawns & ~promo_rank;

    // No promotion
    Bitmask single_push = fwd<C>(non_promo_pawns) & empty;
    Bitmask double_push = fwd<C>(single_push) & empty & double_push_rank;
    Bitmask right_capture = fwd_right<C>(non_promo_pawns) & enemy_pieces;
    Bitmask left_capture = fwd_left<C>(non_promo_pawns) & enemy_pieces;
    
    Bitmask right_en_passant = 0;
    Bitmask left_en_passant = 0;
    if (b.en_passant_target != NO_SQUARE) {
        Bitmask en_passant_target_mask = get_mask(b.en_passant_target);
        right_en_passant = fwd_right<C>(non_promo_pawns) & en_passant_target_mask;
        left_en_passant = fwd_left<C>(non_promo_pawns) & en_passant_target_mask;
    }

    // Promotion moves
    Bitmask single_push_promo = fwd<C>(promo_pawns) & empty;
    Bitmask right_capture_promo = fwd_right<C>(promo_pawns) & enemy_pieces;
    Bitmask left_capture_promo = fwd_left<C>(promo_pawns) & enemy_pieces;

    // Encode and add moves to vector
    emit_pawn_promo_moves(moves, right_capture_promo, CAPTURE, NORMAL, fwd_right_delta);
    emit_pawn_promo_moves(moves, left_capture_promo, CAPTURE, NORMAL, fwd_left_delta);
    emit_pawn_promo_moves(moves, single_push_promo, QUIET, NORMAL, fwd_delta);

    emit_pawn_moves(moves, right_capture, CAPTURE, NORMAL, fwd_right_delta);
    emit_pawn_moves(moves, left_capture, CAPTURE, NORMAL, fwd_left_delta);
    emit_pawn_moves(moves, right_en_passant, CAPTURE, EN_PASSANT, fwd_right_delta);
    emit_pawn_moves(moves, left_en_passant, CAPTURE, EN_PASSANT, fwd_left_delta);
    emit_pawn_moves(moves, single_push, QUIET, NORMAL, fwd_delta);
    emit_pawn_moves(moves, double_push, QUIET, NORMAL, fwd_delta * 2);
}