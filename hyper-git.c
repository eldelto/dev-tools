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
  char remote[MAX_REMOTE_LEN];
  char default_branch[MAX_BRANCH_LEN];
  char path[PATH_MAX];
};

#define MAX_REPO_COUNT 100
struct repository* repos;
unsigned int repo_offset = 0;

char config_path[PATH_MAX] = "";

static void strip_filename(const char path[PATH_MAX], char buffer[PATH_MAX]) {
  size_t len = (strrchr(path, '/') - path) + 1;
  if (len > PATH_MAX) len = PATH_MAX;
  strlcpy(buffer, path, len);
}

static void resolve_relative_path(char relative_path[PATH_MAX], const char base_path[PATH_MAX]) {
  if (!strncmp(relative_path, "/", 1)) return;

  char resolved_base_path[PATH_MAX];
  if (realpath(base_path, resolved_base_path) == NULL) {
      panic("Could not resolve realpath of base_path - exiting.");
  }

  char buffer[PATH_MAX];
  snprintf(buffer, PATH_MAX, "%s/%s", resolved_base_path, relative_path);
  strlcpy(relative_path, buffer, PATH_MAX);
}

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
        panic("Your config file contains too many repositories.");
      }
    }
    strlcpy(prev_section, section, sizeof(prev_section));
  }

  // Populate the repository struct.
  struct repository* repo = &repos[repo_offset];
  if (strcmp("remote", name) == 0)
    strlcpy((char*)repo->remote, value, MAX_REMOTE_LEN);
  else if (strcmp("default_branch", name) == 0)
    strlcpy((char*)repo->default_branch, value, MAX_BRANCH_LEN);
  else if (strcmp("path", name) == 0) {
    char config_directory[PATH_MAX] = "";
    strip_filename(config_path, config_directory);
    strlcpy((char*)repo->path, value, PATH_MAX);
    resolve_relative_path(repo->path, config_directory);
  }

  return 1;
}

static void repo_create_dir(const struct repository* const repo) {
  char command[PATH_MAX];
  snprintf(command, sizeof(command), "mkdir -p %s", repo->path);

  int err = system(command);
  if (err) {
    char error_msg[ERROR_MSG_LEN] = "";
    snprintf(error_msg, ERROR_MSG_LEN, "Failed to create directory '%s' - exiting.", repo->path);
    panic(error_msg);
  }
}

static bool is_git_project(const char path[]) {
  char git_path[PATH_MAX];
  snprintf((char*)git_path, sizeof(git_path), "%s/.git", path);
  return !access(git_path, F_OK);
}

static void hg_sync(const struct repository repos[], const unsigned int repos_len) {
  // Checkout default branch and pull changes.
  const struct command commands[repos_len];

  for (unsigned int i = 0; i < repos_len; ++i) {
    const struct repository repo = repos[i];

    repo_create_dir(&repo);

    const struct command* cmd = &commands[i];
    strlcpy((char*)cmd->directory, repo.path, PATH_MAX);

    // Clone the repository if the .git folder does not exist.
    if (is_git_project(repo.path)) {
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
}

static void usage() {
  puts("Usage: hyper-git [-c config-path] [command]\n");
  puts("Commands:");
  puts("  sync - Executes git clone, stash, checkout <default branch> & pull.");
}

static void validate_command(const char* const command) {
  if (strcmp("sync", command) == 0)
    return;

  usage();
  panic("Unknown command");
}

static void default_config_path(char buffer[PATH_MAX]) {
  char* home = getenv("HOME");
  snprintf(buffer, PATH_MAX, "%s/.dev-tools/hyper-git.ini", home);
}

int main(int argc, char* argv[]) {
  default_config_path(config_path);

  int ch = 0;
  while ((ch = getopt(argc, argv, "hc:")) != -1) {
    switch (ch) {
    case 'h':
    case '?':
      usage();
      return 0;
    case 'c':
      strlcpy(config_path, optarg, PATH_MAX);
    }
  }

  if (argc <= optind) {
    usage();
    panic("No command provided.");
  }

  char* command = argv[optind];
  validate_command(command);

  if (!config_path[0]) {
    usage();
    panic("No config path provided or HOME environment variable is not set.");
  }

  repos = malloc(sizeof(*repos) * MAX_REPO_COUNT);

  int err = ini_parse(config_path, callback, NULL);
  if (err < 0) {
    printf("Can't read config file '%s'.\n", config_path);
    return 2;
  }
  else if (err) {
    printf("Bad config file (first error on line %d).\n", err);
    return 3;
  }

  hg_sync(repos, repo_offset + 1);
  return 0;
}
