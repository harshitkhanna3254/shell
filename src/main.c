#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_EXTERNAL_ARGS 128

static int is_builtin_command(const char *command, const char *const builtin_commands[], size_t builtin_count)
{
  // Check this shell's builtin table before trying any external command path.
  for (size_t i = 0; i < builtin_count; i++)
  {
    // strcmp checks each builtin name; it returns 0 only for an exact C-string match.
    if (strcmp(command, builtin_commands[i]) == 0)
    {
      return 1;
    }
  }

  return 0;
}

static char *find_executable_path(const char *command)
{
  // Empty command names cannot be turned into executable filesystem paths.
  if (command == NULL || command[0] == '\0')
  {
    return NULL;
  }

  // Read PATH from this process environment so type can search shell-visible executables.
  // getenv returns the environment value without copying it.
  char *path = getenv("PATH");
  // Without PATH there is nowhere for type to search for external commands.
  if (path == NULL)
  {
    return NULL;
  }

  // strlen sizes a writable PATH copy; malloc reserves that heap storage.
  // The copy is needed because strtok mutates the string it tokenizes.
  char *path_copy = malloc(strlen(path) + 1);
  // If malloc fails, report no match instead of dereferencing a NULL buffer.
  if (path_copy == NULL)
  {
    return NULL;
  }
  // strcpy copies PATH into our writable buffer for tokenization.
  strcpy(path_copy, path);

  // strtok splits the PATH copy on ':' and writes '\0' at each separator.
  char *directory = strtok(path_copy, ":");

  // Test each PATH directory until an executable regular file is found.
  while (directory != NULL)
  {
    // Use strlen on both pieces to size a "directory/command" candidate exactly.
    size_t candidate_length = strlen(directory) + strlen(command) + 2;

    // malloc reserves heap storage for the candidate path until this loop frees it.
    char *candidate_path = malloc(candidate_length);

    // If candidate allocation fails, clean up the PATH copy before returning.
    if (candidate_path == NULL)
    {
      free(path_copy);
      return NULL;
    }

    // snprintf builds "directory/command" while respecting the candidate buffer size.
    snprintf(candidate_path, candidate_length, "%s/%s", directory, command);

    struct stat file_stat;

    // stat writes metadata into file_stat; keep it on the stack for this local check.
    // access verifies execute permission, and S_ISREG rejects non-regular files.
    if (stat(candidate_path, &file_stat) == 0 && S_ISREG(file_stat.st_mode) && access(candidate_path, X_OK) == 0)
    {
      // The caller owns candidate_path, so only the temporary PATH copy is cleaned up here.
      free(path_copy);
      return candidate_path;
    }

    // Reject this candidate and move on to the next PATH directory.
    free(candidate_path);

    // strtok(NULL, ...) resumes scanning the same PATH copy for the next directory.
    directory = strtok(NULL, ":");
  }

  // No match was found, so clean up the copied PATH before returning.
  free(path_copy);
  return NULL;
}

static void run_external_command(char *command, char *arguments)
{
  // Resolve the command through PATH before forking so the parent can report misses.
  char *executable_path = find_executable_path(command);
  if (executable_path == NULL)
  {
    printf("%s: command not found\n", command);
    return;
  }

  char *external_argv[MAX_EXTERNAL_ARGS];
  size_t external_argc = 0;

  // execv expects argv[0] to be the program name seen by the new process.
  external_argv[external_argc++] = command;

  // Split the remaining input on spaces to pass basic arguments to the program.
  char *argument = arguments == NULL ? NULL : strtok(arguments, " ");
  while (argument != NULL && external_argc < MAX_EXTERNAL_ARGS - 1)
  {
    external_argv[external_argc++] = argument;
    argument = strtok(NULL, " ");
  }
  external_argv[external_argc] = NULL;

  // fork creates a child process so execv can replace only the child image.
  pid_t child_pid = fork();
  if (child_pid == 0)
  {
    // execv runs the resolved program path with our argv array.
    execv(executable_path, external_argv);

    // If execv returns, execution failed; _exit avoids flushing inherited stdio buffers.
    _exit(1);
  }

  if (child_pid > 0)
  {
    // waitpid keeps the shell prompt from returning until this child exits.
    waitpid(child_pid, NULL, 0);
  }

  // The parent no longer needs the heap path returned by find_executable_path.
  free(executable_path);
}

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  // Disable stdout buffering with setbuf so prompts appear before getline waits for input.
  setbuf(stdout, NULL);

  // Builtins are handled inside this process instead of being resolved through PATH.
  const char *builtin_commands[] = {"echo", "exit", "type", "pwd", "cd"};
  const size_t builtin_count = sizeof(builtin_commands) / sizeof(builtin_commands[0]);

  while (1)
  {
    printf("$ ");

    char *line = NULL;
    size_t line_capacity = 0;

    // line starts NULL so getline can allocate a buffer sized for the input.
    // getline reads from stdin and allocates or grows line to hold the full command.
    ssize_t line_length = getline(&line, &line_capacity, stdin);

    // getline returns -1 on EOF or read error, so the shell leaves the loop.
    if (line_length == -1)
    {
      // Clean up any buffer getline allocated before EOF or read failure.
      free(line);
      break;
    }

    // Drop getline's trailing newline so strcmp can compare command names directly.
    if (line_length > 0 && line[line_length - 1] == '\n')
    {
      line[line_length - 1] = '\0';
    }

    // The command name starts at the beginning of the mutable getline buffer.
    char *command = line;

    // strchr locates the first space so we can split command name from arguments.
    char *arguments = strchr(line, ' ');

    // Replace the separator with '\0' so command points to a standalone C string.
    if (arguments != NULL)
    {
      // Split at the space, then advance so arguments begins after it.
      *arguments = '\0';
      arguments++;
    }

    if (!is_builtin_command(command, builtin_commands, builtin_count))
    {
      // Non-builtin commands are resolved through PATH and executed in a child.
      run_external_command(command, arguments);
      // Each loop iteration owns one getline buffer and releases it before continuing.
      free(line);
      continue;
    }

    if (strcmp("pwd", command) == 0)
    {
      char *temp = NULL;
      size_t size = 0;

      char *working_dir = getcwd(temp, size);

      printf("%s\n", working_dir);

      free(working_dir);
      continue;
    }

    if (strcmp("cd", command) == 0)
    {
      if (chdir(arguments) == -1)
      {
        printf("cd: %s: No such file or directory\n", arguments);
      }
    }

    // Builtin dispatch uses strcmp for exact command-name matches.
    if (strcmp("exit", command) == 0)
    {
      // exit takes effect immediately; later builtins are skipped.
      free(line);
      break;
    }

    // echo is handled in-process, so its arguments can be printed directly.
    if (strcmp("echo", command) == 0)
    {
      // echo accepts missing arguments, so NULL is rendered as an empty string.
      printf("%s\n", arguments == NULL ? "" : arguments);
      free(line);
      continue;
    }

    // type explains whether a name resolves to a builtin or PATH executable.
    if (strcmp("type", command) == 0)
    {
      // type reports shell builtins before searching PATH for executable files.
      if (arguments != NULL && is_builtin_command(arguments, builtin_commands, builtin_count))
      {
        printf("%s is a shell builtin\n", arguments);
        free(line);
        continue;
      }
      else
      {
        // Non-builtin type arguments are resolved through PATH like executable names.
        char *executable_path = find_executable_path(arguments);
        // A non-NULL result means find_executable_path returned heap storage to print.
        if (executable_path != NULL)
        {
          printf("%s is %s\n", arguments, executable_path);
          // find_executable_path returns heap storage only when it finds a match.
          free(executable_path);
        }
        else
        {
          // Report that neither a builtin nor PATH executable matched.
          printf("%s: not found\n", arguments == NULL ? "" : arguments);
        }
      }

      free(line);
      continue;
    }

    // Future builtin branches that fall through still must release the input buffer.
    free(line);
  }

  return 0;
}
