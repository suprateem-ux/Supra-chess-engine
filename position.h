#pragma once
#include <string>

enum Color { WHITE, BLACK };

struct Position {
    static void parse_position(const std::string& line) {
        // Stub: implement FEN parsing etc.
    }

    Color side_to_move() const {
        return WHITE; // Stub
    }
};
