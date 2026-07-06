#include "editor.h"

static Editor edtr;

Editor *get_editor() {
  return &edtr;
}

void init_editor(string) {
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
    // strip \n first, then \r if present
    int len = strlen(line);
    if (len > 0 && line[len-1] == '\n') line[--len] = '\0';
    if (len > 0 && line[len-1] == '\r') line[--len] = '\0';

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

void insert_char(int c) {
    // if no rows exist yet, add one empty row
    if (edtr.row_count == 0) {
        edtr.rows = realloc(edtr.rows, sizeof(char *) * 1);
        edtr.rows[0] = strdup("");
        edtr.row_count = 1;
    }

    char *row = edtr.rows[edtr.cy];
    int len = strlen(row);

    // allocate len+2: original + new char + null terminator
    char *new_row = malloc(len + 2);

    // copy everything before cursor
    memcpy(new_row, row, edtr.cx);

    // insert the new character
    new_row[edtr.cx] = c;

    // copy everything from cursor to end, including '\0'
    memcpy(new_row + edtr.cx + 1, row + edtr.cx, len - edtr.cx + 1);

    free(edtr.rows[edtr.cy]);
    edtr.rows[edtr.cy] = new_row;

    edtr.cx++;  // advance cursor past inserted char
}

// Option A: insert literal \t (stored as \t, rendered expanded)
void insert_tab() {
    insert_char('\t');
}

/*
// Option B: insert 4 spaces (simpler cursor math, no tab expansion needed)
void insert_tab() {
    int spaces = 4 - (edtr.cx % 4);  // align to next tab stop
    for (int i = 0; i < spaces; i++) {
        insert_char(' ');
    }
}
*/

void split_row_at_cursor() {
    // make room for one more row pointer
    edtr.rows = realloc(edtr.rows, sizeof(char *) * (edtr.row_count + 1));

    // shift every row below cursor down by one slot
    // memmove handles overlapping regions safely
    memmove(
        &edtr.rows[edtr.cy + 2],   // destination: two slots below current
        &edtr.rows[edtr.cy + 1],   // source: one slot below current
        sizeof(char *) * (edtr.row_count - edtr.cy - 1)
    );

    char *row = edtr.rows[edtr.cy];
    int len = strlen(row);

    // new row = everything FROM cursor to end
    edtr.rows[edtr.cy + 1] = malloc(len - edtr.cx + 1);
    memcpy(edtr.rows[edtr.cy + 1], row + edtr.cx, len - edtr.cx + 1);

    // current row = everything BEFORE cursor (truncate at cx)
    edtr.rows[edtr.cy] = realloc(row, edtr.cx + 1);
    edtr.rows[edtr.cy][edtr.cx] = '\0';

    edtr.row_count++;
    edtr.cy++;   // move cursor to the new row
    edtr.cx = 0; // beginning of new row
}

void refresh_screen() {
  write(STDOUT_FILENO, "\x1b[?25l", 6); // hide cursor
  clear_entire_screen_and_move_cursor_to_home();

  int rows, cols;
  get_window_size(&rows, &cols);

  int i;

  for (i = 0; i < edtr.row_count; i++) {
    write(STDOUT_FILENO, "\x1b[K", 3);
    write(STDOUT_FILENO, edtr.rows[i], strlen(edtr.rows[i]));
    if (i < edtr.row_count - 1) write(STDOUT_FILENO, "\r\n", 2);
  }

  for (; i < rows; i++) {
    write(STDOUT_FILENO, "\x1b[K\r\n", 5);
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
      break;

    case ENTER:
      split_row_at_cursor();
      break;

    case BACKSPACE:
    case DELETE:
      break;

    case TAB:
      insert_tab();
      break;

    case ARROW_UP:
      if (edtr.cy > 0) {
        edtr.cy--;
        int len = line_len(edtr.cy);
        if (edtr.cx > len) edtr.cx = len;
      }
      break;

    case ARROW_DOWN:
      if (edtr.cy < edtr.row_count - 1) {
        edtr.cy++;
        int len = line_len(edtr.cy);
        if (edtr.cx > len) edtr.cx = len;
      }
      break;

    case ARROW_LEFT:
      if (edtr.cx > 0) {
        edtr.cx--;
      } else if (edtr.cy > 0) {
        edtr.cy--;
        edtr.cx = line_len(edtr.cy);
      }
      break;

    case ARROW_RIGHT:
      if (edtr.cy < edtr.row_count) {
        int len = line_len(edtr.cy);

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
        insert_char(c);
      }
      break;
  }

  if (edtr.row_count == 0) {
    edtr.cy = 0;
    edtr.cx = 0;
  }

  if (edtr.cy >= edtr.row_count && edtr.row_count > 0) {
    edtr.cy = edtr.row_count - 1;
  }

  int len = line_len(edtr.cy);
  if (edtr.cx > len) edtr.cx = len;
}
