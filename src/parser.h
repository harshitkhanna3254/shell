#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

#define SHELL_MAX_ARGS 128

/**
 * Parsed command view backed by the mutable input line buffer.
 */
typedef struct
{
  char *name;
  size_t argc;
  char *argv[SHELL_MAX_ARGS + 1];
} shell_command_t;

/**
 * Parse one mutable input line into shell words.
 *
 * @param line Mutable command text read from stdin.
 * @param command Output view whose fields point into line.
 */
void shell_parse_command(char *line, shell_command_t *command);

#endif
