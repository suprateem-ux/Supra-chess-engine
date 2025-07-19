#pragma once
#include <string>
#include <vector>

enum Color { WHITE, BLACK };
enum PieceType { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };

struct Move {
    int from_sq, to_sq;
    int promotion = 0;
    bool is_capture = false;

    std::string to_string() const;
    int from() const { return from_sq; }
    int to() const { return to_sq; }
};

namespace Position {
    void parse_position(const std::string& cmd);
    Color side_to_move();
    std::vector<Move> generate_legal_moves();
}
