#include <vector>
#include <string>
#include <cstdlib>
#include <iostream>

#include "bench.hpp"
#include "uci.hpp"

#include "board.hpp"
#include "perft.hpp"
#include "types.hpp"

int main(int argc, char* argv[]) {
    std::vector<std::string> args;
    for (int i = 1; i < argc; i++) {
        args.push_back(argv[i]);
    }

    if (args.size() == 0) {
        uci_loop();
    } else {
        if (args[0] == "bench") {
            run_bench();
        } else if (args[0] == "perft") {
            Board b;
            b.load_from_fen(START_POS_FEN);
            uint64_t nodes = perft(b, 6);
            std::clog << "Nodes: " << nodes << "\n";
            return EXIT_SUCCESS;
        } else {
            std::clog << "Error: Unknown argument " << "'" << args[0] << "'\n";
            return EXIT_FAILURE;
        }
    }


}
