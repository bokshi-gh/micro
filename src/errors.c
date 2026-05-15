#include "errors.h"

void die(const char *mes) {
  clear_entire_screen();
  move_cursor_to_home();

  perror(mes);
  exit(1);
}
