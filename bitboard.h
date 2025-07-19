#pragma once
#include <cstdint>

typedef uint64_t Bitboard;

extern Bitboard mask_rank[8];
extern Bitboard mask_file[8];
extern Bitboard knight_attacks[64];
extern Bitboard king_attacks[64];

void init_bitboards();
