#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "parser.h"

// Run a non-builtin command by resolving it and executing it in a child.
void shell_run_external_command(const shell_command_t *command);

#endif
