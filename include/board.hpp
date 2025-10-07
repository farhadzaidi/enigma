#pragma once

#include <string>
#include <stack>

#include "types.hpp"
#include "move.hpp"
#include "utils.hpp"
#include "precompute.hpp"

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

    State() :
        en_passant_target(NO_SQUARE),
        castling_rights(NO_CASTLING_RIGHTS),
        halfmoves(0),
        captured_piece(NO_PIECE) {}

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

    // These stacks are implemented as arrays using ply as a pointer to the top
    // They are useful for undoing moves
    int ply;
    std::array<Move, MAX_PLY> moves; // Keeps track of made moves
    std::array<State, MAX_PLY> states; // Keeps track of irreversible board state

    // ### PUBLIC API

    Board();
    void reset();
    void load_from_fen(const std::string& fen = START_POS_FEN);
    void print_board() const;
    void debug();
    void make_move(Move move);
    void unmake_move(Move move);
    bool in_check();

private:

    // ### HELPERS

    inline Color get_color(Square square) const {
        return (colors[BLACK] >> square) & uint64_t{1};
    }

    inline void place_piece(Color color, Piece piece, Square square) {
        // Create a mask based on the square of the piece and use bitwise OR to
        // place the piece on each respective bitboard
        Bitboard mask = get_mask(square);
        pieces[color][piece] |= mask;
        colors[color] |= mask;
        occupied |= mask;

        piece_map[square] = piece;
        if (piece == KING) {
            king_squares[color] = square;
        }
    }

    inline void remove_piece(Color color, Piece piece, Square square) {
        // Create a mask based on the square of the piece and use bitwise AND to
        // remove the piece from each respective bitboard
        Bitboard mask = ~get_mask(square);
        pieces[color][piece] &= mask;
        colors[color] &= mask;
        occupied &= mask;

        piece_map[square] = NO_PIECE;
        // No need to clear king square here as it will be updated in place_piece
    }

    inline void set_en_passant_target(Color color, Piece piece, Square from, Square to) {
        // White moves a pawn 2 squares north
        if (
            color == WHITE
            && piece == PAWN
            && get_rank(from) == RANK_2
            && get_rank(to) == RANK_4
        ) {
            en_passant_target = to - 8; // Directly behind the white pawn (south)
        } 
        
        // Black moves a pawn 2 squares south
        else if (
            color == BLACK
            && piece == PAWN
            && get_rank(from) == RANK_7
            && get_rank(to) == RANK_5
        ) {
            en_passant_target = to + 8; // Directly behind the black pawn (north)
        }

        else {
            en_passant_target = NO_SQUARE;
        }
    }


    inline Piece handle_capture(Square capture_square, Color moving_color, MoveFlag mflag) {
        halfmoves = 0;

        if (mflag == EN_PASSANT) {
            // In the case of en passant, the captured piece (pawn) is one rank
            // "behind" the "to" square. "Behind" can either be south or north
            // depending on whether the moving piece is white or black, respectively
            capture_square = moving_color == WHITE
                ? capture_square + SOUTH
                : capture_square + NORTH;
        }

        Piece captured_piece = piece_map[capture_square];
        remove_piece(moving_color ^ 1, captured_piece, capture_square);
        return captured_piece;
    }


    inline void handle_castle(Square castle_square) {
        switch (castle_square) {
            case C1: // White long castle
                remove_piece(WHITE, ROOK, A1);
                place_piece(WHITE, ROOK, D1);
                break;
            case G1: // White short castle
                remove_piece(WHITE, ROOK, H1);
                place_piece(WHITE, ROOK, F1);
                break;
            case C8: // Black long castle
                remove_piece(BLACK, ROOK, A8);
                place_piece(BLACK, ROOK, D8);
                break;
            case G8: // Black short castle
                remove_piece(BLACK, ROOK, H8);
                place_piece(BLACK, ROOK, F8);
                break;
        }
    }


    inline void update_castling_rights(Square from, Square to) {
        // Use precomputed lookup table to update castling rights
        castling_rights &= ~castling_rights_updates[from];
        castling_rights &= ~castling_rights_updates[to];
    }
};