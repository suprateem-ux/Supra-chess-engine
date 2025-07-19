#pragma once
#include "bitboard.h"
#include "square.h"
#include <vector>
#include <string>
#include "position.h"

enum MoveType { NORMAL, CAPTURE, PROMOTION, EN_PASSANT, CASTLING };

class Move {
    int data;
public:
    Move() : data(0) {}
    Move(int from, int to, int promo = 0, MoveType type = NORMAL);
    int from() const;
    int to() const;
    int promotion() const;
    MoveType type() const;
    std::string to_string() const;
    bool operator==(const Move& other) const { return data == other.data; }
    bool operator!=(const Move& other) const { return !(*this == other); }
};


