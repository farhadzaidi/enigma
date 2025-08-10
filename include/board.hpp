#ifndef BOARD_HPP
#define BOARD_HPP

#include <cstdint>
#include <string>

#define START_POS_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1"

class Board {
public:
    // Bitboards representing each piece type by color
    // (e.g. pieces[WHITE][PAWN])
    uint64_t pieces[2][6];

    // Bitboards representing ALL pieces of each color
    // (e.g. colors[WHITE])
    uint64_t colors[2];

    int to_move;
    uint8_t castling_rights;
    uint64_t en_passant_target;
    int halfmoves;
    int fullmoves;

    Board();

    static uint64_t get_square_index(int rank, int file);
    static uint64_t algebraic_to_index(const std::string &square);

    uint64_t get_occupied_squares();
    uint64_t get_empty_squares();
    void load_from_fen(const std::string &fen = START_POS_FEN);
};

#endif