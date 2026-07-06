#pragma once

#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <utility>

namespace terminal {
    int enable_raw_mode();
    int disable_raw_mode();
    void switch_to_alternate_screen_buffer();
    void return_to_main_screen_buffer();
    void clear_screen();
    void move_cursor_to_home();
    void move_cursor_to(int row, int col);
    void hide_cursor();
    void show_cursor();
    void clear_line();
    std::pair<int, int> get_window_size();
    
    // RAII wrapper for terminal mode
    class TerminalGuard {
    public:
        TerminalGuard();
        ~TerminalGuard();
        TerminalGuard(const TerminalGuard&) = delete;
        TerminalGuard& operator=(const TerminalGuard&) = delete;
        TerminalGuard(TerminalGuard&&) = delete;
        TerminalGuard& operator=(TerminalGuard&&) = delete;
    };
}
