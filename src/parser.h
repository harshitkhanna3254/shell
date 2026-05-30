#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

/**
 * Parsed command view backed by the mutable input line buffer.
 */
typedef struct
{
  char *name;
  char *arguments;
} shell_command_t;

/**
 * Parse one mutable input line into the command name and raw argument tail.
 *
 * @param line Mutable command text read from stdin. The first space is replaced with '\0'.
 * @param command Output view whose fields point into line.
 */
void shell_parse_command(char *line, shell_command_t *command);

/**
 * Parse a mutable raw argument tail into argv-style tokens, handling single quotes.
 *
 * @param arguments Mutable argument text. Quotes are removed and token separators become '\0'.
 * @param argv Output array filled with pointers into arguments and terminated with NULL.
 * @param max_args Capacity of argv, including the final NULL sentinel.
 * @return Number of parsed arguments, excluding the NULL sentinel.
 */
size_t shell_parse_arguments(char *arguments, char **argv, size_t max_args);

#endif
