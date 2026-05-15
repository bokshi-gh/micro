#include "editor.h"

static Editor edtr;

void init_editor() {
  edtr.filename = NULL;
  edtr.rows = NULL;
  edtr.row_count = 0;
}

void editor_open_file() {
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    return; // empty buffer if file doesn't exist
  }

  char *line = NULL;
  size_t cap = 0;
  ssize_t len;

  while ((len = getline(&line, &cap, fp)) != -1) {

    // remove newline
    if (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
      line[len - 1] = '\0';
      len--;
    }

    edtr.rows = realloc(edtr.rows, sizeof(char *) * (edtr.row_count + 1));

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
}

int read_key() {
  int nread;
  char c;

  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }

  if (c == '\x1b') {
    char seq[3];

    if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

    if (seq[0] == '[') {
      if (seq[1] >= '0' && seq[1] <= '9') {

        if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';

        if (seq[2] == '~') {
          switch (seq[1]) {
            case '3':
              return DELETE;
          }
        }

      } else {
        switch (seq[1]) {
          case 'A':
            return ARROW_UP;

          case 'B':
            return ARROW_DOWN;

          case 'C':
            return ARROW_RIGHT;

          case 'D':
            return ARROW_LEFT;
        }
      }
    }

    return '\x1b';

  } else {
    return c;
  }
}

void process_keypress() {
  int c = read_key();

  switch (c) {
    case CTRL_KEY('s'):
      /* save */
      break;

    case CTRL_KEY('q'):
      editor_free(); // optional
      shutdown_editor();
      exit(0);
      break;

    case ENTER:
      /* handle enter */
      break;

    case BACKSPACE:
    case DELETE:
      /* handle delete */
      break;

    case TAB:
      /* handle tab */
      break;

    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_LEFT:
    case ARROW_RIGHT:
      break;

    default:
      /* insert normal character */
      break;
  }
}
