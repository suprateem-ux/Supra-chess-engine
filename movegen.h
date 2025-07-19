#pragma once
#include <string>

struct Move {
    int from() const { return 0; }
    int to() const { return 0; }

    std::string to_string() const {
        return "e2e4"; // Stub
    }

    bool operator!=(const Move& other) const {
        return to() != other.to(); // Basic example
    }
};
