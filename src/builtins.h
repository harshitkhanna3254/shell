#ifndef BUILTINS_H
#define BUILTINS_H

#include "parser.h"

#include <stdbool.h>

/**
 * Check whether a command name is implemented directly by this shell.
 *
 * @param command Command name to compare against the builtin table.
 * @return true when the shell should run the command in-process.
 */
bool shell_is_builtin_command(const char *command);

/**
 * Execute a parsed builtin command in the current shell process.
 *
 * @param command Parsed command name and raw argument tail.
 * @param should_exit Output flag set to true when the exit builtin is run.
 */
void shell_run_builtin_command(const shell_command_t *command, bool *should_exit);

#endif
