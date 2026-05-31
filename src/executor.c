#include "executor.h"

#include "path.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

// Resolve an external command, fork, exec with parsed argv, and wait.
void shell_run_external_command(const shell_command_t *command)
{
  // Resolve first so the parent can print "command not found" without forking.
  char *executable_path = shell_find_executable_path(command->name);
  if (executable_path == NULL)
  {
    printf("%s: command not found\n", command->name);
    return;
  }

  // fork duplicates the current process; only the child calls execv.
  pid_t child_pid = fork();
  if (child_pid == 0)
  {
    // Replace the child process with the target executable.
    execv(executable_path, command->argv);
    _exit(1);
  }

  if (child_pid > 0)
  {
    // Keep the prompt synchronous until the foreground command exits.
    waitpid(child_pid, NULL, 0);
  }

  free(executable_path);
}
