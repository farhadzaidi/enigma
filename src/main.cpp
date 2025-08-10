#include "constants.hpp"
#include "board.hpp"
#include "debug.hpp"

int main() {
    Board b;
    b.load_from_fen();
    print_bitboard(b.pieces[WHITE][KNIGHT]);
    return 0;
}
