#include <iostream>
#include <chrono>

#include "board.hpp"
#include "perft.hpp"

int main(int argc, char* argv[]) {
    using namespace std::chrono;

    Board b;
    b.load_from_fen();

    int depth = 6;

    auto start = std::chrono::high_resolution_clock::now();
    int nodes = perft(b, depth);
    auto end = std::chrono::high_resolution_clock::now();

    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    std::clog << "\nExplored " << nodes << " nodes in " << seconds << " seconds at depth " << depth << "\n";

    int nodes_per_second = nodes / seconds;
    std::clog << "NPS: " << nodes_per_second << "\n\n";
}
