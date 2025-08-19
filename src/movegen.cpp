#include <vector>

#include "movegen.hpp"
#include "types.hpp"
#include "utils.hpp"
#include "precompute.hpp"
#include "board.hpp"

template <Piece P>
static inline Bitboard generate_sliding_attack_mask(const Board& b, Square from) {
    // Assign constants based on sliding piece type
    constexpr auto& attack_table = P == BISHOP ? BISHOP_ATTACK_TABLE : ROOK_ATTACK_TABLE; 
    constexpr auto& blocker_map  = P == BISHOP ? BISHOP_BLOCKER_MAP : ROOK_BLOCKER_MAP;
    constexpr auto& magic        = P == BISHOP ? BISHOP_MAGIC : ROOK_MAGIC;
    constexpr auto& offset       = P == BISHOP ? BISHOP_OFFSET : ROOK_OFFSET;
        
    // Look up sliding piece attacks from attack table based on blocker pattern
    Bitboard blockers = b.occupied & blocker_map[from];
    int num_blockers = std::popcount(blocker_map[from]);
    size_t index = (blockers * magic[from]) >> (64 - num_blockers);
    return attack_table[offset[from] + index];
}

static inline bool is_square_attacked(const Board& b, Color attacker_color, Square sq) {
    // Get attack mask for each piece type and & with that piece's enemy bitboard
    const auto& attackers = b.pieces[attacker_color];
    
    // Check if attacked by knight
    if (KNIGHT_ATTACK_MAP[sq] & attackers[KNIGHT]) return true;

    // Check if attacked by king
    if (KING_ATTACK_MAP[sq] & attackers[KING]) return true;

    // Check if attacked by bishop or queen
    if (attackers[BISHOP] | attackers[QUEEN]) {
        Bitboard bishop_attack_mask = generate_sliding_attack_mask<BISHOP>(b, sq);
        if ((bishop_attack_mask & attackers[BISHOP]) || (bishop_attack_mask & attackers[QUEEN]))
            return true;
    }

    // Check if attacked by rook or queen
    if (attackers[ROOK] | attackers[QUEEN]) {
        Bitboard rook_attack_mask = generate_sliding_attack_mask<ROOK>(b, sq);
        if ((rook_attack_mask & attackers[ROOK]) || (rook_attack_mask & attackers[QUEEN]))
            return true;
    }

    // Check if attacked by pawn
    Bitboard square_mask = get_mask(sq);
    Bitboard pawn_attack_mask = attacker_color == BLACK
        ? shift<NORTHEAST>(square_mask) | shift<NORTHWEST>(square_mask)
        : shift<SOUTHEAST>(square_mask) | shift<SOUTHWEST>(square_mask);
    return pawn_attack_mask & attackers[PAWN];
}

static inline void generate_move_if_legal(
    Board& b, 
    std::vector<Move>& moves,
    Square from,
    Square to,
    MoveType mtype,
    MoveFlag mflag
) {
    Move m = encode_move(from, to, mtype, mflag);
    Color moving_color = b.to_move;
    Color attacker_color = b.to_move ^ 1;

    b.make_move(m);
    if (!is_square_attacked(b, attacker_color, b.king_squares[moving_color])) {
        moves.push_back(m);
    }
    b.unmake_move(m);
}

template <Piece P>
static inline void generate_piece_moves(Board& b, std::vector<Move>& moves) {
    Bitboard piece_bb = b.pieces[b.to_move][P];
    Bitboard not_friendly = ~b.colors[b.to_move];
    while (piece_bb) {
        Square from = pop_lsb(piece_bb);
        Bitboard attack_mask;
        switch (P) {
            case BISHOP: attack_mask = generate_sliding_attack_mask<BISHOP>(b, from); break;
            case KNIGHT: attack_mask = KNIGHT_ATTACK_MAP[from]; break;
            case ROOK:   attack_mask = generate_sliding_attack_mask<ROOK>(b, from); break;
            case KING:   attack_mask = KING_ATTACK_MAP[from]; break;
            case QUEEN:  
                attack_mask = 
                    generate_sliding_attack_mask<BISHOP>(b, from) | 
                    generate_sliding_attack_mask<ROOK>(b, from);
                break;
        }

        attack_mask &= not_friendly;
        while (attack_mask) {
            Square to = pop_lsb(attack_mask);
            MoveType mtype = b.piece_map[to] == NO_PIECE ? QUIET : CAPTURE;
            generate_move_if_legal(b, moves, from, to, mtype, NORMAL);
        }
    }
}

static inline void encode_pawn_moves(
    Board &b,
    std::vector<Move>& moves,
    Bitboard move_mask,
    Direction direction,
    MoveType mtype,
    MoveFlag mflag
) {
    while (move_mask) {
        Square to = pop_lsb(move_mask);
        Square from = to - direction;
        generate_move_if_legal(b, moves, from, to, mtype, mflag);
    }
}

static inline void encode_pawn_promo_moves(
    Board &b,
    std::vector<Move>& moves,
    Bitboard move_mask,
    Direction direction,
    MoveType mtype
) {
    while (move_mask) {
        Square to = pop_lsb(move_mask);
        Square from = to - direction;
        generate_move_if_legal(b, moves, from, to, mtype, PROMOTION_QUEEN);
        generate_move_if_legal(b, moves, from, to, mtype, PROMOTION_ROOK);
        generate_move_if_legal(b, moves, from, to, mtype, PROMOTION_BISHOP);
        generate_move_if_legal(b, moves, from, to, mtype, PROMOTION_KNIGHT);

    }
}

template<Color C>
static inline void generate_pawn_moves(Board& b, std::vector<Move>& moves) {
    // Compile-time constants derived from template
    constexpr Direction FWD              = C == WHITE ? NORTH : SOUTH;
    constexpr Direction FWD_FWD          = C == WHITE ? NORTH_NORTH : SOUTH_SOUTH;
    constexpr Direction FWD_RIGHT        = C == WHITE ? NORTHEAST: SOUTHWEST;
    constexpr Direction FWD_LEFT         = C == WHITE ? NORTHWEST: SOUTHEAST;
    constexpr Bitboard  PROMO_MASK       = C == WHITE ? RANK_7_MASK : RANK_2_MASK;
    constexpr Bitboard  DOUBLE_PUSH_MASK = C == WHITE ? RANK_4_MASK : RANK_5_MASK;

    Bitboard empty = ~b.occupied;
    Bitboard enemy_pieces = b.colors[C ^ 1];
    Bitboard pawns = b.pieces[C][PAWN];
    Bitboard promo_pawns = pawns & PROMO_MASK;
    Bitboard non_promo_pawns = pawns & ~PROMO_MASK;

    // No promotion
    Bitboard single_push    = shift<FWD>(non_promo_pawns) & empty;
    Bitboard double_push    = shift<FWD>(single_push) & empty & DOUBLE_PUSH_MASK;
    Bitboard right_capture  = shift<FWD_RIGHT>(non_promo_pawns) & enemy_pieces;
    Bitboard left_capture   = shift<FWD_LEFT>(non_promo_pawns) & enemy_pieces;
    
    Bitboard right_en_passant = 0;
    Bitboard left_en_passant = 0;
    if (b.en_passant_target != NO_SQUARE) {
        Bitboard en_passant_target_mask = get_mask(b.en_passant_target);
        right_en_passant    = shift<FWD_RIGHT>(non_promo_pawns) & en_passant_target_mask;
        left_en_passant     = shift<FWD_LEFT>(non_promo_pawns) & en_passant_target_mask;
    }

    // Promotion moves
    Bitboard single_push_promo      = shift<FWD>(promo_pawns) & empty;
    Bitboard right_capture_promo    = shift<FWD_RIGHT>(promo_pawns) & enemy_pieces;
    Bitboard left_capture_promo     = shift<FWD_LEFT>(promo_pawns) & enemy_pieces;

    // Encode and add moves to vector
    encode_pawn_promo_moves(b, moves, right_capture_promo, FWD_RIGHT, CAPTURE);
    encode_pawn_promo_moves(b, moves, left_capture_promo, FWD_LEFT, CAPTURE);
    encode_pawn_promo_moves(b, moves, single_push_promo, FWD, QUIET);

    encode_pawn_moves(b, moves, right_capture, FWD_RIGHT, CAPTURE, NORMAL);
    encode_pawn_moves(b, moves, left_capture, FWD_LEFT, CAPTURE, NORMAL);
    encode_pawn_moves(b, moves, right_en_passant, FWD_RIGHT, CAPTURE, EN_PASSANT);
    encode_pawn_moves(b, moves, left_en_passant, FWD_LEFT, CAPTURE, EN_PASSANT);
    encode_pawn_moves(b, moves, single_push, FWD, QUIET, NORMAL);
    encode_pawn_moves(b, moves, double_push, FWD_FWD, QUIET, NORMAL);
}

template <Color C>
static inline void generate_castling_moves(Board &b, std::vector<Move>& moves) {
    constexpr auto SHORT_CASTLING_RIGHTS = C == WHITE ? WHITE_SHORT : BLACK_SHORT;
    constexpr auto LONG_CASTLING_RIGHTS  = C == WHITE ? WHITE_LONG : BLACK_LONG;
    constexpr auto SHORT_CASTLE_PATH     = C == WHITE ? WHITE_SHORT_CASTLE_PATH : BLACK_SHORT_CASTLE_PATH;
    constexpr auto LONG_CASTLE_PATH      = C == WHITE ? WHITE_LONG_CASTLE_PATH : BLACK_LONG_CASTLE_PATH;
    constexpr auto SHORT_TO              = C == WHITE ? G1 : G8;
    constexpr auto LONG_TO               = C == WHITE ? C1 : C8;
    constexpr auto KING_SQUARE           = C == WHITE ? E1 : E8;

    // Castle path squares
    constexpr auto F_SQUARE = C == WHITE ? F1 : F8; // Short
    constexpr auto D_SQUARE = C == WHITE ? D1 : D8; // Long

    constexpr Color ATTACKER_SQUARE = C ^ 1;

    // Can't castle out of check
    if (is_square_attacked(b, ATTACKER_SQUARE, KING_SQUARE)) return;

    // Short castle
    if (
        (b.castling_rights & SHORT_CASTLING_RIGHTS) // Have castling rights for this side
        && ((b.occupied & SHORT_CASTLE_PATH) == 0)  // Path is clear
        && !is_square_attacked(b, ATTACKER_SQUARE, F_SQUARE) // King doesn't pass thru check
    ) {
        generate_move_if_legal(b, moves, KING_SQUARE, SHORT_TO, QUIET, CASTLE);
    } 
    
    // Long castle
    if (
        (b.castling_rights & LONG_CASTLING_RIGHTS)
        && ((b.occupied & LONG_CASTLE_PATH) == 0)
        && !is_square_attacked(b, ATTACKER_SQUARE, D_SQUARE)
    ) {
        generate_move_if_legal(b, moves, KING_SQUARE, LONG_TO, QUIET, CASTLE);
    }
}

// Main move generation function (exposed in header)
std::vector<Move> generate_moves(Board& b) {
    std::vector<Move> moves;
    moves.reserve(256);

    if (b.to_move == WHITE) {
        generate_pawn_moves<WHITE>(b, moves);
        generate_castling_moves<WHITE>(b, moves);
    } else {
        generate_pawn_moves<BLACK>(b, moves);
        generate_castling_moves<BLACK>(b, moves);
    }

    generate_piece_moves<BISHOP>(b, moves);
    generate_piece_moves<KNIGHT>(b, moves);
    generate_piece_moves<ROOK>(b, moves);
    generate_piece_moves<QUEEN>(b, moves);
    generate_piece_moves<KING>(b, moves);

    return moves;
}