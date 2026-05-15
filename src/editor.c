#include "editor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

static Editor edtr;

/* expose if needed elsewhere */
Editor *get_editor() {
  return &edtr;
}

void init_editor() {
  edtr.filename = NULL;
  edtr.rows = NULL;
  edtr.row_count = 0;
  edtr.cy = 0;
  edtr.cx = 0;
}

void editor_open_file(const char *filename) {
  edtr.filename = strdup(filename);

  FILE *fp = fopen(filename, "r");
  if (!fp) return;

  char *line = NULL;
  size_t cap = 0;

  while (getline(&line, &cap, fp) != -1) {

    int len = strlen(line);
    if (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
      line[len - 1] = '\0';
    }

    char **new_rows = realloc(edtr.rows,
                              sizeof(char *) * (edtr.row_count + 1));
    if (!new_rows) break;

    edtr.rows = new_rows;

    edtr.rows[edtr.row_count] = strdup(line);
    edtr.row_count++;
  }

  free(line);
  fclose(fp);
}

// Frees all editor-owned heap memory.
// Note: not strictly required at program exit because the OS
// reclaims all process memory automatically, but we still do it
// for good practice and to catch leaks during development.
void editor_free() {
  for (int i = 0; i < edtr.row_count; i++) {
    free(edtr.rows[i]);
  }

  free(edtr.rows);
  free(edtr.filename);

  edtr.rows = NULL;
  edtr.filename = NULL;
  edtr.row_count = 0;
}

void refresh_screen() {
  write(STDOUT_FILENO, "\x1b[?25l", 6);  // hide cursor
  write(STDOUT_FILENO, "\x1b[H", 3);     // home

  int rows, cols;
  get_window_size(&rows, &cols);

  for (int i = 0; i < rows; i++) {

    if (i < edtr.row_count) {
      write(STDOUT_FILENO,
            edtr.rows[i],
            strlen(edtr.rows[i]));
    }

    write(STDOUT_FILENO, "\x1b[K", 3);  // clear line
    write(STDOUT_FILENO, "\r\n", 2);
  }

  char buf[32];
  snprintf(buf, sizeof(buf),
           "\x1b[%d;%dH",
           edtr.cy + 1,
           edtr.cx + 1);

  write(STDOUT_FILENO, buf, strlen(buf));

  write(STDOUT_FILENO, "\x1b[?25h", 6); // show cursor
}

int read_key() {
  char c;
  int nread;

  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN)
      die("read");
  }

  if (c == '\x1b') {
    char seq[3];

    if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

    if (seq[0] == '[') {

      if (seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';

        if (seq[2] == '~') {
          if (seq[1] == '3') return DELETE;
        }

      } else {
        switch (seq[1]) {
          case 'A': return ARROW_UP;
          case 'B': return ARROW_DOWN;
          case 'C': return ARROW_RIGHT;
          case 'D': return ARROW_LEFT;
        }
      }
    }

    return '\x1b';
  }

  return c;
}

void process_keypress() {
  int c = read_key();

  switch (c) {
    case CTRL_KEY('q'):
      editor_free();
      shutdown_editor();
      exit(0);
      break;

    case CTRL_KEY('s'):
      // TODO: save file
      break;

    case ENTER:
      // TODO: split current line at cx
      break;

    case BACKSPACE:
    case DELETE:
      // TODO: delete character or merge lines
      break;

    case TAB:
      // TODO: insert spaces or tab
      break;

    case ARROW_UP:
      if (edtr.cy > 0) {
        edtr.cy--;

        int len = 0;
        if (edtr.cy < edtr.row_count)
          len = strlen(edtr.rows[edtr.cy]);

        if (edtr.cx > len)
          edtr.cx = len;
      }
      break;

    case ARROW_DOWN:
      if (edtr.cy < edtr.row_count - 1) {
        edtr.cy++;

        int len = strlen(edtr.rows[edtr.cy]);

        if (edtr.cx > len)
          edtr.cx = len;
      }
      break;

    case ARROW_LEFT:
      if (edtr.cx > 0) {
        edtr.cx--;
      } else if (edtr.cy > 0) {
        edtr.cy--;
        edtr.cx = strlen(edtr.rows[edtr.cy]);
      }
      break;

    case ARROW_RIGHT:
      if (edtr.cy < edtr.row_count) {

        int len = strlen(edtr.rows[edtr.cy]);

        if (edtr.cx < len) {
          edtr.cx++;
        } else if (edtr.cy < edtr.row_count - 1) {
          edtr.cy++;
          edtr.cx = 0;
        }
      }
      break;

    default:
      if (c >= 32 && c <= 126) {
        // TODO: insert character at (cx, cy)
      }
      break;
  }
}
