#ifndef PARSER_H
#define PARSER_H

// Parsed command view backed by the mutable input line buffer.
typedef struct
{
  char *name;
  char *arguments;
} shell_command_t;

// Split an input line into a command name and raw argument tail.
void shell_parse_command(char *line, shell_command_t *command);

#endif
