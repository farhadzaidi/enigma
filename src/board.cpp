#include <cctype>
#include <vector>
#include <sstream>
#include <iostream>
#include <cassert>

#include "constants.hpp"
#include "board.hpp"
#include "utils.hpp"
#include "move.hpp"

Board::Board() {
    pieces[WHITE].fill(EMPTY_BITBOARD);
    pieces[BLACK].fill(EMPTY_BITBOARD);
    colors.fill(EMPTY_BITBOARD);

    piece_map.fill(NO_PIECE);
    king_squares.fill(NO_SQUARE);

    occupied = EMPTY_BITBOARD;
    to_move = NO_COLOR;
    castling_rights = NO_CASTLING_RIGHTS;
    en_passant_target = NO_SQUARE;
    halfmoves = 0;
    fullmoves = 0;
}

void Board::remove_piece(Color color, Piece piece, Square square) {
    // Create a mask based on the square of the piece and use bitwise AND to
    // remove the piece from each respective bitboard
    Mask mask = ~get_mask(square);
    pieces[color][piece] &= mask;
    colors[color] &= mask;
    occupied &= mask;

    piece_map[square] = NO_PIECE;
    // No need to king square here as it will be updated in place_piece
}

void Board::place_piece(Color color, Piece piece, Square square) {
    // create a mask based on the square of the piece and use bitwise OR to
    // place the piece on each respective bitboard
    Mask mask = get_mask(square);
    pieces[color][piece] |= mask;
    colors[color] |= mask;
    occupied |= mask;

    piece_map[square] = piece;
    if (piece == KING) {
        king_squares[color] = square;
    }
}

void Board::load_from_fen(const std::string& fen) {
    std::clog << "\n---- Loading board from FEN ----\n";
    std::clog << "FEN: " << fen << "\n";

    std::vector<std::string> parts;
    std::stringstream ss(fen);
    std::string item;

    // Split the fen string using a space as the delimiter
    while(std::getline(ss, item, ' ')) {
        parts.push_back(item);
    }

    std::string position            = parts[0];
    std::string to_move             = parts[1];
    std::string castling_rights     = parts[2];
    std::string en_passant_target   = parts[3];
    std::string halfmoves           = parts[4];
    std::string fullmoves           = parts[5];

    // Set up position starting from top left
    std::clog << "1. Setting up position\n";
    int rank = 7;
    int file = 0;
    for (char c : position) {
        // End of rank; go down one
        if (c == '/') {
            rank -= 1;
            file = 0;
            continue;
        }

        // Number indicating how many empty squares in the file until the next piece
        if (std::isdigit(c)) {
            file += c - '0';
            continue;
        }

        // Must be a piece
        Square square = get_square(rank, file);
        Color color = std::isupper(c) ? WHITE : BLACK;
        Piece piece;

        switch (std::toupper(c)) {
            case 'P':
                piece = PAWN;
                break;
            case 'B':
                piece = BISHOP;
                break;
            case 'N':
                piece = KNIGHT;
                break;
            case 'R':
                piece = ROOK; 
                break;
            case 'Q':
                piece = QUEEN;
                break;
            case 'K':
                piece = KING;
                break;
        }

        place_piece(color, piece, square);
        file++;
    }

    // Side to move
    std::clog << "2. Setting side to move\n";
    this->to_move = to_move == "w" ? WHITE : BLACK;

    // Castling rights
    for (char c : castling_rights) {
        switch (c) {
            case 'K':
                this->castling_rights |= WHITE_SHORT;
                break;
            case 'Q':
                this->castling_rights |= WHITE_LONG;
                break;
            case 'k':
                this->castling_rights |= BLACK_SHORT;
                break;
            case 'q':
                this->castling_rights |= BLACK_LONG;
                break;
        }
    }

    // En passant target square
    std::clog << "3. Setting en passant target square\n";
    if (en_passant_target != "-") {
        this->en_passant_target = algebraic_to_index(en_passant_target);
    }

    // Halfmoves
    std::clog << "4. Setting halfmoves\n";
    this->halfmoves = std::stoi(halfmoves);

    // Fullmoves
    std::clog << "5. Setting fullmoves\n\n";
    this->fullmoves = std::stoi(fullmoves);
}

Color Board::get_color(Square square) {
    return (colors[BLACK] >> square) & uint64_t{1};
}


void Board::set_en_passant_target(Color color, Piece piece, Square from, Square to) {
    // White moves a pawn 2 squares north
    if (
        color == WHITE
        && piece == PAWN
        && get_rank(from) == SECOND_RANK
        && get_rank(to) == FOURTH_RANK
    ) {
        en_passant_target = to - 8; // Directly behind the white pawn (south)
    } 
    
    // Black moves a pawn 2 squares south
    else if (
        color == BLACK
        && piece == PAWN
        && get_rank(from) == SEVENTH_RANK
        && get_rank(to) == FIFTH_RANK
    ) {
        en_passant_target = to + 8; // Directly behind the black pawn (north)
    }

    else {
        en_passant_target = NO_SQUARE;
    }
}

int Board::handle_capture(Square capture_square, Color moving_color, MoveComponent flag) {
    halfmoves = 0;

    if (flag == EN_PASSANT) {
        // In the case of en passant, the captured piece (pawn) is one rank
        // "behind" the "to" square. "Behind" can either be south or north
        // depending on whether the moving piece is white or black, respectively
        capture_square = moving_color == WHITE
            ? capture_square - 8  // 1 step south
            : capture_square + 8; // 1 step north
    }

    int captured_piece = piece_map[capture_square];
    remove_piece(!moving_color, captured_piece, capture_square);
    return captured_piece;
}

void Board::handle_castle(Square castle_square) {
    switch (castle_square) {
        case c1: // White long castle
            remove_piece(WHITE, ROOK, a1);
            place_piece(WHITE, ROOK, d1);
            break;
        case g1: // White short castle
            remove_piece(WHITE, ROOK, h1);
            place_piece(WHITE, ROOK, f1);
            break;
        case c8: // Black long castle
            remove_piece(BLACK, ROOK, a8);
            place_piece(BLACK, ROOK, d8);
            break;
        case g8: // Black short castle
            remove_piece(BLACK, ROOK, h8);
            place_piece(BLACK, ROOK, f8);
            break;
    }
}

void Board::update_castling_rights(Color color, Piece piece) {
    // Castling rights checks:
    // If the king moves --> revoke all castling rights for that color
    // If any corner rook is not in its starting position --> revoke castling
    // rights for that color and side

    if (piece == KING && color == WHITE) {
        castling_rights &= ~WHITE_SHORT;
        castling_rights &= ~WHITE_LONG;
    }

    if (piece == KING && color == BLACK) {
        castling_rights &= ~BLACK_SHORT;
        castling_rights &= ~BLACK_LONG;
    }

    // King-side rook
    bool white_king_side_rook_moved = (pieces[WHITE][ROOK] & get_mask(h1)) == 0;
    if (white_king_side_rook_moved) {
        castling_rights &= ~WHITE_SHORT;
    }

    // Queen-side rook
    bool white_queen_side_rook_moved = (pieces[WHITE][ROOK] & get_mask(a1)) == 0;
    if (white_queen_side_rook_moved) {
        castling_rights &= ~WHITE_LONG;
    }

    // King-side rook
    bool black_king_side_rook_moved = (pieces[BLACK][ROOK] & get_mask(h8)) == 0;
    if (black_king_side_rook_moved) {
        castling_rights &= ~BLACK_SHORT;
    }

    // Queen-side rook
    bool black_queen_side_rook_moved = (pieces[BLACK][ROOK] & get_mask(a8)) == 0;
    if (black_queen_side_rook_moved) {
        castling_rights &= ~BLACK_LONG;
    }
}

void Board::make_move(Move move) {
    // Preserve irreversible board state before making the move
    State state(en_passant_target, castling_rights, halfmoves, NO_PIECE);

    int from    = get_from(move);
    int to      = get_to(move);
    int mtype   = get_mtype(move);
    int flag    = get_flag(move);

    int moving_piece = piece_map[from];
    int moving_color = to_move;

    // Update move clocks
    halfmoves++;
    if (moving_piece == PAWN) halfmoves = 0;
    if (moving_color == BLACK) fullmoves++;

    set_en_passant_target(moving_color, moving_piece, from, to);
    remove_piece(moving_color, moving_piece, from);

    // Handle capture logic including en passant
    if (mtype == CAPTURE) {
        state.captured_piece = handle_capture(to, moving_color, flag);
    }

    // Check if the move is a promotion; if so, update the moving piece
    switch (flag) {
        case PROMOTION_BISHOP:
            moving_piece = BISHOP;
            break;
        case PROMOTION_KNIGHT:
            moving_piece = KNIGHT;
            break;
        case PROMOTION_ROOK:
            moving_piece = ROOK;
            break;
        case PROMOTION_QUEEN:
            moving_piece = QUEEN;
            break;
    }

    // After changing moving_piece (in the case of a promotion), we can now
    // place the piece on the "to" square
    place_piece(moving_color, moving_piece, to);

    if (flag == CASTLE) {
        handle_castle(to);
    }
    update_castling_rights(moving_color, moving_piece);

    // Toggle side to move
    to_move = !to_move;

    // Add the move to the list (stack) of moves in this game
    moves.push(move);

    // Push state info onto the stack
    state_stack.push(state);
}

void unmake_move(uint16_t move) {
    // TODO
}

void Board::print_board() {
    const char* EMPTY_SYMBOL = ".";
    const char* SYMBOLS[NUM_COLORS][NUM_PIECES] = {
        { "♚", "♛", "♜", "♝", "♞", "♟" },
        { "♔", "♕", "♖", "♗", "♘", "♙" }
    };
    const char* FILES[BOARD_SIZE] ={
        "a", "b", "c", "d", "e", "f", "g", "h"
    };

    // Loop through the board top to bottom, left to right
    for (int rank = BOARD_SIZE - 1; rank >= 0; rank--) {
        std::clog << "\t" << rank << "  "; // Print ranks on the side

        for (int file = 0; file < BOARD_SIZE; file++) {
            Square square = get_square(rank, file);
            Piece piece = piece_map[square];
            if (piece == NO_PIECE) {
                std::clog << EMPTY_SYMBOL << " ";
                continue;
            }

            Color color = get_color(square);
            std::clog << SYMBOLS[color][piece] << " ";
        }

        // Move onto the next rank
        std::clog << "\n";
    }

    // Print files at the bottom
    std::clog << "\n\t   ";
    for (int file = 0; file < BOARD_SIZE; file++) {
        std::clog << FILES[file] << " ";
    }
    std::clog << "\n\n";
}