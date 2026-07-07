#include "cli.hpp"
#include "config.hpp"

namespace cli {
    void show_no_input() {
        fprintf(stderr, "%s: no input file provided\n", config::TARGET);
        fprintf(stderr, "Try %s'%s --help'%s for more information.\n", 
                config::color::GREEN, config::TARGET, config::color::RESET);
    }

    void show_help() {
        printf("%s %s - %s\n\n", 
               config::color::BOLD, config::TARGET, config::DESCRIPTION);
        printf("%s", config::color::RESET);
        printf("Usage: %s [OPTIONS] [FILE]\n\n", config::TARGET);
        
        printf("Options:\n");
        printf("  %s-h%s, %s--help%s      Show this help message\n", 
               config::color::YELLOW, config::color::RESET, 
               config::color::YELLOW, config::color::RESET);
        printf("  %s-v%s, %s--version%s   Show version information\n", 
               config::color::YELLOW, config::color::RESET,
               config::color::YELLOW, config::color::RESET);
        
        printf("\nKeybindings:\n");
        printf("  %sCtrl-X%s              Quit editor\n", 
               config::color::YELLOW, config::color::RESET);
        printf("  %sCtrl-S%s              Save file\n", 
               config::color::YELLOW, config::color::RESET);
        printf("  %sCtrl-V%s              Paste from clipboard\n", 
               config::color::YELLOW, config::color::RESET);
        printf("  %sArrow Keys%s          Navigate\n", 
               config::color::YELLOW, config::color::RESET);
        printf("  %sTab%s                 Insert tab (4 spaces)\n", 
               config::color::YELLOW, config::color::RESET);
        printf("\nMouse Selection:\n");
        printf("  Click and drag to select text (terminal native)\n");
        printf("  Middle-click to paste from primary selection\n");
    }

    void show_version() {
        printf("%s %s\n", config::TARGET, config::VERSION);
    }

    void show_unknown(const std::string& arg) {
        fprintf(stderr, "%s: unknown option '%s'\n", config::TARGET, arg.c_str());
        fprintf(stderr, "Try %s'%s --help'%s for more information.\n", 
                config::color::GREEN, config::TARGET, config::color::RESET);
    }

    Options parse_arguments(int argc, char* argv[]) {
        Options opts;
        
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            
            if (arg == "-h" || arg == "--help") {
                opts.show_help = true;
                return opts;
            } else if (arg == "-v" || arg == "--version") {
                opts.show_version = true;
                return opts;
            } else if (arg[0] == '-') {
                show_unknown(arg);
                exit(1);
            } else if (opts.filename.empty()) {
                opts.filename = arg;
            } else {
                fprintf(stderr, "%s: too many arguments\n", config::TARGET);
                exit(1);
            }
        }
        
        if (opts.filename.empty()) {
            show_no_input();
            exit(1);
        }
        
        return opts;
    }
}
