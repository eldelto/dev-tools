#ifndef HYPER_SHELL
#define HYPER_SHELL

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <limits.h>

#include "util.h"

#define MAX_CMD_LEN         200
#define MAX_FULL_CMD_LEN    400
#define MAX_CMD_OUTPUT_LEN  512

void hs_parse_directories(char directories[][PATH_MAX], const int argc, const char* const argv[]) {
  char error_msg[ERROR_MSG_LEN] = "";
  const unsigned int path_count = argc - 1;

  for (unsigned int i = 0; i < path_count; ++i) {
    const char* const path = argv[i + 1];

    struct stat path_stat;
    lstat(path, &path_stat);
    if (!S_ISDIR(path_stat.st_mode)) {
      snprintf(error_msg, ERROR_MSG_LEN, "Path '%s' is not a directory - exiting.", path);
      panic(error_msg);
    }

    if (realpath(path, directories[i]) == NULL) {
      snprintf(error_msg, ERROR_MSG_LEN, "Failed to resolve absolute path of '%s' - exiting.", path);
      panic(error_msg);
    }
  }
}

struct hs_command {
  char directory[PATH_MAX];
  char command[MAX_CMD_LEN];
};

void hs_directories_to_commands(
  const char directories[][PATH_MAX],
  const unsigned int len,
  const struct hs_command commands[],
  const char command[MAX_CMD_LEN]
) {
  for (unsigned int i = 0; i < len; ++i) {
    const struct hs_command* cmd = &commands[i];
    strlcpy((char*)cmd->directory, directories[i], PATH_MAX);
    strlcpy((char*)cmd->command, command, MAX_CMD_LEN);
  }
}

void hs_execute_command(
  const struct hs_command commands[],
  const unsigned int commands_len,
  const bool fail_fast
) {
  const char* const shell = getenv("SHELL");
  if (shell == NULL)
    panic("Environment variable 'SHELL' is not set - exiting.");

  char full_command[MAX_FULL_CMD_LEN] = "";
  char command_output[MAX_CMD_OUTPUT_LEN] = "";

  // Launch processes
  FILE* pipes[commands_len];
  for (unsigned int i = 0; i < commands_len; ++i) {
    const struct hs_command cmd = commands[i];
    snprintf(full_command, MAX_FULL_CMD_LEN, "cd %s && %s -c \"%s\" 2>&1",
      cmd.directory, shell, cmd.command);

    FILE* fd = popen(full_command, "r");
    if (fd == NULL)
      panic("Failed to execute popen command - exiting.");

    pipes[i] = fd;
  }

  // Serialize process output
  for (unsigned int i = 0; i < commands_len; ++i) {
    const struct hs_command cmd = commands[i];
    printf("\n\033[32m%s\033[0m\n", cmd.directory);

    FILE* fd = pipes[i];
    while (fgets(command_output, MAX_CMD_OUTPUT_LEN, fd) != NULL) {
      fputs(command_output, stdout);
    }

    const int exit_status = pclose(fd);
    if (exit_status) {
      printf("\033[31mExit status: %d\033[0m\n", exit_status);

      if (fail_fast)
        panic("Command execution returned non-zero exit status - exiting.");
    }
  }
}

#endif
