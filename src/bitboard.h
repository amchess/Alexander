/*
  Alexander, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2004-2026 The Alexander developers (see AUTHORS file)

  Alexander is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Alexander is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef BITBOARD_H_INCLUDED
#define BITBOARD_H_INCLUDED

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <string>
#include <type_traits>

#include "types.h"
#include "misc.h"

namespace Alexander {
//for classical begin
namespace Bitbases {

void init();
bool probe(Square wksq, Square wpsq, Square bksq, Color us);

}  // namespace Alexander::Bitbases
//for classical end

namespace Bitboards {

std::string pretty(Bitboard b);

}  // namespace Alexander::Bitboards

#ifdef USE_AVX512
// clang-format off
inline const __m512i AllSquares = _mm512_set_epi8(
    63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41,
    40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18,
    17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
// clang-format on
#endif

//for classical begin
// AllSquaresBB: the name AllSquares is used by Stockfish 19 for the AVX-512 index vector
constexpr Bitboard AllSquaresBB = ~Bitboard(0);
constexpr Bitboard DarkSquares  = 0xAA55AA55AA55AA55ULL;
//for classical end

constexpr Bitboard FileABB = 0x0101010101010101ULL;
constexpr Bitboard FileBBB = FileABB << 1;
constexpr Bitboard FileCBB = FileABB << 2;
constexpr Bitboard FileDBB = FileABB << 3;
constexpr Bitboard FileEBB = FileABB << 4;
constexpr Bitboard FileFBB = FileABB << 5;
constexpr Bitboard FileGBB = FileABB << 6;
constexpr Bitboard FileHBB = FileABB << 7;

constexpr Bitboard Rank1BB = 0xFF;
constexpr Bitboard Rank2BB = Rank1BB << (8 * 1);
constexpr Bitboard Rank3BB = Rank1BB << (8 * 2);
constexpr Bitboard Rank4BB = Rank1BB << (8 * 3);
constexpr Bitboard Rank5BB = Rank1BB << (8 * 4);
constexpr Bitboard Rank6BB = Rank1BB << (8 * 5);
constexpr Bitboard Rank7BB = Rank1BB << (8 * 6);
constexpr Bitboard Rank8BB = Rank1BB << (8 * 7);

//for classical begin
constexpr Bitboard QueenSide   = FileABB | FileBBB | FileCBB | FileDBB;
constexpr Bitboard CenterFiles = FileCBB | FileDBB | FileEBB | FileFBB;
constexpr Bitboard KingSide    = FileEBB | FileFBB | FileGBB | FileHBB;
constexpr Bitboard Center      = (FileDBB | FileEBB) & (Rank4BB | Rank5BB);

constexpr Bitboard KingFlank[FILE_NB] = {
  QueenSide ^ FileDBB, QueenSide, QueenSide, CenterFiles,
  CenterFiles,         KingSide,  KingSide,  KingSide ^ FileEBB};
//for classical end

constexpr Bitboard square_bb(Square s) {
    assert(is_ok(s));
    return 1ULL << s;
}


// Overloads of bitwise operators between a Bitboard and a Square for testing
// whether a given bit is set in a bitboard, and for setting and clearing bits.

constexpr Bitboard  operator&(Bitboard b, Square s) { return b & square_bb(s); }
constexpr Bitboard  operator|(Bitboard b, Square s) { return b | square_bb(s); }
constexpr Bitboard  operator^(Bitboard b, Square s) { return b ^ square_bb(s); }
constexpr Bitboard& operator|=(Bitboard& b, Square s) { return b |= square_bb(s); }
constexpr Bitboard& operator^=(Bitboard& b, Square s) { return b ^= square_bb(s); }

constexpr Bitboard operator&(Square s, Bitboard b) { return b & s; }
constexpr Bitboard operator|(Square s, Bitboard b) { return b | s; }
constexpr Bitboard operator^(Square s, Bitboard b) { return b ^ s; }

constexpr Bitboard operator|(Square s1, Square s2) { return square_bb(s1) | s2; }

constexpr bool more_than_one(Bitboard b) { return b & (b - 1); }

//for classical begin
constexpr bool opposite_colors(Square s1, Square s2) {
    return (s1 + rank_of(s1) + s2 + rank_of(s2)) & 1;
}
//for classical end


// rank_bb() and file_bb() return a bitboard representing all the squares on
// the given file or rank.

constexpr Bitboard rank_bb(Rank r) { return Rank1BB << (8 * r); }

constexpr Bitboard rank_bb(Square s) { return rank_bb(rank_of(s)); }

constexpr Bitboard file_bb(File f) { return FileABB << f; }

constexpr Bitboard file_bb(Square s) { return file_bb(file_of(s)); }


// Moves a bitboard one or two steps as specified by the direction D
inline constexpr Bitboard shift(Bitboard b, Direction dir) {
    return dir == NORTH         ? b << 8
         : dir == SOUTH         ? b >> 8
         : dir == NORTH + NORTH ? b << 16
         : dir == SOUTH + SOUTH ? b >> 16
         : dir == EAST          ? (b & ~FileHBB) << 1
         : dir == WEST          ? (b & ~FileABB) >> 1
         : dir == NORTH_EAST    ? (b & ~FileHBB) << 9
         : dir == NORTH_WEST    ? (b & ~FileABB) << 7
         : dir == SOUTH_EAST    ? (b & ~FileHBB) >> 7
         : dir == SOUTH_WEST    ? (b & ~FileABB) >> 9
                                : 0;
}

//for classical begin
// Template form of shift() used by the classical evaluation (Stockfish 18 API)
template<Direction D>
constexpr Bitboard shift(Bitboard b) {
    return shift(b, D);
}
//for classical end


// Returns the squares attacked by pawns of the given color
// from the squares in the given bitboard.
template<Color C>
constexpr Bitboard pawn_attacks_bb(Bitboard b) {
    return C == WHITE ? shift(b, NORTH_WEST) | shift(b, NORTH_EAST)
                      : shift(b, SOUTH_WEST) | shift(b, SOUTH_EAST);
}

//for classical begin
// pawn_double_attacks_bb() returns the squares doubly attacked by pawns of the
// given color from the squares in the given bitboard.
template<Color C>
constexpr Bitboard pawn_double_attacks_bb(Bitboard b) {
    return C == WHITE ? shift<NORTH_WEST>(b) & shift<NORTH_EAST>(b)
                      : shift<SOUTH_WEST>(b) & shift<SOUTH_EAST>(b);
}

// adjacent_files_bb() returns a bitboard representing all the squares on the
// adjacent files of a given square.
constexpr Bitboard adjacent_files_bb(Square s) {
    return shift<EAST>(file_bb(s)) | shift<WEST>(file_bb(s));
}
//for classical end

constexpr Bitboard pawn_single_push_bb(Color c, Bitboard b) {
    return shift(b, c == WHITE ? NORTH : SOUTH);
}

inline constexpr auto PawnPairBB = []() {
    std::array<Bitboard, SQUARE_NB> result{};
    for (Square s = SQ_A1; s <= SQ_H8; ++s)
    {
        Bitboard file  = file_bb(s);
        Bitboard files = file | shift(file, EAST) | shift(file, WEST);
        result[s]      = files & ~(Rank1BB | Rank8BB) & ~square_bb(s);
    }
    return result;
}();

// Returns the squares that can host a pawn forming a "pawn pair" with a pawn
// on s: own file plus adjacent files, restricted to ranks 2-7, excluding s.
// The geometry is color-independent.
constexpr Bitboard pawn_pair_bb(Square s) { return PawnPairBB[s]; }

//for classical begin
// forward_ranks_bb() returns a bitboard representing the squares on the ranks in
// front of the given one, from the point of view of the given color. For instance,
// forward_ranks_bb(BLACK, SQ_D3) will return the 16 squares on ranks 1 and 2.
constexpr Bitboard forward_ranks_bb(Color c, Square s) {
    return c == WHITE ? ~Rank1BB << 8 * relative_rank(WHITE, s)
                      : ~Rank8BB >> 8 * relative_rank(BLACK, s);
}

// forward_file_bb() returns a bitboard representing all the squares along the
// line in front of the given one, from the point of view of the given color.
constexpr Bitboard forward_file_bb(Color c, Square s) { return forward_ranks_bb(c, s) & file_bb(s); }

// pawn_attack_span() returns a bitboard representing all the squares that can
// be attacked by a pawn of the given color when it moves along its file, starting
// from the given square.
constexpr Bitboard pawn_attack_span(Color c, Square s) {
    return forward_ranks_bb(c, s) & adjacent_files_bb(s);
}

// passed_pawn_span() returns a bitboard which can be used to test if a pawn of
// the given color and on the given square is a passed pawn.
constexpr Bitboard passed_pawn_span(Color c, Square s) {
    return pawn_attack_span(c, s) | forward_file_bb(c, s);
}

// Square distance (number of king steps), as in Stockfish 18. Stockfish 19 removed
// the table and the distance() helpers, still used by the classical evaluation.
inline constexpr auto SquareDistance = []() {
    std::array<std::array<u8, SQUARE_NB>, SQUARE_NB> result{};
    for (Square s1 = SQ_A1; s1 <= SQ_H8; ++s1)
        for (Square s2 = SQ_A1; s2 <= SQ_H8; ++s2)
        {
            const int df = file_of(s1) > file_of(s2) ? file_of(s1) - file_of(s2)
                                                     : file_of(s2) - file_of(s1);
            const int dr = rank_of(s1) > rank_of(s2) ? rank_of(s1) - rank_of(s2)
                                                     : rank_of(s2) - rank_of(s1);
            result[s1][s2] = u8(df > dr ? df : dr);
        }
    return result;
}();

// distance() functions return the distance between x and y, defined as the
// number of steps for a king in x to reach y.
template<typename T1 = Square>
inline int distance(Square x, Square y);

template<>
inline int distance<File>(Square x, Square y) {
    return std::abs(file_of(x) - file_of(y));
}

template<>
inline int distance<Rank>(Square x, Square y) {
    return std::abs(rank_of(x) - rank_of(y));
}

template<>
inline int distance<Square>(Square x, Square y) {
    return SquareDistance[x][y];
}
//for classical end

constexpr int edge_distance(File f) { return std::min(f, File(FILE_H - f)); }
constexpr int edge_distance(Rank r) { return std::min(r, Rank(RANK_8 - r)); }  //for classical

template<typename T>
constexpr int constexpr_popcount(T v) {
    static_assert(std::is_integral_v<T>, "constexpr_popcount is undefined for non-integral types");

    if constexpr (sizeof(T) <= 8)
    {
        u64 b = static_cast<std::make_unsigned_t<T>>(v);

        b = b - ((b >> 1) & 0x5555555555555555ULL);
        b = (b & 0x3333333333333333ULL) + ((b >> 2) & 0x3333333333333333ULL);
        b = (b + (b >> 4)) & 0x0F0F0F0F0F0F0F0FULL;

        return static_cast<int>((b * 0x0101010101010101ULL) >> 56);
    }
    else
    {
        int result = 0;

        for (; v; v >>= static_cast<T>(1))
            if (v & static_cast<T>(1))
                ++result;

        return result;
    }
}

// Counts the number of non-zero bits in a bitboard.
inline int popcount(Bitboard b) {

#ifdef _MSC_VER

    return int(_mm_popcnt_u64(b));

#else  // Assumed gcc or compatible compiler

    return __builtin_popcountll(b);

#endif
}

inline constexpr int lsb_index64[64] = {
  0,  47, 1,  56, 48, 27, 2,  60, 57, 49, 41, 37, 28, 16, 3,  61, 54, 58, 35, 52, 50, 42,
  21, 44, 38, 32, 29, 23, 17, 11, 4,  62, 46, 55, 26, 59, 40, 36, 15, 53, 34, 51, 20, 43,
  31, 22, 10, 45, 25, 39, 14, 33, 19, 30, 9,  24, 13, 18, 8,  12, 7,  6,  5,  63};

constexpr int constexpr_lsb(u64 bb) {
    assert(bb != 0);
    constexpr u64 debruijn64 = 0x03F79D71B4CB0A89ULL;
    return lsb_index64[((bb ^ (bb - 1)) * debruijn64) >> 58];
}

// Returns the least significant bit in a non-zero bitboard.
inline Square lsb(Bitboard b) {
    assert(b);

#if defined(__GNUC__)  // GCC, Clang, ICX

    return Square(__builtin_ctzll(b));

#elif defined(_MSC_VER)
    #ifdef _WIN64  // MSVC, WIN64

    unsigned long idx;
    _BitScanForward64(&idx, b);
    return Square(idx);

    #else  // MSVC, WIN32
    unsigned long idx;

    if (b & 0xffffffff)
    {
        _BitScanForward(&idx, i32(b));
        return Square(idx);
    }
    else
    {
        _BitScanForward(&idx, i32(b >> 32));
        return Square(idx + 32);
    }
    #endif
#else  // Compiler is neither GCC nor MSVC compatible
    #error "Compiler not supported."
#endif
}

// Returns the most significant bit in a non-zero bitboard.
inline Square msb(Bitboard b) {
    assert(b);

#if defined(__GNUC__)  // GCC, Clang, ICX

    return Square(63 ^ __builtin_clzll(b));

#elif defined(_MSC_VER)
    #ifdef _WIN64  // MSVC, WIN64

    unsigned long idx;
    _BitScanReverse64(&idx, b);
    return Square(idx);

    #else  // MSVC, WIN32

    unsigned long idx;

    if (b >> 32)
    {
        _BitScanReverse(&idx, i32(b >> 32));
        return Square(idx + 32);
    }
    else
    {
        _BitScanReverse(&idx, i32(b));
        return Square(idx);
    }
    #endif
#else  // Compiler is neither GCC nor MSVC compatible
    #error "Compiler not supported."
#endif
}

// Returns the bitboard of the least significant
// square of a non-zero bitboard. It is equivalent to square_bb(lsb(bb)).
inline Bitboard least_significant_square_bb(Bitboard b) {
    assert(b);
    return b & -b;
}

// Finds and clears the least significant bit in a non-zero bitboard.
inline Square pop_lsb(Bitboard& b) {
    assert(b);
    const Square s = lsb(b);
    b &= b - 1;
    return s;
}

//for classical begin
// frontmost_sq() returns the most advanced square for the given color,
// requires a non-zero bitboard.
inline Square frontmost_sq(Color c, Bitboard b) {
    assert(b);
    return c == WHITE ? msb(b) : lsb(b);
}
//for classical end

}  // namespace Alexander

#endif  // #ifndef BITBOARD_H_INCLUDED
