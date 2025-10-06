#include <cctype>
#include <sstream>
#include <iostream>
#include <string>
#include <array>
#include <stack>
#include <vector>

#include "board.hpp"
#include "types.hpp"
#include "utils.hpp"
#include "move.hpp"
#include "precompute.hpp"

Board::Board() {
    reset();
}

void Board::reset() {
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
    ply = 0;
}

void Board::remove_piece(Color color, Piece piece, Square square) {
    // Create a mask based on the square of the piece and use bitwise AND to
    // remove the piece from each respective bitboard
    Bitboard mask = ~get_mask(square);
    pieces[color][piece] &= mask;
    colors[color] &= mask;
    occupied &= mask;

    piece_map[square] = NO_PIECE;
    // No need to clear king square here as it will be updated in place_piece
}

void Board::place_piece(Color color, Piece piece, Square square) {
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

Color Board::get_color(Square square) const {
    return (colors[BLACK] >> square) & uint64_t{1};
}

void Board::load_from_fen(const std::string& fen) {
    std::vector<std::string> parts;
    std::istringstream iss(fen);
    std::string item;

    // Split the fen string using a space as the delimiter
    while(std::getline(iss, item, ' ')) {
        // Skip empty strings caused by multiple spaces
        if (!item.empty()) {
            parts.push_back(item);
        }
    }

    std::string position            = parts[0];
    std::string to_move             = parts.size() > 1 ? parts[1] : "w";
    std::string castling_rights     = parts.size() > 2 ? parts[2] : "-";
    std::string en_passant_target   = parts.size() > 3 ? parts[3] : "-";
    std::string halfmoves           = parts.size() > 4 ? parts[4] : "0";
    std::string fullmoves           = parts.size() > 5 ? parts[5] : "1";

    // Set up position starting from top left
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
    if (en_passant_target != "-") {
        this->en_passant_target = uci_to_index(en_passant_target);
    }

    // Halfmoves
    this->halfmoves = std::stoi(halfmoves);

    // Fullmoves
    this->fullmoves = std::stoi(fullmoves);
}


void Board::print_board() const {
    std::string EMPTY_SYMBOL = ".";
    std::array<std::array<std::string, NUM_PIECES>, NUM_COLORS> SYMBOLS = {{ // yuck...
        { "♟", "♝", "♞", "♜", "♛", "♚" },
        { "♙", "♗", "♘", "♖", "♕", "♔" }
    }};
    std::array<std::string, BOARD_SIZE> FILES = {
        "a", "b", "c", "d", "e", "f", "g", "h"
    };

    std::clog << "\n";

    // Loop through the board top to bottom, left to right
    for (int rank = BOARD_SIZE - 1; rank >= 0; rank--) {
        std::clog << "\t" << rank + 1 << "  "; // Print ranks on the side

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

void Board::debug() {
    std::string input = "";
    while (true) {
        print_board();

        std::cin >> input;
        if (input == "quit") {
            break;
        } else if (input == "undo") {
            unmake_move(moves[ply]);
        } else {
            Move move = encode_move_from_uci(*this, input);
            make_move(move);
        }
    }
}

void Board::set_en_passant_target(Color color, Piece piece, Square from, Square to) {
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

Piece Board::handle_capture(Square capture_square, Color moving_color, MoveFlag mflag) {
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

void Board::handle_castle(Square castle_square) {
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

void Board::update_castling_rights(Square from, Square to) {
    // Use precomputed lookup table to update castling rights
    castling_rights &= ~castling_rights_updates[from];
    castling_rights &= ~castling_rights_updates[to];
}

void Board::make_move(Move move) {
    // Preserve irreversible board state before making the move
    State state(en_passant_target, castling_rights, halfmoves, NO_PIECE);

    Square from     = move.from();
    Square to       = move.to();
    MoveType mtype  = move.type();
    MoveFlag mflag  = move.flag();

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
        state.captured_piece = handle_capture(to, moving_color, mflag);
    }

    // Check if the move is a promotion; if so, update the moving piece
    switch (mflag) {
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

    if (mflag == CASTLE) {
        handle_castle(to);
    }

    update_castling_rights(from, to);

    // Toggle side to move
    to_move ^= 1;

    // Update stacks and increment ply
    moves[ply] = move;
    states[ply] = state;
    ply += 1;
}

void Board::unmake_move(Move move) {
    Square from     = move.from();
    Square to       = move.to();
    MoveType mtype  = move.type();
    MoveFlag mflag  = move.flag();

    // The color that moved on this move is the opposite of the color that is
    // currently set to move
    Color moving_color = to_move ^ 1;

    // Decrement ply (simulate popping from top of moves and states stacks)
    ply -= 1;

    // Restore state
    const State& prev_state = states[ply];
    en_passant_target = prev_state.en_passant_target;
    castling_rights = prev_state.castling_rights;
    halfmoves = prev_state.halfmoves;

    // Fullmoves is only incremented if black moves, so we decrement it if we
    // are undoing a black move
    if (moving_color == BLACK) {
        fullmoves--;
    }

    // Remove the piece from "to"
    Piece moving_piece = piece_map[to];
    remove_piece(moving_color, moving_piece, to);

    // In the case of a promotion, we change the moving piece to pawn so we place
    // the correct piece back on "from"
    if (move.is_promotion()) {
        moving_piece = PAWN;
    }

    // Put the moving piece back on "from"
    place_piece(moving_color, moving_piece, from);

    // Restore the captured piece
    if (mtype == CAPTURE) {
        Square capture_square = to;

        if (mflag == EN_PASSANT) {
            // Same logic as before
            capture_square = moving_color == WHITE
                ? capture_square - 8 
                : capture_square + 8;
        }

        place_piece(moving_color ^ 1, prev_state.captured_piece, capture_square);
    }

    if (mflag == CASTLE) {
        // Determine the correct corner rook based on where the king moved and
        // move it back to its respective corner
        switch (to) {
            case C1: // White long
                remove_piece(moving_color, ROOK, D1);
                place_piece(moving_color, ROOK, A1);
                break;
            case G1: // White short
                remove_piece(moving_color, ROOK, F1);
                place_piece(moving_color, ROOK, H1);
                break;
            case C8: // Black long
                remove_piece(moving_color, ROOK, D8);
                place_piece(moving_color, ROOK, A8);
                break;
            case G8: // Black short
                remove_piece(moving_color, ROOK, F8);
                place_piece(moving_color, ROOK, H8);
                break;
        }
    }

    // Toggle side to move
    to_move ^= 1;
}
