#pragma once
#include <string>
#include "movegen.h"

namespace Polyglot {
    void load(const std::string& file);
    Move get_book_move(const struct Position& pos);
}
