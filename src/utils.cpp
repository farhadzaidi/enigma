#include <string>
#include <cctype>
#include <iostream>
#include <sstream>

#include "utils.hpp"
#include "types.hpp"
#include "move.hpp"
#include "board.hpp"
#include "movegen.hpp"

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


// Helper struct to hold parsed SAN components
struct ParsedSan {
    bool is_castling_kingside = false;
    bool is_castling_queenside = false;
    Piece piece_type = PAWN;
    Square to_square = NO_SQUARE;
    bool is_capture = false;
    MoveFlag promotion_flag = NORMAL;
    int from_file = -1;
    int from_rank = -1;
};

// Normalize SAN by stripping check/mate/annotation symbols
static std::string normalize_san(std::string san) {
    std::string result;

    // Trim whitespace and build result
    for (char c : san) {
        // Skip whitespace, check/mate indicators and annotations
        if (std::isspace(c) || c == '+' || c == '#' || c == '!' || c == '?') {
            continue;
        }
        result += c;
    }

    // Remove "e.p." or "ep" suffix (case-insensitive)
    std::string lower = result;
    for (char& c : lower) c = std::tolower(c);

    size_t ep_pos = lower.find("e.p.");
    if (ep_pos == std::string::npos) {
        ep_pos = lower.find("ep");
    }
    if (ep_pos != std::string::npos) {
        result = result.substr(0, ep_pos);
    }

    // Normalize castling notation (accept o-o or 0-0 variants)
    if (lower == "o-o" || lower == "0-0") {
        result = "O-O";
    } else if (lower == "o-o-o" || lower == "0-0-0") {
        result = "O-O-O";
    }

    return result;
}

// Parse SAN string into components
static ParsedSan parse_san_components(const std::string& san) {
    ParsedSan parsed;

    // Guard against empty or too-short strings
    if (san.empty()) {
        return parsed;
    }

    // Check for castling
    if (san == "O-O") {
        parsed.is_castling_kingside = true;
        parsed.piece_type = KING;
        return parsed;
    }
    if (san == "O-O-O") {
        parsed.is_castling_queenside = true;
        parsed.piece_type = KING;
        return parsed;
    }

    size_t index = 0;

    // Parse piece type (if uppercase letter at start, it's a piece; otherwise pawn)
    if (index < san.length() && std::isupper(san[index])) {
        switch (std::toupper(san[index])) {
            case 'K': parsed.piece_type = KING; break;
            case 'Q': parsed.piece_type = QUEEN; break;
            case 'R': parsed.piece_type = ROOK; break;
            case 'B': parsed.piece_type = BISHOP; break;
            case 'N': parsed.piece_type = KNIGHT; break;
        }
        index++;
    }

    // Find the destination square (last 2 characters before promotion or end)
    size_t promotion_pos = san.find('=');
    size_t dest_start = (promotion_pos != std::string::npos) ? promotion_pos - 2 : san.length() - 2;

    // Guard against invalid destination extraction
    if (san.length() < 2 || dest_start >= san.length() || dest_start + 2 > san.length()) {
        return parsed; // Invalid SAN
    }

    std::string dest_square = san.substr(dest_start, 2);
    // Validate destination square format
    if (dest_square.length() == 2 && dest_square[0] >= 'a' && dest_square[0] <= 'h' &&
        dest_square[1] >= '1' && dest_square[1] <= '8') {
        parsed.to_square = uci_to_index(dest_square);
    } else {
        return parsed; // Invalid destination square
    }

    // Parse promotion (if exists)
    if (promotion_pos != std::string::npos && promotion_pos + 1 < san.length()) {
        char promo_piece = std::toupper(san[promotion_pos + 1]);
        switch (promo_piece) {
            case 'Q': parsed.promotion_flag = PROMOTION_QUEEN; break;
            case 'R': parsed.promotion_flag = PROMOTION_ROOK; break;
            case 'B': parsed.promotion_flag = PROMOTION_BISHOP; break;
            case 'N': parsed.promotion_flag = PROMOTION_KNIGHT; break;
        }
    }

    // Check for capture (presence of 'x')
    if (san.find('x') != std::string::npos) {
        parsed.is_capture = true;
    }

    // Parse disambiguator (between piece type and destination)
    // Everything between index and dest_start (excluding 'x')
    std::string middle;
    if (index < dest_start) {
        for (size_t i = index; i < dest_start && i < san.length(); i++) {
            if (san[i] != 'x') {
                middle += san[i];
            }
        }
    }

    // Parse the middle part for disambiguator
    if (!middle.empty()) {
        // Check if it's a file (a-h)
        if (middle.length() == 1 && middle[0] >= 'a' && middle[0] <= 'h') {
            parsed.from_file = middle[0] - 'a';
        }
        // Check if it's a rank (1-8)
        else if (middle.length() == 1 && middle[0] >= '1' && middle[0] <= '8') {
            parsed.from_rank = middle[0] - '1';
        }
        // Check if it's both (e.g., "a1")
        else if (middle.length() == 2) {
            if (middle[0] >= 'a' && middle[0] <= 'h') {
                parsed.from_file = middle[0] - 'a';
            }
            if (middle[1] >= '1' && middle[1] <= '8') {
                parsed.from_rank = middle[1] - '1';
            }
        }
    }

    return parsed;
}

// Main function to parse SAN and return the corresponding Move
Move parse_move_from_san(Board& b, const std::string& san) {
    // Normalize the input
    std::string normalized = normalize_san(san);

    // Parse SAN components
    ParsedSan parsed = parse_san_components(normalized);

    // Handle castling separately
    if (parsed.is_castling_kingside || parsed.is_castling_queenside) {
        Square king_from = b.king_squares[b.to_move];
        Square king_to;

        if (parsed.is_castling_kingside) {
            king_to = b.to_move == WHITE ? G1 : G8;
        } else {
            king_to = b.to_move == WHITE ? C1 : C8;
        }

        parsed.to_square = king_to;
    }

    // Generate all legal moves
    MoveList legal_moves = generate_moves<ALL>(b);

    // Filter moves based on parsed criteria
    Move candidate = NULL_MOVE;
    int match_count = 0;

    for (const Move& move : legal_moves) {
        // Check piece type matches
        Piece moving_piece = b.piece_map[move.from()];
        if (moving_piece != parsed.piece_type) {
            continue;
        }

        // Check destination matches
        if (move.to() != parsed.to_square) {
            continue;
        }

        // Check capture flag matches (both directions)
        // If SAN indicates capture, move must be capture
        if (parsed.is_capture && move.type() != CAPTURE) {
            continue;
        }
        // If SAN does NOT indicate capture, move must NOT be capture
        if (!parsed.is_capture && move.type() == CAPTURE) {
            continue;
        }

        // Check promotion flag matches (both directions)
        // If SAN has promotion, move must have that exact promotion
        if (parsed.promotion_flag != NORMAL && move.flag() != parsed.promotion_flag) {
            continue;
        }
        // If SAN has NO promotion, move must NOT be a promotion
        if (parsed.promotion_flag == NORMAL && move.is_promotion()) {
            continue;
        }

        // For castling, check the flag
        if ((parsed.is_castling_kingside || parsed.is_castling_queenside) && move.flag() != CASTLE) {
            continue;
        }

        // Check disambiguator matches
        if (parsed.from_file != -1 && get_file(move.from()) != parsed.from_file) {
            continue;
        }
        if (parsed.from_rank != -1 && get_rank(move.from()) != parsed.from_rank) {
            continue;
        }

        // This move matches all criteria
        candidate = move;
        match_count++;
    }

    // Return NULL_MOVE if no match or ambiguous
    if (match_count != 1) {
        return NULL_MOVE;
    }

    return candidate;
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

// Parses a line in the form [FEN]; D[DEPTH] [NODES]; D[DEPTH] [NODES]; ...
// e.g. 1Q3k2/8/8/p2p1p2/R4p2/5bP1/3B1bP1/5K2 b - - 0 1; D1 3; D2 117; D3 1994; D4 67254
PerftEpdResult parse_perft_epd_line(std::string line) {
    auto pos = line.find(";");
    std::string fen = line.substr(0, pos);
    std::string rest = line.substr(pos + 1);

    std::istringstream iss(rest);
    std::unordered_map<int, uint64_t> depth_nodes;

    std::string depth_str;
    uint64_t nodes;

    // Parse all D[depth] [nodes] pairs
    while (iss >> depth_str >> nodes) {
        int depth = std::stoi(depth_str.substr(1));
        depth_nodes[depth] = nodes;
    }

    return {fen, depth_nodes};
}

// Parses engine benchmark EPD line in the form [FEN]; bm [MOVE]; ...
// e.g. rnbqkb1r/p3pppp/1p6/2ppP3/3N4/2P5/PPP1QPPP/R1B1KB1R w KQkq - ; bm e6; id BK.04
EngineEpdResult parse_engine_epd_line(std::string line) {
    // Find FEN (everything before "; bm ")
    auto bm_pos = line.find("; bm ");
    if (bm_pos == std::string::npos) {
        return {"", ""};  // Invalid format
    }

    std::string fen = line.substr(0, bm_pos);

    // Find SAN move (between "bm " and next ";")
    size_t san_start = bm_pos + 5;  // length of "; bm "
    size_t san_end = line.find(';', san_start);

    std::string san_part;
    if (san_end != std::string::npos) {
        san_part = line.substr(san_start, san_end - san_start);
    } else {
        san_part = line.substr(san_start);
    }

    // Extract first move (in case there are multiple best moves separated by space)
    std::istringstream iss(san_part);
    std::string san;
    iss >> san;

    return {fen, san};
}