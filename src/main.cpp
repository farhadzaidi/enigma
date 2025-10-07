#include <vector>
#include <string>
#include <cstdlib>
#include <iostream>

#include "bench.hpp"
#include "uci.hpp"
#include "board.hpp"
#include "perft.hpp"
#include "utils.hpp"
#include "test.hpp"

int main(int argc, char* argv[]) {
    std::vector<std::string> args;
    for (int i = 1; i < argc; i++) {
        args.push_back(argv[i]);
    }

    if (args.size() == 0) {
        // ### MAIN UCI LOOP
        uci_loop();
    } else {
        if (args[0] == "bench") {
            // ### BENCH - Comprehensive move generation test suite
            bool verbose = false;
            bool fast = false;

            for (int i = 1; i < args.size(); i++) {
                if (args[i] == "--verbose"){
                    verbose = true;
                } else if (args[i] == "--fast") {
                    fast = true;
                } else {
                    std::clog << "Error: Unknown option for bench '" << args[i] << "'\n";
                    return EXIT_FAILURE;
                }
            }

            run_bench(verbose, fast);
        } else if (args[0] == "perft") {
            // ### PERFT - Test move generation with any positon and FEN

            // Require depth
            if (args.size() == 1) {
                std::clog << "Error: Please specify perft depth\n";
                return EXIT_FAILURE;
            }

            // Validate depth
            std::string depth_str = args[1];
            if (!is_pos_int(depth_str)) {
                std::clog << "Error: Invalid depth\n";
                return EXIT_FAILURE;
            }
            int depth = std::stoi(depth_str);

            // TODO - parse FEN

            // Load start position on board and perform perft
            Board b;
            b.load_from_fen();
            perft<true>(b, depth);
        } else if (args[0] == "test") {
            run_tests();
        } else {
            std::clog << "Error: Unknown argument " << "'" << args[0] << "'\n";
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
