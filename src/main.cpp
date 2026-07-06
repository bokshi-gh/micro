#include "editor.hpp"
#include "cli.hpp"

int main(int argc, char* argv[]) {
    auto options = cli::parse_arguments(argc, argv);
    
    if (options.show_help) {
        cli::show_help();
        return 0;
    }
    
    if (options.show_version) {
        cli::show_version();
        return 0;
    }
    
    editor::Editor editor;
    editor.run(options.filename);
    
    return 0;
}
