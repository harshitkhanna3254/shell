#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  // Flush after every printf
  setbuf(stdout, NULL);

  printf("$ ");

  char *line = NULL;
  size_t linecap = 0;

  ssize_t linelen = getline(&line, &linecap, stdin);

  if (linelen == -1)
  {
    free(line);
    return 0;
  }

  // Remove trailing newline
  if (linelen > 0 && line[linelen - 1] == '\n')
  {
    line[linelen - 1] = '\0';
  }

  printf("%s: command not found\n", line);

  free(line);
  return 0;
}