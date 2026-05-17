#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
  // Flush after every printf
  setbuf(stdout, NULL);

  while (1)
  {
    printf("$ ");

    char *line = NULL;
    size_t linecap = 0;

    ssize_t linelen = getline(&line, &linecap, stdin);

    if (linelen == -1)
    {
      free(line);
      break;
    }

    // Remove trailing newline
    if (linelen > 0 && line[linelen - 1] == '\n')
    {
      line[linelen - 1] = '\0';
    }

    // if the shell receives the string "exit" then terminate shell
    if (strcmp("exit", line) == 0)
    {
      break;
    }

    printf("%s: command not found\n", line);

    free(line);
  }

  return 0;
}