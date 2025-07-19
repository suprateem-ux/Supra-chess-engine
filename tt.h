#pragma once
#include <cstdint>
#include <vector>

struct TTEntry {
    uint64_t key;
    int depth;
    int score;
    int flag;
    int bestMove;
};

class TranspositionTable {
public:
    void resize(size_t size);
    void store(uint64_t key, const TTEntry& entry);
    TTEntry* probe(uint64_t key);
};

extern TranspositionTable TT;
void init_zobrist();
