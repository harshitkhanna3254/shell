#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_supported_command(const char *command, const char *commands_supported[], size_t count_commands)
{
  for (size_t i = 0; i < count_commands; i++)
  {
    if (strcmp(command, commands_supported[i]) == 0)
    {
      return 1;
    }
  }

  return 0;
}

int main(int argc, char *argv[])
{
  // Flush after every printf
  setbuf(stdout, NULL);

  // Supported commands
  const char *commands_supported[] = {"echo", "exit", "type"};
  const size_t count_commands = sizeof(commands_supported) / sizeof(commands_supported[0]);

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

    char *command = line;
    char *args = strchr(line, ' ');

    if (args != NULL)
    {
      *args = '\0';
      args++;
    }

    if (!is_supported_command(command, commands_supported, count_commands))
    {
      printf("%s: command not found\n", command);
      free(line);
      continue;
    }

    // if the shell receives the string "exit" then terminate shell
    if (strcmp("exit", command) == 0)
    {
      free(line);
      break;
    }

    if (strcmp("echo", command) == 0)
    {
      printf("%s\n", args == NULL ? "" : args);
      free(line);
      continue;
    }

    if (strcmp("type", command) == 0)
    {
      if (args != NULL && is_supported_command(args, commands_supported, count_commands))
      {
        printf("%s is a shell builtin\n", args);
      }
      else
      {
        printf("%s: not found\n", args == NULL ? "" : args);
      }
    }

    free(line);
  }

  return 0;
}
