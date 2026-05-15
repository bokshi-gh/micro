#include "cli.h"
#include "terminal.h"
#include "editor.h"

#include <stdio.h>

int main(int argc, char *argv[]) {
  handle_cli(argc, argv);

  switch_to_alternate_screen_buffer();
  enable_raw_mode();

  // IMPORTANT: atexit() executes functions in reverse order (LIFO)
  atexit(return_to_main_screen_buffer);
  atexit(disable_raw_mode);

  while (1) { 
    refresh_screen();
    process_keypress();
  }
  return 0;
}
