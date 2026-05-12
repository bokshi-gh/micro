#include "cli.h"

void show_no_input() {
  fprintf(stderr, "%s: no input file provided\n", TARGET);
  fprintf(stderr, "Try %s'%s --help'%s for more information.\n", ANSI_GREEN, TARGET, ANSI_RESET);
}

void show_help() {
  printf("%s - %s\n\n", TARGET, DESCRIPTION);

  printf("Usage: %s [OPTIONS | FILE]\n\n", TARGET);

  printf("Options:\n");
  printf(" %s-h%s, %s--help%s \t Show help\n", ANSI_YELLOW, ANSI_RESET, ANSI_YELLOW, ANSI_RESET);
  printf(" %s-v%s, %s--version%s \t Show version\n", ANSI_YELLOW, ANSI_RESET, ANSI_YELLOW, ANSI_RESET);


  printf("Keybindings:\n");
  printf(" %sCtrl-S%s \t Save file\n", ANSI_YELLOW, ANSI_RESET);
  printf(" %sCtrl-X%s \t Quit editor\n", ANSI_YELLOW, ANSI_RESET);
}

void show_version() { printf("%s %s\n", TARGET, VERSION); }

void show_unknown(const char *arg) {
  fprintf(stderr, "%s: unknown option '%s'\n", TARGET, arg);
  fprintf(stderr, "Try %s'%s --help'%s for more information.\n", ANSI_GREEN, TARGET, ANSI_RESET);
}

void handle_cli(int argc, char *argv[]) {
  if (argc == 1) {
    show_no_input();
    exit(1);
  } else if (argc == 2) {
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
      show_help();
    } else  if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
      show_version();
    } else {
      return;
    }
  } else {
    show_unknown(argv[1]);
    exit(1);
  }

  exit(0);
}
