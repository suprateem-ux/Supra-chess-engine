#pragma once
#include <string>

namespace Syzygy {
    void init(const std::string& path);
    int probe_wdl(const struct Position& pos);
}
