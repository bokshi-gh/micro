#pragma once

#include <string>
#include <vector>
#include <memory>
#include "terminal.hpp"

namespace editor {
    enum class Key {
        NONE = 0,
        ENTER = 13,
        BACKSPACE = 127,
        DELETE = 1000,
        ARROW_UP = 1001,
        ARROW_DOWN = 1002,
        ARROW_LEFT = 1003,
        ARROW_RIGHT = 1004,
        TAB = 9,
        ESC = 27,
        CTRL_X = 24,   // Ctrl+X - Quit
        CTRL_S = 19,   // Ctrl+S - Save
        CTRL_V = 22,   // Ctrl+V - Paste
        CTRL_C = 3,    // Ctrl+C - Copy
    };

    class Editor {
    private:
        struct Row {
            std::string chars;
        };
        
        std::string filename;
        std::vector<Row> rows;
        int cursor_row = 0;
        int cursor_col = 0;
        int scroll_row = 0;
        int scroll_col = 0;
        bool dirty = false;
        bool running = true;
        bool waiting_for_quit = false;
        std::string status_message;
        std::string clipboard;
        std::unique_ptr<terminal::TerminalGuard> terminal_guard;

        // Row operations
        Row& get_current_row();
        const Row& get_current_row() const;
        int get_row_length(int row) const;
        void insert_row(int at, const std::string& content);
        void delete_row(int at);
        
        // Editing operations
        void insert_char(char c);
        void insert_tab();
        void split_row_at_cursor();
        void delete_char();
        void backspace();
        void paste_clipboard();
        void copy_selection();
        void get_system_clipboard();
        void set_system_clipboard(const std::string& text);
        
        // Navigation
        void move_cursor_up();
        void move_cursor_down();
        void move_cursor_left();
        void move_cursor_right();
        void move_cursor_to(int row, int col);
        void clamp_cursor();
        void scroll_cursor();
        
        // File operations
        void save_file();
        void open_file(const std::string& filename);
        size_t get_file_size() const;
        bool confirm_quit();
        
        // Rendering
        void render_rows(int term_rows, int term_cols);
        void move_cursor();
        void show_status_bar(int term_rows, int term_cols);
        void show_status_message(const std::string& message);
        void draw_row(const Row& row, int row_num, int term_cols);
        void clear_line();
        
        // Input
        Key read_key();
        
    public:
        Editor();
        ~Editor() = default;
        
        Editor(const Editor&) = delete;
        Editor& operator=(const Editor&) = delete;
        Editor(Editor&&) = delete;
        Editor& operator=(Editor&&) = delete;
        
        void run(const std::string& filename);
        void process_keypress();
        void refresh_screen();
        void shutdown();
    };
}
