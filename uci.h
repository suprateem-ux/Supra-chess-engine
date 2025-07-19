#pragma once
#include <string>

namespace UCI {
    void set_option(const std::string& cmd);
    void uci_loop();  // <-- Declare this
}
