#include <cctype>
#include <vector>
#include <sstream>
#include <iostream>

#include "constants.hpp"
#include "board.hpp"

// Calculates the index of a square given the 0-indexed rank and file
// (i.e. 0 <= rank, file <= 7)
uint64_t Board::get_square_index(int rank, int file) {
    return rank * 8 + file;
}

uint64_t Board::algebraic_to_index(const std::string &square) {
    // Subtracting by '1' gives us the 0-indexed rank
    // (e.g. '1' - '1' = 0 or '8' - '1' = 7)
    int rank = square[1] - '1';

    // Similarly, subtracting by 'a' gives us the 0-indexed file
    int file = square[0] - 'a';

    return get_square_index(rank, file);
}

Board::Board() :
    pieces{},
    colors{},
    castling_rights(0), 
    en_passant_target(-1) {}

// Returns a bitboard with all bits corresponding to occupied squares set
uint64_t Board::get_occupied_squares() {
    return colors[WHITE] | colors[BLACK];
}

// Returns a bitboard with all bits corresponding to empty squares set
uint64_t Board::get_empty_squares() {
    return ~(colors[WHITE] | colors[BLACK]);
}

void Board::load_from_fen(const std::string &fen) {
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

        std::clog << "Char: " << c;
        // End of rank; go down one
        if (c == '/') {
            std::clog << "\n";
            rank -= 1;
            continue;
        }

        // Number indicating how many empty squares in the file until the next piece
        if (std::isdigit(c)) {
            std::clog << "\n";
            file += c - '0';
            continue;
        }

        // Must be a piece
        int square_index = get_square_index(rank, file);
        std::clog << " - setting index " << square_index << "\n";
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