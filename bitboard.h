#pragma once
#include <cstdint>

using Bitboard = uint64_t;

// Bitboard constants
constexpr Bitboard EMPTY = 0ULL;
constexpr Bitboard FULL = 0xFFFFFFFFFFFFFFFFULL;

// Rank and file masks
extern Bitboard RANK_MASKS[8];
extern Bitboard FILE_MASKS[8];

// Initialization of masks and other bitboard setup
void init_bitboards();

// Bit manipulation utilities

// Popcount: number of set bits in a bitboard
inline int popcount(Bitboard bb) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(bb);
#else
    // Fallback implementation (Kernighan's algorithm)
    int count = 0;
    while (bb) {
        bb &= bb - 1;
        count++;
    }
    return count;
#endif
}

// Least significant bit index (0-based), returns -1 if bb is 0
inline int lsb(Bitboard bb) {
    if (bb == 0) return -1;
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_ctzll(bb);
#else
    int index = 0;
    while ((bb & 1) == 0) {
        bb >>= 1;
        index++;
    }
    return index;
#endif
}

// Most significant bit index (0-based), returns -1 if bb is 0
inline int msb(Bitboard bb) {
    if (bb == 0) return -1;
#if defined(__GNUC__) || defined(__clang__)
    return 63 - __builtin_clzll(bb);
#else
    int index = 63;
    while ((bb & (1ULL << index)) == 0) {
        index--;
    }
    return index;
#endif
}

