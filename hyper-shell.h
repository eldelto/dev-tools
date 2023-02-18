#ifndef HYPER_SHELL
#define HYPER_SHELL

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/syslimits.h>

#define _stringify(x) #x
#define stringify(x) _stringify(x)

#define ERROR_MSG_LEN       100
#define MAX_CMD_LEN         200
#define MAX_FULL_CMD_LEN    400
#define MAX_CMD_OUTPUT_LEN  100

static void handle_error(const char* const message) {
  puts(message);
  exit(-1);
}

void parse_directories(char directories[][PATH_MAX], const int argc, const char* const argv[]) {
  char error_msg[ERROR_MSG_LEN] = "";
  const unsigned int path_count = argc - 1;

  for (unsigned int i = 0; i < path_count; ++i) {
    const char* const path = argv[i + 1];

    struct stat path_stat;
    lstat(path, &path_stat);
    if (!S_ISDIR(path_stat.st_mode)) {
      snprintf(error_msg, ERROR_MSG_LEN, "Path '%s' is not a directory - exiting.", path);
      handle_error(error_msg);
    }

    if (realpath(path, directories[i]) == NULL) {
      snprintf(error_msg, ERROR_MSG_LEN, "Failed to resolve absolute path of '%s' - exiting.", path);
      handle_error(error_msg);
    }
  }
}

void execute_command(
  const char directories[][PATH_MAX],
  const unsigned int dir_len,
  const char* const shell,
  const char* const command
) {
  char full_command[MAX_FULL_CMD_LEN] = "";
  char command_output[MAX_CMD_OUTPUT_LEN] = "";

  for (unsigned int i = 0; i < dir_len; ++i) {
    const char* directory = directories[i];
    printf("\n\033[32m%s\033[0m\n", directory);

    snprintf(full_command, MAX_FULL_CMD_LEN, "cd %s && %s -c %s 2>&1", directory, shell, command);
    FILE* fd = popen(full_command, "r");
    if (fd == NULL)
      handle_error("Failed to execute popen command - exiting.");

    while (fgets(command_output, MAX_CMD_OUTPUT_LEN, fd) != NULL) {
      printf("%s", command_output);
    }

    const int exit_status = pclose(fd);
    if (exit_status != 0)
      printf("\033[31mExit status: %d\033[0m\n", exit_status);
  }
}

#endif
