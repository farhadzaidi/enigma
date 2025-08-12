#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#define NUM_SQUARES 64
#define NUM_COLORS 2
#define NUM_PIECES 6
#define BOARD_SIZE 8
#define NO_EN_PASSANT_TARGET -1
#define NO_CASTLING_RIGHTS 0
#define NO_CAPTURED_PIECE -1

#define START_POS_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1"

#define FIRST_RANK      0
#define SECOND_RANK     1
#define THIRD_RANK      2
#define FOURTH_RANK     3
#define FIFTH_RANK      4
#define SIXTH_RANK      5    
#define SEVENTH_RANK    6
#define EIGHTH_RANK     7

#define A_FILE 0
#define B_FILE 1
#define C_FILE 2
#define D_FILE 3
#define E_FILE 4
#define F_FILE 5
#define G_FILE 6
#define H_FILE 7

// Little Endian Rank-File Mapping
// a1 --> bit 0
// h8 --> bit 63
enum Square {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8,
};


enum Color {
    WHITE,
    BLACK,
    NO_COLOR
};

enum Piece {
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    EMPTY
};

// Castling rights are represented by a 4 bit number (useful for hashing)
enum CASTLING_RIGHTS {
    WHITE_SHORT = 0b1,
    WHITE_LONG  = 0b10,
    BLACK_SHORT = 0b100,
    BLACK_LONG  = 0b1000
};

enum MoveType {
    QUIET,
    CAPTURE
};

enum MoveFlag {
    NORMAL,
    EN_PASSANT,
    CASTLE,
    PROMOTION_BISHOP,
    PROMOTION_KNIGHT,
    PROMOTION_ROOK,
    PROMOTION_QUEEN
};

#endif