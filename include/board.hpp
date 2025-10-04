#pragma once

#include <string>
#include <stack>

#include "types.hpp"

// Type definitions for board representation
using PieceBitboards    = std::array<std::array<Bitboard, NUM_PIECES>, NUM_COLORS>;
using ColorBitboards    = std::array<Bitboard, NUM_COLORS>;
using PieceMap          = std::array<Piece, NUM_SQUARES>;
using KingSquares       = std::array<Square, NUM_COLORS>;

// This struct contains important board state information which is useful for undoing moves
// These attributes are overwritten when making a move and unable to be restored from the move encoding
struct State {
    Square en_passant_target;
    CastlingRights castling_rights;
    uint8_t halfmoves; // Truncating from int to U8 to save space
    Piece captured_piece;

    State(Square ep, CastlingRights cr, uint8_t hm, Piece cp) :
        en_passant_target(ep),
        castling_rights(cr),
        halfmoves(hm),
        captured_piece(cp) {}
};

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
    void reset();

    void remove_piece(Color color, Piece piece, Square square);
    void place_piece(Color color, Piece piece, Square square);

    Color get_color(Square square) const;
    void load_from_fen(const std::string& fen = START_POS_FEN);
    void print_board() const;
    void debug();


    void set_en_passant_target(Color color, Piece piece, Square from, Square to);
    Piece handle_capture(Square capture_square, Color moving_color, MoveFlag mflag);
    void handle_castle(Square castle_square);
    void update_castling_rights(Color color, Piece piece);
    void make_move(Move move);
    void unmake_move(Move move);
};