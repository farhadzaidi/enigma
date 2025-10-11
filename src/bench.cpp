// This file is used to test the accuracy/speed of the chess engine's move generator.
// It loads nearly a hundred thousand FEN strings with known node counts at various depths,
// performs move generation, and compares node counts against expected counts.
// Due to the large number of FEN strings, there is significant overhead from resetting and
// loading each board position, as such this suite is better suited to accurately test the
// move generator's correctness. For raw performance, perft is a better test.
// Usage: ./build/enigma bench [--fast] [--verbose]

#include <filesystem>
#include <iostream>
#include <cstdint>
#include <chrono>
#include <iomanip>

#include "types.hpp"
#include "bench.hpp"
#include "board.hpp"
#include "perft.hpp"
#include "utils.hpp"

// Number of lines to read from each file in fast mode
const int NUM_LINES_FAST = 1000;

static inline std::vector<std::string> collect_lines(bool fast) {
    std::vector<std::string> buffer;
    for (const auto& entry : std::filesystem::directory_iterator(FEN_DIR)) {
        // Only process .epd files
        if (entry.path().extension() == ".epd") {
            read_file(buffer, entry.path(), fast ? NUM_LINES_FAST : -1);
        }
    }

    return buffer;
}

void run_bench(bool verbose, bool fast, bool phased) {
    std::clog << "Running bench...\n";
    Board b;
    auto lines = collect_lines(fast);
    uint64_t total_nodes = 0;

    auto start = std::chrono::high_resolution_clock::now();
    for (const auto& line : lines) {
        auto [fen, depth, expected_nodes] = parse_epd_line(line);

        b.reset();
        b.load_from_fen(fen);
        uint64_t nodes = phased ? perft_phased(b, depth) : perft<false>(b, depth);
        total_nodes += nodes;

        if (nodes != expected_nodes) {
            std::clog << "\n[FAILURE] FEN: " << fen << "\n";
            std::clog << "At depth " << depth << ", expected " << expected_nodes << " nodes, but generated " << nodes << "\n";
            return;
        } else if (verbose) {
            std::clog << "\n[SUCCESS] FEN: " << fen << "\n";
            std::clog << "At depth " << depth << ", generated " << nodes << " nodes\n";
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    std::clog << "[SUCCESS] Bench completed in " << std::fixed << std::setprecision(1) << seconds << " seconds\n";
    std::clog << "Evaluated " << lines.size() << " positions, generating " << total_nodes << " total nodes\n";
    std::clog << "Nodes per second: " << static_cast<uint64_t>(total_nodes / seconds) << "\n";
}




