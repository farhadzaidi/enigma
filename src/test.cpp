#include <filesystem>
#include <vector>
#include <string>


#include "test.hpp"
#include "types.hpp"
#include "board.hpp"
#include "utils.hpp"

const std::filesystem::path SINGLE_CHECK_EPD = "single_check.epd";
const std::filesystem::path DOUBLE_CHECK_EPD = "double_check.epd";
const std::filesystem::path NOT_IN_CHECK_FEN = "not_in_check.fen";

static bool test_in_check(Board& b) {
    // Test positions that should be in check
    std::vector<std::string> in_check_buffer;
    read_file(in_check_buffer, FEN_DIR / SINGLE_CHECK_EPD);
    read_file(in_check_buffer, FEN_DIR / DOUBLE_CHECK_EPD);
    for (const auto& line : in_check_buffer) {
        // Load position
        auto [fen, depth, expected_nodes] = parse_epd_line(line);
        b.reset();
        b.load_from_fen(fen);

        // If the side to move is not in check, print an error and return early
        if (!b.in_check()) {
            std::clog << "[FAILURE] 'in_check' - Expected side to move (" << (b.to_move == WHITE ? "white" : "black")
                << ") to be in check, but function returned false\n";
            std::clog << "FEN: " << fen << "\n";
            return false;
        }
    }

    // Test positions that should not be in check
    std::vector<std::string> not_in_check_buffer;
    read_file(not_in_check_buffer, FEN_DIR / NOT_IN_CHECK_FEN);
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

void run_tests() {
    Board b;
    if (test_in_check(b)) std::clog << "[SUCCESS] 'in_check'\n";
}