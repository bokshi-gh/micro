#pragma once

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>
#include "config.hpp"

namespace cli {
    struct Options {
        bool show_help = false;
        bool show_version = false;
        std::string filename;
    };
    
    void show_no_input();
    void show_help();
    void show_version();
    void show_unknown(const std::string& arg);
    Options parse_arguments(int argc, char* argv[]);
}
