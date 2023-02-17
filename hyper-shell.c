#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/syslimits.h>

#define _stringify(x) #x
#define stringify(x) _stringify(x)

#define ERROR_MSG_LEN       100
#define MAX_CMD_LEN         200
#define MAX_CMD_OUTPUT_LEN  100

static void handle_error(const char* const message) {
  puts(message);
  exit(-1);
}

static void parse_directories(char directories[][PATH_MAX], const int argc, const char* const argv[]) {
  char error_msg[ERROR_MSG_LEN] = "";
  char resolved_path[PATH_MAX];
  const unsigned int path_count = argc - 1;

  for (unsigned int i = 0; i < path_count; ++i) {
    const char* const path = argv[i+1];
    
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

int main(const int argc, const char* const argv[]) {
  char error_msg[ERROR_MSG_LEN] = "";

  if (argc < 2)
    handle_error("No paths provided - exiting.");

  const char* const shell_path = getenv("SHELL");
  if (shell_path == NULL)
    handle_error("Environment variable 'SHELL' is not set - exiting.");

  const unsigned int path_count = argc - 1;
  char directories[path_count][PATH_MAX];
  parse_directories(directories, argc, argv);

  // Main loop
  char command[MAX_CMD_LEN + 1] = "";
  char command_output[MAX_CMD_OUTPUT_LEN] = "";
  while (true) {
    printf("\n> ");
    scanf("%"stringify(MAX_CMD_LEN)"s", command);

    // Execute for each directory
    for (unsigned int i = 0; i < path_count; ++i) {
      printf("\n\033[32m%s\033[0m\n", directories[i]);

      // TODO: Prepend directory
      // TODO: Execute in default shell
      // TODO: Redirect error out
      FILE* fd = popen(command, "r");
      if (fd == NULL)
        handle_error("Failed to execute popen command - exiting.");

      while(fgets(command_output, MAX_CMD_OUTPUT_LEN, fd) != NULL) {
        printf("%s", command_output);
      }

      const int exit_status = pclose(fd);
      if (exit_status != 0)  
        printf("\033[31mExit status: %d\033[0m\n", exit_status);
    }
  }

  return 0;
}
