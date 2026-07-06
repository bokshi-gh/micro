#pragma once


#define TARGET "micro"
#define VERSION "0.0.1"
#define DESCRIPTION "a terminal-based text editor"

#define shutdown_editor() cleanup_terminal()

#define CTRL_KEY(k) ((k) & 0x1f)

#define _POSIX_C_SOURCE 200809L

#include "errors.h"
#include "terminal.h"

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  TAB = '\t',
  ENTER = '\r',
  BACKSPACE = 127,
  DELETE = 1000,

  ARROW_UP,
  ARROW_DOWN,
  ARROW_LEFT,
  ARROW_RIGHT
} KeyboardKey;

typedef struct {
  char *filename;

  char **rows;
  int row_count;

  int cy;
  int cx;
} Editor;

void init_editor();
void editor_open_file();

void refresh_screen();
void process_keypress();
