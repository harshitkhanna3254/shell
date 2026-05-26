#ifndef PATH_H
#define PATH_H

// Return a newly allocated executable path found in PATH, or NULL.
char *shell_find_executable_path(const char *command);

#endif
