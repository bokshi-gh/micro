#include "terminal.hpp"
#include "config.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

namespace terminal {
    static struct termios orig_termios;
    static bool raw_mode_enabled = false;

    std::pair<int, int> get_window_size() {
        struct winsize ws;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
            return {config::DEFAULT_ROWS, config::DEFAULT_COLS};
        }
        return {ws.ws_row, ws.ws_col};
    }

    int enable_raw_mode() {
        if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
            std::cerr << "Error: tcgetattr failed: " << strerror(errno) << std::endl;
            return -1;
        }
        
        struct termios raw = orig_termios;
        raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
        raw.c_oflag &= ~(OPOST);
        raw.c_cflag |= (CS8);
        raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 1;

        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
            std::cerr << "Error: tcsetattr failed: " << strerror(errno) << std::endl;
            return -1;
        }
        
        raw_mode_enabled = true;
        return 0;
    }

    int disable_raw_mode() {
        if (!raw_mode_enabled) return 0;
        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
            std::cerr << "Error: tcsetattr failed: " << strerror(errno) << std::endl;
            return -1;
        }
        raw_mode_enabled = false;
        return 0;
    }

    void switch_to_alternate_screen_buffer() {
        write(STDOUT_FILENO, "\x1b[?1049h", 8);
    }

    void return_to_main_screen_buffer() {
        write(STDOUT_FILENO, "\x1b[?1049l", 8);
    }

    void clear_screen() {
        write(STDOUT_FILENO, "\x1b[2J", 4);
    }

    void move_cursor_to_home() {
        write(STDOUT_FILENO, "\x1b[H", 3);
    }

    void hide_cursor() {
        write(STDOUT_FILENO, "\x1b[?25l", 6);
    }

    void show_cursor() {
        write(STDOUT_FILENO, "\x1b[?25h", 6);
    }

    void clear_line() {
        write(STDOUT_FILENO, "\x1b[K", 3);
    }

    TerminalGuard::TerminalGuard() {
        enable_raw_mode();
        switch_to_alternate_screen_buffer();
    }

    TerminalGuard::~TerminalGuard() {
        return_to_main_screen_buffer();
        disable_raw_mode();
    }
}
