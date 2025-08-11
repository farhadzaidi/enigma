#include "constants.hpp"
#include "board.hpp"

int main() {
    Board b;
    b.load_from_fen();
    b.print_board();
    return 0;
}
