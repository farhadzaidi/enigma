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