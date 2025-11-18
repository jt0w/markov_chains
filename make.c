#define CHIMERA_IMPLEMENTATION
#define CHIMERA_STRIP_PREFIX
#include <chimera.h>
#include <stdbool.h>

Cmd cmd = {0};

#define build_dir "build/"

int main(int argc, char **argv) {
  rebuild_file(argv, argc);
  create_dir(build_dir);
  cmd_push(&cmd, CHIMERA_COMPILER);
  cmd_push(&cmd, "-Wall", "-Wextra", "-Wno-missing-braces", "-Wno-unused-parameter" , "-ggdb", "-std=gnu23");
  cmd_push(&cmd, "main.c");
  cmd_push(&cmd, "-o", build_dir"main");
  if (!cmd_exec(&cmd))
    return 1;
  return 0;
}
