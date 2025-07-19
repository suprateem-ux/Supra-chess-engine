#pragma once
#include <string>
#include "movegen.h"

#define MAX_DEPTH 64
#define INF 100000

namespace Search {
    void start_search(const std::string& cmd);
    void set_threads(int n);
    void update_killer(int ply, Move move);
    void update_history(Color side, Move move, int depth);
}
