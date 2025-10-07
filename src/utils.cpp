#include <string>
#include <cctype>
#include <iostream>
#include <sstream>

#include "utils.hpp"
#include "types.hpp"
#include "move.hpp"
#include "board.hpp"

bool is_pos_int(const std::string& s) {
    if (s.empty()) return false;

    for (char c : s) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }

    return true;
}

Square uci_to_index(const std::string& square) {
    // Subtracting by '1' gives us the 0-indexed rank
    // (e.g. '1' - '1' = 0 or '8' - '1' = 7)
    int rank = square[1] - '1';

    // Similarly, subtracting by 'a' gives us the 0-indexed file
    int file = square[0] - 'a';

    return get_square(rank, file);
}

std::string index_to_uci(Square square) {
    // Reverse the operations from uci_to_index
    char rank = get_rank(square) + '1';
    char file = get_file(square) + 'a';

    return std::string{file, rank};
}

Move encode_move_from_uci(const Board& b, const std::string& uci_move) {
    // UCI notation can either be 4 or 5 characters
    // e.g. e2e4 or f7f8q

    // First 2 characters make up the "from" square
    Square from = uci_to_index(uci_move.substr(0, 2));

    // Next 2 characters make up the "to" square
    Square to = uci_to_index(uci_move.substr(2, 2));

    // The move was a capture if the "to" square is occupied (EP will be handled separately)
    MoveType mtype = QUIET;
    if (b.piece_map[to] != NO_PIECE) {
        mtype = CAPTURE;
    }

    // Determine move flag
    MoveFlag mflag = NORMAL;

    // Optional 5th character indicates the kind of promotion
    if (uci_move.length() == 5) {
        switch(uci_move[4]) {
            case 'b':
                mflag = PROMOTION_BISHOP;
                break;
            case 'n':
                mflag = PROMOTION_KNIGHT;
                break;
            case 'r':
                mflag = PROMOTION_ROOK;
                break;
            case 'q':
                mflag = PROMOTION_QUEEN;
                break;
        }
    } 
    
    // We can determine if the move was a castle if the king moved 2 squares horizontally
    else if (
        b.piece_map[from] == KING && 
        std::abs(static_cast<int>(from) - static_cast<int>(to)) == 2 // Type casting to prevent undeflow
    ) {
        mflag = CASTLE;
    }

    // If the moving piece was a pawn and it moved to the en passant target square,
    // then we know this move was an en passant
    else if (b.piece_map[from] == PAWN && to == b.en_passant_target) {
        mflag = EN_PASSANT;
        mtype = CAPTURE;
    }

    // Encode the move and return
    return Move(from, to, mtype, mflag);
}

std::string decode_move_to_uci(Move move) {
    std::string from = index_to_uci(move.from());
    std::string to = index_to_uci(move.to());

    std::string promotion = "";
    switch(move.flag()) {
        case PROMOTION_BISHOP:
            promotion = "b";
            break;
        case PROMOTION_KNIGHT:
            promotion = "n";
            break;
        case PROMOTION_ROOK:
            promotion = "r";
            break;
        case PROMOTION_QUEEN:
            promotion = "q";
            break;
    }

    return from + to + promotion;
}

void read_file(std::vector<std::string>& buffer, std::filesystem::path file_path, int max_lines) {
    std::ifstream file(file_path);
    if (!file) {
        std::cerr << "Failed to open: " << file_path << "\n";
    }

    int count = 0;
    std::string line;
    while (std::getline(file, line)) {
        count++;
        buffer.push_back(line);
        if (max_lines != -1 && count >= max_lines) {
            break;
        }
    }

    file.close();
}

// Parses a line in the form [FEN]; D[DEPTH] [NODES]
// e.g. 3r2k1/5B2/p7/8/1qr3Q1/4R3/1PP4P/2K5 b - -  0 1; D4 222062
std::tuple<std::string, int, uint64_t> parse_epd_line(std::string line) {
    auto pos = line.find(";");
    std::string fen = line.substr(0, pos);
    std::string rest = line.substr(pos + 1);

    std::istringstream iss(rest);

    std::string depth_str;
    iss >> depth_str;
    int depth = std::stoi(depth_str.substr(1));

    uint64_t nodes;
    iss >> nodes;

    return {fen, depth, nodes};
}