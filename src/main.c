#include "shell.h"

// Entry point: delegate all shell behavior to the REPL runner.
int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  return shell_run();
}
