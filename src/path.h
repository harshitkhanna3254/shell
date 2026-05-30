#ifndef PATH_H
#define PATH_H

/**
 * Find an executable command by searching the PATH environment variable.
 *
 * @param command Executable name to look up.
 * @return Newly allocated executable path when found, otherwise NULL.
 */
char *shell_find_executable_path(const char *command);

#endif
