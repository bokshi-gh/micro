#include "editor.hpp"
#include "config.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <sstream>
#include <iomanip>

namespace editor {
    Editor::Editor() {
        terminal_guard = std::make_unique<terminal::TerminalGuard>();
    }

    auto Editor::get_current_row() -> Row& {
        return rows[cursor_row];
    }

    auto Editor::get_current_row() const -> const Row& {
        return rows[cursor_row];
    }

    int Editor::get_row_length(int row) const {
        if (row < 0 || static_cast<size_t>(row) >= rows.size()) return 0;
        return rows[row].chars.length();
    }

    void Editor::insert_row(int at, const std::string& content) {
        Row new_row;
        new_row.chars = content;
        rows.insert(rows.begin() + at, std::move(new_row));
        dirty = true;
    }

    void Editor::delete_row(int at) {
        if (at < 0 || static_cast<size_t>(at) >= rows.size()) return;
        rows.erase(rows.begin() + at);
        dirty = true;
    }

    void Editor::open_file(const std::string& filename) {
        this->filename = filename;
        rows.clear();
        
        std::ifstream file(filename);
        if (!file.is_open()) return;
        
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            insert_row(rows.size(), line);
        }
        
        file.close();
        dirty = false;
    }

    void Editor::save_file() {
        if (filename.empty()) {
            show_status_message("No filename specified");
            return;
        }
        
        std::ofstream file(filename);
        if (!file.is_open()) {
            show_status_message("Failed to save file: " + filename);
            return;
        }
        
        for (const auto& row : rows) {
            file << row.chars << '\n';
        }
        
        file.close();
        dirty = false;
        show_status_message("File saved: " + filename);
    }

    bool Editor::confirm_unsaved_changes() {
        return true;
    }

    void Editor::insert_char(char c) {
        if (rows.empty()) {
            insert_row(0, "");
        }
        
        auto& row = get_current_row();
        row.chars.insert(cursor_col, 1, c);
        cursor_col++;
        dirty = true;
    }

    void Editor::insert_tab() {
        int spaces = config::TAB_STOP - (cursor_col % config::TAB_STOP);
        for (int i = 0; i < spaces; i++) {
            insert_char(' ');
        }
    }

    void Editor::split_row_at_cursor() {
        if (rows.empty()) {
            insert_row(0, "");
            return;
        }
        
        auto& current = get_current_row();
        std::string new_content = current.chars.substr(cursor_col);
        current.chars = current.chars.substr(0, cursor_col);
        
        insert_row(cursor_row + 1, new_content);
        cursor_row++;
        cursor_col = 0;
        dirty = true;
    }

    void Editor::delete_char() {
        auto& current = get_current_row();
        if (static_cast<size_t>(cursor_col) < current.chars.length()) {
            current.chars.erase(cursor_col, 1);
            dirty = true;
        } else if (static_cast<size_t>(cursor_row) < rows.size() - 1) {
            auto& next_row = rows[cursor_row + 1];
            current.chars += next_row.chars;
            delete_row(cursor_row + 1);
            dirty = true;
        }
    }

    void Editor::backspace() {
        if (cursor_col > 0) {
            auto& current = get_current_row();
            current.chars.erase(cursor_col - 1, 1);
            cursor_col--;
            dirty = true;
        } else if (cursor_row > 0) {
            cursor_row--;
            cursor_col = get_row_length(cursor_row);
            auto& current = get_current_row();
            auto& next_row = rows[cursor_row + 1];
            current.chars += next_row.chars;
            delete_row(cursor_row + 1);
            dirty = true;
        }
    }

    void Editor::paste_clipboard() {
        if (clipboard.empty()) {
            show_status_message("Clipboard is empty");
            return;
        }
        
        for (char c : clipboard) {
            if (c == '\n') {
                split_row_at_cursor();
            } else {
                insert_char(c);
            }
        }
        show_status_message("Pasted from clipboard");
    }

    void Editor::copy_selection() {
        if (!rows.empty()) {
            clipboard = get_current_row().chars;
            show_status_message("Copied line to clipboard");
        }
    }

    void Editor::move_cursor_up() {
        if (cursor_row > 0) {
            cursor_row--;
            clamp_cursor();
        }
    }

    void Editor::move_cursor_down() {
        if (static_cast<size_t>(cursor_row) < rows.size() - 1) {
            cursor_row++;
            clamp_cursor();
        }
    }

    void Editor::move_cursor_left() {
        if (cursor_col > 0) {
            cursor_col--;
        } else if (cursor_row > 0) {
            cursor_row--;
            cursor_col = get_row_length(cursor_row);
        }
    }

    void Editor::move_cursor_right() {
        int len = get_row_length(cursor_row);
        if (cursor_col < len) {
            cursor_col++;
        } else if (static_cast<size_t>(cursor_row) < rows.size() - 1) {
            cursor_row++;
            cursor_col = 0;
        }
    }

    void Editor::move_cursor_to(int row, int col) {
        cursor_row = std::clamp(row, 0, static_cast<int>(rows.size()) - 1);
        cursor_col = std::clamp(col, 0, get_row_length(cursor_row));
    }

    void Editor::clamp_cursor() {
        int len = get_row_length(cursor_row);
        if (cursor_col > len) {
            cursor_col = len;
        }
    }

    Key Editor::read_key() {
        char c;
        int nread;
        
        while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
            if (nread == -1 && errno != EAGAIN) {
                return Key::NONE;
            }
        }
        
        if (c == '\x1b') {
            char seq[3];
            if (read(STDIN_FILENO, &seq[0], 1) != 1) return Key::ESC;
            if (read(STDIN_FILENO, &seq[1], 1) != 1) return Key::ESC;
            
            if (seq[0] == '[') {
                if (seq[1] >= '0' && seq[1] <= '9') {
                    if (read(STDIN_FILENO, &seq[2], 1) != 1) return Key::ESC;
                    if (seq[2] == '~' && seq[1] == '3') return Key::DELETE;
                } else {
                    switch (seq[1]) {
                        case 'A': return Key::ARROW_UP;
                        case 'B': return Key::ARROW_DOWN;
                        case 'C': return Key::ARROW_RIGHT;
                        case 'D': return Key::ARROW_LEFT;
                    }
                }
            }
            return Key::ESC;
        }
        
        if (c >= 0 && c < 32) {
            switch (c) {
                case 24: return Key::CTRL_X;
                case 19: return Key::CTRL_S;
                case 15: return Key::CTRL_O;
                case 22: return Key::CTRL_V;
                default: return static_cast<Key>(c);
            }
        }
        
        return static_cast<Key>(c);
    }

    void Editor::render_rows(int term_rows, int term_cols) {
        int max_rows = std::min(term_rows, static_cast<int>(rows.size()));
        
        int max_line_num = rows.size();
        int line_num_width = std::to_string(max_line_num).length();
        if (line_num_width < 3) line_num_width = 3; // Minimum width
        
        for (int i = 0; i < max_rows; i++) {
            terminal::clear_line();
            
            // Show line numbers (dim/gray)
            std::string line_num = std::to_string(i + 1);
            std::string padding(line_num_width - line_num.length(), ' ');
            std::string line_display = "\x1b[2m" + padding + line_num + " \x1b[0m";
            write(STDOUT_FILENO, line_display.c_str(), line_display.length());
            
            const auto& row = rows[i];
            
            // Truncate line if too long
            int display_len = std::min(static_cast<int>(row.chars.length()), term_cols - line_num_width - 2);
            if (display_len > 0) {
                write(STDOUT_FILENO, row.chars.c_str(), display_len);
            }
            
            if (i < max_rows - 1 || rows.size() > static_cast<size_t>(term_rows)) {
                write(STDOUT_FILENO, "\r\n", 2);
            }
        }
        
        for (int i = rows.size(); i < term_rows; i++) {
            terminal::clear_line();
            if (i < term_rows - 1) {
                write(STDOUT_FILENO, "\r\n", 2);
            }
        }
    }

    void Editor::move_cursor() {
        int max_line_num = rows.size();
        int line_num_width = std::to_string(max_line_num).length();
        if (line_num_width < 3) line_num_width = 3;
        int total_width = line_num_width + 1;
        
        char buf[32];
        snprintf(buf, sizeof(buf), "\x1b[%d;%dH", cursor_row + 1, cursor_col + total_width);
        write(STDOUT_FILENO, buf, strlen(buf));
    }

    void Editor::show_status_bar(int term_cols) {
        // Reset colors first to ensure clean status bar
        write(STDOUT_FILENO, "\x1b[0m", 4);
        terminal::clear_line();
        
        std::string left_status = filename;
        if (left_status.empty()) left_status = "[No Name]";
        if (dirty) left_status += " [modified]";
        
        std::string right_status = "Ln " + std::to_string(cursor_row + 1) + 
                                   ", Col " + std::to_string(cursor_col + 1);
        
        int left_width = left_status.length();
        int right_width = right_status.length();
        int padding = term_cols - left_width - right_width - 2; // -2 for safety
        if (padding < 1) padding = 1;
        
        // Build status bar with inverted colors
        std::string status = "\x1b[7m" + left_status + std::string(padding, ' ') + right_status + "\x1b[0m";
        
        if (status.length() > static_cast<size_t>(term_cols)) {
            status = status.substr(0, term_cols);
        }
        
        write(STDOUT_FILENO, status.c_str(), status.length());
        
        if (status.length() < static_cast<size_t>(term_cols)) {
            std::string remaining(term_cols - status.length(), ' ');
            write(STDOUT_FILENO, remaining.c_str(), remaining.length());
        }
        
        // Ensure colors are reset after status bar
        write(STDOUT_FILENO, "\x1b[0m", 4);
    }

    void Editor::show_status_message(const std::string& message) {
        terminal::clear_line();
        write(STDOUT_FILENO, message.c_str(), message.length());
        write(STDOUT_FILENO, "\x1b[0m", 4); // Reset colors
    }

    void Editor::refresh_screen() {
        terminal::hide_cursor();
        terminal::clear_screen();
        terminal::move_cursor_to_home();
        
        // Reset colors at start
        write(STDOUT_FILENO, "\x1b[0m", 4);
        
        auto [term_rows, term_cols] = terminal::get_window_size();
        render_rows(term_rows - 1, term_cols);
        
        terminal::move_cursor_to(term_rows - 1, 0);
        show_status_bar(term_cols);
        
        move_cursor();
        terminal::show_cursor();
        
        // Final color reset
        write(STDOUT_FILENO, "\x1b[0m", 4);
    }

    void Editor::process_keypress() {
        Key key = read_key();
        
        switch (key) {
            case Key::CTRL_X:
                shutdown();
                break;
                
            case Key::CTRL_S:
                save_file();
                break;
                
            case Key::CTRL_O:
                show_status_message("Open file: Not implemented yet");
                break;
                
            case Key::CTRL_V:
                paste_clipboard();
                break;
                
            case Key::ENTER:
                split_row_at_cursor();
                break;
                
            case Key::BACKSPACE:
                backspace();
                break;
                
            case Key::DELETE:
                delete_char();
                break;
                
            case Key::TAB:
                insert_tab();
                break;
                
            case Key::ARROW_UP:
                move_cursor_up();
                break;
                
            case Key::ARROW_DOWN:
                move_cursor_down();
                break;
                
            case Key::ARROW_LEFT:
                move_cursor_left();
                break;
                
            case Key::ARROW_RIGHT:
                move_cursor_right();
                break;
                
            default:
                if (static_cast<int>(key) >= 32 && static_cast<int>(key) <= 126) {
                    insert_char(static_cast<char>(key));
                }
                break;
        }
        
        if (rows.empty()) {
            cursor_row = 0;
            cursor_col = 0;
        } else {
            clamp_cursor();
            if (static_cast<size_t>(cursor_row) >= rows.size()) {
                cursor_row = rows.size() - 1;
                clamp_cursor();
            }
        }
    }

    void Editor::shutdown() {
        if (dirty) {
            if (confirm_unsaved_changes()) {
                save_file();
                running = false;
            }
        } else {
            running = false;
        }
    }

    void Editor::run(const std::string& filename) {
        open_file(filename);
        running = true;
        
        while (running) {
            refresh_screen();
            process_keypress();
        }
    }
}
