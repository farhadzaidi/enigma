#ifndef BOARD_HPP
#define BOARD_HPP

#include <cstdint>
#include <string>
#include <array>
#include <stack>

#include "state.hpp"

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
    int castling_rights;
    int en_passant_target;
    int halfmoves;
    int fullmoves;

    std::stack<uint16_t> moves;
    std::stack<State> state_stack;

    Board();

    uint64_t get_occupied_squares();
    uint64_t get_empty_squares();
    void load_from_fen(const std::string& fen = START_POS_FEN);
    void remove_piece(int color, int piece, int square);
    void place_piece(int color, int piece, int square);
    void set_en_passant_target(int color, int piece, int from, int to);
    int handle_capture(int capture_square, int moving_color, int flag);
    void handle_castle(int castle_square);
    void handle_castling_rights(int color, int piece);
    void make_move(uint16_t move);
    void unmake_move(uint16_t move);



    void print_board();
};

#endif