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

void enable_raw_mode();
void disable_raw_mode();

void switch_to_alternate_screen_buffer();
void return_to_main_screen_buffer();

void cleanup_terminal();

int get_window_size(int *rows, int *cols);

void clear_entire_screen();

void move_cursor_to_home();

#endif
