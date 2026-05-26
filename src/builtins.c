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

    if (strcmp(command->arguments, "~") == 0)
    {
      char *home_env = getenv("HOME");
      chdir(home_env);
    }

    // Change the parent shell's working directory, so it persists.
    // chdir returns -1 when the target path cannot be entered.
    if (chdir(command->arguments) == -1)
    {
      printf("cd: %s: No such file or directory\n", command->arguments);
    }
  }

  if (strcmp("exit", command->name) == 0)
  {
    // Signal the REPL loop to stop after this command.
    *should_exit = true;
    return;
  }

  if (strcmp("echo", command->name) == 0)
  {
    // Match basic shell behavior by printing nothing for missing arguments.
    printf("%s\n", command->arguments == NULL ? "" : command->arguments);
    return;
  }

  if (strcmp("type", command->name) == 0)
  {
    // Report whether a name resolves to a builtin or executable path.
    if (command->arguments != NULL && shell_is_builtin_command(command->arguments))
    {
      printf("%s is a shell builtin\n", command->arguments);
    }
    else
    {
      // Non-builtin type targets are resolved the same way external commands are.
      char *executable_path = shell_find_executable_path(command->arguments);
      if (executable_path != NULL)
      {
        // shell_find_executable_path returns heap memory that must be freed.
        printf("%s is %s\n", command->arguments, executable_path);
        free(executable_path);
      }
      else
      {
        printf("%s: not found\n", command->arguments == NULL ? "" : command->arguments);
      }
    }
  }
}
