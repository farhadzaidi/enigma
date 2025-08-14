#pragma once

#include <vector>

#include "types.hpp"
#include "board.hpp"
#include "move.hpp"

std::vector<Move> generate_legal_moves(Board& b);
std::vector<Move> generate_pseudolegal_moves(Board& b);
std::vector<Move> generate_pawn_moves(Board& b);
std::vector<Move> generate_sliding_moves(Board& b);
std::vector<Move> generate_non_sliding_moves(Board& b);
std::vector<Move> generate_castling_moves(Board& b);
