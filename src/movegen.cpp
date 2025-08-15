#include <vector>

#include "movegen.hpp"
#include "types.hpp"
#include "utils.hpp"

static void emit_pawn_moves(
    std::vector<Move>& moves,
    Bitmask move_mask,
    Bitmask promotion_rank,
    MoveType mtype,
    MoveFlag mflag, // NORMAL or EN_PASSANT; promotions will be determined
    int delta
) {
    while (move_mask != 0) {
        Square to = pop_lsb(move_mask);
        Square from = to - delta;
        if (get_mask(to) & promotion_rank) {
            // Higher value pieces get added first for move ordering
            moves.push_back(encode_move(from, to, mtype, PROMOTION_QUEEN));
            moves.push_back(encode_move(from, to, mtype, PROMOTION_ROOK));
            moves.push_back(encode_move(from, to, mtype, PROMOTION_BISHOP));
            moves.push_back(encode_move(from, to, mtype, PROMOTION_KNIGHT));
        } else {
            moves.push_back(encode_move(from, to, mtype, mflag));
        }
    }
}

static void generate_pawn_moves(Board& b, std::vector<Move>& moves) {
    Bitmask empty = ~b.occupied;
    Bitboard (*up)(Bitboard), (*up_right)(Bitboard), (*up_left)(Bitboard);
    Bitmask promotion_rank, double_push_rank;
    int up_delta, up_right_delta, up_left_delta;
    if (b.to_move == WHITE) {
        up = shift_north;
        up_right = shift_northeast;
        up_left = shift_northwest;
        promotion_rank = RANK_8_MASK;
        double_push_rank = RANK_4_MASK;
        up_delta = 8;
        up_right_delta = 9;
        up_left_delta = 7;
    } else {
        up = shift_south;
        up_right = shift_southwest;
        up_left = shift_southeast;
        promotion_rank = RANK_1_MASK;
        double_push_rank = RANK_5_MASK;
        up_delta = -8;
        up_right_delta = -9;
        up_left_delta = -7;
    }

    Bitboard enemy_pieces = b.colors[!b.to_move];
    Bitboard pawns = b.pieces[b.to_move][PAWN];
    Bitmask single_push = up(pawns) & empty;
    Bitmask double_push = up(single_push) & empty & double_push_rank;
    Bitmask right_capture = up_right(pawns) & enemy_pieces;
    Bitmask left_capture = up_left(pawns) & enemy_pieces;

    // Captures first for move ordering
    emit_pawn_moves(moves, right_capture, promotion_rank, CAPTURE, NORMAL, up_right_delta);
    emit_pawn_moves(moves, left_capture, promotion_rank, CAPTURE, NORMAL, up_left_delta);


    // Only compute en passant captures if ep target square is set
    if (b.en_passant_target != NO_SQUARE) {
        Bitmask en_passant_target_mask = get_mask(b.en_passant_target);
        Bitmask right_en_passant = up_right(pawns) & en_passant_target_mask;
        Bitmask left_en_passant = up_left(pawns) & en_passant_target_mask;

        emit_pawn_moves(moves, right_en_passant, promotion_rank, CAPTURE, EN_PASSANT, up_right_delta);
        emit_pawn_moves(moves, left_en_passant, promotion_rank, CAPTURE, EN_PASSANT, up_left_delta);
    }

    // Single and double push
    emit_pawn_moves(moves, single_push, promotion_rank, QUIET, NORMAL, up_delta);
    emit_pawn_moves(moves, double_push, promotion_rank, QUIET, NORMAL, up_delta * 2);
}