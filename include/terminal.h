#ifndef TERMINAL_H
#define TERMINAL_H

#define ANSI_RED "\033[31m"
#define ANSI_GREEN "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_RESET "\033[0m"

#include "errors.h"

#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

typedef struct {
  struct termios orig_termios;

  int rows;
  int cols;

  int cy;
  int cx;
} Terminal;

void init_terminal();

void enable_raw_mode();
void disable_raw_mode();

void switch_to_alternate_screen_buffer();
void return_to_main_screen_buffer();

int get_window_size(int *rows, int *cols);

void clear_entire_screen();

void move_cursor_to_home();

void refresh_screen();

#endif
