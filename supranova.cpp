// supranova.cpp
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
#include "movegen.h"  // Move class already defined here
#include "evaluate.h"
#include "uci.h"
#include "syzygy.h"
#include "polyglot.h"
#include "time.h"

const char* SquareNames[64] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};

inline int file_of(int sq) { return sq % 8; }
inline int rank_of(int sq) { return sq / 8; }

void init_bitboards() {}
void init_zobrist() {}

namespace Eval { void init_eval() {} }

class TranspositionTable {
public:
    void resize(size_t sizeMB) {}
};
TranspositionTable TT;

namespace Polyglot { void load(const std::string&) {} }
namespace Syzygy { void init(const std::string&) {} }

enum Color { WHITE, BLACK };
class Position {
public:
    // Minimal members for now
    std::string fen;

    Position() = default;

    Position(const std::string& fen_str) {
        set_fen(fen_str);
    }

    void set_fen(const std::string& fen_str) {
        fen = fen_str;
        // TODO: Actually parse FEN
    }

    std::string get_fen() const {
        return fen;
    }
};


Position current_position;

int multiPV = 1;
bool ponder = false;
std::atomic<bool> quit{false};

std::string uci_format(const Move& move) { return move.to_string(); }

int main() {
    std::ios::sync_with_stdio(false);
    init_bitboards();
    init_zobrist();
    Eval::init_eval();
    TT.resize(64 * 1024 * 1024);
    Polyglot::load("book.bin");
    Syzygy::init("syzygy/");
    UCI::uci_loop();
    return 0;
}

namespace Search {
    constexpr int MAX_DEPTH = 64;
    constexpr int INF = 100000;

    struct SearchStack {
        Move pv[64];
        int pv_length = 0;
    };

    int threads = 4;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> ready{false};
    std::atomic<int> working_threads{0};
    Move killerMoves[2][MAX_DEPTH];
    int historyHeuristics[64][64] = {};
    float contempt = 0.0f;

    void set_threads(int n) {
        threads = std::max(1, n);
    }

    int search(Position&, SearchStack*, int, int, int, int, bool) {
        return 0;
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
            Move best_move;

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

            if (id == 0 && best_move != Move()) {
                std::cout << "bestmove " << uci_format(best_move) << std::endl;
            }

            lock.lock();
            if (--working_threads == 0)
                ready = false;
            lock.unlock();
        }
    }

    void start_search(const std::string& cmd) {
        TimeManager::set_time_control(cmd, current_position.side_to_move() == WHITE);
        TimeManager::start_timer();
        ready = false;
        working_threads = threads;

        std::vector<std::thread> pool;
        for (int i = 0; i < threads; ++i)
            pool.emplace_back(search_thread, i);

        ready = true;
        cv.notify_all();

        for (auto& t : pool)
            t.join();
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
        ss >> token >> token;
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

    void uci_loop() {
        std::string line;
        while (std::getline(std::cin, line)) {
            if (line == "uci") {
                std::cout << "id name SupraNova Engine" << std::endl;
                std::cout << "id author Suprateem" << std::endl;
                std::cout << "option name Threads type spin default 4 min 1 max 512" << std::endl;
                std::cout << "option name MultiPV type spin default 1 min 1 max 5" << std::endl;
                std::cout << "option name Ponder type check default false" << std::endl;
                std::cout << "uciok" << std::endl;
            } else if (line == "isready") {
                std::cout << "readyok" << std::endl;
            } else if (line.substr(0, 8) == "position") {
                current_position.parse_position(line);
            } else if (line.substr(0, 2) == "go") {
                Search::start_search(line);
            } else if (line.substr(0, 9) == "setoption") {
                set_option(line);
            } else if (line == "quit") {
                quit = true;
                Search::cv.notify_all();
                break;
            }
        }
    }
}
