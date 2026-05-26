#ifndef BUILTINS_H
#define BUILTINS_H

#include "parser.h"

#include <stdbool.h>

// Return whether the command name is handled directly by this shell.
bool shell_is_builtin_command(const char *command);

// Execute a builtin command and update loop state when it exits the shell.
void shell_run_builtin_command(const shell_command_t *command, bool *should_exit);

#endif
