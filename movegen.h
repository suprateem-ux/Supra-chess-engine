#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "bitboard.h"

enum Color { WHITE, BLACK };
enum Piece { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, EMPTY };
enum MoveType { NORMAL, PROMOTION, EN_PASSANT, CASTLING };

struct Move {
    uint16_t data;  // 6 bits from, 6 bits to, 2 bits type, 4 bits promotion piece

    Move() : data(0) {}
    Move(int from, int to, MoveType type = NORMAL, Piece promo = EMPTY) {
        data = (from & 0x3F) | ((to & 0x3F) << 6) | ((static_cast<uint16_t>(type) & 0x3) << 12) | ((static_cast<uint16_t>(promo) & 0xF) << 14);
    }

    int from() const { return data & 0x3F; }
    int to() const { return (data >> 6) & 0x3F; }
    MoveType type() const { return static_cast<MoveType>((data >> 12) & 0x3); }
    Piece promotion() const { return static_cast<Piece>((data >> 14) & 0xF); }

    std::string to_string() const {
        static const std::string squares[64] = {
            "a1","b1","c1","d1","e1","f1","g1","h1",
            "a2","b2","c2","d2","e2","f2","g2","h2",
            "a3","b3","c3","d3","e3","f3","g3","h3",
            "a4","b4","c4","d4","e4","f4","g4","h4",
            "a5","b5","c5","d5","e5","f5","g5","h5",
            "a6","b6","c6","d6","e6","f6","g6","h6",
            "a7","b7","c7","d7","e7","f7","g7","h7",
            "a8","b8","c8","d8","e8","f8","g8","h8"
        };
        std::string result = squares[from()] + squares[to()];
        if (type() == PROMOTION) {
            static const char promoChars[] = "nbrq";
            if (promotion() >= KNIGHT && promotion() <= QUEEN)
                result += promoChars[promotion() - KNIGHT];
        }
        return result;
    }

    bool operator==(const Move& other) const { return data == other.data; }
};

class Position {
public:
    Position() { reset(); }

    Bitboard pieces[2][6];  // [color][piece]
    Bitboard occupancy;
    Color stm;
    int castlingRights;     // bits for KQkq castling rights
    int epSquare;           // en passant square or -1
    int halfmoveClock;
    int fullmoveNumber;
    uint64_t hashKey;

    using MoveList = std::vector<Move>;

    MoveList generate_legal() const;
    MoveList generate_captures() const;
    bool is_legal(Move move) const;

    void make_move(Move move);
    void parse_position(const std::string& fen = "startpos");

    Color side_to_move() const { return stm; }
    Piece piece_at(int sq) const;
    uint64_t compute_hash() const;
    void reset();

private:
    void generate_pawn_moves(MoveList& moves, Color side) const;
    void generate_sliding_moves(MoveList& moves, Color side, Piece type) const;
    void generate_king_moves(MoveList& moves, Color side) const;
    bool is_square_attacked(int sq, Color byColor) const;
};

namespace MoveGen {
    int see(const Position& pos, Move move);

    void order_moves(const Position& pos, Position::MoveList& moves, Move ttMove, Move killers[2], int history[64][64]);
}
