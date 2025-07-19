#pragma once
#include <string>
#include "movegen.h"

constexpr int MAX_DEPTH = 64;
constexpr int INF = 100000;
using Score = int;

namespace Search {
    void start_search(const std::string& cmd);
    void set_threads(int n);
    void update_killer(int ply, Move move);
    void update_history(Color side, Move move, int depth);
}
