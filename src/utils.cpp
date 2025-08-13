#include "types.hpp"
#include "utils.hpp"

// Calculates the index of a square given the 0-indexed rank and file
// (i.e. 0 <= rank, file <= 7)
Square get_square(int rank, int file) {
    return rank * BOARD_SIZE + file;
}

Square algebraic_to_index(const std::string& square) {
    // Subtracting by '1' gives us the 0-indexed rank
    // (e.g. '1' - '1' = 0 or '8' - '1' = 7)
    int rank = square[1] - '1';

    // Similarly, subtracting by 'a' gives us the 0-indexed file
    int file = square[0] - 'a';

    return get_square(rank, file);
}