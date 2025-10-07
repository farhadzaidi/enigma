#include "movegen.hpp"
#include "types.hpp"
#include "utils.hpp"
#include "precompute.hpp"
#include "board.hpp"
#include "checkinfo.hpp"

template <Direction D>
static inline bool is_attacked_by_slider(const Board &b, Square sq) {
    constexpr auto& ray_map = get_ray_map<D>();
    Bitboard ray_mask = ray_map[sq] & b.occupied;
    if (ray_mask) {
        Square first = pop_next<D>(ray_mask);
        Bitboard first_mask = get_mask(first);
        if ((first_mask & b.colors[b.to_move ^ 1]) && is_relevant_sliding_piece<D>(b.piece_map[first])) {
            return true;
        }
    }

    return false;
}

static inline bool is_attacked_by_slider(const Board &b, Square sq) {
    return (
        is_attacked_by_slider<NORTH>    (b, sq) ||
        is_attacked_by_slider<SOUTH>    (b, sq) ||
        is_attacked_by_slider<EAST>     (b, sq) ||
        is_attacked_by_slider<WEST>     (b, sq) ||
        is_attacked_by_slider<NORTHEAST>(b, sq) ||
        is_attacked_by_slider<NORTHWEST>(b, sq) ||
        is_attacked_by_slider<SOUTHEAST>(b, sq) ||
        is_attacked_by_slider<SOUTHWEST>(b, sq)
    );
}

template <Piece P>
static inline void generate_piece_moves(Board& b, MoveList& moves, CheckInfo& checkInfo) {
    Bitboard piece_bb = b.pieces[b.to_move][P];
    Bitboard not_friendly = ~b.colors[b.to_move];

    while (piece_bb) {
        Square from = pop_lsb(piece_bb);

        Bitboard attack_mask = 
            P == KING   ? KING_ATTACK_MAP[from] & ~checkInfo.unsafe     :
            P == KNIGHT ? KNIGHT_ATTACK_MAP[from]                       :
            P == BISHOP ? generate_sliding_attack_mask<BISHOP>(b, from) :
            P == ROOK   ? generate_sliding_attack_mask<ROOK>  (b, from) :
            P == QUEEN  ? generate_sliding_attack_mask<BISHOP>(b, from) |
                          generate_sliding_attack_mask<ROOK>  (b, from) :
                          0;

        attack_mask &= not_friendly;
        if constexpr (P != KING) {
            attack_mask &= checkInfo.must_cover;
        }

        if (checkInfo.pinned & get_mask(from)) {
            attack_mask &= checkInfo.pins[from];
        }

        while (attack_mask) {
            Square to = pop_lsb(attack_mask);
            MoveType mtype = b.piece_map[to] == NO_PIECE ? QUIET : CAPTURE;

            if constexpr (P == KING) {
                if (mtype == CAPTURE) {
                    // If the move is a capture by the king, then we need to recompute
                    // enemy sliding attacks to see if an x-ray opened up.
                    Bitboard from_mask = get_mask(from);
                    b.occupied ^= from_mask;
                    bool is_attacked = is_attacked_by_slider(b, to);
                    b.occupied ^= from_mask;
                    if (is_attacked) continue;
                }
            }

            moves.add_move(Move(from, to, mtype, NORMAL));
        }
    }
}

template <Color C, Direction D, MoveType MT, bool IS_PROMOTION = false, bool IS_EN_PASSANT = false>
static inline void encode_pawn_moves(
    Board &b,
    MoveList& moves,
    CheckInfo& checkInfo,
    Bitboard move_mask
) {
    while (move_mask) {
        Square to = pop_lsb(move_mask);
        Square from = to - D;
        
        Bitboard from_mask = get_mask(from);
        Bitboard to_mask = get_mask(to);
        if (checkInfo.pinned & from_mask) {
            to_mask &= checkInfo.pins[from];
            if (!to_mask) continue;
        }

        if constexpr (IS_PROMOTION) {

            moves.add_move(Move(from, to, MT, PROMOTION_QUEEN));
            moves.add_move(Move(from, to, MT, PROMOTION_ROOK));
            moves.add_move(Move(from, to, MT, PROMOTION_BISHOP));
            moves.add_move(Move(from, to, MT, PROMOTION_KNIGHT));
        } else {
            if constexpr (IS_EN_PASSANT) {
                // Handle en passant edge cases

                constexpr Direction BACK = C == WHITE ? SOUTH : NORTH;
                Bitboard capture_mask = shift<BACK>(to_mask);

                if (checkInfo.checkers) {
                    // In the event of a single check, the moving EP pawn can either
                    // capture the checking pawn
                    bool captures_checker = (capture_mask & checkInfo.checkers) != 0;

                    // Or block the ray check (or stay pinned)
                    bool blocks_line = (to_mask & checkInfo.must_cover) != 0;

                    if (!captures_checker && !blocks_line) return;
                }

                // We also need to account for the case en passant opens up an
                // x-ray check

                b.occupied ^= from_mask;
                b.occupied ^= to_mask;
                b.occupied ^= capture_mask;

                bool is_attacked = is_attacked_by_slider(b, b.king_squares[C]);

                b.occupied ^= capture_mask;
                b.occupied ^= to_mask;
                b.occupied ^= from_mask;

                if (is_attacked) return;
                moves.add_move(Move(from, to, MT, EN_PASSANT));
            } else {
                moves.add_move(Move(from, to, MT, NORMAL));
            }
        }
    }
}

template<Color C>
static inline void generate_pawn_moves(Board& b, MoveList& moves, CheckInfo& checkInfo) {
    // Compile-time constants derived from template
    constexpr Direction FWD              = C == WHITE ? NORTH : SOUTH;
    constexpr Direction FWD_FWD          = C == WHITE ? NORTH_NORTH : SOUTH_SOUTH;
    constexpr Direction FWD_RIGHT        = C == WHITE ? NORTHEAST: SOUTHWEST;
    constexpr Direction FWD_LEFT         = C == WHITE ? NORTHWEST: SOUTHEAST;
    constexpr Direction BACK             = C == WHITE ? SOUTH : NORTH;
    constexpr Bitboard  PROMO_MASK       = C == WHITE ? RANK_7_MASK : RANK_2_MASK;
    constexpr Bitboard  DOUBLE_PUSH_MASK = C == WHITE ? RANK_4_MASK : RANK_5_MASK;

    Bitboard empty = ~b.occupied;
    Bitboard enemy_pieces = b.colors[C ^ 1];
    Bitboard pawns = b.pieces[C][PAWN];
    Bitboard promo_pawns = pawns & PROMO_MASK;
    Bitboard non_promo_pawns = pawns & ~PROMO_MASK;

    // Don't mask with must_cover yet, since we have to compute double push
    Bitboard single_push    = shift<FWD>(non_promo_pawns) & empty; 
    Bitboard double_push    = shift<FWD>(single_push) & empty & DOUBLE_PUSH_MASK & checkInfo.must_cover;
    // Now we can mask
    single_push &= checkInfo.must_cover;

    Bitboard right_capture  = shift<FWD_RIGHT>(non_promo_pawns) & enemy_pieces & checkInfo.must_cover;
    Bitboard left_capture   = shift<FWD_LEFT>(non_promo_pawns) & enemy_pieces & checkInfo.must_cover;
    
    Bitboard right_en_passant = 0;
    Bitboard left_en_passant = 0;
    if (b.en_passant_target != NO_SQUARE) {
        Bitboard en_passant_target_mask = get_mask(b.en_passant_target);
        right_en_passant = shift<FWD_RIGHT>(non_promo_pawns) & en_passant_target_mask;
        left_en_passant  = shift<FWD_LEFT>(non_promo_pawns) & en_passant_target_mask;
    }

    // Promotion moves
    Bitboard single_push_promo      = shift<FWD>(promo_pawns) & empty & checkInfo.must_cover;
    Bitboard right_capture_promo    = shift<FWD_RIGHT>(promo_pawns) & enemy_pieces & checkInfo.must_cover;
    Bitboard left_capture_promo     = shift<FWD_LEFT>(promo_pawns) & enemy_pieces & checkInfo.must_cover;

    // Encode and add moves to vector

    // Promotions
    encode_pawn_moves<C, FWD_RIGHT, CAPTURE, true>(b, moves, checkInfo, right_capture_promo);
    encode_pawn_moves<C, FWD_LEFT, CAPTURE, true>(b, moves, checkInfo, left_capture_promo);
    encode_pawn_moves<C, FWD, QUIET, true>(b, moves, checkInfo, single_push_promo);

    // Regular pushes/captures
    encode_pawn_moves<C, FWD_RIGHT, CAPTURE>(b, moves, checkInfo, right_capture);
    encode_pawn_moves<C, FWD_LEFT, CAPTURE>(b, moves, checkInfo, left_capture);
    encode_pawn_moves<C, FWD, QUIET>(b, moves, checkInfo, single_push);
    encode_pawn_moves<C, FWD_FWD, QUIET>(b, moves, checkInfo, double_push);

    // En passant
    encode_pawn_moves<C, FWD_RIGHT, CAPTURE, false, true>(b, moves, checkInfo, right_en_passant);
    encode_pawn_moves<C, FWD_LEFT, CAPTURE, false, true>(b, moves, checkInfo, left_en_passant);
}

template <Color C>
static inline void generate_castling_moves(Board &b, MoveList& moves, CheckInfo& checkInfo) {
    constexpr auto SHORT_CASTLING_RIGHTS = C == WHITE ? WHITE_SHORT : BLACK_SHORT;
    constexpr auto LONG_CASTLING_RIGHTS  = C == WHITE ? WHITE_LONG : BLACK_LONG;
    constexpr auto SHORT_CASTLE_PATH     = C == WHITE ? WHITE_SHORT_CASTLE_PATH : BLACK_SHORT_CASTLE_PATH;
    constexpr auto LONG_CASTLE_PATH      = C == WHITE ? WHITE_LONG_CASTLE_PATH : BLACK_LONG_CASTLE_PATH;
    constexpr auto SHORT_TO              = C == WHITE ? G1 : G8;
    constexpr auto LONG_TO               = C == WHITE ? C1 : C8;
    constexpr auto KING_SQUARE           = C == WHITE ? E1 : E8;

    // Castle path squares (that king walks over)
    constexpr auto F_SQUARE = C == WHITE ? F1 : F8;
    constexpr auto G_SQUARE = C == WHITE ? G1 : G8;

    constexpr auto D_SQUARE = C == WHITE ? D1 : D8;
    constexpr auto C_SQUARE = C == WHITE ? C1 : C8;

    if (std::popcount(checkInfo.checkers) != 0) return;

    // Compute the path that the king walks over (not the full path in the case of a long castle)
    Bitboard king_short_castle_path = get_mask(F_SQUARE) | get_mask(G_SQUARE);
    Bitboard king_long_castle_path = get_mask(D_SQUARE) | get_mask(C_SQUARE);

    // Short castle
    if (
        (b.castling_rights & SHORT_CASTLING_RIGHTS) // Have castling rights for this side
        && ((b.occupied & SHORT_CASTLE_PATH) == 0)  // Path is clear
        && ((king_short_castle_path & checkInfo.unsafe) == 0) // King doesn't pass thru check
    ) {
        moves.add_move(Move(KING_SQUARE, SHORT_TO, QUIET, CASTLE));
    } 
    
    // Long castle
    if (
        (b.castling_rights & LONG_CASTLING_RIGHTS)
        && ((b.occupied & LONG_CASTLE_PATH) == 0)
        && ((king_long_castle_path & checkInfo.unsafe) == 0)
    ) {
        moves.add_move(Move(KING_SQUARE, LONG_TO, QUIET, CASTLE));
    }
}

template <Color C>
static inline void _generate_moves(Board& b, MoveList& moves) {
    CheckInfo checkInfo;
    checkInfo.compute_check_info<C>(b);

    if (std::popcount(checkInfo.checkers) == 2) {
        // Double check; only king moves are valid (noncastling)
        generate_piece_moves<KING>(b, moves, checkInfo);
        return;
    }

    generate_pawn_moves<C>(b, moves, checkInfo);
    generate_castling_moves<C>(b, moves, checkInfo);

    generate_piece_moves<BISHOP>(b, moves, checkInfo);
    generate_piece_moves<KNIGHT>(b, moves, checkInfo);
    generate_piece_moves<ROOK>  (b, moves, checkInfo);
    generate_piece_moves<QUEEN> (b, moves, checkInfo);
    generate_piece_moves<KING>  (b, moves, checkInfo);
}

MoveList generate_moves(Board &b) {
    MoveList moves;

    if (b.to_move == WHITE) _generate_moves<WHITE>(b, moves);
    else                    _generate_moves<BLACK>(b, moves);

    return moves;
}