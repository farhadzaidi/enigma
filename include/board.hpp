#pragma once

#include <cstdint>
#include <string>
#include <stack>

#include "types.hpp"
#include "state.hpp"

// Type definitions for board representation
using PieceBitboards    = std::array<std::array<Bitboard, NUM_PIECES>, NUM_COLORS>;
using ColorBitboards    = std::array<Bitboard, NUM_COLORS>;
using PieceMap          = std::array<Square, NUM_SQUARES>;
using KingSquares       = std::array<Square, NUM_COLORS>;

class Board {
public:
    // --- Board Representation ---
    PieceBitboards pieces;
    ColorBitboards colors;
    PieceMap piece_map;

    // Additional information 
    KingSquares king_squares;
    Bitboard occupied;

    // Board state information
    Color to_move;
    CastlingRights castling_rights;
    Square en_passant_target;
    int halfmoves;
    int fullmoves;

    // Stack of moves - useful for undoing moves
    std::stack<Move> moves;

    // Stack for tracking irreversible board state
    std::stack<State> states;

    Board();

    void remove_piece(Color color, Piece piece, Square square);
    void place_piece(Color color, Piece piece, Square square);

    void load_from_fen(const std::string& fen = START_POS_FEN);
    void print_board();
    void debug();

    Color get_color(Square square);

    void set_en_passant_target(Color color, Piece piece, Square from, Square to);
    int handle_capture(Square capture_square, Color moving_color, MoveFlag mflag);
    void handle_castle(Square castle_square);
    void update_castling_rights(Color color, Piece piece);
    void make_move(Move move);

    void unmake_move(Move move);
};