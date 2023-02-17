#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/syslimits.h>

#include "hyper-shell.h"

int main(const int argc, const char* const argv[]) {
  if (argc < 2)
    handle_error("No paths provided - exiting.");

  const char* const shell = getenv("SHELL");
  if (shell == NULL)
    handle_error("Environment variable 'SHELL' is not set - exiting.");

  const unsigned int dir_count = argc - 1;
  char directories[dir_count][PATH_MAX];
  parse_directories(directories, argc, argv);

  // Main loop
  char command[MAX_CMD_LEN + 1] = "";
  while (true) {
    printf("\n> ");
    scanf("%"stringify(MAX_CMD_LEN)"s", command);
    execute_command(directories, dir_count, shell, command);
  }

  return 0;
}
