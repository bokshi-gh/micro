#include "terminal.h"

static struct termios orig_termios;

int get_window_size(int *rows, int *cols) {
  struct winsize ws;
  
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    return -1;
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

void enable_raw_mode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
  
  struct termios raw = orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disable_raw_mode() { if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) die("tcsetattr"); } 

void switch_to_alternate_screen_buffer() { write(STDOUT_FILENO, "\x1b[?1049h", 8); }
void return_to_main_screen_buffer() { write(STDOUT_FILENO, "\x1b[?1049l", 8); }

void cleanup_terminal() {
  return_to_main_screen_buffer();
  disable_raw_mode();
}

void clear_entire_screen() { write(STDOUT_FILENO, "\x1b[2J", 4); }
void move_cursor_to_home() { write(STDOUT_FILENO, "\x1b[H", 3); }
