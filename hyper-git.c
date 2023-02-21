#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

#include "util.h"
#include "hyper-shell.h"
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

#define MAX_REPO_COUNT 100
struct repository* repos;
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

typedef int(*repo_callback)(const struct repository* const);

static int for_each_repo(
  const struct repository repos[],
  const unsigned int repos_len,
  repo_callback callback
) {
  int err = 0;
  for (unsigned int i = 0; i < repos_len; ++i) {
    const struct repository repo = repos[i];
    if (!repo.path[0] || !repo.default_branch[0] || !repo.remote[0])
      return 0; // TODO: Abort

    err = callback(&repo);
    if (err)
      return err;
  }

  return 0;
}

static int repo_create_dir(const struct repository* const repo) {
  printf("Creating directory '%s'\n", repo->path);

  char command[200];
  snprintf(command, sizeof(command), "mkdir -p %s", repo->path);
  return system(command);
}

static int hg_sync(const struct repository repos[], const unsigned int repos_len) {
  int err = for_each_repo(repos, repos_len, repo_create_dir);
  if (err)
    return err;

  // Checkout default branch and pull changes.
  const struct command commands[repos_len];
  char git_path[PATH_MAX];
  for (unsigned int i = 0; i < repos_len; ++i) {
    const struct command* cmd = &commands[i];
    const struct repository repo = repos[i];

    strlcpy((char*)cmd->directory, repo.path, PATH_MAX);

    // Clone the repository if the .git folder does not exist.
    snprintf((char*)git_path, sizeof(git_path), "%s/.git", repo.path);
    if (access(git_path, F_OK) == 0) {
      snprintf((char*)cmd->command, MAX_CMD_LEN,
        "git stash && git checkout %s && git pull", repo.default_branch);
    }
    else {
      snprintf((char*)cmd->command, MAX_CMD_LEN,
        "git clone %s . && git stash && git checkout %s && git pull",
        repo.remote, repo.default_branch);
    }
  }

  execute_command(commands, repos_len, true);
  return 0;
}

int main(int argc, char* argv[]) {
  if (argc < 2)
    panic("No path to .ini file provided - exiting.");

  repos = malloc(sizeof(*repos) * MAX_REPO_COUNT);

  int error = ini_parse(argv[1], callback, NULL);
  if (error < 0) {
    printf("Can't read '%s'!\n", argv[1]);
    return 2;
  }
  else if (error) {
    printf("Bad config file (first error on line %d)!\n", error);
    return 3;
  }

  int err = hg_sync(repos, repo_offset + 1);
  if (err)
    return err;

  return 0;
}
