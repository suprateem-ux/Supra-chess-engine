#pragma once
#include <cstdint>

using Bitboard = uint64_t;

extern Bitboard RANK_MASKS[8];
extern Bitboard FILE_MASKS[8];
extern Bitboard SQUARE_BB[64];
extern const char* SQUARE_NAMES[64];

inline int popcount(Bitboard bb) { return __builtin_popcountll(bb); }
inline int lsb(Bitboard bb) { return __builtin_ctzll(bb); }
inline int msb(Bitboard bb) { return 63 - __builtin_clzll(bb); }

void init_bitboards();
