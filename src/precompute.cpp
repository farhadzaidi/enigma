#include <array>

#include "precompute.hpp"
#include "types.hpp"
#include "utils.hpp"

static void compute_knight_attack_map(Square sq) {
    Bitboard mask = get_mask(sq);
    Bitboard map = 0;

    map |= shift_north(shift_north(shift_east(mask)));
    map |= shift_north(shift_north(shift_west(mask)));
    map |= shift_south(shift_south(shift_east(mask)));
    map |= shift_south(shift_south(shift_west(mask)));
    map |= shift_east(shift_east(shift_north(mask)));
    map |= shift_east(shift_east(shift_south(mask)));
    map |= shift_west(shift_west(shift_south(mask)));
    map |= shift_west(shift_west(shift_south(mask)));

    knight_attack_map[sq] = map;
}

static void compute_king_attack_map(Square sq) {
    Bitboard mask = get_mask(sq);
    Bitboard map = 0;

    map |= shift_north(map);
    map |= shift_south(map);
    map |= shift_east(map);
    map |= shift_west(map);
    map |= shift_northeast(map);
    map |= shift_northwest(map);
    map |= shift_southeast(map);
    map |= shift_southwest(map);

    king_attack_map[sq] = map;
}


void compute_attack_maps() {
    // Knight Attack Map
    for (int sq = 0; sq < NUM_SQUARES; sq++) {
        compute_knight_attack_map(sq);
        compute_king_attack_map(sq);
        
        compute_bishop_attack_map(sq);
        compute_rook_attack_map(sq);
        queen_attack_map[sq] = bishop_attack_map[sq] | rook_attack_map[sq];
    }
}