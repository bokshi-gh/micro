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
#include <sys/stat.h>
#include <cstdlib>

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

    // Calculate visual column (character position on screen)
    int Editor::get_visual_column(int row, int col) const {
        if (row < 0 || static_cast<size_t>(row) >= rows.size()) return 0;
        
        const std::string& text = rows[row].chars;
        int visual_col = 0;
        
        // Count visual columns up to 'col' character index
        for (int i = 0; i < col && i < static_cast<int>(text.length()); i++) {
            if (text[i] == '\t') {
                // Tab expands to next tab stop
                visual_col += config::TAB_STOP - (visual_col % config::TAB_STOP);
            } else {
                visual_col++;
            }
        }
        return visual_col;
    }

    // Get character index from visual column (for mouse click positioning)
    int Editor::get_char_index_from_visual(int row, int visual_col) const {
        if (row < 0 || static_cast<size_t>(row) >= rows.size()) return 0;
        
        const std::string& text = rows[row].chars;
        int current_visual = 0;
        
        for (int i = 0; i < static_cast<int>(text.length()); i++) {
            if (current_visual >= visual_col) {
                return i;
            }
            if (text[i] == '\t') {
                current_visual += config::TAB_STOP - (current_visual % config::TAB_STOP);
            } else {
                current_visual++;
            }
        }
        return text.length();
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

    size_t Editor::get_file_size() const {
        size_t size = 0;
        for (const auto& row : rows) {
            size += row.chars.length() + 1;
        }
        return size > 0 ? size - 1 : 0;
    }

    void Editor::open_file(const std::string& filename) {
        this->filename = filename;
        rows.clear();
        
        std::ifstream file(filename);
        if (!file.is_open()) {
            status_message = "New file";
            return;
        }
        
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            insert_row(rows.size(), line);
        }
        
        file.close();
        dirty = false;
        cursor_row = 0;
        cursor_col = 0;
        scroll_row = 0;
        scroll_col = 0;
        
        std::string size_str;
        if (get_file_size() < 1024) {
            size_str = std::to_string(get_file_size()) + "B";
        } else if (get_file_size() < 1024 * 1024) {
            size_str = std::to_string(get_file_size() / 1024) + "KB";
        } else {
            size_str = std::to_string(get_file_size() / (1024 * 1024)) + "MB";
        }
        status_message = "Opened " + filename + " (" + size_str + ")";
    }

    void Editor::save_file() {
        if (filename.empty()) {
            status_message = "No filename specified";
            return;
        }
        
        std::ofstream file(filename);
        if (!file.is_open()) {
            status_message = "Failed to save: " + filename;
            return;
        }
        
        for (const auto& row : rows) {
            file << row.chars << '\n';
        }
        
        file.close();
        dirty = false;
        status_message = "Saved " + filename + " (" + std::to_string(get_file_size()) + "B)";
    }

    void Editor::insert_char(char c) {
        if (rows.empty()) {
            insert_row(0, "");
        }
        
        auto& row = get_current_row();
        row.chars.insert(cursor_col, 1, c);
        cursor_col++;
        dirty = true;
        scroll_cursor();
    }

    void Editor::insert_tab() {
        if (rows.empty()) {
            insert_row(0, "");
        }
        
        auto& row = get_current_row();
        row.chars.insert(cursor_col, 1, '\t');
        cursor_col++;
        dirty = true;
        scroll_cursor();
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
        scroll_cursor();
    }

    void Editor::delete_char() {
        if (rows.empty()) return;
        
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
        if (rows.empty()) return;
        
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
        scroll_cursor();
    }

    void Editor::move_cursor_up() {
        if (cursor_row > 0) {
            int visual_col = get_visual_column(cursor_row, cursor_col);
            cursor_row--;
            // Try to keep the same visual column
            int new_col = get_char_index_from_visual(cursor_row, visual_col);
            cursor_col = std::min(new_col, get_row_length(cursor_row));
            scroll_cursor();
        }
    }

    void Editor::move_cursor_down() {
        if (static_cast<size_t>(cursor_row) < rows.size() - 1) {
            int visual_col = get_visual_column(cursor_row, cursor_col);
            cursor_row++;
            int new_col = get_char_index_from_visual(cursor_row, visual_col);
            cursor_col = std::min(new_col, get_row_length(cursor_row));
            scroll_cursor();
        }
    }

    void Editor::move_cursor_left() {
        if (cursor_col > 0) {
            cursor_col--;
        } else if (cursor_row > 0) {
            cursor_row--;
            cursor_col = get_row_length(cursor_row);
        }
        scroll_cursor();
    }

    void Editor::move_cursor_right() {
        int len = get_row_length(cursor_row);
        if (cursor_col < len) {
            cursor_col++;
        } else if (static_cast<size_t>(cursor_row) < rows.size() - 1) {
            cursor_row++;
            cursor_col = 0;
        }
        scroll_cursor();
    }

    void Editor::move_cursor_to(int row, int col) {
        cursor_row = std::clamp(row, 0, static_cast<int>(rows.size()) - 1);
        cursor_col = std::clamp(col, 0, get_row_length(cursor_row));
        scroll_cursor();
    }

    void Editor::clamp_cursor() {
        int len = get_row_length(cursor_row);
        if (cursor_col > len) {
            cursor_col = len;
        }
    }

    void Editor::scroll_cursor() {
        auto [term_rows, term_cols] = terminal::get_window_size();
        int line_num_width = 4;
        int usable_cols = term_cols - line_num_width - 1;
        
        // Get visual column for scrolling
        int visual_col = get_visual_column(cursor_row, cursor_col);
        
        if (cursor_row < scroll_row) {
            scroll_row = cursor_row;
        }
        if (cursor_row >= scroll_row + term_rows - 2) {
            scroll_row = cursor_row - term_rows + 3;
        }
        
        if (visual_col < scroll_col) {
            scroll_col = visual_col;
        }
        if (visual_col >= scroll_col + usable_cols) {
            scroll_col = visual_col - usable_cols + 1;
        }
        
        if (scroll_row < 0) scroll_row = 0;
        if (scroll_col < 0) scroll_col = 0;
    }

    void Editor::draw_row(const Row& row, int row_num, int term_cols) {
        int line_num_width = 4;
        int usable_cols = term_cols - line_num_width - 1;
        
        // Show line number
        std::string line_num = std::to_string(row_num + 1);
        std::string padding(line_num_width - line_num.length(), ' ');
        std::string line_display = "\x1b[2m" + padding + line_num + " \x1b[0m";
        write(STDOUT_FILENO, line_display.c_str(), line_display.length());
        
        // Display text with horizontal scrolling
        const std::string& text = row.chars;
        int len = text.length();
        
        // Find character index for scroll position
        int char_index = 0;
        int visual_pos = 0;
        
        while (char_index < len && visual_pos < scroll_col) {
            if (text[char_index] == '\t') {
                visual_pos += config::TAB_STOP - (visual_pos % config::TAB_STOP);
            } else {
                visual_pos++;
            }
            char_index++;
        }
        
        // Render characters with tab expansion
        int rendered = 0;
        while (char_index < len && rendered < usable_cols) {
            if (text[char_index] == '\t') {
                int spaces = config::TAB_STOP - (rendered % config::TAB_STOP);
                std::string tab_str(spaces, ' ');
                write(STDOUT_FILENO, tab_str.c_str(), spaces);
                rendered += spaces;
            } else {
                write(STDOUT_FILENO, &text[char_index], 1);
                rendered++;
            }
            char_index++;
        }
    }

    void Editor::render_rows(int term_rows, int term_cols) {
        int max_rows = std::min(term_rows - 2, static_cast<int>(rows.size()) - scroll_row);
        
        for (int i = 0; i < max_rows; i++) {
            terminal::clear_line();
            int actual_row = i + scroll_row;
            draw_row(rows[actual_row], actual_row, term_cols);
            
            if (i < max_rows - 1 || rows.size() > static_cast<size_t>(scroll_row + max_rows)) {
                write(STDOUT_FILENO, "\r\n", 2);
            }
        }
        
        for (int i = max_rows; i < term_rows - 2; i++) {
            terminal::clear_line();
            if (i < term_rows - 3) {
                write(STDOUT_FILENO, "\r\n", 2);
            }
        }
    }

    void Editor::move_cursor() {
        int line_num_width = 4;
        int screen_row = cursor_row - scroll_row;
        
        // Use visual column for cursor positioning
        int visual_col = get_visual_column(cursor_row, cursor_col);
        int screen_col = visual_col - scroll_col + line_num_width + 1;
        
        if (screen_row < 0) screen_row = 0;
        if (screen_col < 0) screen_col = 0;
        
        char buf[32];
        snprintf(buf, sizeof(buf), "\x1b[%d;%dH", screen_row + 1, screen_col + 1);
        write(STDOUT_FILENO, buf, strlen(buf));
    }

    void Editor::show_status_bar(int term_rows, int term_cols) {
        // Status message line (second from bottom)
        terminal::move_cursor_to(term_rows - 2, 0);
        terminal::clear_line();
        write(STDOUT_FILENO, "\x1b[0m", 4);
        
        if (!status_message.empty()) {
            std::string msg = status_message;
            if (msg.length() > static_cast<size_t>(term_cols)) {
                msg = msg.substr(0, term_cols - 3) + "...";
            }
            write(STDOUT_FILENO, msg.c_str(), msg.length());
        }
        
        // Status bar (bottom line)
        terminal::move_cursor_to(term_rows - 1, 0);
        terminal::clear_line();
        write(STDOUT_FILENO, "\x1b[0m", 4);
        
        std::string left_status = filename;
        if (left_status.empty()) left_status = "[No Name]";
        if (dirty) left_status += " [modified]";
        
        std::string size_str;
        size_t file_size = get_file_size();
        if (file_size < 1024) {
            size_str = std::to_string(file_size) + "B";
        } else if (file_size < 1024 * 1024) {
            size_str = std::to_string(file_size / 1024) + "KB";
        } else {
            size_str = std::to_string(file_size / (1024 * 1024)) + "MB";
        }
        
        // Calculate visual column for display
        int visual_col = get_visual_column(cursor_row, cursor_col);
        
        // Show: Ln X, Col Y (Y is the visual column, 1-based)
        std::string right_status = size_str + " | Ln " + std::to_string(cursor_row + 1) + 
                                   ", Col " + std::to_string(visual_col + 1);
        
        int left_width = left_status.length();
        int right_width = right_status.length();
        int padding = term_cols - left_width - right_width - 2;
        if (padding < 1) padding = 1;
        
        std::string status = "\x1b[7m" + left_status + std::string(padding, ' ') + right_status + "\x1b[0m";
        
        if (status.length() > static_cast<size_t>(term_cols)) {
            status = status.substr(0, term_cols);
        }
        
        write(STDOUT_FILENO, status.c_str(), status.length());
        
        if (status.length() < static_cast<size_t>(term_cols)) {
            std::string remaining(term_cols - status.length(), ' ');
            write(STDOUT_FILENO, remaining.c_str(), remaining.length());
        }
        
        write(STDOUT_FILENO, "\x1b[0m", 4);
    }

    void Editor::show_status_message(const std::string& message) {
        status_message = message;
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
                default: return static_cast<Key>(c);
            }
        }
        
        return static_cast<Key>(c);
    }

    bool Editor::confirm_quit() {
        status_message = "Save changes? (y)es / (n)o / (c)ancel";
        
        while (true) {
            refresh_screen();
            
            Key key = read_key();
            
            if (key == Key::NONE) continue;
            
            char c = static_cast<char>(key);
            if (c == 'y' || c == 'Y') {
                save_file();
                return true;
            } else if (c == 'n' || c == 'N') {
                dirty = false;
                status_message = "Quit without saving";
                return true;
            } else if (c == 'c' || c == 'C' || key == Key::ESC) {
                status_message = "Quit cancelled";
                return false;
            }
        }
    }

    void Editor::refresh_screen() {
        terminal::hide_cursor();
        terminal::clear_screen();
        terminal::move_cursor_to_home();
        write(STDOUT_FILENO, "\x1b[0m", 4);
        
        auto [term_rows, term_cols] = terminal::get_window_size();
        render_rows(term_rows, term_cols);
        show_status_bar(term_rows, term_cols);
        move_cursor();
        terminal::show_cursor();
        write(STDOUT_FILENO, "\x1b[0m", 4);
    }

    void Editor::process_keypress() {
        Key key = read_key();
        
        switch (key) {
            case Key::CTRL_X:
                if (dirty && !waiting_for_quit) {
                    waiting_for_quit = true;
                    if (confirm_quit()) {
                        running = false;
                    }
                    waiting_for_quit = false;
                } else {
                    running = false;
                }
                break;
                
            case Key::CTRL_S:
                save_file();
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
                
            case Key::ESC:
                status_message = "";
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
        scroll_cursor();
    }

    void Editor::shutdown() {
        running = false;
    }

    void Editor::run(const std::string& filename) {
        open_file(filename);
        running = true;
        waiting_for_quit = false;
        
        while (running) {
            refresh_screen();
            process_keypress();
        }
    }
}
