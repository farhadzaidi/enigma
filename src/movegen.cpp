#include <vector>
#include <span>

#include "movegen.hpp"
#include "types.hpp"
#include "utils.hpp"
#include "precompute.hpp"

// NON-PAWN MOVES

template <Piece P>
static inline Bitboard generate_sliding_attack_mask(const Board& b, Square from) {
    // Assign constants based on sliding piece type
    constexpr bool is_bishop = P == BISHOP;
    const auto& attack_table = is_bishop ? BISHOP_ATTACK_TABLE : ROOK_ATTACK_TABLE; 
    const auto& blocker_map  = is_bishop ? BISHOP_BLOCKER_MAP : ROOK_BLOCKER_MAP;
    const auto& magic        = is_bishop ? BISHOP_MAGIC : ROOK_MAGIC;
    const auto& offset       = is_bishop ? BISHOP_OFFSET : ROOK_OFFSET;
        
    // Look up sliding piece attacks from attack table based on blocker pattern
    Bitboard blockers = b.occupied & blocker_map[from];
    int num_blockers = std::popcount(blocker_map[from]);
    size_t index = (blockers * magic[from]) >> (64 - num_blockers);
    return attack_table[offset[from] + index];
}

template <Piece P>
static inline void generate_piece_moves(const Board& b, std::vector<Move>& moves) {
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
            moves.push_back(encode_move(from, to, mtype, NORMAL));
        }
    }
}

// PAWN MOVES

static inline void encode_pawn_moves(
    std::vector<Move>& moves,
    Bitboard move_mask,
    Direction direction,
    MoveType mtype,
    MoveFlag mflag
) {
    while (move_mask) {
        Square to = pop_lsb(move_mask);
        Square from = to - direction;
        moves.push_back(encode_move(from, to, mtype, mflag));
    }
}

static inline void encode_pawn_promo_moves(
    std::vector<Move>& moves,
    Bitboard move_mask,
    Direction direction,
    MoveType mtype,
    MoveFlag mflag
) {
    while (move_mask) {
        Square to = pop_lsb(move_mask);
        Square from = to - direction;
        moves.push_back(encode_move(from, to, mtype, PROMOTION_QUEEN));
        moves.push_back(encode_move(from, to, mtype, PROMOTION_ROOK));
        moves.push_back(encode_move(from, to, mtype, PROMOTION_BISHOP));
        moves.push_back(encode_move(from, to, mtype, PROMOTION_KNIGHT));
    }
}

template<Color C>
static inline void generate_pawn_moves(const Board& b, std::vector<Move>& moves) {
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
    Bitboard double_push    = shift<FWD_FWD>(single_push) & empty & DOUBLE_PUSH_MASK;
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
    encode_pawn_promo_moves(moves, right_capture_promo, FWD_RIGHT, CAPTURE, NORMAL);
    encode_pawn_promo_moves(moves, left_capture_promo, FWD_LEFT, CAPTURE, NORMAL);
    encode_pawn_promo_moves(moves, single_push_promo, FWD, QUIET, NORMAL);

    encode_pawn_moves(moves, right_capture, FWD_RIGHT, CAPTURE, NORMAL);
    encode_pawn_moves(moves, left_capture, FWD_LEFT, CAPTURE, NORMAL);
    encode_pawn_moves(moves, right_en_passant, FWD_RIGHT, CAPTURE, EN_PASSANT);
    encode_pawn_moves(moves, left_en_passant, FWD_LEFT, CAPTURE, EN_PASSANT);
    encode_pawn_moves(moves, single_push, FWD, QUIET, NORMAL);
    encode_pawn_moves(moves, double_push, FWD_FWD, QUIET, NORMAL);
}

// CASTLING MOVES

static inline void generate_castling_moves() {
    // TODO
}

// Main move generation function (exposed in header)
std::vector<Move> generate_moves(Board& b) {
    
}