#ifndef BOARD_HPP
#define BOARD_HPP

#include <cstdint>
#include <string>
#include <array>

class Board {
public:

    // --- Board Representation ---

    // Bitboards representing each piece type by color
    // (e.g. pieces[WHITE][PAWN])
    std::array<std::array<uint64_t, NUM_PIECES>, NUM_COLORS> pieces;
    // Bitboards representing ALL pieces of each color
    // (e.g. colors[WHITE])
    // uint64_t colors[NUM_COLORS];
    std::array<uint64_t, NUM_COLORS> colors;
    // Array used to lookup piece at a given square
    std::array<int, NUM_SQUARES> piece_lookup;
    // Array used to lookup color of piece at a given square
    std::array<int, NUM_SQUARES> color_lookup;

    int to_move;
    uint8_t castling_rights;
    uint64_t en_passant_target;
    int halfmoves;
    int fullmoves;

    Board();

    uint64_t get_occupied_squares();
    uint64_t get_empty_squares();
    void load_from_fen(const std::string& fen = START_POS_FEN);
    void make_move(uint16_t move);
    void unmake_move(uint16_t move);
    void print_board();
};

#endif