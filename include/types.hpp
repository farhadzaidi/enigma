#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

// --- Type Definitions ---

using Bitmask           = uint64_t;
using Bitboard          = uint64_t;
using Move              = uint16_t;
using MoveType          = uint16_t;
using MoveFlag          = uint16_t;
using Square            = uint8_t;
using Color             = uint8_t;
using Piece             = uint8_t;
using CastlingRights    = uint8_t;

// --- Enums ---

enum SquareEnum : Square {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
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

enum MoveTypeEnum : MoveType {
    QUIET,
    CAPTURE
};

enum MoveFlagEnum : MoveFlag {
    NORMAL,
    EN_PASSANT,
    CASTLE,
    PROMOTION_BISHOP,
    PROMOTION_KNIGHT,
    PROMOTION_ROOK,
    PROMOTION_QUEEN
};

// --- Board Constants ---

inline constexpr int NUM_SQUARES    = 64;
inline constexpr int NUM_COLORS     = 2;
inline constexpr int NUM_PIECES     = 6;
inline constexpr int BOARD_SIZE     = 8;

inline constexpr int RANK_1 = 0;
inline constexpr int RANK_2 = 1;
inline constexpr int RANK_3 = 2;
inline constexpr int RANK_4 = 3;
inline constexpr int RANK_5 = 4;
inline constexpr int RANK_6 = 5;
inline constexpr int RANK_7 = 6;
inline constexpr int RANK_8 = 7;

inline constexpr int A_FILE = 0;
inline constexpr int B_FILE = 1;
inline constexpr int C_FILE = 2;
inline constexpr int D_FILE = 3;
inline constexpr int E_FILE = 4;
inline constexpr int F_FILE = 5;
inline constexpr int G_FILE = 6;
inline constexpr int H_FILE = 7;

// Bitmasks

inline constexpr Bitmask RANK_1_MASK = 0x00000000000000FF;
inline constexpr Bitmask RANK_2_MASK = 0x000000000000FF00;
inline constexpr Bitmask RANK_3_MASK = 0x0000000000FF0000;
inline constexpr Bitmask RANK_4_MASK = 0x00000000FF000000;
inline constexpr Bitmask RANK_5_MASK = 0x000000FF00000000;
inline constexpr Bitmask RANK_6_MASK = 0x0000FF0000000000;
inline constexpr Bitmask RANK_7_MASK = 0x00FF000000000000;
inline constexpr Bitmask RANK_8_MASK = 0xFF00000000000000;

inline constexpr Bitmask A_FILE_MASK = 0x0101010101010101;
inline constexpr Bitmask B_FILE_MASK = 0x0202020202020202;
inline constexpr Bitmask C_FILE_MASK = 0x0404040404040404;
inline constexpr Bitmask D_FILE_MASK = 0x0808080808080808;
inline constexpr Bitmask E_FILE_MASK = 0x1010101010101010;
inline constexpr Bitmask F_FILE_MASK = 0x2020202020202020;
inline constexpr Bitmask G_FILE_MASK = 0x4040404040404040;
inline constexpr Bitmask H_FILE_MASK = 0x8080808080808080;

// --- Sentinel Values ---

inline constexpr Square NO_SQUARE                   = -1;
inline constexpr CastlingRights NO_CASTLING_RIGHTS  =  0;
inline constexpr Bitboard EMPTY_BITBOARD            =  0;

// --- FEN Strings ---

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