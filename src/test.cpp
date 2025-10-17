#include <filesystem>
#include <vector>
#include <string>


#include "test.hpp"
#include "types.hpp"
#include "board.hpp"
#include "utils.hpp"

struct SanTestCase {
    std::string fen;
    std::string san;
    std::string expected_uci;
};

static bool test_in_check(Board& b) {
    // Test positions that should be in check
    std::vector<std::string> in_check_buffer;
    read_file(in_check_buffer, SINGLE_CHECK_EPD);
    read_file(in_check_buffer, DOUBLE_CHECK_EPD);
    for (const auto& line : in_check_buffer) {
        auto result = parse_perft_epd_line(line);

        // Load position
        b.reset();
        b.load_from_fen(result.fen);

        // If the side to move is not in check, print an error and return early
        if (!b.in_check()) {
            std::clog << "[FAILURE] 'in_check' - Expected side to move (" << (b.to_move == WHITE ? "white" : "black")
                << ") to be in check, but function returned false\n";
            std::clog << "FEN: " << result.fen << "\n";
            return false;
        }
    }

    // Test positions that should not be in check
    std::vector<std::string> not_in_check_buffer;
    read_file(not_in_check_buffer, NOT_IN_CHECK_FEN);
    for (const auto& fen : not_in_check_buffer) {
        // Load position
        b.reset();
        b.load_from_fen(fen);

        // If the side to move is in check, print an error and return early
        if (b.in_check()) {
            std::clog << "[FAILURE] 'in_check' - Expected side to move (" << (b.to_move == WHITE ? "white" : "black")
                << ") to not be in check, but function returned true\n";
            std::clog << "FEN: " << fen << "\n";
            return false;
        }
    }

    // All tests passed
    return true;
}

static bool test_parse_move_from_fen(Board& b) {
    SanTestCase test_cases[] = {
        {"rnb1kbnr/pppp1ppp/4p3/6q1/4P3/5K2/PPPP1PPP/RNBQ1BNR b kq - 3 3", "Qg3+", "g5g3"},
        {"1k1r4/pp1b1R2/3q2pp/4p3/2B5/4Q3/PPP2B2/2K5 b - -", "Qd1+", "d6d1"},
        {"3r1k2/4npp1/1ppr3p/p6P/P2PPPP1/1NR5/5K2/2R5 w - -", "d5", "d4d5"},
        {"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1", "e5", "e7e5"},
        {"rnbqkb1r/pppp1ppp/5n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3", "Nxe4", "f6e4"},
        {"rnbqkb1r/pppp1ppp/8/4p3/2B1n3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 4", "O-O", "e1g1"},
        {"rnb1kb1r/pppp1ppp/3n4/4p3/2B4q/5NP1/PPPP1P1P/RNBQ1RK1 w kq - 1 6", "gxh4", "g3h4"},
        {"rnb1k2r/1p1pbppp/8/pPp1pn2/2B4P/P4N2/2PP1P1P/RNBQ1RK1 w kq a6 0 10", "bxa6", "b5a6"},
        {"rn2k2r/P2pbppp/bp6/2p1pn2/2B4P/P4N2/2PP1P1P/RNBQ1RK1 w kq - 1 12", "axb8=Q+", "a7b8q"},
        {"r1b1k2r/1Q2bppp/1p6/2pp1n2/2B1Q2P/P4N2/2Pp1P1P/RNB2RK1 b kq - 1 17", "d1=N", "d2d1n"},
        {"r1b1k2r/1Q2bppp/1p6/2pp1n2/2B1Q2P/P4N2/2P2P1P/RNBn1RK1 w kq - 0 18", "Nc3", "b1c3"},
        {"r3k2r/1Q1bbppp/1p6/1Bpp1n2/1R2Q2P/2N2N2/2Pn1P1P/R1B3K1 w k - 2 24", "Rba4", "b4a4"},
        {"4k2r/rQ1bbppp/1p6/1Bpp1n2/R3Q2P/2N2N2/2Pn1P1P/R1B3K1 w k - 4 25", "R4a3", "a4a3"},
        {"4k2r/rQ1bbppp/1p6/1Bpp1n2/R3Q2P/2N2N2/2Pn1P1P/R1B3K1 w k - 4 25", "R1a2", "a1a2"},
        {"r6r/Q2bbppp/1p1k4/1Bpp1n2/R3Q2P/2N2N2/R1Pn1P1P/2B3K1 b - - 13 29", "Rad8", "a8d8"},
        {"3r3r/Q2bbppp/1p1k4/1Bpp1n2/R3Q2P/2N2N2/R1Pn1P1P/2B3K1 w - - 14 30", "Kg2", "g1g2"},
        {"3r3r/Q2bbppp/1p1k4/1Bpp1n2/R3Q2P/2N2N2/R1Pn1PKP/2B5 b - - 15 30", "Rhf8", "h8f8"},
        {"3r1r2/Q2bbppp/1p1k4/1Bpp1n2/R3Q2P/2N2N2/R1Pn1PKP/2B5 w - - 16 31", "Qxd5#", "e4d5"},
        {"2br3r/Q3bppp/1p1k4/1Bp2n2/3p1R1P/2N3Q1/R1Pn1PKP/2B1N3 w - - 0 38", "Rxd4+", "f4d4"},
        {"8/8/8/7k/5q2/8/8/7K b - - 3 69", "Qf2", "f4f2"},
        {"1q1r3k/3P1pp1/ppBR1n1p/4Q2P/P4P2/8/5PK1/8 w - -", "Rxf6", "d6f6"},
        {"2r3k1/1p2q1pp/2b1pr2/p1pp4/6Q1/1P1PP1R1/P1PN2PP/5RK1 w - - ", "Qxg7 + ", "g4g7"},
        {"4r2r/pppkq1pp/2n1pn2/4p1B1/4N2Q/8/PPP3PP/4RRK1 w - -", "Nxf6 +", "e4f6"},
        {"rnq1nrk1/pp3pbp/6p1/3p4/3P4/5N2/PP2BPPP/R1BQK2R w KQ -", "O-O", "e1g1"},
    };

    for (const auto& test : test_cases) {
        // Load position
        b.reset();
        b.load_from_fen(test.fen);

        // Parse SAN
        Move move = parse_move_from_san(b, test.san);

        // Check if move is NULL
        if (move == NULL_MOVE) {
            std::clog << "[FAILURE] 'san_parsing' - Failed to parse SAN\n";
            std::clog << "FEN: " << test.fen << "\n";
            std::clog << "SAN: " << test.san << "\n";
            std::clog << "Expected UCI: " << test.expected_uci << "\n";
            return false;
        }

        // Convert to UCI
        std::string uci = decode_move_to_uci(move);

        // Compare with expected
        if (uci != test.expected_uci) {
            std::clog << "[FAILURE] 'san_parsing' - UCI mismatch\n";
            std::clog << "FEN: " << test.fen << "\n";
            std::clog << "SAN: " << test.san << "\n";
            std::clog << "Expected UCI: " << test.expected_uci << "\n";
            std::clog << "Got UCI: " << uci << "\n";
            return false;
        }
    }

    // All tests passed
    return true;
}

void run_tests() {
    Board b;
    if (test_in_check(b)) std::clog << "[SUCCESS] 'in_check'\n";
    if (test_parse_move_from_fen(b)) std::clog << "[SUCCESS] 'parse_move_from_fen'\n";
}