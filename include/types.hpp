#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <atomic>
#include <limits>
#include <filesystem>
#include <limits>

// This is really just to silence the IDE warning since PROJECT_ROOT 
// should be defined in CMakeLists.txt
#ifndef PROJECT_ROOT
#define PROJECT_ROOT "./"
#endif

// --- Globals ---
extern std::atomic<bool> stop_requested;
inline std::filesystem::path FEN_DIR = std::filesystem::path(PROJECT_ROOT) / "fen";

// --- FEN/EPD Files ---
inline const std::filesystem::path SINGLE_CHECK_EPD = FEN_DIR / "single_check.epd";
inline const std::filesystem::path DOUBLE_CHECK_EPD = FEN_DIR / "double_check.epd";
inline const std::filesystem::path NOT_IN_CHECK_FEN = FEN_DIR / "not_in_check.fen";
inline const std::filesystem::path MIXED_EPD = FEN_DIR / "mixed.epd";
inline const std::filesystem::path CPW_EPD = FEN_DIR / "cpw.epd";
inline const std::filesystem::path EN_PASSANT_EPD = FEN_DIR / "en_passant.epd";
inline const std::filesystem::path ENGINE_EPD = FEN_DIR / "engine.epd";

// --- Board Constants ---

constexpr int NUM_SQUARES    = 64;
constexpr int NUM_COLORS     = 2;
constexpr int NUM_PIECES     = 6;
constexpr int BOARD_SIZE     = 8;

// --- Bounds ---

// Upper bound for the maximum depth (ply) we can search in a given position
constexpr int MAX_PLY   = 256;

// Upper bound for the maximum number of moves we can generate at a given depth
constexpr int MAX_MOVES = 256;

constexpr int MAX_SCORE          =  30'000;
constexpr int MIN_SCORE          = -MAX_SCORE;
constexpr int CHECKMATE_SCORE    =  32000;
constexpr int STALEMATE_SCORE    =  0;
constexpr int SEARCH_INTERRUPTED =  std::numeric_limits<int>::min() / 2;

// --- Type Definitions ---

using Bitboard          = uint64_t;
using MoveScore         = uint32_t;
using MoveType          = uint16_t;
using MoveFlag          = uint16_t;
using Square            = uint8_t;
using Color             = uint8_t;
using Piece             = uint8_t;
using CastlingRights    = uint8_t;
using Rank              = uint8_t;
using File              = uint8_t;
using CastleType        = uint8_t;
using MoveSelectorPhase = uint8_t;
using Direction         = int;
using SearchMode        = int;
using MoveGenMode       = int;

// --- History Table Type Definitions ---

// color_piece_to[color][piece][to]
using ColorPieceToHistory = std::array<std::array<std::array<MoveScore, NUM_SQUARES>, NUM_PIECES>, NUM_COLORS>;

// from_to[from][to]
using FromToHistory = std::array<std::array<MoveScore, NUM_SQUARES>, NUM_SQUARES>;

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
    NO_SQUARE
};

enum DirectionEnum : Direction {
    NO_DIRECTION = 0,
    NORTH = 8,
    EAST = 1,
    SOUTH = -NORTH,
    WEST = -EAST,

    NORTHEAST = NORTH + EAST,
    NORTHWEST = NORTH + WEST,
    SOUTHEAST = SOUTH + EAST,
    SOUTHWEST = SOUTH + WEST,

    NORTH_NORTH = NORTH + NORTH,
    SOUTH_SOUTH = SOUTH + SOUTH
};

enum RankEnum : Rank {
    RANK_1,
    RANK_2,
    RANK_3,
    RANK_4,
    RANK_5,
    RANK_6,
    RANK_7,
    RANK_8
};

enum FileEnum : File {
    A_FILE,
    B_FILE,
    C_FILE,
    D_FILE,
    E_FILE,
    F_FILE,
    G_FILE,
    H_FILE,
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
    NO_CASTLING_RIGHTS = 0b0,
    WHITE_SHORT        = 0b1,
    WHITE_LONG         = 0b10,
    BLACK_SHORT        = 0b100,
    BLACK_LONG         = 0b1000
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

enum CastleTypeEnum : CastleType {
    NO_CASTLE_TYPE,
    WHITE_SHORT_CASTLE_TYPE,
    WHITE_LONG_CASTLE_TYPE,
    BLACK_SHORT_CASTLE_TYPE,
    BLACK_LONG_CASTLE_TYPE
};

enum SearchModeEnum : SearchMode {
    TIME,
    NODES,
    DEPTH,
    INFINITE
};

enum MoveSelectorPhaseEnum : MoveSelectorPhase {
    TRANSPOSITION,
    GOOD_CAPTURE,
    KILLER,
    QUIET_MOVE,
    BAD_CAPTURE
};

enum MoveGenModeEnum : MoveGenMode {
    ALL,
    QUIET_ONLY,
    CAPTURE_ONLY
};

// Ranks and Files

constexpr Bitboard RANK_1_MASK = 0x00000000000000FFULL;
constexpr Bitboard RANK_2_MASK = 0x000000000000FF00ULL;
constexpr Bitboard RANK_3_MASK = 0x0000000000FF0000ULL;
constexpr Bitboard RANK_4_MASK = 0x00000000FF000000ULL;
constexpr Bitboard RANK_5_MASK = 0x000000FF00000000ULL;
constexpr Bitboard RANK_6_MASK = 0x0000FF0000000000ULL;
constexpr Bitboard RANK_7_MASK = 0x00FF000000000000ULL;
constexpr Bitboard RANK_8_MASK = 0xFF00000000000000ULL;

constexpr Bitboard A_FILE_MASK = 0x0101010101010101ULL;
constexpr Bitboard B_FILE_MASK = 0x0202020202020202ULL;
constexpr Bitboard C_FILE_MASK = 0x0404040404040404ULL;
constexpr Bitboard D_FILE_MASK = 0x0808080808080808ULL;
constexpr Bitboard E_FILE_MASK = 0x1010101010101010ULL;
constexpr Bitboard F_FILE_MASK = 0x2020202020202020ULL;
constexpr Bitboard G_FILE_MASK = 0x4040404040404040ULL;
constexpr Bitboard H_FILE_MASK = 0x8080808080808080ULL;

// Castling Path

constexpr Bitboard WHITE_LONG_CASTLE_PATH   = 0x000000000000000EULL;
constexpr Bitboard WHITE_SHORT_CASTLE_PATH  = 0x0000000000000060ULL;
constexpr Bitboard BLACK_LONG_CASTLE_PATH   = 0x0E00000000000000ULL;
constexpr Bitboard BLACK_SHORT_CASTLE_PATH  = 0x6000000000000000ULL;

// --- Sentinel Values ---

constexpr Bitboard EMPTY_BITBOARD = 0;

// --- FEN Strings ---

constexpr const char* START_POS_FEN = 
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
constexpr const char* KIWIPETE_FEN = 
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
constexpr const char* POSITION_3_FEN = // Castling, en passant, and promotions
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
constexpr const char* POSITION_4_FEN = // En passant legality
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
constexpr const char* POSITION_5_FEN = // Quiet move edge cases
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  ";
constexpr const char* POSITION_6_FEN = // Promotion + check
    "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ";


// --- Evaluation ---
constexpr std::array<int, NUM_PIECES> PIECE_VALUE = {
    // PAWN, KNIGHT/BISHOP, ROOK, QUEEN, KING
    100, 300, 300, 500, 900, 0 // King adds nothing to material value since it can never be captured
};

// --- Magic Maps ---
// These are magic numbers which are useful for looking up attack masks for sliding pieces.
// They are generated (via brute-force) using compute_magic_numbers() inside precompute.hpp

constexpr std::array<uint64_t, NUM_SQUARES> BISHOP_MAGIC = {
    290491063393657344ULL,
    1134842633265152ULL,
    4649984774927155200ULL,
    2568742656016384ULL,
    72356936150419462ULL,
    2328664610067973153ULL,
    14411590344832593920ULL,
    37176689507442688ULL,
    37194863728672896ULL,
    10377146831273508944ULL,
    155389598090953472ULL,
    13245737861120ULL,
    2594077805088082049ULL,
    1271448731648ULL,
    1190077372182825008ULL,
    2258405507072512ULL,
    2885393523753992ULL,
    112590282809671776ULL,
    10957258477512441880ULL,
    596727020021354496ULL,
    1730508165441724440ULL,
    562955457005570ULL,
    2308380733808837120ULL,
    10381958227549489792ULL,
    1189531948111106048ULL,
    2595202034320802122ULL,
    9512730517574002761ULL,
    565149514104840ULL,
    9260108920326725640ULL,
    13836189040448114688ULL,
    90639343106130182ULL,
    10088350140195930369ULL,
    2287056530702880ULL,
    919200349556800ULL,
    20338010176884896ULL,
    18023228964473344ULL,
    565166156677250ULL,
    36327873308853256ULL,
    1157997541804083200ULL,
    77726676527775872ULL,
    2450522384730474505ULL,
    3026564153983631872ULL,
    1162584013462963200ULL,
    9223794532796139522ULL,
    342275945220015104ULL,
    11538226712392253472ULL,
    301747815057523724ULL,
    36593963244716288ULL,
    324541208105844736ULL,
    142940859021312ULL,
    1152928103321305608ULL,
    3299080667201ULL,
    3497678738859231232ULL,
    5101769117974800ULL,
    19144731130068994ULL,
    589372600647680ULL,
    9078702004636236ULL,
    2289187563275264ULL,
    1515461573119320384ULL,
    36038263145793536ULL,
    5764607560348996096ULL,
    9313448582336135684ULL,
    9621958204922792070ULL,
    5206163660333940992ULL,
};

constexpr std::array<uint64_t, NUM_SQUARES>  ROOK_MAGIC = {
    1765411328882712592ULL,
    8088482524017336328ULL,
    4683796427680251968ULL,
    36037593187487744ULL,
    2449975807192863232ULL,
    216179383481140224ULL,
    36029896631255168ULL,
    8214566032254239236ULL,
    2612369401801868544ULL,
    70437465751616ULL,
    576742433975500864ULL,
    36310375343333632ULL,
    2306265256038760832ULL,
    4901043462784163848ULL,
    11532029830263702016ULL,
    171699736900862612ULL,
    9133093340315778ULL,
    90160498803200ULL,
    288301844944409856ULL,
    40542292519428353ULL,
    882846814343015424ULL,
    6953699112105542144ULL,
    4632238090883696976ULL,
    76000443071062145ULL,
    90072544450736192ULL,
    297238435473731584ULL,
    18695996874752ULL,
    1153204117750939680ULL,
    149537877659904ULL,
    567350147547264ULL,
    9150152946764048ULL,
    9223653584848126018ULL,
    9007474141069312ULL,
    295021028688527424ULL,
    7206040957169442816ULL,
    4611773981513484288ULL,
    6919415791750219776ULL,
    140754676621825ULL,
    1297046932488716808ULL,
    176507083293699ULL,
    9259401246262444032ULL,
    585555914099015688ULL,
    153123487115182112ULL,
    13835251569731338368ULL,
    4919619679524356128ULL,
    36592313916489730ULL,
    1170546622472ULL,
    18085181734912004ULL,
    306244922837713024ULL,
    9227876323907600448ULL,
    1689537327694336ULL,
    35527970062592ULL,
    1170949099403870848ULL,
    578996226184216704ULL,
    2305983759587606656ULL,
    1153066937174197760ULL,
    35760199713026ULL,
    2305992620658147329ULL,
    578888551295354946ULL,
    9811373298177560577ULL,
    5810206606768506882ULL,
    189714151617400834ULL,
    3378833860373028ULL,
    9224498503701561378ULL,
};