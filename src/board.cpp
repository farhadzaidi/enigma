#include <cctype>
#include <vector>
#include <sstream>
#include <iostream>
#include <cassert>

#include "constants.hpp"
#include "board.hpp"
#include "utils.hpp"
#include "move.hpp"

Board::Board() :
    pieces{},
    colors{},
    castling_rights(NO_CASTLING_RIGHTS),
    en_passant_target(NO_EN_PASSANT_TARGET) {
        piece_lookup.fill(EMPTY);
        color_lookup.fill(NO_COLOR);
    }

// Returns a bitboard with all bits corresponding to occupied squares set
uint64_t Board::get_occupied_squares() {
    return colors[WHITE] | colors[BLACK];
}

// Returns a bitboard with all bits corresponding to empty squares set
uint64_t Board::get_empty_squares() {
    return ~(colors[WHITE] | colors[BLACK]);
}

void Board::remove_piece(int color, int piece, int square) {
    uint64_t mask = ~get_mask(square);
    pieces[color][piece] &= mask;
    colors[color] &= mask;
    piece_lookup[square] = EMPTY;
    color_lookup[square] = NO_COLOR;
}

void Board::place_piece(int color, int piece, int square) {
    uint64_t mask = get_mask(square);
    pieces[color][piece] |= mask;
    colors[color] |= mask;
    piece_lookup[square] = piece;
    color_lookup[square] = color;
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
        // Ensure file stays within the range 0-7
        // More specifically, reset the file to 0 when it reaches 8
        file %= 8;

        // End of rank; go down one
        if (c == '/') {
            rank -= 1;
            continue;
        }

        // Number indicating how many empty squares in the file until the next piece
        if (std::isdigit(c)) {
            file += c - '0';
            continue;
        }

        // Must be a piece
        int square = get_square_index(rank, file);
        int color = std::isupper(c) ? WHITE : BLACK;
        int piece;

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

void Board::set_en_passant_target(int color, int piece, int from, int to) {
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
        en_passant_target = NO_EN_PASSANT_TARGET;
    }
}

int Board::handle_capture(int capture_square, int moving_color, int flag) {
    halfmoves = 0;

    if (flag == EN_PASSANT) {
        // In the case of en passant, the captured piece (pawn) is one rank
        // "behind" the "to" square. "Behind" can either be south or north
        // depending on whether the moving piece is white or black, respectively
        capture_square = moving_color == WHITE
            ? capture_square - 8  // 1 step south
            : capture_square + 8; // 1 step north
    }

    int captured_piece = piece_lookup[capture_square];
    remove_piece(!moving_color, captured_piece, capture_square);
    return captured_piece;
}

void Board::handle_castle(int castle_square) {
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

void Board::handle_castling_rights(int color, int piece) {
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

void Board::make_move(uint16_t move) {
    // Preserve board state before making the move
    State state(en_passant_target, castling_rights, halfmoves, NO_CAPTURED_PIECE);

    int from    = get_from(move);
    int to      = get_to(move);
    int mtype   = get_mtype(move);
    int flag    = get_flag(move);

    int moving_color = color_lookup[from];
    int moving_piece = piece_lookup[from];

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

    if (flag == CASTLE) handle_castle(to);
    handle_castling_rights(moving_color, moving_piece);

    // Toggle side to move
    to_move = !to_move;

    // Add the move to the list (stack) of moves in this game
    moves.push(move);

    // Push state info onto the stack
    state_stack.push(state);
}

void unmake_move(uint16_t move) {



}

void Board::print_board() {
    const char* EMPTY_SYMBOL = ".";
    const char* SYMBOLS[NUM_COLORS][NUM_PIECES] = {
        { "♚", "♛", "♜", "♝", "♞", "♟" },
        { "♔", "♕", "♖", "♗", "♘", "♙" }
    };

    // Loop through the board top to bottom, left to right
    for (int rank = BOARD_SIZE - 1; rank >= 0; rank--) {
        for (int file = 0; file < BOARD_SIZE; file++) {
            // Get the current square and use it to generate a bitmask
            // for only that square
            int square = get_square_index(rank, file);
            uint64_t mask = get_mask(square);
            
            // Loop through all pieces and check which piece type (if any)
            // occupy this square
            bool occupied = false;
            for (int c = 0; c < NUM_COLORS; c++) {
                for (int p = 0; p < NUM_PIECES; p++) {
                    // Check if this piece occupies the current square
                    if (pieces[c][p] & mask) {
                        std::clog << SYMBOLS[c][p] << " ";
                        occupied = true;

                        // Sanity check
                        assert(piece_lookup[square] == p);
                        assert(color_lookup[square] == c);
                    }
                }
            }

            // Print an empty/unoccupied symbol if the square isn't occupied
            if (!occupied) {
                std::clog << EMPTY_SYMBOL << " ";

                // Sanity check
                assert(piece_lookup[square] == EMPTY);
                assert(color_lookup[square] == NO_COLOR);
            }
        }
        // Move onto the next rank
        std::clog << "\n";
    }
}