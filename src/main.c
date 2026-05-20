#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int is_builtin_command(const char *command, const char *const builtin_commands[], size_t builtin_count)
{
  // Check whether the command is implemented by this shell.
  for (size_t i = 0; i < builtin_count; i++)
  {
    if (strcmp(command, builtin_commands[i]) == 0)
    {
      return 1;
    }
  }

  return 0;
}

static char *find_executable_path(const char *command)
{
  // Empty commands cannot be resolved to executable files.
  if (command == NULL || command[0] == '\0')
  {
    return NULL;
  }

  // Use the process PATH to search for external executables.
  char *path = getenv("PATH");
  if (path == NULL)
  {
    return NULL;
  }

  // Copy PATH because strtok mutates the buffer while scanning directories.
  char *path_copy = malloc(strlen(path) + 1);
  if (path_copy == NULL)
  {
    return NULL;
  }
  strcpy(path_copy, path);

  char *directory = strtok(path_copy, ":");
  while (directory != NULL)
  {
    // Build one possible executable path from the current PATH directory.
    size_t candidate_length = strlen(directory) + strlen(command) + 2;
    char *candidate_path = malloc(candidate_length);
    if (candidate_path == NULL)
    {
      free(path_copy);
      return NULL;
    }

    snprintf(candidate_path, candidate_length, "%s/%s", directory, command);

    struct stat file_stat;
    // A valid match must be a regular file that the current process can execute.
    if (stat(candidate_path, &file_stat) == 0 && S_ISREG(file_stat.st_mode) && access(candidate_path, X_OK) == 0)
    {
      free(path_copy);
      return candidate_path;
    }

    // Move on to the next PATH directory after rejecting this candidate.
    free(candidate_path);
    directory = strtok(NULL, ":");
  }

  free(path_copy);
  return NULL;
}

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  // Keep prompts and command output visible immediately.
  setbuf(stdout, NULL);

  // Builtins are handled inside this process instead of executed from PATH.
  const char *builtin_commands[] = {"echo", "exit", "type"};
  const size_t builtin_count = sizeof(builtin_commands) / sizeof(builtin_commands[0]);

  while (1)
  {
    printf("$ ");

    char *line = NULL;
    size_t line_capacity = 0;

    // Read a full command line for the current prompt.
    ssize_t line_length = getline(&line, &line_capacity, stdin);

    if (line_length == -1)
    {
      free(line);
      break;
    }

    // Drop the newline so commands can be compared directly.
    if (line_length > 0 && line[line_length - 1] == '\n')
    {
      line[line_length - 1] = '\0';
    }

    char *command = line;
    char *arguments = strchr(line, ' ');

    // Split the command name from the rest of the input.
    if (arguments != NULL)
    {
      *arguments = '\0';
      arguments++;
    }

    if (!is_builtin_command(command, builtin_commands, builtin_count))
    {
      // Unknown commands currently fail instead of being executed externally.
      printf("%s: command not found\n", command);
      free(line);
      continue;
    }

    // Exit terminates the shell loop.
    if (strcmp("exit", command) == 0)
    {
      free(line);
      break;
    }

    if (strcmp("echo", command) == 0)
    {
      // Echo prints the argument string exactly as entered after the command.
      printf("%s\n", arguments == NULL ? "" : arguments);
      free(line);
      continue;
    }

    if (strcmp("type", command) == 0)
    {
      // Type first reports shell builtins before checking the PATH.
      if (arguments != NULL && is_builtin_command(arguments, builtin_commands, builtin_count))
      {
        printf("%s is a shell builtin\n", arguments);
        free(line);
        continue;
      }
      else
      {
        // Non-builtin type arguments are resolved like executable names.
        char *executable_path = find_executable_path(arguments);
        if (executable_path != NULL)
        {
          printf("%s is %s\n", arguments, executable_path);
          free(executable_path);
        }
        else
        {
          printf("%s: not found\n", arguments == NULL ? "" : arguments);
        }
      }
    }

    free(line);
  }

  return 0;
}
