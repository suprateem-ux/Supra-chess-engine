#include "time.h"
#include <sstream>
#include <string>
#include <algorithm>

namespace TimeManager {
    int moveTime = 0;
    int increment = 0;
    int timeLeft = 0;
    int movesToGo = 30;
    std::chrono::steady_clock::time_point startTime;

    void set_time_control(const std::string& cmd, bool isWhite) {
        std::istringstream iss(cmd);
        std::string token;
        while (iss >> token) {
            if (token == (isWhite ? "wtime" : "btime")) iss >> timeLeft;
            else if (token == (isWhite ? "winc" : "binc")) iss >> increment;
            else if (token == "movestogo") iss >> movesToGo;
            else if (token == "movetime") iss >> moveTime;
        }
        if (movesToGo <= 0) movesToGo = 30;
    }

    void start_timer() {
        startTime = std::chrono::steady_clock::now();
    }

    int time_remaining() {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
        if (moveTime > 0) return moveTime - (int)elapsed;
        int allocated = timeLeft / std::max(movesToGo, 1) + increment / 2;
        return allocated - (int)elapsed;
    }

    bool time_up() {
        return time_remaining() <= 0;
    }
}
