#pragma once
#include <string>
#include <chrono>

namespace TimeManager {
    extern int moveTime;
    extern int increment;
    extern int timeLeft;
    extern int movesToGo;
    extern std::chrono::steady_clock::time_point startTime;

    void set_time_control(const std::string& cmd, bool isWhite);
    void start_timer();
    int time_remaining();
    bool time_up();
}
