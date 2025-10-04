#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <tuple>
#include <chrono>
#include <iomanip>

#include "bench.hpp"
#include "board.hpp"
#include "perft.hpp"

// This is really just to silence the IDE warning since PROJECT_ROOT
// should be defined in CMakeLists.txt
#ifndef PROJECT_ROOT
#define PROJECT_ROOT "./"
#endif

namespace fs = std::filesystem;

static void read_file(std::vector<std::string>& buffer, fs::path file_path) {
    std::ifstream file(file_path);
    if (!file) {
        std::cerr << "Failed to open: " << file_path << "\n";
    }

    std::string line;
    while (std::getline(file, line)) {
        buffer.push_back(line);
    }

    file.close();
}

static std::vector<std::string> collect_lines() {
    std::vector<std::string> buffer;
    fs::path fen_dir = fs::path(PROJECT_ROOT) / "fen";
    for (const auto& entry : fs::directory_iterator(fen_dir)) {
        read_file(buffer, entry.path());
    }

    return buffer;
}

static std::tuple<std::string, int, uint64_t> parse_line(std::string line) {
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

void run_bench() {
    std::clog << "Running bench...\n";
    Board b;
    auto lines = collect_lines();
    uint64_t total_nodes = 0;

    auto start = std::chrono::high_resolution_clock::now();
    for (const auto& line : lines) {
        auto [fen, depth, expected_nodes] = parse_line(line);

        b.reset();
        b.load_from_fen(fen);
        uint64_t nodes = perft(b, depth);
        total_nodes += nodes;

        if (nodes != expected_nodes) {
            std::clog << "\n[FAILURE] FEN: " << fen << "\n";
            std::clog << "At depth " << depth << ", expected " << expected_nodes << " nodes, but found " << nodes << "\n";
            return;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    std::clog << "[SUCCESS] Bench completed in " << std::fixed << std::setprecision(1) << seconds << " seconds\n";
    std::clog << "NPS: " << static_cast<uint64_t>(total_nodes / seconds) << "\n";
}




