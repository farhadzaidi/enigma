#pragma once

#include <cstdint>

// Type definitions
using U8    = uint8_t;
using U16   = uint16_t;
using U64   = uint64_t;

using Mask              = U64;
using Bitboard          = U64;
using Move              = U16;
using MoveComponent     = U16;
using Square            = U8;
using Color             = U8;
using Piece             = U8;
using CastlingRights    = U8;

// Little Endian Rank-File Mapping
// a1 --> bit 0
// h8 --> bit 63
enum SquareEnum : Square {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8,
};

enum ColorEnum: Color {
    WHITE,
    BLACK,
    NO_COLOR
};

enum PieceEnum : Piece {
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    NO_PIECE
};

enum CastlingRightsEnum : CastlingRights {
    WHITE_SHORT = 0b1,
    WHITE_LONG  = 0b10,
    BLACK_SHORT = 0b100,
    BLACK_LONG  = 0b1000
};

enum MoveTypeEnum : MoveComponent {
    QUIET,
    CAPTURE
};

enum MoveFlagEnum : MoveComponent {
    NORMAL,
    EN_PASSANT,
    CASTLE,
    PROMOTION_BISHOP,
    PROMOTION_KNIGHT,
    PROMOTION_ROOK,
    PROMOTION_QUEEN
};

// Board
inline constexpr int NUM_SQUARES    = 64;
inline constexpr int NUM_COLORS     = 2;
inline constexpr int NUM_PIECES     = 6;
inline constexpr int BOARD_SIZE     = 8;

inline constexpr int FIRST_RANK     = 0;
inline constexpr int SECOND_RANK    = 1;
inline constexpr int THIRD_RANK     = 2;
inline constexpr int FOURTH_RANK    = 3;
inline constexpr int FIFTH_RANK     = 4;
inline constexpr int SIXTH_RANK     = 5;
inline constexpr int SEVENTH_RANK   = 6;
inline constexpr int EIGHTH_RANK    = 7;

inline constexpr int A_FILE = 0;
inline constexpr int B_FILE = 1;
inline constexpr int C_FILE = 2;
inline constexpr int D_FILE = 3;
inline constexpr int E_FILE = 4;
inline constexpr int F_FILE = 5;
inline constexpr int G_FILE = 6;
inline constexpr int H_FILE = 7;

// Sentinal values
inline constexpr Square NO_SQUARE                   = -1;
inline constexpr CastlingRights NO_CASTLING_RIGHTS  =  0;
inline constexpr Bitboard EMPTY_BITBOARD            =  0;

// FEN strings
inline constexpr const char* START_POS_FEN = 
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1";
inline constexpr const char* KIWIPETE_FEN = 
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
inline constexpr const char* POSITION_3_FEN = // Castling, en passant, and promotions
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
inline constexpr const char* POSITION_4_FEN = // En passant legality
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
inline constexpr const char* POSITION_5_FEN = // Quiet move edge cases
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
inline constexpr const char* POSITION_6_FEN = // Promotion + check
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";