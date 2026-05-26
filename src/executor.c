#include "executor.h"

#include "path.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_EXTERNAL_ARGS 128

// Resolve an external command, prepare argv, fork, exec, and wait.
void shell_run_external_command(const shell_command_t *command)
{
  // Resolve first so the parent can print "command not found" without forking.
  char *executable_path = shell_find_executable_path(command->name);
  if (executable_path == NULL)
  {
    printf("%s: command not found\n", command->name);
    return;
  }

  char *external_argv[MAX_EXTERNAL_ARGS];
  size_t external_argc = 0;

  // By convention argv[0] is the command name seen by the new program.
  external_argv[external_argc++] = command->name;

  // External commands need argv split into separate tokens for execv.
  char *argument = command->arguments == NULL ? NULL : strtok(command->arguments, " ");
  while (argument != NULL && external_argc < MAX_EXTERNAL_ARGS - 1)
  {
    external_argv[external_argc++] = argument;
    argument = strtok(NULL, " ");
  }
  // execv expects argv to end with a NULL sentinel.
  external_argv[external_argc] = NULL;

  // fork duplicates the current process; only the child calls execv.
  pid_t child_pid = fork();
  if (child_pid == 0)
  {
    // Replace the child process with the target executable.
    execv(executable_path, external_argv);
    _exit(1);
  }

  if (child_pid > 0)
  {
    // Keep the prompt synchronous until the foreground command exits.
    waitpid(child_pid, NULL, 0);
  }

  free(executable_path);
}
