#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <limits.h>

#include "hyper-shell.h"
#include "util.h"

int main(const int argc, const char* const argv[]) {
  if (argc < 2)
    panic("No paths provided - exiting.");

  const char* const shell = getenv("SHELL");
  if (shell == NULL)
    panic("Environment variable 'SHELL' is not set - exiting.");

  const unsigned int dir_count = argc - 1;
  char directories[dir_count][PATH_MAX];
  hs_parse_directories(directories, argc, argv);

  // Main loop
  char command[MAX_CMD_LEN] = "";
  const struct hs_command commands[dir_count];
  while (true) {
    printf("\n> ");
    fgets(command, MAX_CMD_LEN, stdin);
    hs_directories_to_commands(directories, dir_count, commands, command);
    hs_execute_command(commands, dir_count, false);
  }

  return 0;
}
