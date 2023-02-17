#include <stdlib.h>
#include <stdio.h>

void handle_error(const char* const message) {
  puts(message);
  exit(-1);
}

int main(void) {

  const char* shell_path = getenv("SHELL");
  if (shell_path == NULL) {
    handle_error("Environment variable 'SHELL' is not set - exiting.");
  }

  printf("SHELL: %s", shell_path);

  return 0;
}
