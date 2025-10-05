#include <vector>
#include <string>
#include <cstdlib>
#include <iostream>

#include "bench.hpp"
#include "uci.hpp"

#include "board.hpp"
#include "perft.hpp"
#include "types.hpp"
#include "utils.hpp"

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
            // Require depth
            if (args.size() == 1) {
                std::clog << "Error: Please specify perft depth\n";
                return EXIT_FAILURE;
            }

            // Validate depth
            std::string depth_str = args[1];
            if (!is_pos_int(depth_str)) {
                std::clog << "Error: Invalid depth\n";
            }
            int depth = std::stoi(depth_str);

            // Load start position on board and perform perft
            Board b;
            b.load_from_fen();
            perft<true>(b, depth);
            return EXIT_SUCCESS;
        } else {
            std::clog << "Error: Unknown argument " << "'" << args[0] << "'\n";
            return EXIT_FAILURE;
        }
    }


}
