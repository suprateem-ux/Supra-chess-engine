#pragma once
#include <string>
#include <cstdint>
#include "movegen.h"

struct Position {
    static void parse_position(const std::string& command);
    Color side_to_move() const;
};
