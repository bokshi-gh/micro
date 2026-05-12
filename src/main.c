#include "cli.h"

#include <stdio.h>

int main(int argc, char *argv[]) {
  handle_cli(argc, argv);
  printf("Hello, World!\n");
  return 0;
}
