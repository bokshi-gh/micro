#include "errors.h"

void die(const char *mes) {
  /* optional */
  // editor_free();
  
  cleanup_terminal();

  perror(mes);
  exit(1);
}
