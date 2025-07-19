#pragma once
#include <cstdint>
#include <vector>

struct TTEntry {
    uint64_t key;
    int depth;
    int score;
    int flag;
};

class TranspositionTable {
public:
    void resize(size_t mb);
    void store(uint64_t key, int depth, int score, int flag);
    bool probe(uint64_t key, TTEntry& entry);
};

extern TranspositionTable TT;
void init_zobrist();
