#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "parser.h"

/**
 * Resolve and run a non-builtin command in a child process.
 *
 * @param command Parsed command name and raw argument tail.
 */
void shell_run_external_command(const shell_command_t *command);

#endif
