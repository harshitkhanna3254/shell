#ifndef INPUT_H
#define INPUT_H

#include <sys/types.h>

// Configure terminal-facing IO behavior before the prompt loop starts.
void shell_configure_io(void);

// Prompt for and read one input line, returning getline-style length/status.
ssize_t shell_read_line(char **line);

#endif
