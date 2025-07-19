#pragma once

// Forward declaration to avoid circular dependency
class Position;

namespace Eval {
    void init_eval();
    int eval(const Position& pos);
}
