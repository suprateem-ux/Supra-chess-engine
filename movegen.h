#pragma once
#include <vector>
#include <string>

enum Color { WHITE, BLACK };
enum Piece { NONE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };

struct Move {
    int from_sq, to_sq;
    Piece promotion = NONE;
    bool is_capture = false;

    Move(int from = 0, int to = 0, Piece promo = NONE, bool cap = false)
        : from_sq(from), to_sq(to), promotion(promo), is_capture(cap) {}

    std::string to_string() const;
    int from() const { return from_sq; }
    int to() const { return to_sq; }
};
