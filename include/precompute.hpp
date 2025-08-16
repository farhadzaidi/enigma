#pragma once

#include "types.hpp"
#include "utils.hpp"

using AttackMap = std::array<Bitboard, NUM_SQUARES>;

template <Direction D>
static constexpr Bitboard explore(Square sq) {
    Bitboard attack = 0;
    Bitboard mask = shift<D>(get_mask(sq));
    while (mask) {
        attack |= mask;
        mask = shift<D>(mask);
    }
    return attack;
}

constexpr AttackMap BISHOP_ATTACK_MAP = []() {
    AttackMap map{};
    for (Square sq = 0; sq < NUM_SQUARES; sq++) {
        map[sq] =
            explore<NORTHEAST>(sq) |
            explore<NORTHWEST>(sq) |
            explore<SOUTHEAST>(sq) |
            explore<SOUTHWEST>(sq);
    }
    return map;
}();

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


constexpr AttackMap ROOK_ATTACK_MAP = []() {
    AttackMap map{};
    for (Square sq = 0; sq < NUM_SQUARES; sq++) {
        map[sq] =
            explore<NORTH>(sq) |
            explore<SOUTH>(sq) |
            explore<EAST> (sq) |
            explore<WEST> (sq);
    }
    return map;
}();

constexpr AttackMap QUEEN_ATTACK_MAP = []() {
    AttackMap map{};
    for (Square sq = 0; sq < NUM_SQUARES; sq++) {
        map[sq] = BISHOP_ATTACK_MAP[sq] | ROOK_ATTACK_MAP[sq];
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