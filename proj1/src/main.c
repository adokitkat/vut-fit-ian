#include "etc.h"
#include <elf.h>
#include <gelf.h>

int main(int argc, char *argv[])
{
  bool show_help = false;
  bool verbose = false;
  char* token; 

  parseArgs(argc, argv, &show_help, &verbose, &token);

  printf("%s %s %s\n", show_help ? "true" : "false", verbose ? "true" : "false", token);


  if (argc < 2) {
    showHelp();
    return 0;
  }

  return 0;
}