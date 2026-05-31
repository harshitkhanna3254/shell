#include "builtins.h"

#include "path.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Builtins must run in the shell process when they affect shell state.
static const char *builtin_commands[] = {"echo", "exit", "type", "pwd", "cd"};
static const size_t builtin_count = sizeof(builtin_commands) / sizeof(builtin_commands[0]);

// Check the fixed builtin table before falling back to PATH lookup.
bool shell_is_builtin_command(const char *command)
{
  // Linear search is fine for this tiny fixed set of builtin names.
  for (size_t i = 0; i < builtin_count; i++)
  {
    if (strcmp(command, builtin_commands[i]) == 0)
    {
      return true;
    }
  }

  return false;
}

// Dispatch supported builtins using the parsed command name.
void shell_run_builtin_command(const shell_command_t *command, bool *should_exit)
{
  const char *first_argument = command->argc > 1 ? command->argv[1] : NULL;

  if (strcmp("pwd", command->name) == 0)
  {
    // Ask libc for an allocated buffer sized to the current directory.
    char *temp = NULL;
    size_t size = 0;

    // getcwd(NULL, 0) returns heap memory sized for the current directory.
    char *working_dir = getcwd(temp, size);

    printf("%s\n", working_dir);

    free(working_dir);
    return;
  }

  if (strcmp("cd", command->name) == 0)
  {
    const char *target = first_argument == NULL ? getenv("HOME") : first_argument;

    if (target != NULL && strcmp(target, "~") == 0)
    {
      target = getenv("HOME");
    }

    // Change the parent shell's working directory, so it persists.
    // chdir returns -1 when the target path cannot be entered.
    if (target == NULL || chdir(target) == -1)
    {
      printf("cd: %s: No such file or directory\n", first_argument == NULL ? "" : first_argument);
    }

    return;
  }

  if (strcmp("exit", command->name) == 0)
  {
    // Signal the REPL loop to stop after this command.
    *should_exit = true;
    return;
  }

  if (strcmp("echo", command->name) == 0)
  {
    // Print parsed arguments with one separating space, like a basic echo.
    for (size_t i = 1; i < command->argc; i++)
    {
      if (i > 1)
      {
        printf(" ");
      }

      printf("%s", command->argv[i]);
    }

    printf("\n");
    return;
  }

  if (strcmp("type", command->name) == 0)
  {
    // Report whether a name resolves to a builtin or executable path.
    if (first_argument != NULL && shell_is_builtin_command(first_argument))
    {
      printf("%s is a shell builtin\n", first_argument);
    }
    else
    {
      // Non-builtin type targets are resolved the same way external commands are.
      char *executable_path = shell_find_executable_path(first_argument);
      if (executable_path != NULL)
      {
        // shell_find_executable_path returns heap memory that must be freed.
        printf("%s is %s\n", first_argument, executable_path);
        free(executable_path);
      }
      else
      {
        printf("%s: not found\n", first_argument == NULL ? "" : first_argument);
      }
    }
  }
}
