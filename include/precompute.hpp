#pragma once

#include <bit>
#include <vector>
#include <cstdint>
#include <iostream>
#include <span>

#include "types.hpp"
#include "utils.hpp"
#include "random.hpp"

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
constexpr std::array<AttackMap, NUM_COLORS> PAWN_ATTACK_MAPS = []() {
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
static constexpr Bitboard walk(Square sq, Bitboard blockers) {
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

static constexpr auto _BISHOP_ATTACK_TABLE = []() {
    std::array<Bitboard, BISHOP_ATTACK_TABLE_SIZE> table{};
    for (Square sq = 0; sq < NUM_SQUARES; sq++) {
        Bitboard blocker_mask = BISHOP_BLOCKER_MAP[sq];
        int num_blockers = std::popcount(blocker_mask);

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

            // Compute index using the magic number for this square and only retain
            // the relevant bits
            size_t index = (subset * BISHOP_MAGIC[sq]) >> (64 - num_blockers);

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

// Too big to be computed at compile time
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
 
            size_t index = (subset * ROOK_MAGIC[sq]) >> (64 - num_blockers);
            table[ROOK_OFFSET[sq] + index] = attack_mask;

            if (subset == 0) {
                break;
            }
        }
    }

    return table;
}();

// Expose attack tables as std::span since they have different types due to different sizes
constexpr std::span<const Bitboard> BISHOP_ATTACK_TABLE{_BISHOP_ATTACK_TABLE};
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