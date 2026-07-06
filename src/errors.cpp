#include "errors.h"

void die(const char *message) {
  perror(message);
  exit(1);
}
