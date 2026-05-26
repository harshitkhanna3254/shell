#include "path.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Search PATH entries for a regular executable file matching command.
char *shell_find_executable_path(const char *command)
{
  if (command == NULL || command[0] == '\0')
  {
    // Ignore empty command names before touching PATH.
    return NULL;
  }

  // getenv returns process-owned memory, so copy before tokenizing.
  char *path = getenv("PATH");
  if (path == NULL)
  {
    return NULL;
  }

  char *path_copy = malloc(strlen(path) + 1);
  if (path_copy == NULL)
  {
    return NULL;
  }

  strcpy(path_copy, path);

  // strtok mutates the PATH copy while walking colon-separated directories.
  char *directory = strtok(path_copy, ":");
  while (directory != NULL)
  {
    // Account for slash and NUL terminator in "directory/command".
    size_t candidate_length = strlen(directory) + strlen(command) + 2;
    char *candidate_path = malloc(candidate_length);

    if (candidate_path == NULL)
    {
      free(path_copy);
      return NULL;
    }

    snprintf(candidate_path, candidate_length, "%s/%s", directory, command);

    // stat gives file type information before access checks executability.
    struct stat file_stat;
    // A command match must be a regular file and executable by this process.
    if (stat(candidate_path, &file_stat) == 0 && S_ISREG(file_stat.st_mode) && access(candidate_path, X_OK) == 0)
    {
      free(path_copy);
      return candidate_path;
    }

    free(candidate_path);
    // Passing NULL continues tokenizing the same PATH copy.
    directory = strtok(NULL, ":");
  }

  free(path_copy);
  return NULL;
}
