#include "cli.h"
#include "terminal.h"
#include "editor.h"

int main(int argc, char *argv[]) {
  handle_cli(argc, argv);

  enable_raw_mode();
  switch_to_alternate_screen_buffer();

  // IMPORTANT: atexit() executes functions in reverse order (LIFO)
  atexit(disable_raw_mode);
  atexit(return_to_main_screen_buffer);

  init_editor();
  editor_open_file(argv[1]);

  while (1) { 
    refresh_screen();
    process_keypress();
  }

  return 0;
}
