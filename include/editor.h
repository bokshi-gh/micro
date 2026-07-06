#pragma once

#define TARGET "micro"
#define VERSION "0.0.1"
#define DESCRIPTION "a terminal-based text editor"

#define CTRL_KEY(k) ((k) & 0x1f)

#include "errors.h"
#include "terminal.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

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
  string filename;

  vector<string> rows;

  int cy;
  int cx;
} Editor;


void editor_open_file();
void init_editor();

void refresh_screen();
void process_keypress();
