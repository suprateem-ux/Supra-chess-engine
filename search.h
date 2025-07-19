#pragma once
#include "movegen.h"
#include <string>

namespace Search {
    constexpr int MAX_DEPTH = 64;
    constexpr int INF = 100000;
    void start_search(const std::string& cmd);
    void set_threads(int n);
}
