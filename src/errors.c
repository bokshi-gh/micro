#include "errors.h"

void die(const char *mes) {
  cleanup_terminal();

  perror(mes);
  exit(1);
}
