#pragma once
#include "movegen.h"
#include "position.h"
#include <string>

namespace Search {
    constexpr int MAX_DEPTH = 64;
    constexpr int INF = 100000;

    // Stack frame used during search (stores principal variation, etc.)
    struct SearchStack {
        Move pv[MAX_DEPTH];
        int pv_length = 0;
    };

    // Set number of threads to be used
    void set_threads(int n);

    // Start search based on UCI "go" command
    void start_search(const std::string& cmd);

    // Worker thread function
    void search_thread(int id);

    // Core search function (PVS, alpha-beta, etc.)
    int search(Position& pos, SearchStack* stack, int alpha, int beta, int depth, int ply, bool root);
}
