#include <cctype>
#include <vector>
#include <sstream>
#include <iostream>

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
        color_lookup.fill(EMPTY);
    }

// Returns a bitboard with all bits corresponding to occupied squares set
uint64_t Board::get_occupied_squares() {
    return colors[WHITE] | colors[BLACK];
}

// Returns a bitboard with all bits corresponding to empty squares set
uint64_t Board::get_empty_squares() {
    return ~(colors[WHITE] | colors[BLACK]);
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
        int square_index = get_square_index(rank, file);
        int color = std::isupper(c) ? WHITE : BLACK;
        int type;

        switch (std::toupper(c)) {
            case 'P':
                type = PAWN;
                break;
            case 'B':
                type = BISHOP;
                break;
            case 'N':
                type = KNIGHT;
                break;
            case 'R':
                type = ROOK; 
                break;
            case 'Q':
                type = QUEEN;
                break;
            case 'K':
                type = KING;
                break;
        }

        // Set the bit corresponding to the square index for the given piece
        uint64_t mask = uint64_t{1} << square_index;
        pieces[color][type] |= mask;
        colors[color] |= mask;

        // Update piece and color lookup tables
        piece_lookup[square_index] = type;
        color_lookup[square_index] = color;

        file++;
    }

    // Side to move
    std::clog << "2. Setting side to move\n";
    this->to_move = to_move == "w" ? WHITE : BLACK;

    // Castling rights
    for (char c : castling_rights) {
        switch (c) {
            case 'K':
                this->castling_rights |= WHITE_KING_SIDE;
                break;
            case 'Q':
                this->castling_rights |= WHITE_QUEEN_SIDE;
                break;
            case 'k':
                this->castling_rights |= BLACK_KING_SIDE;
                break;
            case 'q':
                this->castling_rights |= BLACK_QUEEN_SIDE;
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

void Board::make_move(uint16_t move) {
    int from = get_from(move);
    int to = get_to(move);
    int mtype = get_mtype(move);
    int flag = get_flag(move);

    uint64_t from_mask = uint64_t{1} << from;
    uint64_t to_mask = uint64_t{1} << to;

    int moving_piece = piece_lookup[from];
    int moving_color = color_lookup[from];

    // Unset the "from" bit for the moving piece and color bitboards
    pieces[moving_color][moving_piece] ^= from_mask;
    colors[moving_color] ^= from_mask;

    // Similarly, empty the "from" square for the piece and color lookup tables
    piece_lookup[from] = EMPTY;
    color_lookup[from] = EMPTY;

    if (mtype == CAPTURE) {
        int capture_square = to;
        uint64_t capture_mask = to_mask;

        if (flag == EN_PASSANT) {
            // In the case of en passant, the captured piece (pawn) is one rank
            // "behind" the "to" square. "Behind" can either be south or north
            // depending on whether the moving piece is white or black, respectively
            if (moving_color == WHITE) {
                capture_square -= 8;
                capture_mask = shift_south(capture_mask);
            } else {
                capture_square += 8;
                capture_mask = shift_north(capture_mask);
            }
        }

        int captured_piece = piece_lookup[capture_square];
        int captured_color = color_lookup[capture_square];

        // Remove the captured piece from the respective piece and color bitboards
        pieces[captured_color][captured_piece] ^= capture_mask;
        colors[captured_color] ^= capture_mask;

        // Remove the captured piece from the piece and color lookup tables
        piece_lookup[capture_square] = EMPTY;
        color_lookup[capture_square] = EMPTY;
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

    // Set the "to" bit for the moving piece and color bitboards
    pieces[moving_color][moving_piece] ^= to_mask;
    colors[moving_color] ^= to_mask; // This would be the same regardless of promotion

    // Similarly, update the "to" square for the piece and color lookup tables
    piece_lookup[to] = moving_piece;
    color_lookup[to] = moving_color; // This would be the same regardless of promotion

    if (flag == CASTLE) {
        switch (to) {
            case c1: // White long castle
                // Move rook from a1 to d1
                uint64_t a1_mask = uint64_t{1} << a1;
                uint64_t d1_mask = uint64_t{1} << d1;

                pieces[WHITE][ROOK] ^= a1_mask;
                pieces[WHITE][ROOK] ^= d1_mask;
                colors[WHITE] ^= a1_mask;
                colors[WHITE] ^= d1_mask;

                // Update lookup tables
                piece_lookup[a1] = EMPTY;
                piece_lookup[d1] = ROOK;
                color_lookup[a1] = EMPTY;
                color_lookup[d1] = WHITE;
                break;
            case g1: // White short castle
                // Move rook from h1 to f1
                uint64_t h1_mask = uint64_t{1} << h1;
                uint64_t f1_mask = uint64_t{1} << f1;

                pieces[WHITE][ROOK] ^= h1_mask;
                pieces[WHITE][ROOK] ^= f1_mask;
                colors[WHITE] ^= h1_mask;
                colors[WHITE] ^= f1_mask;

                // Update lookup tables
                piece_lookup[h1] = EMPTY;
                piece_lookup[f1] = ROOK;
                color_lookup[h1] = EMPTY;
                color_lookup[f1] = WHITE;
                break;
            case c8: // Black long castle
                // Move rook from a8 to d8
                uint64_t a8_mask = uint64_t{1} << a8;
                uint64_t d8_mask = uint64_t{1} << d8;
                pieces[BLACK][ROOK] ^= a8_mask;
                pieces[BLACK][ROOK] ^= d8_mask;
                colors[BLACK] ^= a8_mask;
                colors[BLACK] ^= d8_mask;

                // Update lookup tables
                piece_lookup[a8] = EMPTY;
                piece_lookup[d8] = ROOK;
                color_lookup[a8] = EMPTY;
                color_lookup[d8] = BLACK;
                break;
            case g8: // Black short castle
                // Move rook from h8 to f8
                uint64_t h8_mask = uint64_t{1} << h8;
                uint64_t f8_mask = uint64_t{1} << f8;
                pieces[BLACK][ROOK] ^= h8_mask;
                pieces[BLACK][ROOK] ^= f8_mask;
                colors[BLACK] ^= h8_mask;
                colors[BLACK] ^= f8_mask;

                // Update lookup tables
                piece_lookup[h8] = EMPTY;
                piece_lookup[f8] = ROOK;
                color_lookup[h8] = EMPTY;
                color_lookup[f8] = BLACK;
                break;
        }
    }

    // TODO

    // Update castling rights if:
    // King or respective rook moves
    // Respective rook gets captured

    // Set EP target square if pawn moves up 2 squares
    // Clear EP target square otherwise

    // Update halfmove and fullmove clocks
    
    // Toggle side to move

    // Update zobrist hash (later)

    // Push move to stack

    // irreversible aspects:
    // - castling rights
    // - en pas target square
    // - halfmove clock
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
            uint64_t mask = uint64_t{1} << square;
            
            // Loop through all pieces and check which piece type (if any)
            // occupy this square
            bool occupied = false;
            for (int c = 0; c < NUM_COLORS; c++) {
                for (int p = 0; p < NUM_PIECES; p++) {
                    // Check if this piece occupies the current square
                    if (this->pieces[c][p] & mask) {
                        std::clog << SYMBOLS[c][p] << " ";
                        occupied = true;
                    }
                }
            }

            // Print an empty/unoccupied symbol if the square isn't occupied
            if (!occupied) {
                std::clog << EMPTY_SYMBOL << " ";
            }
        }
        // Move onto the next rank
        std::clog << "\n";
    }
}