#pragma once

#include <bit>
#include <vector>
#include <cstdint>
#include <iostream>
#include <span>

#include "types.hpp"
#include "utils.hpp"
#include "random.hpp"

using AttackMap         = std::array<Bitboard, NUM_SQUARES>;
using BlockerMap        = std::array<Bitboard, NUM_SQUARES>;

// We can precompute castling rights updates to make it much faster during make move.
// This lookup table keeps track of which castling rights are lost when a piece
// moves from or to that square. 
constexpr auto castling_rights_updates = []() {
    std::array<CastlingRights, NUM_SQUARES> castling_rights_updates = {NO_CASTLING_RIGHTS};
    castling_rights_updates[E1] = WHITE_SHORT | WHITE_LONG;
    castling_rights_updates[H1] = WHITE_SHORT;
    castling_rights_updates[A1] = WHITE_LONG;
    castling_rights_updates[E8] = BLACK_SHORT | BLACK_LONG;
    castling_rights_updates[H8] = BLACK_SHORT;
    castling_rights_updates[A8] = BLACK_LONG;
    return castling_rights_updates;
}();

// NON-SLIDING PIECES

// Straightforward attack map generation: from each square, we just try going
// in every direction that the piece can go in and union the result of all
// directions
// Shift functions ensure that there is no wrap-around from a file to h file and
// vice versa. Furthemore, bitshift behavior naturally handles going off the board.

constexpr AttackMap KNIGHT_ATTACK_MAP = []() {
    AttackMap map{};
    for (Square sq = 0; sq < NUM_SQUARES; sq++) {
        Bitboard mask = get_mask(sq);
        map[sq] = 
            shift<NORTHEAST>(shift<NORTH>(mask)) |
            shift<NORTHEAST>(shift<EAST> (mask)) |
            shift<NORTHWEST>(shift<NORTH>(mask)) |
            shift<NORTHWEST>(shift<WEST> (mask)) |
            shift<SOUTHEAST>(shift<SOUTH>(mask)) |
            shift<SOUTHEAST>(shift<EAST> (mask)) |
            shift<SOUTHWEST>(shift<SOUTH>(mask)) |
            shift<SOUTHWEST>(shift<WEST> (mask));
    }
    return map;
}();

constexpr AttackMap KING_ATTACK_MAP = []() {
    AttackMap map{};
    for (Square sq = 0; sq < NUM_SQUARES; sq++) {
        Bitboard mask = get_mask(sq);
        map[sq] = shift<NORTH>    (mask) |
                  shift<SOUTH>    (mask) |
                  shift<EAST>     (mask) |
                  shift<WEST>     (mask) |
                  shift<NORTHEAST>(mask) |
                  shift<NORTHWEST>(mask) |
                  shift<SOUTHEAST>(mask) |
                  shift<SOUTHWEST>(mask);
    }
    return map;
}();


// Array of attack maps used to check if a square is attacked by pawns
// Indexed by attacking color (e.g. PAWN_ATTACK_MAP[BLACK] checks if
// that square is attacked by black pawns)
constexpr auto PAWN_ATTACK_MAPS = []() {
    AttackMap white_map{}; // White attacking pawns
    AttackMap black_map{}; // Black attacking pawns
    for (Square sq = 0; sq < NUM_SQUARES; sq++) {
        Bitboard mask = get_mask(sq);
        white_map[sq] = shift<SOUTHEAST>(mask) | shift<SOUTHWEST>(mask);
        black_map[sq] = shift<NORTHEAST>(mask) | shift<NORTHWEST>(mask);
    }

    return std::array<AttackMap, NUM_COLORS>{white_map, black_map};
}();

// SLIDING PIECES

// Helper function that creates a mask from a given square and shifts that mask in
// a given direction until it encounters some blocker or goes off the board
// Returns a mask with nonblocked squares on the board set to 1
template <Direction D>
static constexpr Bitboard walk(Square sq, Bitboard blockers = 0) {
    Bitboard attack = 0;
    Bitboard mask = shift<D>(get_mask(sq));
    while (mask && ((mask & blockers) == 0)) {
        attack |= mask;
        mask = shift<D>(mask);
    }
    return attack;
}

// Map of all blocker squares for each square that the bishop is on
// So each entry contains a mask of all blocker squares for the bishop on that square
// Doesn't include edges since a piece on the edge isn't blocking another square
constexpr BlockerMap BISHOP_BLOCKER_MAP = []() {
    BlockerMap map{};
    for (Square sq = 0; sq < NUM_SQUARES; sq++) {
        map[sq] =
            walk<NORTHEAST>(sq, RANK_8_MASK | H_FILE_MASK) |
            walk<NORTHWEST>(sq, RANK_8_MASK | A_FILE_MASK) |
            walk<SOUTHEAST>(sq, RANK_1_MASK | H_FILE_MASK) |
            walk<SOUTHWEST>(sq, RANK_1_MASK | A_FILE_MASK);
    }
    return map;
}();

// Same thing as above but for rook
constexpr BlockerMap ROOK_BLOCKER_MAP = []() {
    BlockerMap map{};
    for (Square sq = 0; sq < NUM_SQUARES; sq++) {
        map[sq] =
            walk<NORTH>(sq, RANK_8_MASK) |
            walk<SOUTH>(sq, RANK_1_MASK) |
            walk<EAST> (sq, H_FILE_MASK) |
            walk<WEST> (sq, A_FILE_MASK);
    }
    return map;
}();

// Helper function used to compute sizes for rook and bishop attack tables
static constexpr size_t compute_attack_table_size(const BlockerMap& blocker_map) {
    size_t size = 0;

    for (Square sq = 0; sq < NUM_SQUARES; sq++) {
        Bitboard blocker_mask = blocker_map[sq];
        // There are 2^N blocker configurations for this square where N is the
        // number of possible blocker squares i.e. popcount(blocker_mask)
        size += std::size_t{1} << std::popcount(blocker_mask);
    }

    return size;
}
static constexpr size_t BISHOP_ATTACK_TABLE_SIZE = compute_attack_table_size(BISHOP_BLOCKER_MAP);
static constexpr size_t ROOK_ATTACK_TABLE_SIZE = compute_attack_table_size(ROOK_BLOCKER_MAP);

// Helper function to compute offset for indexing into attack tables for each square
// Very similar logic to compute_attack_table_sizes but here we're saving cumulative
// sizes as we loop through all the squares
static constexpr std::array<size_t, NUM_SQUARES> compute_offset(const BlockerMap& blocker_map) {
    size_t size = 0;
    std::array<size_t, NUM_SQUARES> offset;

    for (Square sq = 0; sq < NUM_SQUARES; sq++) {
        offset[sq] = size;
        Bitboard blocker_mask = blocker_map[sq];
        size += 1 << std::popcount(blocker_mask);
    }
    
    return offset;
};
constexpr auto BISHOP_OFFSET = compute_offset(BISHOP_BLOCKER_MAP);
constexpr auto ROOK_OFFSET = compute_offset(ROOK_BLOCKER_MAP);

static inline const auto _BISHOP_ATTACK_TABLE = []() {
    std::array<Bitboard, BISHOP_ATTACK_TABLE_SIZE> table{};
    for (Square sq = 0; sq < NUM_SQUARES; sq++) {
        Bitboard blocker_mask = BISHOP_BLOCKER_MAP[sq];

        // Neat bit manipulation trick to enumerate all subsets of the blocker mask
        // i.e. get all possible blocker configurations
        for (Bitboard subset = blocker_mask;; subset = (subset - 1) & blocker_mask) {
            // Compute the attack mask by walking in each direction until we encounter
            // a blocker. Since we want to consider captures, we stop AFTER the blocker
            // which is why we pass the blocker mask shifted one step forward.
            Bitboard attack_mask =
                walk<NORTHEAST>(sq, shift<NORTHEAST>(subset)) |
                walk<NORTHWEST>(sq, shift<NORTHWEST>(subset)) |
                walk<SOUTHEAST>(sq, shift<SOUTHEAST>(subset)) |
                walk<SOUTHWEST>(sq, shift<SOUTHWEST>(subset));

            // Get index by either using PEXT or magic number
            size_t index = get_attack_table_index(subset, blocker_mask, BISHOP_MAGIC[sq]);

            // Index into the attack table using the offset and cache the attack mask
            table[BISHOP_OFFSET[sq] + index] = attack_mask;

            // Terminating condition
            if (subset == 0) {
                break;
            }
        };
    }

    return table;
}();

static inline const auto _ROOK_ATTACK_TABLE = []() {
    // Same logic as bishop attack table but using rook constants
    std::array<Bitboard, ROOK_ATTACK_TABLE_SIZE> table{};
    for (Square sq = 0; sq < NUM_SQUARES; sq++) {
        Bitboard blocker_mask = ROOK_BLOCKER_MAP[sq];
        int num_blockers = std::popcount(blocker_mask);

        for (Bitboard subset = blocker_mask;; subset = (subset - 1) & blocker_mask) {
            Bitboard attack_mask =
                walk<NORTH>(sq, shift<NORTH>(subset)) |
                walk<SOUTH>(sq, shift<SOUTH>(subset)) |
                walk<EAST> (sq, shift<EAST> (subset)) |
                walk<WEST> (sq, shift<WEST> (subset));
 
            size_t index = get_attack_table_index(subset, blocker_mask, ROOK_MAGIC[sq]);
            table[ROOK_OFFSET[sq] + index] = attack_mask;

            if (subset == 0) {
                break;
            }
        }
    }

    return table;
}();

// Expose attack tables as std::span since they have different types due to different sizes
inline const std::span<const Bitboard> BISHOP_ATTACK_TABLE{_BISHOP_ATTACK_TABLE};
inline const std::span<const Bitboard> ROOK_ATTACK_TABLE{_ROOK_ATTACK_TABLE};

// This function is used to compute magic numbers which are useful for generating
// indices for rook and bishop attack tables.
// Note that the source code contains hardcoded values generated using this function,
// but this function is available incase these values ever need to be regenerated.
inline void compute_magic_numbers(const BlockerMap& blocker_map) {
    for (Square sq = 0; sq < NUM_SQUARES; sq++) {
        Bitboard blocker_mask = blocker_map[sq];
        int num_blockers = std::popcount(blocker_mask);
        std::size_t num_subsets = uint64_t{1} << num_blockers;

        // General algorithm for computing magic for each square goes like this:
        // 1. Generate a random number to try and assume it's valid
        // 2. Keep an array of used indices
        // 3. Generate the index for a given subset using the candidate magic number
        // 4. Check if the index has already been marked used 
            // If so, we have a collision --> restart with an empty used array and new candidate
            // Else, mark this index as used and keep going
        // 5. If a number generates all unique indices, then this is a valid magic number
        while (true) {
            uint64_t candidate = random_magic();
            std::vector<bool> used(num_subsets, false);
            bool is_valid_magic = true;

            for (Bitboard subset = blocker_mask;; subset = (subset - 1) & blocker_mask) {
                size_t index = (subset * candidate) >> (NUM_SQUARES - num_blockers);
                if (used[index]) {
                    is_valid_magic = false;
                    break;
                }
                used[index] = true;

                if (subset == 0) {
                    break;
                }
            }

            if (is_valid_magic) {
                std::clog << candidate << "\n";
                break;
            }
        }
    }
}

// Ray masks from each square to the end of the board (not including the square)
using RayMap = std::array<Bitboard, NUM_SQUARES>;

template <Direction D>
static constexpr RayMap compute_rays() {
    RayMap ray_map{};
    for (Square sq = 0; sq < NUM_SQUARES; sq++) {
        ray_map[sq] = walk<D>(sq);
    }
    return ray_map;
};

constexpr RayMap NORTH_RAY_MAP     = compute_rays<NORTH>();
constexpr RayMap SOUTH_RAY_MAP     = compute_rays<SOUTH>();
constexpr RayMap EAST_RAY_MAP      = compute_rays<EAST>();
constexpr RayMap WEST_RAY_MAP      = compute_rays<WEST>();
constexpr RayMap NORTHEAST_RAY_MAP = compute_rays<NORTHEAST>();
constexpr RayMap NORTHWEST_RAY_MAP = compute_rays<NORTHWEST>();
constexpr RayMap SOUTHEAST_RAY_MAP = compute_rays<SOUTHEAST>();
constexpr RayMap SOUTHWEST_RAY_MAP = compute_rays<SOUTHWEST>();
constexpr RayMap EMPTY_RAY_MAP{};



// Using custom absolute value function since std::abs is not constexpr
constexpr int abs_val(int x) { return x > 0 ? x : -x;}

// Get the direction from square a to square b if they are collinear, else return NO_DIRECTION
constexpr Direction get_direction(Square a, Square b) {
    if (a == b) return NO_DIRECTION;

    int a_rank = get_rank(a);
    int a_file = get_file(a);

    int b_rank = get_rank(b);
    int b_file = get_file(b);

    // Check collinearity
    int dx = abs_val(a_file - b_file);
    int dy = abs_val(a_rank - b_rank);
    bool are_colinear = (
        dx == 0 || // same file
        dy == 0 || // same rank
        abs_val(dx) == abs_val(dy) // same diagonal
    );
    if (!are_colinear) return NO_DIRECTION;

    int vertical = a_rank != b_rank
        ? (a_rank < b_rank ? NORTH : SOUTH)
        : NO_DIRECTION;
    
    int horizontal = a_file != b_file
        ? (a_file < b_file ? EAST : WEST)
        : NO_DIRECTION;
    
    return vertical + horizontal;
}

// Maps directions to ray maps since we can't index with directions
constexpr const RayMap& get_ray_map(Direction direction) {
    switch (direction) {
        case NORTH:        return NORTH_RAY_MAP;
        case SOUTH:        return SOUTH_RAY_MAP;
        case EAST:         return EAST_RAY_MAP;
        case WEST:         return WEST_RAY_MAP;
        case NORTHEAST:    return NORTHEAST_RAY_MAP;
        case NORTHWEST:    return NORTHWEST_RAY_MAP;
        case SOUTHEAST:    return SOUTHEAST_RAY_MAP;
        case SOUTHWEST:    return SOUTHWEST_RAY_MAP;
        default:           return EMPTY_RAY_MAP;
    }
}

template <Direction D>
constexpr const RayMap& get_ray_map() {
    if constexpr (D == NORTH)     return NORTH_RAY_MAP;
    if constexpr (D == SOUTH)     return SOUTH_RAY_MAP;
    if constexpr (D == EAST)      return EAST_RAY_MAP;
    if constexpr (D == WEST)      return WEST_RAY_MAP;
    if constexpr (D == NORTHEAST) return NORTHEAST_RAY_MAP;
    if constexpr (D == NORTHWEST) return NORTHWEST_RAY_MAP;
    if constexpr (D == SOUTHEAST) return SOUTHEAST_RAY_MAP;
    if constexpr (D == SOUTHWEST) return SOUTHWEST_RAY_MAP;
    else                          return EMPTY_RAY_MAP;
}

// Computes lines from square a to square b including square b
constexpr auto LINES = []() {
    std::array<std::array<Bitboard, NUM_SQUARES>, NUM_SQUARES> lines{};
    for (Square a = 0; a < NUM_SQUARES; a++) {
        for (Square b = 0; b < NUM_SQUARES; b++) {
            Direction towards_b = get_direction(a, b);
            if (towards_b == NO_DIRECTION) {
                lines[a][b] = uint64_t{0};
                continue;
            }

            Bitboard ray_towards_b = get_ray_map(towards_b)[a];

            Direction towards_a = get_direction(b, a);
            Bitboard ray_towards_a = get_ray_map(towards_a)[b];

            // Intersect both rays, leaving only squares between the a and b
            lines[a][b] = (ray_towards_b & ray_towards_a) | get_mask(b); // Include square b
        }
    }

    return lines;
}();