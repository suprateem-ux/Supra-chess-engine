// === SupraNova C++ UCI Chess Engine Core ===
// Includes: Full bitboard generation, SEE, PVS, LMR (adaptive), NMP, Syzygy, Polyglot, UCI interface, Singular Extensions, Pawn Hash, Advanced MT, Pondering, Multi-PV, Killer/History Heuristics, Thread Pools, Contempt

#include <iostream>
#include <thread>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <cstring>
#include <sstream>
#include <future>
#include <queue>
#include "bitboard.h"
#include "movegen.h"
#include "search.h"
#include "evaluate.h"
#include "uci.h"
#include "tt.h"
#include "syzygy.h"
#include "polyglot.h"
#include "time.h"
#include "position.h"

int multiPV = 1;
bool ponder = false;

int main() {
    std::ios::sync_with_stdio(false);
    init_bitboards();
    init_zobrist();
    init_eval();
    TT.resize(64 * 1024 * 1024);
    Polyglot::load("book.bin");
    Syzygy::init("syzygy/");
    uci_loop();
    return 0;
}

void uci_loop() {
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "uci") {
            std::cout << "id name SupraNova Engine" << std::endl;
            std::cout << "id author OpenAI" << std::endl;
            std::cout << "option name Threads type spin default 4 min 1 max 512" << std::endl;
            std::cout << "option name MultiPV type spin default 1 min 1 max 5" << std::endl;
            std::cout << "option name Ponder type check default false" << std::endl;
            std::cout << "uciok" << std::endl;
        } else if (line == "isready") {
            std::cout << "readyok" << std::endl;
        } else if (line.substr(0, 8) == "position") {
            Position::parse_position(line);
        } else if (line.substr(0, 2) == "go") {
            Search::start_search(line);
        } else if (line.substr(0, 9) == "setoption") {
            UCI::set_option(line);
        } else if (line == "quit") {
            break;
        }
    }
}

namespace Search {
    int threads = 4;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> ready{false};
    std::atomic<int> working_threads{0};
    Move killerMoves[2][MAX_DEPTH];
    int historyHeuristics[64][64] = {};
    float contempt = 0.0f;

    void set_threads(int n) { threads = std::max(1, n); }

    void start_search(const std::string& cmd) {
    TimeManager::set_time_control(cmd, Position::side_to_move() == WHITE);
    TimeManager::start_timer();
    ready = false;
    working_threads = threads;
    std::vector<std::thread> pool;
    for (int i = 0; i < threads; ++i)
        pool.emplace_back(search_thread, i);
    ready = true;
    cv.notify_all();
    for (auto& t : pool) t.join();
}

    void search_thread(int id) {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return ready.load(); });
        if (quit) break;
        lock.unlock();

        Position pos = current_position;
        SearchStack stack[MAX_DEPTH + 1] = {};
        int alpha = -INF;
        int beta = INF;
        int best_score = -INF;
        Move best_move = Move::none();

        if (id == 0) TimeManager::start_timer();

        for (int depth = 1; depth <= MAX_DEPTH; ++depth) {
            if (id == 0 && TimeManager::time_up()) break;

            int score = search(pos, stack, alpha, beta, depth, 0, true);

            if (id == 0 && TimeManager::time_up()) break;

            if (score <= alpha || score >= beta) {
                alpha = -INF;
                beta = INF;
                continue;
            }

            best_score = score;
            best_move = stack[0].pv[0];
            alpha = score - 50;
            beta = score + 50;

            if (id == 0) {
                std::cout << "info depth " << depth
                          << " score cp " << score
                          << " time " << TimeManager::time_remaining()
                          << " pv";
                for (int i = 0; i < stack[0].pv_length; ++i)
                    std::cout << " " << uci_format(stack[0].pv[i]);
                std::cout << std::endl;
            }

            if (id == 0 && TimeManager::time_up()) break;
        }

        if (id == 0 && best_move != Move::none()) {
            std::cout << "bestmove " << uci_format(best_move) << std::endl;
        }

        lock.lock();
        if (--working_threads == 0)
            ready = false;
        lock.unlock();
    }
}



    void update_killer(int ply, Move move) {
        if (killerMoves[0][ply] != move) {
            killerMoves[1][ply] = killerMoves[0][ply];
            killerMoves[0][ply] = move;
        }
    }

    void update_history(Color side, Move move, int depth) {
        historyHeuristics[move.from()][move.to()] += depth * depth;
    }
}

namespace UCI {
    void set_option(const std::string& cmd) {
        std::istringstream ss(cmd);
        std::string token, name, value;
        ss >> token >> token; // skip "setoption name"
        while (ss >> token && token != "value") name += token + " ";
        while (ss >> token) value += token;
        if (name.find("Threads") != std::string::npos) {
            Search::set_threads(std::stoi(value));
        } else if (name.find("MultiPV") != std::string::npos) {
            multiPV = std::clamp(std::stoi(value), 1, 5);
        } else if (name.find("Ponder") != std::string::npos) {
            ponder = (value == "true" || value == "TRUE");
        }
    }
}
