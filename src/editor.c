#include "editor.h"

static Editor edtr;

void init_editor() {
  edtr.filename = NULL;
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
