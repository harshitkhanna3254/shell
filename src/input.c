#include "input.h"

#include <stdio.h>
#include <stdlib.h>

// Disable stdout buffering so prompts appear immediately.
void shell_configure_io(void)
{
  setbuf(stdout, NULL);
}

// Print the shell prompt, read a line, and strip the trailing newline.
ssize_t shell_read_line(char **line)
{
  // A zero capacity lets getline allocate a buffer for the caller.
  size_t line_capacity = 0;

  printf("$ ");

  // getline returns -1 for EOF or error and otherwise stores bytes in *line.
  ssize_t line_length = getline(line, &line_capacity, stdin);
  if (line_length == -1)
  {
    return -1;
  }

  // Keep command parsing focused on command text, not the Enter key.
  if (line_length > 0 && (*line)[line_length - 1] == '\n')
  {
    (*line)[line_length - 1] = '\0';
  }

  return line_length;
}
