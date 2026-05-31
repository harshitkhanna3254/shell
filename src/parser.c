#include "parser.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>

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

static bool shell_is_stdout_redirect_token(const char *token)
{
  return strcmp(token, ">") == 0 || strcmp(token, "1>") == 0;
}

static void shell_mark_current_argument_quoted(const shell_command_t *command, bool argument_was_quoted[])
{
  if (command->argc > 0)
  {
    argument_was_quoted[command->argc - 1] = true;
  }
}

static void shell_extract_stdout_redirect(shell_command_t *command, const bool argument_was_quoted[])
{
  for (size_t i = 0; i < command->argc; i++)
  {
    if (argument_was_quoted[i] || !shell_is_stdout_redirect_token(command->argv[i]))
    {
      continue;
    }

    if (i + 1 < command->argc)
    {
      command->stdout_path = command->argv[i + 1];
    }

    command->argc = i;
    command->argv[i] = NULL;
    return;
  }
}

// Parse shell syntax in-place so argv points directly into the input buffer.
void shell_parse_command(char *line, shell_command_t *command)
{
  // Start with an empty command view for blank or whitespace-only input.
  command->name = "";
  command->stdout_path = NULL;
  command->argc = 0;
  bool argument_was_quoted[SHELL_MAX_ARGS] = {false};

  // read consumes source text while write compacts parsed text over it.
  char *read = line;
  char *write = line;

  // Quote state decides whether whitespace is data or an argument boundary.
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
      // Double quotes only let backslash escape a small set of characters.
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
      // Quotes are syntax, not data, unless protected by the other quote type.
      shell_begin_argument(command, write, &argument_open);
      shell_mark_current_argument_quoted(command, argument_was_quoted);
      in_single_quotes = !in_single_quotes;
      continue;
    }

    if (current == '"' && !in_single_quotes)
    {
      // Quotes are syntax, not data, unless protected by the other quote type.
      shell_begin_argument(command, write, &argument_open);
      shell_mark_current_argument_quoted(command, argument_was_quoted);
      in_double_quotes = !in_double_quotes;
      continue;
    }

    if (shell_is_argument_separator(current, in_single_quotes, in_double_quotes))
    {
      // Repeated unquoted whitespace collapses into one argument boundary.
      shell_end_argument(&write, &argument_open);
      continue;
    }

    // Normal characters become part of the current shell word.
    shell_begin_argument(command, write, &argument_open);
    *write++ = current;
  }

  // Close the final word if the line ended while one was open.
  shell_end_argument(&write, &argument_open);

  // Keep argv directly usable by execv.
  command->argv[command->argc] = NULL;

  shell_extract_stdout_redirect(command, argument_was_quoted);

  if (command->argc > 0)
  {
    // The executable name is always the first parsed word.
    command->name = command->argv[0];
  }
}
