#pragma once

#include <string>

namespace config {
    constexpr const char* TARGET = "micro";
    constexpr const char* VERSION = "1.0.0";
    constexpr const char* DESCRIPTION = "A minimal terminal text editor";
    
    // ANSI color codes
    namespace color {
        constexpr const char* RESET   = "\x1b[0m";
        constexpr const char* GREEN   = "\x1b[32m";
        constexpr const char* YELLOW  = "\x1b[33m";
        constexpr const char* RED     = "\x1b[31m";
        constexpr const char* BLUE    = "\x1b[34m";
        constexpr const char* CYAN    = "\x1b[36m";
        constexpr const char* BOLD    = "\x1b[1m";
        constexpr const char* DIM     = "\x1b[2m";
    }
    
    // Editor defaults
    constexpr int TAB_STOP = 4;
    constexpr int DEFAULT_ROWS = 24;
    constexpr int DEFAULT_COLS = 80;
}
