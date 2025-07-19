#pragma once

#include "bitboard.h"
#include "square.h"
#include <string>
#include <vector>

enum MoveType {
    NORMAL,
    CAPTURE,
    PROMOTION,
    EN_PASSANT,
    CASTLING
};

// Move encoding:
// bits 0-5   = from square (6 bits)
// bits 6-11  = to square   (6 bits)
// bits 12-15 = promotion   (4 bits)
// bits 16-19 = move type   (4 bits)

class Move {
    int data;
public:
    Move() : data(0) {}

    Move(int from, int to, int promo = 0, MoveType type = NORMAL) {
        data = from | (to << 6) | (promo << 12) | (type << 16);
    }

    int from() const {
        return data & 0x3F;
    }

    int to() const {
        return (data >> 6) & 0x3F;
    }

    int promotion() const {
        return (data >> 12) & 0xF;
    }

    MoveType type() const {
        return static_cast<MoveType>((data >> 16) & 0xF);
    }

    std::string to_string() const {
        std::string moveStr;
        moveStr += SquareNames[from()];
        moveStr += SquareNames[to()];
        if (promotion()) {
            // Promotion: 1=n, 2=b, 3=r, 4=q
            static const char promChar[] = {' ', 'n', 'b', 'r', 'q'};
            moveStr += promChar[promotion()];
        }
        return moveStr;
    }

    bool operator==(const Move& other) const {
        return data == other.data;
    }

    bool operator!=(const Move& other) const {
        return !(*this == other);
    }

    int raw() const {
        return data;
    }
};
