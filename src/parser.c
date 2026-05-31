#include "parser.h"

#include <ctype.h>
#include <stdbool.h>

static void shell_begin_argument(shell_command_t *command, char *start, bool *argument_open)
{
  if (*argument_open)
  {
    return;
  }

  if (command->argc < SHELL_MAX_ARGS)
  {
    command->argv[command->argc++] = start;
  }

  *argument_open = true;
}

static void shell_end_argument(char **write, bool *argument_open)
{
  if (!*argument_open)
  {
    return;
  }

  **write = '\0';
  (*write)++;
  *argument_open = false;
}

static bool shell_is_argument_separator(char current, bool in_single_quotes, bool in_double_quotes)
{
  return !in_single_quotes && !in_double_quotes && isspace((unsigned char)current);
}

static bool shell_is_double_quote_escape(char current)
{
  return current == '"' || current == '\\';
}

// Split a mutable command line into shell words, removing quote syntax.
void shell_parse_command(char *line, shell_command_t *command)
{
  // Reset the command view before filling it from the input buffer.
  command->name = "";
  command->argc = 0;

  // read scans the original text; write stores the cleaned parsed text.
  char *read = line;
  char *write = line;

  // Inside quotes, whitespace is copied instead of ending an argument.
  bool in_single_quotes = false;
  bool in_double_quotes = false;
  bool argument_open = false;

  for (; *read != '\0'; read++)
  {
    char current = *read;

    if (current == '\\' && !in_single_quotes && !in_double_quotes)
    {
      shell_begin_argument(command, write, &argument_open);
      read++;

      if (*read == '\0')
      {
        break;
      }

      *write++ = *read;
      continue;
    }

    if (current == '\\' && in_double_quotes)
    {
      // Inside double quotes, backslash only escapes selected characters.
      shell_begin_argument(command, write, &argument_open);

      if (shell_is_double_quote_escape(read[1]))
      {
        read++;
        *write++ = *read;
        continue;
      }

      *write++ = current;
      continue;
    }

    if (current == '\'' && !in_double_quotes)
    {
      // Single quote marks affect parsing, but are not copied into the argument.
      shell_begin_argument(command, write, &argument_open);
      in_single_quotes = !in_single_quotes;
      continue;
    }

    if (current == '"' && !in_single_quotes)
    {
      // Double quote marks affect parsing, but are not copied into the argument.
      shell_begin_argument(command, write, &argument_open);
      in_double_quotes = !in_double_quotes;
      continue;
    }

    if (shell_is_argument_separator(current, in_single_quotes, in_double_quotes))
    {
      // Repeated unquoted whitespace collapses because closed args stay closed.
      shell_end_argument(&write, &argument_open);
      continue;
    }

    // Regular characters start or continue the current argument.
    shell_begin_argument(command, write, &argument_open);
    *write++ = current;
  }

  // Terminate the last argument when the line ends.
  shell_end_argument(&write, &argument_open);

  // execv-style argv arrays must end with a NULL sentinel.
  command->argv[command->argc] = NULL;

  if (command->argc > 0)
  {
    // Keep name as a convenient alias for argv[0].
    command->name = command->argv[0];
  }
}
