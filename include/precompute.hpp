#pragma once

#include <bit>
#include <vector>
#include <cstdint>
#include <iostream>

#include "types.hpp"
#include "utils.hpp"
#include "random.hpp"

// NON-SLIDING PIECES

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

// SLIDING PIECES

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

static constexpr BlockerMap BISHOP_BLOCKER_MAP = []() {
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

static constexpr BlockerMap ROOK_BLOCKER_MAP = []() {
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

inline void compute_magic_numbers(const BlockerMap& blocker_map) {
    for (Square sq = 0; sq < NUM_SQUARES; sq++) {
        Bitboard blocker_mask = blocker_map[sq];
        int num_blockers = std::popcount(blocker_mask);
        std::size_t num_subsets = uint64_t{1} << num_blockers;

        while (true) {
            bool is_valid_magic = true;
            MagicNumber candidate = random_magic();
            std::vector<bool> used(num_subsets, false);

            Bitboard subset = blocker_mask;
            do {
                uint64_t index = (subset * candidate) >> (64 - num_blockers);
                if (used[index]) {
                    is_valid_magic = false;
                    break;
                }

                used[index] = true;
                subset = (subset - 1) & blocker_mask;
            } while (subset);

            if (is_valid_magic) {
                std::clog << candidate << "\n";
                break;
            }
        }
    }
}