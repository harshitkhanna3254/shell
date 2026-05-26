#include "parser.h"

#include <string.h>

// Split on the first space so builtins can decide how to treat arguments.
void shell_parse_command(char *line, shell_command_t *command)
{
  // The command name always starts at the beginning of the input buffer.
  command->name = line;
  // A first space means everything after it is the raw argument string.
  command->arguments = strchr(line, ' ');

  if (command->arguments != 0)
  {
    // Terminate the command name in-place and advance to the argument tail.
    *command->arguments = '\0';
    command->arguments++;
  }
}
