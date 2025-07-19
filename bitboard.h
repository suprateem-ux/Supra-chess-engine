#pragma once
#include <cstdint>

using Bitboard = uint64_t;

extern Bitboard RANK_MASKS[8];
extern Bitboard FILE_MASKS[8];

void init_bitboards();

inline int popcount(Bitboard bb);
inline int lsb(Bitboard bb);
inline int msb(Bitboard bb);
