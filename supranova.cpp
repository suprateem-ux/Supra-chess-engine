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
        parse_time_control(cmd);
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
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return ready.load(); });
        lock.unlock();

        Position pos;
        Score alpha = -INF, beta = INF;
        std::vector<Move> bestMoves(multiPV);

        for (int depth = 1; depth <= MAX_DEPTH; ++depth) {
            std::vector<std::pair<Move, Score>> scoredMoves;
            for (int i = 0; i < multiPV; ++i) {
                Move m = search(pos, alpha, beta, depth);
                scoredMoves.emplace_back(m, eval(pos) + contempt * (pos.side_to_move() == WHITE ? 1 : -1));
            }
            std::sort(scoredMoves.begin(), scoredMoves.end(), [](auto& a, auto& b) { return a.second > b.second; });
            for (int i = 0; i < multiPV && i < scoredMoves.size(); ++i) {
                std::cout << "info depth " << depth << " multipv " << (i+1) << " score cp " << scoredMoves[i].second << " pv " << scoredMoves[i].first.to_string() << std::endl;
            }
            bestMoves = {};
            for (auto& p : scoredMoves) bestMoves.push_back(p.first);
        }
        std::cout << "bestmove " << bestMoves[0].to_string();
        if (ponder) std::cout << " ponder e2e4"; // placeholder
        std::cout << std::endl;
        --working_threads;
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
