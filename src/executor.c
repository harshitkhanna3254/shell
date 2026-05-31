#include "executor.h"

#include "path.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static bool shell_apply_stdout_redirect(const char *path)
{
  if (path == NULL)
  {
    return true;
  }

  int file_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (file_fd == -1)
  {
    perror(path);
    return false;
  }

  if (dup2(file_fd, STDOUT_FILENO) == -1)
  {
    perror("dup2");
    close(file_fd);
    return false;
  }

  close(file_fd);
  return true;
}

// Resolve a non-builtin command, run it in a child, and wait.
void shell_run_external_command(const shell_command_t *command)
{
  // Resolve first so the parent can print "command not found" without forking.
  char *executable_path = shell_find_executable_path(command->name);
  if (executable_path == NULL)
  {
    printf("%s: command not found\n", command->name);
    return;
  }

  // The child owns redirection so the parent shell keeps its stdout.
  pid_t child_pid = fork();
  if (child_pid == 0)
  {
    if (!shell_apply_stdout_redirect(command->stdout_path))
    {
      _exit(1);
    }

    // execv receives the parser-built argv directly.
    execv(executable_path, command->argv);
    _exit(1);
  }

  if (child_pid > 0)
  {
    // Foreground commands finish before the next prompt.
    waitpid(child_pid, NULL, 0);
  }

  free(executable_path);
}
