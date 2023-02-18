#include <stdio.h>
#include <string.h>

#include <sys/syslimits.h>

#include "deps/inih/ini.h"

#define MAX_REMOTE_LEN 100
#define MAX_BRANCH_LEN 50
struct repository {
  const char remote[MAX_REMOTE_LEN];
  const char default_branch[MAX_BRANCH_LEN];
  const char path[PATH_MAX];
};

// static void repository_new(
//   const struct repository* repo,
//   const char* const remote,
//   const char* const default_branch,
//   const char* const path,
// ) {
//   strlcpy((char*)repo->remote, remote, MAX_REMOTE_LEN);
//   strlcpy((char*)repo->default_branch, default_branch, MAX_BRANCH_LEN);
//   strlcpy((char*)repo->path, path, PATH_MAX);
// }

#define MAX_REPO_COUNT 10
const struct repository repos[MAX_REPO_COUNT];
unsigned int repo_offset = 0;

static int callback(
  __attribute__((__unused__)) void* user,
  const char* section,
  const char* name,
  const char* value
) {
  // Check for new section.
  static char prev_section[50] = "";
  if (strcmp(section, prev_section)) {
    if (prev_section[0]) {
      ++repo_offset;
      if (repo_offset >= MAX_REPO_COUNT) {
        // TODO: Abort with error
        return 0;
      }
    }
    strlcpy(prev_section, section, sizeof(prev_section));
  }

  // Populate the repository struct.
  const struct repository* repo = &repos[repo_offset];
  if (strcmp("remote", name) == 0)
    strlcpy((char*)repo->remote, value, MAX_REMOTE_LEN);
  else if (strcmp("default_branch", name) == 0)
    strlcpy((char*)repo->default_branch, value, MAX_BRANCH_LEN);
  else if (strcmp("path", name) == 0)
    strlcpy((char*)repo->path, value, PATH_MAX);

  return 1;
}

int main(int argc, char* argv[]) {
  int error;

  if (argc <= 1) {
    printf("Usage: ini_dump filename.ini\n");
    return 1;
  }

  error = ini_parse(argv[1], callback, NULL);
  if (error < 0) {
    printf("Can't read '%s'!\n", argv[1]);
    return 2;
  }
  else if (error) {
    printf("Bad config file (first error on line %d)!\n", error);
    return 3;
  }

  for (unsigned int i = 0; i <= repo_offset; ++i) {
    const struct repository repo = repos[i];
    printf("remote=%s branch=%s path=%s\n", repo.remote, repo.default_branch, repo.path);
  }
  return 0;
}
