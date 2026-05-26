#include "shell.h"

#include "builtins.h"
#include "executor.h"
#include "input.h"
#include "parser.h"

#include <stdbool.h>
#include <stdlib.h>

// Coordinate input, parsing, dispatch, and cleanup for each prompt cycle.
int shell_run(void)
{
  shell_configure_io();

  bool should_exit = false;

  while (!should_exit)
  {
    // Read a single command line from the user.
    char *line = NULL;
    ssize_t line_length = shell_read_line(&line);

    // EOF or read failure ends the interactive session cleanly.
    if (line_length == -1)
    {
      free(line);
      break;
    }

    shell_command_t command;
    // The parsed command points into line, so line must stay alive while it runs.
    shell_parse_command(line, &command);

    // Builtins run in-process; external commands run through fork/exec.
    if (shell_is_builtin_command(command.name))
    {
      shell_run_builtin_command(&command, &should_exit);
    }
    else
    {
      shell_run_external_command(&command);
    }

    // Free the getline buffer after command handling is complete.
    free(line);
  }

  return 0;
}
