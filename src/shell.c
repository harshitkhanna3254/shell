#include "shell.h"

#include "builtins.h"
#include "executor.h"
#include "input.h"
#include "parser.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int shell_open_stdout_redirect(const char *path)
{
  int file_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (file_fd == -1)
  {
    perror(path);
  }

  return file_fd;
}

static void shell_run_builtin_with_redirect(const shell_command_t *command, bool *should_exit)
{
  if (command->stdout_path == NULL)
  {
    shell_run_builtin_command(command, should_exit);
    return;
  }

  int saved_stdout = dup(STDOUT_FILENO);
  if (saved_stdout == -1)
  {
    perror("dup");
    return;
  }

  int file_fd = shell_open_stdout_redirect(command->stdout_path);
  if (file_fd == -1)
  {
    close(saved_stdout);
    return;
  }

  // Builtins run in the shell process, so stdout must be restored afterward.
  fflush(stdout);
  if (dup2(file_fd, STDOUT_FILENO) == -1)
  {
    perror("dup2");
    close(file_fd);
    close(saved_stdout);
    return;
  }

  close(file_fd);
  shell_run_builtin_command(command, should_exit);

  fflush(stdout);
  if (dup2(saved_stdout, STDOUT_FILENO) == -1)
  {
    perror("dup2");
  }

  close(saved_stdout);
}

// Run the prompt loop and dispatch each parsed command.
int shell_run(void)
{
  shell_configure_io();

  bool should_exit = false;

  while (!should_exit)
  {
    // getline owns this buffer until the command has finished running.
    char *line = NULL;
    ssize_t line_length = shell_read_line(&line);

    // EOF or read failure ends the interactive session cleanly.
    if (line_length == -1)
    {
      free(line);
      break;
    }

    shell_command_t command;
    // Parsed fields point into line, so line stays alive through dispatch.
    shell_parse_command(line, &command);

    if (command.argc == 0)
    {
      free(line);
      continue;
    }

    // Builtins mutate shell state; external programs run in child processes.
    if (shell_is_builtin_command(command.name))
    {
      shell_run_builtin_with_redirect(&command, &should_exit);
    }
    else
    {
      shell_run_external_command(&command);
    }

    // Releasing line also invalidates command argv/name pointers.
    free(line);
  }

  return 0;
}
