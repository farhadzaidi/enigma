#pragma once

#include <array>

#include "types.hpp"

using AttackMap = std::array<Bitboard, NUM_SQUARES>;
 
AttackMap bishop_attack_map;
AttackMap knight_attack_map;
AttackMap rook_attack_map;
AttackMap queen_attack_map;
AttackMap king_attack_map;

void compute_attack_maps();