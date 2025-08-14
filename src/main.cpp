#include "types.hpp"
#include "board.hpp"

int main() {
    Board b;
    b.load_from_fen();
    b.debug();
    return 0;
}
