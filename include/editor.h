#ifndef EDITOR_H
#define EDITOR_H

#define TARGET "micro"
#define VERSION "0.0.1"
#define DESCRIPTION "a terminal-based text editor"

#define shutdown_editor() cleanup_terminal()

#define CTRL_KEY(k) ((k) & 0x1f)

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
} Editor;

void init_editor();
void process_keypress();

#endif
